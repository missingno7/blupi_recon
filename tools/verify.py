#!/usr/bin/env python3
"""Clean-build and strictly verify the canonical reconstructed translation units."""

from __future__ import annotations

import argparse
import hashlib
import json
import os
from pathlib import Path
import shutil
import subprocess
import sys
from typing import Any

ROOT = Path(__file__).resolve().parents[1]
MANIFEST = ROOT / "recon.json"
BUILD = ROOT / "build" / "verify"
ORACLE_TEXT_START = 0x1000
GAME_TEXT_END = 0x24140
GAME_TEXT_SIZE = GAME_TEXT_END - ORACLE_TEXT_START


def parse_number(value: int | str) -> int:
    return int(value, 0) if isinstance(value, str) else int(value)


def load_manifest() -> dict[str, Any]:
    return json.loads(MANIFEST.read_text(encoding="utf-8"))


def safe_clean_build() -> None:
    build_root = (ROOT / "build").resolve()
    target = BUILD.resolve()
    if target != build_root / "verify" or build_root not in target.parents:
        raise RuntimeError(f"refusing to clean unexpected path: {target}")
    if target.exists():
        shutil.rmtree(target)
    target.mkdir(parents=True)


def parse_substitutions(items: list[str], manifest: dict[str, Any]) -> dict[str, Path]:
    known = {unit["source"] for unit in manifest["translation_units"]}
    by_name = {Path(name).name: name for name in known}
    result: dict[str, Path] = {}
    for item in items:
        if "=" not in item:
            raise ValueError(f"--tu must be src/name.cpp=path: {item}")
        key, value = item.split("=", 1)
        canonical = key if key in known else by_name.get(key)
        if canonical is None:
            raise ValueError(f"unknown translation unit for substitution: {key}")
        source = Path(value)
        if not source.is_absolute():
            source = ROOT / source
        source = source.resolve()
        if not source.is_file():
            raise ValueError(f"candidate source does not exist: {source}")
        result[canonical] = source
    return result


def hash_file(path: Path) -> str:
    digest = hashlib.sha256()
    with path.open("rb") as stream:
        for block in iter(lambda: stream.read(1024 * 1024), b""):
            digest.update(block)
    return digest.hexdigest()


def tool_hash_paths(tree_name: str) -> tuple[Path, Path, Path]:
    sys.path.insert(0, str(ROOT / "tools"))
    import msvc

    tree = msvc.resolve_tree(tree_name).resolve()
    compiler_bin, vc_root, shared_bin, _ = msvc.tool_layout(tree, None)
    names = ("cl.exe", "c1xx.dll", "c2.exe")
    roots = [compiler_bin, vc_root / "BIN", shared_bin or compiler_bin, tree]
    found: list[Path] = []
    for name in names:
        match = next((base / name for base in roots if base and (base / name).is_file()), None)
        if match is None:
            match = next((p for p in tree.rglob(name) if p.is_file()), None)
        if match is None:
            raise FileNotFoundError(f"{name} not found under {tree}")
        found.append(match)
    return tuple(found)  # type: ignore[return-value]


def compile_units(manifest: dict[str, Any], substitutions: dict[str, Path]) -> list[tuple[str, Path]]:
    tool = manifest["toolchain"]
    obj_dir = BUILD / "objects"
    obj_dir.mkdir(parents=True)
    results: list[tuple[str, Path]] = []
    for index, unit in enumerate(manifest["translation_units"]):
        canonical = unit["source"]
        source = substitutions.get(canonical, ROOT / canonical)
        obj = obj_dir / f"{index:02d}_{Path(canonical).stem}.obj"
        command = [sys.executable, str(ROOT / "tools" / "msvc.py"), str(source),
                   "--tree", tool["tree"], "--out", str(obj), "--cwd", str(ROOT),
                   "--log-dir", str(BUILD / "logs")]
        includes = [ROOT / "src"] + [Path(p) for p in tool.get("include_dirs", [])]
        if source.parent != ROOT / "src":
            includes.insert(0, source.parent)
        for include in includes:
            command.extend(["--include", str(include)])
        for define in tool.get("defines", []):
            command.extend(["--define", define])
        for flag in tool.get("flags", []):
            command.extend(["--flag", flag])
        print(f"compile {canonical} <- {source}")
        run = subprocess.run(command, cwd=ROOT, stdout=subprocess.PIPE, stderr=subprocess.STDOUT,
                             text=True, encoding="mbcs", errors="replace")
        if run.returncode:
            print(run.stdout, end="")
            raise RuntimeError(f"compile failed for {canonical} (exit {run.returncode})")
        results.append((canonical, obj))
    return results


