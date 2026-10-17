#!/usr/bin/env python3
"""Residue map: every unmatched game function, its candidate body, and classified differing regions.

Reads the last canonical verify run (build/verify/objects + results.json) and oracle/tu_map.csv. Each unmatched
oracle function is paired with the candidate defined between the same two located neighbours (functions are in
oracle order per TU). Candidate and oracle instruction streams are aligned with difflib on normalized
instructions (relocated operands masked), and every non-equal region is classified:

  register   same instruction shapes, only register names differ
  order      same instructions (as a multiset), different order          -> scheduling / statement order
  constant   same shapes, immediates/displacements differ                 -> constant, offset or layout
  insert     candidate has instructions the oracle lacks
  delete     oracle has instructions the candidate lacks
  mixed      anything else

Diagnostic only: nothing here proves strictness. Writes build/residues.json and prints a summary.
  .pyenv\\python.exe tools\\residues.py [--detail RVA]
"""

from __future__ import annotations

import argparse
from collections import Counter
import csv
import difflib
import json
from pathlib import Path
import re
import sys

ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / "tools"))
import capstone  # noqa: E402
import cmpfunc  # noqa: E402
import pefile  # noqa: E402

REG = r"\b(e?[abcd]x|[abcd][lh]|e?[sd]i|e?[sb]p)\b"
GAME_END = 0x238B0
NOSTACK = False  # set by --nostack: mask stack offsets (frame-size differences)


def _fold_address(text: str) -> str:
    # a relocated displacement may print as "+ A" or vanish (addend 0): fold both spellings together
    text = re.sub(r" [+-] A\]", "]", text)
    return re.sub(r"\[A\]", "[A]", text)


def norm(insn, masked: set[int] | None) -> str:
    """masked=None: oracle side (absolute image addresses become A); else candidate side (relocated fields)."""
    text = f"{insn.mnemonic} {insn.op_str}".strip()
    if insn.mnemonic.startswith("j") or insn.mnemonic == "call":
        text = re.sub(r"(?<![\w*])(0x[0-9a-f]+|\d+)(?![\w*])", "T", text)
        return re.sub(r" \+ T\]", "]", text).replace("[T]", "[]")
    if masked is None:
        text = re.sub(r"0x[0-9a-f]+", lambda m: "A" if 0x400000 <= int(m.group(0), 16) < 0x500000 else m.group(0), text)
    elif any(insn.address + k in masked for k in range(insn.size)):
        # only the relocated operand is unknown; keep register structure, drop numbers
        text = re.sub(r"(?<![\w*])(0x[0-9a-f]+|\d+)(?![\w*])", "A", text)
    text = _fold_address(text)
    if NOSTACK:
        text = re.sub(r"\[esp \+ 0x[0-9a-f]+\]", "[esp+S]", text)
        text = re.sub(r"\b(add|sub) esp, (0x[0-9a-f]+|\d+)", r"\1 esp, S", text)
    return text


def shape(text: str) -> str:
    return re.sub(REG, "R", text)


def noconst(text: str) -> str:
    return re.sub(r"0x[0-9a-f]+|\b\d+\b", "K", text)


def classify(a: list[str], b: list[str]) -> str:
    if not a:
        return "delete"
    if not b:
        return "insert"
    if len(a) == len(b):
        if [shape(x) for x in a] == [shape(y) for y in b]:
            return "register"
        if [noconst(x) for x in a] == [noconst(y) for y in b]:
            return "constant"
    if Counter(a) == Counter(b):
        return "order"
    if Counter(shape(x) for x in a) == Counter(shape(y) for y in b):
        return "order"  # reordered and re-registered
    return "mixed"


TABLE_USE = re.compile(r"^(jmp|mov|movzx|xor) .*(dword|byte) ptr \[")


