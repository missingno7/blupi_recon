#!/usr/bin/env python3
"""Locate and compare MSVC COFF functions against the oracle PE image."""

from __future__ import annotations

import argparse
from collections import Counter, defaultdict
from dataclasses import dataclass, field
import csv
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
    alignment_padding: int = 0

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
                padding = max(0, next_value - end) if aux_size else 0
                funcs.append(Function(symbol, section, symbol.value, end, relocs, padding))
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
    # DPLAYX is imported by ordinal in this executable. The pinned DX6 import
    # library identifies these public symbols; the PE records their slots as
    # ordinals 1 and 2 rather than carrying import-by-name strings.
    ordinal_aliases = {
        ("dplayx.dll", 1): ("DirectPlayCreate",),
        ("dplayx.dll", 2): ("DirectPlayEnumerateA",),
    }
    result = {}
    for entry in getattr(pe, "DIRECTORY_ENTRY_IMPORT", []):
        dll = entry.dll.decode("ascii", "replace").casefold()
        for imp in entry.imports:
            if imp.name:
                names = (imp.name.decode("ascii", "replace"),)
            else:
                names = ordinal_aliases.get((dll, imp.ordinal), ())
            for name in names:
                key = normalize_import(name)
                previous = result.get(key)
                if previous is not None and previous != imp.address:
                    raise ValueError(f"import alias {name} resolves to multiple IAT slots")
                result[key] = imp.address
    return result


def normalize_import(name: str) -> str:
    value = name
    if value.startswith("__imp_"):
        value = value[len("__imp_"):]
    value = value.lstrip("_")
    value = re.sub(r"@[0-9]+$", "", value)
    return value.casefold()


def symbol_map_key(obj_path: Path, symbol: Symbol) -> str:
    if symbol.storage == 2:
        return "global:" + symbol.name
    return f"local:{obj_path.resolve()}:{symbol.index}"


def align16(value: int) -> int:
    return (value + 15) & ~15


def reachable_function_starts(start: int, funcs: list[Function]) -> set[int]:
    """Return starts reachable if each unlocated function is kept or dead-stripped."""
    positions = {start}
    for func in funcs:
        # A function section may be retained or removed by the original linker.
        positions |= {align16(position + len(func.data)) for position in positions}
        if len(positions) > 4096:
            return set()
    return positions


def select_tu_locations(loaded: list[tuple[Path, Coff, list[Function], dict[int, list[int]]]],
                        text_rva: int) -> dict[tuple[str, int], tuple[int, str]]:
    """Select only byte-unique locations or duplicates proved by TU order.

    For an ambiguous body, its nearest uniquely located neighbours must bracket
    exactly one hit. At a TU boundary, one unique neighbour is enough when the
    hit and neighbour are adjacent after /O2's 16-byte function alignment.
    Unlocated functions can be omitted by the original linker (for example dead
    COMDATs), so the boundary check uses the hit's actual byte extent and the
    nearest unique function rather than assuming every COFF section survived.
    """
    selected: dict[tuple[str, int], tuple[int, str]] = {}
    for obj_path, _coff, funcs, hits_by_index in loaded:
        raw: dict[int, list[int]] = {f.symbol.index: sorted(set(hits_by_index.get(f.symbol.index, [])))
                                     for f in funcs}
        chosen: dict[int, tuple[int, str]] = {}
        for func in funcs:
            hits = raw[func.symbol.index]
            if len(hits) == 1:
                chosen[func.symbol.index] = (hits[0], "unique")

        # The compiler emits the function sections in TU order. The linker in
        # this build keeps that order; section indices preserve it in the COFF.
        ordered_funcs = list(funcs)
        for pos, func in enumerate(ordered_funcs):
            func_hits = raw[func.symbol.index]
            if len(func_hits) < 2:
                continue
            prev_pos = next((i for i in range(pos - 1, -1, -1)
                             if len(raw[ordered_funcs[i].symbol.index]) == 1), None)
            next_pos = next((i for i in range(pos + 1, len(ordered_funcs))
                             if len(raw[ordered_funcs[i].symbol.index]) == 1), None)
            candidates: list[int] = []
            if prev_pos is not None and next_pos is not None:
                prev = ordered_funcs[prev_pos]
                next_func = ordered_funcs[next_pos]
                prev_va = raw[prev.symbol.index][0]
                next_va = raw[next_func.symbol.index][0]
                before = reachable_function_starts(align16(prev_va + len(prev.data)),
                                                   ordered_funcs[prev_pos + 1:pos])
                for hit in func_hits:
                    if hit not in before or hit % 16:
                        continue
                    after = reachable_function_starts(align16(hit + len(func.data)),
                                                      ordered_funcs[pos + 1:next_pos])
                    if next_va in after:
                        candidates.append(hit)
            elif pos == 0 and next_pos is not None:
                # First surviving function in this TU: boundary plus the
                # nearest unique successor. Intervening unlocated sections
                # may be omitted, but the gap must be reachable by alignment.
                next_func = ordered_funcs[next_pos]
                next_va = raw[next_func.symbol.index][0]
                for hit in func_hits:
                    after = reachable_function_starts(align16(hit + len(func.data)),
                                                      ordered_funcs[pos + 1:next_pos])
                    if hit % 16 == 0 and next_va in after:
                        candidates.append(hit)
            elif pos == len(ordered_funcs) - 1 and prev_pos is not None:
                prev = ordered_funcs[prev_pos]
                prev_va = raw[prev.symbol.index][0]
                before = reachable_function_starts(align16(prev_va + len(prev.data)),
                                                   ordered_funcs[prev_pos + 1:pos])
                for hit in func_hits:
                    if hit % 16 == 0 and hit in {align16(p + len(func.data)) for p in before}:
                        candidates.append(hit)
            if len(set(candidates)) == 1:
                chosen[func.symbol.index] = (candidates[0], "placed by TU order")

        for func in funcs:
            if func.symbol.index in chosen:
                selected[(str(obj_path.resolve()), func.symbol.index)] = chosen[func.symbol.index]
    return selected


def relocation_targets(reloc: Relocation, section: Section, base_offset: int,
                       exe_text: bytes, text_rva: int, image_base: int,
                       start: int) -> tuple[int, int, int] | None:
    size = REL_SIZES.get(reloc.typ)
    if size is None or reloc.typ not in (0x0006, 0x0007, 0x0014):
        return None
    site = base_offset + reloc.offset - start
    field = exe_text[site:site + size]
    raw_addend = section.raw[reloc.offset:reloc.offset + size]
    if len(field) != size or len(raw_addend) != size:
        return None
    addend = int.from_bytes(raw_addend, "little", signed=True)
    field_value = int.from_bytes(field, "little", signed=(reloc.typ == 0x0014))
    field_va = image_base + text_rva + site
    if reloc.typ == 0x0014:
        effective_va = field_va + 4 + field_value
    elif reloc.typ == 0x0006:
        effective_va = field_value & 0xFFFFFFFF
    else:
        effective_va = image_base + (field_value & 0xFFFFFFFF)
    return effective_va - addend, effective_va, addend


def pe_section_at(pe: pefile.PE, va: int):
    rva = va - pe.OPTIONAL_HEADER.ImageBase
    for section in pe.sections:
        begin = section.VirtualAddress
        end = begin + max(section.Misc_VirtualSize, section.SizeOfRawData)
        if begin <= rva < end:
            return section, rva - begin
    return None, None


def pe_bytes(pe: pefile.PE, va: int, size: int) -> bytes | None:
    section, offset = pe_section_at(pe, va)
    if section is None or offset + size > max(section.Misc_VirtualSize, section.SizeOfRawData):
        return None
    rva = va - pe.OPTIONAL_HEADER.ImageBase
    raw_size = min(size, max(0, section.SizeOfRawData - offset))
    raw = pe.get_data(rva, raw_size) if raw_size else b""
    if len(raw) < size:
        raw += b"\0" * (size - len(raw))
    return raw


def section_kind(name: str) -> str:
    value = name.lower().split("$")[0]
    if value.startswith(".text"):
        return "text"
    if value.startswith(".rdata"):
        return "rdata"
    if value.startswith(".data"):
        return "data"
    if value.startswith(".bss"):
        return "bss"
    return "other"


