#!/usr/bin/env python3
"""Run isolated MSVC 5.x listing, counter, compiler, and instruction probes.

The driver uses this repository's tools/msvc.py for compiler-tree resolution
and tools/cmpfunc.py for relocation-aware strict PE comparisons. See the KB
README for limitations and the standalone extraction note.
"""

from __future__ import annotations

import argparse
from collections import Counter
import hashlib
import json
import os
from pathlib import Path
import re
import shutil
import subprocess
import sys
from typing import Any

import capstone
import pefile

ROOT = Path(__file__).resolve().parents[2]
sys.path.insert(0, str(ROOT / "tools"))
import msvc  # repository-local environment and compiler-tree resolver


def digest(path: Path) -> str:
    h = hashlib.sha256()
    with path.open("rb") as stream:
        for block in iter(lambda: stream.read(1024 * 1024), b""):
            h.update(block)
    return h.hexdigest()


def safe_output(path: Path) -> Path:
    resolved = path.resolve()
    allowed = ((ROOT / "build" / "kb").resolve(),
               (ROOT / "scratch" / "kb" / "merge").resolve())
    if not any(resolved == base or base in resolved.parents for base in allowed):
        raise ValueError(f"output must be inside build/kb or scratch/kb/merge: {resolved}")
    resolved.mkdir(parents=True, exist_ok=True)
    return resolved


def tree_layout(tree_name: str) -> tuple[Path, Path, Path | None, list[Path], dict[str, str]]:
    tree = msvc.resolve_tree(tree_name).resolve()
    compiler_bin, vc_root, shared_bin, default_includes = msvc.tool_layout(tree, None)
    temp = ROOT / "build" / "tmp"
    temp.mkdir(parents=True, exist_ok=True)
    system_root = Path(os.environ.get("SystemRoot", r"C:\Windows"))
    path_dirs = [compiler_bin]
    if shared_bin and shared_bin.is_dir():
        path_dirs.append(shared_bin)
    path_dirs += [system_root / "System32", system_root]
    env = {
        "PATH": os.pathsep.join(map(str, path_dirs)),
        "INCLUDE": os.pathsep.join(map(str, default_includes)),
        "LIB": str(vc_root / "LIB"),
        "TMP": str(temp),
        "TEMP": str(temp),
        "SystemRoot": str(system_root),
    }
    return compiler_bin, vc_root, shared_bin, default_includes, env


def compile_tu(source: Path, obj: Path, tree: str, flags: list[str],
               includes: list[Path], defines: list[str], listing: Path | None = None) -> dict[str, Any]:
    compiler_bin, vc_root, _, default_includes, env = tree_layout(tree)
    resolved_tree = msvc.resolve_tree(tree).resolve()
    obj.parent.mkdir(parents=True, exist_ok=True)
    source = source.resolve()
    # The original TU directory is searched for quoted headers even when the
    # probe copy itself lives under scratch/.
    search_dirs = [source.parent] + [p.resolve() for p in includes]
    command = [str(compiler_bin / "CL.EXE"), "/nologo", "/c", str(source), f"/Fo{obj.resolve()}"]
    command += [f"/I{p}" for p in search_dirs]
    command += [f"/D{x}" for x in defines]
    if listing:
        listing.parent.mkdir(parents=True, exist_ok=True)
        command += ["/FAs", f"/Fa{listing.resolve()}"]
    command += flags
    run = subprocess.run(command, cwd=ROOT, env=env, stdout=subprocess.PIPE,
                         stderr=subprocess.STDOUT, text=True, encoding="mbcs", errors="replace")
    banner = subprocess.run([str(compiler_bin / "CL.EXE")], cwd=ROOT, env=env,
                            stdout=subprocess.PIPE, stderr=subprocess.STDOUT, text=True,
                            encoding="mbcs", errors="replace").stdout
    banner_line = next((line.strip() for line in banner.splitlines() if line.strip()), "unknown")
    roots = [compiler_bin, vc_root / "BIN", resolved_tree]
    components: dict[str, dict[str, str] | None] = {}
    for candidates in (("c1xx.dll",), ("c2.exe", "c2.dll")):
        path = next((root / name for root in roots for name in candidates if (root / name).is_file()), None)
        if path is None:
            path = next((p for name in candidates for p in resolved_tree.rglob(name) if p.is_file()), None)
        key = "/".join(candidates)
        components[key] = ({"path": str(path.resolve()), "sha256": digest(path)} if path else None)
    return {"command": subprocess.list2cmdline(command), "exit_code": run.returncode,
            "output": run.stdout, "compiler": str(compiler_bin / "CL.EXE"),
            "compiler_banner": banner_line, "compiler_tree": str(resolved_tree),
            "compiler_components": components,
            "default_include_dirs": list(map(str, default_includes)),
            "default_library_dir": str(vc_root / "LIB"),
            "cl_sha256": digest(compiler_bin / "CL.EXE") if (compiler_bin / "CL.EXE").is_file() else None}