def analyse(func, coff, target: bytes, target_va: int, oracle_size: int) -> dict:
    md = capstone.Cs(capstone.CS_ARCH_X86, capstone.CS_MODE_32)
    masked = set()
    for reloc in func.relocs:
        masked.update(range(reloc.offset - func.start, reloc.offset - func.start + 4))
    cand = list(md.disasm(func.data, 0))
    # switch tables follow the code in the same COMDAT: stop at the first table referenced from the function
    code_end = len(func.data)
    for reloc in func.relocs:
        sym = coff.sym_by_index.get(reloc.symbol_index)
        if sym is not None and sym.section == func.section.index and func.start <= sym.value < func.end:
            site = reloc.offset - func.start
            insn = next((i for i in cand if i.address <= site < i.address + i.size), None)
            if insn is not None and TABLE_USE.match(f"{insn.mnemonic} {insn.op_str}"):
                code_end = min(code_end, sym.value - func.start)
    cand = [i for i in cand if i.address < code_end]
    targ_end = min(len(target), oracle_size)
    for i in md.disasm(target, 0):
        if i.address >= targ_end:
            break
        if TABLE_USE.match(f"{i.mnemonic} {i.op_str}"):
            for m in re.finditer(r"0x[0-9a-f]+", i.op_str):
                off = int(m.group(0), 16) - target_va
                if i.address < off < targ_end:
                    targ_end = off
    targ = [i for i in md.disasm(target, 0) if i.address < targ_end]
    ca = [norm(i, masked) for i in cand]
    ta = [norm(i, None) for i in targ]
    sm = difflib.SequenceMatcher(a=ca, b=ta, autojunk=False)
    regions = []
    equal = 0
    for op, i1, i2, j1, j2 in sm.get_opcodes():
        if op == "equal":
            equal += i2 - i1
            continue
        regions.append({"cand_off": cand[i1].address if i1 < len(cand) else len(func.data),
                        "oracle_off": targ[j1].address if j1 < len(targ) else len(target),
                        "class": classify(ca[i1:i2], ta[j1:j2]),
                        "cand": ca[i1:i2][:12], "oracle": ta[j1:j2][:12],
                        "n_cand": i2 - i1, "n_oracle": j2 - j1})
    return {"cand_insns": len(cand), "oracle_insns": len(targ), "equal_insns": equal, "regions": regions}


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("--detail", help="print every region of the function at this oracle RVA")
    parser.add_argument("--nostack", action="store_true", help="mask [esp+N] offsets and frame size")
    args = parser.parse_args()
    global NOSTACK
    NOSTACK = args.nostack
    manifest = json.loads((ROOT / "recon.json").read_text(encoding="utf-8"))
    accepted = {int(a["rva"], 0) for a in manifest["accepted_functions"] if not a.get("withdrawn")}
    rows = [r for r in csv.DictReader(open(ROOT / "oracle" / "tu_map.csv", encoding="utf-8-sig"))
            if r["tu"] != "crt" and int(r["rva"], 16) < GAME_END]
    results = json.loads((ROOT / "build" / "verify" / "results.json").read_text(encoding="utf-8"))
    pe = pefile.PE(str(ROOT / "assets" / "Speedy Blupi" / "BLUPI.EXE"), fast_load=True)
    out = []
    for unit in results:
        obj = Path(unit["object"])
        tu = re.sub(r"^\d+_", "", obj.stem) + ".cpp"
        coff_funcs = {f.name: f for f in cmpfunc.Coff(obj).functions()}
        order = [f["name"] for f in unit["functions"]]
        located = {f["name"]: int(f["matches"][0]["rva"], 16) for f in unit["functions"]
                   if len(f["matches"]) == 1 and f["status"] in ("STRICT", "FOLDED")}
        for row in rows:
            rva = int(row["rva"], 16)
            if row["tu"] != tu or rva in accepted:
                continue
            before = max((r for r in located.values() if r < rva), default=None)
            after = min((r for r in located.values() if r > rva), default=None)
            lo = max((order.index(n) for n, r in located.items() if r == before), default=-1)
            hi = min((order.index(n) for n, r in located.items() if r == after), default=len(order))
            gap = [order[k] for k in range(lo + 1, hi) if unit["functions"][k]["status"] not in ("STRICT", "FOLDED")
                   and not unit["functions"][k]["name"].startswith("??")]
            gap_rvas = [int(r["rva"], 16) for r in rows if r["tu"] == tu and int(r["rva"], 16) not in accepted
                        and (before is None or int(r["rva"], 16) > before) and (after is None or int(r["rva"], 16) < after)]
            name = gap[gap_rvas.index(rva)] if len(gap) == len(gap_rvas) else None
            if name is None:
                size = int(row["size"])
                name = min(gap, key=lambda n: abs(len(coff_funcs[n].data) - size), default=None) if gap else None
            entry = {"tu": tu, "rva": f"{rva:#x}", "oracle_size": int(row["size"]), "name": name}
            if name and name in coff_funcs:
                func = coff_funcs[name]
                entry["cand_size"] = len(func.data)
                entry.update(analyse(func, cmpfunc.Coff(obj), pe.get_data(rva, max(len(func.data), int(row["size"]) + 1024)), pe.OPTIONAL_HEADER.ImageBase + rva, int(row["size"])))
                entry["oracle_insns_in_extent"] = None
            out.append(entry)
    (ROOT / "build" / "residues.json").write_text(json.dumps(out, indent=1), encoding="utf-8")
    print(f"{'tu':<13}{'rva':>9} {'name':<44}{'cand/oracle B':>15} {'eq/insn':>11}  regions by class")
    for e in sorted(out, key=lambda e: -e["oracle_size"]):
        if "regions" not in e:
            print(f"{e['tu']:<13}{e['rva']:>9} {str(e['name'])[:43]:<44} (no candidate paired)")
            continue
        cls = Counter(r["class"] for r in e["regions"])
        print(f"{e['tu']:<13}{e['rva']:>9} {e['name'][:43]:<44}{e['cand_size']:>7}/{e['oracle_size']:<7}"
              f"{e['equal_insns']:>5}/{e['oracle_insns']:<5}  " + " ".join(f"{k}:{v}" for k, v in cls.most_common()))
    if args.detail:
        e = next(e for e in out if int(e["rva"], 16) == int(args.detail, 0))
        for r in e.get("regions", []):
            print(f"\n[{r['class']}] cand +{r['cand_off']:#x} ({r['n_cand']}) / oracle +{r['oracle_off']:#x} ({r['n_oracle']})")
            for a, b in zip(r["cand"] + [""] * 12, r["oracle"] + [""] * 12):
                if a or b:
                    print(f"   {a:<46} {b}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
