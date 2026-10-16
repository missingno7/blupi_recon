#!/usr/bin/env python3
"""Locate and compare MSVC COFF functions against the oracle PE image."""

from __future__ import annotations

import argparse
from collections import Counter, defaultdict
from dataclasses import dataclass, field
import json
from pathlib import Path
import re
import struct
from typing import Any

import capstone
import pefile


ROOT = Path(__file__).resolve().parents[1]
DEFAULT_EXE = ROOT / "assets" / "Speedy Blupi" / "BLUPI.EXE"
REL_SIZES = {0x0001: 2, 0x0002: 2, 0x0006: 4, 0x0007: 4, 0x000A: 2, 0x000B: 4, 0x000C: 4, 0x000D: 1, 0x0014: 4}
REL_NAMES = {0x0000: "ABSOLUTE", 0x0001: "DIR16", 0x0002: "REL16", 0x0006: "DIR32", 0x0007: "DIR32NB",
             0x000A: "SECTION", 0x000B: "SECREL", 0x000C: "TOKEN", 0x000D: "SECREL7", 0x0014: "REL32"}


def cstr(raw: bytes) -> str:
    return raw.split(b"\0", 1)[0].decode("cp1252", "replace")


@dataclass
class Symbol:
    index: int
    name: str
    value: int
    section: int
    typ: int
    storage: int
    aux: list[bytes] = field(default_factory=list)

    @property
    def is_function(self) -> bool:
        return (self.typ & 0x30) == 0x20

    @property
    def key(self) -> str:
        return f"{self.name}|sec={self.section}|val={self.value}|class={self.storage}"


@dataclass
class Relocation:
    section: int
    offset: int
    symbol_index: int
    typ: int


@dataclass
class Section:
    index: int
    name: str
    raw: bytes
    raw_offset: int
    raw_size: int
    relocations: list[Relocation]
    characteristics: int
    aux_length: int = 0


@dataclass
class Function:
    symbol: Symbol
    section: Section
    start: int
    end: int
    relocs: list[Relocation]

    @property
    def data(self) -> bytes:
        return self.section.raw[self.start:self.end]

    @property
    def name(self) -> str:
        return self.symbol.name


