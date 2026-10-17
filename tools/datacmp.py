#!/usr/bin/env python3
"""Compare initialized COFF data (or mapped linked data) with the oracle PE.

Coverage is a union of fully verified oracle byte intervals. Ambiguous payload
hits, uninitialized storage, masked bytes and unresolved pointers add no coverage.
"""
from __future__ import annotations

import argparse
from collections import Counter, defaultdict
import csv
import json
from pathlib import Path
import re
import struct

import pefile
import cmpfunc as cmp
from relink import read_map

ROOT = Path(__file__).resolve().parents[1]


def initialized_sections(pe):
    return {s.Name.rstrip(b"\0").decode(): s for s in pe.sections
            if s.Name.rstrip(b"\0") in (b".data", b".rdata")}


def initialized_bytes(pe, va, size):
    """Never fill a virtual/BSS tail with zeros and call it initialized data."""
    for name, section in initialized_sections(pe).items():
        start = pe.OPTIONAL_HEADER.ImageBase + section.VirtualAddress
        length = min(section.Misc_VirtualSize, section.SizeOfRawData)
        if start <= va and va + size <= start + length:
            return name, pe.get_data(va - pe.OPTIONAL_HEADER.ImageBase, size)
    return None, None


def union_size(intervals):
    end, total = -1, 0
    for start, stop in sorted(intervals):
        total += max(0, stop - max(start, end))
        end = max(end, stop)
    return total


def find_payload(pe, data, kind, alignment):
    hits = []
    if not data or not any(data):
        return hits
    section = initialized_sections(pe).get("." + kind)
    if section is None:
        return hits
    raw = section.get_data()[:min(section.SizeOfRawData, section.Misc_VirtualSize)]
    offset = 0
    while len(hits) < 16:
        offset = raw.find(data, offset)
        if offset < 0:
            break
        if offset % alignment == 0:
            hits.append(pe.OPTIONAL_HEADER.ImageBase + section.VirtualAddress + offset)
        offset += 1
    return hits


def code_bindings(objects, reports, pe):
    """Decode references only from independently placed, fixed-byte-equal code.

    This also locates data with wrong contents: a data mismatch is not allowed
    to erase the independent code-reference evidence locating that definition.
    """
    values, proofs = defaultdict(set), defaultdict(set)
    reports = {str(Path(r["object"]).resolve()): r for r in reports}
    text = next(s for s in pe.sections if s.Name.rstrip(b"\0") == b".text")
    raw = text.get_data()
    for path, coff in objects:
        report = reports.get(str(path.resolve()))
        if report is None:
            continue
        funcs = {f.name: f for f in coff.functions()}
        for item in report["functions"]:
            if len(item["matches"]) != 1 or item["placement_basis"] in ("ambiguous", "unlocated"):
                continue
            func = funcs.get(item["name"])
            if func is None:
                continue
            rva = int(item["matches"][0]["rva"], 0)
            target = pe.get_data(rva, len(func.data))
            if len(target) != len(func.data) or any(func.data[a:b] != target[a:b] for a, b in cmp.fixed_runs(func)):
                continue
            key = cmp.symbol_map_key(path, func.symbol)
            values[key].add(pe.OPTIONAL_HEADER.ImageBase + rva)
            proofs[key].add("placed fixed-byte-equal function")
            for reloc in func.relocs:
                symbol = coff.sym_by_index.get(reloc.symbol_index)
                decoded = cmp.relocation_targets(reloc, func.section, rva-text.VirtualAddress,
                                                 raw, text.VirtualAddress, pe.OPTIONAL_HEADER.ImageBase, func.start)
                if decoded is not None and symbol is not None:
                    key = cmp.symbol_map_key(path, symbol)
                    values[key].add(decoded[0])
                    proofs[key].add(f"fixed-code reference: {path.name}:{func.name}@{rva:#x}+{reloc.offset-func.start:#x}")
    return values, proofs


