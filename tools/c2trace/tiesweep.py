"""Is an allocation residue tie-order or cost-quantity? (VC5 SP3 c2, diagnostic only, never a match.)

At each insertion into the global allocation queue (0x41b6fa, ecx = node, edx = queue head; +0x0c signed priority,
+0x40 unsigned definition rank, exact ties newest first), a placement is identity-decided when the node lands in a
group of equal priority. `enum` lists those placements; `flip K MODE` replays c2 and moves only insertion K to the
other end of its equal-priority group (front: rank := 0xffffffff; back: priority := priority - 1) and diffs the
function against the oracle; `sweep` flips every enumerated placement both ways. If no flip gives the oracle, the
residue is quantity-driven (costs, references, benefits), and identity rewrites (interning or declaration order,
BLUPI-ALLOC-15) are not the lever (msvc5kb BLUPI-ALLOC-16; docs/RESIDUAL_CAUSES.md, "Methodology test").

  tiesweep.py enum NAME  [--until-value V --until-refs N]   (stop enumerating after that constant range is queued)
  tiesweep.py flip NAME K front|back
  tiesweep.py sweep NAME [--max N]                          results appended to build/c2trace/tiesweep.jsonl
Common: --function SUB --rva RVA --size ORACLE_BYTES; --caps DIR (default build/c2trace/freelist, where
`freelist.py capture NAME src/X.cpp[=cand.cpp]` writes; `c2trace.py capture` captures are in build/c2trace/cap).
Run one command at a time (host safety); each replay is bounded to 300 s.
"""
import argparse, json, shutil, sys, time, uuid
from pathlib import Path

HERE = Path(__file__).resolve().parent
ROOT = HERE.parents[1]
sys.path[:0] = [str(HERE), str(ROOT / "tools")]
import colour_trace as F  # noqa: E402  (pins the SP3 c2.exe hash)
import freelist  # noqa: E402

SNAPSHOT, INSERT = 0x42491e, 0x41b6fa


def node(p, a):
    d = freelist.describe(p, a)
    d.update(prio=F.signed(p.u32(a + 0x0c)), rank=p.u32(a + 0x40))
    return d


def replay(a, flip=None):
    cap = Path(a.caps) / a.name
    dest = ROOT / "build/c2trace/tiesweep" / f"{a.name}-{uuid.uuid4().hex[:8]}"
    dest.mkdir(parents=True)
    for f in cap.iterdir():
        if f.is_file() and f.suffix not in (".exe", ".obj"):
            shutil.copyfile(f, dest / f.name)
    st = dict(active=False, k=0, rows=[], stop=False)
    started = time.monotonic()

    def hit(p, c, addr):
        if time.monotonic() - started > 300:
            raise KeyboardInterrupt("bounded replay")
        if addr == SNAPSHOT:
            st["active"] = a.function in freelist.function_name(p)
            if st["active"]:
                p.arm(INSERT)
            elif INSERT in p.orig:
                p.wr(INSERT, p.orig.pop(INSERT))
            return
        if not st["active"] or addr != INSERT:
            return
        me = node(p, c.Ecx)
        equal = [x for x in (node(p, q) for q in F.chain(p, c.Edx, 0x14))] if c.Edx else []
        equal = [x for x in equal if x["prio"] == me["prio"]]
        k = st["k"]
        st["k"] += 1
        if equal and not st["stop"]:
            st["rows"].append(dict(k=k, **me, equal=len(equal)))
        if a.until_value is not None and me["value"] == a.until_value and me["refs"] == a.until_refs:
            st["stop"] = True
        if flip and k == flip[0]:
            if flip[1] == "front":
                p.wr(c.Ecx + 0x40, (0xffffffff).to_bytes(4, "little"))
            else:
                p.wr(c.Ecx + 0x0c, ((me["prio"] - 1) & 0xffffffff).to_bytes(4, "little"))

    obj = F.owned_run(dest, (SNAPSHOT,), hit)
    return st["rows"], freelist.oracle_diff(obj, a.function, int(a.rva, 0), a.size)


def main():
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("cmd", choices=["enum", "flip", "sweep"])
    ap.add_argument("name")
    ap.add_argument("args", nargs="*")
    ap.add_argument("--caps", default=str(freelist.WORK))
    ap.add_argument("--function", required=True)
    ap.add_argument("--rva", required=True)
    ap.add_argument("--size", type=int, required=True, help="oracle code size for instruction alignment")
    ap.add_argument("--until-value", type=int)
    ap.add_argument("--until-refs", type=int)
    ap.add_argument("--max", type=int, default=0)
    a = ap.parse_args()
    if a.cmd == "flip":
        print(replay(a, (int(a.args[0]), a.args[1]))[1])
        return
    rows, base = replay(a)
    print(f"{len(rows)} identity-decided insertions; baseline {base}", flush=True)
    if a.cmd == "enum":
        for r in rows:
            print(f"  k={r['k']} lr={r['lr']} kind={r['kind']} value={r['value']} refs={r['refs']} prio={r['prio']} "
                  f"rank={r['rank']} equal={r['equal']}")
        return
    log = ROOT / "build/c2trace/tiesweep.jsonl"
    for r in rows[:a.max or None]:
        for mode in ("front", "back"):
            d = replay(a, (r["k"], mode))[1]
            with open(log, "a") as fh:
                fh.write(json.dumps(dict(cap=a.name, function=a.function, mode=mode, result=d, **r)) + "\n")
            print(f"  k={r['k']:4d} lr={r['lr']:4d} value={r['value']} prio={r['prio']:6d} {mode:5s} {d}", flush=True)


if __name__ == "__main__":
    main()
