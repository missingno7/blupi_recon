#!/usr/bin/env python3
"""Matching regression gate across every target (docs/VERIFYING.md).

    .pyenv/python.exe tools/regression_gate.py run BEFORE [TARGET ...]   # verify + relink, one target at a time
    ... make the change ...
    .pyenv/python.exe tools/regression_gate.py run AFTER
    .pyenv/python.exe tools/regression_gate.py compare BEFORE AFTER

`run` executes tools/verify.py for each target and relinks its fresh objects with tools/relink.py
(the primary with --historical-host). It saves the verifier transcript, results.json, equivalence.json and
the relink layout/gate under scratch/gate/<label>/. `compare` passes when every verifier result and
certificate check is byte-identical, every transcript is identical apart from staged source-tree names,
the primary relink image is byte-identical, and every other relinked image is identical apart from its PE
and resource-directory timestamps (LINK 5.x writes the current time). Any difference is printed for
review; an intended improvement must be justified, never accepted silently.
"""
from __future__ import annotations

import json
from pathlib import Path
import re
import shutil
import subprocess
import sys

ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / "tools"))
import build  # noqa: E402
import targets  # noqa: E402

GATES = ROOT / "scratch/gate"


def run(label, names):
    out = GATES / label
    out.mkdir(parents=True, exist_ok=True)
    summary = {}
    for name in names or targets.names():
        target = targets.load(name)
        slug = target.slug
        verify_cmd = [sys.executable, "tools/verify.py"] + ([] if target.primary else ["--target", name])
        result = subprocess.run(verify_cmd, cwd=ROOT, capture_output=True, text=True, encoding="utf-8", errors="replace")
        (out / f"verify-{slug}.txt").write_text(result.stdout + result.stderr, encoding="utf-8")
        verify_dir = ROOT / "build" / ("verify" if target.primary else f"verify-{slug}")
        for item in ("results.json", "equivalence.json"):
            if (verify_dir / item).is_file():
                shutil.copy(verify_dir / item, out / f"{slug}-{item}")
        record = dict(verify_exit=result.returncode)
        relink_dir = ROOT / "build" / f"gate-{label}-{slug}"
        if relink_dir.exists():
            shutil.rmtree(relink_dir)
        relink_cmd = [sys.executable, "tools/relink.py", "--objects-dir", str(verify_dir / "objects"),
                      "--out-dir", str(relink_dir)] + (["--historical-host"] if target.primary else ["--target", name])
        result = subprocess.run(relink_cmd, cwd=ROOT, capture_output=True, text=True, encoding="utf-8", errors="replace")
        (out / f"relink-{slug}.txt").write_text(result.stdout + result.stderr, encoding="utf-8")
        record["relink_exit"] = result.returncode
        image = relink_dir / targets.build_spec(name).image_name
        if image.is_file():
            _pe, fields = build.timestamp_fields(image)
            data = image.read_bytes()
            record.update(image_sha256=build.digest(image),
                          image_without_timestamps=build.reproducible_digest(data, [o for _, o, _ in fields]))
        for item in ("layout.json", "gate.json"):
            if (relink_dir / item).is_file():
                shutil.copy(relink_dir / item, out / f"{slug}-relink-{item}")
        summary[name] = record
        print(name, json.dumps(record), flush=True)
        (out / "summary.json").write_text(json.dumps(summary, indent=2) + "\n", encoding="utf-8")
    return 0 if all(r["verify_exit"] == 0 and r["relink_exit"] == 0 for r in summary.values()) else 1


def compare(before_label, after_label):
    before, after = GATES / before_label, GATES / after_label
    old = json.loads((before / "summary.json").read_text(encoding="utf-8"))
    new = json.loads((after / "summary.json").read_text(encoding="utf-8"))
    staged = re.compile(r"sources-[0-9a-f]{32}")
    problems = []
    for name, record in old.items():
        slug = name.replace("/", "-")
        other = new.get(name)
        if other is None:
            problems.append(f"{name}: missing in {after_label}")
            continue
        for key in ("verify_exit", "relink_exit"):
            if record[key] != other[key]:
                problems.append(f"{name}: {key} {record[key]} -> {other[key]}")
        for item in ("results.json", "equivalence.json"):
            a, b = before / f"{slug}-{item}", after / f"{slug}-{item}"
            if a.is_file() != b.is_file() or (a.is_file() and a.read_bytes() != b.read_bytes()):
                problems.append(f"{name}: {item} differs")
        text = [staged.sub("sources-X", (d / f"verify-{slug}.txt").read_text(encoding="utf-8")) for d in (before, after)]
        if text[0] != text[1]:
            problems.append(f"{name}: verifier transcript differs")
        key = "image_sha256" if targets.build_spec(name).primary else "image_without_timestamps"
        if record.get(key) != other.get(key):
            problems.append(f"{name}: relinked image differs ({key})")
        print(f"{name:16} {'same' if not [p for p in problems if p.startswith(name + ':')] else 'DIFFERS'}")
    print("\n".join(problems) or "PASS: every target identical (relinked images apart from timestamps; primary exact)")
    return 1 if problems else 0


def main(argv=None):
    argv = sys.argv[1:] if argv is None else argv
    if len(argv) >= 2 and argv[0] == "run":
        return run(argv[1], argv[2:])
    if len(argv) == 3 and argv[0] == "compare":
        return compare(argv[1], argv[2])
    print(__doc__)
    return 2


if __name__ == "__main__":
    raise SystemExit(main())