def parse_counter_listing(path: Path) -> dict[str, Any]:
    text = path.read_text(encoding="mbcs", errors="replace")
    proc_re = re.compile(r"^\s*([^\s;]+)\s+PROC\b", re.IGNORECASE)
    end_re = re.compile(r"^\s*([^\s;]+)\s+ENDP\b", re.IGNORECASE)
    counter_re = re.compile(r"\$([LT])([0-9A-Fa-f]{1,8})\b")
    functions: list[dict[str, Any]] = []
    current: dict[str, Any] | None = None
    unscoped: dict[str, list[int]] = {"L": [], "T": []}
    for number, line in enumerate(text.splitlines(), 1):
        start = proc_re.match(line)
        end = end_re.match(line)
        if start:
            current = {"symbol": start.group(1), "first_line": number,
                       "last_line": number, "L": [], "T": []}
            functions.append(current)
        values = [(kind, int(value, 16)) for kind, value in counter_re.findall(line)]
        if current is not None:
            current["last_line"] = number
            for kind, value in values:
                current[kind].append(value)
        else:
            for kind, value in values:
                unscoped[kind].append(value)
        if end and current is not None:
            current["last_line"] = number
            current = None
    for function in functions:
        for kind in ("L", "T"):
            vals = sorted(set(function.pop(kind)))
            function[f"${kind}_values"] = vals
            function[f"${kind}_range"] = [vals[0], vals[-1]] if vals else None
            function[f"${kind}_range_hex"] = [f"0x{vals[0]:x}", f"0x{vals[-1]:x}"] if vals else None
    return {"listing": str(path), "functions": functions,
            "unscoped": {f"${k}_values": sorted(set(v)) for k, v in unscoped.items()},
            "note": "Ranges summarize labels visible inside PROC/ENDP blocks; compiler listing formats can leave labels unscoped."}


def add_fillers(source: Path, destination: Path, count: int, position: str, anchor: str | None) -> None:
    raw = source.read_bytes()
    bom = b"\xef\xbb\xbf" if raw.startswith(b"\xef\xbb\xbf") else b""
    payload = raw[len(bom):]
    if bom:
        encoding = "utf-8"
    else:
        try:
            payload.decode("utf-8")
            encoding = "utf-8"
        except UnicodeDecodeError:
            encoding = "cp1252"
    body = payload.decode(encoding)
    newline = "\r\n" if "\r\n" in body else "\n"
    decls = newline.join(f"extern int vc5probe_filler_{i:06d};" for i in range(count))
    if count:
        decls += newline
    if position == "top":
        body = decls + body
    elif position == "end":
        if body and not body.endswith(("\n", "\r")):
            body += newline
        body += decls
    elif position == "before":
        assert anchor
        rows = body.splitlines(keepends=True)
        candidates: list[int] = []
        for i, row in enumerate(rows):
            if row.lstrip().startswith("#") or not re.search(rf"\b{re.escape(anchor)}\s*\(", row):
                continue
            accumulated = row
            j = i
            while j + 1 < len(rows) and "{" not in accumulated and ";" not in accumulated and j - i < 12:
                j += 1
                accumulated += rows[j]
            brace = accumulated.find("{")
            semi = accumulated.find(";")
            if brace >= 0 and (semi < 0 or brace < semi):
                candidates.append(i)
        if len(candidates) != 1:
            raise ValueError(f"before:{anchor} needs one recognizable function definition; found {len(candidates)}")
        rows.insert(candidates[0], decls)
        body = "".join(rows)
    else:
        raise ValueError(f"unsupported insertion position: {position}")
    destination.parent.mkdir(parents=True, exist_ok=True)
    destination.write_bytes(bom + body.encode(encoding))


