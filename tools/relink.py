#!/usr/bin/env python3
"""Build/link canonical TUs with VC5; inspect the result without executing it."""
from __future__ import annotations

import argparse
import contextlib
import csv
import hashlib
import io
import json
import os
from pathlib import Path
import re
import subprocess
import sys

import pefile

ROOT = Path(__file__).resolve().parents[1]
LINKERS = {
    "sp3": Path(r"C:\tools\msvc5_sp3\VC\BIN\LINK.EXE"),
    "7255": Path(r"C:\tools\msvc5_link_ce_beta1_7255\bin\link.exe"),
    "1592": Path(r"C:\tools\msvc5_link_nt5_1592\bin\LINK.EXE"),
    "1691": Path(r"C:\tools\msvc5_link_nt5_1691\bin\LINK.EXE"),
    "1197": Path(r"C:\tools\msvc5_link_psdk1197\bin\LINK.EXE"),
    "7351": Path(r"C:\tools\msvc5_link_ce200_7351\bin\link.exe"),
}


def build_resources(out, rc_source, vc_root):
    """Compile a target's source resource script; LINK runs its matching CVTRES."""
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


def parse_number(value):
    return int(value, 0) if isinstance(value, str) else int(value)


def toolchain_roots(tree_name):
    sys.path.insert(0, str(ROOT / "tools"))
    import msvc
    tree = msvc.resolve_tree(tree_name).resolve()
    _compiler_bin, vc_root, shared_bin, _ = msvc.tool_layout(tree, None)
    return tree, vc_root, shared_bin


def linker_path(selection, target_tree, vc_root):
    if selection == "rtm":
        if target_tree.lower() == "rtm":
            return vc_root / "BIN" / "LINK.EXE"
        _tree, rtm_vc, _shared = toolchain_roots("rtm")
        return rtm_vc / "BIN" / "LINK.EXE"
    return LINKERS[selection]


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
        dx_lib_dirs = [Path(r"C:\tools\dx5sdk\LIB")]
    # DX5 SDK: oracle GUIDs (docs/TOOLCHAIN.md); network.cpp defines INITGUID.
    libraries += [dx_lib_dirs[0] / (s + ".lib") for s in ("dsound", "ddraw", "dplayx")]
    # Explicit game import libraries precede LIBC, as in the SB1 proven link.
    libraries += [library_vc_root / "LIB" / "LIBC.LIB", library_vc_root / "LIB" / "OLDNAMES.LIB"]
    return libraries


def rich_header(path):
    """Decode a PE Rich header as static provenance; never use it as link input."""
    data = Path(path).read_bytes()
    pe_offset = int.from_bytes(data[0x3c:0x40], "little") if len(data) >= 0x40 else 0
    rich = data.rfind(b"Rich", 0, pe_offset)
    if rich < 0 or rich + 8 > len(data):
        return None
    key = int.from_bytes(data[rich + 4:rich + 8], "little")
    dans = None
    for pos in range(rich - 4, 0x3f, -4):
        if int.from_bytes(data[pos:pos + 4], "little") ^ key == 0x536e6144:
            dans = pos
            break
    entries = []
    if dans is not None:
        # DanS is followed by three zero dwords, then encoded compid/count pairs.
        for pos in range(dans + 16, rich, 8):
            comp = int.from_bytes(data[pos:pos + 4], "little") ^ key
            count = int.from_bytes(data[pos + 4:pos + 8], "little") ^ key
            entries.append(dict(comp_id=f"0x{comp:08x}", count=count))
    return dict(offset=rich, key=f"0x{key:08x}", dans_offset=dans,
                entries=entries)


def digest(path):
    return hashlib.sha256(Path(path).read_bytes()).hexdigest()


def read_map(path):
    """Public and static symbols; retain owner to disambiguate TU-local names."""
    if not path.is_file():
        return []
    result = []
    pattern = re.compile(r"^\s*([0-9a-f]{4}):([0-9a-f]{8})\s+(\S+)\s+([0-9a-f]{8})\s+(.*?)\s*$", re.I)
    for line in path.read_text(encoding="cp1252").splitlines():
        match = pattern.match(line)
        if match:
            segment, offset, name, va, owner = match.groups()
            result.append(dict(segment=int(segment, 16), offset=int(offset, 16), name=name,
                               va=int(va, 16), owner=re.sub(r"^f\s+", "", owner)))
    return result


