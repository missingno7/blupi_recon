"""DIAGNOSTIC ONLY. ctxsweep_tu.py TU CAND.cpp START STOP [--extra SYM=RVA ...] [--at ANCHOR] :
insert N `extern int` probes at the top of the candidate TU (or before ANCHOR) and list, per N, every accepted function
of that TU (plus extras) whose unmasked bytes differ. Probes are never source.

The probe file is written to a freshly emptied folder (stale private headers must never shadow quoted includes), the
object is deleted before each compile, and top-of-file probes are followed by `#line 1` so __LINE__ is unchanged
(diagnostic file only)."""
import json, shutil, subprocess, sys
from pathlib import Path
ROOT = Path(__file__).resolve().parents[1]
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
work = ROOT / "build/tmp/ctxsweep_tu"
obj = ROOT / "build/fnprobe/ctxsweep_tu" / (Path(tu).stem + ".obj")
for n in range(start, stop + 1):
    probes = b"".join(b"extern int ctxsweep_%d;" % i for i in range(n))
    if anchor:
        p = src.index(anchor)
        text = src[:p] + probes + src[p:]
    else:
        text = (probes + b"\r\n#line 1\r\n" if n else b"") + src
    if work.exists():
        shutil.rmtree(work)
    work.mkdir(parents=True)
    path = work / Path(tu).name
    path.write_bytes(text)
    obj.unlink(missing_ok=True)
    r = subprocess.run([sys.executable, str(ROOT / "tools/fnprobe.py"), f"{tu}={path}", "?", "0x1000", "--name", "ctxsweep_tu"],
                       stdout=subprocess.PIPE, stderr=subprocess.STDOUT, text=True)
    if not obj.is_file():
        print(n, "COMPILE FAILED", r.stdout[-300:].replace("\n", " "), flush=True)
        continue
    funcs = cmpfunc.Coff(obj).functions()
    bad = []
    for sym, rva in acc:
        f = next((f for f in funcs if f.name == sym), None)
        if f is None:
            bad.append(sym.split("@")[0].lstrip("?") + "=missing")
            continue
        t = pe.get_data(rva, len(f.data)); masked = set()
        for rl in f.relocs:
            masked.update(range(rl.offset - f.start, rl.offset - f.start + 4))
        c = sum(1 for i in range(len(f.data)) if f.data[i] != t[i] and i not in masked)
        if c:
            bad.append(f"{sym.split('@')[0].lstrip('?')}={c}")
    print(n, "ALL ZERO" if not bad else " ".join(bad), flush=True)
