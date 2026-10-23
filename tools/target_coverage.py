#!/usr/bin/env python3
"""Accepted coverage of a target, in unique game .text bytes (docs/SB2.md).

    python tools/target_coverage.py --target sb2/eggbert/en

Reads the target's manifest only (no build): the union of the accepted
functions' [rva, rva+size) intervals inside the game range, once per byte
(FOLDED bodies count once, under their owner's address), split by the
manifest's "coverage_groups" (group name -> TU file names). This is the
progress measure; a verify run's total also counts STRICT bodies that are not
accepted (excluded or unproven).
"""
from __future__ import annotations

import argparse
from pathlib import Path

import targets

ROOT = Path(__file__).resolve().parents[1]


def union(intervals: list[tuple[int, int]]) -> int:
    total, end = 0, -1
    for lo, hi in sorted(intervals):
        if hi <= end:
            continue
        total += hi - max(lo, end)
        end = hi
    return total


def main() -> int:
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    targets.add_argument(ap)
    args = ap.parse_args()
    target = targets.load(args.target)
    manifest = target.manifest()
    start, end = 0x1000, target.game_text_end
    groups = manifest.get("coverage_groups", {})
    group_of = {tu: name for name, tus in groups.items() for tu in tus}
    per: dict[str, list[tuple[int, int]]] = {}
    counts: dict[str, list[int]] = {}
    for item in manifest.get("accepted_functions", []):
        if item.get("withdrawn"):
            continue
        group = group_of.get(Path(item["tu"]).name, "other")
        folded = item.get("status") == "folded"
        counts.setdefault(group, [0, 0])[1 if folded else 0] += 1
        rva = int(str(item["into"]["rva"] if folded else item["rva"]), 0)
        lo, hi = max(rva, start), min(rva + int(item["size"]), end)
        if lo < hi:
            per.setdefault(group, []).append((lo, hi))
    print(f"{target.name}: game .text 0x{start:x}-0x{end:x} ({end - start:,} bytes)")
    for name in [*groups, *sorted(set(per) - set(groups))]:
        strict, folded = counts.get(name, [0, 0])
        print(f"  {name:<12} {strict:>4} STRICT {folded:>3} FOLDED  {union(per.get(name, [])):>7,} bytes")
    total = union([iv for ivs in per.values() for iv in ivs])
    strict = sum(c[0] for c in counts.values())
    folded = sum(c[1] for c in counts.values())
    print(f"  {'total':<12} {strict:>4} STRICT {folded:>3} FOLDED  {total:>7,} bytes "
          f"({100 * total / (end - start):.1f}%)")
    equivalent = manifest.get('target_equivalent_functions', {}).get(target.name, [])
    if equivalent:
        intervals = [(max(int(str(e['rva']), 0), start), min(int(str(e['rva']), 0)+e['size'], end))
                     for e in equivalent]
        print(f"  EQUIVALENT   {len(equivalent):>4} functions       {union(intervals):>7,} bytes "
              '(declared certificates; rechecked by verify; excluded from STRICT)')
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