def verify_import_relocation(pe: pefile.PE, symbol: Symbol, reloc: Relocation,
                             symbol_va: int, effective_va: int, addend: int,
                             imports: dict[str, int]) -> dict[str, Any] | None:
    """Verify an import reference as an IAT address or linker-generated thunk."""
    if symbol.section != 0:
        return None
    iat_va = imports.get(normalize_import(symbol.name))
    if iat_va is None:
        return None
    if symbol.name.lower().startswith("__imp_"):
        ok = symbol_va == iat_va and effective_va == iat_va
        return {"verified": ok, "status": "STRICT" if ok else "MISMATCH",
                "reason": "import IAT slot" if ok else f"expected IAT slot 0x{iat_va:08x}",
                "import_iat_va": f"0x{iat_va:08x}"}
    if reloc.typ != 0x0014:
        return {"verified": False, "status": "RELOC-UNVERIFIED",
                "reason": "non-IAT import reference is not a REL32 thunk call",
                "import_iat_va": f"0x{iat_va:08x}"}

    thunk_section, _ = pe_section_at(pe, symbol_va)
    thunk_bytes = pe_bytes(pe, symbol_va, 6)
    expected = b"\xff\x25" + struct.pack("<I", iat_va)
    section_name = (thunk_section.Name.rstrip(b"\0").decode("ascii", "replace")
                    if thunk_section is not None else "")
    ok = (addend == 0 and effective_va == symbol_va and thunk_section is not None
          and section_kind(section_name) == "text" and thunk_bytes == expected)
    return {"verified": ok, "status": "STRICT" if ok else "MISMATCH",
            "reason": ("import jump thunk targets the matching IAT slot" if ok else
                       f"REL32 import target is not an ff 25 thunk to IAT 0x{iat_va:08x}"),
            "import_iat_va": f"0x{iat_va:08x}",
            "thunk_bytes": thunk_bytes.hex(" ") if thunk_bytes is not None else None}


def data_extent(coff: Coff, symbol: Symbol, addend: int) -> tuple[int, bytes] | None:
    if symbol.section <= 0 or symbol.section > len(coff.sections):
        return None
    section = coff.sections[symbol.section - 1]
    target_offset = symbol.value + addend
    if target_offset < 0 or target_offset >= max(section.raw_size, len(section.raw)):
        return None
    named_data = [s for s in coff.symbols
                  if s.section == symbol.section and not s.is_function and s.value >= 0
                  and s.storage in (2, 3) and not (s.typ == 0 and s.name == section.name)
                  and s.value <= target_offset]
    if symbol.name.startswith("??_C@"):
        if section_kind(section.name) == "bss" and symbol.name.startswith("??_C@_00"):
            # VC5 emits the empty-string COMDAT as a one-byte zero-fill BSS
            # section instead of a raw .rdata byte.
            return target_offset, b"\0"
        raw = section.raw[target_offset:]
        nul = raw.find(b"\0")
        if nul < 0:
            return None
        return target_offset, raw[:nul + 1]
    owners = [s for s in named_data if s.value <= target_offset]
    if not owners:
        return None
    owner = max(owners, key=lambda s: (s.value, s.index))
    following = [s.value for s in coff.symbols
                 if s.section == symbol.section and not s.is_function and s.value > owner.value
                 and s.storage in (2, 3) and not (s.typ == 0 and s.name == section.name)]
    end = min(following) if following else max(section.raw_size, len(section.raw))
    if end <= target_offset:
        return None
    if section.raw:
        content = section.raw[target_offset:min(end, len(section.raw))]
        if len(content) < end - target_offset:
            content += b"\0" * (end - target_offset - len(content))
    else:
        content = b"\0" * (end - target_offset)
    # The numerically named dectables arrays are count-prefixed int tables.
    # Their next COFF symbol can be separated by compiler alignment padding;
    # use the declared element count to keep that padding out of the payload.
    if re.fullmatch(r"\?table_[0-9a-f]{5}@@3PAHA", symbol.name, re.IGNORECASE) and len(content) >= 4:
        count = struct.unpack_from("<i", content, 0)[0]
        meaningful_size = (count + 1) * 4
        if 0 <= count and meaningful_size <= len(content):
            content = content[:meaningful_size]
    return target_offset, content


def path_string(data: bytes) -> str | None:
    value = data.split(b"\0", 1)[0].decode("cp1252", "replace")
    if re.search(r"[A-Za-z]:[\\/].*\.(?:cpp|cxx|c)\Z", value, re.IGNORECASE):
        return value
    return None


def verify_data_target(pe: pefile.PE, coff: Coff, symbol: Symbol, addend: int,
                       target_va: int) -> tuple[str, str, dict[str, Any]]:
    # A relocation to exactly one-past a proven initialized object is valid
    # linker output (for example, the limit pointer for a fixed table loop).
    # Prove the object at its base first; never reinterpret the adjacent bytes
    # as belonging to the source symbol.
    if (addend > 0 and symbol.storage in (2, 3) and symbol.section > 0
            and symbol.section <= len(coff.sections) and not symbol.name.startswith("??_C@")):
        base_extent = data_extent(coff, symbol, 0)
        if base_extent is not None:
            base_offset, expected = base_extent
            obj_section = coff.sections[symbol.section - 1]
            kind = section_kind(obj_section.name)
            base_va = target_va - addend
            start_section, start_offset = pe_section_at(pe, base_va)
            end_section, end_offset = pe_section_at(pe, target_va)
            start_name = (start_section.Name.rstrip(b"\0").decode("ascii", "replace")
                          if start_section else "<unmapped>")
            end_name = (end_section.Name.rstrip(b"\0").decode("ascii", "replace")
                        if end_section else "<unmapped>")
            expected_name = {"rdata": ".rdata", "data": ".data"}.get(kind)
            section_limit = (max(end_section.Misc_VirtualSize, end_section.SizeOfRawData)
                             if end_section else 0)
            at_object_end = (symbol.value == base_offset and addend == len(expected)
                             and base_va == target_va - len(expected)
                             and start_section is not None and start_section is end_section
                             and start_name == expected_name == end_name
                             and end_offset == start_offset + len(expected)
                             and end_offset <= section_limit)
            if at_object_end:
                actual = pe_bytes(pe, base_va, len(expected))
                entry = {"target_va": f"0x{target_va:08x}",
                         "object_section": obj_section.name,
                         "oracle_section": end_name,
                         "object_offset": base_offset,
                         "data_size": len(expected),
                         "one_past_data_object": True,
                         "one_past_offset": addend}
                if actual is None:
                    return "MISMATCH", "one-past data object's base exceeds oracle section bounds", entry
                if actual != expected:
                    entry["object_bytes"] = expected[:32].hex()
                    entry["oracle_bytes"] = actual[:32].hex()
                    return "MISMATCH", "one-past data object's initialized bytes differ", entry
                return "STRICT", "relocation targets the end of a byte-verified initialized data object", entry

    extent = data_extent(coff, symbol, addend)
    entry: dict[str, Any] = {"target_va": f"0x{target_va:08x}"}
    if extent is None:
        return "RELOC-UNVERIFIED", "initialized data extent is not identifiable", entry
    target_offset, expected = extent
    obj_section = coff.sections[symbol.section - 1]
    kind = section_kind(obj_section.name)
    oracle_section, oracle_offset = pe_section_at(pe, target_va)
    oracle_name = oracle_section.Name.rstrip(b"\0").decode("ascii", "replace") if oracle_section else "<unmapped>"
    entry.update({"object_section": obj_section.name, "oracle_section": oracle_name,
                  "object_offset": target_offset, "data_size": len(expected)})
    actual = pe_bytes(pe, target_va, len(expected))
    if kind == "bss":
        # MSVC's uninitialized globals share the image's writable .data output
        # section. PE file alignment can put the start of that zero-fill area in
        # the last file-alignment unit of raw .data, before VirtualSize exceeds
        # SizeOfRawData. Accept only the virtual tail or an all-zero aligned tail;
        # an object may straddle the end of the raw bytes into the virtual tail
        # (LINK 5.00, Speedy Eggbert 1: g_CDPath), all zero either way.
        file_alignment = max(1, int(pe.OPTIONAL_HEADER.FileAlignment))
        aligned_tail = max(0, oracle_section.SizeOfRawData - file_alignment) if oracle_section else 0
        in_virtual_bss = (oracle_section is not None and oracle_name == ".data"
                          and oracle_offset >= oracle_section.SizeOfRawData)
        in_zero_file_tail = (oracle_section is not None and oracle_name == ".data"
                             and oracle_offset >= aligned_tail
                             and oracle_offset + len(expected) <= max(oracle_section.SizeOfRawData,
                                                                      oracle_section.Misc_VirtualSize)
                             and actual is not None and not any(actual))
        kind_ok = in_virtual_bss or in_zero_file_tail
        if kind_ok:
            entry["oracle_bss_range"] = "virtual zero-fill" if in_virtual_bss else "zero file-alignment tail"
    else:
        expected_name = {"rdata": ".rdata", "data": ".data", "text": ".text"}.get(kind)
        kind_ok = oracle_name == expected_name
    if not kind_ok:
        return "MISMATCH", f"data target section kind {kind} landed in {oracle_name}", entry
    if actual is None:
        return "MISMATCH", "data target exceeds oracle section bounds", entry
    if actual != expected:
        obj_path = path_string(expected)
        oracle_path = path_string(actual)
        if obj_path or oracle_path:
            detail = {"object_path": obj_path, "oracle_path": oracle_path,
                      "object_offset": target_offset, "target_va": f"0x{target_va:08x}"}
            entry["build_path_difference"] = detail
            return "RELOC-UNVERIFIED", f"build-path data difference: object={obj_path!r}, oracle={oracle_path!r}", entry
        entry["object_bytes"] = expected[:32].hex()
        entry["oracle_bytes"] = actual[:32].hex()
        return "MISMATCH", "initialized data bytes differ", entry
    return "STRICT", "data section and initialized bytes match", entry


