"""DIAGNOSTIC ONLY. ctxsweep_all.py START STOP [TU=CAND ...] [--only stem,stem] : for each probe count N, put N
`extern int` probes at the top of every recon TU (candidate files where given), compile each and report the accepted
functions (and DecorNextAction) whose unmasked bytes differ. Simulates a shared-header shift of N counter numbers.
Never source. Same hygiene as ctxsweep_tu.py (fresh folder, stale object removed, `#line 1` after the probes)."""
import json, shutil, subprocess, sys
from pathlib import Path
ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / "tools"))
import cmpfunc, pefile

args = sys.argv[1:]
only = None
if "--only" in args:
    k = args.index("--only"); only = set(args[k + 1].split(",")); del args[k:k + 2]
start, stop = int(args[0]), int(args[1])
cands = dict(a.split("=", 1) for a in args[2:])
recon = json.load(open(ROOT / "recon.json"))
tus = [u["source"] for u in recon["translation_units"]]
acc = {}
for a in recon["accepted_functions"]:
    acc.setdefault(a["tu"], []).append((a["symbol"], int(a["rva"], 16)))
acc.setdefault("src/decblock.cpp", []).append(("?DecorNextAction@CDecor@@QAE?AUtagPOINT@@XZ", 0x5670))
pe = pefile.PE(str(ROOT / "assets/Speedy Blupi/BLUPI.EXE"), fast_load=True)
work = ROOT / "build/tmp/ctxsweep_all"
for n in range(start, stop + 1):
    bad = []
    for tu in tus:
        if tu not in acc or (only and Path(tu).stem not in only):
            continue
        src = (ROOT / cands.get(tu, tu)).read_bytes()
        probes = b"".join(b"extern int ctxsweep_%d;" % i for i in range(n))
        text = (probes + b"\r\n#line 1\r\n" if n else b"") + src
        if work.exists():
            shutil.rmtree(work)
        work.mkdir(parents=True)
        path = work / Path(tu).name
        path.write_bytes(text)
        obj = ROOT / "build/fnprobe/ctxsweep_all" / (Path(tu).stem + ".obj")
        obj.unlink(missing_ok=True)
        subprocess.run([sys.executable, str(ROOT / "tools/fnprobe.py"), f"{tu}={path}", "?", "0x1000", "--name", "ctxsweep_all"],
                       stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
        if not obj.is_file():
            bad.append(f"{Path(tu).stem}:COMPILE-FAILED")
            continue
        funcs = {f.name: f for f in cmpfunc.Coff(obj).functions()}
        for sym, rva in acc[tu]:
            f = funcs.get(sym)
            if f is None:
                continue
            t = pe.get_data(rva, len(f.data)); masked = set()
            for rl in f.relocs:
                masked.update(range(rl.offset - f.start, rl.offset - f.start + 4))
            if any(f.data[i] != t[i] and i not in masked for i in range(len(f.data))):
                bad.append(f"{Path(tu).stem}:{sym.split('@')[0].lstrip('?')}")
    print(n, "ALL ZERO" if not bad else " ".join(bad), flush=True)
