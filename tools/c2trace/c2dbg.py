"""Bounded WOW64 replay of pinned VC5 SP3 or RTM C2 on captured IL.

SP3 rotor/scratch APIs remain calibrated only for SP3; RTM supports expression
windows. Breakpoints are the only mutations in observer runs. Children are reaped.
"""
import ctypes, ctypes.wintypes as W, shutil, time, hashlib, json
from pathlib import Path

K = ctypes.WinDLL("kernel32", use_last_error=True)
try:
    import profiles
except ModuleNotFoundError:   # imported as the c2trace package (tools/histlink.py)
    from . import profiles
C2 = str(profiles.get().c2)
C2_SHA256 = profiles.get().sha256
ROOT = Path(__file__).resolve().parents[2]
FUNC_MARK, ASSIGN, SET = 0x42a918, 0x435c37, 0x435f38
ROTOR, ROTOR_LIST, ROTOR_FLAG = 0x491120, 0x491100, 0x48fb60

def apply_profile(profile):
    global C2, C2_SHA256
    C2, C2_SHA256 = str(profile.c2), profile.sha256

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

K.TerminateProcess.argtypes = [W.HANDLE, W.UINT]
K.WaitForSingleObject.argtypes = [W.HANDLE, W.DWORD]
K.DebugActiveProcessStop.argtypes = [W.DWORD]
K.GetExitCodeProcess.argtypes = [W.HANDLE, ctypes.POINTER(W.DWORD)]

DBG_CONTINUE, DBG_NOT_HANDLED = 0x00010002, 0x80010001
WATCH_RW = False  # True: hardware watchpoints trigger on reads as well as writes
BP_CODES = {0x80000003, 0x4000001F}   # native / WOW64 breakpoint
SS_CODES = {0x80000004, 0x4000001E}   # native / WOW64 single step


class Process:
    """Memory access to the debuggee."""

    def __init__(self, handle):
        self.h = handle
        self.orig = {}       # armed breakpoint -> original byte
        self.oneshot = set()  # breakpoints removed after their first hit

    def arm(self, addr, oneshot=False):
        """Add a breakpoint from inside a hit handler (no effect if one is already armed there)."""
        if addr in self.orig:
            return False
        self.orig[addr] = self.rd(addr, 1)
        self.wr(addr, b"\xcc")
        if oneshot:
            self.oneshot.add(addr)
        return True

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
    if hashlib.sha256(Path(C2).read_bytes()).hexdigest() != C2_SHA256:
        raise RuntimeError('Wrong C2: selected hook addresses are SHA-256 pinned')
    capdir = capdir.resolve()
    if not capdir.is_relative_to(profiles.get().work):
        raise ValueError('Debugger work must stay in the selected compiler build directory')
    profiles.check_capture(capdir)
    flags = (capdir / "flags.txt").read_text().strip()
    il_prefix = flags.split("-il ")[1].split()[0]
    work = capdir / "run"
    work.mkdir(exist_ok=True)
    stem = Path(il_prefix).name
    for f in capdir.glob(stem + "*"):
        shutil.copy(f, work / f.name)
    obj = capdir / "trace.obj"
    obj.unlink(missing_ok=True)
    (capdir / 'replay-receipt.json').unlink(missing_ok=True)
    parts = flags.split()
    parts[parts.index("-il") + 1] = str(work / stem)
    parts = [p if not p.startswith("-Fo") else "-Fo" + str(obj) for p in parts]
    tmp = ROOT / "build/tmp"
    tmp.mkdir(parents=True, exist_ok=True)
    env = {"MSC_CMD_FLAGS": " ".join(parts), "SystemRoot": r"C:\Windows", "TMP": str(tmp), "TEMP": str(tmp),
           "PATH": str(Path(C2).parent) + ";" + str(Path(C2).parents[2] / "SHAREDIDE/BIN") + r";C:\Windows\System32"}
    envblock = "".join(f"{k}={v}\0" for k, v in env.items()) + "\0"
    si, pi = SI(), PI()
    si.cb = ctypes.sizeof(SI)
    if not K.CreateProcessW(C2, ctypes.create_unicode_buffer(f'"{C2}"'), None, None, False,
                            0x2 | 0x400, ctypes.create_unicode_buffer(envblock), ctypes.create_unicode_buffer(str(work)), ctypes.byref(si), ctypes.byref(pi)):
        raise OSError(ctypes.get_last_error())
    proc = Process(pi.hProcess)
    orig, rearm, armed = proc.orig, {}, False
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

    deadline = time.monotonic() + 180
    exited = False
    try:
        while True:
            if time.monotonic() > deadline:
                raise TimeoutError('c2 replay exceeded 180 seconds')
            if not K.WaitForDebugEvent(ctypes.byref(ev), 1000):
                if ctypes.get_last_error() == 121:
                    continue
                raise OSError(ctypes.get_last_error())
            status = DBG_CONTINUE
            if ev.Code == 5:  # EXIT_PROCESS_DEBUG_EVENT
                K.ContinueDebugEvent(ev.Pid, ev.Tid, DBG_CONTINUE)
                exited = True
                break
            if ev.Code in (2, 3, 6):  # CREATE_THREAD hThread; CREATE_PROCESS/LOAD_DLL hFile
                handle = ctypes.c_void_p.from_buffer(ev, 16).value
                if handle:
                    K.CloseHandle(handle)
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
                            dr7 |= (1 << (2 * i)) | ((0b1111 if WATCH_RW else 0b1101) << (16 + 4 * i))
                        ctx.Dr[5] = dr7
                        commit(ht, ctx)
                elif ec in BP_CODES and addr in orig:
                    ht, ctx = context(ev.Tid, 0x10007)
                    ctx.Eip = addr
                    proc.wr(addr, orig[addr])
                    try:
                        on_hit(proc, ctx, addr)
                    except BaseException:
                        K.CloseHandle(ht)
                        raise
                    if addr in proc.oneshot:
                        proc.oneshot.discard(addr)
                        del orig[addr]
                    elif addr in orig:
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
        if not exited:
            K.TerminateProcess(pi.hProcess, 1)
            K.DebugActiveProcessStop(pi.pid)
        if K.WaitForSingleObject(pi.hProcess, 10000) != 0:
            raise RuntimeError('c2 child did not terminate')
        code = W.DWORD()
        K.GetExitCodeProcess(pi.hProcess, ctypes.byref(code))
        K.CloseHandle(pi.hThread)
        K.CloseHandle(pi.hProcess)
    if not obj.is_file() and not code.value:
        raise RuntimeError('c2 replay produced no object')
    if code.value:
        raise RuntimeError('c2 replay exited with code %d' % code.value)
    (capdir / 'replay-receipt.json').write_text(json.dumps(dict(pid=pi.pid,c2_sha256=C2_SHA256,
        exit_code=code.value,waited_for_exit=True,cwd=str(work),object=str(obj)),indent=2)+'\n')
    return obj