def relocation_targets_pe(reloc: Relocation, section: Section, site_va: int,
                          pe: pefile.PE) -> tuple[int, int, int] | None:
    """Resolve one relocated field at an arbitrary PE VA, including read-only data."""
    size = REL_SIZES.get(reloc.typ)
    if size is None or reloc.typ not in (0x0006, 0x0007, 0x0014):
        return None
    field = pe_bytes(pe, site_va, size)
    raw_addend = section.raw[reloc.offset:reloc.offset + size]
    if field is None or len(raw_addend) != size:
        return None
    addend = int.from_bytes(raw_addend, "little", signed=True)
    field_value = int.from_bytes(field, "little", signed=(reloc.typ == 0x0014))
    if reloc.typ == 0x0014:
        effective_va = site_va + 4 + field_value
    elif reloc.typ == 0x0006:
        effective_va = field_value & 0xFFFFFFFF
    else:
        effective_va = pe.OPTIONAL_HEADER.ImageBase + (field_value & 0xFFFFFFFF)
    return effective_va - addend, effective_va, addend


def verify_eh_sections(pe: pefile.PE, coff: Coff, obj_path: Path,
                       start_section: Section, start_base: int,
                       global_symbols: dict[str, int | None], imports: dict[str, int],
                       oracle_function_vas: set[int]
                       ) -> tuple[str, str, dict[str, Any]]:
    """Check VC5 .text$x funclets and their .xdata tables as linked sections."""
    anchors: dict[int, set[int]] = defaultdict(set)
    for symbol in coff.symbols:
        if symbol.section <= 0 or symbol.section > len(coff.sections):
            continue
        section = coff.sections[symbol.section - 1]
        if not (section.name.lower().startswith(".text$") or section.name.lower().startswith(".xdata")):
            continue
        value = global_symbols.get(symbol_map_key(obj_path, symbol))
        if value is not None:
            anchors[symbol.section].add(value - symbol.value)
    anchors[start_section.index].add(start_base)
    pending = [start_section.index]
    visited: set[int] = set()
    unresolved = False
    mismatch = False
    checked_sections = []
    relocation_details = []
    while pending:
        section_index = pending.pop()
        if section_index in visited:
            continue
        visited.add(section_index)
        section = coff.sections[section_index - 1]
        bases = anchors.get(section_index, set())
        if len(bases) != 1:
            unresolved = True
            continue
        base_va = next(iter(bases))
        expected_kind = "rdata" if section.name.lower().startswith(".xdata") else "text"
        oracle_section, oracle_offset = pe_section_at(pe, base_va)
        oracle_name = (oracle_section.Name.rstrip(b"\0").decode("ascii", "replace")
                       if oracle_section else "<unmapped>")
        actual = pe_bytes(pe, base_va, len(section.raw))
        if (actual is None or section_kind(oracle_name) != expected_kind
                or oracle_offset + len(section.raw) > max(oracle_section.Misc_VirtualSize,
                                                         oracle_section.SizeOfRawData)):
            mismatch = True
            checked_sections.append({"section": section.name, "base": f"0x{base_va:08x}",
                                     "oracle_section": oracle_name, "status": "MISMATCH"})
            continue
        fixed = bytearray(b"\1" * len(section.raw))
        for reloc in section.relocations:
            size = relocation_size(reloc.typ)
            if size is None:
                unresolved = True
                continue
            left = max(0, reloc.offset)
            right = min(len(section.raw), reloc.offset + size)
            if left < right:
                fixed[left:right] = b"\0" * (right - left)
        if any(fixed[i] and section.raw[i] != actual[i] for i in range(len(actual))):
            mismatch = True
        checked_sections.append({"section": section.name, "base": f"0x{base_va:08x}",
                                 "oracle_section": oracle_name,
                                 "fixed_bytes_match": not any(fixed[i] and section.raw[i] != actual[i]
                                                               for i in range(len(actual)))})
        for reloc in section.relocations:
            symbol = coff.sym_by_index.get(reloc.symbol_index)
            site_va = base_va + reloc.offset
            target = relocation_targets_pe(reloc, section, site_va, pe)
            detail: dict[str, Any] = {"section": section.name, "offset": reloc.offset,
                                      "type": REL_NAMES.get(reloc.typ, f"0x{reloc.typ:04x}"),
                                      "symbol": symbol.name if symbol else "<missing>"}
            if symbol is None or target is None:
                unresolved = True
                detail.update({"status": "RELOC-UNVERIFIED", "reason": "unresolved section relocation"})
                relocation_details.append(detail)
                continue
            symbol_va, effective_va, addend = target
            key = symbol_map_key(obj_path, symbol)
            import_result = verify_import_relocation(pe, symbol, reloc, symbol_va,
                                                     effective_va, addend, imports)
            if import_result is not None:
                detail.update(import_result)
                if import_result["status"] != "STRICT":
                    mismatch |= import_result["status"] == "MISMATCH"
                    unresolved |= import_result["status"] != "STRICT"
                relocation_details.append(detail)
                continue
            if symbol.section == section.index:
                expected_symbol_va = base_va + symbol.value
            elif (symbol.section > 0 and symbol.section <= len(coff.sections)
                  and symbol.section in anchors and len(anchors[symbol.section]) == 1):
                target_section = coff.sections[symbol.section - 1]
                if target_section.name.lower().startswith((".text$", ".xdata")):
                    expected_symbol_va = next(iter(anchors[symbol.section])) + symbol.value
                    if symbol.section not in visited:
                        pending.append(symbol.section)
                else:
                    expected_symbol_va = global_symbols.get(key)
            elif key in global_symbols:
                expected_symbol_va = global_symbols[key]
            else:
                expected_symbol_va = imports.get(normalize_import(symbol.name))
            if expected_symbol_va is None:
                unresolved = True
                detail.update({"status": "RELOC-UNVERIFIED", "reason": "no unique target binding"})
            elif symbol_va != expected_symbol_va or effective_va != expected_symbol_va + addend:
                mismatch = True
                detail.update({"status": "MISMATCH", "expected": f"0x{expected_symbol_va:08x}",
                               "actual": f"0x{symbol_va:08x}"})
            elif (symbol.is_function or reloc.typ == 0x0014) and effective_va not in oracle_function_vas:
                mismatch = True
                detail.update({"status": "MISMATCH", "reason": "code relocation misses an oracle function start"})
            else:
                detail.update({"status": "STRICT", "target": f"0x{symbol_va:08x}"})
            relocation_details.append(detail)
    status = "MISMATCH" if mismatch else "RELOC-UNVERIFIED" if unresolved else "STRICT"
    reason = ("VC5 exception sections and relocations match" if status == "STRICT" else
              "VC5 exception section contains a mismatching byte or target" if mismatch else
              "VC5 exception section has an unresolved relocation or placement")
    return status, reason, {"exception_sections": checked_sections,
                            "exception_relocations": relocation_details}


def local_label_target(pe: pefile.PE, coff: Coff, symbol: Symbol, func: Function,
                       base_offset: int, text_rva: int, image_base: int,
                       symbol_va: int, effective_va: int, addend: int) -> tuple[bool, str]:
    """Prove a compiler-local code label from its containing COFF section."""
    if (not symbol.name.startswith("$L") or symbol.storage != 6
            or symbol.section <= 0 or symbol.section > len(coff.sections)):
        return False, "not a function-local code label"
    section = coff.sections[symbol.section - 1]
    if section_kind(section.name) != "text":
        return False, "local label is not in a code section"

    if symbol.section == func.section.index and func.start <= symbol.value < func.end:
        expected_va = image_base + text_rva + base_offset + symbol.value - func.start
        ok = symbol_va == expected_va and effective_va == expected_va + addend
        return ok, "same-function section-relative label" if ok else "same-function label offset differs"

    # VC5 exception funclets can live in a companion .text$x section. Their
    # local label has no function symbol of its own, so verify the emitted code
    # from that label through the end of its section at the address reached by
    # the referring function. Relocation fields are checked by their own
    # relocations and are omitted only from this fixed-byte comparison.
    end = local_label_end(coff, symbol)
    if not symbol.value < end <= len(section.raw):
        return False, "local label has no identifiable code extent"
    target_section, _ = pe_section_at(pe, symbol_va)
    if target_section is None or target_section.Name.rstrip(b"\0") != b".text":
        return False, "local label target is outside oracle .text"
    expected = section.raw[symbol.value:end]
    actual = pe_bytes(pe, symbol_va, len(expected))
    if actual is None:
        return False, "local label extent exceeds oracle .text"
    fixed = bytearray(len(expected))
    fixed[:] = b"\1" * len(expected)
    for reloc in section.relocations:
        size = relocation_size(reloc.typ)
        left = max(symbol.value, reloc.offset)
        right = min(end, reloc.offset + size)
        if left < right:
            fixed[left - symbol.value:right - symbol.value] = b"\0" * (right - left)
    matches = all(not fixed[i] or expected[i] == actual[i] for i in range(len(expected)))
    ok = matches and effective_va == symbol_va + addend
    return ok, "companion local-label code bytes match" if ok else "companion local-label code differs"