def numeric_arrays(source_dirs):
    """Optional precise sizes for ordinary, flat numeric source initializers.

    A size is used only when its complete packed initializer equals the COFF
    prefix. Never infer a size from a bare oracle address or the next table.
    """
    files = {}
    for directory in source_dirs:
        for path in directory.iterdir():
            if path.suffix in (".h", ".cpp"):
                files[path.name] = path
    result = {}
    pattern = re.compile(r"^(?:extern\s+)?(unsigned\s+)?(char|short|int)\s+(\w+)\s*\[\s*(\d*)\s*\]\s*=\s*\{([^{}]*)\}", re.M)
    for path in files.values():
        text = re.sub(r"/\*.*?\*/|//[^\n]*", "", path.read_text(encoding="utf-8", errors="replace"), flags=re.S)
        for unsigned, typ, name, count, body in pattern.findall(text):
            tokens = [t.strip() for t in body.strip().rstrip(",").split(",")]
            if not tokens or any(not re.fullmatch(r"-?(?:0[xX][0-9a-fA-F]+|\d+)", t) for t in tokens):
                continue
            values = [int(t, 16) if "x" in t.lower() else int(t, 10) for t in tokens]
            if count:
                if len(values) > int(count):
                    continue
                values += [0]*(int(count)-len(values))
            width = {"char": 1, "short": 2, "int": 4}[typ]
            payload = b"".join((v & ((1 << (8*width))-1)).to_bytes(width, "little") for v in values)
            result[name] = (payload, str(path))
    return result


def data_symbols(coff, arrays):
    for symbol in coff.symbols:
        if symbol.section <= 0 or symbol.is_function or symbol.storage not in (2, 3):
            continue
        section = coff.sections[symbol.section-1]
        if cmp.section_kind(section.name) not in ("data", "rdata") or not section.raw:
            continue
        if symbol.name.startswith(".") or symbol.value >= len(section.raw):
            continue
        extent = cmp.data_extent(coff, symbol, 0)
        if extent and extent[1]:
            start, payload = extent
            name = re.fullmatch(r"\?(\w+)@@3.*", symbol.name)
            declared = arrays.get(name[1]) if name else None
            precise = bool(declared and payload.startswith(declared[0]))
            yield symbol, section, (start, declared[0] if precise else payload), precise


