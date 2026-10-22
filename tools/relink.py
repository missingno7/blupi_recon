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
DEFAULT_ORACLE = ROOT / "assets/Speedy Blupi/BLUPI.EXE"
LINKERS = {
    "sp3": Path(r"C:\tools\msvc5_sp3\VC\BIN\LINK.EXE"),
    "7255": Path(r"C:\tools\msvc5_link_ce_beta1_7255\bin\link.exe"),
    "1592": Path(r"C:\tools\msvc5_link_nt5_1592\bin\LINK.EXE"),
    "1691": Path(r"C:\tools\msvc5_link_nt5_1691\bin\LINK.EXE"),
    "1197": Path(r"C:\tools\msvc5_link_psdk1197\bin\LINK.EXE"),
    "7351": Path(r"C:\tools\msvc5_link_ce200_7351\bin\link.exe"),
}
VC = Path(r"C:\tools\msvc5_sp3\VC")
DX = Path(r"C:\tools\dx5sdk\LIB")  # DX5 SDK: oracle GUIDs (docs/TOOLCHAIN.md)
LIBRARIES = [VC / "LIB" / (s + ".lib") for s in
             ("kernel32", "user32", "gdi32", "winmm")]
# Oracle import descriptors are in this order; the 5.10 linker's IAT order is
# independently sorted by DLL. Actual full-image order still needs a clean link.
LIBRARIES += [DX / (s + ".lib") for s in ("dsound", "ddraw", "dplayx")]  # dxguid.lib was not linked (network.cpp INITGUID)
# Oracle hint/name records put game imports before the later LIBC imports.
# Keep explicit import libraries first; LINK rescans them after loading LIBC.
LIBRARIES += [VC / "LIB/LIBC.LIB", VC / "LIB/OLDNAMES.LIB"]


def build_resources(out):
    """Compile src/res/blupi.rc with the VC5 resource compiler (LINK converts the .res with CVTRES 5.00.1668)."""
    rc = VC.parent / "SHAREDIDE/BIN/RC.EXE"
    res = out / "blupi.res"
    temp = ROOT / "build/tmp"
    temp.mkdir(parents=True, exist_ok=True)
    system = os.environ.get("SystemRoot", r"C:\Windows")
    env = dict(SystemRoot=system, TMP=str(temp), TEMP=str(temp), INCLUDE=f"{VC / 'INCLUDE'};{VC / 'MFC/INCLUDE'}",
               PATH=os.pathsep.join(map(str, [rc.parent, VC / "BIN", Path(system) / "System32", Path(system)])))
    run = subprocess.run([str(rc), "/r", "/fo", str(res), "blupi.rc"], cwd=ROOT / "src/res", env=env,
                         capture_output=True, text=True, encoding="mbcs", errors="replace")
    if run.returncode:
        raise SystemExit("RC failed: " + run.stdout + run.stderr)
    return res


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


def oracle_order(units):
    first = {}
    with (ROOT / "oracle/tu_map.csv").open(newline="", encoding="utf-8-sig") as stream:
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


