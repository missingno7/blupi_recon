# Speedy Blupi 2 / Speedy Eggbert 2: matching

Status (2026-10-23, round 3): `sb2/eggbert/en` (`assets/eggbert2/Eggbert2.exe`) is an active matching
target with its own oracle, source tree `games/sb2/src/` and manifest `games/sb2/recon.json`.
Accepted: **445 STRICT + 11 FOLDED functions, 135,648 / 172,992 game `.text` bytes (78.4%)**: engine 25,344, application 5,408, game logic 104,896 (round 1: 23,408 bytes, 13.5%). The same tree also verifies **Speedy Blupi 2** (`sb2/blupi/en`, `_EGAMES=0`): 452 STRICT + 6 FOLDED, 121,296 / 173,072 bytes (70.1%). The SB1 targets, certificates and relinked image are unchanged.

```powershell
.\.pyenv\python.exe tools\verify.py --target sb2/eggbert/en             # clean build + strict report
.\.pyenv\python.exe tools\crossmap.py --target sb2/eggbert/en --audit --candidates   # identity audit (before accepting)
.\.pyenv\python.exe tools\verify.py --target sb2/eggbert/en --accept    # promote into games/sb2/recon.json
.\.pyenv\python.exe tools\target_coverage.py --target sb2/eggbert/en    # accepted unique bytes: engine / app / game logic
.\.pyenv\python.exe tools\verify.py --target sb2/eggbert/en --build-dir NAME --tu games/sb2/src/X.cpp=scratch\NAME\X.cpp
.\.pyenv\python.exe tools\fnprobe.py --target sb2/eggbert/en games/sb2/src/X.cpp=CAND Symbol 0xRVA --aligned SIZE --regions
.\.pyenv\python.exe tools\c2trace\ctxpredict.py games/sb2/src/X.cpp Symbol --micro --sdk dx5sdk --include games/sb2/src --define WIN32 --define NDEBUG --define _WINDOWS --define nullptr=0
.\.pyenv\python.exe tools\crossmap.py --target sb2/eggbert/en           # SB1 correspondence + oracle/sb2-eggbert-en/tu_map.csv
.\.pyenv\python.exe tools\target_oracle.py --target sb2/eggbert/en ghidra|inventory
```

A candidate is accepted only when `verify.py` reports it STRICT (code bytes and every relocation bound;
unique or proven placement) **and** the cross-map audit finds no identity conflict (section 9).

## 1. Target and toolchain

| Item | Finding |
|---|---|
| Executable | `Eggbert2.exe`, 602,624 bytes, SHA-256 `e6b2ac0c…7d8d`, PE timestamp 2004-05-03 08:56:02 |
| Linker | PE 5.10; Rich `(0x00000, 271)`, `(0x60684, 1)`: SB1's family |
| Compiler | VC5 SP3, `/W3 /GX /O2 /ML /Gd /Gy`, `WIN32 NDEBUG _WINDOWS` (as SB1) |
| CRT | VC5 SP3 LIBC: 167/168 of SB1's CRT functions occur in SB2; `__setargv` located uniquely |
| Game code | 0x1000–0x2b3c0 (172,992 bytes), ending at the DirectX import thunks |
| Build path | `D:\Blupi\Profil egames 2\sound.cpp` (`tools/buildpath.py` folder mode; sandboxes fall back to "gate INCOMPLETE") |
| DirectX headers | manifest: `dx5sdk/INCLUDE` as a reproducing configuration; history: DX7 `dsound.h` (below) |

- [EVIDENCE] The one semantic SDK fact: `CSound::CreateSoundBuffer`'s `DSBUFFERDESC` is 0x24 bytes
  (`guid3DAlgorithm`), declared only by DirectX 7's `dsound.h`. With it, the function differs only in
  its `TRY_DS` `__LINE__` (64; SB1 and sb2-decomp have 63). Each executable has exactly one `TRY_DS`
  site.
- [EVIDENCE] Switching every TU to DX7 headers moves the next front-end symbol ID by +690 (pixmap.cpp)
  and +1,229 (decblupi.cpp), and the functions that then change (also ones without DirectX code) are
  key-order decisions whose windows are far narrower than that (`QuickIcon` +46, `IsLave` +45 under
  DX5). So register residues do not show the SDK; they show entity counts (msvc5kb `BLUPI-SB2-01`,
  `BLUPI-CTX-03`). [HYPOTHESIS] SB2 was compiled with DX7 headers throughout, with headers whose entity
  counts differ from today's reconstruction. [OPEN] the extra line before line 64 of sound.cpp.