def local_label_end(coff: Coff, symbol: Symbol) -> int:
    """Return the next symbol boundary for a local label in a code section."""
    section = coff.sections[symbol.section - 1]
    later = [other.value for other in coff.symbols
             if other.section == symbol.section and other.value > symbol.value]
    return min(later) if later else len(section.raw)


def resolve_sites(func: Function, base_offset: int, exe_text: bytes, text_rva: int, image_base: int,
                  pe: pefile.PE, coff: Coff, obj_path: Path, all_candidates: dict[str, list[int]],
                  function_defs: dict[str, list[Function]], data_defs: dict[str, list[tuple[Coff, Symbol]]],
                  imports: dict[str, int], explicit: dict[str, int],
                  global_symbols: dict[str, int | None], oracle_function_vas: set[int],
                  crt_data_evidence: dict[str, dict[str, Any]] | None = None) -> tuple[str, list[dict[str, Any]]]:
    reports: list[dict[str, Any]] = []
    unresolved = False
    hard_mismatch = False
    for reloc in func.relocs:
        symbol = coff.sym_by_index.get(reloc.symbol_index)
        site_rel = reloc.offset - func.start
        entry: dict[str, Any] = {"offset": site_rel, "type": REL_NAMES.get(reloc.typ, f"0x{reloc.typ:04x}"),
                                 "symbol": symbol.name if symbol else f"<index-{reloc.symbol_index}>"}
        target = relocation_targets(reloc, func.section, base_offset, exe_text, text_rva, image_base, func.start)
        if symbol is None or target is None:
            entry.update({"verified": False, "status": "RELOC-UNVERIFIED", "reason": "relocation type or field not decoded"})
            reports.append(entry)
            unresolved = True
            continue
        symbol_va, effective_va, addend = target
        entry.update({"addend": addend, "resolved_symbol_va": f"0x{symbol_va:08x}",
                      "effective_target_va": f"0x{effective_va:08x}"})

        # The x86 exception-chain head is an absolute FS:0 linker symbol, not
        # an image data object. The oracle field and COFF symbol must both be
        # exactly zero for this special binding.
        if (symbol.name == "__except_list" and symbol.section == 0 and symbol.value == 0
                and symbol.typ == 0 and reloc.typ == 0x0006):
            ok = symbol_va == 0 and effective_va == 0
            entry.update({"verified": ok, "status": "STRICT" if ok else "MISMATCH",
                          "reason": "VC5 absolute FS:0 exception-chain symbol" if ok
                          else "exception-chain absolute value is not zero"})
            hard_mismatch |= not ok
            unresolved |= not ok
            reports.append(entry)
            continue

        if (symbol.name.startswith("$L") and symbol.storage == 6
                and (symbol.section == func.section.index or symbol.name not in explicit)):
            ok, reason = local_label_target(pe, coff, symbol, func, base_offset, text_rva,
                                            image_base, symbol_va, effective_va, addend)
            status = "STRICT" if ok else "MISMATCH"
            if ok and symbol.section != func.section.index:
                section = coff.sections[symbol.section - 1]
                end = local_label_end(coff, symbol)
                companion_relocs = [r for r in section.relocations
                                    if symbol.value <= r.offset
                                    and r.offset + relocation_size(r.typ) <= end]
                helper = Function(func.symbol, section, symbol.value, end, companion_relocs)
                companion_status, companion_reports = resolve_sites(
                    helper, symbol_va - image_base - text_rva, exe_text, text_rva,
                    image_base, pe, coff, obj_path, all_candidates, function_defs,
                    data_defs, imports, explicit, global_symbols, oracle_function_vas,
                    crt_data_evidence)
                entry["companion_relocations"] = companion_reports
                if companion_status != "STRICT":
                    status = companion_status
                    reason += "; companion funclet relocation(s) " + companion_status
            entry.update({"verified": status == "STRICT", "status": status, "reason": reason})
            hard_mismatch |= status == "MISMATCH"
            unresolved |= status != "STRICT"
            reports.append(entry)
            continue

        key = symbol_map_key(obj_path, symbol)
        observed = global_symbols.get(key)
        if key not in global_symbols or observed is None or observed != symbol_va:
            conflict = key in global_symbols and observed is None
            reason = ("symbol has conflicting global bindings" if conflict else
                      "symbol has no unique run-wide binding")
            entry.update({"verified": False,
                          "status": "MISMATCH" if conflict else "RELOC-UNVERIFIED",
                          "reason": reason})
            reports.append(entry)
            hard_mismatch |= conflict
            unresolved = True
            continue

        import_result = verify_import_relocation(pe, symbol, reloc, symbol_va,
                                                 effective_va, addend, imports)
        if import_result is not None:
            entry.update(import_result)
            hard_mismatch |= import_result["status"] == "MISMATCH"
            unresolved |= import_result["status"] != "STRICT"
            reports.append(entry)
            continue
        if symbol.name in explicit:
            ok = symbol_va == explicit[symbol.name]
            reason = "explicit target mapping" if ok else f"expected 0x{explicit[symbol.name]:08x}"
            if ok and symbol.section == 0 and not symbol.is_function:
                base_section, _ = pe_section_at(pe, symbol_va)
                target_section, _ = pe_section_at(pe, effective_va)
                allowed = ("rdata", "data", "bss")
                section_name = (base_section.Name.rstrip(b"\0").decode("ascii", "replace")
                                if base_section is not None else "")
                ok = (base_section is not None and target_section is base_section
                      and section_kind(section_name) in allowed)
                reason = (f"explicit external data binding in oracle {section_name}" if ok else
                          "explicit external data mapping is outside one oracle data section")
            entry.update({"verified": ok, "status": "STRICT" if ok else "MISMATCH",
                          "reason": reason})
            hard_mismatch |= not ok
            unresolved |= not ok
            reports.append(entry)
            continue

        # Calls and function pointers to reconstructed TUs must resolve to the one
        # byte-verified candidate for that symbol across the complete object set.
        target_function: Function | None = None
        if symbol.name in function_defs and function_defs[symbol.name]:
            definitions = function_defs[symbol.name]
            if len(definitions) == 1:
                target_function = definitions[0]
        if target_function is None and symbol.section > 0 and symbol.section <= len(coff.sections):
            if section_kind(coff.sections[symbol.section - 1].name) == "text":
                target_offset = symbol.value + addend
                local = [f for f in coff.functions() if f.section.index == symbol.section and f.start == target_offset]
                if len(local) == 1:
                    target_function = local[0]
        if target_function is not None:
            hits = sorted(set(all_candidates.get(target_function.name, [])))
            if target_function.name == func.name:
                hits = [base_offset]
            if not hits:
                ok = effective_va in oracle_function_vas and effective_va == symbol_va
                entry.update({"verified": ok, "status": "STRICT" if ok else "RELOC-UNVERIFIED",
                              "reason": "oracle function entry bound for unreconstructed candidate"
                              if ok else "unreconstructed candidate call does not land on an oracle function entry"})
                unresolved |= not ok
            elif len(hits) == 1:
                expected_va = image_base + text_rva + hits[0]
                ok = effective_va == expected_va and (addend == 0 or symbol.section > 0 and target_function.start == symbol.value + addend)
                entry.update({"verified": ok, "status": "STRICT" if ok else "MISMATCH",
                              "reason": "verified reconstructed function RVA" if ok else f"expected function RVA 0x{expected_va:08x}"})
                hard_mismatch |= not ok
                unresolved |= not ok
            else:
                entry.update({"verified": False, "status": "RELOC-UNVERIFIED", "reason": "reconstructed function has no unique verified RVA"})
                unresolved = True
            reports.append(entry)
            continue

        # Unreconstructed direct calls are accepted only at an exact function entry
        # from the Ghidra oracle inventory, and the symbol binding remains global.
        if reloc.typ == 0x0014 or symbol.is_function:
            ok = effective_va in oracle_function_vas and effective_va == symbol_va
            entry.update({"verified": ok, "status": "STRICT" if ok else "RELOC-UNVERIFIED",
                          "reason": "oracle function entry bound for run" if ok else "target is not an oracle function entry"})
            unresolved |= not ok
            reports.append(entry)
            continue

        if (symbol.section > 0 and symbol.section <= len(coff.sections)
                and coff.sections[symbol.section - 1].name.lower().startswith(".xdata")):
            eh_status, eh_reason, eh_detail = verify_eh_sections(
                pe, coff, obj_path, coff.sections[symbol.section - 1],
                symbol_va - symbol.value, global_symbols, imports, oracle_function_vas)
            entry.update(eh_detail)
            entry.update({"verified": eh_status == "STRICT", "status": eh_status,
                          "reason": eh_reason})
            hard_mismatch |= eh_status == "MISMATCH"
            unresolved |= eh_status != "STRICT"
            reports.append(entry)
            continue

        # Defined TU data and data definitions from another reconstructed TU must
        # land in the matching PE section and retain their initialized payload.
        data_source: tuple[Coff, Symbol] | None = None
        if symbol.section > 0 and symbol.section <= len(coff.sections) and section_kind(coff.sections[symbol.section - 1].name) in ("rdata", "data", "bss"):
            data_source = (coff, symbol)
        elif symbol.section == 0:
            defs = data_defs.get(symbol.name, [])
            if len(defs) == 1:
                data_source = defs[0]
        if data_source is not None:
            data_status, reason, detail = verify_data_target(pe, data_source[0], data_source[1], addend, effective_va)
            entry.update(detail)
            evidence = (crt_data_evidence or {}).get(symbol.name)
            if evidence and global_symbols.get(key) == symbol_va:
                entry["crt_data_evidence"] = evidence
            entry.update({"verified": data_status == "STRICT", "status": data_status, "reason": reason})
            hard_mismatch |= data_status == "MISMATCH"
            unresolved |= data_status != "STRICT"
            reports.append(entry)
            continue

        entry.update({"verified": False, "status": "RELOC-UNVERIFIED", "reason": "external data target has no unique reconstructed definition"})
        unresolved = True
        reports.append(entry)
    if hard_mismatch:
        return "MISMATCH", reports
    return ("RELOC-UNVERIFIED" if unresolved else "STRICT"), reports


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
                   want_diff: bool = True, shared_candidates: dict[str, list[int]] | None = None,
                   global_symbols: dict[str, int | None] | None = None,
                   function_defs: dict[str, list[Function]] | None = None,
                   data_defs: dict[str, list[tuple[Coff, Symbol]]] | None = None,
                   oracle_function_vas: set[int] | None = None,
                   locations: dict[tuple[str, int], tuple[int, str]] | None = None,
                   folded: dict[tuple[str, int], dict[str, Any]] | None = None,
                   location_issues: dict[tuple[str, int], str] | None = None,
                   crt_data_evidence: dict[str, dict[str, Any]] | None = None) -> dict[str, Any]:
    coff = Coff(obj_path)
    functions = coff.functions()
    candidates: dict[str, list[int]] = {}
    function_hits = {}
    for func in functions:
        hits = candidates_for(func, exe_text)
        candidates.setdefault(func.name, []).extend(hits)
        function_hits[id(func)] = hits
    # Locations are shared across every object so cross-TU references use the
    # same binding as same-TU calls.
    unique_candidates = ({name: list(values) for name, values in shared_candidates.items()}
                         if shared_candidates is not None
                         else {name: sorted(set(hits)) for name, hits in candidates.items()})
    results = []
    for func in functions:
        data = func.data
        hits = function_hits[id(func)]
        specified = at.get(func.name)
        located = (locations or {}).get((str(obj_path.resolve()), func.symbol.index))
        if specified is not None:
            hit_offsets = [specified - text_rva]
            placement_basis = (located[1] if located is not None and located[0] == specified - text_rva
                               else "forced by --at")
        elif located is not None:
            hit_offsets = [located[0]]
            placement_basis = located[1]
        else:
            hit_offsets = hits
            placement_basis = (location_issues or {}).get((str(obj_path.resolve()), func.symbol.index),
                             "ambiguous" if len(set(hits)) > 1 else "unlocated")
        records = []
        raw_exact_hits = 0
        folded_into = (folded or {}).get((str(obj_path.resolve()), func.symbol.index))
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
            elif located is None and specified is None:
                # Candidate bytes alone do not authorize their relocations to
                # affect run-wide bindings. Keep all locations visible, but
                # leave their address-dependent evidence unverified.
                status = "RELOC-UNVERIFIED"
                reloc_reports = [{"verified": False, "status": "RELOC-UNVERIFIED",
                                  "reason": (location_issues or {}).get(
                                      (str(obj_path.resolve()), func.symbol.index),
                                      "function location is ambiguous" if len(set(hits)) > 1
                                      else "function location is not proven") }]
            else:
                status, reloc_reports = resolve_sites(
                    func, base, exe_text, text_rva, pe.OPTIONAL_HEADER.ImageBase, pe, coff, obj_path,
                    unique_candidates, function_defs or {}, data_defs or {}, imports, explicit,
                    global_symbols or {}, oracle_function_vas or set(), crt_data_evidence)
                if status == "STRICT" and folded_into is not None:
                    status = "FOLDED"
            records.append({"rva": f"0x{text_rva + base:08x}", "status": status,
                            "size": len(data), "code_size": len(data),
                            "alignment_padding_after": func.alignment_padding,
                            "placement_basis": placement_basis,
                            **({"folded_into": folded_into} if folded_into is not None else {}),
                            "relocations": reloc_reports})
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
        overall_status = "MISMATCH"
        if len(records) == 1:
            overall_status = records[0]["status"]
        elif records and any(r["status"] == "RELOC-UNVERIFIED" for r in records):
            overall_status = "RELOC-UNVERIFIED"
        results.append({"name": func.name, "section": func.section.name, "size": len(data),
                        "code_size": len(data), "alignment_padding_after": func.alignment_padding,
                        "placement_basis": placement_basis,
                        "status": overall_status,
                        **({"folded_into": folded_into} if folded_into is not None else {}),
                        "relocation_count": len(func.relocs), "wildcard_hit_count": len(hits),
                        "raw_exact_hit_count": raw_exact_hits,
                        "matches": records, "best_candidate": best, "disassembly_diff": diff})
    return {"object": str(obj_path), "functions": results}


