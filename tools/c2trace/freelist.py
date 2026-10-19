"""Range free-list history of one function in VC5 SP3 c2 (diagnostic only, never a match).

  freelist.py capture NAME src/X.cpp[=cand.cpp]      compile the TU (recon.json settings), keep its c2 input
  freelist.py trace NAME [--function SUB]            first 0x41a6f8 drop pass, first group split (0x437e67):
                                                     members, free-list top, children in pop order; diff vs oracle
  freelist.py force NAME LR[,LR...] ...              diagnostic: make the first drop pass drop these ranges too
                                                     (EIP 0x41a775 -> existing drop path 0x41a73e), report the diff

Model (docs/RESIDUAL_CAUSES.md, "Build STRICT"): range descriptors pop from the LIFO list 0x493230 (link +0x2c,
ID +0x1c); 0x41a6f8 drops ranges with < 2 references (0x4375fe) or a single-use copy (0x41a86c); 0x437e67 walks
the exhausted group in ascending LR and each member pops one child; 0x40fbfa (mode 1) materializes boundary
reloads in ascending child ID. Captures live under build/c2trace/freelist. Run one command at a time.
"""
import argparse, hashlib, json, shutil, sys, time, uuid
from pathlib import Path

HERE = Path(__file__).resolve().parent
ROOT = HERE.parents[1]
sys.path[:0] = [str(HERE), str(ROOT / "tools")]
import colour_trace as F  # noqa: E402  (pins the SP3 c2.exe hash)
import cmpfunc, fnprobe, pefile, residues  # noqa: E402

WORK = ROOT / "build/c2trace/freelist"
STUB = ROOT / "build/c2trace/stub/c2stub.exe"
EXE = ROOT / "assets/Speedy Blupi/BLUPI.EXE"
FREEHEAD, SNAPSHOT, DROP_PASS, DROP_CHECK, DROP_PATH = 0x493230, 0x42491e, 0x41a6f8, 0x41a775, 0x41a73e
ALLOC, FREE, REBUILD, CHILD_RET = 0x40ebb6, 0x40ecd2, 0x437e67, 0x4386b9


def capture(name, unit):
    canonical, _, candidate = unit.partition("=")
    if not STUB.is_file():
        sys.exit("run `tools/c2trace/c2trace.py build-stub` first")
    cap = WORK / name
    shutil.rmtree(cap, ignore_errors=True)
    (cap / "il").mkdir(parents=True)
    shutil.copyfile(STUB, cap / "c2stub.exe")
    source = (ROOT / (candidate or canonical)).resolve()
    obj = fnprobe.compile_unit(canonical.replace("\\", "/"), source, cap, ["/B2" + str(cap / "c2stub.exe")])
    for f in (cap / "il").iterdir():
        if f.is_file():
            shutil.copyfile(f, cap / f.name)
    (cap / "capture.json").write_text(json.dumps(dict(object=str(obj), source=str(source), unit=canonical,
                                                      sha256=hashlib.sha256(source.read_bytes()).hexdigest())))
    print(cap)


def function_name(p):
    sy = p.u32(p.u32(0x48fb48))
    return p.rd(p.u32(sy + 24), 200).split(b"\0")[0].decode("ascii", "replace")


def describe(p, a):
    n = F.words(p, a, 17)
    d = n[0]
    kind = p.rd(d + 4, 1)[0] if d else None
    value = None
    if kind == 13:
        literal = p.u32(d + 0x28)
        if literal and p.rd(literal + 8, 1)[0] == 7:
            value = F.signed(p.u32(literal + 0x18))
    return dict(lr=n[7], kind=kind, source_id=p.u32(d + 28) if d else None, refs=n[9], value=value)


def free_list(p, limit=64):
    out, a = [], p.u32(FREEHEAD)
    while a and len(out) < limit:
        out.append(p.u32(a + 0x1c))
        a = p.u32(a + 0x2c)
    return out