- No per-TU include environment is used: nothing in the binary requires one.

### Speedy Blupi 2 (`sb2/blupi/en`)

[EVIDENCE] `assets/blupi2/BLUPI.EXE` (2001-05-03, "Version 2.2") is the Epsitec edition of the same
source. Its toolchain differs from Eggbert 2's: **VC5 RTM compiler** (c1xx/c2 11.00.7022; SP2 and SP3
give the same wrong register allocation in `DDCopyBitmap`/`DDColorMatch`, RTM gives 0 differing bytes)
and LINK 5.00 (no Rich header, separate `.idata`), i.e. the complete VC5 RTM toolchain, like SB1's
Speedy Eggbert 1. targets.json sets `toolchain_tree: rtm`, `_EGAMES=0` and the build path
`D:\Blupi\Profil\sound.cpp` (`tools/targets.py` now applies a target's `build_path`). Its inventory is
derived from Eggbert 2's (`oracle/sb2-blupi-en`, 446/449 game functions located; game code
0x1000-0x2b410). The unchanged SB2 tree verifies 106,512 / 173,072 of its bytes STRICT before any
edition switch.

[EVIDENCE] With the edition switches below, `verify.py --target sb2/blupi/en` passes (exit 0):
**452 STRICT + 6 FOLDED functions, 121,296 / 173,072 game `.text` bytes (70.1%)**, and Speedy Eggbert 2's
results stay identical function by function (`scratch/sb2/samestatus.py`: 0 differing). Real differences
found in Blupi 2's code and data, each under `#if _EGAMES` (worker reports `scratch/w-b2app`,
`scratch/w-b2event`):
- No timed publisher intro: `CEvent::m_introTime` is absent (`new CEvent` pushes 0x9be0, Eggbert 2
  0x9be4; the chat members sit 4 bytes lower), `IntroStep` is not compiled, and `UpdateFrame`,
  `DrawButtons`, `MousePosToSprite`, `ChangePhase` and `TreatEventBase` have no INTRO1/INTRO2 cases.
- Its own phase table: 38 phases (no 0x635/0x636), generated from BLUPI.EXE at VA 0x43f000.
- `ChangePhase`: ends on `WM_PHASE_DOQUIT`; cache and read errors set the CD retry state and go to
  INSERT (`CEvent::ChangePhase [Read error]`); the TESTCD retry stores count before phase. No
  "by www.epsitec.com" line on the gamer screen.
- `ReadConfig`: a missing `CD-Rom=` returns FALSE and `-nocd` skips the `DRIVE_CDROM` test; `AddCDPath`
  keeps SB1's folder-prefix test (data, image, image08, image16, movie).

targets.json records what the Eggbert 2 manifest cannot: sizes of Blupi 2's own bodies
(`_DDColorMatch` 240, `MousePosToSprite` 128, `TreatEventBase` 8368, `ReadConfig` 912, `AddCDPath` 336),
`UpdateFrame` at 0x13c0 (the derived address map had 0x13a0; inventory corrected from the binary), and
six symbols that LINK 5.00 keeps as the STRICT owner of a body Eggbert 2's linker folds (`~CButton`,
`~CMenu`, `OutputDebug`, `GetNbSessions`, `GetBenchmarkSuccess`, `CSound::GetEnable`).

[OPEN] Nine accepted Eggbert 2 functions differ on Blupi 2 only in operand or register order and are
excluded there: `GetBlupiInfo`, `IsBridge`, `IsDoor`, `IsTeleporte`, `IsOutWater` (and the already
withdrawn `IsDeepWater`, `SetMoveTraj`), `BltFast(int,int,POINT,RECT,int)`, `MouseBackDraw`,
`DrawButtons`, `ChangePhase`. Diagnostic extra declarations make each of the decblupi/pixmap ones equal,
so they are declaration-context residues of the RTM compiler on the same source, not code differences
(section 8); the c2trace tools are calibrated for SP3's c2.exe, so their windows are not yet measured.

## 2. Oracle, inventory and identifications

`oracle/sb2-eggbert-en/` (Ghidra 12.1 on Eggbert2.exe alone): 627 functions (449 game, 167,139 body
bytes; 170 CRT; 8 thunks). Ghidra misses address-only functions (callbacks, constructors given to
the vector-constructor iterator, `WindowProc`, `TimerStep`); the comparator locates them by bytes.