def object_function(obj: Path, symbol: str):
    from cmpfunc import Coff
    coff = Coff(obj)
    matches = [func for func in coff.functions() if func.name == symbol]
    if len(matches) != 1:
        raise ValueError(f"object has {len(matches)} functions named {symbol!r}")
    return matches[0]


def target_bytes(args: argparse.Namespace, length: int) -> bytes:
    if args.target_bytes:
        data = args.target_bytes.read_bytes()
        if args.target_length is not None and len(data) != args.target_length:
            raise ValueError(f"target byte file has {len(data)} bytes; --target-length is {args.target_length}")
        return data
    if not args.target_pe or args.rva is None:
        raise ValueError("supply --target-pe with --rva, or --target-bytes")
    pe = pefile.PE(str(args.target_pe), fast_load=False)
    section = next((s for s in pe.sections if s.VirtualAddress <= args.rva <
                    s.VirtualAddress + max(s.Misc_VirtualSize, s.SizeOfRawData)), None)
    if section is None:
        raise ValueError(f"RVA 0x{args.rva:x} is not inside a PE section")
    size = args.target_length if args.target_length is not None else length
    if args.rva + size > section.VirtualAddress + max(section.Misc_VirtualSize, section.SizeOfRawData):
        raise ValueError("requested target range exceeds its PE section")
    data = pe.get_data(args.rva, size)
    if len(data) != size:
        raise ValueError(f"PE returned {len(data)} target bytes; expected {size}")
    return data


def compare_candidate(obj: Path, args: argparse.Namespace, report_dir: Path) -> dict[str, Any]:
    function = object_function(obj, args.function)
    if args.target_bytes:
        expected = target_bytes(args, len(function.data))
        status = "STRICT_BYTES" if function.data == expected else "MISMATCH"
        return {"symbol": args.function, "status": status, "candidate_size": len(function.data),
                "target_size": len(expected), "byte_equal": function.data == expected,
                "relocation_count": len(function.relocs),
                "strictness": "raw complete-byte equality; relocation values are not masked"}
    if not args.target_pe or args.rva is None:
        raise ValueError("probe/sweep requires --target-pe with --rva, or --target-bytes")
    result_path = report_dir / (obj.stem + ".cmp.json")
    command = [sys.executable, str(ROOT / "tools" / "cmpfunc.py"), str(obj),
               "--exe", str(args.target_pe), "--at", f"{args.function}={args.rva:#x}",
               "--summary", "--json-out", str(result_path)]
    for mapping in args.symbol_rva:
        if "=" not in mapping:
            raise ValueError("--symbol-rva syntax is DECORATED_SYMBOL=RVA")
        command += ["--at", mapping]
    for mapping in args.symbol_va:
        if "=" not in mapping:
            raise ValueError("--symbol-va syntax is SYMBOL=VA")
        command += ["--symbol-va", mapping]
    run = subprocess.run(command, cwd=ROOT, stdout=subprocess.PIPE, stderr=subprocess.STDOUT,
                         text=True, encoding="utf-8", errors="replace")
    if run.returncode:
        raise RuntimeError(f"cmpfunc failed: {run.stdout}")
    parsed = json.loads(result_path.read_text(encoding="utf-8"))
    all_functions = [f for unit in parsed for f in unit["functions"]]
    matches = [f for f in all_functions if f["name"] == args.function]
    if len(matches) != 1:
        raise ValueError(f"cmpfunc found {len(matches)} functions named {args.function!r}")
    match = matches[0]
    status = match["status"]
    size_check = None
    if args.target_length is not None:
        size_check = match["code_size"] == args.target_length
        if not size_check and status == "STRICT":
            status = "SIZE_MISMATCH"
    target_rva_ok = (len(match["matches"]) == 1 and
                     int(match["matches"][0]["rva"], 16) == args.rva)
    per_function = [{"symbol": f["name"], "status": f["status"],
                     "candidate_size": f["code_size"], "wildcard_hits": f["wildcard_hit_count"],
                     "rvas": [m.get("rva") for m in f["matches"]]}
                    for f in all_functions]
    summary = {key: sum(f["status"] == key for f in all_functions)
               for key in ("STRICT", "RELOC-UNVERIFIED", "MISMATCH")}
    return {"symbol": args.function, "status": status, "candidate_size": match["code_size"],
            "target_length": args.target_length, "target_size_equal": size_check,
            "target_rva": f"0x{args.rva:x}", "target_rva_equal": target_rva_ok,
            "relocation_count": match["relocation_count"],
            "translation_unit_functions": len(all_functions), "strict_status_counts": summary,
            "per_function": per_function,
            "strictness": "tools/cmpfunc.py forced-RVA strict relocation and binding checks; peer functions use its unique-location rules"}


