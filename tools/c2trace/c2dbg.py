"""Minimal WOW64 debugger that runs MSVC 5.0 SP3 c2.exe (pass 2) on captured IL files and traces its temp-register
assignment. Toolchain analysis only: nothing on disk is modified and the traced run writes a normal object file.

Known c2.exe SP3 addresses (docs/COMPILER_STUDY.md, round 4):
  0x42a918  just after the per-function rotor reset (one hit per compiled function)
  0x435c37  assign(temp=ecx, tuple=edx): gives a temp its register (preference, rotor, first free)
  0x435f38  set(reg=ecx, lr=edx): records the chosen register
  0x491120  rotor pointer into the register list at 0x491100 (eax, ecx, edx)
"""
import ctypes, ctypes.wintypes as W, shutil
from pathlib import Path

K = ctypes.WinDLL("kernel32", use_last_error=True)
C2 = r"C:\tools\msvc5_sp3\VC\BIN\c2.exe"
ROOT = Path(__file__).resolve().parents[2]
FUNC_MARK, ASSIGN, SET = 0x42a918, 0x435c37, 0x435f38
ROTOR, ROTOR_LIST, ROTOR_FLAG = 0x491120, 0x491100, 0x48fb60
REG = {1: "eax", 2: "ecx", 3: "edx", 4: "ebx", 5: "esp", 6: "ebp", 7: "esi", 8: "edi"}


class FSA(ctypes.Structure):
    _fields_ = [("a", W.DWORD * 7), ("RegisterArea", ctypes.c_byte * 80), ("Cr0NpxState", W.DWORD)]


class CTX(ctypes.Structure):
    _fields_ = [("ContextFlags", W.DWORD), ("Dr", W.DWORD * 6), ("FloatSave", FSA),
                ("SegGs", W.DWORD), ("SegFs", W.DWORD), ("SegEs", W.DWORD), ("SegDs", W.DWORD),
                ("Edi", W.DWORD), ("Esi", W.DWORD), ("Ebx", W.DWORD), ("Edx", W.DWORD), ("Ecx", W.DWORD), ("Eax", W.DWORD),
                ("Ebp", W.DWORD), ("Eip", W.DWORD), ("SegCs", W.DWORD), ("EFlags", W.DWORD), ("Esp", W.DWORD),
                ("SegSs", W.DWORD), ("Ext", ctypes.c_byte * 512)]


class EXC_REC(ctypes.Structure):
    _fields_ = [("Code", W.DWORD), ("Flags", W.DWORD), ("Rec", ctypes.c_void_p), ("Addr", ctypes.c_void_p),
                ("N", W.DWORD), ("Info", ctypes.c_size_t * 15)]


class DEBUG_EVENT(ctypes.Structure):
    _fields_ = [("Code", W.DWORD), ("Pid", W.DWORD), ("Tid", W.DWORD), ("pad", W.DWORD),
                ("Exc", EXC_REC), ("First", W.DWORD), ("u", ctypes.c_byte * 64)]


class SI(ctypes.Structure):
    _fields_ = [("cb", W.DWORD), ("r", ctypes.c_void_p), ("d", ctypes.c_void_p), ("t", ctypes.c_void_p),
                ("x", W.DWORD * 8), ("f", W.WORD), ("s", W.WORD), ("r2", ctypes.c_void_p),
                ("i", W.HANDLE), ("o", W.HANDLE), ("e", W.HANDLE)]


class PI(ctypes.Structure):
    _fields_ = [("hProcess", W.HANDLE), ("hThread", W.HANDLE), ("pid", W.DWORD), ("tid", W.DWORD)]