TU order is SB1's (blupi, button, ddutil, decblock, decdesign, decblupi, decnet, decmove, decor, event,
jauge, menu, misc, movie, network, pixmap, sound, text); no SB2-only TU (static analysis,
`scratch/w-bigfn/REPORT.md`). Identified blocks:

| RVA | Size | Function |
|---|---:|---|
| 0x1fe0 | 1,340 | `DoInit` (blupi.cpp; allocates CDecor 0x26320, CEvent 0x9be4) |
| 0xa850 | 35,449 | `CDecor::BlupiStep`: one function, one `ret`, no jump tables, 9,211 instructions, one caller (`MoveStep`) |
| 0x9f20–0xa7e0, 0x132d0 | | `BlupiSearchIcon`, `BlupiIsGround`, `BlupiRect`, `BlupiAdjust`, `BlupiBloque`, `BlupiDead` |
| 0x1c5e0–0x1ca00 | | `AdaptDoors`, `OpenDoorsTresor`, `OpenDoor`, `OpenDoorsWin`, `OpenGoldsWin`, `DoorsLost` |
| 0x1ca50 | 342 | `CEvent::CEvent` |
| 0x23e00 | 7,937 | `CEvent::TreatEventBase` and its five switch tables (to 0x25ed3) |
| 0x28250 / 0x28890 | | `CPixmap::CacheAll` / `CPixmap::Create` (six arguments) |

## 3. Coverage

Accepted unique bytes per TU (`tools/target_coverage.py` counts them by group):

| TU | STRICT | FOLDED | bytes |
|---|---:|---:|---:|
| blupi.cpp | 13 |  | 5,408 |
| button.cpp | 18 | 1 | 1,808 |
| ddutil.cpp | 8 |  | 2,064 |
| decblock.cpp | 28 |  | 10,528 |
| decblupi.cpp | 38 |  | 9,088 |
| decdesign.cpp | 45 |  | 16,832 |
| decmove.cpp | 42 |  | 17,616 |
| decnet.cpp | 7 |  | 1,120 |
| decor.cpp | 25 |  | 12,032 |
| event.cpp | 86 |  | 37,680 |
| jauge.cpp | 10 |  | 560 |
| menu.cpp | 7 | 1 | 480 |
| misc.cpp | 11 | 1 | 3,872 |
| movie.cpp | 13 |  | 1,152 |
| network.cpp | 19 | 1 | 2,672 |
| pixmap.cpp | 42 | 4 | 8,512 |
| sound.cpp | 26 | 3 | 3,552 |
| text.cpp | 7 |  | 640 |

Not yet STRICT (about 37,500 oracle bytes): `CDecor::BlupiStep` 35,449 (SB2 body in the tree: 9,187/9,211
instructions aligned, 20 residue regions: operand order in the wind cell expressions, two constant
initialisation orders, literal 1/10 promotion in the vertigo and Jeep edge guards, one `test` operand
order; `scratch/w-blupistep/REPORT.md`); twelve small decblupi predicates and `MoveObjectSort` (operand-order context);
`CEvent::CEvent` (one scheduling position); `CSound::CreateSoundBuffer` (line 64); withdrawn functions
below.

Withdrawn from the gate (declaration context, section 8; source unchanged, each with evidence in the
manifest): `CDecor::IsWorld`, `IsDeepWater`, `SetMoveTraj`, `IsMoveTraj`, `IsVentillo` (1,040 bytes).
Withdrawn earlier and re-accepted after later genuine context: `MouseMove` (twice), `BltFast`,
`NetAscenseurSynchro`, `GetBlupiInfo`, `IsLave`, `IsRessort`, `IsVentillo` (once). `MouseMove` and
`BltFast` returned in round 3 once inherited SB1 definitions that SB2 does not have (`CButton::Redraw`,
`CButton::GetEnable`, the empty `CPixmap::Fill` stub; no callers, no SB2 body) were removed.

