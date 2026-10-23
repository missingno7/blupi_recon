"""c2trace: trace pinned MSVC 5.0 SP3/RTM c2.exe temp-register assignment for one function (diagnostic only, never a match).

  Common options: --compiler {sp3,rtm} --target GAME/EDITION/LANGUAGE
  Rotor diagnostics remain SP3 only.

  c2trace.py build-stub                         build the /B2 pass-2 stand-in (VC6 tree) into build/c2trace/stub
  c2trace.py capture NAME src/X.cpp[=cand.cpp]  compile a recon TU (recon.json settings) and keep its c2 input
  c2trace.py capture-micro NAME file.cpp        same for a standalone micro TU (/W3 /GX /O2 /ML /Gd /Gy)
  c2trace.py assigns NAME SYMBOL                every temp assignment of that function: path, register, rotor
  c2trace.py shift NAME SYMBOL RVA              shift the rotor by 1 and 2 slots at each assignment; differing bytes
                                                against BLUPI.EXE (finds where the original had one rotor step more/less)
  c2trace.py map NAME SYMBOL K...               first instruction that changes when the rotor moves at assignment K

The rotor model and the c2.exe addresses are documented in docs/COMPILER_STUDY.md (round 4). Captures live in
build/c2trace/cap/NAME. SYMBOL is a substring of the decorated name. Run one command at a time (host safety).
"""
import argparse, json, shutil, subprocess, sys
import profiles
from pathlib import Path

HERE = Path(__file__).resolve().parent
ROOT = HERE.parents[1]
sys.path.insert(0, str(HERE))
sys.path.insert(0, str(ROOT / "tools"))
import c2dbg  # noqa: E402

WORK = ROOT / "build/c2trace"
STUB = WORK / "stub" / "c2stub.exe"
PY = sys.executable
VC6 = Path(r"C:\tools\msvc6_rtm")


def configure(compiler=None, target=None):
    global WORK, STUB
    profile = profiles.configure(compiler, target)
    c2dbg.apply_profile(profile)
    WORK, STUB = profile.work, profile.work / 'stub/c2stub.exe'
    return profile


def build_stub():
    (WORK / "stub").mkdir(parents=True, exist_ok=True)
    obj = WORK / "stub" / "c2stub.obj"
    subprocess.run([PY, str(ROOT / "tools/msvc.py"), str(HERE / "c2stub.c"), "--tree", "vc6", "--out", str(obj),
                    "--log-dir", str(WORK / "stub/logs"), "--flag", "/O1", "--define", 'C2_DEFAULT_PATH="' + str(profiles.get().c2).replace('\\', '/').replace('"', '') + '"'], check=True)
    env = {"PATH": f"{VC6 / 'COMMON/MSDEV98/BIN'};{VC6 / 'VC98/BIN'};C:\\Windows\\System32",
           "LIB": str(VC6 / "VC98/LIB"), "SystemRoot": r"C:\Windows",
           "TMP": str(ROOT / "build/tmp"), "TEMP": str(ROOT / "build/tmp")}
    subprocess.run([str(VC6 / "VC98/BIN/LINK.EXE"), "/nologo", "/subsystem:console", f"/out:{STUB}", str(obj),
                    "kernel32.lib"], check=True, env=env, cwd=str(WORK / "stub"))
    profiles.stamp(STUB.parent)
    print(STUB)


def capture_dir(name, kind='cap'):
    cap = (WORK / kind / name).resolve()
    if not cap.is_relative_to((WORK / kind).resolve()) or cap == (WORK / kind).resolve():
        raise ValueError('Capture must be below the compiler work directory')
    return cap


def _keep(name):
    il = STUB.parent / "il"
    cap = capture_dir(name)
    shutil.rmtree(cap, ignore_errors=True)
    shutil.copytree(il, cap)
    profiles.stamp(cap)
    print(cap)


def _fresh_il():
    if not STUB.is_file():
        sys.exit("run `c2trace.py build-stub` first")
    profiles.get().verify()
    profiles.check_capture(STUB.parent, check_target=False)
    il = (STUB.parent / "il").resolve()
    if not il.is_relative_to(WORK.resolve()):
        raise ValueError("IL directory outside compiler work directory")
    shutil.rmtree(il, ignore_errors=True)
    il.mkdir()


def capture(name, unit):
    _fresh_il()
    import fnprobe
    fnprobe.TARGET = profiles.target()
    fnprobe.EXE = fnprobe.TARGET.exe if fnprobe.TARGET else ROOT / 'assets/Speedy Blupi/BLUPI.EXE'
    canonical, _, candidate = unit.partition('=')
    canonical = canonical.replace('\\', '/')
    source = (ROOT / (candidate or canonical)).resolve()
    folder = WORK / 'capture'
    folder.mkdir(parents=True, exist_ok=True)
    fnprobe.compile_unit(canonical, source, folder, ['/B2' + str(STUB)], tree=profiles.get().name)
    if not (STUB.parent / "il" / "flags.txt").is_file():
        sys.exit("C2 stub did not capture flags")
    _keep(name)


