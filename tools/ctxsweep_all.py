"""DIAGNOSTIC ONLY. ctxsweep_all.py START STOP [TU=CAND ...] [--only tu,tu] : for each probe count N, put N `extern int`
probes at the top of every recon TU (candidate files where given), compile each and report the accepted functions (and
DecorNextAction) whose unmasked bytes differ. Simulates a shared-header shift of N counter numbers. Never source."""
import json, subprocess, sys
from pathlib import Path
ROOT = Path(r"D:\Prog\blupi_recon")
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
for n in range(start, stop + 1):
    bad = []
    for tu in tus:
        if tu not in acc or (only and Path(tu).stem not in only):
            continue
        src = (ROOT / cands.get(tu, tu)).read_bytes()
        text = b"".join(b"extern int allsweep_%d; " % i for i in range(n)) + src  # one line: keeps __LINE__
        d = ROOT / "build/tmp/rx/allsweep"; d.mkdir(parents=True, exist_ok=True)
        p = d / Path(tu).name; p.write_bytes(text)
        subprocess.run([sys.executable, str(ROOT / "tools/fnprobe.py"), f"{tu}={p}", "?", "0x1000", "--name", "allsweep"],
                       stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
        obj = ROOT / "build/fnprobe/allsweep" / (Path(tu).stem + ".obj")
        funcs = {f.name: f for f in cmpfunc.Coff(obj).functions()}
        for sym, rva in acc[tu]:
            f = funcs.get(sym)
            if f is None:
                continue
            t = pe.get_data(rva, len(f.data)); m = set()
            for rl in f.relocs:
                m.update(range(rl.offset - f.start, rl.offset - f.start + 4))
            if any(f.data[i] != t[i] and i not in m for i in range(len(f.data))):
                bad.append(f"{Path(tu).stem}:{sym.split('@')[0].lstrip('?')}")
    print(n, "ALL ZERO" if not bad else " ".join(bad), flush=True)
