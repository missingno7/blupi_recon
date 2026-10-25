#!/usr/bin/env python3
"""Ordinary historical build: compile and link one target's reconstructed source (docs/BUILDING.md).

    .pyenv/python.exe tools/build.py --target sb2/blupi/en     # build/standalone/sb2-blupi-en/
    .pyenv/python.exe tools/build.py --all                     # every registered target, one at a time
    .pyenv/python.exe tools/build.py --list | --plan [--target T]

Inputs are the target's entry in targets.json, the build sections of its manifest, its source tree
(plus overlays), its resource script and the pinned historical toolchain (toolchains.json). The original
executable, oracle inventories, certificates and research tools are never read
(tests/test_build_boundary.py). A linked image is a build result only: tools/verify.py and tools/relink.py
compare against the original; behaviour is tested separately (docs/RUNTIME.md, port/). The output is never
executed. relink.py imports this module's compile/link helpers, so verification links the same way.
"""
from __future__ import annotations

import argparse
import hashlib
import json
import os
from pathlib import Path
import shutil
import subprocess
import sys

ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / "tools"))
import msvc  # noqa: E402  (tree layout only; compiles run msvc.py as a subprocess)
import source_tree  # noqa: E402
import targets  # noqa: E402

PINS = ROOT / "toolchains.json"
DX5_ROOT = Path(r"C:\tools\dx5sdk")


def digest(path):
    return hashlib.sha256(Path(path).read_bytes()).hexdigest()


def toolchain_roots(tree_name):
    tree = msvc.resolve_tree(tree_name).resolve()
    _compiler_bin, vc_root, shared_bin, _ = msvc.tool_layout(tree, None)
    return tree, vc_root, shared_bin


def build_resources(out, rc_source, vc_root):
    """Compile a target's resource script to .res; the selected linker converts it."""
    rc = vc_root.parent / "SHAREDIDE/BIN/RC.EXE"
    res = out / (rc_source.stem + ".res")
    temp = ROOT / "build/tmp"
    temp.mkdir(parents=True, exist_ok=True)
    system = os.environ.get("SystemRoot", r"C:\Windows")
    env = dict(SystemRoot=system, TMP=str(temp), TEMP=str(temp), INCLUDE=f"{vc_root / 'INCLUDE'};{vc_root / 'MFC/INCLUDE'}",
               PATH=os.pathsep.join(map(str, [rc.parent, vc_root / "BIN", Path(system) / "System32", Path(system)])))
    run = subprocess.run([str(rc), "/r", "/fo", str(res), rc_source.name], cwd=rc_source.parent, env=env,
                         capture_output=True, text=True, encoding="mbcs", errors="replace")
    if run.returncode:
        raise SystemExit("RC failed: " + run.stdout + run.stderr)
    return res


def resource_script(spec, effective_root):
    """The target's resource script and how it was chosen (explicit registry entry, or by name)."""
    if spec.resource_script:
        return ROOT / spec.resource_script, spec.resource_note or "explicit (targets.json)"
    candidates = [effective_root / "res" / f"{spec.slug}.rc"]
    if effective_root != ROOT / "src":
        candidates.insert(0, ROOT / "games/sb2/res" / f"{spec.slug}.rc")
    candidates.append(effective_root / "res" / "blupi.rc")
    script = next((c for c in candidates if c.is_file()), None)
    return script, None if script is None else f"{script.relative_to(ROOT).as_posix()} by target name"


def target_libraries(manifest, vc_root, library_vc_root=None):
    """Use the target linker's CRT and the SDK named by its include path."""
    library_vc_root = library_vc_root or vc_root
    libraries = [library_vc_root / "LIB" / (s + ".lib") for s in
                 ("kernel32", "user32", "gdi32", "winmm")]
    dx_lib_dirs = []
    for include in manifest["toolchain"].get("include_dirs", []):
        path = Path(include)
        if path.name.lower() == "include":
            candidate = path.parent / "LIB"
            if candidate.is_dir() and candidate not in dx_lib_dirs:
                dx_lib_dirs.append(candidate)
    if not dx_lib_dirs:
        dx_lib_dirs = [DX5_ROOT / "LIB"]
    # DX5 SDK: oracle GUIDs (docs/TOOLCHAIN.md); network.cpp defines INITGUID.
    libraries += [dx_lib_dirs[0] / (s + ".lib") for s in ("dsound", "ddraw", "dplayx")]
    # Explicit game import libraries precede LIBC, as in the SB1 proven link.
    libraries += [library_vc_root / "LIB" / "LIBC.LIB", library_vc_root / "LIB" / "OLDNAMES.LIB"]
    return libraries