def run_comparator(objects: list[Path], manifest: dict[str, Any]) -> list[dict[str, Any]]:
    output = BUILD / "results.json"
    command = [sys.executable, str(ROOT / "tools" / "cmpfunc.py"), *map(str, objects),
               "--summary", "--json-out", str(output)]
    for accepted in manifest.get("accepted_functions", []):
        command.extend(["--at", f"{accepted['symbol']}={parse_number(accepted['rva']):#x}"])
    for mapping in manifest.get("symbol_mappings", []):
        command.extend(["--symbol-va", f"{mapping['symbol']}={parse_number(mapping['va']):#x}"])
    run = subprocess.run(command, cwd=ROOT, stdout=subprocess.PIPE, stderr=subprocess.STDOUT,
                         text=True, encoding="utf-8", errors="replace")
    if run.returncode:
        print(run.stdout, end="")
        raise RuntimeError(f"comparator failed (exit {run.returncode})")
    return json.loads(output.read_text(encoding="utf-8"))


def union_size(intervals: list[tuple[int, int]]) -> int:
    if not intervals:
        return 0
    intervals.sort()
    total = 0
    start, end = intervals[0]
    for next_start, next_end in intervals[1:]:
        if next_start <= end:
            end = max(end, next_end)
        else:
            total += end - start
            start, end = next_start, next_end
    return total + end - start


def report(results: list[dict[str, Any]], unit_order: list[str]) -> tuple[dict[str, list[dict[str, Any]]], int]:
    by_unit: dict[str, list[dict[str, Any]]] = {name: [] for name in unit_order}
    intervals_by_owner: dict[str, list[tuple[int, int]]] = {name: [] for name in unit_order}
    folded_owner_intervals: dict[str, list[tuple[int, int]]] = {}
    path_exceptions: set[tuple[str, str, str]] = set()
    for result, unit in zip(results, unit_order):
        by_unit[unit] = result["functions"]
        for func in result["functions"]:
            for match in func["matches"]:
                for relocation in match.get("relocations", []):
                    detail = relocation.get("build_path_difference")
                    if detail:
                        path_exceptions.add((str(detail.get("object_path")), str(detail.get("oracle_path")),
                                             str(detail.get("target_va"))))
                if func["status"] not in ("STRICT", "FOLDED") or len(func["matches"]) != 1:
                    continue
                rva = int(func["matches"][0]["rva"], 16)
                low, high = max(rva, ORACLE_TEXT_START), min(rva + func["code_size"], GAME_TEXT_END)
                if low >= high:
                    continue
                interval = (low, high)
                if func["status"] == "STRICT":
                    intervals_by_owner[unit].append(interval)
                else:
                    owner = str(func.get("folded_into", {}).get("tu", "unknown TU"))
                    owner_name = Path(owner).name.lower()
                    owner_unit = next((name for name in unit_order if Path(name).name.lower() == owner_name), None)
                    if owner_unit is not None:
                        intervals_by_owner[owner_unit].append(interval)
                    else:
                        folded_owner_intervals.setdefault(owner, []).append(interval)
    for object_path, oracle_path, target_va in sorted(path_exceptions):
        print(f"build-path data difference: object={object_path!r} oracle={oracle_path!r} target={target_va}")
    for result in results:
        for warning in result.get("layout_warnings", []):
            print(f"layout warning ({Path(result['object']).name}): {warning}")
    print("\nTU                      functions  strict  folded  unverified  mismatch  locations U/P/C/F/A  game bytes")
    print("----------------------  ---------  ------  ------  ----------  --------  ------------------  ----------")
    for unit in unit_order:
        functions = by_unit[unit]
        counts = {key: sum(f["status"] == key for f in functions)
                  for key in ("STRICT", "FOLDED", "RELOC-UNVERIFIED", "MISMATCH")}
        bases = {key: sum(f.get("placement_basis") == key for f in functions)
                 for key in ("unique", "placed by TU order", "placed by verified call targets", "forced by --at")}
        unresolved_locations = sum(f.get("placement_basis") in ("ambiguous", "unlocated") for f in functions)
        intervals = intervals_by_owner[unit]
        covered = union_size(intervals)
        location_counts = (f"{bases['unique']}/{bases['placed by TU order']}/"
                           f"{bases['placed by verified call targets']}/{bases['forced by --at']}/"
                           f"{unresolved_locations}")
        print(f"{unit:<22}  {len(functions):>9}  {counts['STRICT']:>6}  {counts['FOLDED']:>6}  "
              f"{counts['RELOC-UNVERIFIED']:>10}  {counts['MISMATCH']:>8}  "
              f"{location_counts:>16}  "
              f"{covered:>5} / {GAME_TEXT_SIZE}")
    print("Location counts U/P/C/F/A = unique / placed by TU order / placed by verified calls / forced by --at / unresolved.")
    for owner, intervals in sorted(folded_owner_intervals.items()):
        covered = union_size(intervals)
        print(f"folded owner {owner}: {covered} game .text bytes")
    total = union_size([interval for intervals in intervals_by_owner.values() for interval in intervals]
                       + [interval for intervals in folded_owner_intervals.values() for interval in intervals])
    print(f"All TUs strict plus folded-owner game .text coverage: {total} / {GAME_TEXT_SIZE} bytes")
    return by_unit, total


