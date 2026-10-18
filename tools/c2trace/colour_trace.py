"""Observe VC5 SP3 global allocation without changing compiler decisions.

  .pyenv\python.exe tools/c2trace/colour_trace.py CAP --function 0 --out build/c2trace/colour/trace.json

CAP is a capture directory (flags.txt and IL), or a name under build/c2trace/cap.
--function uses the zero-based FUNC_MARK / compile order used by c2trace.py.
Use 51 for ChangePhase in the existing event captures, 15 for Build in decblock.
Copies the capture before replay: shared inputs and trace.obj are untouched.
Outputs EVERY sort insertion, dequeue/requeue, selection, rematerialization and
stack-home reference, plus JSON with sparse sets and independently predicted
queue insertion / register penalties. --uses includes exact tuple use chains.
Software breakpoints are the only writes to the debuggee. No perturbation.
The callback records errors and lets c2 finish before reporting failure.
"""
import argparse
import ctypes
from ctypes import wintypes as W
import hashlib
import json
import os
import shutil
import sys
import time
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
HERE = Path(__file__).resolve().parent
sys.path[:0] = [str(ROOT / "tools/c2trace"), str(ROOT / "tools")]
import c2dbg as dbg

REG = dbg.REG
PIN = hashlib.sha256(Path(dbg.C2).read_bytes()).hexdigest()
EXPECTED_PIN = "e75aecaf4073b0817ffb638cae5fff636b2e2d1a090daabf8f68dbc954515fae"
if PIN != EXPECTED_PIN:
    raise RuntimeError("Unsupported c2.exe image; fixed addresses require VC5 SP3 " + EXPECTED_PIN)


def owned_run(cap, breakpoints, callback):
    """Keep side outputs private when replaying captures from read-only projects."""
    cap = cap.resolve()
    if not any(cap.is_relative_to(ROOT / subdir) for subdir in ("scratch", "build")):
        raise ValueError("Replay output must be under this repo's scratch/ or build/")
    flagfile = cap / "flags.txt"
    parts = flagfile.read_text().split()
    outputs = {"-Fd": "compiler.idb", "-Fa": "listing.asm", "-Fr": "browse.sbr", "-FR": "browse.sbr"}
    rewritten = []
    i = 0
    while i < len(parts):
        part = parts[i]
        prefix = next((s for s in outputs if part.startswith(s)), None)
        if prefix:
            if part == prefix and i+1 < len(parts) and not parts[i+1].startswith("-"):
                i += 1
            part = prefix + str(cap / outputs[prefix])
        rewritten.append(part)
        i += 1
    flagfile.write_text(" ".join(rewritten) + "\n")
    previous = Path.cwd()
    k = dbg.K
    k.GetCurrentProcess.restype = W.HANDLE
    k.GetProcessId.argtypes, k.GetProcessId.restype = [W.HANDLE], W.DWORD
    k.DuplicateHandle.argtypes = [W.HANDLE, W.HANDLE, W.HANDLE, ctypes.POINTER(W.HANDLE), W.DWORD, W.BOOL, W.DWORD]
    k.TerminateProcess.argtypes = [W.HANDLE, W.UINT]
    k.WaitForSingleObject.argtypes, k.WaitForSingleObject.restype = [W.HANDLE, W.DWORD], W.DWORD
    k.GetExitCodeProcess.argtypes = [W.HANDLE, ctypes.POINTER(W.DWORD)]
    k.DebugActiveProcessStop.argtypes = [W.DWORD]
    retained = W.HANDLE()
    original_process = dbg.Process

    def reap(handle, terminate=False):
        if terminate:
            pid = k.GetProcessId(handle)
            k.TerminateProcess(handle, 0xc2a110c)
            k.DebugActiveProcessStop(pid)
        if k.WaitForSingleObject(handle, 10000) != 0:
            raise RuntimeError("c2 process exit wait failed")

    class RetainedProcess(original_process):
        def __init__(self, handle):
            super().__init__(handle)
            current = k.GetCurrentProcess()
            if not k.DuplicateHandle(current, handle, current, ctypes.byref(retained), 0, False, 2):
                reap(handle, terminate=True)
                raise OSError(ctypes.get_last_error(), "Cannot retain c2 exit handle")

    try:
        os.chdir(cap)
        dbg.Process = RetainedProcess
        obj = dbg.run(cap, breakpoints, callback)
        reap(retained)
        code = W.DWORD()
        k.GetExitCodeProcess(retained, ctypes.byref(code))
        if code.value:
            raise RuntimeError(f"c2 exited with {code.value:#x}")
        if not obj.is_file() or obj.stat().st_size < 20:
            raise RuntimeError("c2 produced no complete COFF object")
        return obj
    except BaseException:
        if retained:
            reap(retained, terminate=True)
        raise
    finally:
        dbg.Process = original_process
        if retained:
            k.CloseHandle(retained)
        os.chdir(previous)


