#!/usr/bin/env python3
"""Cross-game function correspondence and a first TU map for a target with its
own inventory (docs/SB2.md).

    python tools/crossmap.py --target sb2/eggbert/en [--from sb1/blupi/us]

Every function of the source target's inventory (oracle/functions.csv with its
TU map) is compared with every function of the target's own inventory
(oracle/<slug>/functions.csv, tools/target_oracle.py), statically:

  exact      the whole body, byte for byte, with image addresses and call
             targets masked, at a target function start (unique)
  sequence   the same instruction sequence (mnemonics and instruction sizes),
             operands ignored (unique on both sides)

Matches are diagnostics, never strict proof: they place TUs and pick
candidate source. Writes:

  oracle/<slug>/tu_map.csv           rva,size,tu,confidence,best_source,status,evidence
                                     - "anchored": a unique exact/sequence match
                                       to a source function of that TU
                                     - "bracketed": between two anchors of the
                                       same TU, in address order
                                     - empty tu: not placed
  build/targets/<slug>/crossmap.csv  every pairing found, with the source symbol
                                     where the source target's manifest has one

    python tools/crossmap.py --target sb2/eggbert/en --audit

checks the target's accepted functions against the source target's, through
that cross-map (independent of the comparator; a conflict means a STRICT
placement names the wrong function):
  callers   every source-target caller of an accepted symbol that is paired
            in the target calls the target's address of that symbol
  calls     every call of a STRICT function in the last verify run
            (build/verify-<slug>/results.json) that lands on a paired
            function lands on the source target's function of that name, or
            on the body the target manifest's "source_correspondence" names
            (a function split or renamed between the games), or on a shared
            (folded) body that the run also locates under that name
"""
from __future__ import annotations

import argparse
import csv
import json
import re
from collections import Counter, defaultdict
from pathlib import Path

import pefile
from capstone import CS_ARCH_X86, CS_MODE_32, Cs

import targets
from derive_target import masked_regex, sections

ROOT = Path(__file__).resolve().parents[1]
BASE = 0x400000


def load_rows(path: Path) -> list[dict]:
    with path.open(newline="", encoding="utf-8-sig") as stream:
        return list(csv.DictReader(stream))


def bodies(pe, rows):
    text = sections(pe)[".text"]
    data = text.get_data()
    out = {}
    for r in rows:
        rva, size = int(r["rva"], 16), int(r["size"])
        out[rva] = data[rva - text.VirtualAddress:rva - text.VirtualAddress + size]
    return out


def shape(code: bytes, va: int) -> tuple:
    md = Cs(CS_ARCH_X86, CS_MODE_32)
    return tuple((i.mnemonic, i.size) for i in md.disasm(code, va))


def direct_calls(pe, rows) -> dict[int, set[int]]:
    """callee rva -> caller function rvas, from the E8 calls of each inventoried function."""
    md = Cs(CS_ARCH_X86, CS_MODE_32)
    code = bodies(pe, rows)
    out: dict[int, set[int]] = defaultdict(set)
    for r in rows:
        rva = int(r["rva"], 16)
        for ins in md.disasm(code[rva], rva):
            if ins.mnemonic == "call" and ins.bytes[0] == 0xE8:
                out[int(ins.op_str, 16)].add(rva)
    return out


