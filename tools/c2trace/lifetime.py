"""Where does a source construct die inside VC5 SP3 c2? (diagnostic only, never a match)

One c2 replay of a capture. For every function it counts tuples, blocks and the highest range id at three stages:
read = 0x42491e (function snapshot after the C1 IL is read, all tuples still C1 opcodes), cost = first entry of the
allocation cost pass 0x40ee1d (after early C2), pref = preference pass 0x42ac33 (after global allocation, before temp
assignment). It also lists the temp assignments (rotor slot > register, * = preference) and whether each function's
bytes equal a base function's. Compare a construct variant with its base: a larger read count and equal cost/pref
counts mean the construct reached C1 output and was removed by early C2; a different id high-water mark is an
identity residue (BLUPI-STEER-01); different cost counts mean it survives into allocation.

  lifetime.py CAPNAME [BASE_SUBSTR]          captures from `c2trace.py capture` / `capture-micro` (build/c2trace/cap)
  lifetime.py CAPNAME BASE --lines FUNC A-B  per-line tuple ops of one function at the three stages
Run one command at a time (host safety).
"""
import argparse, collections, sys
from pathlib import Path

HERE = Path(__file__).resolve().parent
sys.path[:0] = [str(HERE), str(HERE.parent)]
import c2dbg, cmpfunc  # noqa: E402
from c2dbg import ASSIGN, SET, REG, ROTOR, ROTOR_LIST  # noqa: E402

STAGES = {0x42491e: "read", 0x40ee1d: "cost", 0x42ac33: "pref"}
CAPS = HERE.parents[1] / "build/c2trace/cap"


def chain(p, head, off=0, limit=20000):
    seen = set()
    while head and head not in seen and len(seen) < limit:
        seen.add(head)
        yield head
        head = p.u32(head + off)


def fname(p):
    sy = p.u32(p.u32(0x48fb48))
    return p.rd(p.u32(sy + 24), 200).split(b"\0")[0].decode("ascii", "replace")


def opnd(p, u):
    tag = p.rd(u + 8, 1)[0]
    if tag == 1:
        r = p.u32(u + 0x18)
        rd = p.u32(r + 8)
        reg = REG.get(p.u32(rd + 0x1c)) if rd else None
        return f"R{p.u32(r + 0x1c)}c{p.rd(r + 4, 1)[0]}{'=' + reg if reg else ''}"
    return f"#{p.u32(u + 0x18)}" if tag == 7 else f"t{tag}"


def walk(p, lines=None):
    """(tuples, blocks, max range id, {line: [op text]})"""
    n = nb = mx = 0
    per = collections.defaultdict(list)
    for bi, bb in enumerate(chain(p, p.u32(p.u32(p.u32(0x48fb48) + 8)))):
        nb += 1
        end = p.u32(bb + 0x20)
        for t in chain(p, p.u32(bb + 0x1c)):
            n += 1
            rd, wr = p.u32(t + 0x18), p.u32(t + 0x1c)
            for head in (rd, wr):
                for u in chain(p, head, limit=16):
                    if p.rd(u + 8, 1)[0] == 1:
                        mx = max(mx, p.u32(p.u32(u + 0x18) + 0x1c))
            line = p.u32(t + 0x10) & 0xffff
            if lines and line in lines:
                per[line].append(f"B{bi}:op{p.u32(t + 4)} r={[opnd(p, u) for u in chain(p, rd, limit=8)]} "
                                 f"w={[opnd(p, u) for u in chain(p, wr, limit=8)]}")
            if t == end:
                break
    return n, nb, mx, per


def run(cap, lines_for=None, lines=None):
    st = dict(fn=None, seen=set(), stages=collections.defaultdict(dict), assigns=collections.defaultdict(list))

    def hit(p, c, a):
        if a == 0x42491e:
            st["fn"], st["seen"] = fname(p), set()
        if a in STAGES and st["fn"] and STAGES[a] not in st["seen"]:
            st["seen"].add(STAGES[a])
            want = lines if lines_for and lines_for in st["fn"] else None
            st["stages"][st["fn"]][STAGES[a]] = walk(p, want)
        if a == ASSIGN and st["fn"]:
            lr = p.u32(c.Ecx + 0x18)
            pref = p.u32(lr + 0x2c)
            st["assigns"][st["fn"]].append([REG.get((p.u32(ROTOR) - ROTOR_LIST) // 4 + 1),
                                            REG.get(p.u32(pref + 0x1c)) if pref else None, None])
        if a == SET and st["fn"] and st["assigns"][st["fn"]] and st["assigns"][st["fn"]][-1][2] is None:
            st["assigns"][st["fn"]][-1][2] = REG.get(c.Ecx, c.Ecx)

    obj = c2dbg.run(cap, [0x42491e, 0x40ee1d, 0x42ac33, ASSIGN, SET], hit)
    return st, {f.name: f for f in cmpfunc.Coff(obj).functions()}


def main():
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("cap")
    ap.add_argument("base", nargs="?")
    ap.add_argument("--lines", nargs=2, metavar=("FUNC", "A-B"))
    a = ap.parse_args()
    lines = None
    if a.lines:
        lo, _, hi = a.lines[1].partition("-")
        lines = set(range(int(lo), int(hi or lo) + 1))
    st, funcs = run(CAPS / a.cap, a.lines[0] if a.lines else None, lines)
    base = next((n for n in funcs if a.base and a.base in n), None)
    for n, s in st["stages"].items():
        cell = lambda k: s.get(k, (0, 0, 0, {}))
        temps = " ".join(f"{x[0][1]}>{x[2]}{'*' if x[1] and x[1] == x[2] else ''}" for x in st["assigns"][n])
        f = funcs.get(n)
        same = (f.data == funcs[base].data) if (f and base) else "-"
        print(f"{n[:40]:40s} read={cell('read')[0]:4d} cost={cell('cost')[0]:4d}/b{cell('cost')[1]:3d}/id{cell('cost')[2]:4d} "
              f"pref={cell('pref')[0]:4d} size={len(f.data) if f else '-'} same_as_base={same} [{temps}]", flush=True)
        if lines and a.lines[0] in n:
            for stg in ("read", "cost", "pref"):
                print(f"  [{stg}]")
                for line, ops in sorted(cell(stg)[3].items()):
                    print(f"    L{line}: " + " | ".join(ops))


if __name__ == "__main__":
    main()