def recorded_order(units, settings):
    names = settings['object_order']
    by_name = {unit['source']: unit for unit in units}
    if len(names) != len(set(names)) or set(names) != set(by_name):
        raise ValueError('standalone object_order must contain every translation unit exactly once')
    return [by_name[name] for name in names]


def link_options(image_base, alignment, image_path, map_path, opt="ref,icf", entry=None, default_libs=False):
    return ["/nologo", "/machine:i386", "/subsystem:windows,4.0", *([f"/entry:{entry}"] if entry else []),
            f"/base:0x{image_base:x}", f"/align:{alignment}",
            "/fixed", "/incremental:no", *([] if default_libs else ["/nodefaultlib"]),
            *["/opt:" + s for s in opt.split(",")],
            f"/out:{image_path}", f"/map:{map_path}"]


def link_environment(linker, vc_root, shared_bin):
    temp = ROOT / "build/tmp"
    temp.mkdir(parents=True, exist_ok=True)
    system = os.environ.get("SystemRoot", r"C:\Windows")
    path = [linker.parent, vc_root / "BIN", shared_bin or vc_root.parent / "SHAREDIDE/BIN",
            Path(system) / "System32", Path(system)]
    return dict(SystemRoot=system, TMP=str(temp), TEMP=str(temp), LIB=str(vc_root / "LIB"),
                PATH=os.pathsep.join(map(str, path))), path


def compile_unit(manifest, effective_root, unit, obj, log_dir, candidate=None):
    """Compile one TU through msvc.py; return (command, run, historical-path-unavailable)."""
    source = source_tree.compile_source(manifest, effective_root, unit["source"], candidate)
    command = [sys.executable, str(ROOT / "tools/msvc.py"), str(source),
               "--out", str(obj), "--tree", manifest["toolchain"]["tree"],
               "--cwd", str(ROOT), "--log-dir", str(log_dir)]
    for include in [str(effective_root)] + manifest["toolchain"].get("include_dirs", []):
        command += ["--include", include]
    for define in manifest["toolchain"].get("defines", []) + unit.get("defines", []):
        command += ["--define", define]
    for flag in manifest["toolchain"]["flags"]:
        command += ["--flag", flag]
    if unit.get("build_path"):
        command += ["--build-path", unit["build_path"]]
    run = subprocess.run(command, cwd=ROOT, capture_output=True, text=True, encoding="mbcs", errors="replace")
    return command, run, "buildpath: UNAVAILABLE" in run.stdout


def imports(pe):
    return [dict(dll=e.dll.decode("ascii"), descriptor_rva=pe.get_rva_from_offset(e.struct.get_file_offset()),
                 int_rva=e.struct.OriginalFirstThunk, iat_rva=e.struct.FirstThunk,
                 names=[i.name.decode("ascii") if i.name else "#" + str(i.ordinal) for i in e.imports])
            for e in getattr(pe, "DIRECTORY_ENTRY_IMPORT", [])]


def image_info(pe):
    header = pe.OPTIONAL_HEADER
    return dict(file_timestamp=pe.FILE_HEADER.TimeDateStamp,
                image_base=header.ImageBase, entry_rva=header.AddressOfEntryPoint,
                section_alignment=header.SectionAlignment, file_alignment=header.FileAlignment,
                linker_version=[header.MajorLinkerVersion, header.MinorLinkerVersion],
                sections=[dict(name=s.Name.rstrip(b"\0").decode(), rva=s.VirtualAddress,
                               virtual_size=s.Misc_VirtualSize, raw_size=s.SizeOfRawData,
                               raw_offset=s.PointerToRawData, characteristics=s.Characteristics)
                          for s in pe.sections], imports=imports(pe))


def timestamp_fields(path):
    """File offsets of the image's clock-dependent fields: PE header and resource directories."""
    import pefile
    pe = pefile.PE(str(path))
    fields = [("pe_header", pe.FILE_HEADER.get_file_offset() + 4, pe.FILE_HEADER.TimeDateStamp)]
    pending = [pe.DIRECTORY_ENTRY_RESOURCE] if hasattr(pe, "DIRECTORY_ENTRY_RESOURCE") else []
    while pending:
        directory = pending.pop()
        fields.append(("resource_directory", directory.struct.get_file_offset() + 4, directory.struct.TimeDateStamp))
        pending.extend(e.directory for e in directory.entries if hasattr(e, "directory"))
    return pe, fields