def replay(name, selector, force=()):
    cap = WORK / name
    dest = WORK / "replays" / f"{name}-{uuid.uuid4().hex[:8]}"
    dest.mkdir(parents=True)
    for f in cap.iterdir():
        if f.is_file() and f.suffix not in (".exe", ".obj"):
            shutil.copyfile(f, dest / f.name)
    st = dict(active=False, passes=0, rebuilds=0, frees=[], forced=[], group=None, top=None, children=[])
    pending, started = {}, time.monotonic()

    def hit(p, c, a):
        if time.monotonic() - started > 300:
            raise KeyboardInterrupt("bounded replay")
        if pending.get(a):
            pending[a].pop()(p, c)
            return
        if a == SNAPSHOT:
            st["active"] = selector in function_name(p)
            for q in (DROP_PASS, DROP_CHECK, FREE, ALLOC, REBUILD):
                if st["active"]:
                    p.arm(q)
                elif q in p.orig:
                    p.wr(q, p.orig.pop(q))
            return
        if not st["active"]:
            return
        if a == DROP_PASS:
            st["passes"] += 1
        elif a == DROP_CHECK and st["passes"] == 1 and p.u32(c.Eax + 0x1c) in force:
            st["forced"].append(describe(p, c.Eax))
            c.Eip = DROP_PATH
        elif a == FREE and st["rebuilds"] == 0:
            st["frees"].append(dict(caller=hex(p.u32(c.Esp)), **describe(p, c.Ecx)))
        elif a == REBUILD:
            st["rebuilds"] += 1
            if st["rebuilds"] == 1:
                st["group"], st["top"] = F.bitset(p, c.Edx), free_list(p)
        elif a == ALLOC and st["rebuilds"] == 1 and p.u32(c.Esp) == CHILD_RET:
            pending.setdefault(CHILD_RET, []).append(lambda p, c: st["children"].append(describe(p, c.Eax)))
            p.arm(CHILD_RET)

    obj = F.owned_run(dest, (SNAPSHOT,), hit)
    return st, obj


def oracle_diff(obj, selector, rva, size):
    func = next(f for f in cmpfunc.Coff(obj).functions() if selector in f.name)
    pe = pefile.PE(str(EXE), fast_load=True)
    target = pe.get_data(rva, len(func.data))
    masked = {i for r in func.relocs for i in range(r.offset - func.start, r.offset - func.start + 4)}
    diffs = [k for k in range(len(func.data)) if func.data[k] != target[k] and k not in masked]
    res = residues.analyse(func, cmpfunc.Coff(obj), pe.get_data(rva, max(len(func.data), size) + 1024),
                           pe.OPTIONAL_HEADER.ImageBase + rva, size)
    return f"masked differing bytes={len(diffs)} aligned={res['equal_insns']}/{res['oracle_insns']}"


def main():
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("cmd", choices=["capture", "trace", "force"])
    ap.add_argument("name")
    ap.add_argument("args", nargs="*")
    ap.add_argument("--function", default="?Build@CDecor@@")
    ap.add_argument("--rva", default="0x4350")
    ap.add_argument("--size", type=int, default=3866, help="oracle code size for instruction alignment")
    a = ap.parse_args()
    if a.cmd == "capture":
        return capture(a.name, a.args[0])
    sets = [set()] if a.cmd == "trace" else [{int(x) for x in s.split(",")} for s in a.args]
    for force in sets:
        st, obj = replay(a.name, a.function, force)
        last_lea = max([i for i, f in enumerate(st["frees"]) if f["caller"] == "0x41ccbc"], default=-1)
        print(f"force={sorted(force)} forced={[(x['lr'], x['refs']) for x in st['forced']]}")
        print(f"  frees after the LEA batch (top of list): {len(st['frees']) - 1 - last_lea}; "
              f"group={len(st['group'] or [])} members {st['group']}")
        print(f"  free-list top {st['top'][:36]}")
        print("  children: " + " ".join(f"{c['lr']}:{'#%s' % c['value'] if c['kind'] == 13 else 's%s' % c['source_id']}"
                                       for c in st["children"]))
        print("  " + oracle_diff(obj, a.function, int(a.rva, 0), a.size))


if __name__ == "__main__":
    main()