def signed(x):
    return x - 0x100000000 if x & 0x80000000 else x


def words(p, a, n=17):
    b = p.rd(a, n * 4)
    return [int.from_bytes(b[k:k + 4], "little") for k in range(0, n * 4, 4)]


def chain(p, head, offset=0, limit=10000):
    seen = set()
    while head and head not in seen and len(seen) < limit:
        seen.add(head)
        yield head
        head = p.u32(head + offset)


def bitset(p, a):
    return sorted(base + bit for node in chain(p, p.u32(a) if a else 0, 4)
                  for base, _, bits in [words(p, node, 3)]
                  for bit in range(32) if bits & (1 << bit))


def ranges(p):
    out = {}
    for head in words(p, 0x493238, 1024):
        for a in chain(p, head, 0x2c):
            out[p.u32(a + 0x1c)] = a
    return out


def prefs(p, a):
    return [dict(reg=p.u32(p.u32(n + 4) + 0x1c), weight=p.u32(n + 8))
            for n in chain(p, p.u32(a + 0x34))]


def blocks(p, root):
    if getattr(p, "alloc_cache", {}).get("root") != root:
        p.alloc_cache = dict(root=root)
    if "blocks" not in p.alloc_cache:
        p.alloc_cache["blocks"] = list(chain(p, p.u32(p.u32(root + 8))))
    return p.alloc_cache["blocks"]


def uses(p, root, target):
    result = []
    nearest = 0
    for bi, bb in enumerate(blocks(p, root)):
        end = p.u32(bb + 0x20)
        for t in chain(p, p.u32(bb + 0x1c)):
            line = p.u32(t + 0x10) & 0xffff
            nearest = line or nearest
            for role, off in [("read", 0x18), ("write", 0x1c)]:
                for u in chain(p, p.u32(t + off), limit=256):
                    if p.rd(u + 8, 1)[0] == 1 and p.u32(u + 0x18) == target:
                        result.append(dict(block=bi, tuple=hex(t), op=p.u32(t + 4),
                                           line=line, nearest_line=nearest, role=role,
                                           flags=hex(p.u32(u + 0x10)), use=hex(u)))
            if t == end:
                break
    return result


def describe(p, root, a, include_uses=False):
    n = words(p, a)
    operand = n[0]
    kind = p.rd(operand + 4, 1)[0] if operand else 0
    val = None
    if kind == 13:
        orig = p.u32(operand + 0x28)
        if p.rd(orig + 8, 1)[0] == 7:
            val = signed(p.u32(orig + 0x18))
    bs = blocks(p, root)
    first = bs.index(n[10]) if n[10] in bs else None
    last = bs.index(n[12]) if n[12] in bs else None
    row = dict(ptr=hex(a), lr=n[7], kind=kind, value=val, refs=n[9],
               operand=hex(operand), source_id=p.u32(operand + 0x1c) if operand else None,
               priority=signed(n[3]), benefit=signed(n[15]), tie=n[16],
               first=first, last=last, span_blocks=last - first + 1 if first is not None and last is not None else None,
               available=bitset(p, n[8]), preferences=prefs(p, a), raw=n)
    if "live_sets" not in p.alloc_cache:
        p.alloc_cache["live_sets"] = [set(bitset(p, p.u32(bb + 0x48))) for bb in bs]
    row["occupied_blocks"] = [bi for bi, live in enumerate(p.alloc_cache["live_sets"]) if n[7] in live]
    if include_uses:
        row["uses"] = uses(p, root, a)
    return row


def queue(p, head):
    return [dict(ptr=hex(n), lr=p.u32(n + 0x1c), priority=signed(p.u32(n + 0xc)), tie=p.u32(n + 0x40))
            for n in chain(p, head, 0x14)]


def insert_prediction(q, row):
    key = (row["priority"], row["tie"])
    k = next((i for i, x in enumerate(q) if key >= (x["priority"], x["tie"])), len(q))
    adjacent = q[k] if k < len(q) else (q[-1] if q else None)
    reason = "empty" if not adjacent else (
        "signed-priority" if row["priority"] != adjacent["priority"] else
        "unsigned-+40" if row["tie"] != adjacent["tie"] else "exact-tie-new-before-old")
    return k, reason


def stack_order(p):
    out = []
    for a in chain(p, p.u32(0x4910ac), 0x2c):
        n = words(p, a, 15)
        kind = n[1] & 255
        out.append(dict(ptr=hex(a), kind=kind, id=n[7], size=n[8], weight=signed(n[13]),
                        slot=n[14], native_operand=hex(n[0]),
                        offset=signed(p.u32(n[0] + 0xc) if kind == 4 else n[10]), raw=n))
    return out