class Coff:
    def __init__(self, path: Path):
        self.path = path
        self.data = path.read_bytes()
        if len(self.data) < 20:
            raise ValueError("short COFF object")
        machine, nsects, _, ptrsym, nsym, optsize, _ = struct.unpack_from("<HHIIIHH", self.data, 0)
        if machine != 0x14C:
            raise ValueError(f"expected i386 COFF, got machine 0x{machine:04x}")
        self.symbol_ptr = ptrsym
        self.symbol_count = nsym
        sec_off = 20 + optsize
        raw_sections = []
        for i in range(nsects):
            off = sec_off + 40 * i
            name = self.data[off:off + 8]
            raw_sections.append((name, *struct.unpack_from("<IIIIIIHHI", self.data, off + 8)))
        sym_end = ptrsym + nsym * 18
        self.string_table = self.data[sym_end:sym_end + 4]
        str_len = struct.unpack_from("<I", self.string_table, 0)[0] if len(self.string_table) == 4 else 0
        self.string_data = self.data[sym_end:sym_end + str_len] if str_len >= 4 else b""

        def table_string(offset: int) -> str:
            if offset < 4 or offset >= len(self.string_data):
                return f"<bad-string-{offset}>"
            return cstr(self.string_data[offset:])

        def decode_name(raw: bytes, section_name: bool = False) -> str:
            if section_name and raw.startswith(b"/"):
                try:
                    return table_string(int(raw[1:].split(b"\0", 1)[0]))
                except ValueError:
                    return cstr(raw)
            if not section_name and raw[:4] == b"\0\0\0\0":
                return table_string(struct.unpack_from("<I", raw, 4)[0])
            return cstr(raw)

        self.sections: list[Section] = []
        for idx, (raw_name, paddr, vaddr, size, ptrraw, ptrrel, ptrline, nrel, nline, chars) in enumerate(raw_sections, 1):
            name = decode_name(raw_name, True)
            raw = self.data[ptrraw:ptrraw + size] if ptrraw and size else b""
            relocs = []
            for r in range(nrel):
                roff = ptrrel + 10 * r
                address, symbol_index, typ = struct.unpack_from("<IIH", self.data, roff)
                relocs.append(Relocation(idx, address, symbol_index, typ))
            self.sections.append(Section(idx, name, raw, ptrraw, size, relocs, chars))

        self.symbols: list[Symbol] = []
        sym_by_index: dict[int, Symbol] = {}
        i = 0
        while i < nsym:
            off = ptrsym + 18 * i
            raw_name = self.data[off:off + 8]
            value, section, typ, storage, naux = struct.unpack_from("<IhHBB", self.data, off + 8)
            aux = [self.data[off + 18 * (j + 1):off + 18 * (j + 2)] for j in range(naux)]
            symbol = Symbol(i, decode_name(raw_name), value, section, typ, storage, aux)
            self.symbols.append(symbol)
            sym_by_index[i] = symbol
            i += 1 + naux
        self.sym_by_index = sym_by_index

        # Section-definition auxiliary records carry true COMDAT/function-section size.
        for symbol in self.symbols:
            if symbol.storage == 3 and symbol.typ == 0 and 0 < symbol.section <= len(self.sections) and symbol.aux:
                section = self.sections[symbol.section - 1]
                if symbol.name == section.name or symbol.name.startswith(section.name):
                    section.aux_length = struct.unpack_from("<I", symbol.aux[0], 0)[0]

    def functions(self) -> list[Function]:
        by_section: dict[int, list[Symbol]] = defaultdict(list)
        for symbol in self.symbols:
            if symbol.section > 0 and symbol.section <= len(self.sections) and symbol.is_function:
                section = self.sections[symbol.section - 1]
                if section.name.lower().startswith(".text") and symbol.value < len(section.raw):
                    by_section[symbol.section].append(symbol)
        funcs: list[Function] = []
        for section_number, symbols in by_section.items():
            section = self.sections[section_number - 1]
            symbols.sort(key=lambda s: (s.value, s.index))
            for pos, symbol in enumerate(symbols):
                aux_size = 0
                if symbol.aux:
                    # Function-definition aux: TagIndex at 0, TotalSize at 4.
                    aux_size = struct.unpack_from("<I", symbol.aux[0], 4)[0]
                next_value = next((other.value for other in symbols[pos + 1:] if other.value > symbol.value), len(section.raw))
                end = symbol.value + aux_size if aux_size else next_value
                if section.name.upper().startswith(".TEXT$") and section.aux_length:
                    end = symbol.value + (aux_size or section.aux_length)
                end = min(max(end, symbol.value), len(section.raw))
                if end <= symbol.value:
                    continue
                relocs = [r for r in section.relocations if symbol.value <= r.offset < end]
                funcs.append(Function(symbol, section, symbol.value, end, relocs))
        return sorted(funcs, key=lambda f: (f.section.index, f.start, f.name))


def relocation_size(typ: int) -> int:
    return REL_SIZES.get(typ, 4)


def fixed_runs(func: Function) -> list[tuple[int, int]]:
    masked = bytearray(len(func.data))
    for reloc in func.relocs:
        size = relocation_size(reloc.typ)
        a = max(reloc.offset - func.start, 0)
        b = min(a + size, len(masked))
        masked[a:b] = b"\1" * max(0, b - a)
    runs = []
    i = 0
    while i < len(masked):
        if masked[i]:
            i += 1
            continue
        start = i
        while i < len(masked) and not masked[i]:
            i += 1
        runs.append((start, i))
    return runs


def is_byte_match(pattern: bytes, target: bytes, masked: list[bool]) -> bool:
    return all(masked[i] or pattern[i] == target[i] for i in range(len(pattern)))