def oracle_order(units, target):
    first = {}
    inventory = ((target.oracle_dir / "tu_map.csv") if target.oracle_dir else ROOT / "oracle/tu_map.csv")
    with inventory.open(newline="", encoding="utf-8-sig") as stream:
        for row in csv.DictReader(stream):
            first[row["tu"]] = min(first.get(row["tu"], 1 << 32), int(row["rva"], 0))
    # Atlas data is one oracle band at RVA 0x78dd8..0x7b25f, immediately
    # before pixmap's MouseHotSpot at 0x7b260 (independent code references).
    # The data-only TU has no text anchor; place it by that data evidence.
    if "pixmap.cpp" in first:
        first["dectables.cpp"] = first["pixmap.cpp"] - 1
    return sorted(units, key=lambda u: first.get(Path(u["source"]).name, 1 << 32))


def imports(pe):
    return [dict(dll=e.dll.decode("ascii"), descriptor_rva=pe.get_rva_from_offset(e.struct.get_file_offset()),
                 int_rva=e.struct.OriginalFirstThunk, iat_rva=e.struct.FirstThunk,
                 names=[i.name.decode("ascii") if i.name else "#" + str(i.ordinal) for i in e.imports])
            for e in getattr(pe, "DIRECTORY_ENTRY_IMPORT", [])]


def image_info(pe):
    header = pe.OPTIONAL_HEADER
    return dict(image_base=header.ImageBase, entry_rva=header.AddressOfEntryPoint,
                section_alignment=header.SectionAlignment, file_alignment=header.FileAlignment,
                linker_version=[header.MajorLinkerVersion, header.MinorLinkerVersion],
                sections=[dict(name=s.Name.rstrip(b"\0").decode(), rva=s.VirtualAddress,
                               virtual_size=s.Misc_VirtualSize, raw_size=s.SizeOfRawData,
                               raw_offset=s.PointerToRawData, characteristics=s.Characteristics)
                          for s in pe.sections], imports=imports(pe))


def import_comparison(oracle, candidate):
    before, after = imports(oracle), imports(candidate)
    by_before = {item["dll"].lower(): item for item in before}
    by_after = {item["dll"].lower(): item for item in after}
    shared = sorted(by_before.keys() & by_after.keys())
    sets = {dll: set(by_before[dll]["names"]) == set(by_after[dll]["names"]) for dll in shared}
    orders = {dll: by_before[dll]["names"] == by_after[dll]["names"] for dll in shared}
    return dict(oracle_dll_order=[item["dll"] for item in before],
                candidate_dll_order=[item["dll"] for item in after],
                dll_order_equal=[item["dll"].lower() for item in before] ==
                                [item["dll"].lower() for item in after],
                oracle_import_count=sum(len(item["names"]) for item in before),
                candidate_import_count=sum(len(item["names"]) for item in after),
                symbol_sets_equal_by_dll=sets, symbol_order_equal_by_dll=orders)


