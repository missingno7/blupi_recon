"""Runs tools/cmpfunc.py for a target other than the primary one.

    python tools/cmpfunc_target.py --oracle-dir oracle/sb1-eggbert-en OBJECTS... --exe EXE ...

cmpfunc.py reads the oracle inventory (functions.csv, tu_map.csv) from
oracle/, the primary target's. Its bytes are pinned by the equivalence
certificates (evidence/equivalence/), so instead of changing it this wrapper
points those two readers at the target's inventory folder and then runs
cmpfunc.main() unchanged. The primary target never goes through here.
"""
from __future__ import annotations

import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))
import cmpfunc  # noqa: E402


def main() -> int:
    argv = sys.argv[1:]
    if "--oracle-dir" not in argv:
        print("--oracle-dir is required (the primary target uses cmpfunc.py directly)")
        return 2
    i = argv.index("--oracle-dir")
    oracle_dir = Path(argv[i + 1]).resolve()
    del argv[i:i + 2]
    for name in ("functions.csv", "tu_map.csv"):
        if not (oracle_dir / name).is_file():
            print(f"missing {oracle_dir / name}")
            return 2
    starts, tus = cmpfunc.oracle_function_starts, cmpfunc.oracle_tu_at_rva
    cmpfunc.oracle_function_starts = lambda path=oracle_dir / "functions.csv": starts(path)
    cmpfunc.oracle_tu_at_rva = lambda path=oracle_dir / "tu_map.csv": tus(path)
    # the CRT data-binding check reads ROOT / "oracle" / "functions.csv" at call time
    shadow = Path(__file__).resolve().parents[1] / "build" / "targets" / oracle_dir.name / "root"
    (shadow / "oracle").mkdir(parents=True, exist_ok=True)
    for name in ("functions.csv", "tu_map.csv"):
        (shadow / "oracle" / name).write_bytes((oracle_dir / name).read_bytes())
    cmpfunc.ROOT = shadow
    sys.argv = [sys.argv[0], *argv]
    return cmpfunc.main()


if __name__ == "__main__":
    sys.exit(main())
