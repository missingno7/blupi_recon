"""Derives a target's oracle inventory and address map from another target of
the same game (docs/MULTI_GAME.md).

    python tools/derive_target.py --target sb1/eggbert/en [--from sb1/blupi/us]

For builds of the same source with a different layout (another linker or
edition), each function of the source target's inventory (oracle/functions.csv)
is searched in the target's .text, byte for byte with image addresses and call
targets masked. Unique hits are kept when they keep the inventory's order;
functions between two hits with the same shift take that shift. Writes, under
oracle/<target>/:

  functions.csv, tu_map.csv   the inventory at the target's addresses
                              (functions not located are left out)
  address_map.json            code and data segments: source VA range -> shift,
                              used to translate the manifest's addresses
                              (tools/targets.py); the data shifts come from
                              aligning the initialised .rdata and .data bytes.

Static reads of both executables only.
"""
from __future__ import annotations

import argparse
import csv
import json
import re
from pathlib import Path

import pefile
from capstone import CS_ARCH_X86, CS_MODE_32, Cs

import targets

ROOT = Path(__file__).resolve().parents[1]
BASE = 0x400000


def sections(pe):
    return {s.Name.rstrip(b"\0").decode(): s for s in pe.sections}


def masked_regex(code: bytes, va: int) -> re.Pattern:
    md = Cs(CS_ARCH_X86, CS_MODE_32)
    parts, wild, pos = [], 0, 0
    for ins in md.disasm(code, va):
        b = bytes(ins.bytes)
        mask = [False] * len(b)
        for i in range(len(b) - 3):
            if BASE <= int.from_bytes(b[i:i + 4], "little") < BASE + 0x100000:
                mask[i:i + 4] = [True] * 4
        if b[0] in (0xE8, 0xE9) and len(b) == 5:
            mask[1:5] = [True] * 4
        for i, c in enumerate(b):
            if mask[i]:
                wild += 1
                continue
            if wild:
                parts.append(b".{%d}" % wild)
                wild = 0
            parts.append(re.escape(bytes([c])))
        pos += len(b)
    if wild:
        parts.append(b".{%d}" % wild)
    if pos < len(code) * 0.9:
        return None
    return re.compile(b"".join(parts), re.S)


def locate_functions(src_pe, dst_pe, rows):
    s_text, d_text = sections(src_pe)[".text"], sections(dst_pe)[".text"]
    s_data = s_text.get_data()
    d_data = d_text.get_data()[:d_text.Misc_VirtualSize]
    hits = {}
    for row in rows:
        rva, size = int(row["rva"], 16), int(row["size"])
        code = s_data[rva - s_text.VirtualAddress:rva - s_text.VirtualAddress + size]
        rx = masked_regex(code, BASE + rva) if size >= 8 else None
        if rx is None:
            continue
        found = [m.start() for m in rx.finditer(d_data)]
        if len(found) == 1:
            hits[rva] = found[0] + d_text.VirtualAddress
    # keep the hits that preserve the inventory's order (longest increasing run)
    order = sorted(hits)
    best, prev = [], {}
    tails = []
    for rva in order:
        target = hits[rva]
        lo, hi = 0, len(tails)
        while lo < hi:
            mid = (lo + hi) // 2
            if hits[tails[mid]] < target:
                lo = mid + 1
            else:
                hi = mid
        prev[rva] = tails[lo - 1] if lo else None
        if lo == len(tails):
            tails.append(rva)
        else:
            tails[lo] = rva
    keep = set()
    node = tails[-1] if tails else None
    while node is not None:
        keep.add(node)
        node = prev[node]
    mapped = {rva: hits[rva] for rva in keep}
    # Fill the gaps between kept hits, in order:
    # - between two hits with the same shift, a function takes that shift;
    # - otherwise (a function whose size changed, or a body shared by several
    #   functions) it starts at its predecessor's shift, when that address
    #   follows padding or a return and lies before the next hit.
    d_base = d_text.VirtualAddress
    all_rvas = sorted(int(r["rva"], 16) for r in rows)
    for rva in all_rvas:
        if rva in mapped:
            continue
        before = [k for k in mapped if k < rva]
        after = [k for k in mapped if k > rva]
        if not before or not after:
            continue
        prev, nxt = max(before), min(after)
        d1, d2 = mapped[prev] - prev, mapped[nxt] - nxt
        if d1 == d2:
            mapped[rva] = rva + d1
            continue
        start = rva + d1
        if not (mapped[prev] < start < mapped[nxt]):
            continue
        before_byte = d_data[start - d_base - 1] if start - d_base >= 1 else 0
        if before_byte in (0xCC, 0x90, 0xC3) or d_data[start - d_base - 3] == 0xC2:
            mapped[rva] = start
    return mapped


