# Speedy Blupi 1 — matching decompilation

Reconstructing the C++ source of **Speedy Blupi 1** (Epsitec, 1998; `BLUPI.EXE`, English/US build, SHA-256
`be6de5a2…8290ac`) so that the historical compiler reproduces the original machine code exactly. Matching comes
first; a clean SDL3 port is a later, separate goal. Rules for contributors and agents: [AGENTS.md](AGENTS.md).

## Status (verified with `tools/verify.py`)

**246 functions STRICT + 2 FOLDED, 31,200 / 142,064 unique game `.text` bytes (22.0 %).**
STRICT = every non-relocated byte equal at one proven location and every relocation target proven (imports, calls,
data, string payloads). FOLDED = an identical body the original linker merged into another function. Relocation-
unverified, masked or behavioural similarity is never counted.

| TU | strict | TU | strict | TU | strict |
|---|---:|---|---:|---|---:|
| blupi.cpp | 11 | decor.cpp | 12 | movie.cpp | 13 |
| button.cpp | 14 (+1 folded) | event.cpp | 54 | network.cpp | 15 |
| ddutil.cpp | 8 | jauge.cpp | 9 | pixmap.cpp | 29 |
| decblock.cpp | 12 | menu.cpp | 8 | sound.cpp | 26 |
| decblupi.cpp | 7 | misc.cpp | 7 (+1 folded) | text.cpp | 5 |
| decdesign.cpp | 4 | decmove.cpp | 12 | decnet.cpp | 5 |

Per-TU details, near misses and blockers: `docs/*_TU.md`, `docs/CORE_CLUSTER.md`, `docs/UI_CLUSTER.md`,
`docs/ENGINE_PROBE.md`.

## Key facts ([EVIDENCE] unless noted)

- **Toolchain** ([docs/TOOLCHAIN.md](docs/TOOLCHAIN.md)): MSVC 5.0, compiler pinned to VS97 SP3 `cl 11.00.7022`
  (RTM differs on some functions), `/O2 /GX /ML /Gd /Gy`, statically linked VC5 SP3 `LIBC.LIB`, linker 5.10 family
  (exact build open), identical-COMDAT folding observed. Original build path `F:\Profil\` (`__FILE__`).
- **Layout** ([docs/TU_MAP.md](docs/TU_MAP.md), `oracle/tu_map.csv`): `.text` is TU blocks in alphabetical source
  order, functions in source order; game code ends at RVA `0x23af0`, VC5 LIBC follows.
- **Ancestry** ([docs/ANCESTRY_PLANETBLUPI.md](docs/ANCESTRY_PLANETBLUPI.md),
  [docs/ANCESTRY_SB2.md](docs/ANCESTRY_SB2.md)): Planet Blupi v1.8.0 sources compile to exact SB1 functions for
  sound/movie/misc/ddutil/pixmap/UI code; game logic is the Speedy Blupi engine (SB2 reconstruction is a structural
  hint; SB1's `CDecor` lacks SB2's trajectory arrays). Free Eggbert is a fork of sb2-decomp, not independent evidence.
- **Compiler knowledge** ([msvc5kb/README.md](msvc5kb/README.md)): rule catalog compatible with Blood II's VC5 KB;
  18 rules validated on SB1 (incl. TU symbol-count context: a correct declaration that breaks peers means the real
  missing declarations must be found — resolved for `StopSound` with genuine sound helpers).

## Reproduce

Inputs (ignored by git): `assets/` (original game files and ISO), `external/` (read-only references:
planetblupi, sb2-decomp, free-eggbert), historical toolchains under `C:\tools` (`msvc5_sp3`, `dx6sdk`, Ghidra 12.1),
Python with pefile + capstone (`.pyenv\python.exe` locally).

```powershell
.\.pyenv\python.exe tools\verify.py                       # clean build of src/ + strict compare, regression gate
.\.pyenv\python.exe tools\verify.py --tu src/x.cpp=scratch\tu\x.cpp --build-dir me   # test a candidate
.\.pyenv\python.exe tools\verify.py --accept              # promote newly STRICT functions (no regressions allowed)
.\.pyenv\python.exe tools\oracle.py all --fresh           # regenerate oracle/ inventory and function map
.\.pyenv\python.exe tools\decomp.py <rva|name>            # seeded Ghidra pseudocode (evidence only)
.\.pyenv\python.exe msvc5kb\tools\kb.py hints <func> --json <cmpfunc.json>   # compiler-rule hints for a mismatch
```

Workflow details: [docs/WORKFLOW.md](docs/WORKFLOW.md). Source provenance and every compile adaptation:
[src/PROVENANCE.md](src/PROVENANCE.md).

## Where to continue

1. `event.cpp` large handlers (`DrawButtons`, `TreatEventBase`, `ChangePhase` ≈ 21 KB) — biggest single prize.
2. Engine blocks `decblupi.cpp` (~29.5 KB) and `decdesign.cpp` (~13.7 KB): behaviour-first reconstruction from
   pseudocode + SB2 hints worked for `decblock`.
3. `pixmap.cpp` drawing family, `decor.cpp` save/map methods, `button.cpp`/`text.cpp` residuals.
4. Open questions: `CDecor` is allocated as 0x258d4 bytes but no member access beyond +0xdbec is known; exact
   linker build (Rich header has no linker entry); `__FILE__` build path `F:\Profil\` (reported, not masked).