def reproducible_digest(data, offsets):
    """SHA-256 of bytes with the given 4-byte timestamp fields read as zero. A report only:
    the files are never modified (no patching to make builds look identical)."""
    data = bytearray(data)
    for offset in offsets:
        data[offset:offset + 4] = b"\0" * 4
    return hashlib.sha256(data).hexdigest()


def object_digest(path):
    """An object's SHA-256 without its COFF header TimeDateStamp (bytes 4..8)."""
    return reproducible_digest(Path(path).read_bytes(), [4])


def load_pins():
    return json.loads(PINS.read_text(encoding="utf-8"))


def pinned_hash(pins, path):
    """(pinned root alias, expected SHA-256) for a file below a root named in toolchains.json."""
    for group in (pins["trees"], pins["sdk"]):
        for alias, entry in group.items():
            try:
                rel = Path(path).relative_to(Path(entry["root"])).as_posix().upper()
            except ValueError:
                continue
            for files in entry["files"].values():
                for key, value in files.items():
                    if key.upper() == rel:
                        return alias, value
            return alias, None
    return None, None


def resolve_dll(name, search):
    return next((d / name for d in search if (d / name).is_file()), Path(name))


def preflight(manifest, link_tree, pins):
    """Hash every toolchain component this target loads and compare with toolchains.json."""
    tree = manifest["toolchain"]["tree"].lower()
    for alias in (tree, link_tree):
        if alias not in pins["trees"]:
            raise SystemExit(f"build: toolchain {alias!r} has no pins in toolchains.json")
    _t, vc_root, shared_bin = toolchain_roots(tree)
    _l, link_vc_root, _ls = toolchain_roots(link_tree)
    linker = link_vc_root / "BIN" / "LINK.EXE"
    _env, link_search = link_environment(linker, vc_root, shared_bin)
    used = [("compile", vc_root / "BIN" / n) for n in ("CL.EXE", "C1XX.DLL", "C2.EXE")]
    used.append(("compile", resolve_dll("MSPDB50.DLL", [vc_root / "BIN", shared_bin])))
    used += [("resource", vc_root.parent / "SHAREDIDE/BIN" / n) for n in ("RC.EXE", "RCDLL.DLL")]
    used.append(("resource", vc_root / "MFC/INCLUDE/AFXRES.H"))
    # LINK loads its DLLs from its own directory, then PATH (link_environment); it starts CVTRES likewise.
    used += [("link", linker), ("link", resolve_dll("CVTRES.EXE", link_search))]
    used += [("link", resolve_dll(n, link_search)) for n in ("MSPDB50.DLL", "MSDIS100.DLL")]
    used += [("libraries", p) for p in target_libraries(manifest, vc_root, link_vc_root)]
    used += [("include", DX5_ROOT / "INCLUDE" / n) for n in ("DDRAW.H", "DSOUND.H", "DPLAY.H")]
    components = []
    for role, path in used:
        alias, expected = pinned_hash(pins, path)
        actual = digest(path) if Path(path).is_file() else None
        components.append(dict(role=role, tree=alias, path=str(path), sha256=actual, expected=expected,
                               pinned=actual is not None and actual == expected))
    return components


def plan(spec):
    """Everything a build of this target will use, without compiling."""
    manifest = spec.manifest()
    settings = manifest.get("standalone_link")
    if not settings or not all(k in settings for k in ("image_base", "section_alignment", "object_order")):
        raise SystemExit(f"build: {spec.manifest_path.relative_to(ROOT)} has no complete standalone_link")
    tree = manifest["toolchain"]["tree"].lower()
    link_tree = (spec.link_tree or tree).lower()
    return manifest, settings, tree, link_tree


def prepare_output(out):
    out = out.resolve()
    if not any(out.is_relative_to(ROOT / d) for d in ("build", "scratch")):
        raise SystemExit("build: --out-dir must be inside build/ or scratch/")
    if out.exists() and any(out.iterdir()):
        if not (out / "build.json").is_file() and not (out / "build.started").is_file():
            raise SystemExit(f"build: {out} is not empty and not a previous build output; choose another --out-dir")
        shutil.rmtree(out)   # a fresh build: no stale objects, staged sources or images
    out.mkdir(parents=True, exist_ok=True)
    (out / "build.started").write_text("", encoding="utf-8")
    return out