def layout_report(oracle, candidate, map_symbols, manifest):
    result = dict(oracle=image_info(oracle), candidate=image_info(candidate))
    result["import_comparison"] = import_comparison(oracle, candidate)
    result["sections"] = []
    candidate_sections = {s.Name: s for s in candidate.sections}
    for section in oracle.sections:
        other = candidate_sections.get(section.Name)
        n = min(section.Misc_VirtualSize, section.SizeOfRawData)
        item = dict(name=section.Name.rstrip(b"\0").decode(), oracle_initialized_size=n)
        if other is not None:
            a, b = section.get_data()[:n], other.get_data()[:min(other.SizeOfRawData, other.Misc_VirtualSize)]
            item.update(rva_delta=other.VirtualAddress - section.VirtualAddress,
                        virtual_size_delta=other.Misc_VirtualSize - section.Misc_VirtualSize,
                        same_section_offset_equal_bytes=sum(x == y for x, y in zip(a, b)),
                        byte_exact=a == b)
        else:
            item["missing"] = True
        result["sections"].append(item)
    oracle_names = {s.Name for s in oracle.sections}
    result["candidate_only_sections"] = [dict(name=s.Name.rstrip(b"\0").decode(),
        rva=s.VirtualAddress, virtual_size=s.Misc_VirtualSize, raw_size=s.SizeOfRawData,
        raw_offset=s.PointerToRawData, characteristics=s.Characteristics)
        for s in candidate.sections if s.Name not in oracle_names]
    by_name = {}
    for symbol in map_symbols:
        by_name.setdefault(symbol["name"], []).append(symbol)
    shifts = []
    for accepted in manifest.get("accepted_functions", []):
        if accepted.get("withdrawn"):
            continue
        names = by_name.get(accepted["symbol"], [])
        if len(names) == 1:
            original = parse_number(accepted["rva"])
            rva = names[0]["va"] - candidate.OPTIONAL_HEADER.ImageBase
            exact_at_linked = (candidate.get_data(rva, accepted["size"]) ==
                               oracle.get_data(original, accepted["size"]))
            exact_at_original = (candidate.get_data(original, accepted["size"]) ==
                                 oracle.get_data(original, accepted["size"]))
            shifts.append(dict(tu=accepted["tu"], name=accepted["symbol"], oracle_rva=original,
                               linked_rva=rva, delta=rva-original,
                               byte_exact_at_linked_rva=exact_at_linked,
                               byte_exact_at_original_rva=exact_at_original))
    result["accepted_function_layout"] = shifts
    result["accepted_function_summary"] = dict(
        manifest_count=len(manifest.get("accepted_functions", [])),
        uniquely_mapped=len(shifts),
        at_original_rva=sum(item["delta"] == 0 for item in shifts),
        byte_exact_at_linked_rva=sum(item["byte_exact_at_linked_rva"] for item in shifts),
        byte_exact_at_original_rva=sum(item["byte_exact_at_original_rva"] for item in shifts),
        delta_histogram={str(delta): sum(item["delta"] == delta for item in shifts)
                         for delta in sorted({item["delta"] for item in shifts})})
    oracle_rsrc = next((s for s in oracle.sections if s.Name.rstrip(b"\0") == b".rsrc"), None)
    candidate_rsrc = next((s for s in candidate.sections if s.Name.rstrip(b"\0") == b".rsrc"), None)
    result["resource_section"] = dict(
        oracle=(dict(rva=oracle_rsrc.VirtualAddress, virtual_size=oracle_rsrc.Misc_VirtualSize,
                     raw_size=oracle_rsrc.SizeOfRawData,
                     sha256=hashlib.sha256(oracle_rsrc.get_data()).hexdigest()) if oracle_rsrc else None),
        candidate=(dict(rva=candidate_rsrc.VirtualAddress, virtual_size=candidate_rsrc.Misc_VirtualSize,
                        raw_size=candidate_rsrc.SizeOfRawData,
                        sha256=hashlib.sha256(candidate_rsrc.get_data()).hexdigest()) if candidate_rsrc else None),
        byte_equal=(oracle_rsrc.get_data() == candidate_rsrc.get_data()
                    if oracle_rsrc is not None and candidate_rsrc is not None else
                    oracle_rsrc is None and candidate_rsrc is None),
        note="comparison only; original .rsrc bytes are never used as linked input")
    return result


def recorded_order(units, settings):
    names = settings['object_order']
    by_name = {unit['source']: unit for unit in units}
    if len(names) != len(set(names)) or set(names) != set(by_name):
        raise ValueError('standalone object_order must contain every translation unit exactly once')
    return [by_name[name] for name in names]