def capture_micro(name, src):
    _fresh_il()
    cmd = [PY, str(ROOT / "tools/msvc.py"), src, "--tree", profiles.get().name, "--out", str(WORK / f"{name}.obj"),
           "--log-dir", str(WORK / "logs")]
    for f in ["/W3", "/GX", "/O2", "/ML", "/Gd", "/Gy", "/B2" + str(STUB)]:
        cmd += ["--flag", f]
    for d in profiles.target().defines if profiles.target() else []:
        cmd += ['--define',d]
    subprocess.run(cmd, check=True)
    _keep(name)


def _functions(obj):
    import cmpfunc
    return {f.name: f for f in cmpfunc.Coff(obj).functions()}


def find_index(name, symbol):
    """Compile-order index of SYMBOL: shift the rotor at the first assignment of each candidate function and see which
    object function changes. Cached in the capture folder."""
    cap = WORK / "cap" / name
    cache = cap / "index.json"
    known = json.loads(cache.read_text()) if cache.is_file() else {}
    if symbol in known:
        return known[symbol]
    funcs, obj = c2dbg.assigns(cap)
    base = _functions(obj)
    target = next(n for n in base if symbol in n)
    for i in sorted(range(len(funcs)), key=lambda i: -len(funcs[i])):
        if not funcs[i]:
            break
        _, obj2 = c2dbg.assigns(cap, perturb=(i, 0, 1))
        cur = _functions(obj2)
        changed = [n for n in base if cur[n].data != base[n].data]
        if target in changed:
            known[symbol] = i
            cache.write_text(json.dumps(known, indent=1))
            return i
    sys.exit(f"{symbol}: no compiled function with temp assignments changes it")


def cmd_assigns(name, symbol):
    i = find_index(name, symbol)
    funcs, _ = c2dbg.assigns(WORK / "cap" / name)
    for k, a in enumerate(funcs[i]):
        print(f"{k:4d} {a['path']:<5} {a['reg'] or '?':<4} op={a['op']:#x} lr={a['lr']} rotor={a['rotor']} pref={a['pref']}")


def _diff(obj, symbol, rva, pe):
    f = next(f for n, f in _functions(obj).items() if symbol in n)
    t = pe.get_data(rva, len(f.data))
    masked = set()
    for r in f.relocs:
        masked.update(range(r.offset - f.start, r.offset - f.start + 4))
    return [k for k in range(len(f.data)) if f.data[k] != t[k] and k not in masked]


def cmd_shift(name, symbol, rva):
    import pefile
    pe = pefile.PE(str(ROOT / "assets/Speedy Blupi/BLUPI.EXE"), fast_load=True)
    cap = WORK / "cap" / name
    i = find_index(name, symbol)
    funcs, obj = c2dbg.assigns(cap)
    print("base", len(_diff(obj, symbol, rva, pe)), "differing bytes;", len(funcs[i]), "assignments")
    for k in range(len(funcs[i])):
        out = []
        for s in (1, 2):
            _, obj = c2dbg.assigns(cap, perturb=(i, k, s))
            d = _diff(obj, symbol, rva, pe)
            out.append(f"+{s}: {len(d)}" + (f" @{d[0]:#x}" if d else "  <== MATCH"))
        print(f"{k:4d}  " + "   ".join(out), flush=True)


def cmd_map(name, symbol, ks):
    import capstone
    md = capstone.Cs(capstone.CS_ARCH_X86, capstone.CS_MODE_32)
    cap = WORK / "cap" / name
    i = find_index(name, symbol)

    def code(perturb):
        _, obj = c2dbg.assigns(cap, perturb=perturb)
        f = next(f for n, f in _functions(obj).items() if symbol in n)
        return list(md.disasm(f.data, 0))

    base = code(None)
    for k in ks:
        for a, b in zip(base, code((i, k, 1))):
            if (a.mnemonic, a.op_str) != (b.mnemonic, b.op_str):
                print(f"assign {k}: +{a.address:#x}  {a.mnemonic} {a.op_str}  ->  {b.mnemonic} {b.op_str}")
                break
        else:
            print(f"assign {k}: no change")


def main(argv):
    if not argv or argv[0] in ("-h", "--help"):
        print(__doc__)
        return
    parser = argparse.ArgumentParser(add_help=False)
    profiles.add_arguments(parser)
    options, argv = parser.parse_known_args(argv)
    configure(options.compiler, options.target)
    cmd, args = argv[0], argv[1:]
    if cmd in ('assigns', 'shift', 'map'):
        profiles.require_sp3(cmd)
    if cmd == "build-stub":
        build_stub()
    elif cmd == "capture":
        capture(args[0], args[1])
    elif cmd == "capture-micro":
        capture_micro(args[0], args[1])
    elif cmd == "assigns":
        cmd_assigns(args[0], args[1])
    elif cmd == "shift":
        cmd_shift(args[0], args[1], int(args[2], 16))
    elif cmd == "map":
        cmd_map(args[0], args[1], [int(k) for k in args[2:]])
    else:
        sys.exit(__doc__)


if __name__ == "__main__":
    main(sys.argv[1:])