def slot_groups(p):
    base, count = p.u32(0x4910b8), p.u32(0x4910c0)
    if count > 10000:
        raise ValueError("implausible stack slot count")
    return [dict(index=i, identities=bitset(p, p.u32(base + 20*i)),
                 forbidden=bitset(p, p.u32(base + 20*i + 4)),
                 size=p.u32(base + 20*i + 8), weight=signed(p.u32(base + 20*i + 12)))
            for i in range(count)]


def predict_packing(p, root, order, mode):
    """Independent ordinary-slot model; explicit exclusions for special branches."""
    if any(n["raw"][1] & 0x40000 for n in order):
        return dict(limit="per-node 40d1e6 interference expansion is not modeled")
    matrix = p.u32(0x4910c8)
    interference = {n["slot"]: set(bitset(p, p.u32(matrix + 4*n["slot"]))) for n in order}
    fixed = [n for n in order if n["kind"] == 5]
    groups = []
    for n in fixed:
        native = int(n["native_operand"], 16)
        groups.append(dict(identities={n["slot"]}, forbidden=set(interference[n["slot"]]),
                           size=(p.u32(native + 0x10) + 3) & ~3,
                           fixed_offset=signed(p.u32(native + 0xc)), weight=0))
    seed_count = len(groups)
    allow_fixed = not (p.u32(root + 0x34) & (0x40 | 0x300000))
    def pack(groups, seed_count, allow_fixed):
        reuses = 0
        for n in order:
            if n["kind"] == 5:
                continue
            identity, size = n["slot"], n["size"]
            near = interference[identity]
            target = None
            if allow_fixed:
                target = next((i for i, g in enumerate(groups[:seed_count])
                               if size <= g["size"] and identity not in g["forbidden"] and not (g["identities"] & near)), None)
            if target is None:
                target = next((i for i in range(len(groups)-1, seed_count-1, -1)
                               if size <= 2*groups[i]["size"] and identity not in groups[i]["forbidden"]
                               and not (groups[i]["identities"] & near)), None)
            if target is None:
                groups.append(dict(identities={identity}, forbidden=set(near), size=size, weight=n["weight"]))
            else:
                g = groups[target]
                g["identities"].add(identity)
                g["forbidden"].update(near)
                g["size"] = max(g["size"], size)
                if target < seed_count:
                    reuses += 1
                else:
                    g["weight"] += n["weight"]
        return groups, reuses
    groups, fixed_reuses = pack(groups, seed_count, allow_fixed)
    ordinary_bytes = sum(g["size"] for g in groups[seed_count:])
    restarted = bool(mode and fixed_reuses and ordinary_bytes > 111)
    if restarted:
        seed_count = 0
        groups, _ = pack([], 0, False)
        ordinary_bytes = sum(g["size"] for g in groups)
    def density(g):
        x = signed((g["weight"] * 1000) & 0xffffffff)
        return x // g["size"] if x >= 0 else -((-x)//g["size"])
    def sort(lo, hi):
        while lo < hi:
            mid = (lo + hi)//2
            groups[lo], groups[mid] = groups[mid], groups[lo]
            k = lo
            for j in range(lo+1, hi+1):
                if density(groups[j]) > density(groups[lo]):
                    k += 1
                    groups[k], groups[j] = groups[j], groups[k]
            groups[lo], groups[k] = groups[k], groups[lo]
            sort(lo, k-1)
            lo = k+1
    if ordinary_bytes > 128:
        sort(seed_count, len(groups)-1)
    cursor = signed(p.u32(0x49107c))
    alignment = p.u32(0x495f24)
    before = cursor
    bias = signed(p.u32(0x4911ac))
    offsets = {}
    def align(x, size, boundary):
        if size == 2 and boundary > 1:
            return (x+1) & ~1
        return (x+boundary-1) & ~(boundary-1) if size != 1 else x
    def allocate(size, boundary):
        nonlocal cursor
        if boundary:
            size = (size+boundary-1) & ~(boundary-1)
        cursor = -align(size-cursor, size, boundary or alignment)
        return cursor
    typed = {n["slot"]: p.u32(int(n["ptr"], 16)+0x10) & 0xffff for n in order}
    for i in (range(len(groups)-1, -1, -1) if mode else range(len(groups))):
        g = groups[i]
        if i < seed_count:
            offset = g["fixed_offset"]
        else:
            boundary = 8 if mode and p.u32(root+0x34) & 0x300000 and any(typed[x] == 0x4008 for x in g["identities"]) else 0
            offset = allocate(g["size"], boundary) + (0 if mode else bias)
        for n in order:
            if n["slot"] in g["identities"]:
                offsets[n["slot"]] = offset if mode else -align(n["size"]-g["size"]-offset, n["size"], alignment)
    return dict(groups=[dict(index=i, identities=sorted(g["identities"]), forbidden=sorted(g["forbidden"]), size=g["size"], weight=g["weight"])
                        for i, g in enumerate(groups)],
                interference={k: sorted(v) for k, v in interference.items()},
                restarted=restarted, ordinary_bytes=ordinary_bytes, frame_mode=mode,
                frame_cursor_before=before, frame_cursor_after=cursor, alignment=alignment,
                offsets=offsets)