def main(argv=None):
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--out-dir", type=Path, default=ROOT / "build/relink")
    parser.add_argument("--objects-dir", type=Path, help="reuse objects named as verify.py names them")
    parser.add_argument("--link-only", action="store_true", help="reuse this output directory's objects")
    parser.add_argument("--linker", choices=sorted(set(LINKERS) | {"rtm"}), default=None)
    parser.add_argument("--historical-host", action="store_true",
                        help="run unmodified SP3 LINK with the VC5 CRT and oracle UTC clock via histlink.py")
    parser.add_argument("--order", choices=("oracle", "manifest", "recorded", "eggbert-jauge-before-menu"))
    parser.add_argument("--standalone", action="store_true",
                        help="build from recorded source settings without reading an oracle or running verification")
    parser.add_argument("--opt", choices=("ref,icf", "ref,noicf", "noref,icf", "noref,noicf"), default="ref,icf")
    parser.add_argument("--library", action="append", default=[], type=Path, help="additional genuine library")
    parser.add_argument("--library-order", choices=("target", "crt-first"), default="target",
                        help="secondary SB1 layout hypothesis: place LIBC/OLDNAMES before import libraries")
    parser.add_argument("--oracle", type=Path, help="override the selected target's executable oracle")
    parser.add_argument("--no-res", action="store_true", help="omit the selected source tree's resource script")
    parser.add_argument("--default-libs", action="store_true", help="IDE-style: LIBC/OLDNAMES via the objects' /defaultlib directives, no /nodefaultlib")
    parser.add_argument("--entry", default=None, help="explicit /entry symbol (default: LINK chooses for /subsystem:windows)")
    parser.add_argument("--diagnostic", action="store_true",
                        help="link even when accepted functions regress (layout diagnostics only; the gate stays FAIL)")
    import targets
    targets.add_argument(parser)
    args = parser.parse_args(argv)
    target = targets.load(args.target)
    if not target.exe.is_file():
        parser.error(f"target oracle is missing: {target.exe}")
    target.check_exe()
    manifest = target.manifest()
    settings = manifest.get('standalone_link', {})
    tree, vc_root, shared_bin = toolchain_roots(manifest["toolchain"]["tree"])
    default_linker = ("rtm" if manifest["toolchain"]["tree"].lower() == "rtm" or
                      target.name.startswith("sb1/eggbert/") else "sp3")
    args.linker = args.linker or default_linker
    args.order = args.order or ('recorded' if args.standalone else 'oracle')
    if args.standalone and args.order in ('oracle', 'eggbert-jauge-before-menu'):
        parser.error('--standalone cannot use an oracle-based object order')
    if args.standalone and not all(key in settings for key in ('image_base', 'section_alignment')):
        parser.error('--standalone requires image_base and section_alignment in the selected manifest')
    if args.historical_host and args.linker != "sp3":
        parser.error("--historical-host requires --linker sp3 (the pinned LINK.EXE)")
    out = args.out_dir.resolve()
    # Generated artifacts belong in build/ or scratch/, never beside original inputs.
    if not any(out.is_relative_to(ROOT / d) for d in ("build", "scratch")):
        parser.error("--out-dir must be inside build/ or scratch/")
    out.mkdir(parents=True, exist_ok=True)
    units = manifest["translation_units"]
    obj_dir = args.objects_dir.resolve() if args.objects_dir else out / "objects"
    obj_dir.mkdir(parents=True, exist_ok=True)
    objects = {}
    degraded = {}
    compile_commands = []
    for index, unit in enumerate(units):
        source = unit["source"]
        obj = obj_dir / f"{index:02d}_{Path(source).stem}.obj"
        objects[source] = obj
        if not args.link_only and not args.objects_dir:
            command = [sys.executable, str(ROOT / "tools/msvc.py"), str(ROOT / source),
                       "--out", str(obj), "--tree", manifest["toolchain"]["tree"],
                       "--cwd", str(ROOT), "--log-dir", str(out / "logs")]
            for include in [str(ROOT / manifest.get("source_root", "src"))] + manifest["toolchain"].get("include_dirs", []):
                command += ["--include", include]
            for define in manifest["toolchain"].get("defines", []) + unit.get("defines", []):
                command += ["--define", define]
            for flag in manifest["toolchain"]["flags"]:
                command += ["--flag", flag]
            if unit.get("build_path"):
                command += ["--build-path", unit["build_path"]]
            compile_commands.append(command)
            run = subprocess.run(command, cwd=ROOT, capture_output=True, text=True,
                                 encoding="mbcs", errors="replace")
            print(f"compile {source}: exit {run.returncode}", flush=True)
            if "buildpath: UNAVAILABLE" in run.stdout:
                degraded[source] = "historical build path unavailable"
                print("  gate INCOMPLETE: historical build path unavailable", flush=True)
            if run.returncode:
                print(run.stdout + run.stderr)
                return run.returncode
        if not obj.is_file():
            parser.error(f"missing object: {obj}")
        if (args.link_only or args.objects_dir) and unit.get("build_path"):
            log = obj_dir.parent / "logs" / (obj.stem + ".log")
            commands = [line for line in log.read_text(encoding="utf-8").splitlines()
                        if line.startswith("command:")] if log.is_file() else []
            if not any(unit["build_path"].lower() in line.lower() for line in commands):
                degraded[source] = "historical source path not attested in reused object's compile log"
    # Preserve degraded-path provenance when relinking the same objects.
    previous = out / "build.json"
    if args.link_only and not args.objects_dir and previous.is_file():
        old_record = json.loads(previous.read_text())
        if all(old_record.get("objects", {}).get(k, {}).get("sha256") == digest(v) for k, v in objects.items()):
            degraded.update(old_record.get("degraded", {}))
            compile_commands = old_record.get("compile_commands", [])
    # Reuse the canonical verifier's comparison/regression logic, without its
    # clean-build operation or any manifest promotion. Supplemental CRT evidence
    # is compiled via msvc.py and is deliberately absent from the link inputs.
    if args.standalone:
        regressions = []
        gate = dict(status='NOT_RUN', reason='standalone build; no oracle read', degraded=degraded)
    else:
        import verify
        verify.BUILD = out
        verify.TARGET = target
        verify.GAME_TEXT_END = target.game_text_end
        verify.GAME_TEXT_SIZE = target.game_text_end - verify.ORACLE_TEXT_START
        verify.DEGRADED.clear()
        verify.DEGRADED.update(degraded)
        evidence_objects = verify.compile_crt_evidence(manifest)
        comparison = verify.run_comparator(list(objects.values()), evidence_objects, manifest)
        stream = io.StringIO()
        with contextlib.redirect_stdout(stream):
            by_unit, coverage = verify.report(comparison, [u["source"] for u in units])
        (out / "verify.log").write_text(stream.getvalue(), encoding="utf-8")
        regressions = verify.accepted_regressions(manifest, by_unit)
        if target.primary:
            import equivalence
            equivalent = equivalence.check(manifest, comparison, evidence_objects, out)
        elif target.name.startswith('sb2/'):
            import equivalence_target
            equivalent = equivalence_target.check(target, manifest, comparison, evidence_objects, out)
        else:
            equivalent = dict(status="NOT_APPLICABLE", reason="SB2 target has its own oracle inventory")
        gate = dict(regressions=regressions, degraded=degraded, coverage=coverage,
                    equivalence=equivalent,
                    status="FAIL" if regressions else "INCOMPLETE" if degraded else "PASS")
    (out / "gate.json").write_text(json.dumps(gate, indent=2) + "\n", encoding="utf-8")
    print(f"accepted-function gate {gate['status']}" +
          (f"; coverage {coverage} / {verify.GAME_TEXT_SIZE}" if not args.standalone else ''), flush=True)
    if regressions:
        print("\n".join(regressions))
        if not args.diagnostic:
            return 1
        print("DIAGNOSTIC LINK: accepted-function gate FAILED; the image is for layout measurement only", flush=True)
    if args.order == 'recorded' and not settings.get('object_order'):
        parser.error('--order recorded requires standalone_link.object_order in the selected manifest')
    if args.order == 'eggbert-jauge-before-menu' and not target.name.startswith('sb1/eggbert/'):
        parser.error('--order eggbert-jauge-before-menu requires an SB1 Eggbert target')
    if args.order in ('oracle', 'eggbert-jauge-before-menu'):
        ordered = oracle_order(units, target)
        if args.order == 'eggbert-jauge-before-menu':
            jauge = next(u for u in ordered if Path(u['source']).name == 'jauge.cpp')
            menu = next(u for u in ordered if Path(u['source']).name == 'menu.cpp')
            ordered.remove(jauge)
            ordered.insert(ordered.index(menu), jauge)
    elif args.order == 'recorded':
        ordered = recorded_order(units, settings)
    else:
        ordered = units
    oracle_path = args.oracle.resolve() if args.oracle else target.exe
    oracle = None if args.standalone else pefile.PE(str(oracle_path))
    image_base = int(settings['image_base'], 0) if args.standalone else oracle.OPTIONAL_HEADER.ImageBase
    alignment = settings['section_alignment'] if args.standalone else oracle.OPTIONAL_HEADER.SectionAlignment
    linker = linker_path(args.linker, manifest["toolchain"]["tree"], vc_root)
    library_vc_root = vc_root
    if args.linker == "rtm" and manifest["toolchain"]["tree"].lower() != "rtm":
        _tree, library_vc_root, _shared = toolchain_roots("rtm")
    libraries = target_libraries(manifest, vc_root, library_vc_root)
    if args.library_order == "crt-first":
        if target.primary or not target.name.startswith("sb1/") or args.default_libs:
            parser.error("--library-order crt-first requires an explicit-library secondary SB1 target")
        crt_libraries = [lib for lib in libraries if lib.name.upper() in ("LIBC.LIB", "OLDNAMES.LIB")]
        import_libraries = [lib for lib in libraries if lib not in crt_libraries]
        if len(crt_libraries) != 2:
            parser.error("--library-order crt-first requires both LIBC.LIB and OLDNAMES.LIB")
        libraries = crt_libraries + import_libraries
    if args.default_libs:
        libraries = [l for l in libraries if l.name.upper() not in ("LIBC.LIB", "OLDNAMES.LIB")]
    libraries += args.library
    image_name = "BLUPI.EXE" if target.primary else target.exe.name
    image_path = out / image_name
    map_path = out / (Path(image_name).stem + ".map")
    options = ["/nologo", "/machine:i386", "/subsystem:windows,4.0", *([f"/entry:{args.entry}"] if args.entry else []),
               f"/base:0x{image_base:x}", f"/align:{alignment}",
               "/fixed", "/incremental:no", *([] if args.default_libs else ["/nodefaultlib"]),
               *["/opt:" + s for s in args.opt.split(",")],
               f"/out:{image_path}", f"/map:{map_path}"]
    source_root = ROOT / manifest.get("source_root", "src")
    resource_candidates = [source_root / "res" / f"{target.slug}.rc"]
    if source_root != ROOT / "src":
        resource_candidates.insert(0, ROOT / "games/sb2/res" / f"{target.slug}.rc")
    resource_candidates.append(source_root / "res" / "blupi.rc")
    resource_script = next((candidate for candidate in resource_candidates if candidate.is_file()), None)
    if args.no_res:
        res_files, resource_status = [], "omitted by --no-res"
    elif resource_script is not None:
        res_files = [build_resources(out, resource_script, vc_root)]
        resource_status = f"{resource_script.relative_to(ROOT)} via target RC + CVTRES"
    else:
        res_files, resource_status = [], "no source .rc in target tree; original .rsrc is comparison-only"
    arguments = options + [str(objects[u["source"]]) for u in ordered] + list(map(str, res_files)) + list(map(str, libraries))
    response = out / "link.rsp"
    response.write_text("\n".join(subprocess.list2cmdline([arg]) for arg in arguments) + "\n", encoding="cp1252")
    command = [str(linker), "@" + str(response)]
    host_report = out / "historical-host.json" if args.historical_host else None
    if host_report:
        command = [sys.executable, str(ROOT / "tools/histlink.py"), "--report", str(host_report),
                   "--", "@" + str(response)]
    temp = ROOT / "build/tmp"
    temp.mkdir(parents=True, exist_ok=True)
    system = os.environ.get("SystemRoot", r"C:\Windows")
    env = dict(SystemRoot=system, TMP=str(temp), TEMP=str(temp), LIB=str(vc_root / "LIB"),
               PATH=os.pathsep.join(map(str, [linker.parent, vc_root / "BIN",
                                             shared_bin or vc_root.parent / "SHAREDIDE/BIN",
                                             Path(system) / "System32", Path(system)])))
    # Never leave a stale successful image/map after a failed link.
    for path in (image_path, map_path, out / "layout.json", out / "image.json"):
        if path.exists():
            path.unlink()
    if host_report and host_report.exists():
        host_report.unlink()
    run = subprocess.run(command, cwd=ROOT, env=env, capture_output=True, text=True,
                         encoding="mbcs", errors="replace")
    (out / "link.log").write_text(run.stdout + run.stderr, encoding="utf-8")
    transcript = run.stdout + run.stderr
    duplicates = sorted(set(re.findall(r"error LNK2005: (.*?)(?: already defined)", transcript)))
    unresolved = sorted(set(re.findall(r"unresolved external symbol (.*)", transcript)))
    record = dict(target=target.name, target_oracle=str(oracle_path), oracle_sha256=target.sha256,
                  linker_selection=args.linker,
                  command=command, expanded_command=[str(linker)] + arguments,
                  compile_commands=compile_commands, order=[u["source"] for u in ordered],
                  objects={k: dict(path=str(v), sha256=digest(v)) for k, v in objects.items()},
                  canonical_sources={u["source"]: digest(ROOT / u["source"]) for u in units},
                  object_mode="reused; see object compile logs for candidate source provenance" if
                  args.objects_dir else "canonical", toolchain=manifest["toolchain"],
                  linker_sha256=digest(linker), libraries={str(p): digest(p) for p in libraries},
                  degraded=degraded, exit_code=run.returncode, standalone=args.standalone,
                  oracle_read=not args.standalone,
                  resource_status=resource_status, duplicate_symbols=duplicates,
                  unresolved_symbols=unresolved)
    if run.returncode >= 0xC0000000:
        record["linker_process_status"] = f"NTSTATUS 0x{run.returncode:08x}"
    if host_report:
        record["historical_host"] = dict(report=str(host_report),
            launcher_sha256=digest(ROOT / "tools/histlink.py"),
            **(json.loads(host_report.read_text(encoding="utf-8")) if host_report.is_file() else {}))
    (out / "build.json").write_text(json.dumps(record, indent=2) + "\n", encoding="utf-8")
    if run.returncode:
        # A source-definition failure can print hundreds of redundant errors.
        # Keep the complete transcript on disk and summarize unique blockers.
        diagnostics = [line for line in transcript.splitlines() if "LNK2001" in line or "warning" in line or "fatal" in line]
        print(f"link: {len(duplicates)} distinct multiply defined symbols; full log: {out / 'link.log'}")
        print("\n".join(diagnostics), flush=True)
    else:
        print(transcript, end="", flush=True)
    if run.returncode:
        if oracle is not None:
            record["oracle"] = image_info(oracle)
        (out / "build.json").write_text(json.dumps(record, indent=2) + "\n", encoding="utf-8")
        # LINK can leave an empty map on LNK1120. It is not a successful layout.
        for path in (image_path, map_path):
            if path.exists():
                path.unlink()
        return run.returncode
    candidate = pefile.PE(str(image_path))
    if args.standalone:
        result = image_info(candidate)
        result['sha256'] = digest(image_path)
        (out / 'image.json').write_text(json.dumps(result, indent=2) + '\n', encoding='utf-8')
        print(f"linked {image_path} from source (no oracle read; static inspection only)")
        return 0
    result = layout_report(oracle, candidate, read_map(map_path), manifest)
    result["target"] = target.name
    result["oracle_path"] = str(oracle_path)
    result["linker_selection"] = args.linker
    result["oracle_sha256"] = target.sha256
    result["candidate_sha256"] = digest(image_path)
    result["rich_header"] = dict(oracle=rich_header(oracle_path), candidate=rich_header(image_path))
    (out / "layout.json").write_text(json.dumps(result, indent=2) + "\n", encoding="utf-8")
    print(f"linked {image_path} (static inspection only)")
    for item in result["sections"]:
        print(item)
    print("accepted function layout:", result["accepted_function_summary"])
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