def build(spec, out, allow_unpinned=False):
    manifest, settings, tree, link_tree = plan(spec)
    components = preflight(manifest, link_tree, load_pins())
    unpinned = [c for c in components if not c["pinned"]]
    degraded = {}
    if unpinned:
        listing = "\n".join(f"  {c['path']}: {c['sha256'] or 'MISSING'}" for c in unpinned)
        if not allow_unpinned:
            print(f"build: toolchain differs from toolchains.json:\n{listing}", file=sys.stderr)
            return 2
        degraded["toolchain"] = f"{len(unpinned)} component(s) differ from toolchains.json"
    out = prepare_output(out)
    effective_root = source_tree.prepare(manifest, out)
    units = manifest["translation_units"]
    objects, compile_commands, historical_paths = {}, [], {}
    (out / "objects").mkdir()
    for index, unit in enumerate(units):
        obj = out / "objects" / f"{index:02d}_{Path(unit['source']).stem}.obj"
        command, run, unavailable = compile_unit(manifest, effective_root, unit, obj, out / "logs")
        compile_commands.append(command)
        print(f"compile {unit['source']}: exit {run.returncode}", flush=True)
        if unit.get("build_path"):
            historical_paths[unit["source"]] = dict(path=unit["build_path"],
                                                    status="UNAVAILABLE (compiled at its real path)" if unavailable
                                                    else "historical")
            if unavailable:
                degraded[unit["source"]] = "historical build path unavailable; __FILE__ data differs"
                print("  DEGRADED: historical build path unavailable", flush=True)
        if run.returncode:
            print(run.stdout + run.stderr)
            return run.returncode
        objects[unit["source"]] = obj
    _tree, vc_root, shared_bin = toolchain_roots(tree)
    _ltree, link_vc_root, _lshared = toolchain_roots(link_tree)
    linker = link_vc_root / "BIN" / "LINK.EXE"
    libraries = target_libraries(manifest, vc_root, link_vc_root)
    script, resource_status = resource_script(spec, effective_root)
    res_files = [build_resources(out, script, vc_root)] if script else []
    image_path = out / spec.image_name
    map_path = out / (Path(spec.image_name).stem + ".map")
    ordered = recorded_order(units, settings)
    arguments = (link_options(int(settings["image_base"], 0), settings["section_alignment"], image_path, map_path)
                 + [str(objects[u["source"]]) for u in ordered] + list(map(str, res_files))
                 + list(map(str, libraries)))
    response = out / "link.rsp"
    response.write_text("\n".join(subprocess.list2cmdline([a]) for a in arguments) + "\n", encoding="cp1252")
    env, link_path = link_environment(linker, vc_root, shared_bin)
    run = subprocess.run([str(linker), "@" + str(response)], cwd=ROOT, env=env, capture_output=True, text=True,
                         encoding="mbcs", errors="replace")
    (out / "link.log").write_text(run.stdout + run.stderr, encoding="utf-8")
    print(run.stdout + run.stderr, end="", flush=True)
    record = dict(
        kind="ordinary build (tools/build.py)", target=spec.name, status=spec.status,
        reads=dict(registry="targets.json", manifest=spec.manifest_path.relative_to(ROOT).as_posix(),
                   manifest_sections=sorted(manifest), toolchain_pins="toolchains.json",
                   original_executable=False, oracle=False, certificates=False),
        compiler_tree=tree, linker_tree=link_tree, source_root=manifest.get("source_root", "src"),
        source_overlays=manifest.get("source_overlays", []), effective_root=str(effective_root),
        defines=manifest["toolchain"].get("defines", []), flags=manifest["toolchain"]["flags"],
        sources={u["source"]: dict(input=source_tree.resolve(manifest, u["source"]).relative_to(ROOT).as_posix(),
                                   sha256=digest(source_tree.resolve(manifest, u["source"]))) for u in units},
        historical_paths=historical_paths, compile_commands=compile_commands,
        objects={k: dict(path=str(v), sha256=digest(v), sha256_without_timestamp=object_digest(v))
                 for k, v in objects.items()},
        resources=dict(script=script.relative_to(ROOT).as_posix() if script else None, status=resource_status,
                       res_sha256=digest(res_files[0]) if res_files else None),
        link=dict(linker=str(linker), order=[u["source"] for u in ordered],
                  order_provenance=settings.get("provenance"), arguments=arguments,
                  search_path=[str(p) for p in link_path], exit_code=run.returncode,
                  libraries={str(p): digest(p) for p in libraries}),
        toolchain=components, degraded=degraded)
    if run.returncode:
        record["result"] = "LINK FAILED"
        (out / "build.json").write_text(json.dumps(record, indent=2) + "\n", encoding="utf-8")
        for path in (image_path, map_path):
            if path.exists():
                path.unlink()
        return run.returncode
    pe, fields = timestamp_fields(image_path)
    data = image_path.read_bytes()
    record["image"] = dict(path=str(image_path), bytes=len(data), sha256=hashlib.sha256(data).hexdigest(),
                           sha256_without_timestamps=reproducible_digest(data, [o for _, o, _ in fields]),
                           timestamp_fields=[dict(field=f, offset=o, value=f"0x{v:08x}") for f, o, v in fields],
                           info=image_info(pe))
    record["result"] = "BUILT" + (" (DEGRADED)" if degraded else "")
    record["not_claimed"] = ("A linked executable only: not byte-identical to the original and not behaviourally "
                             "verified. Historical comparison: tools/verify.py, tools/relink.py; runtime: docs/RUNTIME.md.")
    (out / "build.json").write_text(json.dumps(record, indent=2) + "\n", encoding="utf-8")
    (out / "build.started").unlink()
    summary(record)
    return 0