def disassembly(data: bytes, address: int = 0) -> list[tuple[str, str]]:
    md = capstone.Cs(capstone.CS_ARCH_X86, capstone.CS_MODE_32)
    return [(ins.mnemonic, ins.op_str) for ins in md.disasm(data, address)]


REGEX_REG = re.compile(r"\b(eax|ebx|ecx|edx|esi|edi|ebp|esp|ax|bx|cx|dx|si|di|bp|sp|al|ah|bl|bh|cl|ch|dl|dh)\b", re.I)


def sorted_operands(rows: list[tuple[str, str]]) -> list[tuple[str, str]]:
    return [(mnemonic, ",".join(sorted(x.strip() for x in operands.split(","))))
            for mnemonic, operands in rows]


def register_renaming_only(left: list[tuple[str, str]], right: list[tuple[str, str]]) -> bool:
    if len(left) != len(right):
        return False
    forward: dict[str, str] = {}
    reverse: dict[str, str] = {}
    changed = False
    for (lm, lo), (rm, ro) in zip(left, right):
        if lm != rm:
            return False
        lregs = [m.group(0).lower() for m in REGEX_REG.finditer(lo.lower())]
        rregs = [m.group(0).lower() for m in REGEX_REG.finditer(ro.lower())]
        if len(lregs) != len(rregs):
            return False
        lshape = REGEX_REG.sub("{r}", lo.lower())
        rshape = REGEX_REG.sub("{r}", ro.lower())
        if lshape != rshape:
            return False
        for lreg, rreg in zip(lregs, rregs):
            lwidth = "8" if lreg in {"al", "ah", "bl", "bh", "cl", "ch", "dl", "dh"} else ("16" if lreg in {"ax", "bx", "cx", "dx", "si", "di", "bp", "sp"} else "32")
            rwidth = "8" if rreg in {"al", "ah", "bl", "bh", "cl", "ch", "dl", "dh"} else ("16" if rreg in {"ax", "bx", "cx", "dx", "si", "di", "bp", "sp"} else "32")
            if lwidth != rwidth or forward.get(lreg, rreg) != rreg or reverse.get(rreg, lreg) != lreg:
                return False
            forward[lreg] = rreg
            reverse[rreg] = lreg
            if lreg != rreg:
                changed = True
    return changed


def canonical_sib(rows: list[tuple[str, str]]) -> list[tuple[str, str]]:
    out = []
    for mnemonic, operands in rows:
        def mem(match: re.Match[str]) -> str:
            expr = match.group(1)
            terms = [x.strip() for x in re.split(r"\s*\+\s*", expr)]
            if len(terms) == 2 and all(REGEX_REG.fullmatch(t) for t in terms):
                return "[" + "+".join(sorted(t.lower() for t in terms)) + "]"
            return match.group(0).lower()
        out.append((mnemonic, re.sub(r"\[([^\]]+)\]", mem, operands.lower())))
    return out


def has_reversed_sib(left: list[tuple[str, str]], right: list[tuple[str, str]]) -> bool:
    for (_, lo), (_, ro) in zip(left, right):
        left_terms = re.findall(r"\[([^\]]+)\]", lo.lower())
        right_terms = re.findall(r"\[([^\]]+)\]", ro.lower())
        if len(left_terms) != len(right_terms):
            continue
        for a, b in zip(left_terms, right_terms):
            aa = [x.strip() for x in re.split(r"\s*\+\s*", a)]
            bb = [x.strip() for x in re.split(r"\s*\+\s*", b)]
            if (len(aa) == len(bb) == 2 and all(REGEX_REG.fullmatch(x) for x in aa + bb)
                    and aa == list(reversed(bb)) and aa != bb):
                return True
    return False