def oracle_function_starts(path: Path = ROOT / "oracle" / "functions.csv") -> set[int]:
    starts = set()
    with path.open("r", encoding="utf-8-sig", newline="") as stream:
        for row in csv.DictReader(stream):
            value = row.get("rva", "")
            if value:
                starts.add(int(value, 0))
    return starts


def oracle_tu_at_rva(path: Path = ROOT / "oracle" / "tu_map.csv") -> dict[int, str]:
    """Return source-TU ownership for exact oracle function starts."""
    result: dict[int, str] = {}
    with path.open("r", encoding="utf-8-sig", newline="") as stream:
        for row in csv.DictReader(stream):
            if row.get("rva") and row.get("tu"):
                result[int(row["rva"], 0)] = row["tu"]
    return result


def object_tu_name(path: Path) -> str:
    """Recover the canonical TU stem from verify.py's NN_<stem>.obj naming."""
    stem = path.stem
    match = re.match(r"^\d+_(.+)$", stem)
    if match:
        stem = match.group(1)
    return Path(stem.split("-", 1)[0]).stem.lower()


def tu_order_proves_location(funcs: list[Function], locations: dict[tuple[str, int], tuple[int, str]],
                             obj_key: str, func: Function, offset: int) -> bool:
    """Check an independently found location against adjacent functions in this object."""
    if offset % 16:
        return False
    pos = next((i for i, candidate in enumerate(funcs)
                if candidate.symbol.index == func.symbol.index), None)
    if pos is None:
        return False

    def placed(i: int) -> tuple[int, str] | None:
        location = locations.get((obj_key, funcs[i].symbol.index))
        if location is None or location[1] not in ("unique", "placed by TU order"):
            return None
        return location

    prev_pos = next((i for i in range(pos - 1, -1, -1) if placed(i) is not None), None)
    next_pos = next((i for i in range(pos + 1, len(funcs)) if placed(i) is not None), None)
    if prev_pos is not None and next_pos is not None:
        prev = funcs[prev_pos]
        next_func = funcs[next_pos]
        prev_base = placed(prev_pos)[0]  # type: ignore[index]
        next_base = placed(next_pos)[0]  # type: ignore[index]
        before = reachable_function_starts(align16(prev_base + len(prev.data)), funcs[prev_pos + 1:pos])
        after = reachable_function_starts(align16(offset + len(func.data)), funcs[pos + 1:next_pos])
        return offset in before and next_base in after
    if pos == 0 and next_pos is not None:
        next_func = funcs[next_pos]
        next_base = placed(next_pos)[0]  # type: ignore[index]
        after = reachable_function_starts(align16(offset + len(func.data)), funcs[pos + 1:next_pos])
        return next_base in after
    if pos == len(funcs) - 1 and prev_pos is not None:
        prev = funcs[prev_pos]
        prev_base = placed(prev_pos)[0]  # type: ignore[index]
        before = reachable_function_starts(align16(prev_base + len(prev.data)), funcs[prev_pos + 1:pos])
        return offset in before
    return False


