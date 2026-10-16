#!/usr/bin/env python3
"""Compile one translation unit with an isolated historical MSVC environment."""

from __future__ import annotations

import argparse
import os
from pathlib import Path
import subprocess


ROOT = Path(__file__).resolve().parents[1]
TOOLS = Path(r"C:\tools")
ALIASES = {
    "sp3": TOOLS / "msvc5_sp3",
    "sp2": TOOLS / "msvc5_sp2",
    "rtm": TOOLS / "msvc5_rtm",
    "vc6": TOOLS / "msvc6_rtm",
    "nt5-1592": TOOLS / "msvc5_cc_nt5_1592",
    "nt5-1691": TOOLS / "msvc5_cc_nt5_1691",
    "sp3-deu": TOOLS / "msvc5_cc_sp3_deu",
    "sp3-jpn": TOOLS / "msvc5_cc_sp3_jpn",
}


def resolve_tree(value: str) -> Path:
    return ALIASES.get(value.lower(), Path(value))


def tool_layout(tree: Path, vc_root_arg: str | None) -> tuple[Path, Path, Path | None, list[Path]]:
    tree = tree.resolve()
    if (tree / "VC" / "BIN" / "CL.EXE").is_file():
        vc_root = tree / "VC"
        compiler_bin = vc_root / "BIN"
        shared_bin = tree / "SHAREDIDE" / "BIN"
    elif (tree / "VC98" / "BIN" / "CL.EXE").is_file():
        vc_root = tree / "VC98"
        compiler_bin = vc_root / "BIN"
        shared_bin = tree / "COMMON" / "MSDEV98" / "BIN"
    else:
        compiler_bin = tree / "bin"
        vc_root = Path(vc_root_arg) if vc_root_arg else TOOLS / "msvc5_sp3" / "VC"
        shared_bin = vc_root.resolve().parent / "SHAREDIDE" / "BIN"
    if not (compiler_bin / "CL.EXE").is_file():
        raise FileNotFoundError(f"cl.exe not found below toolchain {tree}")
    vc_root = vc_root.resolve()
    include_dirs = [vc_root / "INCLUDE"]
    return compiler_bin, vc_root, shared_bin, include_dirs


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("source", type=Path, help="one .c or .cpp translation unit")
    parser.add_argument("--tree", default="sp3", help="sp3, sp2, rtm, vc6, standalone alias, or toolchain path")
    parser.add_argument("--vc-root", help="VC include/lib root for standalone compiler trees")
    parser.add_argument("--out", type=Path, help="COFF object path (default build/match/obj/<source>.obj)")
    parser.add_argument("--include", action="append", default=[], help="additional include directory; repeatable")
    parser.add_argument("--lib", action="append", default=[], help="additional LIB directory; repeatable")
    parser.add_argument("--define", action="append", default=[], help="preprocessor definition; repeatable")
    parser.add_argument("--flag", action="append", default=[], help="additional cl.exe argument; repeatable")
    parser.add_argument("--cwd", type=Path, default=ROOT, help="compiler working directory")
    args = parser.parse_args()

    source = args.source.resolve()
    if not source.is_file():
        parser.error(f"source does not exist: {source}")
    tree = resolve_tree(args.tree)
    try:
        compiler_bin, vc_root, shared_bin, default_includes = tool_layout(tree, args.vc_root)
    except FileNotFoundError as exc:
        parser.error(str(exc))

    output = args.out or (ROOT / "build" / "match" / "obj" / (source.stem + ".obj"))
    output = output.resolve()
    output.parent.mkdir(parents=True, exist_ok=True)
    temp_dir = ROOT / "build" / "tmp"
    temp_dir.mkdir(parents=True, exist_ok=True)

    user_include_dirs = [Path(item).resolve() for item in args.include]
    include_dirs = default_includes + user_include_dirs
    lib_dirs = [vc_root / "LIB"] + [Path(item).resolve() for item in args.lib]
    path_dirs = [compiler_bin]
    if shared_bin and shared_bin.is_dir():
        path_dirs.append(shared_bin)
    path_dirs.extend([Path(os.environ.get("SystemRoot", r"C:\Windows")) / "System32",
                      Path(os.environ.get("SystemRoot", r"C:\Windows"))])
    env = {
        "PATH": os.pathsep.join(str(item) for item in path_dirs),
        "INCLUDE": os.pathsep.join(str(item) for item in include_dirs),
        "LIB": os.pathsep.join(str(item.resolve()) for item in lib_dirs),
        "TMP": str(temp_dir),
        "TEMP": str(temp_dir),
        "SystemRoot": os.environ.get("SystemRoot", r"C:\Windows"),
    }
    # A tiny stable environment keeps inherited CL/LINK and user PATH entries out.
    command = [str(compiler_bin / "CL.EXE"), "/nologo", "/c", str(source), f"/Fo{output}"]
    # /I entries precede INCLUDE; put the explicitly selected SDK first so an older
    # DirectX header shipped with VC cannot shadow the requested SDK.
    command += [f"/I{directory}" for directory in user_include_dirs]
    command += [f"/D{definition}" for definition in args.define]
    command += args.flag

    banner_run = subprocess.run([str(compiler_bin / "CL.EXE")], cwd=args.cwd.resolve(), env=env,
                                stdout=subprocess.PIPE, stderr=subprocess.STDOUT, text=True,
                                encoding="mbcs", errors="replace")
    banner_text = banner_run.stdout
    banner_line = next((line.strip() for line in banner_text.splitlines() if line.strip()), "<banner unavailable>")
    log_dir = ROOT / "build" / "match" / "logs"
    log_dir.mkdir(parents=True, exist_ok=True)
    log_path = log_dir / f"{output.stem}.log"
    lines = [f"toolchain: {tree}", f"cl banner: {banner_line}", f"cwd: {args.cwd.resolve()}",
             f"PATH: {env['PATH']}", f"INCLUDE: {env['INCLUDE']}", f"LIB: {env['LIB']}",
             f"TMP/TEMP: {temp_dir}", "command: " + subprocess.list2cmdline(command), "", "banner output:", banner_text]
    print("cl banner: " + banner_line)
    print("command: " + subprocess.list2cmdline(command))
    result = subprocess.run(command, cwd=args.cwd.resolve(), env=env,
                            stdout=subprocess.PIPE, stderr=subprocess.STDOUT, text=True,
                            encoding="mbcs", errors="replace")
    lines.extend(["", "compile output:", result.stdout, f"exit code: {result.returncode}"])
    log_path.write_text("\n".join(lines), encoding="utf-8")
    if result.stdout:
        print(result.stdout, end="")
    print(f"log: {log_path}")
    if result.returncode == 0:
        print(f"object: {output}")
    return result.returncode


if __name__ == "__main__":
    raise SystemExit(main())