def segments(pairs, gap_ok=0x1000):
    """[(src, dst)] sorted -> [[start, end, shift]] runs of equal shift."""
    out = []
    for src, dst in sorted(pairs):
        shift = dst - src
        if out and out[-1][2] == shift and src - out[-1][1] <= gap_ok:
            out[-1][1] = src + 1
        else:
            out.append([src, src + 1, shift])
    return out


def align_data(src_pe, dst_pe):
    pairs = []
    for name in (".rdata", ".data"):
        s, d = sections(src_pe).get(name), sections(dst_pe).get(name)
        if s is None or d is None:
            continue
        sb, db = s.get_data()[:s.SizeOfRawData], d.get_data()[:d.SizeOfRawData]
        index = {}
        width = 24
        for off in range(0, len(db) - width, 4):
            chunk = db[off:off + width]
            if chunk.count(0) > width // 2:
                continue
            index.setdefault(chunk, []).append(off)
        for off in range(0, len(sb) - width, 8):
            chunk = sb[off:off + width]
            where = index.get(chunk)
            if where and len(where) == 1:
                pairs.append((BASE + s.VirtualAddress + off, BASE + d.VirtualAddress + where[0]))
        # the uninitialised tail of .data follows the last aligned shift
        if pairs:
            last_src, last_dst = max(p for p in pairs if p[0] < BASE + s.VirtualAddress + s.Misc_VirtualSize)
            end = BASE + s.VirtualAddress + s.Misc_VirtualSize
            pairs.append((end - 1, end - 1 + (last_dst - last_src)))
    runs = segments(pairs, gap_ok=0x4000)
    # drop isolated one-pair runs (chance matches), then merge equal neighbours
    runs = [r for r in runs if r[1] - r[0] > 64 or len(runs) == 1]
    merged = []
    for r in runs:
        if merged and merged[-1][2] == r[2]:
            merged[-1][1] = r[1]
        else:
            merged.append(r)
    return merged


def main() -> int:
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("--target", required=True)
    ap.add_argument("--from", dest="source", default=None, help="source target (default: the primary)")
    args = ap.parse_args()
    reg = targets.registry()
    src_name = args.source or reg["default"]
    src_spec, dst_spec = reg["targets"][src_name], reg["targets"][args.target]
    src_pe, dst_pe = pefile.PE(str(ROOT / src_spec["exe"])), pefile.PE(str(ROOT / dst_spec["exe"]))
    src_oracle = ROOT / src_spec.get("oracle", "oracle")
    rows = list(csv.DictReader(open(src_oracle / "functions.csv", encoding="utf-8-sig")))
    tu_rows = list(csv.DictReader(open(src_oracle / "tu_map.csv", encoding="utf-8-sig")))

    mapped = locate_functions(src_pe, dst_pe, rows)
    out = ROOT / "oracle" / args.target.replace("/", "-")
    out.mkdir(parents=True, exist_ok=True)

    def write(name, source_rows):
        fields = list(source_rows[0].keys())
        with open(out / name, "w", newline="", encoding="utf-8") as f:
            w = csv.DictWriter(f, fieldnames=fields)
            w.writeheader()
            for r in source_rows:
                rva = int(r["rva"], 16)
                if rva not in mapped:
                    continue
                r = dict(r)
                r["rva"] = "0x%x" % mapped[rva]
                if "end" in r:
                    r["end"] = "0x%x" % (mapped[rva] + int(r["size"]))
                w.writerow(r)

    write("functions.csv", rows)
    write("tu_map.csv", tu_rows)
    code = segments([(BASE + s, BASE + d) for s, d in mapped.items()])
    data = align_data(src_pe, dst_pe)
    amap = {"from": src_name, "to": args.target,
            "code": [["0x%x" % a, "0x%x" % b, s] for a, b, s in code],
            "data": [["0x%x" % a, "0x%x" % b, s] for a, b, s in data]}
    (out / "address_map.json").write_text(json.dumps(amap, indent=1) + "\n", encoding="utf-8")
    game = [r for r in rows if r["kind"] == "game"]
    print(f"{args.target}: {sum(int(r['rva'], 16) in mapped for r in game)}/{len(game)} game functions, "
          f"{len(mapped)}/{len(rows)} inventory entries located; {len(code)} code and {len(data)} data segments")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