def compare(objects, reports, pe, manifest, image=None, map_symbols=(), arrays=None):
    values, proofs = code_bindings(objects, reports, pe)
    definitions = [(path, coff, symbol, section, start, data, precise)
                   for path, coff in objects for symbol, section, (start, data), precise in data_symbols(coff, arrays or {})]
    # Explicit mappings locate data; only a subsequent whole-payload comparison
    # can contribute coverage. They never certify the initializer by themselves.
    for mapping in manifest.get("symbol_mappings", []):
        key = "global:" + mapping["symbol"]
        values[key].add(int(mapping["va"], 0))
        proofs[key].add("manifest address evidence: " + mapping.get("evidence", ""))
    for path, coff, symbol, section, start, data, precise in definitions:
        key = cmp.symbol_map_key(path, symbol)
        if key not in values and not any(start <= r.offset < start+len(data) for r in section.relocations):
            alignment = 1 if symbol.name.startswith("??_C@") else 2 if symbol.name.endswith("PAFA") else 4
            hits = find_payload(pe, data, cmp.section_kind(section.name), alignment)
            if len(hits) == 1:
                values[key].add(hits[0])
                proofs[key].add("unique whole initialized-payload hit at natural alignment")
    # Never infer a pointer target from the very data field being compared:
    # that would simply recreate masked equality. Pointer targets below require
    # independent code, manifest, or unique full-payload location evidence.
    linked = defaultdict(list)
    for entry in map_symbols:
        linked[entry["name"]].append(entry)
    rows = []
    for path, coff, symbol, section, start, data, precise in definitions:
        key = cmp.symbol_map_key(path, symbol)
        candidates = sorted(values.get(key, ()))
        row = dict(object=str(path), symbol=symbol.name, object_section=section.name, object_offset=start,
                   size=len(data), extent="source numeric initializer (COFF prefix checked)" if precise else
                   "COFF symbol span; count-prefix/string terminator trimmed where known; may include padding",
                   oracle_va=None, oracle_rva=None, linked_rva=None, status="UNLOCATED", byte_exact=False,
                   location_evidence=sorted(proofs.get(key, ())), candidate_vas=[hex(v) for v in candidates])
        relocs = [r for r in section.relocations if start <= r.offset < start+len(data)]
        if len(candidates) > 1:
            row["status"] = "CONFLICT"
        elif not candidates:
            alignment = 1 if symbol.name.startswith("??_C@") else 2 if symbol.name.endswith("PAFA") else 4
            hits = find_payload(pe, data, cmp.section_kind(section.name), alignment) if not relocs else []
            row["candidate_vas"] = [hex(v) for v in hits]
            row["status"] = "AMBIGUOUS" if hits else "UNLOCATED"
        else:
            va = candidates[0]
            row.update(oracle_va=va, oracle_rva=va-pe.OPTIONAL_HEADER.ImageBase)
            oracle_section, target = initialized_bytes(pe, va, len(data))
            row["oracle_section"] = oracle_section
            expected = bytearray(data)
            unresolved = []
            for reloc in relocs:
                ref = coff.sym_by_index.get(reloc.symbol_index)
                choices = values.get(cmp.symbol_map_key(path, ref), ()) if ref else ()
                offset = reloc.offset-start
                if len(choices) != 1 or reloc.typ not in (6, 7, 0x14) or offset+4 > len(data):
                    unresolved.append(dict(offset=offset, symbol=ref.name if ref else None, type=reloc.typ))
                    continue
                addend = struct.unpack_from("<i", data, offset)[0]
                value = next(iter(choices))+addend
                if reloc.typ == 7:
                    value -= pe.OPTIONAL_HEADER.ImageBase
                elif reloc.typ == 0x14:
                    value -= va+offset+4
                struct.pack_into("<I", expected, offset, value & 0xffffffff)
            row["unresolved_relocations"] = unresolved
            same_kind = oracle_section == "." + cmp.section_kind(section.name)
            if target is None or not same_kind:
                row["status"] = "OUTSIDE-INITIALIZED-SECTION"
            elif unresolved:
                row["status"] = "RELOC-UNVERIFIED"
            else:
                row["byte_exact"] = bytes(expected) == target
                row["status"] = "EXACT" if row["byte_exact"] else "MISMATCH"
                diffs = [i for i, (a, b) in enumerate(zip(expected, target)) if a != b]
                row["differing_bytes"] = len(diffs)
                row["first_difference"] = diffs[0] if diffs else None
            if image is not None:
                options = linked.get(symbol.name, [])
                # Map static names can repeat. Match the owning object basename.
                owned = [x for x in options if path.stem.lower() in x["owner"].lower()]
                options = owned or options
                if len(options) == 1:
                    linked_va = options[0]["va"]
                    row["linked_rva"] = linked_va-image.OPTIONAL_HEADER.ImageBase
                    row["rva_delta"] = row["linked_rva"]-row["oracle_rva"]
                    _, payload = initialized_bytes(image, linked_va, len(data))
                    row["linked_byte_exact"] = payload is not None and payload == target
        rows.append(row)
    defined_names = {s.name for _, _, s, _, _, _, _ in definitions}
    for mapping in manifest.get("symbol_mappings", []):
        if mapping["symbol"] not in defined_names:
            va = int(mapping["va"], 0)
            if initialized_bytes(pe, va, 1)[0] is not None:
                rows.append(dict(object=None, symbol=mapping["symbol"], size=0, oracle_va=va,
                                 oracle_rva=va-pe.OPTIONAL_HEADER.ImageBase, status="MISSING-DEFINITION",
                                 byte_exact=False, location_evidence=[mapping.get("evidence", "")]))
    return rows


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--objects-dir", type=Path, default=ROOT / "build/relink/objects")
    parser.add_argument("--comparison", type=Path, help="cmpfunc.py JSON for precisely these objects")
    parser.add_argument("--oracle", type=Path, default=cmp.DEFAULT_EXE)
    parser.add_argument("--manifest", type=Path, default=ROOT / "recon.json")
    parser.add_argument("--source-dir", action="append", type=Path, default=[],
                        help="private source/header directory overriding src/ for precise numeric array extents")
    parser.add_argument("--image", type=Path, help="optional relinked PE (never executed)")
    parser.add_argument("--map", type=Path, help="VC5 map for --image; needed for linked symbol addresses")
    parser.add_argument("--json-out", type=Path, default=ROOT / "build/relink/data.json")
    parser.add_argument("--csv-out", type=Path, help="optional per-symbol CSV")
    args = parser.parse_args()
    paths = sorted(p.resolve() for p in args.objects_dir.glob("*.obj") if not p.name.startswith("crt_evidence_"))
    if not paths:
        parser.error("no game objects in --objects-dir")
    if bool(args.image) != bool(args.map):
        parser.error("--image and --map must be used together")
    objects = [(p, cmp.Coff(p)) for p in paths]
    reports = json.loads(args.comparison.read_text(encoding="utf-8")) if args.comparison else []
    # Require the same object paths; code_bindings additionally rechecks current
    # fixed bytes before reusing a report's function location.
    if reports and {str(Path(r["object"]).resolve()) for r in reports} != set(map(str, paths)):
        parser.error("--comparison object paths differ from --objects-dir")
    manifest = json.loads(args.manifest.read_text(encoding="utf-8"))
    pe = pefile.PE(str(args.oracle))
    image = pefile.PE(str(args.image)) if args.image else None
    rows = compare(objects, reports, pe, manifest, image, read_map(args.map) if args.map else (),
                   numeric_arrays([ROOT / "src"] + args.source_dir))
    summary = {}
    for name, section in initialized_sections(pe).items():
        size = min(section.SizeOfRawData, section.Misc_VirtualSize)
        intervals = [(r["oracle_rva"], r["oracle_rva"]+r["size"]) for r in rows
                     if r["status"] == "EXACT" and r.get("oracle_section") == name]
        linked_intervals = [(r["oracle_rva"], r["oracle_rva"]+r["size"]) for r in rows
                            if r.get("linked_byte_exact") and r.get("oracle_section") == name]
        summary[name] = dict(covered_bytes=union_size(intervals), initialized_bytes=size,
                             coverage_percent=100*union_size(intervals)/size,
                             linked_exact_bytes=union_size(linked_intervals) if image else None)
    covered = sum(s["covered_bytes"] for s in summary.values())
    total = sum(s["initialized_bytes"] for s in summary.values())
    result = dict(oracle=str(args.oracle), image=str(args.image) if args.image else None,
                  note="EXACT means complete initialized span and every pointer checked; not masked equality. "
                       "Coverage includes game objects only, excludes CRT/import data not represented by these objects, "
                       "raw file padding and virtual zero-fill. Repeated/overlapping bytes count once.",
                  sections=summary, covered_bytes=covered, initialized_bytes=total,
                  coverage_percent=100*covered/total, counts=dict(Counter(r["status"] for r in rows)), symbols=rows)
    args.json_out.parent.mkdir(parents=True, exist_ok=True)
    args.json_out.write_text(json.dumps(result, indent=2) + "\n", encoding="utf-8")
    if args.csv_out:
        args.csv_out.parent.mkdir(parents=True, exist_ok=True)
        fields = ["object", "symbol", "object_section", "oracle_va", "oracle_rva", "linked_rva", "size",
                  "status", "byte_exact", "linked_byte_exact", "differing_bytes", "first_difference"]
        with args.csv_out.open("w", newline="", encoding="utf-8") as stream:
            writer = csv.DictWriter(stream, fieldnames=fields, extrasaction="ignore")
            writer.writeheader()
            writer.writerows(rows)
    print(json.dumps({k: v for k, v in result.items() if k != "symbols"}, indent=2))
    print(f"per-symbol report: {args.json_out}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