def layout_warnings(obj_path: Path, locations: dict[tuple[str, int], tuple[int, str]],
                    text_rva: int, folded: dict[tuple[str, int], dict[str, Any]] | None = None) -> list[str]:
    """Report selected adjacent object functions that do not occupy adjacent aligned RVAs."""
    coff = Coff(obj_path)
    funcs = coff.functions()
    warnings = []
    obj_key = str(obj_path.resolve())
    for left, right in zip(funcs, funcs[1:]):
        left_key = (obj_key, left.symbol.index)
        right_key = (obj_key, right.symbol.index)
        if folded and (left_key in folded or right_key in folded):
            continue
        left_location = locations.get(left_key)
        right_location = locations.get(right_key)
        if left_location is None or right_location is None:
            continue
        left_base, left_basis = left_location
        right_base, right_basis = right_location
        expected = align16(left_base + len(left.data))
        if right_base != expected:
            relation = "out-of-order/overlap" if right_base < left_base + len(left.data) else "gap beyond alignment padding"
            warnings.append(
                f"{left.name} ({text_rva + left_base:#x}, {left_basis}) -> "
                f"{right.name} ({text_rva + right_base:#x}, {right_basis}): {relation}, "
                f"expected next RVA {text_rva + expected:#x}")
    return warnings


def bind_observed_symbols(loaded: list[tuple[Path, Coff, list[Function], dict[int, list[int]]]],
                          locations: dict[tuple[str, int], tuple[int, str]],
                          exe_text: bytes, text_rva: int, image_base: int,
                          pe: pefile.PE) -> dict[str, int | None]:
    observed: dict[str, set[int]] = defaultdict(set)
    pending: list[tuple[Path, Coff, Section, int]] = []
    queued: set[tuple[str, int, int]] = set()

    def queue_section(obj_path: Path, coff: Coff, section: Section, base_va: int) -> None:
        if not (section.name.lower().startswith(".text$")
                or section.name.lower().startswith(".xdata")):
            return
        marker = (str(obj_path.resolve()), section.index, base_va)
        if marker not in queued:
            queued.add(marker)
            pending.append((obj_path, coff, section, base_va))

    for obj_path, coff, funcs, _hits_by_index in loaded:
        for func in funcs:
            location = locations.get((str(obj_path.resolve()), func.symbol.index))
            if location is None:
                continue
            for reloc in func.relocs:
                target = relocation_targets(reloc, func.section, location[0], exe_text,
                                            text_rva, image_base, func.start)
                symbol = coff.sym_by_index.get(reloc.symbol_index)
                if target is not None and symbol is not None:
                    observed[symbol_map_key(obj_path, symbol)].add(target[0])
                    if (symbol.section > 0 and symbol.section <= len(coff.sections)
                            and symbol.section != func.section.index):
                        queue_section(obj_path, coff, coff.sections[symbol.section - 1],
                                      target[0] - symbol.value)

    while pending:
        obj_path, coff, section, base_va = pending.pop()
        for symbol in coff.symbols:
            if (symbol.section == section.index and symbol.storage in (2, 3, 6)
                    and symbol.value >= 0):
                observed[symbol_map_key(obj_path, symbol)].add(base_va + symbol.value)
        for reloc in section.relocations:
            target = relocation_targets_pe(reloc, section, base_va + reloc.offset, pe)
            symbol = coff.sym_by_index.get(reloc.symbol_index)
            if target is None or symbol is None:
                continue
            observed[symbol_map_key(obj_path, symbol)].add(target[0])
            if symbol.section > 0 and symbol.section <= len(coff.sections):
                queue_section(obj_path, coff, coff.sections[symbol.section - 1],
                              target[0] - symbol.value)
    return {key: next(iter(values)) if len(values) == 1 else None
            for key, values in observed.items()}


def bind_unique_data_symbols(data_defs: dict[str, list[tuple[Coff, Symbol]]], pe: pefile.PE,
                             global_symbols: dict[str, int | None]) -> dict[str, dict[str, Any]]:
    """Bind unobserved initialized symbols only when their full payload has one section hit."""
    evidence: dict[str, dict[str, Any]] = {}
    image_base = int(pe.OPTIONAL_HEADER.ImageBase)
    for name, definitions in data_defs.items():
        key = "global:" + name
        # Observed references, including conflicting observations, outrank a
        # content search. The payload check will then validate the actual target.
        if key in global_symbols or len(definitions) != 1:
            continue
        coff, symbol = definitions[0]
        if symbol.section <= 0 or symbol.section > len(coff.sections):
            continue
        object_section = coff.sections[symbol.section - 1]
        kind = section_kind(object_section.name)
        if kind not in ("rdata", "data"):
            continue
        extent = data_extent(coff, symbol, 0)
        if extent is None:
            continue
        start, expected = extent
        if not expected:
            continue
        if any(start <= reloc.offset < start + len(expected)
               and reloc.offset + relocation_size(reloc.typ) > start
               for reloc in object_section.relocations):
            continue
        alignment = 2 if symbol.name.endswith("@@3PAFA") else 4
        matches: list[tuple[Any, int]] = []
        expected_name = {"rdata": ".rdata", "data": ".data"}[kind]
        for oracle_section in pe.sections:
            oracle_name = oracle_section.Name.rstrip(b"\0").decode("ascii", "replace")
            if oracle_name != expected_name:
                continue
            raw = oracle_section.get_data()
            offset = 0
            while True:
                offset = raw.find(expected, offset)
                if offset < 0:
                    break
                if offset % alignment == 0:
                    matches.append((oracle_section, offset))
                    if len(matches) > 1:
                        break
                offset += 1
            if len(matches) > 1:
                break
        if len(matches) != 1:
            continue
        oracle_section, offset = matches[0]
        target_va = image_base + int(oracle_section.VirtualAddress) + offset
        global_symbols[key] = target_va
        evidence[name] = {
            "target_va": f"0x{target_va:08x}",
            "object_section": object_section.name,
            "oracle_section": expected_name,
            "data_size": len(expected),
            "proof": "unique exact initialized-payload hit at natural alignment",
        }
    return evidence


def derive_crt_data_evidence(evidence_paths: list[Path],
                             locations: dict[tuple[str, int], tuple[int, str]],
                             global_symbols: dict[str, int | None], pe: pefile.PE,
                             exe_text: bytes, text_rva: int, image_base: int) -> dict[str, dict[str, Any]]:
    """Bind __pgmptr only from the pinned VC5 CRT declaration, reference, and data extent."""
    by_name = {path.name.lower(): path for path in evidence_paths}
    stdargv_path = by_name.get("crt_evidence_stdargv.obj")
    crt0dat_path = by_name.get("crt_evidence_crt0dat.obj")
    if not stdargv_path or not crt0dat_path:
        return {}
    stdargv = Coff(stdargv_path)
    crt0dat = Coff(crt0dat_path)
    refs = [func for func in stdargv.functions() if func.name == "__setargv"]
    definitions = [symbol for symbol in crt0dat.symbols
                   if symbol.name == "__pgmptr" and symbol.storage == 2
                   and not symbol.is_function and symbol.section > 0]
    if len(refs) != 1 or len(definitions) != 1:
        return {}
    func = refs[0]
    location = locations.get((str(stdargv_path.resolve()), func.symbol.index))
    if location is None or location[1] != "unique":
        return {}
    offset, basis = location
    rva = text_rva + offset
    try:
        with (ROOT / "oracle" / "functions.csv").open(newline="", encoding="utf-8-sig") as stream:
            oracle_record = next((row for row in csv.DictReader(stream)
                                  if int(row["rva"], 0) == rva), None)
    except (OSError, ValueError, KeyError):
        return {}
    if not oracle_record or "crt" not in oracle_record.get("kind", "").lower():
        return {}
    masked = [False] * len(func.data)
    for reloc in func.relocs:
        begin = reloc.offset - func.start
        for index in range(max(0, begin), min(len(masked), begin + relocation_size(reloc.typ))):
            masked[index] = True
    if not is_byte_match(func.data, exe_text[offset:offset + len(func.data)], masked):
        return {}
    pgmptr_relocs = []
    for reloc in func.relocs:
        symbol = stdargv.sym_by_index.get(reloc.symbol_index)
        if symbol and symbol.name == "__pgmptr":
            pgmptr_relocs.append((reloc, symbol))
    if len(pgmptr_relocs) != 1:
        return {}
    reloc, reference_symbol = pgmptr_relocs[0]
    if (reference_symbol.section != 0 or reference_symbol.storage != 2
            or reloc.typ != 0x0006):
        return {}
    target = relocation_targets(reloc, func.section, offset, exe_text, text_rva,
                                image_base, func.start)
    if target is None or target[2] != 0 or target[0] != target[1]:
        return {}
    target_va = target[0]
    if global_symbols.get("global:__pgmptr") != target_va:
        return {}
    definition = definitions[0]
    extent = data_extent(crt0dat, definition, 0)
    if (extent is None or len(extent[1]) != 4 or extent[1] != b"\0" * 4
            or section_kind(crt0dat.sections[definition.section - 1].name) != "bss"):
        return {}
    data_status, _reason, detail = verify_data_target(pe, crt0dat, definition, 0, target_va)
    if data_status != "STRICT":
        return {}
    return {"__pgmptr": {
        "target_va": f"0x{target_va:08x}",
        "reference_source": "VC5 SP3 CRT/SRC/STDARGV.C::__setargv",
        "reference_rva": f"0x{rva:08x}",
        "reference_placement": basis,
        "definition_source": "VC5 SP3 CRT/SRC/CRT0DAT.C::_pgmptr",
        "definition_section": detail.get("object_section"),
        "definition_size": len(extent[1]),
        "oracle_section": detail.get("oracle_section"),
        "proof": "unique CRT code match references the same externally defined zero-filled data object"
    }}


