# Speedy Blupi 1 — matching decompilation

Reconstructing the C++ source of **Speedy Blupi 1** (Epsitec, 1998; `BLUPI.EXE`, English/US build, SHA-256
`be6de5a2…8290ac`) so that the historical compiler reproduces the original machine code exactly. Matching comes
first; a clean SDL3 port is a later, separate goal. Rules for contributors and agents: [AGENTS.md](AGENTS.md).

## Status (verified with `tools/verify.py`)

**405 functions STRICT + 4 FOLDED, 116,160 / 141,488 unique game `.text` bytes (82.1 %)**, built with one shared
header set for every TU (no per-TU preprocessor views; no active withdrawals; withdrawal history in
[docs/WORKFLOW.md](docs/WORKFLOW.md) and [docs/HEADERS.md](docs/HEADERS.md)). `decmove.cpp`, `decdesign.cpp` and
`decblupi.cpp` are fully strict; 4 mapped game functions (about 24.8 KB) remain open (docs/LINK_CONTEXT.md, docs/COMPILER_STUDY.md).
STRICT = every non-relocated byte equal at one proven location and every relocation target proven (imports, calls,
data, string payloads). FOLDED = an identical body the original linker merged into another function. Relocation-
unverified, masked or behavioural similarity is never counted.

| TU | strict | TU | strict | TU | strict |
|---|---:|---|---:|---|---:|
| blupi.cpp | 12 | decor.cpp | 17 | movie.cpp | 13 |
| button.cpp | 17 (+1 folded) | event.cpp | 87 | network.cpp | 18 |
| ddutil.cpp | 8 | jauge.cpp | 9 (+1 folded) | pixmap.cpp | 34 |
| decblock.cpp | 24 | menu.cpp | 8 | sound.cpp | 28 |
| decblupi.cpp | 20 | misc.cpp | 8 (+1 folded) | text.cpp | 7 |
| decdesign.cpp | 42 | decmove.cpp | 39 | decnet.cpp | 6 |

(Per-TU counts are the verifier's and include header inline bodies compiled into each TU.)

Data: `dectables.cpp` holds SB1 tables as initialized data whose bytes the verifier checks (relocations into them are
proven by payload). Calls to imports may go through exact `jmp [IAT]` thunks, as in the original.

Largest open bodies (best-known reconstructions in `src/`, not yet strict): `CEvent::DrawButtons` 0x16f70 (7,419 B,
65 differing bytes), `CEvent::ChangePhase` 0x19310 (6,922 vs 6,919 B), `CEvent::TreatEventBase` 0x1d230 (6,616 B,
8 register bytes), `CDecor::Build` 0x4350 (3,846 vs 3,866 B), `CDecor::DoInit` 0x1ef0 (CDecor allocation size only).

Per-TU details, near misses and blockers: `docs/*_TU.md`, `docs/CORE_CLUSTER.md`, `docs/UI_CLUSTER.md`,
`docs/ENGINE_PROBE.md`.

## Key facts ([EVIDENCE] unless noted)

- **Toolchain** ([docs/TOOLCHAIN.md](docs/TOOLCHAIN.md)): MSVC 5.0, compiler pinned to VS97 SP3 `cl 11.00.7022`
  (RTM differs on some functions), `/O2 /GX /ML /Gd /Gy`, statically linked VC5 SP3 `LIBC.LIB`, linker 5.10 family
  (exact build open), identical-COMDAT folding observed. Original build path `F:\Profil\` (`__FILE__`), reproduced
  by a temporary drive mapping for `sound.cpp` ([docs/BUILD_PATH.md](docs/BUILD_PATH.md)).
- **Layout** ([docs/TU_MAP.md](docs/TU_MAP.md), `oracle/tu_map.csv`): `.text` is TU blocks in alphabetical source
  order, functions in source order; game code ends at RVA `0x238b0`, VC5 LIBC follows (no SB1 `wave.cpp`).
- **Ancestry** ([docs/ANCESTRY_PLANETBLUPI.md](docs/ANCESTRY_PLANETBLUPI.md),
  [docs/ANCESTRY_SB2.md](docs/ANCESTRY_SB2.md)): Planet Blupi v1.8.0 sources compile to exact SB1 functions for
  sound/movie/misc/ddutil/pixmap/UI code; game logic is the Speedy Blupi engine (SB2 reconstruction is a structural
  hint; SB1's `CDecor` lacks SB2's trajectory arrays). Free Eggbert is a fork of sb2-decomp, not independent evidence.
- **Compiler knowledge** ([msvc5kb/README.md](msvc5kb/README.md)): rule catalog compatible with Blood II's VC5 KB;
  45 rules, 21 validated on SB1 (incl. TU symbol-count context resolved by genuine declarations for `StopSound`,
  statement order VC5-ORD-01, local-array layout BLUPI-LOC-01, SIB order from function-wide block shape BLUPI-REG-01).

## Reproduce

Inputs (ignored by git): `assets/` (original game files and ISO), `external/` (read-only references:
planetblupi, sb2-decomp, free-eggbert), historical toolchains under `C:\tools` (`msvc5_sp3`, `dx5sdk`, Ghidra 12.1),
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

## Distance to a byte-identical executable (2026-10-18)

| Item | State |
|---|---|
| Game code | 123,584 / 141,488 unique bytes STRICT (87.3 %); 3 functions open: ChangePhase (8 instr), TreatEventBase (8 instr), Build (8 masked bytes, size exact); docs/RESIDUAL_CAUSES.md |
| Whole file | same size (535,552 B); historical relink (2026-10-19, Build size exact): 121,401 bytes differ at the same offset. Nearly all are a +16 shift from RVA 0x2a30 to about 0x1f000: LINK keeps button's folded empty function (~CButton/~CMenu/~CJauge/OutputDebug) where the oracle keeps a later copy (ICF survivor, link inputs), plus the CRT filter survivor, imports and the three open functions |
| Link | no unresolved externals; Rich header: CVTRES 5.00.1668 entry exact, stamp-less count 268 vs oracle 270 (two modules unidentified) |
| `.text` | 16 bytes short; ICF survivors depend on LINK's MSVCRT `qsort` (historical runtime keeps the oracle's choice) |
| `.rdata` | size exact; 4,546 / 4,978 equal (import names, hints, descriptors exact; IAT/thunk values follow `.text`) |
| `.data` | size exact; 329,713 / 329,728 raw bytes equal (rest: pointers into shifted code); all 366 STRICT BSS references exact |
| `.rsrc` | byte-exact (docs/RESOURCES.md) |
| PE header | timestamp and `.text` size equal; 2 bytes differ: Rich count 268/270 and entry 0x24d20/0x24d40 (CRT filter survivor) |

## Where to continue

All 4 open functions have had several worker passes; the disproven families are recorded per target
(docs/COMPILER_STUDY.md, msvc5kb), so new work needs a genuinely new source or context hypothesis.
1. `event.cpp` register/scheduling residues: `TreatEventBase` (H1, terminator byte load), `DrawButtons` (330
   rematerialisation, pixmap scheduling), `ChangePhase` (constant webs) (docs/COMPILER_STUDY.md rounds 4-5).
2. `decblock.cpp` `Build` (zero-constant pool, shared drawing tail, swapped spill slots; docs/COMPILER_STUDY.md round 6).
3. Open questions: the internal layout of CDecor's proven 100,044-byte tail (docs/LAYOUT_DEBT.md); exact linker build (Rich header has no linker entry); data-section matching and a full relink.