def classify_diff(candidate: bytes, target: bytes) -> dict[str, Any]:
    left = disassembly(candidate)
    right = disassembly(target)
    if candidate == target:
        kind = "byte-identical"
    elif (len(left) == len(right) and [x[0] for x in left] == [x[0] for x in right]
          and canonical_sib(left) == canonical_sib(right) and has_reversed_sib(left, right)):
        kind = "SIB-order"
    elif Counter((m, re.sub(r"\s+", " ", o.lower()).strip()) for m, o in left) == \
         Counter((m, re.sub(r"\s+", " ", o.lower()).strip()) for m, o in right):
        kind = "scheduling-same-instruction-multiset"
    elif (len(left) == len(right) and [x[0] for x in left] == [x[0] for x in right]
          and sorted_operands(left) == sorted_operands(right)):
        kind = "operand-order"
    elif register_renaming_only(left, right):
        kind = "register-renaming-only"
    else:
        kind = "size-or-structural"
    return {"class": kind, "candidate_bytes": len(candidate), "target_bytes": len(target),
            "candidate_instructions": len(left), "target_instructions": len(right),
            "candidate": [f"{m} {o}".strip() for m, o in left],
            "target": [f"{m} {o}".strip() for m, o in right],
            "note": "Instruction classification is heuristic; only byte equality or cmpfunc strict status proves a match. Relocation fields in COFF are unresolved during this view."}


def add_common(parser: argparse.ArgumentParser, target: bool = False) -> None:
    parser.add_argument("--tu", required=True, type=Path, help="one C/C++ translation unit")
    parser.add_argument("--tree", default="sp3", help="compiler tree alias or path")
    parser.add_argument("--include", action="append", default=[], type=Path)
    parser.add_argument("--define", action="append", default=[])
    parser.add_argument("--flag", action="append", default=[], help="one cl.exe argument; repeat")
    parser.add_argument("--function", required=True, help="decorated COFF function symbol")
    parser.add_argument("--target-pe", type=Path)
    parser.add_argument("--target-bytes", type=Path, help="file containing the exact reference function bytes")
    parser.add_argument("--rva", type=lambda x: int(x, 0), help="target PE RVA (hex or decimal)")
    parser.add_argument("--target-length", type=int, help="known full target function length")
    parser.add_argument("--symbol-rva", action="append", default=[], metavar="SYMBOL=RVA",
                        help="independent strict mapping for a referenced peer function; repeatable")
    parser.add_argument("--symbol-va", action="append", default=[], metavar="SYMBOL=VA",
                        help="independently verified VA for a relocation target; repeatable")
    parser.add_argument("--output-dir", type=Path, default=ROOT / "build" / "kb" / "vc5probe")


def do_listing(args: argparse.Namespace) -> int:
    out = safe_output(args.output_dir)
    stem = args.tu.stem
    obj, listing = out / f"{stem}.obj", out / f"{stem}.asm"
    info = compile_tu(args.tu, obj, args.tree, args.flag, args.include, args.define, listing)
    if info["exit_code"]:
        raise RuntimeError(info["output"])
    parsed = parse_counter_listing(listing)
    report = {"input": str(args.tu.resolve()), "input_sha256": digest(args.tu),
              "flags": args.flag, "defines": args.define, "includes": list(map(str, args.include)),
              "compile": {k: v for k, v in info.items() if k != "output"}, "counters": parsed}
    path = out / f"{stem}.listing.json"
    path.write_text(json.dumps(report, indent=2) + "\n", encoding="utf-8")
    print(json.dumps({"listing": str(listing), "report": str(path),
                      "function_ranges": len(parsed["functions"]), "compile": info["compiler_banner"]}, indent=2))
    return 0