def audit(dst, src_name, src_spec, candidates: bool = False, results: Path | None = None) -> int:
    work = ROOT / "build" / "targets" / dst.slug
    pairs = {int(r["src_rva"], 16): int(r["dst_rva"], 16) for r in load_rows(work / "crossmap.csv")}
    by_dst = {d: s for s, d in pairs.items()}
    src_manifest = targets.load(src_name).manifest()
    src_at = {a["symbol"]: int(str(a["rva"]), 0) for a in src_manifest.get("accepted_functions", [])
              if a.get("status") != "folded" and not a.get("withdrawn")}
    src_name_at = {v: k for k, v in src_at.items()}
    dst_manifest = dst.manifest()
    renamed = {item["symbol"]: item["source_symbol"] for item in dst_manifest.get("source_correspondence", [])}
    src_oracle = ROOT / src_spec.get("oracle", "oracle")
    c_src = direct_calls(pefile.PE(str(ROOT / src_spec["exe"])), load_rows(src_oracle / "functions.csv"))
    c_dst = direct_calls(pefile.PE(str(dst.exe)), load_rows(dst.oracle_dir / "functions.csv"))
    conflicts = consistent = 0
    checked = dst_manifest.get("accepted_functions", [])
    results = results or ROOT / "build" / f"verify-{dst.slug}" / "results.json"
    if candidates:
        # every STRICT function of the last verify run, accepted or not
        checked = [{"symbol": f["name"], "rva": f["matches"][0]["rva"]}
                   for obj in json.loads(results.read_text(encoding="utf-8")) for f in obj["functions"]
                   if f["status"] == "STRICT" and len(f["matches"]) == 1]
    for item in checked:
        if item.get("status") == "folded" or item.get("withdrawn") or item["symbol"] not in src_at:
            continue
        at = int(str(item["rva"]), 0)
        for caller in c_src.get(src_at[item["symbol"]], ()):
            if caller not in pairs:
                continue
            if pairs[caller] in c_dst.get(at, ()):
                consistent += 1
            else:
                conflicts += 1
                print(f"CONFLICT callers: {item['symbol']} at 0x{at:x}; {src_name} caller 0x{caller:x} "
                      f"is 0x{pairs[caller]:x} here and does not call it")
    print(f"callers: {consistent} consistent, {conflicts} conflicting"
          + (" (all STRICT functions of the last run)" if candidates else ""))
    if results.is_file():
        same = other = 0
        run = json.loads(results.read_text(encoding="utf-8"))
        # symbols this run locates at each address: a shared (folded) body answers to all of them
        at_address: dict[int, set[str]] = defaultdict(set)
        for obj in run:
            for func in obj["functions"]:
                if func["status"] in ("STRICT", "FOLDED") and len(func["matches"]) == 1:
                    at_address[int(func["matches"][0]["rva"], 16)].add(func["name"])
        for obj in run:
            for func in obj["functions"]:
                if func["status"] != "STRICT":
                    continue
                for rel in func["matches"][0].get("relocations", []):
                    target = rel.get("target_va") or rel.get("effective_target_va")
                    if not target or (int(target, 16) - BASE) not in by_dst:
                        continue
                    name = src_name_at.get(by_dst[int(target, 16) - BASE])
                    if name is None:
                        continue
                    if (name in (rel.get("symbol"), renamed.get(rel.get("symbol")))
                            or name in at_address.get(int(target, 16) - BASE, set())):
                        same += 1
                    else:
                        other += 1
                        print(f"CONFLICT calls: {func['name']} -> {rel.get('symbol')} at {target}, "
                              f"which is {name} in {src_name}")
        conflicts += other
        print(f"calls: {same} consistent, {other} conflicting ({results.relative_to(ROOT)})")
    else:
        print(f"calls: skipped, no {results.relative_to(ROOT)} (run tools/verify.py --target {dst.name})")
    return 1 if conflicts else 0