def layout_report(oracle, candidate, map_symbols, manifest):
    result = dict(oracle=image_info(oracle), candidate=image_info(candidate))
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
    by_name = {}
    for symbol in map_symbols:
        by_name.setdefault(symbol["name"], []).append(symbol)
    shifts = []
    for accepted in manifest.get("accepted_functions", []):
        if accepted.get("withdrawn"):
            continue
        names = by_name.get(accepted["symbol"], [])
        if len(names) == 1:
            original = int(accepted["rva"], 0)
            rva = names[0]["va"] - candidate.OPTIONAL_HEADER.ImageBase
            shifts.append(dict(tu=accepted["tu"], name=accepted["symbol"], oracle_rva=original,
                               linked_rva=rva, delta=rva-original,
                               byte_exact=candidate.get_data(rva, accepted["size"]) ==
                               oracle.get_data(original, accepted["size"])))
    result["accepted_function_layout"] = shifts
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
    parser.add_argument("--linker", choices=LINKERS, default="sp3")
    parser.add_argument("--historical-host", action="store_true",
                        help="run unmodified SP3 LINK with the VC5 CRT and oracle UTC clock via histlink.py")
    parser.add_argument("--order", choices=("oracle", "manifest", "recorded"))
    parser.add_argument("--standalone", action="store_true",
                        help="build from recorded source settings without reading an oracle or running verification")
    parser.add_argument("--opt", choices=("ref,icf", "ref,noicf", "noref,noicf"), default="ref,icf")
    parser.add_argument("--library", action="append", default=[], type=Path, help="additional genuine library")
    parser.add_argument("--oracle", type=Path, default=DEFAULT_ORACLE)
    parser.add_argument("--no-res", action="store_true", help="omit src/res/blupi.rc (resources)")
    parser.add_argument("--default-libs", action="store_true", help="IDE-style: LIBC/OLDNAMES via the objects' /defaultlib directives, no /nodefaultlib")
    parser.add_argument("--entry", default=None, help="explicit /entry symbol (default: LINK chooses for /subsystem:windows)")
    parser.add_argument("--diagnostic", action="store_true",
                        help="link even when accepted functions regress (layout diagnostics only; the gate stays FAIL)")
    args = parser.parse_args(argv)
    args.order = args.order or ('recorded' if args.standalone else 'oracle')
    if args.standalone and args.order == 'oracle':
        parser.error('--standalone cannot use --order oracle')
    if args.historical_host and args.linker != "sp3":
        parser.error("--historical-host requires --linker sp3 (the pinned LINK.EXE)")
    out = args.out_dir.resolve()
    # Generated artifacts belong in build/ or scratch/, never beside original inputs.
    if not any(out.is_relative_to(ROOT / d) for d in ("build", "scratch")):
        parser.error("--out-dir must be inside build/ or scratch/")
    out.mkdir(parents=True, exist_ok=True)
    manifest = json.loads((ROOT / "recon.json").read_text(encoding="utf-8"))
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
            for include in [str(ROOT / "src")] + manifest["toolchain"].get("include_dirs", []):
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
        verify.DEGRADED.clear()
        verify.DEGRADED.update(degraded)
        evidence_objects = verify.compile_crt_evidence(manifest)
        comparison = verify.run_comparator(list(objects.values()), evidence_objects, manifest)
        stream = io.StringIO()
        with contextlib.redirect_stdout(stream):
            by_unit, coverage = verify.report(comparison, [u["source"] for u in units])
        (out / "verify.log").write_text(stream.getvalue(), encoding="utf-8")
        regressions = verify.accepted_regressions(manifest, by_unit)
        import equivalence
        equivalent = equivalence.check(manifest, comparison, evidence_objects, out)
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
    settings = manifest['standalone_link']
    ordered = (oracle_order(units) if args.order == 'oracle' else
               recorded_order(units, settings) if args.order == 'recorded' else units)
    oracle = None if args.standalone else pefile.PE(str(args.oracle))
    image_base = int(settings['image_base'], 0) if args.standalone else oracle.OPTIONAL_HEADER.ImageBase
    alignment = settings['section_alignment'] if args.standalone else oracle.OPTIONAL_HEADER.SectionAlignment
    linker = LINKERS[args.linker]
    libraries = ([l for l in LIBRARIES if l.name.upper() not in ("LIBC.LIB", "OLDNAMES.LIB")] if args.default_libs else LIBRARIES) + args.library
    options = ["/nologo", "/machine:i386", "/subsystem:windows,4.0", *([f"/entry:{args.entry}"] if args.entry else []),
               f"/base:0x{image_base:x}", f"/align:{alignment}",
               "/fixed", "/incremental:no", *([] if args.default_libs else ["/nodefaultlib"]),
               *["/opt:" + s for s in args.opt.split(",")],
               f"/out:{out / 'BLUPI.EXE'}", f"/map:{out / 'BLUPI.map'}"]
    res_files = [] if args.no_res else [build_resources(out)]
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
    env = dict(SystemRoot=system, TMP=str(temp), TEMP=str(temp), LIB=str(VC / "LIB"),
               PATH=os.pathsep.join(map(str, [linker.parent, VC / "BIN", VC.parent / "SHAREDIDE/BIN",
                                             Path(system) / "System32", Path(system)])))
    # Never leave a stale successful image/map after a failed link.
    for path in (out / "BLUPI.EXE", out / "BLUPI.map", out / "layout.json", out / "image.json"):
        if path.exists():
            path.unlink()
    if host_report and host_report.exists():
        host_report.unlink()
    run = subprocess.run(command, cwd=ROOT, env=env, capture_output=True, text=True,
                         encoding="mbcs", errors="replace")
    (out / "link.log").write_text(run.stdout + run.stderr, encoding="utf-8")
    record = dict(command=command, expanded_command=[str(linker)] + arguments,
                  compile_commands=compile_commands, order=[u["source"] for u in ordered],
                  objects={k: dict(path=str(v), sha256=digest(v)) for k, v in objects.items()},
                  canonical_sources={u["source"]: digest(ROOT / u["source"]) for u in units},
                  object_mode="reused; see object compile logs for candidate source provenance" if
                  args.objects_dir else "canonical", toolchain=manifest["toolchain"],
                  linker_sha256=digest(linker), libraries={str(p): digest(p) for p in libraries},
                  degraded=degraded, exit_code=run.returncode, standalone=args.standalone,
                  oracle_read=not args.standalone,
                  resource_status="omitted" if args.no_res else "src/res/blupi.rc via VC5 RC + CVTRES")
    if host_report:
        record["historical_host"] = dict(report=str(host_report),
            launcher_sha256=digest(ROOT / "tools/histlink.py"),
            **(json.loads(host_report.read_text(encoding="utf-8")) if host_report.is_file() else {}))
    (out / "build.json").write_text(json.dumps(record, indent=2) + "\n", encoding="utf-8")
    transcript = run.stdout + run.stderr
    if run.returncode:
        # A source-definition failure can print hundreds of redundant errors.
        # Keep the complete transcript on disk and summarize unique blockers.
        diagnostics = [line for line in transcript.splitlines() if "LNK2001" in line or "warning" in line or "fatal" in line]
        duplicates = sorted(set(re.findall(r"error LNK2005: (.*?)(?: already defined)", transcript)))
        print(f"link: {len(duplicates)} distinct multiply defined symbols; full log: {out / 'link.log'}")
        print("\n".join(diagnostics), flush=True)
    else:
        print(transcript, end="", flush=True)
    if run.returncode:
        record["duplicate_symbols"] = duplicates
        record["unresolved_symbols"] = sorted(set(re.findall(r"unresolved external symbol (.*)", transcript)))
        if oracle is not None:
            record["oracle"] = image_info(oracle)
        (out / "build.json").write_text(json.dumps(record, indent=2) + "\n", encoding="utf-8")
        # LINK can leave an empty map on LNK1120. It is not a successful layout.
        for path in (out / "BLUPI.EXE", out / "BLUPI.map"):
            if path.exists():
                path.unlink()
        return run.returncode
    candidate = pefile.PE(str(out / "BLUPI.EXE"))
    if args.standalone:
        result = image_info(candidate)
        result['sha256'] = digest(out / 'BLUPI.EXE')
        (out / 'image.json').write_text(json.dumps(result, indent=2) + '\n', encoding='utf-8')
        print(f"linked {out / 'BLUPI.EXE'} from source (no oracle read; static inspection only)")
        return 0
    result = layout_report(oracle, candidate, read_map(out / "BLUPI.map"), manifest)
    (out / "layout.json").write_text(json.dumps(result, indent=2) + "\n", encoding="utf-8")
    print(f"linked {out / 'BLUPI.EXE'} (static inspection only)")
    for item in result["sections"]:
        print(item)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
