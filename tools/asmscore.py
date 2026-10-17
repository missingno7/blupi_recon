#!/usr/bin/env python3
"""Relocation-aware instruction scoring for one VC5 COFF function."""

from __future__ import annotations

import difflib
import sys
from pathlib import Path
from typing import Any

import capstone
import pefile

ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / "tools"))
import cmpfunc  # noqa: E402


def oracle_text(pe: pefile.PE) -> tuple[bytes, int]:
    section = next((s for s in pe.sections if s.Name.rstrip(b"\0") == b".text"), None)
    if section is None:
        raise ValueError("oracle has no .text section")
    return section.get_data()[:section.SizeOfRawData], int(section.VirtualAddress)


def function_spans() -> dict[int, int]:
    import csv

    result: dict[int, int] = {}
    with (ROOT / "oracle" / "functions.csv").open("r", encoding="utf-8-sig", newline="") as stream:
        for row in csv.DictReader(stream):
            if row.get("rva") and row.get("size"):
                result[int(row["rva"], 0)] = int(row["size"], 0)
    starts = sorted(result)
    return {rva: max(size, starts[i + 1] - rva if i + 1 < len(starts) else size)
            for i, (rva, size) in enumerate(sorted(result.items()))}


def _operand_tuple(insn: capstone.CsInsn, operand: Any, relocation: int | None,
                   branch_base: int, branch_limit: int) -> tuple[Any, ...]:
    x86 = capstone.x86_const
    if operand.type == x86.X86_OP_REG:
        return ("reg", insn.reg_name(operand.reg))
    if operand.type == x86.X86_OP_IMM:
        value = int(operand.imm)
        if relocation is not None:
            return ("imm-reloc", relocation)
        if insn.group(capstone.CS_GRP_JUMP) or insn.group(capstone.CS_GRP_CALL):
            if branch_base <= value <= branch_limit:
                value -= branch_base
        return ("imm", value)
    if operand.type == x86.X86_OP_MEM:
        mem = operand.mem
        displacement: Any = ("disp-reloc", relocation) if relocation is not None else int(mem.disp)
        return ("mem", insn.reg_name(mem.segment) if mem.segment else "", insn.reg_name(mem.base) if mem.base else "",
                insn.reg_name(mem.index) if mem.index else "", int(mem.scale), displacement)
    return ("other", int(operand.type))


def instruction_tuples(data: bytes, address: int, relocations: list[tuple[int, int, int]],
                       function_base: int, function_end: int) -> list[tuple[Any, ...]]:
    md = capstone.Cs(capstone.CS_ARCH_X86, capstone.CS_MODE_32)
    md.detail = True
    result: list[tuple[Any, ...]] = []
    relocations = sorted(relocations)
    for insn in md.disasm(data, address):
        fields: list[tuple[int, int, int]] = []
        if insn.encoding.imm_size:
            fields.append((insn.address - address + insn.encoding.imm_offset,
                           insn.encoding.imm_size, -1))
        if insn.encoding.disp_size:
            fields.append((insn.address - address + insn.encoding.disp_offset,
                           insn.encoding.disp_size, -2))
        operands = []
        for op_index, operand in enumerate(insn.operands):
            kind = None
            for field_start, field_size, _field_kind in fields:
                kind = next((typ for reloc_at, reloc_size, typ in relocations
                             if field_start < reloc_at + reloc_size and reloc_at < field_start + field_size), None)
                if kind is not None:
                    break
            operands.append(_operand_tuple(insn, operand, kind, function_base, function_end))
        result.append((insn.mnemonic.lower(), tuple(operands)))
    return result