def physical_scores(p):
    order = []
    for r in words(p, 0x491100, 9):
        if not r:
            break
        order.append(r)
    return dict(order=order, scores={r: signed(p.u32(0x494758 + r * 0x50 + 0x34)) for r in order})


def check_stack_history(events):
    """Predict the 4367f0 list from the reference stream, before reading its final order."""
    ordered = []
    nodes = {}
    for e in events:
        if e["stage"] == "stack-reference":
            ptr = e["ptr"]
            if ptr not in nodes:
                nodes[ptr] = dict(ptr=ptr, size=e["size"], weight=e["add"])
                position = next((i for i, n in enumerate(ordered) if n["size"] > e["size"]), len(ordered))
                ordered.insert(position, nodes[ptr])
            else:
                node = nodes[ptr]
                node["weight"] += e["add"]
                i = ordered.index(node)
                while i > 0 and ordered[i - 1]["size"] == node["size"] and ordered[i - 1]["weight"] < node["weight"]:
                    ordered[i], ordered[i - 1] = ordered[i - 1], ordered[i]
                    i -= 1
        elif e["stage"] == "stack-ordered":
            actual = e["order"]
            present = {n["ptr"] for n in actual}
            predicted = [dict(n) for n in ordered if n["ptr"] in present]
            e["predicted_order"] = predicted
            e["history_prediction_ok"] = [n["ptr"] for n in predicted] == [n["ptr"] for n in actual]
            e["weight_prediction_ok"] = all(n["ptr"] in nodes and n["weight"] == nodes[n["ptr"]]["weight"] for n in actual)


def attach_homes(events):
    """Annotate the source symbol's final home, never equate LR and stack IDs."""
    homes = next((e["order"] for e in reversed(events) if e["stage"] == "stack-assigned"), [])
    late = next((e for e in reversed(events) if e["stage"] == "saved-register-threshold"), None)
    for e in events:
        n = e.get("node")
        if not n:
            continue
        if n["kind"] != 13:
            matches = [h for h in homes if h["id"] == n.get("source_id") and n.get("operand") in (h["ptr"], h.get("native_operand"))]
            if len(matches) == 1:
                h = matches[0]
                n["source_stack_home"] = dict(identity=h["slot"], group=h.get("packed_slot"), frame_offset=h["offset"], size=h["size"])
        if e["stage"] == "select" and late is not None and "chosen" in e:
            e["late_binding"] = "removed" if e["chosen"] in late["actual_demotions"] else "survived"


def tuple_operands(p, t, offset):
    """Operand lists, retaining raw list offsets instead of guessing semantics."""
    result = []
    for u in chain(p, p.u32(t + offset), limit=256):
        if p.rd(u + 8, 1)[0] != 1:
            continue
        operand = p.u32(u + 0x18)
        result.append(dict(use=hex(u), operand=hex(operand),
                           kind=p.rd(operand + 4, 1)[0],
                           identity=p.u32(operand + 0x1c),
                           cls=p.u32(0x494350 + 4*(p.u32(u + 8) >> 28)),
                           flags=hex(p.u32(u + 0x10))))
    return result


def cut_prediction(p, t, live, exclude, force):
    initial = set(bitset(p, live))
    operands = tuple_operands(p, t, 0x18) + tuple_operands(p, t, 0x1c)
    removed = {u["identity"] for u in operands if u["kind"] == 2} if live else set()
    excluded, forced = set(bitset(p, exclude)), set(bitset(p, force))
    return dict(initial=sorted(initial), tuple_lrs=sorted(removed), excluded=sorted(excluded),
                forced=sorted(forced), expected=sorted(((initial - removed) - excluded) | forced))


def eviction_prediction(p, t, cls):
    # 48a4a2 scans link +0c toward a kind-18 boundary. The list offsets and
    # traversal direction are explicit: this is not a guessed source distance.
    seen = {u["identity"] for u in tuple_operands(p, t, 0x18) if u["kind"] == 3}
    initial = sorted(seen)
    candidate = None
    path = []
    for other in chain(p, p.u32(t + 0xc), 0xc):
        if p.rd(other + 8, 1)[0] == 0x18:
            break
        if not (p.rd(other + 9, 1)[0] & 1):
            continue
        eligible = [u for u in tuple_operands(p, other, 0x1c)
                    if u["kind"] == 3 and u["cls"] == cls and u["identity"] not in seen]
        if eligible:
            candidate = eligible[-1]
            path.append(dict(tuple=hex(other), candidates=eligible))
        seen.update(u["identity"] for u in tuple_operands(p, other, 0x18) if u["kind"] == 3)
    return dict(initially_excluded=initial, candidate=candidate, path=path)