def do_probe(args: argparse.Namespace) -> int:
    out = safe_output(args.output_dir)
    function_slug = re.sub(r"[^A-Za-z0-9_-]+", "_", args.function).strip("_")[:48]
    # Reuse one source pathname for every count and placement. A changing
    # pathname can itself alter code through __FILE__ or diagnostic macros.
    probe_root = safe_output(ROOT / "scratch" / "kb" / "merge" / "vc5probe" /
                             "sources" / function_slug)
    if args.target_pe is None and args.target_bytes is None:
        raise ValueError("probe needs --target-pe/--rva or --target-bytes")
    if args.target_pe and args.rva is None:
        raise ValueError("--target-pe requires --rva")
    if args.target_bytes and args.rva is not None:
        raise ValueError("choose --target-pe/--rva or --target-bytes")
    if args.target_pe and not args.target_length:
        print("note: pass --target-length to verify the full reference function boundary", file=sys.stderr)
    if args.position.startswith("before:"):
        position, anchor = "before", args.position.split(":", 1)[1]
    else:
        position, anchor = args.position, None
    counts = args.count if args.count else list(range(0, args.max_count + 1, args.step))
    rows = []
    src = args.tu.resolve()
    for n in counts:
        copy = probe_root / src.name
        add_fillers(src, copy, n, position, anchor)
        obj = out / f"{src.stem}_{function_slug}_{position}_{n}.obj"
        info = compile_tu(copy, obj, args.tree, args.flag, args.include, args.define)
        if info["exit_code"]:
            row = {"count": n, "status": "COMPILE_ERROR", "output": info["output"], "command": info["command"]}
        else:
            result = compare_candidate(obj, args, out)
            row = {"count": n, **result, "command": info["command"],
                   "compiler_banner": info["compiler_banner"],
                   "compiler": info["compiler"], "compiler_tree": info["compiler_tree"],
                   "compiler_components": info["compiler_components"]}
        rows.append(row)
        counts_summary = row.get("strict_status_counts", {})
        peer_text = (f" strict={counts_summary.get('STRICT', 0)}/"
                     f"{row.get('translation_unit_functions', 0)}" if counts_summary else "")
        print(f"n={n}: {row.get('status')} size={row.get('candidate_size', '-')}{peer_text}")
    report = {"input": str(src), "input_sha256": digest(src), "function": args.function,
              "position": args.position, "counts": counts, "tree": args.tree,
              "scratch_source_copy": str(probe_root / src.name),
              "flags": args.flag, "defines": args.define, "includes": list(map(str, args.include)),
              "target_pe": str(args.target_pe.resolve()) if args.target_pe else None,
              "target_bytes": str(args.target_bytes.resolve()) if args.target_bytes else None,
              "rva": args.rva, "target_length": args.target_length, "results": rows}
    path = out / f"{src.stem}_{function_slug}_{position}_probe.json"
    path.write_text(json.dumps(report, indent=2) + "\n", encoding="utf-8")
    print(f"report: {path}")
    return 0


def do_sweep(args: argparse.Namespace) -> int:
    out = safe_output(args.output_dir)
    if args.target_pe is None and args.target_bytes is None:
        raise ValueError("sweep needs --target-pe/--rva or --target-bytes")
    trees = [x for x in args.trees.split(",") if x]
    flagsets: list[tuple[str, list[str]]] = []
    for item in args.flag_set:
        if "=" not in item:
            raise ValueError("--flag-set syntax is NAME=/O2;/GX;/ML")
        name, flags = item.split("=", 1)
        flagsets.append((name, [x for x in flags.split(";") if x]))
    if not flagsets:
        flagsets = [("base", [])]
    rows = []
    source = args.tu.resolve()
    for tree in trees:
        for label, selected in flagsets:
            obj = out / f"{source.stem}_{tree}_{label}.obj"
            info = compile_tu(source, obj, tree, args.flag + selected, args.include, args.define)
            if info["exit_code"]:
                row = {"tree": tree, "flag_set": label, "status": "COMPILE_ERROR", "output": info["output"]}
            else:
                row = {"tree": tree, "flag_set": label, **compare_candidate(obj, args, out)}
            row["flags"] = args.flag + selected
            row["command"] = info["command"]
            row["compiler_banner"] = info["compiler_banner"]
            row["compiler"] = info["compiler"]
            row["compiler_tree"] = info["compiler_tree"]
            row["compiler_components"] = info["compiler_components"]
            rows.append(row)
            counts_summary = row.get("strict_status_counts", {})
            peer_text = (f" strict={counts_summary.get('STRICT', 0)}/"
                         f"{row.get('translation_unit_functions', 0)}" if counts_summary else "")
            print(f"{tree}/{label}: {row.get('status')} size={row.get('candidate_size', '-')}{peer_text}")
    report = {"input": str(source), "input_sha256": digest(source), "function": args.function,
              "trees": trees, "base_flags": args.flag, "flag_sets": args.flag_set,
              "defines": args.define, "includes": list(map(str, args.include)),
              "target_pe": str(args.target_pe.resolve()) if args.target_pe else None,
              "target_bytes": str(args.target_bytes.resolve()) if args.target_bytes else None,
              "rva": args.rva, "target_length": args.target_length, "results": rows}
    path = out / f"{source.stem}_sweep.json"
    path.write_text(json.dumps(report, indent=2) + "\n", encoding="utf-8")
    print(f"report: {path}")
    return 0


