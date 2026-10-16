#!/usr/bin/env python3
"""Bounded VC5 count probes and compiler/flag sweeps; comparison is delegated to cmpfunc.py."""
from __future__ import annotations

import argparse, json, re, subprocess, sys
from pathlib import Path

ROOT=Path(__file__).resolve().parents[2]
PY=sys.executable
MSVC=ROOT/"tools"/"msvc.py"
CMP=ROOT/"tools"/"cmpfunc.py"
DEFAULT_EXE=ROOT/"assets"/"Speedy Blupi"/"BLUPI.EXE"


def output_dir(path):
    p=path.resolve(); allowed=[(ROOT/"build"/"kb").resolve(),(ROOT/"scratch"/"kb"/"sync").resolve()]
    if not any(p==x or x in p.parents for x in allowed): raise ValueError("outputs must stay under build/kb or scratch/kb/sync")
    p.mkdir(parents=True,exist_ok=True); return p


def run(args,cwd=ROOT):
    p=subprocess.run(args,cwd=cwd,stdout=subprocess.PIPE,stderr=subprocess.STDOUT,text=True,encoding="utf-8",errors="replace")
    if p.returncode: raise RuntimeError(p.stdout)
    return p.stdout


def compile_one(src,obj,a,tree,flags):
    cmd=[PY,str(MSVC),str(src),"--tree",tree,"--out",str(obj)]
    inc=[a.tu.resolve().parent,*[Path(x).resolve() for x in a.include]]
    for x in dict.fromkeys(inc): cmd += ["--include",str(x)]
    for x in a.define: cmd += ["--define",x]
    for x in flags: cmd += ["--flag",x]
    log=run(cmd); obj.parent.mkdir(parents=True,exist_ok=True); (obj.with_suffix(".log")).write_text(log,encoding="utf-8")


def compare(obj,a,stem):
    out=output_dir(a.output_dir); js=out/(stem+".cmp.json")
    cmd=[PY,str(CMP),str(obj),"--exe",str(a.target_pe.resolve()),"--at",f"{a.function}={a.rva:#x}","--summary","--json-out",str(js)]
    for x in a.symbol_va: cmd += ["--symbol-va",x]
    run(cmd)
    units=json.loads(js.read_text(encoding="utf-8")); fs=[f for u in units for f in u["functions"] if f["name"]==a.function]
    if len(fs)!=1: raise ValueError(f"cmpfunc returned {len(fs)} rows for {a.function}")
    f=fs[0]; return {"status":f["status"],"candidate_bytes":f["code_size"],"matches":f["matches"]}


def inject(source,dest,count,position,anchor=None):
    raw=source.read_bytes(); bom=raw.startswith(b"\xef\xbb\xbf"); body=raw[3:] if bom else raw
    enc="utf-8" if bom else "utf-8"
    try: text=body.decode(enc)
    except UnicodeDecodeError: enc="cp1252"; text=body.decode(enc)
    nl="\r\n" if "\r\n" in text else "\n"; decl=nl.join(f"extern int kbprobe_filler_{i:06d};" for i in range(count))
    if count: decl+=nl
    if position=="top": text=decl+text
    elif position=="end": text=text+("" if text.endswith(("\n","\r")) else nl)+decl
    else:
        rows=text.splitlines(keepends=True); hits=[]
        for i,row in enumerate(rows):
            if row.lstrip().startswith("#") or not re.search(rf"\b{re.escape(anchor)}\s*\(",row): continue
            sig=row; j=i
            while j+1<len(rows) and "{" not in sig and ";" not in sig and j-i<12: j+=1; sig+=rows[j]
            if "{" in sig and (";" not in sig or sig.index("{")<sig.index(";")): hits.append(i)
        if len(hits)!=1: raise ValueError(f"before:{anchor} needs one definition; found {len(hits)}")
        rows.insert(hits[0],decl); text="".join(rows)
    dest.parent.mkdir(parents=True,exist_ok=True); dest.write_bytes((b"\xef\xbb\xbf" if bom else b"")+text.encode(enc))


