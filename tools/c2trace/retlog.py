"""retlog.py: log (function, return value) for every direct call into c2.exe functions during a traced run, so two runs can
be diffed. Function entries come from a Ghidra function list (build/c2/out/calls.tsv, first column). Diagnostic only."""
import sys
from pathlib import Path

HERE = Path(__file__).resolve().parent
sys.path.insert(0, str(HERE))
import c2dbg  # noqa: E402


def entries(tsv):
    out = []
    for line in open(tsv):
        a = line.split("\t")[0]
        if ":" not in a:
            out.append(int(a, 16))
    return out


def trace(capdir, funcs, perturb=None, extra=()):
    """Returns the list of events: ('R', func, eax) at each return, ('F',) at each function start marker,
    ('A', k) at each temp assignment. PERTURB as in c2dbg.assigns (diagnostic only)."""
    funcset = set(funcs)
    persistent = funcset | {c2dbg.FUNC_MARK, c2dbg.ASSIGN} | set(extra)
    events, pending = [], {}
    state = {"f": -1, "k": 0}

    def hit(p, ctx, addr):
        if addr == c2dbg.FUNC_MARK:
            state["f"] += 1
            state["k"] = 0
            events.append(("F",))
        if addr == c2dbg.ASSIGN:
            if perturb and (state["f"], state["k"]) == tuple(perturb[:2]):
                slot = ((p.u32(c2dbg.ROTOR) - c2dbg.ROTOR_LIST) // 4 + perturb[2]) % 3
                p.wr(c2dbg.ROTOR, (c2dbg.ROTOR_LIST + 4 * slot).to_bytes(4, "little"))
            events.append(("A", state["k"]))
            state["k"] += 1
        if addr in pending:
            stack = pending[addr]
            f = stack.pop()
            events.append(("R", f, ctx.Eax))
            if not stack:
                del pending[addr]
                if addr not in persistent:
                    p.oneshot.add(addr)
            else:
                p.oneshot.discard(addr)
        if addr in funcset:
            ra = p.u32(ctx.Esp)
            code = p.rd(ra - 5, 5)
            if code[0] == 0xE8 and (ra + int.from_bytes(code[1:], "little", signed=True)) & 0xFFFFFFFF == addr:
                pending.setdefault(ra, []).append(addr)
                if not p.arm(ra, oneshot=ra not in persistent):
                    p.oneshot.discard(ra)

    c2dbg.run(Path(capdir), sorted(persistent), hit)
    return events
