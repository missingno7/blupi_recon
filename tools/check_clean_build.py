#!/usr/bin/env python3
"""Clean-checkout test of the ordinary build (docs/BUILDING.md).

    .pyenv/python.exe tools/check_clean_build.py [--ref HEAD] [--compare build/standalone]

Exports a commit with `git archive` to build/clean-export/tree, keeps only the ordinary-build inputs
(KEEP: sources, overlays, resource scripts, registry, manifests, toolchain pins and the five build modules),
and runs the documented `tools/build.py --all` there with this interpreter. No original executable, asset,
oracle, certificate, research tool, generated object or cache exists in the export. Checks: every target
links, no output names a path of this checkout outside the export, every toolchain component matches
toolchains.json, every historical __FILE__ path was established, output directories are distinct, and
(with --compare) each image and object equals an independent build apart from its timestamp fields.
Writes build/clean-export/report.json. The export is replaced on the next run.
"""
from __future__ import annotations

import argparse
import io
import json
from pathlib import Path
import re
import shutil
import subprocess
import sys
import tarfile

ROOT = Path(__file__).resolve().parents[1]
KEEP = ("src", "games", "targets.json", "recon.json", "toolchains.json",
        "tools/build.py", "tools/targets.py", "tools/source_tree.py", "tools/msvc.py", "tools/buildpath.py")


def export(ref, tree):
    archive = subprocess.run(["git", "archive", "--format=tar", ref], cwd=ROOT, capture_output=True, check=True).stdout
    with tarfile.open(fileobj=io.BytesIO(archive)) as tar:
        tar.extractall(tree, filter="data")
    kept = {Path(k) for k in KEEP}
    removed = []
    for path in sorted(tree.rglob("*"), reverse=True):
        rel = path.relative_to(tree)
        if any(rel == k or k in rel.parents or rel in k.parents for k in kept):
            continue
        removed.append(rel.as_posix())
        path.unlink() if path.is_file() else path.rmdir()
    return removed


def foreign_paths(text, export_root):
    """Absolute paths into this checkout that are not inside the export."""
    pattern = re.compile(re.escape(str(ROOT)) + r"(?:\\\\|\\|/)?([^\"'\s;]*)", re.I)
    inside = str(export_root.relative_to(ROOT)).lower().replace("/", "\\")
    hits = set()
    for match in pattern.finditer(text):
        rest = match.group(1).replace("\\\\", "\\").replace("/", "\\").lower()
        if not rest.startswith(inside) and not rest.startswith(".pyenv\\"):   # the interpreter: external
            hits.add(match.group(0))
    return sorted(hits)


def main(argv=None):
    parser = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("--ref", default="HEAD", help="commit to export (uncommitted changes are not exported)")
    parser.add_argument("--compare", type=Path, help="build/standalone of an independent build to compare with")
    args = parser.parse_args(argv)
    work = ROOT / "build/clean-export"
    if work.exists():
        if not (work / "clean-export.marker").is_file():
            raise SystemExit(f"{work} exists and is not a previous clean export")
        shutil.rmtree(work)
    tree = work / "tree"
    tree.mkdir(parents=True)
    (work / "clean-export.marker").write_text("", encoding="utf-8")
    commit = subprocess.run(["git", "rev-parse", args.ref], cwd=ROOT, capture_output=True, text=True,
                            check=True).stdout.strip()
    removed = export(commit, tree)
    for forbidden in ("assets", "oracle", "evidence", "external", "build", "scratch", ".pyenv"):
        assert not (tree / forbidden).exists(), forbidden
    run = subprocess.run([sys.executable, str(tree / "tools/build.py"), "--all"], cwd=tree,
                         capture_output=True, text=True, encoding="utf-8", errors="replace")
    (work / "build.log").write_text(run.stdout + run.stderr, encoding="utf-8")
    registered = json.loads((tree / "targets.json").read_text(encoding="utf-8"))["targets"]
    rows, problems, outputs = [], [], set()
    for name in registered:
        slug = name.replace("/", "-")
        out = tree / "build/standalone" / slug
        record_path = out / "build.json"
        row = dict(target=name, output=str(out.relative_to(tree)))
        if not record_path.is_file():
            problems.append(f"{name}: no build.json")
            rows.append(dict(row, result="FAILED"))
            continue
        record = json.loads(record_path.read_text(encoding="utf-8"))
        row.update(result=record.get("result"), compiler=record["compiler_tree"], linker=record["linker_tree"],
                   image=record.get("image", {}).get("sha256_without_timestamps"),
                   unpinned=[c["path"] for c in record["toolchain"] if not c["pinned"]],
                   historical_paths={k: v["status"] for k, v in record["historical_paths"].items()},
                   resources=record["resources"]["status"])
        if record.get("result") != "BUILT":
            problems.append(f"{name}: {record.get('result')}")
        if row["unpinned"]:
            problems.append(f"{name}: unpinned toolchain {row['unpinned']}")
        if any(v != "historical" for v in row["historical_paths"].values()):
            problems.append(f"{name}: historical build path not established")
        texts = [record_path, out / "link.rsp", *sorted((out / "logs").glob("*.log"))]
        foreign = sorted({hit for path in texts if path.is_file()
                          for hit in foreign_paths(path.read_text(encoding="utf-8", errors="replace"), tree)})
        row["foreign_paths"] = foreign
        if foreign:
            problems.append(f"{name}: references this checkout outside the export: {foreign[:3]}")
        image = Path(record["image"]["path"])
        if image.parent.resolve() in outputs:
            problems.append(f"{name}: output directory shared")
        outputs.add(image.parent.resolve())
        if args.compare:
            other_path = (ROOT / args.compare / slug / "build.json")
            if other_path.is_file():
                other = json.loads(other_path.read_text(encoding="utf-8"))
                same_image = other["image"]["sha256_without_timestamps"] == record["image"]["sha256_without_timestamps"]
                same_objects = all(other["objects"][k]["sections_sha256"] == v["sections_sha256"]
                                   for k, v in record["objects"].items())
                row["reproducible"] = ("yes: image equal apart from timestamp fields, object sections equal"
                                       if same_image and same_objects else
                                       f"NO (image {'equal' if same_image else 'differs'}, "
                                       f"objects {'equal' if same_objects else 'differ'})")
                if row["reproducible"].startswith("NO"):
                    problems.append(f"{name}: differs from {other_path}")
            else:
                row["reproducible"] = "not compared"
        rows.append(row)
    report = dict(commit=commit, exported_inputs=list(KEEP), removed_from_export=len(removed),
                  build_exit=run.returncode, interpreter=sys.executable, targets=rows, problems=problems,
                  status="PASS" if not problems and run.returncode == 0 else "FAIL")
    (work / "report.json").write_text(json.dumps(report, indent=2) + "\n", encoding="utf-8")
    for row in rows:
        print(f"{row['target']:16} {row['result']:10} {row.get('compiler', '?')}/{row.get('linker', '?')} "
              f"reproducible: {row.get('reproducible', '-')}")
    print("\n".join(problems) or "no problems")
    print(f"clean export {commit[:10]}: {report['status']} ({work / 'report.json'})")
    return 0 if report["status"] == "PASS" else 1


if __name__ == "__main__":
    raise SystemExit(main())