def common(parser,target=False):
    parser.add_argument("--tu",required=True,type=Path); parser.add_argument("--function",required=True)
    parser.add_argument("--rva",required=True,type=lambda x:int(x,0)); parser.add_argument("--target-pe",type=Path,default=DEFAULT_EXE)
    parser.add_argument("--tree",default="sp3"); parser.add_argument("--include",action="append",default=[])
    parser.add_argument("--define",action="append",default=[]); parser.add_argument("--flag",action="append",default=[])
    parser.add_argument("--symbol-va",action="append",default=[]); parser.add_argument("--output-dir",type=Path,default=ROOT/"build"/"kb"/"vc5probe")


def probe(a):
    out=output_dir(a.output_dir); src=a.tu.resolve(); slug=re.sub(r"[^A-Za-z0-9_-]+","_",a.function)[:44]
    stable=output_dir(ROOT/"scratch"/"kb"/"sync"/"probes"/slug)/src.name
    if a.position.startswith("before:"): position,anchor="before",a.position.split(":",1)[1]
    else: position,anchor=a.position,None
    flags=a.flag or ["/O2","/GX","/ML","/Gd"]
    counts=a.count or [0,1,2]; results=[]
    for n in counts:
        inject(src,stable,n,position,anchor)
        obj=out/f"{slug}_{position}_{n}.obj"; compile_one(stable,obj,a,a.tree,flags)
        row={"count":n,**compare(obj,a,f"{slug}_{position}_{n}")}; results.append(row)
        print(f"{position} {n}: {row['status']} ({row['candidate_bytes']} candidate bytes)")
    report={"function":a.function,"rva":hex(a.rva),"tu":str(src),"position":a.position,"flags":flags,"tree":a.tree,"results":results}
    path=out/f"{slug}_{position}_probe.json"; path.write_text(json.dumps(report,indent=2)+"\n",encoding="utf-8"); print(f"report: {path}")


def sweep(a):
    out=output_dir(a.output_dir); source=a.tu.resolve(); sets=[]
    for item in a.flag_set:
        if "=" not in item: raise ValueError("--flag-set must be LABEL=/O2;/GX")
        label,raw=item.split("=",1); sets.append((label,[x for x in raw.split(";") if x]))
    if not sets: sets=[("base",[])]
    base_flags=a.flag or ["/O2","/GX","/ML","/Gd"]
    results=[]
    for tree in [x for x in a.trees.split(",") if x]:
        for label,extra in sets:
            obj=out/f"{source.stem}_{tree}_{label}.obj"; compile_one(source,obj,a,tree,base_flags+extra)
            row={"tree":tree,"flag_set":label,"flags":base_flags+extra,**compare(obj,a,f"{source.stem}_{tree}_{label}")}; results.append(row)
            print(f"{tree}/{label}: {row['status']} ({row['candidate_bytes']} candidate bytes)")
    path=out/f"{source.stem}_sweep.json"; path.write_text(json.dumps({"tu":str(source),"results":results},indent=2)+"\n",encoding="utf-8"); print(f"report: {path}")


def main():
    p=argparse.ArgumentParser(description=__doc__); s=p.add_subparsers(dest="cmd",required=True)
    x=s.add_parser("probe",help="scratch-only extern-count placement probe"); common(x); x.add_argument("--position",required=True,help="top, end, or before:FUNCTION"); x.add_argument("--count",action="append",type=int,default=[])
    y=s.add_parser("sweep",help="compiler-tree and flag sweep"); common(y); y.add_argument("--trees",default="sp3"); y.add_argument("--flag-set",action="append",default=[])
    a=p.parse_args()
    if not a.tu.is_file(): p.error(f"TU not found: {a.tu}")
    if a.cmd=="probe":
        if a.position not in ("top","end") and not a.position.startswith("before:"): p.error("invalid --position")
        if any(n<0 for n in a.count): p.error("counts must be nonnegative")
        if not a.count: a.count=[0,1,2]
        probe(a)
    else: sweep(a)
    return 0


if __name__=="__main__":
    try: raise SystemExit(main())
    except (OSError,ValueError,RuntimeError,KeyError) as e: print(f"vc5probe: {e}",file=sys.stderr); raise SystemExit(1)
