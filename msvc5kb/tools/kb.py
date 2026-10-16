#!/usr/bin/env python3
"""SB1-focused VC5 symptom tags and deterministic rule lookup."""
from __future__ import annotations

import argparse, csv, difflib, json, re, struct, sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
CATALOG = ROOT / "msvc5kb" / "catalog.json"
sys.path.insert(0, str(ROOT))
import capstone
import pefile
from tools.cmpfunc import Coff

REG = re.compile(r"\b(e?[abcd]x|[abcd][lh]|e?[sd]i|e?[sb]p|st\(\d\)|st|mm\d)\b")
MEM = re.compile(r"\[([^\]]*)\]")
STACK = re.compile(r"\b(esp|ebp)\b")
COND = re.compile(r"^j(?!mp)")
OPPOSITE = {"je":"jne","jne":"je","jz":"jnz","jnz":"jz","jl":"jge","jge":"jl",
            "jle":"jg","jg":"jle","jb":"jae","jae":"jb","jbe":"ja","ja":"jbe"}
REVERSE = {"fsub":"fsubr","fsubr":"fsub","fdiv":"fdivr","fdivr":"fdiv",
           "fsubp":"fsubrp","fsubrp":"fsubp","fdivp":"fdivrp","fdivrp":"fdivp"}
TIERS = {"PROVEN":5,"CORPUS":4,"MICRO":3,"INDICATED":2,"HYPOTHESIS":1,"FALSIFIED":0}

def instructions(data: bytes, address: int, image_base: int, image_top: int):
    md = capstone.Cs(capstone.CS_ARCH_X86, capstone.CS_MODE_32)
    out = []
    for x in md.disasm(data, address):
        mn, ops = x.mnemonic, x.op_str
        if mn == "call" and "[" not in ops: norm = "F"
        elif mn.startswith("j") and "[" not in ops: norm = "J"
        else:
            norm = re.sub(r"\b0x[0-9a-f]+\b", lambda m: "A" if image_base <= int(m[0],16) < image_top else m[0], ops)
        l0 = mn + (" " + norm if norm else "")
        l1 = REG.sub("R", l0)
        kinds = []
        for op in [p.strip() for p in norm.split(",") if p.strip()]:
            kinds.append("M" if "[" in op else "A" if op in ("A","F","J") else
                         "I" if re.fullmatch(r"-?(?:0x[\da-f]+|\d+)",op) else "R")
        out.append({"off":x.address-address,"size":x.size,"mn":mn,"text":mn+" "+ops,
                    "l0":l0,"l1":l1,"l2":mn+" "+",".join(kinds),"branch":"cond" if COND.match(mn) else None})
    while out and out[-1]["mn"] in ("int3","nop"): out.pop()
    return out

def _reads(i):
    if i["mn"] == "lea": return 0
    parts=i["text"].split(None,1)[-1].split(",") if " " in i["text"] else []
    n=0
    for k,p in enumerate(parts):
        m=MEM.search(p)
        if not m or STACK.search(m[1]): continue
        if k==0 and i["mn"].startswith("mov") and len(parts)>1: continue
        n+=1
    return n

def _stack_refs(i):
    if i["mn"] in ("push", "pop") and "[" not in i["text"]: return 0
    return sum(1 for m in MEM.findall(i["text"]) if STACK.search(m) and m.strip() != "esp")

def _frame(rows):
    h=[x["text"] for x in rows[:8]]
    ebp=any(t=="push ebp" for t in h[:3]) and any(t.replace(" ","")=="movebp,esp" for t in h[:4])
    m=next((re.match(r"sub esp, (0x[0-9a-f]+|\d+)$",t) for t in h if re.match(r"sub esp, (0x[0-9a-f]+|\d+)$",t)),None)
    return ebp,int(m[1],0) if m else 0


def _store(i):
    p=i["text"].split(None,1)[-1].lstrip()
    return i["mn"].startswith("mov") and p.startswith(("dword ptr [","word ptr [","byte ptr [","[")) and not STACK.search(p.split(",")[0])

def _sib(a,b):
    x,y=MEM.search(a),MEM.search(b)
    if not x or not y or a.replace(x[1],"")!=b.replace(y[1],""): return False
    p,q=[z.strip() for z in x[1].split("+")],[z.strip() for z in y[1].split("+")]
    return p!=q and sorted(p)==sorted(q)