def candidates_for(func: Function, exe_text: bytes) -> list[int]:
    data = func.data
    runs = fixed_runs(func)
    if not runs:
        return []
    start, end = max(runs, key=lambda item: item[1] - item[0])
    needle = data[start:end]
    found = []
    cursor = 0
    while True:
        hit = exe_text.find(needle, cursor)
        if hit < 0:
            break
        base = hit - start
        if base >= 0 and base + len(data) <= len(exe_text):
            masked = [False] * len(data)
            for reloc in func.relocs:
                a = reloc.offset - func.start
                size = relocation_size(reloc.typ)
                for p in range(max(0, a), min(len(masked), a + size)):
                    masked[p] = True
            if is_byte_match(data, exe_text[base:base + len(data)], masked):
                found.append(base)
        cursor = hit + 1
    return found


def closest_candidate(func: Function, exe_text: bytes) -> tuple[int | None, int | None]:
    """Find a useful nearby alignment for a disassembly diff without quadratic full scans."""
    data = func.data
    runs = fixed_runs(func)
    anchors = []
    for length in (8, 6, 4, 3, 2, 1):
        for start, end in runs:
            if end - start < length:
                continue
            for at in range(start, end - length + 1):
                needle = data[at:at + length]
                count = exe_text.count(needle)
                if count:
                    anchors.append((count, -length, at, needle))
        if anchors:
            # At the longest useful anchor width, choose the rarest anchor.
            chosen = [x for x in anchors if -x[1] == length]
            if chosen:
                break
    if not anchors:
        return (0 if len(data) <= len(exe_text) else None, None)
    _, neg_length, at, needle = min(anchors)
    candidates = []
    cursor = 0
    while len(candidates) < 4000:
        hit = exe_text.find(needle, cursor)
        if hit < 0:
            break
        base = hit - at
        if 0 <= base <= len(exe_text) - len(data):
            candidates.append(base)
        cursor = hit + 1
    if not candidates:
        return None, None
    fixed_positions = [p for a, b in runs for p in range(a, b)]
    samples = fixed_positions if len(fixed_positions) <= 24 else [fixed_positions[(i * (len(fixed_positions) - 1)) // 23] for i in range(24)]
    target_len = len(data)
    best_base = None
    best_score = target_len + 1
    for base in candidates:
        score = 0
        for p in samples:
            if data[p] != exe_text[base + p]:
                score += 1
        if score < best_score:
            best_score, best_base = score, base
            if score == 0:
                break
    if best_base is None:
        return None, None
    # Re-score the selected alignment over every non-relocated byte.
    full_score = sum(data[p] != exe_text[best_base + p] for p in fixed_positions)
    return best_base, full_score


def import_map(pe: pefile.PE) -> dict[str, int]:
    result = {}
    for entry in getattr(pe, "DIRECTORY_ENTRY_IMPORT", []):
        for imp in entry.imports:
            if imp.name:
                result[imp.name.decode("ascii", "replace").casefold()] = imp.address
    return result


def normalize_import(name: str) -> str:
    value = name
    if value.startswith("__imp_"):
        value = value[len("__imp_"):]
    value = value.lstrip("_")
    value = re.sub(r"@[0-9]+$", "", value)
    return value.casefold()


def resolve_sites(func: Function, base_offset: int, exe_text: bytes, text_rva: int, image_base: int,
                  coff: Coff, all_candidates: dict[str, list[int]], imports: dict[str, int],
                  explicit: dict[str, int], global_external: dict[str, tuple[int, int]]) -> tuple[str, list[dict[str, Any]]]:
    mappings: dict[str, int] = {}
    reports = []
    unresolved = []
    target_functions: dict[str, list[int]] = all_candidates
    for reloc in func.relocs:
        symbol = coff.sym_by_index.get(reloc.symbol_index)
        size = REL_SIZES.get(reloc.typ)
        site_rel = reloc.offset - func.start
        site = base_offset + site_rel
        rname = REL_NAMES.get(reloc.typ, f"0x{reloc.typ:04x}")
        entry: dict[str, Any] = {"offset": site_rel, "type": rname, "symbol": symbol.name if symbol else f"<index-{reloc.symbol_index}>"}
        if symbol is None or size is None or reloc.typ not in (0x0006, 0x0007, 0x0014):
            entry["verified"] = False
            entry["reason"] = "relocation type or symbol not decoded"
            reports.append(entry)
            unresolved.append(entry["reason"])
            continue
        field = exe_text[site:site + size]
        if len(field) != size:
            entry["verified"] = False
            entry["reason"] = "relocation field outside oracle .text"
            reports.append(entry)
            unresolved.append(entry["reason"])
            continue
        addend_raw = func.section.raw[reloc.offset:reloc.offset + size]
        if len(addend_raw) != size:
            addend = 0
        else:
            addend = int.from_bytes(addend_raw, "little", signed=True)
        field_value = int.from_bytes(field, "little", signed=(reloc.typ == 0x0014))
        field_va = image_base + text_rva + site
        if reloc.typ == 0x0014:
            # COFF REL32: field = S + A - (P + 4), P = address of the field.
            symbol_va = field_va + 4 + field_value - addend
            effective_va = field_va + 4 + field_value
        elif reloc.typ == 0x0006:
            symbol_va = (field_value & 0xFFFFFFFF) - addend
            effective_va = field_value & 0xFFFFFFFF
        else:
            symbol_va = image_base + (field_value & 0xFFFFFFFF) - addend
            effective_va = image_base + (field_value & 0xFFFFFFFF)
        entry.update({"addend": addend, "resolved_symbol_va": f"0x{symbol_va:08x}",
                      "effective_target_va": f"0x{effective_va:08x}"})
        symbol_key = symbol.key
        old = mappings.get(symbol_key)
        if old is not None and old != symbol_va:
            entry["verified"] = False
            entry["reason"] = f"symbol maps inconsistently: 0x{old:08x} vs 0x{symbol_va:08x}"
            unresolved.append(entry["reason"])
        else:
            mappings[symbol_key] = symbol_va
            target_import = imports.get(normalize_import(symbol.name)) if symbol.section == 0 else None
            if target_import is not None:
                # pefile ImportData.address is already an absolute VA.
                expected_va = target_import
                ok = symbol_va == expected_va
                entry.update({"verified": ok, "reason": "import IAT slot" if ok else f"expected IAT 0x{expected_va:08x}"})
                if not ok:
                    unresolved.append(entry["reason"])
            elif symbol.name in explicit:
                expected_va = explicit[symbol.name]
                ok = symbol_va == expected_va
                entry.update({"verified": ok, "reason": "explicit symbol mapping" if ok else f"expected 0x{expected_va:08x}"})
                if not ok:
                    unresolved.append(entry["reason"])
            elif symbol.section > 0 and symbol.is_function:
                target_hits = target_functions.get(symbol.name, [])
                if symbol.name == func.name:
                    target_hits = [base_offset]
                if len(target_hits) == 1:
                    expected_va = image_base + text_rva + target_hits[0]
                    ok = symbol_va == expected_va
                    entry.update({"verified": ok, "reason": "located function target" if ok else f"function target expected 0x{expected_va:08x}"})
                    if not ok:
                        unresolved.append(entry["reason"])
                else:
                    entry["verified"] = False
                    entry["reason"] = "internal function target is not uniquely located"
                    unresolved.append(entry["reason"])
            elif symbol.section > 0:
                # Defined TU data/section symbols are mapped by their linked address; repeated
                # references to the same symbol must agree above.
                entry["verified"] = True
                entry["reason"] = "consistent translation-unit data mapping"
            elif symbol_key in global_external:
                global_va, use_count = global_external[symbol_key]
                ok = symbol_va == global_va and use_count >= 2
                entry["verified"] = ok
                entry["reason"] = (f"consistent external symbol mapping across {use_count} sites" if ok
                                    else "external target mapping is inconsistent or observed only once")
                if not ok:
                    unresolved.append(entry["reason"])
            else:
                entry["verified"] = False
                entry["reason"] = "external target has no independent mapping"
                unresolved.append(entry["reason"])
        reports.append(entry)
    return ("STRICT" if not unresolved else "RELOC-UNVERIFIED"), reports


def disasm_lines(data: bytes, address: int, count: int = 8) -> list[str]:
    md = capstone.Cs(capstone.CS_ARCH_X86, capstone.CS_MODE_32)
    lines = []
    for insn in md.disasm(data, address, count=count):
        lines.append(f"{insn.address:08x}: {insn.bytes.hex(' '):<24} {insn.mnemonic} {insn.op_str}".rstrip())
    return lines


def parse_mapping(items: list[str], kind: str) -> dict[str, int]:
    result = {}
    for item in items:
        if "=" not in item:
            raise ValueError(f"{kind} mapping must be NAME=VALUE: {item}")
        name, value = item.rsplit("=", 1)
        result[name] = int(value, 0)
    return result


def compare_object(obj_path: Path, pe: pefile.PE, exe_text: bytes, text_rva: int,
                   imports: dict[str, int], explicit: dict[str, int], at: dict[str, int],
                   want_diff: bool = True) -> dict[str, Any]:
    coff = Coff(obj_path)
    functions = coff.functions()
    candidates: dict[str, list[int]] = {}
    function_hits = {}
    for func in functions:
        hits = candidates_for(func, exe_text)
        candidates.setdefault(func.name, []).extend(hits)
        function_hits[id(func)] = hits
    # Keep unique locations for verification of internal direct calls.
    unique_candidates = {name: sorted(set(hits)) for name, hits in candidates.items()}
    for name, rva in at.items():
        unique_candidates[name] = [rva - text_rva]
    # Build TU-wide external-symbol evidence from functions with a single selected location.
    external_uses: dict[str, list[int]] = defaultdict(list)
    for func in functions:
        selected = unique_candidates.get(func.name, [])
        if len(selected) != 1:
            continue
        base = selected[0]
        for reloc in func.relocs:
            symbol = coff.sym_by_index.get(reloc.symbol_index)
            size = REL_SIZES.get(reloc.typ)
            site = base + reloc.offset - func.start
            if symbol is None or symbol.section != 0 or size is None or reloc.typ not in (0x0006, 0x0007, 0x0014):
                continue
            field = exe_text[site:site + size]
            addend_raw = func.section.raw[reloc.offset:reloc.offset + size]
            if len(field) != size or len(addend_raw) != size:
                continue
            addend = int.from_bytes(addend_raw, "little", signed=True)
            field_value = int.from_bytes(field, "little", signed=(reloc.typ == 0x0014))
            field_va = pe.OPTIONAL_HEADER.ImageBase + text_rva + site
            if reloc.typ == 0x0014:
                symbol_va = field_va + field_value - addend
            elif reloc.typ == 0x0006:
                symbol_va = (field_value & 0xFFFFFFFF) - addend
            else:
                symbol_va = pe.OPTIONAL_HEADER.ImageBase + (field_value & 0xFFFFFFFF) - addend
            external_uses[symbol.key].append(symbol_va)
    global_external = {}
    for key, values in external_uses.items():
        if len(set(values)) == 1:
            global_external[key] = (values[0], len(values))
    results = []
    for func in functions:
        data = func.data
        hits = function_hits[id(func)]
        specified = at.get(func.name)
        if specified is not None:
            hit_offsets = [specified - text_rva]
        else:
            hit_offsets = hits
        records = []
        raw_exact_hits = 0
        for base in hit_offsets:
            if base < 0 or base + len(data) > len(exe_text):
                records.append({"rva": f"0x{specified:08x}" if specified is not None else None,
                                "status": "MISMATCH", "error": "candidate outside .text"})
                continue
            masked = [False] * len(data)
            for reloc in func.relocs:
                a = reloc.offset - func.start
                for p in range(max(0, a), min(len(masked), a + relocation_size(reloc.typ))):
                    masked[p] = True
            bytes_equal = is_byte_match(data, exe_text[base:base + len(data)], masked)
            if data == exe_text[base:base + len(data)]:
                raw_exact_hits += 1
            if not bytes_equal:
                status = "MISMATCH"
                reloc_reports = []
            else:
                status, reloc_reports = resolve_sites(func, base, exe_text, text_rva, pe.OPTIONAL_HEADER.ImageBase,
                                                      coff, unique_candidates, imports, explicit, global_external)
                if specified is None and len(set(hits)) != 1:
                    status = "RELOC-UNVERIFIED"
                    reloc_reports.append({"verified": False, "reason": "function location is ambiguous"})
            records.append({"rva": f"0x{text_rva + base:08x}", "status": status,
                            "size": len(data), "relocations": reloc_reports})
        best = None
        diff = None
        if want_diff and (not records or any(r["status"] == "MISMATCH" for r in records)):
            if specified is not None:
                best_offset = specified - text_rva
                score = None
            else:
                best_offset, score = closest_candidate(func, exe_text)
            if best_offset is not None and 0 <= best_offset < len(exe_text):
                best = {"rva": f"0x{text_rva + best_offset:08x}", "sampled_score": score}
                diff = {"object": disasm_lines(data, pe.OPTIONAL_HEADER.ImageBase + text_rva + best_offset),
                        "oracle": disasm_lines(exe_text[best_offset:best_offset + len(data)],
                                               pe.OPTIONAL_HEADER.ImageBase + text_rva + best_offset)}
        results.append({"name": func.name, "section": func.section.name, "size": len(data),
                        "relocation_count": len(func.relocs), "wildcard_hit_count": len(hits),
                        "raw_exact_hit_count": raw_exact_hits,
                        "matches": records, "best_candidate": best, "disassembly_diff": diff})
    return {"object": str(obj_path), "functions": results}


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("objects", nargs="+", type=Path)
    parser.add_argument("--exe", type=Path, default=DEFAULT_EXE)
    parser.add_argument("--at", action="append", default=[], metavar="SYMBOL=RVA", help="force comparison at one RVA")
    parser.add_argument("--symbol-va", action="append", default=[], metavar="SYMBOL=VA", help="independent target mapping")
    parser.add_argument("--json-out", type=Path, help="write full structured results")
    parser.add_argument("--summary", action="store_true", help="print only per-object counts")
    args = parser.parse_args()
    at = parse_mapping(args.at, "--at")
    explicit = parse_mapping(args.symbol_va, "--symbol-va")
    pe = pefile.PE(str(args.exe), fast_load=False)
    text_section = next((s for s in pe.sections if s.Name.rstrip(b"\0") == b".text"), None)
    if text_section is None:
        raise SystemExit("oracle has no .text section")
    text_rva = text_section.VirtualAddress
    exe_text = text_section.get_data()[:text_section.SizeOfRawData]
    imports = import_map(pe)
    all_results = []
    for obj_path in args.objects:
        obj_path = obj_path.resolve()
        result = compare_object(obj_path, pe, exe_text, text_rva, imports, explicit, at, not args.summary)
        all_results.append(result)
        counts = Counter()
        wildcard = 0
        exact_bytes = 0
        for func in result["functions"]:
            records = func["matches"]
            if func["wildcard_hit_count"]:
                wildcard += 1
                exact_bytes += int(func["raw_exact_hit_count"] > 0)
                statuses = [record["status"] for record in records]
                if len(records) == 1 and statuses == ["STRICT"]:
                    counts["STRICT"] += 1
                else:
                    counts["RELOC-UNVERIFIED"] += 1
            else:
                counts["MISMATCH"] += 1
        print(f"{obj_path.name}: functions={len(result['functions'])} wildcard-hit={wildcard} exact-byte-functions={exact_bytes} "
              f"strict={counts['STRICT']} reloc-unverified={counts['RELOC-UNVERIFIED']} mismatch={counts['MISMATCH']}")
        if not args.summary:
            for func in result["functions"]:
                statuses = ",".join(f"{m['rva']}:{m['status']}" for m in func["matches"]) or "no hit"
                print(f"  {func['name']} size={func['size']} relocs={func['relocation_count']} hits={func['wildcard_hit_count']} {statuses}")
                if func["disassembly_diff"]:
                    print(f"    nearest {func['best_candidate']}")
                    for left, right in zip(func["disassembly_diff"]["object"], func["disassembly_diff"]["oracle"]):
                        print(f"    OBJ {left}")
                        print(f"    EXE {right}")
    if args.json_out:
        output = args.json_out.resolve()
        output.parent.mkdir(parents=True, exist_ok=True)
        output.write_text(json.dumps(all_results, indent=2), encoding="utf-8")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