def do_diff(args: argparse.Namespace) -> int:
    out = safe_output(args.output_dir)
    obj = out / f"{args.tu.stem}_diff.obj"
    info = compile_tu(args.tu, obj, args.tree, args.flag, args.include, args.define)
    if info["exit_code"]:
        raise RuntimeError(info["output"])
    function = object_function(obj, args.function)
    expected = target_bytes(args, len(function.data))
    report = classify_diff(function.data, expected)
    report.update({"input": str(args.tu.resolve()), "function": args.function,
                   "object": str(obj), "target_rva": args.rva,
                   "target_bytes_path": str(args.target_bytes) if args.target_bytes else None,
                   "command": info["command"], "compiler_banner": info["compiler_banner"],
                   "compiler": info["compiler"], "compiler_tree": info["compiler_tree"],
                   "compiler_components": info["compiler_components"]})
    path = out / f"{args.tu.stem}_diff.json"
    path.write_text(json.dumps(report, indent=2) + "\n", encoding="utf-8")
    print(json.dumps({"class": report["class"], "report": str(path)}, indent=2))
    return 0


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    sub = parser.add_subparsers(dest="command", required=True)
    listing = sub.add_parser("listing", help="compile with /FAs and summarize per-function $L/$T counters")
    add_common(listing)
    probe = sub.add_parser("probe", help="sweep filler extern declarations in a scratch copy of one TU")
    add_common(probe, target=True)
    probe.add_argument("--position", required=True, help="top, end, or before:SOURCE_FUNCTION")
    probe.add_argument("--count", action="append", type=int, help="one exact count; repeat for selected counts")
    probe.add_argument("--max-count", type=int, default=0, help="sweep 0..max-count when --count is omitted")
    probe.add_argument("--step", type=int, default=1)
    sweep = sub.add_parser("sweep", help="compile a compiler-tree and flag matrix")
    add_common(sweep, target=True)
    sweep.add_argument("--trees", default="sp3", help="comma-separated compiler aliases/paths")
    sweep.add_argument("--flag-set", action="append", default=[], help="NAME=/O2;/GX; repeatable")
    diff = sub.add_parser("diff", help="compile and classify a candidate instruction diff")
    add_common(diff, target=True)
    args = parser.parse_args()
    if not args.tu.is_file():
        parser.error(f"TU does not exist: {args.tu}")
    if hasattr(args, "step") and args.step < 1:
        parser.error("--step must be positive")
    if hasattr(args, "max_count") and args.max_count < 0:
        parser.error("--max-count must be nonnegative")
    if hasattr(args, "target_length") and args.target_length is not None and args.target_length < 1:
        parser.error("--target-length must be positive")
    try:
        return {"listing": do_listing, "probe": do_probe, "sweep": do_sweep, "diff": do_diff}[args.command](args)
    except (OSError, ValueError, RuntimeError, KeyError) as exc:
        print(f"vc5probe: {exc}", file=sys.stderr)
        return 1


if __name__ == "__main__":
    raise SystemExit(main())