def tags(native, ours):
    a,b=native,ours; A=[x["l0"] for x in a]; B=[x["l0"] for x in b]
    A1=[x["l1"] for x in a]; B1=[x["l1"] for x in b]
    out=[]
    if sum(x["size"] for x in a)!=sum(x["size"] for x in b): out.append("size_differs")
    if A==B: return []
    if A1==B1: out.append("regalloc_only")
    elif sorted(A)==sorted(B): out.append("reorder_only")
    elif sorted(A1)==sorted(B1): out.append("reorder_regalloc")
    cj=lambda x: "jcc" if x["branch"] else x["l1"]
    if sorted(map(cj,a))==sorted(map(cj,b)): out.append("block_reorder")
    if any(x["branch"] and y["branch"] and OPPOSITE.get(x["mn"])==y["mn"] for x,y in zip(a,b)):
        out.append("branch_flip")
    if "reorder_only" in out or "reorder_regalloc" in out:
        sm=difflib.SequenceMatcher(None,A1,B1,autojunk=False)
        moved=[a[k] for op,i1,i2,_,_ in sm.get_opcodes() if op!="equal" for k in range(i1,i2)]
        if any(_store(x) for x in moved): out.append("store_reorder")
        if any(x["mn"]=="push" for x in moved): out.append("push_reorder")
        if any(_reads(x) for x in moved): out.append("load_reorder")
    fa=[x for x in a if x["mn"].startswith("f")]; fb=[x for x in b if x["mn"].startswith("f")]
    reverse=lambda xs: sorted(REVERSE.get(x["mn"],x["mn"]) if x["mn"] in REVERSE and x["mn"]>REVERSE[x["mn"]] else x["mn"] for x in xs)
    if fa and fb and len(fa)==len(fb) and reverse(fa)==reverse(fb) and [x["l1"] for x in a if not x["mn"].startswith("f")]==[x["l1"] for x in b if not x["mn"].startswith("f")]: out.append("x87_operand_order")
    sm=difflib.SequenceMatcher(None,[x["l2"] for x in a],[x["l2"] for x in b],autojunk=False)
    sib=imm=disp=0
    branch_flip=condition_diff=0
    for op,i1,i2,j1,j2 in sm.get_opcodes():
        if op not in ("equal","replace") or i2-i1!=j2-j1: continue
        for x,y in zip(a[i1:i2],b[j1:j2]):
            if x["l0"]==y["l0"]: continue
            if x["branch"] and y["branch"]:
                if OPPOSITE.get(x["mn"])==y["mn"]: branch_flip+=1
                elif x["mn"]!=y["mn"]: condition_diff+=1
                continue
            if _sib(x["text"],y["text"]): sib+=1; continue
            if x["mn"]==y["mn"] and x["mn"] in ("cmp","test") and x["text"].split(None,1)[-1].split(",")[::-1]==y["text"].split(None,1)[-1].split(","): continue
            if "[" in x["text"] and "[" in y["text"] and re.sub(r"\[[^\]]+\]","[M]",x["text"])==re.sub(r"\[[^\]]+\]","[M]",y["text"]): disp+=1
            elif re.sub(r"-?(?:0x[\da-f]+|\b\d+\b)","I",x["text"])==re.sub(r"-?(?:0x[\da-f]+|\b\d+\b)","I",y["text"]): imm+=1
    if sib: out.append("sib_order")
    if branch_flip: out.append("branch_flip")
    if condition_diff: out.append("condition_differs")
    if imm: out.append("immediate_differs")
    if disp: out.append("displacement_differs")
    ra,rb=sum(map(_reads,a)),sum(map(_reads,b))
    if ra>rb: out.append("native_more_loads")
    elif rb>ra: out.append("native_fewer_loads")
    ca=sum(x["mn"]=="call" for x in a); cb=sum(x["mn"]=="call" for x in b)
    if ca>cb: out.append("native_calls_more")
    elif cb>ca: out.append("ours_calls_more")
    if sum(x["branch"] is not None for x in a)!=sum(x["branch"] is not None for x in b): out.append("branch_count_differs")
    if sum(x["mn"].startswith("ret") for x in a)!=sum(x["mn"].startswith("ret") for x in b): out.append("ret_count_differs")
    sa,sb=sum(map(_stack_refs,a)),sum(map(_stack_refs,b))
    if sa>sb: out.append("native_more_stack")
    elif sb>sa: out.append("native_fewer_stack")
    if _frame(a)!=_frame(b): out.append("frame_differs")
    if difflib.SequenceMatcher(None,A1,B1,autojunk=False).ratio()<.6: out.append("structural")
    return list(dict.fromkeys(out))


