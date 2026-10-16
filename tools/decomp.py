#!/usr/bin/env python3
"""Seed Ghidra from the canonical function/TU maps and export compact C pseudocode."""
from __future__ import annotations

import argparse
import csv
import json
import os
from pathlib import Path
import re
import subprocess
import sys

ROOT = Path(__file__).resolve().parents[1]
GHIDRA = Path(r"C:\tools\ghidra_12.1_PUBLIC\support\analyzeHeadless.bat")
JAVA_HOME = Path(r"C:\Program Files\Eclipse Adoptium\jdk-25.0.4.101-hotspot")
PRIMARY = ROOT / "assets" / "Speedy Blupi" / "BLUPI.EXE"
BUILD = ROOT / "build" / "decomp"
PROJECT = BUILD / "project"
SCRIPTS = ROOT / "tools" / "ghidra"
SEEDS = BUILD / "seeds.tsv"
OUT = BUILD / "tu"


def load_seeds() -> list[dict[str, str]]:
    recon = json.loads((ROOT / "recon.json").read_text(encoding="utf-8"))
    accepted = {int(x["rva"], 0): x["symbol"] for x in recon.get("accepted_functions", [])}
    mapped = {}
    with (ROOT / "oracle" / "tu_map.csv").open(newline="", encoding="utf-8-sig") as stream:
        mapped = {int(x["rva"], 0): x for x in csv.DictReader(stream)}
    rows = []
    with (ROOT / "oracle" / "functions.csv").open(newline="", encoding="utf-8-sig") as stream:
        for func in csv.DictReader(stream):
            rva = int(func["rva"], 0)
            m = mapped.get(rva, {})
            best = m.get("best_source", "none") or "none"
            parts = best.split(":", 2)
            source = parts[2] if len(parts) == 3 else ""
            if not source and accepted.get(rva):
                decorated = accepted[rva]
                match = re.match(r"\?([^@]+)@([^@]+)@@", decorated)
                if decorated.startswith("??0"):
                    match = re.match(r"\?\?0([^@]+)@@", decorated)
                    source = f"{match.group(1)}::{match.group(1)}" if match else decorated
                elif decorated.startswith("??1"):
                    match = re.match(r"\?\?1([^@]+)@@", decorated)
                    source = f"{match.group(1)}::~{match.group(1)}" if match else decorated
                elif match:
                    source = f"{match.group(2)}::{match.group(1)}"
                else:
                    source = decorated.lstrip("_")
            label = re.sub(r"[^A-Za-z0-9_]", "_", source).strip("_")
            if label and not label[0].isalpha():
                label = "f_" + label
            if accepted.get(rva):
                name = "STRICT_" + (label or f"rva_{rva:x}")
            elif label and ".cpp" not in source:
                name = "HYP_" + label
            else:
                name = func["name"]
            cc = "thiscall" if "::" in best or (accepted.get(rva) and re.search(r"@@[QIU]..", accepted[rva])) else "default"
            rows.append({
                "rva": f"{rva:x}", "size": func["size"], "name": name,
                "tu": m.get("tu") or ("crt_library.cpp" if func["kind"] != "game" else "unmapped.cpp"),
                "best": best, "status": m.get("status", "none"),
                "symbol": accepted.get(rva, ""), "cc": cc,
            })
    return rows


def write_seeds(rows: list[dict[str, str]]) -> None:
    BUILD.mkdir(parents=True, exist_ok=True)
    SEEDS.write_text("".join("\t".join(row[k].replace("\t", " ") for k in
        ("rva", "size", "name", "tu", "best", "status", "symbol", "cc")) + "\n" for row in rows),
        encoding="utf-8")


def run_export(rows: list[dict[str, str]]) -> None:
    if not GHIDRA.is_file():
        raise FileNotFoundError(f"Ghidra launcher missing: {GHIDRA}")
    (BUILD / "appdata").mkdir(parents=True, exist_ok=True)
    PROJECT.mkdir(parents=True, exist_ok=True)
    OUT.mkdir(parents=True, exist_ok=True)
    SCRIPTS.mkdir(parents=True, exist_ok=True)
    write_seeds(rows)
    project_file = PROJECT / "blupi.gpr"
    cmd = [str(GHIDRA), str(PROJECT), "blupi"]
    if not project_file.is_file():
        cmd += ["-import", str(PRIMARY), "-overwrite"]
    else:
        cmd += ["-process", "BLUPI.EXE"]
    cmd += ["-scriptPath", str(SCRIPTS), "-postScript", "SeedExport.java", str(SEEDS), str(OUT)]
    env = os.environ.copy()
    env["JAVA_HOME"] = str(JAVA_HOME)
    env["APPDATA"] = str(BUILD / "appdata")
    log = BUILD / "ghidra.log"
    result = subprocess.run(cmd, cwd=ROOT, env=env, stdout=subprocess.PIPE,
                            stderr=subprocess.STDOUT, text=True, encoding="utf-8", errors="replace")
    log.write_text(result.stdout, encoding="utf-8")
    if result.returncode:
        raise RuntimeError(f"Ghidra exited {result.returncode}; see {log}")
    if not list(OUT.glob("*.c")):
        raise RuntimeError(f"Ghidra produced no C exports; see {log}")
    print(f"seeded {len(rows)} function entries; exported {len(list(OUT.glob('*.c')))} TU files")


def locate(rows: list[dict[str, str]], key: str) -> dict[str, str]:
    raw = key.lower().removeprefix("0x")
    if re.fullmatch(r"[0-9a-f]+", raw):
        value = int(raw, 16)
        hit = next((r for r in rows if int(r["rva"], 16) == value), None)
    else:
        hit = next((r for r in rows if key in (r["name"], r["symbol"], r["best"].split(":", 2)[-1])), None)
    if not hit:
        raise ValueError(f"no oracle function found for {key!r}")
    return hit


def extract(path: Path, rva: str) -> str:
    lines = path.read_text(encoding="utf-8", errors="replace").splitlines()
    marker = re.compile(r"^/\* RVA=0x([0-9a-f]+)\b", re.I)
    start = None
    for i, line in enumerate(lines):
        match = marker.match(line)
        if match and int(match.group(1), 16) == int(rva, 16):
            start = i + 1
            break
    if start is None:
        return ""
    end = next((i for i in range(start, len(lines)) if marker.match(lines[i])), len(lines))
    return "\n".join(lines[start:end]).rstrip() + "\n"


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("function", nargs="?", help="function name or RVA (hex, with or without 0x)")
    parser.add_argument("--force", action="store_true", help="rerun analysis/export in the cached project")
    args = parser.parse_args()
    rows = load_seeds()
    if args.function is None:
        if args.force or not list(OUT.glob("*.c")):
            run_export(rows)
        else:
            print(f"using cached pseudocode: {OUT}")
        return 0
    row = locate(rows, args.function)
    path = OUT / (Path(row["tu"]).stem + ".c")
    code = extract(path, row["rva"])
    if not code:
        run_export(rows)
        code = extract(path, row["rva"])
    if not code:
        raise RuntimeError(f"no pseudocode exported for RVA 0x{row['rva']}")
    sys.stdout.write(code)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