Shared bodies with unproven owner (counted once): `CButton::GetHide`/`CNetwork::IsHost` (0x28100, not
accepted under either name); `CPixmap::SetTrueColorBack`/`CSound::SetState` (0x28cf0) and
`CPixmap::SetTrueColorDecor`/`CSound::SetCDAudio` (0x28d00), mutually FOLDED (both have compiled
callers; the bodies lie in pixmap.cpp's block); `CEvent::NetDraw`/`DrawMap`, `BuildUp`/`PlayDown`/
`PlayMove`/`PlayUp` (as in SB1).

## 4. SB2 structures and behaviours recovered (all [EVIDENCE] from STRICT code unless marked)

- **CPixmap**: `m_bBenchmarkSuccess` (+4), `m_bTrueColorDecor` (+0xc); `~CPixmap` tail-calls a release
  method (`ReleaseAll`, name [HYPOTHESIS]); `Create(HWND, POINT, fullScreen, trueColorBack,
  trueColorDecor, mouseType)`; `CacheAll` (re)loads all atlases; `BackgroundCache` picks image16/image08
  by file kind and the true-colour flags; `Benchmark` (data\bench.blp); arrow hot spot x = 25.
- **CSound**: CD audio (`m_bCDAudio`, default TRUE; `PlayCDAudio` with `MCI_FROM|MCI_TO` TMSF
  tracks), music by number (`sound\musicNNN.blp`), `MCI_STOP` before `MCI_CLOSE`, MIDI volume 10.
- **Application**: `ReadConfig` (CD path fallback, benchmark flag, separate true-colour flags, CD audio,
  32,000,000-byte physical-memory cut-off), `DoInit` (`SetBenchmarkSuccess` + `CacheAll`), `WindowProc`
  (`BackgroundCache("little.blp")`), `UpdateFrame` (`IntroStep` on the intro screens).
- **CEvent** (0x9be4 bytes): `int m_menuDecor[20]` at +0x9810 (one 80-byte clear in the constructor:
  [0] selected editor menu, [1..11] each menu's choice; array shape [HYPOTHESIS]); `m_9984` after
  `m_introTime` (cleared in ChangePhase; meaning [OPEN]). The phase table has 40 phases (290,444 bytes,
  generated from the binary; background names without the `image%.2d\` prefix). Multiplayer missions are
  `m_multi + 200`, demos `m_demoNumber + 300`, 12 multiplayer missions. Intro screens
  (`WM_PHASE_INTRO1/2`, 60 ticks each). 32 regions; `WM_DIMS1..4` = `WM_USER+241..244`. The settings
  screen switches true-colour backgrounds and decors (reloading via `LoadImageFromDisc`). The editor
  has 11 decor menus (`WM_DECOR1..11` → `DeleteCel`, `PlaceDecor1..10`); menu 11 hides six icons unless
  "build official missions" is on.
- **CDecor** (0x26320 bytes = `new CDecor`): sb2-decomp's member list with two corrections:
  `m_balleTraj`/`m_moveTraj` are 1,300 bytes (13-byte rows, `y*13 + x/8`) and `m_notifText` keeps 4
  lines; `m_doors` is `BYTE[200]`; Jeep precedes Tank (BlupiSearchIcon's icon remaps; the save code),
  `m_bBuildOfficialMissions` at +0xd800 (its setter at 0x9e10). The save record `DescGameFile` is
  0xdeb0 bytes (trajectory maps, key, invert/balloon/over/crush/no-ascent timer/dynamite flags).
- **CEvent large functions** (STRICT): 25 cheat codes (22..24 new, life cheat sets 10 lives); eleven
  editor toolbar cases written out; `ChangePhase` uses `BackgroundCache`, byte-wide door scans,
  official missions up to 320, `m_9984` reset, `m_introTime` reset on the intro screens.
- **BlupiStep / decblupi**: `MAXQUART` is 441 (tile collision table bound, `cmp 0x1b9`); trajectory maps
  are flat 1,300-byte arrays indexed `y*13 + x/8`; `BlupiSearchIcon` opens decblupi.cpp (SB1 had it at
  decdesign.cpp's end; placement [HYPOTHESIS], chosen by sb2-decomp's grouping and compiler context).
- **Doors**: treasure doors open for icons 0x1a5..0x1a4+`m_nbTresor`; `OpenDoor` builds a `TYPE_DOOR`
  move object; gold doors at `m_doors[180 + mission/10]`.

## 5. What SB1 gives SB2

- Unchanged SB1 TUs that are STRICT for every located function: ddutil, jauge, menu, movie, network,
  text. Most game-logic gains came from layouts read off the binary applied to SB1's code (CEvent +
  CDecor layouts: event.cpp 52 → 73 STRICT, game logic +11 KB in one step).
- Of SB1's functions, 143 recur byte-exact (masked) and 109 with the same instruction sequence
  (`tools/crossmap.py`); `DrawButtons` and `BlupiStep` are STRICT in SB1 and are the starting point for
  SB2's (ChangePhase/TreatEventBase were only EQUIVALENT in SB1).

## 6. Corrections to sb2-decomp (checked against STRICT code)

CPixmap layout (no `m_field5_0x14/0x18`); CSound PlayMusic/PlayCDAudio/SuspendMusic/constructor;
`TRY_DS` line 63 → 64; CDecor trajectory arrays (1,251 → 1,300 bytes) and `m_notifText[5]` → `[4]`;
`LoadImageFromDisc` passes `NULL`, not `m_hWnd`; `CreateButtons` subtracts six icons (sb2-decomp:
`iconMenu++`); `PlaceDecor` index/flag details and `m_bBuildOfficialMissions` (not `m_bMulti`) in
PlaceDecor10; `DoorsLost` compares the multiplied index; `BlupiDead` has no Vent reset; BlupiStep's
action switch is a condition chain, with per-vehicle explosion bodies and motor-sound sites
sb2-decomp lacks, and without its debug tail (`scratch/w-bigfn/REPORT.md` section 3); BackgroundCache's
image16 rule; ReadConfig's `>= 3100` test. Free Eggbert's benchmark `POSDRAW`/`CHOBJECT` values are wrong
for this binary.

## 7. Process

Parallel Codex workers (`scratch/cx/*.md` briefs; reports in `scratch/w-*/REPORT.md`) reconstruct
disjoint TUs in scratch and verify with their own `--build-dir`; only the supervisor writes
`games/sb2/src` and the manifest. Integration is tested on a full copy of the tree with every TU
substituted (`scratch/sb2/integ.sh`), because shared headers change other TUs (section 8).

## 8. Main blocker: declaration context

[EVIDENCE] VC5 orders commutative operands and some loads by keys hashed from front-end symbol IDs
(msvc5kb `BLUPI-CTX-03`). Every declaration added to a shared header (decor.h, pixmap.h, event.h)
shifts the IDs of everything after it in every TU that includes it. Measured windows
(`ctxpredict.py`, relative to the header at the time): `CButton::MouseMove` [−2, +5],
`CDecor::NetAscenseurSynchro` [0, +1], `CDecor::IsVentillo` [−255, 0]: at that point no addition at all
was possible without breaking an accepted function.

Policy: the SB2 headers move towards authenticity: a declaration is added when an SB2 body that is
proven to exist needs it (never as filler). A function that matched only through SB1's header entity
count and breaks is withdrawn with its window as evidence (it may return: `MouseMove` broke, then
matched again after later genuine declarations). 1,488 bytes are withdrawn now; every integration that caused a withdrawal gained more than it cost
(the smallest: the SB2 `BlupiStep` body, net +700 bytes, because the new context also matched ten
other functions). The full sb2-decomp declaration set
(+31 CDecor methods) breaks four accepted functions, so sb2-decomp's header is not SB2's either.
Recovering the real header context (method set and order, include sets) is the next structural task.

[EVIDENCE] Window survey (round 3, `tools/c2trace/ctxwindows.py`, one capture and one c2 replay per TU;
results in `build/ctxwin/*.json`): 84 context-sensitive functions in 15 TUs. The STRICT functions of
event.cpp and decblock.cpp tolerate a uniform shift of exactly [0, 0], decblupi.cpp's [−1, +3], decor.cpp's
[−7, 0]: many independent windows meeting at today's point, which says the entity counts in front of
those functions are already right. The remaining decblupi mismatches (`IsWorld`, `IsDeepWater`,
`IsVentillo`, the trajectory helpers) are decided by their own parameters, whose IDs depend on every
entity defined before them in the TU, i.e. on the exact source of earlier bodies (BlupiStep, the
unmatched predicates); several need shifts of +40..+220 that no header change could give without
breaking event.cpp. So the lever is authentic earlier bodies, solved function by function in
definition order, not header totals.

[EVIDENCE] Exact ledgers of decblupi.cpp (`scratch/w-context/REPORT.md`): BlupiStep's body consumes 838
front-end IDs, its helpers 185 more before `IsWorld`. Isolated diagnostic shifts flip each remaining
mismatch to zero differing bytes (`IsWorld` +187, `IsDeepWater` +45, `SetMoveTraj`/`IsMoveTraj` +5/+4,
`IsVentillo` +216; BlupiStep's wind expression +179, which removes 5 of its 20 regions). [NEGATIVE]
No single insertion of k entities (k = −300..600) at any function boundary, shifting only IDs after
it, satisfies all 25 context-sensitive functions together (exact c2 key model,
`scratch/sb2/deadcode2.py`): the original TU differs from ours in several local places, not by one
missing block (such as one dead-stripped function). [NEGATIVE] Local respellings of IsWorld,
IsDeepWater and the trajectory helpers are inert. `IsBlitz` became STRICT by returning BlitzActif's
result unnormalised (+2 IDs for later functions, all still inside their windows).

[EVIDENCE] Round 3 results: `CDecor::Build` is STRICT (5,024 bytes, 80 relocations) after recovering
two source forms from its stack-home map (animated-column storage in the existing `x`, one combined
big-decoration Y reset) with its entity count unchanged (`scratch/w-build/REPORT.md`). BlupiStep's
source-form residues: snapshotting the key mask before the IsDoor/key guard removes three residue
groups (9,199/9,211 instructions, 9 regions) but loads the member before its guard, which the oracle
does not; that candidate stays in `scratch/w-step2` and the tree keeps the 9,187/9,211 body. Guarded
variants (nested guards, conditional snapshots, comma assignments) are inert or worse
(`scratch/w-step2/REPORT.md`).

## 9. Tools and limitations

- `verify.py`: `source_root`, own-manifest `--accept`, folded owners in the game's tree.
- `crossmap.py --audit [--candidates] [--results PATH]`: callers of each symbol in SB1 must call the SB2
  address; calls of STRICT functions must land on the same-named SB1 function, a documented rename
  (`source_correspondence`) or a shared body the run locates under that name.
- `target_coverage.py`: accepted unique bytes per group (`coverage_groups` in the manifest).
- `fnprobe.py --target`; `buildpath.py` folder mode with a sandbox fallback.
- Limitations: STRICT placement by unique bytes cannot name a renamed/split body without a compiled
  caller (the audit catches it); fold ownership flips with the set of compiled callers (cmpfunc is
  pinned); withdrawn entries stay forced at their RVA; `relink.py`/`residues.py`/permuter know only SB1.

## 10. SB1 protection

[EVIDENCE] After each integration and at the end of round 2: the verifier output of all nine SB1
targets is byte-identical to the pre-SB2 baseline (equivalence certificates checked); the relinked
SB1 image is `557ca92e…`; nothing under `src/`, `port/`, `assets/` or `external/` changed.

## 11. Structure

No change justified. SB2 lives beside `src/`; the tools take a manifest `source_root`. The engine TUs
that are textually identical in both trees include headers that differ (`pixmap.h`, `sound.h`,
`decor.h`, `event.h`), and those headers decide code generation through declaration context, so a
shared engine source file would need per-game headers anyway. Revisit when SB2's headers are final.

## 12. Next targets (estimated bytes)

1. **Declaration/TU context** (structural; unlocks most of what is left): `BlupiStep` (35.4 KB, 20
   regions), the withdrawn 1.5 KB, the decblupi predicates and `MoveObjectSort` (about 2 KB). Collect
   `ctxpredict.py` windows for every sensitive function, then reconstruct the genuine SB2 header method
   sets/order and include lists that satisfy them; sb2-decomp's header is not it (section 8).
2. **`Build`** (5 KB): a stack-slot allocation investigation like SB1's (msvc5kb `BLUPI-ALLOC-16`).
3. Small items: the CEvent constructor's scheduling, `CreateSoundBuffer`'s line, the owners of shared
   bodies (0x28100, 0x28cf0, 0x28d00), `GetDoors(short*)` → `BYTE*`.
4. [HYPOTHESIS] SB2's ChangePhase/TreatEventBase became STRICT with SB2's authentic forms where SB1's
   stayed EQUIVALENT; SB1's two residues may deserve a look with the same approach (SB1 work, not SB2).
5. `sb2/blupi/en`: the nine RTM context residues above (about 15 KB with `DrawButtons` and
   `ChangePhase`); a c2trace calibration for VC5 RTM's c2.exe would let `ctxwindows.py` measure them.
