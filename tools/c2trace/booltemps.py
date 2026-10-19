"""SETcc result temps in c2trace captures and how each got its register (diagnostic only, never a match).

A boolean temp is detected at temp assignment (0x435c37) when its defining tuple is `R = 0` (op 1, literal 0) and a
following tuple (op 271, setcc) writes R. For each one this records the function, line, rotor slot, preference,
chosen register and path, plus the preference-pass rule that fired for that range (0x42acf4 copy from a
register-held range, 0x42b051 copy into one, 0x42afe5 cleared) with that tuple's opcode/line and register.
Positive-control search for TreatEventBase's rank-0 toggle (docs/RESIDUAL_CAUSES.md, "Methodology test").

  booltemps.py CAPNAME [CAPNAME...]   captures made by `c2trace.py capture NAME src/X.cpp` (build/c2trace/cap)

Results are appended to build/c2trace/booltemps.jsonl. Run one command at a time (host safety).
"""
import json
import sys
from pathlib import Path

HERE = Path(__file__).resolve().parent
ROOT = HERE.parents[1]
sys.path[:0] = [str(HERE), str(ROOT / "tools")]
import c2dbg  # noqa: E402
from c2dbg import FUNC_MARK, ASSIGN, SET, ROTOR, ROTOR_LIST, REG  # noqa: E402

PREF_FROM, PREF_INTO, PREF_CLR, PREF_TUPLE = 0x42acf4, 0x42b051, 0x42afe5, 0x42afdb
SETCC = 271
CAPS = ROOT / "build/c2trace/cap"


def fname(p):
    try:
        sy = p.u32(p.u32(0x48fb48))
        return p.rd(p.u32(sy + 24), 200).split(bytes([0]))[0].decode("ascii", "replace")
    except Exception:
        return "?"


def chain(p, head, off=0, limit=64):
    seen = set()
    while head and head not in seen and len(seen) < limit:
        seen.add(head)
        yield head
        head = p.u32(head + off)


def run(cap):
    st = dict(rows=[], pend=None, prefs={}, cur_tuple=None)

    def hit(p, ctx, addr):
        if addr == FUNC_MARK:
            return
        if addr == PREF_TUPLE:
            st["cur_tuple"] = p.u32(ctx.Esp + 0x10)
            return
        if addr in (PREF_FROM, PREF_INTO, PREF_CLR):
            lr = ctx.Esi if addr == PREF_FROM else ctx.Ebp
            t = st["cur_tuple"] if addr != PREF_FROM else p.u32(ctx.Esp + 0x10)
            reg = REG.get(p.u32(ctx.Ecx + 0x1c)) if addr != PREF_CLR else None
            info = dict(rule={PREF_FROM: "from", PREF_INTO: "into", PREF_CLR: "clear"}[addr], reg=reg)
            if t:
                info.update(op=p.u32(t + 4), line=p.u32(t + 0x10) & 0xffff)
            lst = st["prefs"].setdefault(lr, [])
            if info["rule"] == "clear" or not lst or lst[-1].get("done"):
                lst.clear()
            lst.append(info)
            return
        if addr == ASSIGN:
            t = ctx.Edx
            lr = p.u32(ctx.Ecx + 0x18)
            st["pend"] = None
            if p.u32(t + 4) != 1:
                return
            src = p.u32(t + 0x18)
            if not src or p.rd(src + 8, 1)[0] != 7 or p.u32(src + 0x18) != 0:
                return
            is_bool = False
            for n in chain(p, p.u32(t), 0, 8):          # look ahead for the setcc writing this range
                if p.u32(n + 4) == SETCC:
                    for u in chain(p, p.u32(n + 0x1c), 0, 8):
                        if p.rd(u + 8, 1)[0] == 1 and p.u32(u + 0x18) == lr:
                            is_bool = True
                    break
            if not is_bool:
                return
            pref = p.u32(lr + 0x2c)
            st["pend"] = dict(func=fname(p), line=p.u32(t + 0x10) & 0xffff, lr=p.u32(lr + 0x1c),
                              rotor=REG.get((p.u32(ROTOR) - ROTOR_LIST) // 4 + 1),
                              pref=REG.get(p.u32(pref + 0x1c)) if pref else None,
                              prefs=list(st["prefs"].get(lr, [])))
            for x in st["prefs"].get(lr, []):
                x["done"] = True
            return
        if addr == SET and st["pend"] is not None:
            row = st["pend"]
            st["pend"] = None
            row["reg"] = REG.get(ctx.Ecx, ctx.Ecx)
            # The rotor path updates its cursor before calling SET. The direct
            # preference path returns at 435db5 without advancing it.
            row["rotor_at_set"] = REG.get((p.u32(ROTOR) - ROTOR_LIST) // 4 + 1)
            row["assignment_return"] = hex(p.u32(ctx.Esp))
            row["path"] = "pref" if row["pref"] and row["pref"] == row["reg"] else (
                "rotor" if row["reg"] == row["rotor"] else "rotor-skip")
            st["rows"].append(row)

    c2dbg.run(cap, [FUNC_MARK, PREF_TUPLE, PREF_FROM, PREF_INTO, PREF_CLR, ASSIGN, SET], hit)
    return st["rows"]


def main():
    log = ROOT / "build/c2trace/booltemps.jsonl"
    log.parent.mkdir(parents=True, exist_ok=True)
    for name in sys.argv[1:]:
        rows = run(CAPS / name)
        with open(log, "a") as fh:
            for r in rows:
                fh.write(json.dumps(dict(cap=name, **r)) + "\n")
        counts = {k: sum(1 for r in rows if r["path"] == k) for k in ("pref", "rotor", "rotor-skip")}
        print(f"{name}: {len(rows)} boolean temps {counts}", flush=True)
        for r in rows:
            if r["path"] != "rotor":
                print(f"   {r['path']:10s} {r['func'][:44]} line {r['line']} rotor {r['rotor']} -> {r['reg']} "
                      + str([(x["rule"], x.get("reg"), x.get("op"), x.get("line")) for x in r["prefs"]]))


if __name__ == "__main__":
    main()