for _name, _res, _args in [
        ("OpenThread", W.HANDLE, [W.DWORD, W.BOOL, W.DWORD]),
        ("ReadProcessMemory", W.BOOL, [W.HANDLE, ctypes.c_void_p, ctypes.c_void_p, ctypes.c_size_t, ctypes.c_void_p]),
        ("WriteProcessMemory", W.BOOL, [W.HANDLE, ctypes.c_void_p, ctypes.c_void_p, ctypes.c_size_t, ctypes.c_void_p]),
        ("FlushInstructionCache", W.BOOL, [W.HANDLE, ctypes.c_void_p, ctypes.c_size_t]),
        ("Wow64GetThreadContext", W.BOOL, [W.HANDLE, ctypes.POINTER(CTX)]),
        ("Wow64SetThreadContext", W.BOOL, [W.HANDLE, ctypes.POINTER(CTX)]),
        ("WaitForDebugEvent", W.BOOL, [ctypes.POINTER(DEBUG_EVENT), W.DWORD]),
        ("ContinueDebugEvent", W.BOOL, [W.DWORD, W.DWORD, W.DWORD]),
        ("CloseHandle", W.BOOL, [W.HANDLE])]:
    getattr(K, _name).restype, getattr(K, _name).argtypes = _res, _args

DBG_CONTINUE, DBG_NOT_HANDLED = 0x00010002, 0x80010001
BP_CODES = {0x80000003, 0x4000001F}   # native / WOW64 breakpoint
SS_CODES = {0x80000004, 0x4000001E}   # native / WOW64 single step


class Process:
    """Memory access to the debuggee."""

    def __init__(self, handle):
        self.h = handle

    def rd(self, addr, n):
        buf = ctypes.create_string_buffer(n)
        K.ReadProcessMemory(self.h, addr, buf, n, None)
        return buf.raw

    def u32(self, addr):
        return int.from_bytes(self.rd(addr, 4), "little")

    def wr(self, addr, data):
        K.WriteProcessMemory(self.h, addr, data, len(data), None)
        K.FlushInstructionCache(self.h, addr, len(data))


def run(capdir: Path, breakpoints, on_hit, watch=(), on_watch=None):
    """Run c2.exe on the IL captured in CAPDIR (flags.txt + IL files). ON_HIT(proc, ctx, addr) runs at each software
    breakpoint before the instruction executes (it may change memory); WATCH lists up to four addresses for 4-byte
    hardware write watchpoints, reported through ON_WATCH(proc, ctx, index). Returns the object path."""
    flags = (capdir / "flags.txt").read_text().strip()
    il_prefix = flags.split("-il ")[1].split()[0]
    work = capdir / "run"
    work.mkdir(exist_ok=True)
    stem = Path(il_prefix).name
    for f in capdir.glob(stem + "*"):
        shutil.copy(f, work / f.name)
    obj = capdir / "trace.obj"
    parts = flags.split()
    parts[parts.index("-il") + 1] = str(work / stem)
    parts = [p if not p.startswith("-Fo") else "-Fo" + str(obj) for p in parts]
    tmp = ROOT / "build/tmp"
    tmp.mkdir(parents=True, exist_ok=True)
    env = {"MSC_CMD_FLAGS": " ".join(parts), "SystemRoot": r"C:\Windows", "TMP": str(tmp), "TEMP": str(tmp),
           "PATH": r"C:\tools\msvc5_sp3\VC\BIN;C:\tools\msvc5_sp3\SHAREDIDE\BIN;C:\Windows\System32"}
    envblock = "".join(f"{k}={v}\0" for k, v in env.items()) + "\0"
    si, pi = SI(), PI()
    si.cb = ctypes.sizeof(SI)
    if not K.CreateProcessW(C2, ctypes.create_unicode_buffer(f'"{C2}"'), None, None, False,
                            0x2 | 0x400, ctypes.create_unicode_buffer(envblock), None, ctypes.byref(si), ctypes.byref(pi)):
        raise OSError(ctypes.get_last_error())
    proc = Process(pi.hProcess)
    orig, rearm, armed = {}, {}, False
    ev = DEBUG_EVENT()

    def context(tid, flags_):
        ht = K.OpenThread(0x1FFFFF, False, tid)
        ctx = CTX()
        ctx.ContextFlags = flags_
        K.Wow64GetThreadContext(ht, ctypes.byref(ctx))
        return ht, ctx

    def commit(ht, ctx):
        K.Wow64SetThreadContext(ht, ctypes.byref(ctx))
        K.CloseHandle(ht)

    try:
        while True:
            K.WaitForDebugEvent(ctypes.byref(ev), 0xFFFFFFFF)
            status = DBG_CONTINUE
            if ev.Code == 5:  # EXIT_PROCESS_DEBUG_EVENT
                K.ContinueDebugEvent(ev.Pid, ev.Tid, DBG_CONTINUE)
                break
            if ev.Code == 1:
                ec, addr = ev.Exc.Code, ev.Exc.Addr or 0
                if ec in BP_CODES and not armed:  # loader breakpoint: the image is mapped now
                    for a in breakpoints:
                        orig[a] = proc.rd(a, 1)
                        proc.wr(a, b"\xcc")
                    armed = True
                    if watch:
                        ht, ctx = context(ev.Tid, 0x10010)
                        dr7 = 0
                        for i, a in enumerate(list(watch)[:4]):
                            ctx.Dr[i] = a
                            dr7 |= (1 << (2 * i)) | (0b1101 << (16 + 4 * i))
                        ctx.Dr[5] = dr7
                        commit(ht, ctx)
                elif ec in BP_CODES and addr in orig:
                    ht, ctx = context(ev.Tid, 0x10007)
                    ctx.Eip = addr
                    proc.wr(addr, orig[addr])
                    on_hit(proc, ctx, addr)
                    ctx.EFlags |= 0x100  # single-step, then re-arm
                    rearm[ev.Tid] = addr
                    commit(ht, ctx)
                elif ec in SS_CODES:
                    if ev.Tid in rearm:
                        proc.wr(rearm.pop(ev.Tid), b"\xcc")
                    if watch:
                        ht, ctx = context(ev.Tid, 0x10017)
                        for i in range(4):
                            if ctx.Dr[4] & (1 << i) and on_watch:
                                on_watch(proc, ctx, i)
                        ctx.Dr[4] = 0
                        commit(ht, ctx)
                elif ec not in BP_CODES:
                    status = DBG_NOT_HANDLED
            K.ContinueDebugEvent(ev.Pid, ev.Tid, status)
    finally:
        K.CloseHandle(pi.hThread)
        K.CloseHandle(pi.hProcess)
    return obj