def main() -> int:
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    targets.add_argument(ap)
    ap.add_argument("--from", dest="source", default=None, help="source target (default: the primary)")
    ap.add_argument("--audit", action="store_true", help="check accepted functions against the cross-map")
    ap.add_argument("--results", type=Path, help="with --audit: a verify run's results.json "
                    "(default build/verify-<target>/results.json; e.g. build/verify-NAME/results.json for --build-dir NAME)")
    ap.add_argument("--candidates", action="store_true",
                    help="with --audit: check every STRICT function of the last verify run, not only accepted ones")
    args = ap.parse_args()
    reg = targets.registry()
    dst = targets.load(args.target)
    src_name = args.source or reg["default"]
    src_spec = reg["targets"][src_name]
    if args.audit:
        return audit(dst, src_name, src_spec, args.candidates, args.results and args.results.resolve())
    src_oracle = ROOT / src_spec.get("oracle", "oracle")
    src_pe, dst_pe = pefile.PE(str(ROOT / src_spec["exe"])), pefile.PE(str(dst.exe))
    src_rows = [r for r in load_rows(src_oracle / "functions.csv")]
    src_tu = {int(r["rva"], 16): r["tu"] for r in load_rows(src_oracle / "tu_map.csv")}
    dst_rows = load_rows(dst.oracle_dir / "functions.csv")
    dst_game = [r for r in dst_rows if r["kind"] == "game"]
    manifest = targets.load(src_name).manifest()
    src_symbol = {int(str(a["rva"]), 0): a["symbol"] for a in manifest.get("accepted_functions", [])
                  if a.get("status", "strict") != "folded"}

    s_code, d_code = bodies(src_pe, src_rows), bodies(dst_pe, dst_rows)
    d_text = sections(dst_pe)[".text"]
    d_data = d_text.get_data()[:d_text.Misc_VirtualSize]
    d_starts = {int(r["rva"], 16) for r in dst_rows}

    pairs: dict[int, list[tuple[int, str]]] = defaultdict(list)   # dst rva -> [(src rva, kind)]
    for r in src_rows:
        rva = int(r["rva"], 16)
        tu = src_tu.get(rva, "")
        if not tu or tu == "crt" or int(r["size"]) < 8:
            continue
        rx = masked_regex(s_code[rva], BASE + rva)
        if rx is None:
            continue
        hits = [m.start() + d_text.VirtualAddress for m in rx.finditer(d_data)]
        hits = [h for h in hits if h in d_starts]
        if len(hits) == 1:
            pairs[hits[0]].append((rva, "exact"))
    exact_src = {s for v in pairs.values() for s, _ in v}
    # instruction-sequence matches, unique on both sides
    s_shapes, d_shapes = defaultdict(list), defaultdict(list)
    for r in src_rows:
        rva = int(r["rva"], 16)
        if src_tu.get(rva, "") in ("", "crt") or int(r["size"]) < 16:
            continue
        s_shapes[shape(s_code[rva], BASE + rva)].append(rva)
    for r in dst_game:
        rva = int(r["rva"], 16)
        d_shapes[shape(d_code[rva], BASE + rva)].append(rva)
    for key, srcs in s_shapes.items():
        dsts = d_shapes.get(key, [])
        if len(srcs) == 1 and len(dsts) == 1 and srcs[0] not in exact_src and not pairs.get(dsts[0]):
            pairs[dsts[0]].append((srcs[0], "sequence"))

    # TU map: anchors, then brackets between anchors of one TU
    anchor: dict[int, tuple[str, str, int]] = {}
    for d, items in pairs.items():
        tus = {src_tu[s] for s, _ in items}
        if len(tus) == 1:
            s, kind = items[0]
            anchor[d] = (tus.pop(), kind, s)
    game_rvas = sorted(int(r["rva"], 16) for r in dst_game)
    dst_manifest = dst.manifest()
    accepted = {int(str(a["rva"]), 0): ("folded" if a.get("status") == "folded" else "strict")
                for a in dst_manifest.get("accepted_functions", []) if not a.get("withdrawn")}
    size = {int(r["rva"], 16): int(r["size"]) for r in dst_rows}
    out_rows = []
    anchored = sorted(anchor)
    for rva in game_rvas:
        if rva in anchor:
            tu, kind, s = anchor[rva]
            sym = src_symbol.get(s, "")
            ev = f"[EVIDENCE] {kind} match to {src_name} 0x{s:x}" + (f" ({sym})" if sym else "")
            out_rows.append((rva, size[rva], tu, "anchored", f"{src_name}:{tu}:{sym or hex(s)}",
                             accepted.get(rva, "none"), ev))
            continue
        before = [a for a in anchored if a < rva]
        after = [a for a in anchored if a > rva]
        if before and after and anchor[before[-1]][0] == anchor[after[0]][0]:
            tu = anchor[before[-1]][0]
            out_rows.append((rva, size[rva], tu, "bracketed", f"{src_name}:{tu}", accepted.get(rva, "none"),
                             f"[EVIDENCE] between {tu} anchors 0x{before[-1]:x} and 0x{after[0]:x}"))
        else:
            out_rows.append((rva, size[rva], "", "unplaced", "", accepted.get(rva, "none"), ""))
    for r in dst_rows:
        rva = int(r["rva"], 16)
        if r["kind"] != "game" and rva >= dst.game_text_end:
            out_rows.append((rva, size[rva], "crt", "inventory", "", "none",
                             "[EVIDENCE] after the import thunks that end the game objects"))
    with (dst.oracle_dir / "tu_map.csv").open("w", newline="", encoding="utf-8") as stream:
        w = csv.writer(stream, lineterminator="\n")
        w.writerow(("rva", "size", "tu", "confidence", "best_source", "status", "evidence"))
        for row in sorted(out_rows):
            w.writerow((f"0x{row[0]:x}",) + row[1:])
    work = ROOT / "build" / "targets" / dst.slug
    work.mkdir(parents=True, exist_ok=True)
    with (work / "crossmap.csv").open("w", newline="", encoding="utf-8") as stream:
        w = csv.writer(stream, lineterminator="\n")
        w.writerow(("dst_rva", "dst_size", "src_rva", "src_size", "kind", "src_tu", "src_symbol"))
        ssize = {int(r["rva"], 16): int(r["size"]) for r in src_rows}
        for d in sorted(pairs):
            for s, kind in pairs[d]:
                w.writerow((f"0x{d:x}", size[d], f"0x{s:x}", ssize[s], kind, src_tu[s], src_symbol.get(s, "")))

    # report: per source TU
    per_tu = defaultdict(Counter)
    for d, (tu, kind, _s) in anchor.items():
        per_tu[tu][kind] += 1
        per_tu[tu][kind + "_bytes"] += size[d]
    src_per_tu = Counter(src_tu.get(int(r["rva"], 16), "") for r in src_rows)
    print(f"{'source TU':<14} {'src fns':>7} {'exact':>6} {'bytes':>7} {'seq':>5} {'bytes':>7}")
    for tu in sorted(per_tu, key=lambda t: -per_tu[t]["exact_bytes"] - per_tu[t]["sequence_bytes"]):
        c = per_tu[tu]
        print(f"{tu:<14} {src_per_tu[tu]:>7} {c['exact']:>6} {c['exact_bytes']:>7} {c['sequence']:>5} "
              f"{c['sequence_bytes']:>7}")
    conf = Counter(r[3] for r in out_rows if r[2] != "crt")
    placed = sum(r[1] for r in out_rows if r[3] in ("anchored", "bracketed"))
    total = sum(size[r] for r in game_rvas)
    print(f"{dst.name} game functions: {len(game_rvas)}; anchored {conf['anchored']}, bracketed "
          f"{conf['bracketed']}, unplaced {conf['unplaced']}; placed {placed}/{total} body bytes")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