def assigns(capdir: Path, perturb=None):
    """Trace every temp assignment. Returns (functions, obj): functions is a list (compile order) of lists of dicts
    {path: 'pref'|'rotor'|'first', op, lr, reg, rotor, pref}. PERTURB=(function, assign, slots) shifts the rotor
    pointer by SLOTS just before that assignment (diagnostic only)."""
    profiles.require_sp3('rotor assignment')
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


SCRATCH = 0x42b2c4
SCRATCH_LAST, SCRATCH_N = 0x4911a8, 0x4911a4
SCRATCH_EXITS = (0x42b3a6, 0x42b3b9)
SCRATCH_FALLBACK = 0x46c46c


def scratches(capdir: Path, perturb=None):
    """Trace every scratch pick. Returns (functions, obj); per function a list of dicts {last, n, byte, reg}.
    PERTURB=(function, pick, slots) shifts the last-pick index by SLOTS before that pick (diagnostic only)."""
    profiles.require_sp3('scratch picker')
    funcs = []

    def hit(p, ctx, addr):
        if addr == FUNC_MARK:
            funcs.append([])
        elif addr == SCRATCH and funcs:
            f, k = len(funcs) - 1, len(funcs[-1])
            n = p.u32(SCRATCH_N)
            if perturb and (f, k) == tuple(perturb[:2]) and n:
                last = p.u32(SCRATCH_LAST)
                p.wr(SCRATCH_LAST, (((last % n) + perturb[2]) % n).to_bytes(4, "little"))
            funcs[-1].append({"last": p.u32(SCRATCH_LAST), "n": n, "byte": ctx.Edx, "reg": None})
        elif addr in SCRATCH_EXITS and funcs and funcs[-1]:
            funcs[-1][-1]["reg"] = REG.get(ctx.Eax, ctx.Eax)

    obj = run(capdir, [FUNC_MARK, SCRATCH, *SCRATCH_EXITS], hit)
    return funcs, obj