def _target_size(rva, pe):
    p=ROOT/"oracle"/"functions.csv"
    if p.exists():
        with p.open(encoding="utf-8-sig",newline="") as f:
            for row in csv.DictReader(f):
                if row.get("rva") and int(row["rva"],0)==rva: return int(row["size"],0)
    return 0


def _pair(objpath,symbol,exe,rva):
    coff=Coff(objpath); fs=[f for f in coff.functions() if f.name==symbol]
    if len(fs)!=1: raise ValueError(f"object has {len(fs)} matches for {symbol}")
    f=fs[0]; pe=pefile.PE(str(exe),fast_load=False); base=pe.OPTIONAL_HEADER.ImageBase; top=base+pe.OPTIONAL_HEADER.SizeOfImage
    data=bytearray(f.data); byidx={s.index:s for s in coff.symbols}
    for rel in f.relocs:
        off=rel.offset-f.start
        if rel.typ==6 and off>=0 and off+4<=len(data):
            add=struct.unpack_from("<I",data,off)[0]; sym=byidx.get(rel.symbol_index)
            own=sym is not None and sym.section==f.symbol.section
            struct.pack_into("<I",data,off,(base+(rva if own else 0x1000)+add)&0xffffffff)
    code=data
    size=_target_size(rva,pe) or len(code); native=pe.get_data(rva,size)
    if not native: raise ValueError(f"no oracle bytes at RVA 0x{rva:x}")
    return instructions(native,base+rva,base,top),instructions(bytes(code),base+rva,base,top),{"symbol":symbol,"rva":f"0x{rva:x}","candidate_bytes":len(code),"oracle_bytes":size,"pairing":"forced RVA; advisory only"}


def _from_json(path, selector):
    doc=json.loads(Path(path).read_text(encoding="utf-8")); rows=doc if isinstance(doc,list) else [doc]; found=[]
    for unit in rows:
        obj=Path(unit["object"])
        if not obj.is_absolute(): obj=(ROOT/obj).resolve()
        for f in unit.get("functions",[]):
            if selector.lower().startswith("0x"):
                if selector.lower() not in str(f.get("matches",[])).lower() and selector.lower() not in str(f.get("best_candidate",{})).lower(): continue
            elif selector.lower() not in f["name"].lower(): continue
            ms=f.get("matches",[]); rv=next((m.get("rva") for m in ms if m.get("rva")),None) or (f.get("best_candidate") or {}).get("rva")
            if rv: found.append((obj,f["name"],int(rv,0),f.get("status")))
    if len(found)!=1: raise ValueError(f"cmpfunc JSON selector matched {len(found)} rows; use a unique decorated symbol")
    return found[0]

def _hints(symptoms, catalog, top=5, symbol=None):
    rules=[]
    for r in catalog["rules"]:
        o=r.get("applies",{}).get("opt","any");
        if o!="any" and "O2" not in (o if isinstance(o,list) else [o]): continue
        s=r.get("symptoms",{}); ts=set(symptoms); any_tags=set(s.get("any",[])); all_tags=set(s.get("all",[]))
        if not any_tags and not all_tags:
            if not symbol or symbol.lower() not in r.get("keywords","").lower(): continue
        if all_tags and not all_tags<=ts: continue
        if any_tags and not any_tags&ts and not all_tags: continue
        if set(s.get("none",[]))&ts: continue
        rules.append((r,len(any_tags&ts)+len(all_tags)))
    if ts&{"immediate_differs","condition_differs","displacement_differs"}: rules=[x for x in rules if x[0]["blupi_status"]=="validated" or x[0]["id"]=="VC5-SEM-01"]
    active=[x for x in rules if x[0]["blupi_status"] in ("validated","transferred-unverified")]
    active.sort(key=lambda x:(0 if x[0]["blupi_status"]=="validated" else 1,-TIERS.get(x[0]["tier"],0),-x[1],x[0]["id"]))
    blocked=[r for r in catalog["rules"] if r["blupi_status"] in ("inert-here","contradicted") or r["tier"]=="FALSIFIED"]
    return [r for r,_ in active[:top]],blocked[:3]