def score_function(obj_path: Path, decorated: str, rva: int) -> dict[str, Any]:
    """Return comparator status and an instruction-edit score for a COFF function."""
    pe = pefile.PE(str(ROOT / "assets" / "Speedy Blupi" / "BLUPI.EXE"), fast_load=False)
    text, text_rva = oracle_text(pe)
    coff = cmpfunc.Coff(obj_path)
    matches = [func for func in coff.functions() if func.name == decorated]
    if len(matches) != 1:
        raise ValueError(f"expected one COFF function named {decorated}, found {len(matches)}")
    func = matches[0]
    explicit: dict[str, int] = {}
    at = {decorated: rva}
    base = rva - text_rva
    target = text[base:base + len(func.data)]
    masked = [False] * len(func.data)
    for reloc in func.relocs:
        left = reloc.offset - func.start
        size = cmpfunc.relocation_size(reloc.typ)
        for pos in range(max(0, left), min(len(masked), left + size)):
            masked[pos] = True
    fixed_equal = len(target) == len(func.data) and cmpfunc.is_byte_match(func.data, target, masked)
    if fixed_equal:
        (shared, globals_, function_defs, data_defs, locations, folded,
         location_issues, _payloads) = cmpfunc.collect_global_state([obj_path], pe, text, text_rva, at, explicit)
        starts = {pe.OPTIONAL_HEADER.ImageBase + item for item in cmpfunc.oracle_function_starts()}
        compared = cmpfunc.compare_object(obj_path, pe, text, text_rva, cmpfunc.import_map(pe), explicit, at,
                                          False, shared, globals_, function_defs, data_defs, starts,
                                          locations, folded, location_issues)
        record = next(item for item in compared["functions"] if item["name"] == decorated)
        match = record["matches"][0] if record["matches"] else {}
        status = str(record["status"])
    else:
        # cmpfunc returns immediately with MISMATCH when any fixed byte differs.
        # Avoid its run-wide symbol-binding pass for these common search trials.
        match = {}
        status = "MISMATCH"
    oracle_size = function_spans().get(rva, len(func.data))
    oracle_bytes = text[rva - text_rva:rva - text_rva + oracle_size]
    address = int(pe.OPTIONAL_HEADER.ImageBase) + rva
    relocs = [(reloc.offset - func.start, cmpfunc.relocation_size(reloc.typ), reloc.typ)
              for reloc in func.relocs]
    left = instruction_tuples(func.data, address, relocs, address, address + len(func.data))
    # Relocation slots are projected at the same byte positions on the oracle side.
    right = instruction_tuples(oracle_bytes, address, relocs, address, address + len(oracle_bytes))
    # Dense UI handlers contain thousands of repeated loads/stores. difflib's
    # popular-token handling keeps their alignment bounded while retaining the
    # exact-prefix and local instruction runs used to rank source variants.
    matcher = difflib.SequenceMatcher(a=left, b=right,
                                      autojunk=(len(left) > 200 or len(right) > 200))
    exact = sum(block.size for block in matcher.get_matching_blocks())
    prefix = 0
    for a, b in zip(left, right):
        if a != b:
            break
        prefix += 1
    edits = len(left) + len(right) - 2 * exact
    size_delta = abs(len(func.data) - len(oracle_bytes))
    # A verification point keeps identical normalized streams with unresolved or
    # wrong relocations above STRICT, matching cmpfunc's acceptance boundary.
    verification_penalty = 0 if status == "STRICT" else 1
    score = 2 * edits + size_delta + verification_penalty
    if status == "STRICT" and score != 0:
        raise ValueError(f"STRICT byte/relocation match normalized to a nonzero score ({score})")
    return {
        "symbol": decorated,
        "rva": f"0x{rva:08x}",
        "status": status,
        "score": score,
        "instruction_edits": edits,
        "size_delta": size_delta,
        "verification_penalty": verification_penalty,
        "candidate_instructions": len(left),
        "oracle_instructions": len(right),
        "exact_matching_instructions": exact,
        "identical_prefix": prefix,
        "relocation_count": len(func.relocs),
        "relocations": match.get("relocations", []),
        "candidate_size": len(func.data),
        "oracle_size": len(oracle_bytes),
    }

