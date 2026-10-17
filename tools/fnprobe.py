#!/usr/bin/env python3
"""Fast single-function probe: compile one TU and diff one function against BLUPI.EXE (diagnostic only).

Compiles ONE translation unit with the recon.json settings (flags, defines, includes, historical build path)
through tools/msvc.py (machine-wide compile cap applies) in about a second, then compares one function's bytes
with the oracle at the given RVA. Relocation fields are masked, so `diff=0` is NOT a strict match - use
tools/verify.py for acceptance. Meant for tight source/compiler experiments (docs/COMPILER_STUDY.md).
In Git Bash set MSYS_NO_PATHCONV=1 before --extra-flag=/X, or MSYS silently turns the flag into a path.

  .pyenv\\python.exe tools\\fnprobe.py src/event.cpp ?TreatEventBase@CEvent@@QAEHIIJ@Z 0x1d230
  .pyenv\\python.exe tools\\fnprobe.py src/event.cpp=scratch\\me\\event.cpp DrawButtons 0x16f70 --region 0x1a30:0x1ac1
"""

from __future__ import annotations

import argparse
from itertools import zip_longest
import json
from pathlib import Path
import subprocess
import sys

ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / "tools"))
import capstone  # noqa: E402
import cmpfunc  # noqa: E402
import pefile  # noqa: E402

EXE = ROOT / "assets" / "Speedy Blupi" / "BLUPI.EXE"


def compile_unit(canonical: str, source: Path, out_dir: Path, extra_flags: list[str] | None = None) -> Path:
    manifest = json.loads((ROOT / "recon.json").read_text(encoding="utf-8"))
    tool = manifest["toolchain"]
    unit = next((u for u in manifest["translation_units"] if u["source"] == canonical), {"source": canonical})
    obj = out_dir / (Path(canonical).stem + ".obj")
    command = [sys.executable, str(ROOT / "tools" / "msvc.py"), str(source), "--tree", tool["tree"],
               "--out", str(obj), "--log-dir", str(out_dir / "logs"), "--cwd", str(ROOT)]
    if unit.get("build_path"):
        command += ["--build-path", unit["build_path"]]
    includes = [ROOT / "src"] + [Path(p) for p in tool.get("include_dirs", [])]
    if source.parent != ROOT / "src":
        includes.insert(0, source.parent)
    for include in includes:
        command += ["--include", str(include)]
    for define in tool.get("defines", []) + unit.get("defines", []):
        command += ["--define", define]
    for flag in tool.get("flags", []) + (extra_flags or []):
        command += ["--flag", flag]
    run = subprocess.run(command, cwd=ROOT, stdout=subprocess.PIPE, stderr=subprocess.STDOUT, text=True,
                         encoding="mbcs", errors="replace")
    if run.returncode:
        sys.exit("\n".join(line for line in run.stdout.splitlines() if "error" in line.lower()) or run.stdout)
    for line in run.stdout.splitlines():
        if line.startswith("buildpath: UNAVAILABLE"):
            print("note: " + line)
    return obj


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("unit", help="src/x.cpp or src/x.cpp=candidate.cpp")
    parser.add_argument("function", help="decorated symbol, or a unique substring of it")
    parser.add_argument("rva", help="oracle RVA of the function, e.g. 0x1d230")
    parser.add_argument("--region", help="lo:hi function offsets to print side by side (unaligned sequences)")
    parser.add_argument("--name", default="default", help="private output folder build/fnprobe/<name>")
    parser.add_argument("--extra-flag", action="append", default=[], help="additional cl flag for experiments only")
    args = parser.parse_args()

    canonical, _, candidate = args.unit.partition("=")
    source = (ROOT / (candidate or canonical)).resolve()
    out_dir = ROOT / "build" / "fnprobe" / args.name
    out_dir.mkdir(parents=True, exist_ok=True)
    obj = compile_unit(canonical.replace("\\", "/"), source, out_dir, args.extra_flag)

    functions = [f for f in cmpfunc.Coff(obj).functions() if args.function == f.name or args.function in f.name]
    if len(functions) != 1:
        sys.exit(f"function selector matched {len(functions)} symbols: {[f.name for f in functions][:8]}")
    func = functions[0]
    rva = int(args.rva, 0)
    pe = pefile.PE(str(EXE), fast_load=True)
    target = pe.get_data(rva, len(func.data))
    masked = set()
    for reloc in func.relocs:
        masked.update(range(reloc.offset - func.start, reloc.offset - func.start + 4))
    diffs = [k for k in range(len(func.data)) if func.data[k] != target[k] and k not in masked]
    print(f"{func.name}  size={len(func.data)}  relocations={len(func.relocs)}  "
          f"unmasked differing bytes={len(diffs)}" + (f"  first=+{diffs[0]:#x}" if diffs else ""))
    if diffs:
        print("  offsets: " + " ".join(f"+{k:#x}" for k in diffs[:24]) + (" ..." if len(diffs) > 24 else ""))
    if args.region:
        lo, hi = (int(x, 0) for x in args.region.split(":"))
        md = capstone.Cs(capstone.CS_ARCH_X86, capstone.CS_MODE_32)
        cand = [f"{i.mnemonic} {i.op_str}" for i in md.disasm(func.data, 0) if lo <= i.address < hi]
        targ = [f"{i.mnemonic} {i.op_str}" for i in md.disasm(target, 0) if lo <= i.address < hi]
        print(f"  {'candidate':<44} oracle")
        for a, b in zip_longest(cand, targ, fillvalue=""):
            print(("  " if a == b else "!!") + f"{a:<44} {b}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