def assigns(capdir: Path, perturb=None):
    """Trace every temp assignment. Returns (functions, obj): functions is a list (compile order) of lists of dicts
    {path: 'pref'|'rotor'|'first', op, lr, reg, rotor, pref}. PERTURB=(function, assign, slots) shifts the rotor
    pointer by SLOTS just before that assignment (diagnostic only)."""
    funcs = []

    def hit(p, ctx, addr):
        if addr == FUNC_MARK:
            funcs.append([])
        elif addr == ASSIGN and funcs:
            f, k = len(funcs) - 1, len(funcs[-1])
            if perturb and (f, k) == tuple(perturb[:2]):
                slot = ((p.u32(ROTOR) - ROTOR_LIST) // 4 + perturb[2]) % 3
                p.wr(ROTOR, (ROTOR_LIST + 4 * slot).to_bytes(4, "little"))
            lr = p.u32(ctx.Ecx + 0x18)
            pref = p.u32(lr + 0x2c)
            funcs[-1].append({"op": p.u32(ctx.Edx + 4), "lr": p.u32(lr + 0x1c),
                              "rotor": REG.get((p.u32(ROTOR) - ROTOR_LIST) // 4 + 1),
                              "pref": REG.get(p.u32(pref + 0x1c)) if pref else None,
                              "flag": p.u32(ROTOR_FLAG), "reg": None})
        elif addr == SET and funcs and funcs[-1]:
            funcs[-1][-1]["reg"] = REG.get(ctx.Ecx, ctx.Ecx)

    obj = run(capdir, [FUNC_MARK, ASSIGN, SET], hit)
    for f in funcs:
        for a in f:
            a["path"] = "pref" if a["pref"] == a["reg"] and a["pref"] else ("rotor" if a["flag"] else "first")
    return funcs, obj