def trace(cap, index, include_uses, include_costs=False, include_cuts=False):
    events = []
    state = dict(f=-1, active=False, root=0, returns={}, selection=None, stack=False, late=False)
    errors = []
    started = time.monotonic()
    global_bps = [0x416e6a, 0x40ee1d, 0x41b6fa, 0x41a985, 0x41b785, 0x41b90e,
                  0x4375fe, 0x439385, 0x437c69, 0x40ec8b, 0x416d6f, 0x41bdd7,
                  0x451ac3, 0x41ba2b, 0x44ca36, 0x45aaf9]
    stack_bps = [0x4367f0, 0x43f93b]
    late_bps = [0x42bf00, 0x443d3f]
    if include_costs:
        global_bps.append(0x40fada)
    if include_cuts:
        global_bps += [0x437d7c, 0x48a4a2, 0x41b2b9, 0x469181]

    def disarm(p, addrs):
        for a in addrs:
            if a in p.orig:
                p.wr(a, p.orig.pop(a))

    def record(stage_name, **kwargs):
        row = dict(seq=len(events), stage=stage_name, **kwargs)
        events.append(row)
        if len(events) % 250 == 0:
            print(f"trace function={index} events={len(events)} stage={stage_name} elapsed={time.monotonic()-started:.1f}s", file=sys.stderr, flush=True)
        return row

    def arm_return(p, c, payload):
        ret = p.u32(c.Esp)
        state["returns"].setdefault(ret, []).append(payload)
        p.arm(ret)

    def hit(p, c, addr):
        # Never let a callback exception strand a stopped c2 process.
        try:
            on_hit(p, c, addr)
        except Exception as exc:
            if len(errors) < 20:
                errors.append(f"{addr:#x}: {type(exc).__name__}: {exc}")

    def on_hit(p, c, addr):
        p.alloc_cache = {}  # immutable snapshot shared by all descriptions at this stop
        if addr in state["returns"] and state["returns"][addr]:
            payload = state["returns"][addr].pop()
            if payload["type"] == "sort":
                actual = queue(p, c.Eax)
                predicted = payload["before"][:]
                predicted.insert(payload["position"], payload["node"])
                payload["event"]["actual_queue"] = actual
                payload["event"]["prediction_ok"] = [r["ptr"] for r in actual] == [r["ptr"] for r in predicted]
            elif payload["type"] == "stack":
                order = stack_order(p)
                for n in order:
                    n["packed_slot"] = next((g["index"] for g in state.get("packing", []) if n["slot"] in g["identities"]), None)
                record("stack-assigned", order=order, frame_bias=p.u32(0x4911ac), frame_cursor=signed(p.u32(0x49107c)))
                state["stack"] = False
                disarm(p, stack_bps)
            elif payload["type"] == "packing":
                state["packing"] = slot_groups(p)
                e = record("stack-packed", groups=state["packing"], prediction=payload["prediction"])
                if "groups" in payload["prediction"]:
                    key = lambda gs: [(g["identities"], g["size"]) for g in gs]
                    e["packing_prediction_ok"] = key(state["packing"]) == key(payload["prediction"]["groups"])
                    actual = {n["slot"]: n["offset"] for n in stack_order(p)}
                    expected = payload["prediction"]["offsets"]
                    e["offset_prediction_ok"] = all(actual.get(k) == v for k,v in expected.items())
                    e["actual_offsets"] = actual
            elif payload["type"] == "cost":
                record("cost-end", ranges=[describe(p, state["root"], a) for a in ranges(p).values()])
            elif payload["type"] == "queue":
                record("sort-order", queue=queue(p, p.u32(0x4910d4)))
            elif payload["type"] == "clone-eligible":
                payload["event"]["result"] = c.Eax
            elif payload["type"] == "late-end":
                record("late-end", **physical_scores(p))
                if state.get("late_event"):
                    e = state["late_event"]
                    e["demotion_prediction_ok"] = e["predicted_demotions"] == e["actual_demotions"]
                state["late"] = False
                disarm(p, late_bps)
            elif payload["type"] == "operand-benefit":
                payload["event"]["result"] = c.Eax
            elif payload["type"] == "reprioritize":
                e = payload["event"]
                a = payload["ptr"]
                e.update(result=c.Eax, survived=bool(p.u32(a + 0x20)), after_priority=signed(p.u32(a + 0xc)))
                if c.Eax and e["survived"] and e["inputs"]:
                    expected = signed((min(n["priority"] for n in e["inputs"]) - 1) & 0xffffffff)
                    e.update(predicted_priority=expected, priority_prediction_ok=e["after_priority"] == expected)
            elif payload["type"] == "cut":
                e = payload["event"]
                actual = bitset(p, p.u32(c.Eax + 0x14)) if c.Eax else []
                e.update(result=hex(c.Eax), actual=actual,
                         cut_set_prediction_ok=actual == e["prediction"]["expected"] and
                         bool(c.Eax) == bool(e["prediction"]["expected"]))
            elif payload["type"] == "eviction":
                e = payload["event"]
                candidate = e["prediction"]["candidate"]
                e.update(result=hex(c.Eax),
                         eviction_prediction_ok=bool(candidate) and candidate["operand"] == hex(c.Eax))
            return
        if addr == dbg.FUNC_MARK:
            state["f"] += 1
            return
        if addr == 0x414a64:
            state["active"] = state["f"] + 1 == index
            state["root"] = p.u32(0x48fb48)
            if state["active"]:
                for a in global_bps:
                    p.arm(a)
                print(f"trace function={index} target entered", file=sys.stderr, flush=True)
            return
        if addr == 0x42b7aa:
            state["stack"] = state["f"] == index
            if state["stack"]:
                for a in stack_bps:
                    p.arm(a)
                arm_return(p, c, dict(type="stack"))
            return
        if addr == 0x42a8aa:
            if state["active"]:
                disarm(p, global_bps)
                state["late"] = True
                for a in late_bps:
                    p.arm(a)
                arm_return(p, c, dict(type="late-end"))
            state["active"] = False
            return
        if state["stack"] and addr == 0x4367f0:
            n = words(p, c.Ecx, 15)
            record("stack-reference", ptr=hex(c.Ecx), id=n[7], size=n[8], add=signed(c.Edx),
                   before=signed(n[13]), line=p.u32(c.Ebx + 0x10) & 0xffff,
                   caller=hex(p.u32(c.Esp)), tuple=hex(c.Ebx))
            return
        if state["stack"] and addr == 0x43f93b:
            order = stack_order(p)
            record("stack-ordered", order=order)
            arm_return(p, c, dict(type="packing", prediction=predict_packing(p, c.Ecx, order, c.Edx)))
            return
        if state["late"] and addr == 0x42bf00:
            data = physical_scores(p)
            state["late_event"] = record("saved-register-threshold", **data,
                                         predicted_demotions=[r for r in data["order"] if data["scores"][r] < 3],
                                         actual_demotions=[])
            return
        if state["late"] and addr == 0x443d3f:
            reg = p.u32(c.Edx + 0x1c)
            record("demote-saved-register", reg=reg, score=signed(p.u32(c.Edx + 0x34)))
            if state.get("late_event"):
                state["late_event"]["actual_demotions"].append(reg)
            return
        if not state["active"]:
            return
        root = state["root"]
        if addr == 0x416e6a:
            record("allocator", root=hex(c.Ecx))
        elif addr == 0x40ee1d:
            record("cost-start", ranges=[describe(p, root, a) for a in ranges(p).values()])
            arm_return(p, c, dict(type="cost"))
        elif addr == 0x41bdd7:
            arm_return(p, c, dict(type="queue"))
        elif addr == 0x41b6fa:
            row = describe(p, root, c.Ecx, include_uses)
            before = queue(p, c.Edx)
            pos, reason = insert_prediction(before, row)
            event = record("sort", node=row, before=before, position=pos, tie_break=reason,
                           caller=hex(p.u32(c.Esp)))
            arm_return(p, c, dict(type="sort", node=row, before=before, position=pos, event=event))
        elif addr == 0x416d6f:
            record("definition-rank", ptr=hex(c.Ebp), lr=p.u32(c.Ebp + 0x1c), rank=c.Esi)
        elif addr == 0x41a985:
            row = describe(p, root, c.Edx, include_uses)
            state["colour_start"] = record("colour-start", node=row, queue=queue(p, p.u32(0x4910d4)))["seq"]
        elif addr == 0x41b785:
            node = describe(p, root, c.Ecx, include_uses)
            near = bitset(p, p.u32(c.Esp + 4))
            rs = ranges(p)
            neighbours = [describe(p, root, rs[x]) for x in near if x in rs]
            penalties = [0] * 9
            for n in neighbours:
                if n["benefit"] > 0:
                    for pref in n["preferences"]:
                        r = pref["reg"]
                        if r in n["available"] and r < 9:
                            penalties[r] += pref["weight"]
                    if len(n["available"]) == 1 and n["available"][0] < 9:
                        penalties[n["available"][0]] += 100 * n["benefit"]
            for pref in node["preferences"]:
                if pref["reg"] < 9:
                    penalties[pref["reg"]] -= pref["weight"]
            penalties = [signed(x & 0xffffffff) for x in penalties]
            order = []
            for k in range(32):
                r = p.u32(p.u32(0x4931d0 + c.Edx * 4) + 4 * k)
                if not r:
                    break
                order.append(r)
            eligible = [r for r in order if r in node["available"]]
            expected = min(eligible, key=lambda r: signed(penalties[r] & 0xffffffff)) if eligible else 0
            state["selection"] = record("select", node=node, interference=near, neighbours=neighbours,
                                         preference_order=order, predicted_penalties=penalties,
                                         predicted_reg=expected)
        elif addr == 0x41ba2b:
            state["update_range"] = describe(p, root, p.u32(c.Esp + 4))
        elif addr == 0x451ac3:
            record("clone-definition", node=describe(p, root, c.Ecx, include_uses),
                   caused_by=state.get("update_range"), caller=hex(p.u32(c.Esp)))
        elif addr == 0x44ca36:
            event = record("clone-eligible", tuple=hex(c.Ecx), op=p.u32(c.Ecx + 4), mode=c.Edx,
                           flags=hex(p.u32(c.Ecx + 0x10)), caller=hex(p.u32(c.Esp)))
            arm_return(p, c, dict(type="clone-eligible", event=event))
        elif addr == 0x45aaf9:
            node = describe(p, root, c.Ecx)
            inputs = []
            definition = p.u32(c.Ecx + 0x38)
            if definition:
                for u in chain(p, p.u32(definition + 0x18)):
                    if p.rd(u + 8, 1)[0] == 1 and p.u32(u + 0x10) & 0x20:
                        child = p.u32(u + 0x18)
                        if p.rd(child + 4, 1)[0] == 2:
                            inputs.append(describe(p, root, child))
            event = record("reprioritize", node=node, inputs=inputs)
            arm_return(p, c, dict(type="reprioritize", ptr=c.Ecx, event=event))
        elif addr == 0x40fada:
            operand = c.Edx
            kind = p.rd(operand + 4, 1)[0]
            value = None
            if kind == 13:
                original = p.u32(operand + 0x28)
                if p.rd(original + 8, 1)[0] == 7:
                    value = signed(p.u32(original + 0x18))
            event = record("operand-benefit", tuple=hex(c.Ecx), op=p.u32(c.Ecx + 4),
                           line=p.u32(c.Ecx + 0x10) & 0xffff, kind=kind, value=value,
                           operands=[words(p, p.u32(c.Ecx + off), 12) for off in (0x18, 0x1c)])
            arm_return(p, c, dict(type="operand-benefit", event=event))
        elif addr == 0x41b90e:
            row = state["selection"]
            if row is not None:
                actual = [signed(x) for x in words(p, 0x4931d8, 9)]
                row.update(chosen=c.Edi, actual_penalties=actual,
                           prediction_ok=c.Edi == row["predicted_reg"],
                           penalty_prediction_ok=actual == row["predicted_penalties"])
        elif addr in (0x4375fe, 0x439385, 0x437c69):
            node = c.Edx if addr in (0x4375fe, 0x437c69) else c.Ecx
            record({0x4375fe: "spill", 0x439385: "split", 0x437c69: "defer"}[addr],
                   node=describe(p, root, node, include_uses), queue=queue(p, p.u32(0x4910d4)))
        elif addr == 0x40ec8b:
            original = p.u32(c.Esp + 4)
            if p.rd(original + 4, 1)[0] == 13:
                n = p.u32(c.Edx + 0x18)
                record("rematerialize", node=describe(p, root, n), caller=hex(p.u32(c.Esp)),
                       tuple=hex(c.Ecx), line=p.u32(c.Ecx + 0x10) & 0xffff,
                       flags=hex(p.u32(c.Edx + 0x10)))
        elif addr in (0x41b2b9, 0x469181):
            record("pressure-threshold", tuple=hex(c.Ebx), pressure=signed(c.Edi),
                   capacity=signed(c.Eax), cls=p.u32(c.Esp + 0x7c),
                   op=p.u32(c.Ebx+4), line=p.u32(c.Ebx+0x10) & 0xffff,
                   colour_start=state.get("colour_start"),
                   already_cut=bool(p.rd(c.Ebx + 0x12, 1)[0] & 1),
                   temporary_evictions=max(0, signed(c.Edi)-signed(c.Eax)),
                   path="over-capacity" if addr == 0x469181 else "at-capacity")
        elif addr == 0x437d7c:
            e = record("cut-set", tuple=hex(c.Ecx), caller=hex(p.u32(c.Esp)),
                       op=p.u32(c.Ecx+4), line=p.u32(c.Ecx+0x10) & 0xffff,
                       colour_start=state.get("colour_start"),
                       prediction=cut_prediction(p, c.Ecx, c.Edx, p.u32(c.Esp+4), p.u32(c.Esp+8)))
            arm_return(p, c, dict(type="cut", event=e))
        elif addr == 0x48a4a2:
            t = p.u32(c.Esp+4)
            e = record("evict-temporary", tuple=hex(t), cls=c.Edx,
                       prediction=eviction_prediction(p, t, c.Edx))
            arm_return(p, c, dict(type="eviction", event=e))

    bps = [dbg.FUNC_MARK, 0x414a64, 0x42a8aa, 0x42b7aa]
    obj = owned_run(cap, bps, hit)
    check_stack_history(events)
    attach_homes(events)
    return dict(compiler_sha256=PIN, function=index, events=events, errors=errors,
                object=str(obj), object_sha256=hashlib.sha256(obj.read_bytes()).hexdigest())


