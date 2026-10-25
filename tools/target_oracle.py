#!/usr/bin/env python3
"""Independent oracle inventory for a target with its own executable layout
(docs/SB2.md), from a Ghidra headless analysis of that executable alone.

    python tools/target_oracle.py --target sb2/eggbert/en ghidra     # analyse (minutes)
    python tools/target_oracle.py --target sb2/eggbert/en inventory  # write oracle/<slug>/

`ghidra` imports the target's executable into a private project under
build/targets/<slug>/ghidra and exports functions, call references, body ranges
and symbols with tools/oracle.py's export script (unchanged). `inventory`
turns that export into oracle/<slug>/functions.csv (rva,size,end,name,kind) in
the same format as oracle/functions.csv, splitting game code from the
statically linked CRT at the first function Ghidra's function-ID database
names as a known MSVC runtime routine, and writes oracle/<slug>/summary.json.

The TU map (oracle/<slug>/tu_map.csv) is a separate step
(tools/crossmap.py, docs/SB2.md): it needs evidence from other targets.

Static analysis only: the executable is never run.
"""
from __future__ import annotations

import argparse
import csv
import hashlib
import json
import os
import subprocess
import sys
from collections import Counter
from pathlib import Path

import pefile

sys.path.insert(0, str(Path(__file__).resolve().parent))
import oracle  # noqa: E402  (its Ghidra export script and tool paths; not modified)
import targets  # noqa: E402

ROOT = Path(__file__).resolve().parents[1]
# Routines Ghidra's FID names in VC5's static CRT; the lowest one starts the CRT block (as in tools/oracle.py).
KNOWN_CRT = {"__global_unwind2", "__local_unwind2", "__abnormal_termination", "__NLG_Notify1", "_strncpy",
             "__exit", "__amsg_exit", "__CallSettingFrame@12", "_abort", "__aulldiv", "__aullrem"}


def work_dir(target: targets.Target) -> Path:
    return ROOT / "build" / "targets" / target.slug / "ghidra"


def run_ghidra(target: targets.Target) -> None:
    out = work_dir(target)
    scripts = out / "scripts"
    scripts.mkdir(parents=True, exist_ok=True)
    (scripts / "ExportOracle.java").write_text(oracle.GHIDRA_SCRIPT, encoding="utf-8")
    project = out / "project"
    project.mkdir(parents=True, exist_ok=True)
    (out / "appdata").mkdir(parents=True, exist_ok=True)
    cmd = [str(oracle.GHIDRA), str(project), target.slug, "-import", str(target.exe), "-overwrite",
           "-scriptPath", str(scripts), "-postScript", "ExportOracle.java", str(out)]
    env = os.environ.copy()
    env["JAVA_HOME"] = str(oracle.JAVA_HOME)
    env["APPDATA"] = str(out / "appdata")
    proc = subprocess.run(cmd, cwd=ROOT, env=env, text=True, stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
    (out / "ghidra.log").write_text(proc.stdout, encoding="utf-8", errors="replace")
    if proc.returncode != 0 or not (out / "ghidra_functions.csv").is_file():
        raise RuntimeError(f"Ghidra failed (exit {proc.returncode}); see {out / 'ghidra.log'}")
    print(f"Ghidra export: {out}")


def write_inventory(target: targets.Target) -> None:
    out = work_dir(target)
    rows = []
    with (out / "ghidra_functions.csv").open(newline="", encoding="utf-8-sig") as stream:
        for row in csv.DictReader(stream):
            rows.append({"rva": int(row["rva"], 16), "end": int(row["end_rva"], 16), "name": row["name"],
                         "thunk": row["thunk"].lower() == "true", "size": int(row["body_bytes"])})
    rows.sort(key=lambda r: r["rva"])
    pe = pefile.PE(str(target.exe), fast_load=True)
    entry = pe.OPTIONAL_HEADER.AddressOfEntryPoint
    text = next(s for s in pe.sections if s.Name.rstrip(b"\0") == b".text")
    text_end = text.VirtualAddress + text.Misc_VirtualSize
    split = min((r["rva"] for r in rows if r["name"] in KNOWN_CRT), default=None)
    if split is None:
        raise RuntimeError("Ghidra named no known MSVC CRT routine; cannot place the CRT split")
    # The import-library thunks (DirectX) follow the last game object; LIBC members follow them, before the first
    # routine FID names (docs/ORACLE.md: SB1's strstr/fclose/fread at 0x238b0). Game code ends after the thunks.
    thunks = [r for r in rows if r["thunk"] and r["rva"] < split and r["name"].startswith("Direct")]
    if thunks:
        last = max(thunks, key=lambda r: r["rva"])
        split = min(r["rva"] for r in rows if r["rva"] >= last["end"])
    dest = target.oracle_dir
    dest.mkdir(parents=True, exist_ok=True)
    kinds = Counter()
    sizes = Counter()
    with (dest / "functions.csv").open("w", newline="", encoding="utf-8") as stream:
        writer = csv.writer(stream, lineterminator="\n")
        writer.writerow(("rva", "size", "end", "name", "kind"))
        for r in rows:
            if r["rva"] >= text_end:
                continue  # import stubs outside .text
            kind = "thunk" if r["thunk"] else "crt/library" if r["rva"] >= split else "game"
            name = "WinMainCRTStartup" if r["rva"] == entry else r["name"]
            if r["rva"] == entry:
                kind = "crt/library"
            kinds[kind] += 1
            sizes[kind] += r["size"]
            writer.writerow((f"0x{r['rva']:x}", r["size"], f"0x{r['end']:x}", name, kind))
    summary = {
        "target": target.name,
        "exe_sha256": hashlib.sha256(target.exe.read_bytes()).hexdigest(),
        "source": "Ghidra headless analysis of the target executable alone (tools/target_oracle.py)",
        "function_count": sum(kinds.values()),
        "function_kind_counts": dict(kinds),
        "function_body_bytes_by_kind": dict(sizes),
        "crt_split_rva": f"0x{split:x}",
        "game_text_end": f"0x{split:x}",
        "entry_rva": f"0x{entry:x}",
        "text_rva_range": [f"0x{text.VirtualAddress:x}", f"0x{text_end:x}"],
    }
    (dest / "summary.json").write_text(json.dumps(summary, indent=1) + "\n", encoding="utf-8")
    print(json.dumps(summary, indent=1))


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("command", choices=("ghidra", "inventory"))
    targets.add_argument(parser)
    args = parser.parse_args()
    target = targets.load(args.target)
    if target.primary or target.oracle_dir is None:
        print("the target needs its own oracle folder in targets.json (\"oracle\")")
        return 2
    target.check_exe()
    if args.command == "ghidra":
        run_ghidra(target)
    else:
        write_inventory(target)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