def accepted_regressions(manifest: dict[str, Any], by_unit: dict[str, list[dict[str, Any]]]) -> list[str]:
    errors = []
    for accepted in manifest.get("accepted_functions", []):
        tu = accepted["tu"]
        matches = [func for func in by_unit.get(tu, []) if func["name"] == accepted["symbol"]]
        if len(matches) != 1:
            errors.append(f"{tu}: {accepted['symbol']} expected once, found {len(matches)}")
            continue
        func = matches[0]
        records = func["matches"]
        rva_ok = (len(records) == 1 and int(records[0]["rva"], 16) == parse_number(accepted["rva"]))
        size_ok = func["code_size"] == parse_number(accepted["size"])
        expected_status = "FOLDED" if accepted.get("status", "strict").lower() == "folded" else "STRICT"
        owner_ok = True
        if expected_status == "FOLDED":
            expected_owner = accepted.get("into", {})
            observed_owner = func.get("folded_into", {})
            owner_ok = (observed_owner.get("rva") == f"0x{parse_number(expected_owner.get('rva', 0)):08x}"
                        and observed_owner.get("tu", "").lower().endswith(
                            Path(expected_owner.get("tu", "")).name.lower())
                        and (not expected_owner.get("symbol")
                             or observed_owner.get("symbol") == expected_owner["symbol"]))
        if func["status"] != expected_status or not rva_ok or not size_ok or not owner_ok:
            errors.append(f"{tu}: {accepted['symbol']} status={func['status']} "
                          f"rva={records[0]['rva'] if records else 'none'} size={func['code_size']}"
                          + (" owner mismatch" if not owner_ok else ""))
    return errors


def promote(manifest: dict[str, Any], by_unit: dict[str, list[dict[str, Any]]]) -> int:
    known = {(item["symbol"], parse_number(item["rva"]), item["tu"])
             for item in manifest.get("accepted_functions", [])}
    additions = []
    for tu, functions in by_unit.items():
        for func in functions:
            if func["status"] not in ("STRICT", "FOLDED") or len(func["matches"]) != 1:
                continue
            rva = int(func["matches"][0]["rva"], 16)
            key = (func["name"], rva, tu)
            if key in known:
                continue
            item: dict[str, Any] = {"symbol": func["name"], "rva": rva,
                                    "size": int(func["code_size"]), "tu": tu}
            if func["status"] == "FOLDED":
                owner = func["folded_into"]
                item["status"] = "folded"
                item["into"] = {key: owner[key] for key in ("symbol", "rva", "tu")}
            additions.append(item)
    manifest.setdefault("accepted_functions", []).extend(additions)
    for item in manifest["accepted_functions"]:
        item["rva"] = f"0x{parse_number(item['rva']):08x}"
    manifest["accepted_functions"].sort(key=lambda item: (parse_number(item["rva"]), item["tu"], item["symbol"]))
    MANIFEST.write_text(json.dumps(manifest, indent=2) + "\n", encoding="utf-8")
    strict_count = sum(item.get("status") != "folded" for item in additions)
    folded_count = len(additions) - strict_count
    print(f"accepted {strict_count} newly STRICT and {folded_count} newly FOLDED function(s)")
    return len(additions)


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--tu", action="append", default=[], metavar="src/x.cpp=path",
                        help="substitute one source file for this clean-build run")
    parser.add_argument("--accept", action="store_true", help="promote newly STRICT functions into recon.json")
    args = parser.parse_args()
    manifest = load_manifest()
    try:
        substitutions = parse_substitutions(args.tu, manifest)
        if args.accept and substitutions:
            raise ValueError("--accept cannot promote substituted candidate sources")
        clean_hashes = tool_hash_paths(manifest["toolchain"]["tree"])
        print("compiler SHA-256:")
        for name, path in zip(("cl.exe", "c1xx.dll", "c2.exe"), clean_hashes):
            print(f"  {name}: {hash_file(path)}")
        safe_clean_build()
        units = compile_units(manifest, substitutions)
        by_unit_order = [name for name, _ in units]
        comparison = run_comparator([obj for _, obj in units], manifest)
        by_unit, _ = report(comparison, by_unit_order)
        regressions = accepted_regressions(manifest, by_unit)
        if regressions:
            print("\nAccepted-function regression(s):")
            for error in regressions:
                print(f"  {error}")
            return 1
        if args.accept:
            promote(manifest, by_unit)
        return 0
    except (OSError, ValueError, RuntimeError, KeyError, json.JSONDecodeError) as exc:
        print(f"verify: {exc}", file=sys.stderr)
        return 1


if __name__ == "__main__":
    raise SystemExit(main())