def main():
    p=argparse.ArgumentParser(description=__doc__); sub=p.add_subparsers(dest="cmd",required=True)
    t=sub.add_parser("tags"); t.add_argument("--json",type=Path); t.add_argument("--function"); t.add_argument("--object",type=Path); t.add_argument("--symbol"); t.add_argument("--rva",type=lambda x:int(x,0)); t.add_argument("--exe",type=Path,default=ROOT/"assets"/"Speedy Blupi"/"BLUPI.EXE")
    h=sub.add_parser("hints"); h.add_argument("target"); h.add_argument("--json",type=Path); h.add_argument("--object",type=Path); h.add_argument("--symbol"); h.add_argument("--rva",type=lambda x:int(x,0)); h.add_argument("--exe",type=Path,default=ROOT/"assets"/"Speedy Blupi"/"BLUPI.EXE"); h.add_argument("--top",type=int,default=5)
    r=sub.add_parser("rule"); r.add_argument("id")
    q=sub.add_parser("query"); q.add_argument("text")
    a=p.parse_args(); catalog=json.loads(CATALOG.read_text(encoding="utf-8"))
    if a.cmd=="tags" or (a.cmd=="hints" and (a.json or a.object)):
        selector=getattr(a,"function",None) or getattr(a,"target",None)
        if getattr(a,"json",None): obj,sym,rva,status=_from_json(a.json,selector); exe=a.exe
        else:
            obj=a.object; sym=(getattr(a,"symbol",None) or selector); rva=getattr(a,"rva",None); status=None; exe=a.exe
            if not (obj and sym and rva is not None): p.error("tags needs --json --function, or --object --symbol --rva")
        n,o,meta=_pair(obj,sym,exe,rva); ts=tags(n,o)
        print(json.dumps({**meta,"status":status,"tags":ts},indent=2) if a.cmd=="tags" else f"{meta['symbol']} {meta['rva']}: {status}; {meta['candidate_bytes']}/{meta['oracle_bytes']} bytes; {meta['pairing']}; tags={','.join(ts)}")
        if a.cmd=="hints":
            active,blocked=_hints(ts,catalog,a.top,sym); print(f"{selector}: tags={','.join(ts)}")
            validated=[r for r in active if r["blupi_status"]=="validated"]
            transferred=[r for r in active if r["blupi_status"]=="transferred-unverified"]
            print("  SB1 validated:")
            for rule in validated[:3]: print(f"    {rule['id']} {rule['title']} | {rule['experiment']}")
            print("  transferred hypotheses:")
            for rule in transferred[:5]: print(f"    {rule['id']} [{rule['tier']}] {rule['title']} | {rule['experiment']}")
            if blocked:
                print("  already inert/falsified - do not try:")
                for rule in blocked: print(f"    {rule['id']} {rule['title']}")
    elif a.cmd=="hints":
        w=set(re.findall(r"[a-z0-9_]+",a.target.lower())); rows=[r for r in catalog["rules"] if w&set(re.findall(r"[a-z0-9_]+",(r["title"]+" "+r.get("keywords","")).lower()))]
        print(f"{a.target}: keyword matches only")
        for r in rows[:5]: print(f"  {r['id']} {r['title']}")
    elif a.cmd=="rule":
        rid=catalog.get("legacy_map",{}).get(a.id,a.id); ids=rid if isinstance(rid,list) else [rid]
        matches=[x for x in catalog["rules"] if x["id"] in ids]
        if not matches: raise SystemExit(f"unknown rule: {a.id}")
        for rule in matches:
            print(f"{rule['id']} [{rule['blupi_status']}/{rule['tier']}] {rule['title']}")
            print(f"  Symptom: {rule['symptom']}")
            print(f"  Try: {rule['experiment']}")
            print(f"  Source form: {rule['construct']}")
            if rule.get("not_when"): print(f"  Limit: {rule['not_when']}")
            ev=rule.get("blupi_evidence") or {}
            refs=[x.get("ref","") for x in rule.get("evidence",[]) if x.get("ref")]
            print(f"  SB1: {ev.get('outcome','not tested')} ({ev.get('doc_ref','')})")
            if refs: print(f"  Provenance: {', '.join(refs)}")
    else:
        words=set(re.findall(r"[a-z0-9_]+",a.text.lower())); rows=[]
        for x in catalog["rules"]:
            text=" ".join([x["id"],x["title"],x.get("keywords","")]).lower(); score=sum(w in text for w in words)
            if score: rows.append((score,x))
        for _,x in sorted(rows,key=lambda z:(-z[0],z[1]["id"]))[:8]: print(f"{x['id']} [{x['blupi_status']}] {x['title']}")
    return 0


if __name__=="__main__":
    try: raise SystemExit(main())
    except (OSError,ValueError,KeyError) as e: print(f"kb: {e}",file=sys.stderr); raise SystemExit(1)
