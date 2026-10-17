"""DIAGNOSTIC ONLY. ctxsweep_tu.py TU CAND.cpp START STOP [--extra SYM=RVA ...] [--at ANCHOR] :
insert N `extern int` probes at the top of the candidate TU (or before ANCHOR) and list, per N, every accepted function
of that TU (plus extras) whose unmasked bytes differ. Probes are never source."""
import json, subprocess, sys
from pathlib import Path
ROOT = Path(r"D:\Prog\blupi_recon")
sys.path.insert(0, str(ROOT / "tools"))
import cmpfunc, pefile
args = sys.argv[1:]
extra, anchor = [], None
if "--at" in args:
    k = args.index("--at"); anchor = args[k + 1].encode(); del args[k:k + 2]
if "--extra" in args:
    k = args.index("--extra"); extra = args[k + 1:]; del args[k:]
tu, cand, start, stop = args[0], Path(args[1]), int(args[2]), int(args[3])
acc = [(a["symbol"], int(a["rva"], 16)) for a in json.load(open(ROOT / "recon.json"))["accepted_functions"] if a["tu"] == tu]
acc += [(e.split("=")[0], int(e.split("=")[1], 16)) for e in extra]
pe = pefile.PE(str(ROOT / "assets/Speedy Blupi/BLUPI.EXE"), fast_load=True)
src = cand.read_bytes()
for n in range(start, stop + 1):
    decl = b"".join(b"extern int tusweep_%d; " % i for i in range(n))  # one line: keeps __LINE__
    if anchor:
        p = src.index(anchor)
        text = src[:p] + decl + src[p:]
    else:
        text = decl + src
    d = ROOT / "build/tmp/rx/tusweep"; d.mkdir(parents=True, exist_ok=True)
    p = d / Path(tu).name; p.write_bytes(text)
    subprocess.run([sys.executable, str(ROOT / "tools/fnprobe.py"), f"{tu}={p}", "?", "0x1000", "--name", "tusweep"],
                   stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
    funcs = cmpfunc.Coff(ROOT / "build/fnprobe/tusweep" / (Path(tu).stem + ".obj")).functions()
    bad = []
    for sym, rva in acc:
        f = next((f for f in funcs if f.name == sym), None) or next((f for f in funcs if sym in f.name), None)
        if f is None:
            bad.append(sym.split("@")[0] + "=missing"); continue
        t = pe.get_data(rva, len(f.data)); m = set()
        for rl in f.relocs:
            m.update(range(rl.offset - f.start, rl.offset - f.start + 4))
        c = sum(1 for i in range(len(f.data)) if f.data[i] != t[i] and i not in m)
        if c:
            bad.append(f"{sym.split('@')[0].lstrip('?')}={c}")
    print(n, "ALL ZERO" if not bad else " ".join(bad), flush=True)
