# Speedy Blupi 1 — matching decompilation

Reconstructing the C++ source of **Speedy Blupi 1** (Epsitec, 1998; `BLUPI.EXE`, English/US build, SHA-256
`be6de5a2…8290ac`) so that the historical compiler reproduces the original machine code exactly. Matching comes
first. Rules for contributors and agents: [AGENTS.md](AGENTS.md).

**Native SDL3 port (separate from the matching work):** [port/README.md](port/README.md) builds this same
`src/` unchanged with a modern compiler and SDL3. It is a faithful 640×480 baseline: menus, levels, sound,
MIDI music (GeneralUser GS SoundFont, a git submodule) and the original attract-mode demos run on Windows x64. The port
never modifies the historical source.

## Status (verified with `tools/verify.py`)

Recovery is consolidated: [milestone and cleanup record](docs/RECOVERY_MILESTONE.md).
Obsolete experiments are quarantined under ignored `to_delete/`; compact research
inputs remain in `build/recovery/` and canonical findings remain in Git.

**Complete standalone source build available:** `tools/build.py` compiles all 19 TUs and resources without
the original EXE. The two remaining game bodies are accepted as **EQUIVALENT** under checked static assumptions;
strict matching research is deferred until new evidence. See [docs/STANDALONE.md](docs/STANDALONE.md)
for certificates, build instructions and validation limits. A cnc-ddraw runtime smoke test reaches the first
playable level and exits cleanly; see [docs/RUNTIME.md](docs/RUNTIME.md). Full gameplay equivalence remains unproven.

**127,456 / 141,488 game `.text` bytes (90.1 %)**, built with one shared
header set for every TU (no per-TU preprocessor views; no active withdrawals; withdrawal history in
[docs/WORKFLOW.md](docs/WORKFLOW.md) and [docs/HEADERS.md](docs/HEADERS.md)). Two mapped game functions remain non-STRICT:
`CEvent::ChangePhase` and `CEvent::TreatEventBase`, each with eight differing instructions.
Fresh verification and historical relink gates pass with zero accepted regressions, including the sound build path
([docs/BLOCKER_CHECKS.md](docs/BLOCKER_CHECKS.md), 2026-10-19). The verifier reports 411 STRICT and 6 FOLDED emitted
function records; these include repeated bodies across TUs.
STRICT = every non-relocated byte equal at one proven location and every relocation target proven (imports, calls,
data, string payloads). FOLDED = an identical body the original linker merged into another function. Relocation-
unverified, masked or behavioural similarity is never counted.

| TU | strict | TU | strict | TU | strict |
|---|---:|---|---:|---|---:|
| blupi.cpp | 13 | decor.cpp | 21 | movie.cpp | 13 |
| button.cpp | 18 (+1 folded) | event.cpp | 88 | network.cpp | 19 (+2 folded) |
| ddutil.cpp | 8 | jauge.cpp | 9 (+1 folded) | pixmap.cpp | 36 (+1 folded) |
| decblock.cpp | 26 | menu.cpp | 8 | sound.cpp | 28 |
| decblupi.cpp | 20 | misc.cpp | 10 (+1 folded) | text.cpp | 7 |
| decdesign.cpp | 42 | decmove.cpp | 39 | decnet.cpp | 6 |

(Per-TU counts are the verifier's and include header inline bodies compiled into each TU.)

Data: `dectables.cpp` holds SB1 tables as initialized data whose bytes the verifier checks (relocations into them are
proven by payload). Calls to imports may go through exact `jmp [IAT]` thunks, as in the original.

EQUIVALENT bodies (best-known reconstructions in `src/`, strict work deferred): `CEvent::ChangePhase` 0x19310
(32 differing non-relocation bytes) and `CEvent::TreatEventBase` 0x1d230 (8 register bytes).
`CEvent::DrawButtons`, `CDecor::Build` and `CDecor::DoInit` are STRICT.

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
  SB1 validations include TU symbol-count context resolved by genuine declarations for `StopSound`,
  statement order VC5-ORD-01, local-array layout BLUPI-LOC-01, and SIB order from function-wide block shape BLUPI-REG-01.

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

## Distance to a byte-identical executable (2026-10-19)

| Item | State |
|---|---|
| Game code | 127,456 / 141,488 bytes (90.1 %); 2 functions open: ChangePhase and TreatEventBase, 8 differing instructions each; docs/RESIDUAL_CAUSES.md |
| Whole file | same size (535,552 B); fresh historical relink: 121,401 bytes differ at the same offset. Most are a +16 shift from RVA 0x2a30 to about 0x1f000: LINK keeps button's folded empty function (~CButton/~CMenu/~CJauge/OutputDebug) where the oracle keeps a later copy (ICF survivor, link inputs), plus the CRT filter survivor, imports and the two open functions |
| Link | no unresolved externals; Rich header: CVTRES 5.00.1668 entry exact, stamp-less count 268 vs oracle 270 (two modules unidentified) |
| `.text` | size exact; 121,100 differing bytes; the historical runtime alone does not recover the oracle's ICF survivors |
| `.rdata` | size exact; 4,680 / 4,978 initialized bytes equal |
| `.data` | size exact; 329,727 / 329,728 raw bytes equal; one differing byte in a CRT pointer |
| `.rsrc` | byte-exact (docs/RESOURCES.md) |
| PE header | timestamp and `.text` size equal; 2 bytes differ: Rich count 268/270 and entry 0x24d20/0x24d40 (CRT filter survivor) |

## Where to continue

Current state and the paused blockers: **docs/STATUS.md** (2026-10-19).

The tested families and unresolved compiler decisions are recorded in `docs/RESIDUAL_CAUSES.md` and `msvc5kb`.
TreatEventBase needs a rotor-neutral rank-zero toggle; ChangePhase needs the oracle's test order with PLAY
allocated before zero. Original project/build evidence or LINK 5.10.7287 could help discriminate the linker
input and version hypotheses. The latest declaration and historical compiler-front-end controls produced no
new match ([docs/BLOCKER_CHECKS.md](docs/BLOCKER_CHECKS.md)).

The independent [Planet Blupi study](docs/PLANET_ORACLE.md) found a real RTM/SP3 back-end fingerprint and natural
Boolean/constant-allocation controls. It does not close either SB1 function; historical acceptance remains 90.1%.