def collect_global_state(obj_paths: list[Path], pe: pefile.PE, exe_text: bytes, text_rva: int,
                         at: dict[str, int], explicit: dict[str, int]) -> tuple[dict[str, list[int]], dict[str, int | None],
                                                       dict[str, list[Function]], dict[str, list[tuple[Coff, Symbol]]],
                                                        dict[tuple[str, int], tuple[int, str]],
                                                        dict[tuple[str, int], dict[str, Any]],
                                                        dict[tuple[str, int], str],
                                                        dict[str, dict[str, Any]]]:
    loaded: list[tuple[Path, Coff, list[Function], dict[int, list[int]]]] = []
    candidates: dict[str, list[int]] = defaultdict(list)
    function_defs: dict[str, list[Function]] = defaultdict(list)
    data_defs: dict[str, list[tuple[Coff, Symbol]]] = defaultdict(list)
    for obj_path in obj_paths:
        coff = Coff(obj_path)
        funcs = coff.functions()
        hits_by_index: dict[int, list[int]] = {}
        for func in funcs:
            function_defs[func.name].append(func)
            hits = candidates_for(func, exe_text)
            hits_by_index[func.symbol.index] = hits
        for symbol in coff.symbols:
            if symbol.storage == 2 and symbol.section > 0 and not symbol.is_function and symbol.section <= len(coff.sections):
                if section_kind(coff.sections[symbol.section - 1].name) in ("rdata", "data", "bss"):
                    data_defs[symbol.name].append((coff, symbol))
        loaded.append((obj_path, coff, funcs, hits_by_index))
    locations = select_tu_locations(loaded, text_rva)
    tu_by_rva = oracle_tu_at_rva()
    location_issues: dict[tuple[str, int], str] = {}
    function_by_key: dict[tuple[str, int], Function] = {}
    object_tu_by_key: dict[tuple[str, int], str] = {}
    for obj_path, _coff, funcs, _hits in loaded:
        obj_key = str(obj_path.resolve())
        for func in funcs:
            key = (obj_key, func.symbol.index)
            function_by_key[key] = func
            object_tu_by_key[key] = object_tu_name(obj_path)

    # A unique byte sequence outside the source TU is still only a byte hit.
    # Keep it when neighbouring functions prove its place in this TU; otherwise
    # defer it until verified callers and an identical owner body prove folding.
    for obj_path, _coff, funcs, _hits in loaded:
        obj_key = str(obj_path.resolve())
        obj_tu = object_tu_name(obj_path)
        for func in funcs:
            key = (obj_key, func.symbol.index)
            location = locations.get(key)
            if location is None or location[1] != "unique":
                continue
            mapped_tu = tu_by_rva.get(text_rva + location[0])
            # Evidence-only CRT objects are not reconstructed TUs. Their
            # unique hits are usable only when the oracle map identifies CRT.
            if obj_tu.startswith("crt_evidence_") and mapped_tu == "crt":
                continue
            if not mapped_tu or Path(mapped_tu).stem.lower() == obj_tu:
                continue
            if tu_order_proves_location(funcs, locations, obj_key, func, location[0]):
                locations[key] = (location[0], "placed by TU order")
            else:
                del locations[key]
                location_issues[key] = (f"unique byte hit is mapped to {mapped_tu}; no same-TU order proof "
                                        "or cross-TU fold proof")

    # Apply explicit target assertions before using calls as placement proof.
    # Accepted functions are still checked for strict bytes and relocations
    # below; forced mismatches therefore cannot become call witnesses.
    for obj_path, _coff, funcs, hits_by_index in loaded:
        obj_key = str(obj_path.resolve())
        for func in funcs:
            expected = at.get(func.name)
            if expected is None:
                continue
            hit = expected - text_rva
            if hit in hits_by_index.get(func.symbol.index, []):
                key = (obj_key, func.symbol.index)
                current = locations.get(key)
                if current is None or current[0] != hit:
                    locations[key] = (hit, "forced by --at")

    # A located caller can prove an ambiguous direct callee when its bytes and
    # all other relocations are strict. The unresolved REL32 itself is the
    # placement witness, so do not require its target to be pre-bound.
    image_base = pe.OPTIONAL_HEADER.ImageBase
    oracle_function_vas = {image_base + rva for rva in oracle_function_starts()}
    located_candidates: dict[str, list[int]] = defaultdict(list)
    for obj_path, _coff, funcs, _hits in loaded:
        for func in funcs:
            location = locations.get((str(obj_path.resolve()), func.symbol.index))
            if location is not None:
                located_candidates[func.name].append(location[0])
    located_candidates = {name: sorted(set(values)) for name, values in located_candidates.items()}
    imports = import_map(pe)
    initial_bindings = bind_observed_symbols(loaded, locations, exe_text, text_rva, image_base, pe)
    call_observations: dict[str, list[tuple[tuple[str, int], int]]] = defaultdict(list)
    companion_witnesses: dict[tuple[str, int], set[tuple[str, int]]] = defaultdict(set)
    # VC5 exception cleanup calls live in companion .text$x sections. Verify
    # that whole linked section graph, then let its direct REL32s witness the
    # placement of ambiguous function bodies referenced by the parent method.
    for obj_path, coff, funcs, hits_by_index in loaded:
        obj_key = str(obj_path.resolve())
        for func in funcs:
            caller_key = (obj_key, func.symbol.index)
            location = locations.get(caller_key)
            if location is None or location[0] not in hits_by_index.get(func.symbol.index, []):
                continue
            for reloc in func.relocs:
                symbol = coff.sym_by_index.get(reloc.symbol_index)
                if (symbol is None or not symbol.name.startswith("$L") or symbol.storage != 6
                        or symbol.section <= 0 or symbol.section > len(coff.sections)
                        or symbol.section == func.section.index):
                    continue
                section = coff.sections[symbol.section - 1]
                if not section.name.lower().startswith(".text$"):
                    continue
                label_target = relocation_targets(reloc, func.section, location[0], exe_text,
                                                  text_rva, image_base, func.start)
                if label_target is None:
                    continue
                eh_status, _eh_reason, eh_detail = verify_eh_sections(
                    pe, coff, obj_path, section, label_target[0] - symbol.value,
                    initial_bindings, imports, oracle_function_vas)
                if eh_status != "STRICT":
                    continue
                for detail in eh_detail["exception_relocations"]:
                    name = detail.get("symbol", "")
                    if detail.get("type") != "REL32" or detail.get("status") != "STRICT":
                        continue
                    definitions = function_defs.get(name, [])
                    if len(definitions) != 1:
                        continue
                    target_va = int(detail["target"], 16)
                    raw_hits = candidates_for(definitions[0], exe_text)
                    if (target_va in oracle_function_vas
                            and target_va - image_base - text_rva in raw_hits):
                        call_observations[name].append((caller_key, target_va))
                        companion_witnesses[caller_key].add((name, target_va))
    for obj_path, coff, funcs, _hits_by_index in loaded:
        obj_key = str(obj_path.resolve())
        for func in funcs:
            caller_key = (obj_key, func.symbol.index)
            location = locations.get(caller_key)
            if location is None or location[1] not in ("unique", "placed by TU order",
                                                        "placed by verified references", "forced by --at"):
                continue
            if location[0] not in _hits_by_index.get(func.symbol.index, []):
                continue
            caller_status, caller_relocations = resolve_sites(
                func, location[0], exe_text, text_rva, image_base, pe, coff, obj_path,
                located_candidates, function_defs, data_defs, imports, explicit,
                initial_bindings, oracle_function_vas)
            if caller_status == "MISMATCH":
                continue
            if caller_status != "STRICT":
                unresolved = [r for r in caller_relocations if r.get("status") != "STRICT"]
                call_witnesses_only = bool(unresolved)
                for report in unresolved:
                    defs = function_defs.get(report.get("symbol", ""), [])
                    candidate_reference = (report.get("type") == "REL32"
                                           or report.get("type") == "DIR32"
                                           and len(defs) == 1 and defs[0].symbol.is_function)
                    if (report.get("status") != "RELOC-UNVERIFIED"
                            or not candidate_reference or len(defs) != 1):
                        call_witnesses_only = False
                        break
                    raw_hits = sorted(set(candidates_for(defs[0], exe_text)))
                    target_va = int(report.get("effective_target_va", "0"), 16)
                    direct_witness = (target_va in oracle_function_vas
                                      and target_va - image_base - text_rva in raw_hits)
                    companion_witness = (report.get("symbol", ""), target_va) in companion_witnesses[caller_key]
                    if (len(raw_hits) <= 1 or not (direct_witness or companion_witness)):
                        call_witnesses_only = False
                        break
                if not call_witnesses_only:
                    continue
            for reloc in func.relocs:
                symbol = coff.sym_by_index.get(reloc.symbol_index)
                if (symbol is None or symbol.name not in function_defs
                        or reloc.typ not in (0x0006, 0x0014)
                        or reloc.typ == 0x0006 and not symbol.is_function):
                    continue
                if len(function_defs[symbol.name]) != 1:
                    continue
                definition = function_defs[symbol.name][0]
                raw_hits = sorted(set(candidates_for(definition, exe_text)))
                if not raw_hits:
                    continue
                target = relocation_targets(reloc, func.section, location[0], exe_text, text_rva,
                                            image_base, func.start)
                if (target is not None and target[2] == 0
                        and target[1] in oracle_function_vas
                        and target[1] - image_base - text_rva in raw_hits):
                    call_observations[symbol.name].append((caller_key, target[1]))
    for name, observations in call_observations.items():
        defs = function_defs.get(name, [])
        if len(defs) != 1:
            continue
        definition = defs[0]
        # Resolve the object owner by symbol identity, not basename.
        owner = next((obj_path for obj_path, coff, _funcs, _hits in loaded
                      if any(symbol is definition.symbol for symbol in coff.symbols)), None)
        if owner is None:
            continue
        key = (str(owner.resolve()), definition.symbol.index)
        if key in locations:
            continue
        targets = {target for _caller, target in observations}
        callers = {caller for caller, _target in observations}
        if len(targets) != 1 or not callers:
            continue
        target_va = next(iter(targets))
        target_offset = target_va - image_base - text_rva
        raw_hits = candidates_for(definition, exe_text)
        if (target_va in oracle_function_vas and target_offset in raw_hits):
            locations[key] = (target_offset, "placed by verified references")
    folded: dict[tuple[str, int], dict[str, Any]] = {}
    coff_by_key = {str(obj_path.resolve()): coff for obj_path, coff, _funcs, _hits in loaded}
    hits_by_key = {(str(obj_path.resolve()), func.symbol.index): hits_by_index[func.symbol.index]
                   for obj_path, _coff, funcs, hits_by_index in loaded for func in funcs}
    for name, observations in call_observations.items():
        defs = function_defs.get(name, [])
        if len(defs) != 1:
            continue
        definition = defs[0]
        key = next((candidate_key for candidate_key, function in function_by_key.items()
                    if function is definition), None)
        if key is None:
            continue
        targets = {target for _caller, target in observations}
        callers = {caller for caller, _target in observations}
        if len(targets) != 1 or not callers:
            continue
        target_va = next(iter(targets))
        offset = target_va - image_base - text_rva
        location = locations.get(key)
        if (target_va not in oracle_function_vas or location is None or location[0] != offset
                or offset not in hits_by_key.get(key, [])):
            continue
        source_func = function_by_key[key]
        source_obj_tu = object_tu_by_key[key]
        source_coff = coff_by_key[key[0]]
        source_relocs = [(r.offset - source_func.start, r.typ) for r in source_func.relocs]
        for owner_key, owner_func in function_by_key.items():
            if owner_key == key or object_tu_by_key[owner_key] == source_obj_tu:
                continue
            owner_location = locations.get(owner_key)
            if (owner_location is None or owner_location[0] != offset
                    or owner_location[1] not in ("unique", "placed by TU order")
                    or owner_func.data != source_func.data
                    or [(r.offset - owner_func.start, r.typ) for r in owner_func.relocs] != source_relocs):
                continue
            owner_coff = coff_by_key[owner_key[0]]
            relocations_agree = True
            for source_reloc, owner_reloc in zip(source_func.relocs, owner_func.relocs):
                left = relocation_targets(source_reloc, source_func.section, offset, exe_text, text_rva,
                                          image_base, source_func.start)
                right = relocation_targets(owner_reloc, owner_func.section, offset, exe_text, text_rva,
                                           image_base, owner_func.start)
                if left is None or right is None or left[0] != right[0]:
                    relocations_agree = False
                    break
            if not relocations_agree:
                continue
            owner_tu = f"src/{object_tu_by_key[owner_key]}.cpp"
            folded[key] = {"symbol": owner_func.name, "rva": f"0x{text_rva + offset:08x}",
                           "tu": owner_tu, "owner_object_present": True,
                           "proof": f"{len(callers)} verified caller objects bind here; identical body "
                                    f"independently located in {owner_tu}"}
            location_issues.pop(key, None)
            break
    for obj_path, _coff, funcs, _hits_by_index in loaded:
        for func in funcs:
            location = locations.get((str(obj_path.resolve()), func.symbol.index))
            if location is not None:
                candidates[func.name].append(location[0])
    candidates = {name: sorted(set(values)) for name, values in candidates.items()}

    global_symbols = bind_observed_symbols(loaded, locations, exe_text, text_rva, image_base, pe)
    data_payload_bindings = bind_unique_data_symbols(data_defs, pe, global_symbols)
    return (candidates, global_symbols, dict(function_defs), dict(data_defs), locations, folded,
            location_issues, data_payload_bindings)


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("objects", nargs="+", type=Path)
    parser.add_argument("--evidence-object", action="append", default=[], type=Path,
                        help="supplemental CRT object used only to prove a symbol binding")
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
    object_paths = [path.resolve() for path in args.objects]
    evidence_paths = [path.resolve() for path in args.evidence_object]
    all_paths = object_paths + evidence_paths
    (shared_candidates, global_symbols, function_defs, data_defs, locations, folded,
     location_issues, data_payload_bindings) = collect_global_state(
        all_paths, pe, exe_text, text_rva, at, explicit)
    if data_payload_bindings:
        print(f"unique initialized-data payload bindings: {len(data_payload_bindings)}")
    crt_data_evidence = derive_crt_data_evidence(
        evidence_paths, locations, global_symbols, pe, exe_text, text_rva,
        pe.OPTIONAL_HEADER.ImageBase)
    oracle_function_vas = {pe.OPTIONAL_HEADER.ImageBase + rva for rva in oracle_function_starts()}
    all_results = []
    for obj_path in object_paths:
        result = compare_object(obj_path, pe, exe_text, text_rva, imports, explicit, at, not args.summary,
                                shared_candidates, global_symbols, function_defs, data_defs,
                                oracle_function_vas, locations, folded, location_issues,
                                crt_data_evidence)
        result["layout_warnings"] = layout_warnings(obj_path, locations, text_rva, folded)
        coff = Coff(obj_path)
        used_data_symbols = {
            symbol.name
            for section in coff.sections
            for reloc in section.relocations
            if (symbol := coff.sym_by_index.get(reloc.symbol_index)) is not None
        }
        result["data_payload_bindings"] = {
            name: detail for name, detail in data_payload_bindings.items()
            if name in used_data_symbols
        }
        all_results.append(result)
        counts = Counter()
        placement_counts = Counter()
        wildcard = 0
        exact_bytes = 0
        for func in result["functions"]:
            placement_counts[func["placement_basis"]] += 1
            if func["wildcard_hit_count"]:
                wildcard += 1
                exact_bytes += int(func["raw_exact_hit_count"] > 0)
            counts[func["status"]] += 1
        print(f"{obj_path.name}: functions={len(result['functions'])} wildcard-hit={wildcard} exact-byte-functions={exact_bytes} "
              f"strict={counts['STRICT']} folded={counts['FOLDED']} reloc-unverified={counts['RELOC-UNVERIFIED']} mismatch={counts['MISMATCH']} "
              f"locations(unique={placement_counts['unique']}, placed by TU order={placement_counts['placed by TU order']}, "
              f"placed by verified references={placement_counts['placed by verified references']}, "
              f"forced={placement_counts['forced by --at']}, ambiguous={placement_counts['ambiguous']})")
        for warning in result["layout_warnings"]:
            print(f"  layout warning: {warning}")
        if not args.summary:
            for func in result["functions"]:
                statuses = ",".join(f"{m['rva']}:{m['status']}[{m.get('placement_basis', '?')}]"
                                     for m in func["matches"]) or "no hit"
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