def summary(record):
    image = record["image"]
    print(f"\n{record['target']}: {record['result']}")
    print(f"  compiler {record['compiler_tree'].upper()}, linker {record['linker_tree'].upper()}; source "
          f"{record['source_root']}" + (f" + {', '.join(record['source_overlays'])}" if record['source_overlays'] else ""))
    print(f"  resources: {record['resources']['status']}")
    for source, item in record["historical_paths"].items():
        print(f"  {source}: __FILE__ {item['path']} ({item['status']})")
    print(f"  {image['path']}: {image['bytes']} bytes, sha256 {image['sha256']}")
    print(f"  without {len(image['timestamp_fields'])} timestamp field(s): {image['sha256_without_timestamps']}")
    print("  no original executable, oracle or certificate read; not compared with the original")


def main(argv=None):
    parser = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    targets.add_argument(parser)
    parser.add_argument("--all", action="store_true", help="build every registered target, sequentially")
    parser.add_argument("--out-dir", type=Path, help="default build/standalone/<target slug> (with --all: a parent)")
    parser.add_argument("--list", action="store_true", help="list targets and their build configuration")
    parser.add_argument("--plan", action="store_true", help="resolve inputs and toolchain pins, do not compile")
    parser.add_argument("--allow-unpinned-toolchain", action="store_true",
                        help="build with components that differ from toolchains.json (recorded as DEGRADED)")
    args = parser.parse_args(argv)
    names = targets.names() if args.all or args.list else [args.target or targets.registry()["default"]]
    if args.list or args.plan:
        for name in names:
            spec = targets.build_spec(name)
            manifest, settings, tree, link_tree = plan(spec)
            line = dict(target=name, compiler=tree, linker=link_tree, source_root=manifest.get("source_root", "src"),
                        overlays=manifest.get("source_overlays", []), defines=spec.defines, image=spec.image_name,
                        resource=spec.resource_script, build_path=spec.build_path)
            if args.plan:
                line["toolchain"] = preflight(manifest, link_tree, load_pins())
                line["unpinned"] = [c["path"] for c in line["toolchain"] if not c["pinned"]]
            print(json.dumps(line))
        return 0
    results = {}
    for name in names:
        spec = targets.build_spec(name)
        if args.all:
            out = (args.out_dir or ROOT / "build/standalone") / spec.slug
        else:
            out = args.out_dir or ROOT / "build/standalone" / spec.slug
        print(f"=== {name} -> {out}", flush=True)
        results[name] = build(spec, out, args.allow_unpinned_toolchain)
    if len(names) > 1:
        print("\n" + "\n".join(f"{n}: {'ok' if r == 0 else f'FAILED ({r})'}" for n, r in results.items()))
    return max(results.values(), default=0)


if __name__ == "__main__":
    raise SystemExit(main())