def main():
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("capture")
    ap.add_argument("--function", type=int, default=0)
    ap.add_argument("--out", type=Path, required=True)
    ap.add_argument("--uses", action="store_true")
    ap.add_argument("--costs", action="store_true", help="also trace operand benefit helper calls (slower)")
    ap.add_argument("--cuts", action="store_true", help="also predict cut sets and local temporary eviction")
    args = ap.parse_args()
    cap = Path(args.capture)
    if not (cap / "flags.txt").is_file():
        cap = ROOT / "build/c2trace/cap" / args.capture
    if not (cap / "flags.txt").is_file():
        ap.error("capture does not contain flags.txt")
    args.out.parent.mkdir(parents=True, exist_ok=True)
    replay = args.out.parent / (args.out.stem + "-replay")
    replay.mkdir(exist_ok=True)
    for p in cap.iterdir():
        if p.is_file() and p.name not in ("trace.obj", "index.json"):
            shutil.copyfile(p, replay / p.name)
    result = trace(replay.resolve(), args.function, args.uses, args.costs, args.cuts)
    args.out.write_text(json.dumps(result, indent=1))
    for e in result["events"]:
        if "node" in e:
            n = e["node"]
            info = f"LR={n['lr']} sid={n.get('source_id')} k={n['kind']} v={n['value']} refs={n['refs']} priority={n['priority']} benefit={n['benefit']} tie={n['tie']} lenBB={n['span_blocks']} BB={n['first']}..{n['last']}"
            if e["stage"] == "sort":
                info += f" insert={e['position']} by={e['tie_break']} check={e.get('prediction_ok')}"
            elif e["stage"] == "select":
                info += f" interference={e['interference']} available={n['available']} -> {REG.get(e.get('chosen'), '?')} predict={REG.get(e['predicted_reg'], '?')} check={e.get('prediction_ok')} penalties={e.get('actual_penalties')}"
                info += f" late={e.get('late_binding', 'unobserved')}"
            if "source_stack_home" in n:
                info += f" source_home={n['source_stack_home']}"
            print(f"{e['seq']:5} {e['stage']:16} {info}")
        elif e["stage"] in ("stack-ordered", "stack-assigned"):
            print(e["stage"])
            for i, n in enumerate(e["order"]):
                print(f" {i:3} id={n['id']} size={n['size']} weight={n['weight']} identity={n['slot']} packed={n.get('packed_slot')} frame_offset={n['offset']}")
        elif e["stage"] == "sort-order":
            print("sort-order", " ".join(f"{n['lr']}({n['priority']},{n['tie']})" for n in e["queue"]))
        elif e["stage"] == "saved-register-threshold":
            print("saved-register-threshold", e["scores"], "demote", e["actual_demotions"], "check", e.get("demotion_prediction_ok"))
        elif e["stage"] in ("cut-set", "evict-temporary", "pressure-threshold"):
            print(e["seq"], e["stage"], json.dumps({k:v for k,v in e.items() if k not in ("seq", "stage")}))
    checks = [e for e in result["events"] if "prediction_ok" in e]
    print("checks", sum(e["prediction_ok"] for e in checks), "/", len(checks), "errors", result["errors"])
    model_checks = [(e["seq"], key, value) for e in result["events"] for key, value in e.items()
                    if key == "prediction_ok" or key.endswith("_prediction_ok")]
    failed = [(seq, key) for seq, key, value in model_checks if not value]
    print("all-model-checks", len(model_checks)-len(failed), "/", len(model_checks), "failed", failed)
    if result["errors"] or failed:
        sys.exit(1)


if __name__ == "__main__":
    main()
