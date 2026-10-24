# Speedy Blupi 2 / Speedy Eggbert 2: matching

Status (2026-10-24): `sb2/eggbert/en` (`assets/eggbert2/Eggbert2.exe`) is an active matching
target with its own oracle, source tree `games/sb2/src/` and manifest `games/sb2/recon.json`.
Accepted **Eggbert 2 en: 450 STRICT + 12 FOLDED, 136,448 / 172,992 game `.text` bytes**, plus
4 EQUIVALENT functions / 36,304 bytes. **Blupi 2 en: 451 STRICT + 5 FOLDED,
121,968 / 173,072 bytes**, plus 10 EQUIVALENT / 51,072 bytes; **Blupi 2 de** and **fr** each have
the same independently verified counts. Section 11 documents the minimal edition overlay and
the three recovered Blupi STRICT functions. All fourteen matching target gates exit 0; the sandbox
sound-path gate remains INCOMPLETE as expected. SB1 function reports are unchanged.

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

[EVIDENCE] The initial integration of the edition switches below passed with
**452 STRICT + 6 FOLDED functions, 121,296 / 173,072 game `.text` bytes (70.1%)**, and Speedy Eggbert 2's
results stayed identical function by function (`scratch/sb2/samestatus.py`: 0 differing). These were
complete comparator totals, not accepted coverage; current accepted totals are above. Real differences
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
withdrawn `IsDeepWater`), `BltFast(int,int,POINT,RECT,int)`, `MouseBackDraw`,
`DrawButtons`, `ChangePhase`. Diagnostic extra declarations make each of the decblupi/pixmap ones equal,
so they are declaration-context residues of the RTM compiler on the same source, not code differences
(section 8). RTM expression tracing is now calibrated; measured windows and the remaining allocation
boundary are recorded there.

[EVIDENCE] German and French Speedy Blupi 2 (`sb2/blupi/de`, `sb2/blupi/fr`;
`assets/blupi2_variants/{deutsch,francais}/Blupi.exe`; the `english` copy is BLUPI.EXE itself) have
`.text` and `.data` byte-identical to the English build; only `.rsrc` (string tables) differs, so they
need no source switch. They are active with the English target's settings and overrides and their own
inventories (`oracle/sb2-blupi-de`, `oracle/sb2-blupi-fr`): 449 STRICT + 5 FOLDED, 121,712 bytes each,
identical function results (`scratch/w-b2lang/REPORT.md`).

### Edition policy (2026-10-23)

[SUPERSEDED 2026-10-24] Replaced by the independent-target policy in AGENTS.md: each SB2 edition is matched on its own,
with edition-specific source where the originals differ; no edition gives up STRICT coverage for another. The text
below records the earlier rule and its applications.

[EVIDENCE] Every structural fix so far that helped one SB2 edition hurt the other (pixmap.cpp's `CacheAll`
order: Blupi 2 +512 STRICT bytes, Eggbert 2 −512; the decblupi predicates and GetBlupiInfo/MoveObjectSort
need opposite contexts per edition): the 2001 and 2004 originals were compiled from different source
revisions (round 6-13 notes). The large residues (BlupiStep, ChangePhase, the decblupi context gaps) fail on
each edition alone.

Policy: **Speedy Eggbert 2 is the primary STRICT target; Speedy Blupi 2 is a protected secondary.** A change
may cost a Blupi 2 STRICT function only when Eggbert 2 gains and the Blupi 2 function keeps a re-proven
EQUIVALENT certificate in its target tier; the reverse is not allowed. Edition-specific source (`#if _EGAMES`)
still needs both executables to show the difference. EQUIVALENT never counts as STRICT.

[EVIDENCE] First policy application (`scratch/w-eggprimary/REPORT.md`): `IsMoveTraj` computes its cell in a used
`POINT cel` (the form that matches Eggbert 2), making it STRICT on Eggbert 2 (+112 bytes, no loss). Blupi 2
en/de/fr lose that 112-byte STRICT body; it is certified EQUIVALENT in their target tier (static proof, negative
control) and withdrawn there with that certificate as evidence. Eggbert 2: 448 STRICT + 12 FOLDED, 136,192
bytes; Blupi 2: 448 STRICT + 5 FOLDED, 121,600 bytes, 12 EQUIVALENT.

### Structural-recovery experiment: decision (2026-10-23)

Isolated arms, all compiled serially in scratch (about 500 compilations in total): `scratch/w-struct` (decblupi:
inline helpers, stripped functions, class order), `w-island` (decblupi with the 2013 C# port's structure,
external/mobile-eggbert-legacy), `w-pixstruct` (pixmap.cpp original order), `w-tables` (20 editor tables),
`w-sb1struct` + `w-sb1island` (SB1 event.cpp), `w-invsearch` (TreatEventBase inverse search), `w-eggprimary`,
`w-audit` (dependency map, authenticity annotations).

**A. Islands.** [EVIDENCE] c2 resets its per-function state; other code reaches a function only through
front-end/label ID counts in the same TU (`w-sb1island`). Independent islands: decblupi.cpp, decblock.cpp,
pixmap.cpp, event.cpp. Coupling: shared headers (decor.h → sound.h, pixmap.h, network.h; event.h; def.h; the
DirectX headers), and data definitions placed before code in the same TU (decblock's tables before
GetBlupiInfo). Function-local residues: BlupiStep's allocator/scheduling groups, ChangePhase (zero/PLAY
priority), TreatEventBase (rank-0 temp).

**B. Authenticity.** [EVIDENCE] Most remaining functions are STRUCTURE_SUPPORTED (logic confirmed by several
binaries); exact forms are STRUCTURE_OPEN or MATCH_AMBIGUOUS. COMPILER_STEERED forms exist (IsOutWater's POINT
cel, the trajectory flusher loops, CEvent's constructor loop, SB1's DrawButtons edits and ChangePhase guard
order), but none was shown to lock another function. One local optimum is demonstrated: both originals emit
pixmap.cpp in one order, and Eggbert 2's BltFast, MouseBackDraw and QuickIcon match only with our non-original
order. [OPEN] Unknowable from the binaries: the 2001 vs 2004 declaration contexts and the allocators' internal
histories (two different histories give the same bytes).

**C. Experiment results.** No structural change improved a region on the primary edition without losses.
Inline helpers renumber back-end IDs only (`MAKE_POINT`: Eggbert 2 +112 −592 bytes); the attested uncalled
`IsNormalJump` is inert (+16 entities, 44/44 payloads unchanged); the C# port's method order equals ours;
`decor.h` reordering is inert; pixmap's original order nets Eggbert 2 −1,296 and Blupi 2 −272 bytes; editor
tables in address order +123 `.data` bytes but lose Eggbert 2's GetBlupiInfo; SB1 event.cpp has no locking
form; TreatEventBase: 10 natural forms, no change. The experiments explain the pattern (two source revisions,
local optima) but not the missing source.

**D. Editions.** Eggbert 2 primary, Blupi 2 a protected secondary (policy above) works; its first use gave
+112 STRICT bytes. A source split per edition is not justified: the evidenced edition differences are
invisible declaration context, so a split would have to invent it.

**E. Recommendation: preserve the STRICT/EQUIVALENT reconstruction and prioritise the SB2 SDL3 port.**
Continue byte work only from new evidence (an original source or symbol witness) or for already-known natural
forms under the edition policy. The C# port is a strong behaviour and naming reference for the port.

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
| event.cpp | 87 | 1 | 38,032 |
| jauge.cpp | 10 |  | 560 |
| menu.cpp | 7 | 1 | 480 |
| misc.cpp | 11 | 1 | 3,872 |
| movie.cpp | 13 |  | 1,152 |
| network.cpp | 19 | 1 | 2,672 |
| pixmap.cpp | 42 | 4 | 8,512 |
| sound.cpp | 26 | 3 | 3,552 |
| text.cpp | 7 |  | 640 |

Not yet STRICT on Eggbert 2: `CDecor::BlupiStep` 35,449 (SB2 body in the tree: 9,187/9,211
instructions aligned, 20 residue regions: operand order in the wind cell expressions, two constant
initialisation orders, literal 1/10 promotion in the vertigo and Jeep edge guards, one `test` operand
order; `scratch/w-blupistep/REPORT.md`); `MoveObjectSort` (operand-order context),
`CSound::CreateSoundBuffer` (descriptor size and line 64), and the four withdrawn functions below.
`CEvent::CEvent` is now STRICT in both editions (blocker repair below).

Withdrawn from the gate (declaration context, section 8; source unchanged, each with evidence in the
manifest): `CDecor::IsWorld`, `IsDeepWater`, `IsMoveTraj`, `IsVentillo` (960 bytes).
`SetMoveTraj` has been re-accepted in both editions (trajectory repair below).
Withdrawn earlier and re-accepted after later genuine context: `MouseMove` (twice), `BltFast`,
`NetAscenseurSynchro`, `GetBlupiInfo`, `IsLave`, `IsRessort`, `IsVentillo` (once). `MouseMove` and
`BltFast` returned in round 3 once inherited SB1 definitions that SB2 does not have (`CButton::Redraw`,
`CButton::GetEnable`, the empty `CPixmap::Fill` stub; no callers, no SB2 body) were removed.

[EVIDENCE] The shared 16-byte getter belongs to **CNetwork::IsHost**, STRICT at Eggbert RVA `0x28100`
and Blupi RVA `0x28150`: both occupy the final network.cpp slot, and all ten original call sites call
IsHost. It is now accepted on all four SB2 targets (+16 unique bytes each; Eggbert 136,432 -> 136,448,
Blupi en/de/fr 121,360 -> 121,376). CButton::GetHide has no compiled caller; its identical byte hit
is RELOC-UNVERIFIED, an attribution boundary rather than a body mismatch or a separate STRICT body.
Only inventory ownership/acceptance changed; certificates were re-proved with only their inventory
hash changing (`scratch/w-sb2small/REPORT.md`). The DX7 sound witness remains scratch-only.

Other shared bodies with unproven owner (counted once): `CPixmap::SetTrueColorBack`/`CSound::SetState` (0x28cf0) and
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
matched again after later genuine declarations). 1,040 bytes are withdrawn on Eggbert 2; every integration that caused a withdrawal gained more than it cost
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

[EVIDENCE] Round 5 (`scratch/w-ctxprofile/REPORT.md`): three body respellings after `IsOutWater` (a
`POINT cel` in IsOutWater, pointer loops in the two trajectory flushers; +1/+2/+2 entities) make
`SetMoveTraj` and `IsMoveTraj` STRICT on Eggbert 2 with every other function kept (192 bytes). That
candidate was not integrated because Blupi 2 loses `IsMoveTraj`; the later trajectory repair below
preserves the SetMoveTraj gain in both editions while retaining Blupi's IsMoveTraj. On
Blupi 2 (RTM tools, below) `SetBalleTraj` and `SetMoveTraj` have the same decisive comparator sign at
every uniform shift in ±4096 while needing opposite choices, so a positional difference between them
is required in the 2001 source.

[EVIDENCE] VC5 RTM's c2 uses the same key and hash algebra as SP3 (`scratch/w-rtmtrace/REPORT.md`:
address table, 4,539 hash and 884 comparator calls observed live, zero mispredictions). Blupi 2's ten
operand-order residues are each removed by a diagnostic uniform shift per TU (decblock +65, pixmap +32,
event +28, decblupi positional +92/+96/+101/+112); `ChangePhase`'s residue is register allocation and
no comparator choice changes it.

### BlupiStep with the corrected model (round 6, `scratch/w-stepwind/REPORT.md`)

[EVIDENCE] The two editions' originals differ in the wind block: Eggbert 2 (0xa850) computes the cell
index from `end.x` before `end.y`, Blupi 2 (0xa840, 35,452 bytes; the inherited 35,449 was corrected from
its RET at 0x132bb) computes y first, **as our source does**. Our SP3 and RTM compiles of BlupiStep are
byte-identical and have the same 37 context-sensitive comparator decisions (window [-3,0]). Aligned:
Eggbert 2 9,187/9,211 instructions in 20 regions, Blupi 2 9,199/9,212 in 14.
- Wind: a key-order decision on `end` (ID 33613; decisive bit 8). Keeping `this` and the later locals
  fixed, every `end` ID in 33608..33747 gives the current (Blupi 2) order, so no reordering of the early
  locals can give Eggbert 2's; a +179 suffix shift fixes Eggbert 2's wind and the dependent opening (20 → 15
  regions) and breaks Blupi 2's. One source cannot give both under the same context: Eggbert 2's
  original differs from Blupi 2's before `end` (source or header context; [OPEN] what).
- The other residues are not symbol-ID decisions: constant 1/2 order (scheduling), literal 1/10
  (constant promotion / live-range allocation), the key-mask TEST (unequal operand costs; encoded operand
  orientation). They remain [OPEN] outside the key model.
- Stack frame 0x16c in both originals and ours; every retained local's home (`end` at ESP+0x10, ...)
  matches. [NEGATIVE] sb2-decomp's direct wind index is byte-identical to our `windCell` form;
  reusing SB1's `tinyPoint` (either assignment order) is worse on both editions. No candidate.

### Speedy Blupi 2 context and ChangePhase (round 6, `scratch/w-b2ctx/REPORT.md`)

[EVIDENCE] Edition-only code: Eggbert 2 has one function Blupi 2 lacks, `IntroStep` (its declaration
guard is byte-inert, -1 entity in event.cpp); `ChatFlush`/`ChatPop` exist in both (layout offsets
differ). `CheckWorld1` differs: Blupi 2 checks `data\world001.blp` on the CD; Eggbert 2's
ChangePhase calls a `mov eax,1; ret` body shared with `CPixmap::Flush`. `def.h` now sets
`REQUIRE_CD` to `!_EGAMES`, and Eggbert 2's `CheckWorld1` is accepted FOLDED into `Flush` (no new unique
bytes; every other status unchanged on both targets). No other edition-only declaration was found.

[EVIDENCE] Blupi 2's per-TU profiles (RTM, radius ±4096, all captured decisions held): no uniform shift
for decblock, decblupi or pixmap; event.cpp allows exactly 32 + 512k (fixes DrawButtons, not
ChangePhase). A piecewise profile exists only with deltas that change between neighbouring functions
(decblupi: +189 from IsBridge, 0 at IsSurfWater, +64 from IsDeepWater, −4 from SetMoveTraj), i.e. local
differences in the 2001 bodies, which no evidence identifies yet.

[EVIDENCE] Blupi 2's ChangePhase is 6,897 bytes (RET at 0x2173e; inventory corrected from the inherited
6,934). Its residue is constant live-range allocation from the first difference at +0x84: the original
keeps zero in EDI and WM_PHASE_PLAY in EBX, ours the reverse, with the downstream TEST/CMP and
PUSH 0/PUSH EBX changes. [NEGATIVE] Five alternative spellings of the Blupi-only retry/read paths
keep that first difference (`scratch/w-b2ctx/*/candidate.diff`). [OPEN] The source or context behind
the allocator's choice.

### Blocker repair (2026-10-23)

[EVIDENCE] `CEvent::CEvent` is STRICT at Blupi 2 RVA 0x1ca40 and Eggbert 2 RVA 0x1ca50:
352 verifier bytes each, including padding and eight verified relocations. A counted loop clearing
the 20 `m_menuDecor` elements lowers to the oracle's `lea edi; mov ecx,20; xor eax,eax; rep stosd`
setup. The inherited `ZeroMemory` call emitted count/zero/address order. Only the constructor's
emitted body changes in either TU; every other function keeps its status, location and size. The
candidate passed both complete gates and the identity audit before canonical integration and
`--accept`. This is a source-form repair, not declaration filler (KB `BLUPI-SB2-02`).

[EVIDENCE] A fresh Blupi 2 gate initially failed because the newly accepted Eggbert `CheckWorld1`
was translated as `Flush` at 0x28ca0. Its Blupi override now requires its own STRICT 128-byte body
at 0x1fbd0. `IsWorld`, `IsMoveTraj` and `IsVentillo` (880 bytes) are STRICT in Blupi 2 and are
reactivated there with explicit `withdrawn: null` overrides; Eggbert's withdrawals remain intact.
Five further proofs (592 bytes) are target-local `accepted_functions` in targets.json: `CJauge`'s
constructor, `ChatFlush`, `ChatPop`, `CreateSoundBuffer` and `MoveObjectSort`. The first three were
lost in address translation; the last two still mismatch on Eggbert. Target-local records use
Blupi RVAs directly and enter the ordinary relocation/size/status regression gate.

[EVIDENCE] `target_coverage.py` and `crossmap.py` now use the effective target manifest, including
translation, exclusions and local proofs. Previously they read Eggbert's base addresses and counts
for Blupi. Cross-map generation now uses the registry's game-code boundary, since derived oracles
need not have summary.json. Four focused regression tests in `tools/test_targets.py` cover the CD
body override, withdrawal isolation and enforcement, local RVA/proof enforcement, and projected
coverage. No comparator or equivalence verifier was changed.

[EVIDENCE] Final complete builds pass without an INCOMPLETE sound gate. Blupi's identity audit has
259 consistent callers and 747 consistent calls; Eggbert's has 251 and 989, with zero conflicts.
SB1's fresh gate passes both equivalence certificates and has zero function-status/location/size
differences from its stored report. Detailed receipts: `scratch/b2-blockers/REPORT.md`.

[EVIDENCE] Blupi's `ChangePhase` body and symbolic relocations are identical under a controlled SP3
compile and the target RTM compile. The existing SP3 allocation tracer therefore provides a useful
diagnostic control without claiming an RTM internal trace. Its unmodified replay reproduces the
body and passes 616/616 model checks: early zero (22 references) sorts at -65 before PLAY
(9 references) at -78, takes EBX, and PLAY later splits to 8 references/-63 and takes EDI. This is
the same 13-point priority boundary known in SB1 (`BLUPI-ALLOC-01`), not a declaration-key decision.
The detailed per-use/per-cost trace hit its bounded timeout; the lighter trace completed and reaped
its process. [OPEN] A natural source form that produces the original lifetime/allocation is still
unknown; `ChangePhase` remains MISMATCH. This round adds no BlupiStep matching claim.

### Trajectory repair (2026-10-23 continuation)

[EVIDENCE] `SetMoveTraj` is now STRICT in both editions: Blupi RVA 0x14d80, Eggbert RVA 0x14d90,
80 verifier bytes each, one unique raw-exact hit and no relocations. The integrated source uses a
cell POINT in `IsOutWater`, size/pointer clearing loops in both trajectory flushers, and converts
the by-value `pos` parameter directly to cell coordinates in `IsMoveTraj`. Every local is used;
there are no extra declarations, edition switches or compiler controls. The setter body itself
is unchanged. These source forms reproduce its required Y-index-first, load/OR/store sequence.

[EVIDENCE] Reusing `pos` resolves the earlier candidate's Blupi IsMoveTraj regression. Every other
full-build function retains its status, location and size in both editions, and all previously
STRICT/FOLDED decblupi bodies retain their raw bytes. Eggbert IsMoveTraj remains MISMATCH; this is
an 80-byte gain per edition, not the earlier candidate's 192-byte Eggbert gain. Both complete gates
and pre-acceptance identity audits pass (zero conflicts). Canonical `--accept` reactivates exactly
SetMoveTraj and retains its withdrawal history. Evidence: `scratch/b2-continue/REPORT.md`,
`receipt.json`; KB `BLUPI-SB2-03`.

[NEGATIVE] Two alternatives for IsMoveTraj (scalar coordinates and an early-return guard) retain
the new setter match but lose Blupi's query match. A cached filename pointer changes DrawButtons'
body and loses its Eggbert match; explicit GetBlupiInfo mask locals are inert. None is integrated.
[OPEN] The historical spellings are not uniquely recovered by these equal-byte results. The
remaining BlupiStep, ChangePhase, DrawButtons and smaller residues are still unresolved.

### Round 7: allocator and edition-context studies (`scratch/w-chgphase`, `w-stepalloc`, `w-eggctx`)

[EVIDENCE] **Blupi 2 ChangePhase** is exactly an allocator boundary. In a private c2 replay (SP3; our
body is identical under RTM), either of two diagnostic interventions reproduces all 2,012 original
instructions with zero non-relocated byte differences: excluding four late zero uses from constant
pooling (`index < 0` at line 2831, `HideMouse(FALSE)`, `!m_bBuildOfficialMissions`,
`table[m_index].bLocked`; the original tests them against an immediate 0), or moving the zero (22
reads) or WM_PHASE_PLAY (9 reads) priority by 14 points (they are 13 apart: SB1's BLUPI-ALLOC-01
boundary). All decisive references are shared source; the Blupi-only retry blocks own none. [OPEN] Which
source/IL difference gives the original's pooling or priority; no natural form found.

[EVIDENCE] **BlupiStep allocator residues**: (b) the 1/2 constant order is the boundary reload order by
fragment ID; (c) the 1/10 promotion needs the literal-1 component to fall from 49 to 46 references and
the literal-10 one from 11 to 9 (consumers identified: vertigo guards, first DecorDetect, Jeep and
Tank/Skate entries); (d) the key TEST differs only in encoded operand orientation. [OPEN] The source
form that splits those fragments while keeping the guarded member load; the w-step2 snapshot stays
rejected.

[EVIDENCE] **Eggbert 2 decblupi context**: the two originals' decblupi functions are identical except
BlupiStep's wind block (register choice/order only) and data addresses, and no CDecor method is
edition-only: one source, different declaration context. Diagnostic insertions after BlupiStep
(k = 178..218) make up to four withdrawn functions STRICT (+960 bytes) but lose up to 15 (−2,032);
isolated insertions fix IsMoveTraj (k = 6) or IsVentillo (k = 218) alone, which are not genuine.
With the current tree, DX7 `dsound.h` over DX5 still loses GetBlupiInfo and SetMoveTraj (−160) and gains
nothing on Eggbert 2.
[OPEN] The entities by which Eggbert 2's 2004 context differs before these functions.

### Round 8: c2 allocator rules (`tools/c2trace/{pool,split,cost}_trace.py`, KB BLUPI-ALLOC-18)

[EVIDENCE] VC5 SP3 c2's constant-pooling eligibility is a pure tuple-shape predicate (0x4156b6): a literal
call argument such as `HideMouse(FALSE)` always pools, so the four-late-zero diagnostic for Blupi 2's
ChangePhase is not a respelling recipe. Splitting is an endpoint union over dataflow masks with a
cut/kill bit; queue priority is loop weight × pressure-set cardinality × block benefit, minus carrying
charges (validated: 6,462 eligibility decisions, 234 partitions, 141,443 priority deltas, zero errors).

[EVIDENCE] Upstream (`scratch/w-cuts/REPORT.md`): in our Blupi 2 ChangePhase the zero range is
selected first, late pressure (7/7) cuts PLAY, and zero/22 keeps EBX. The original needs an early zero
component of 18 reads with no path to the four late zero reads. Eggbert 2's STRICT ChangePhase has
the same orientation as our Blupi 2 build (zero/23 in EBX; its intro blocks move the gap from 13 to 17),
so it is not a template for Blupi 2's. SB1's canonical reordered guards give Blupi 2's orientation but
not its instruction order. [OPEN] The genuine source or C1 input behind the original's cut history;
two output-identical internal histories exist and the bytes do not choose between them.

### Round 9: evidence checks (`scratch/w-edcmp`, `scratch/w-sb1event`)

[EVIDENCE] The two originals compared function by function across blupi, pixmap, decor, event,
decblock, decdesign and decmove (static, 287 pairs): every real code difference is already modelled by an
edition switch (phase table, intro phases/timer, credit line, quit/bye flow, cache/read retries,
`REQUIRE_CD`, installation path) or is RTM-vs-SP3 emission in functions STRICT on both targets. The only
one-sided function is Eggbert 2's `IntroStep` (guarded). [NEGATIVE] So no unmodelled edition code is
left to explain Blupi 2's declaration-context residues.

[NEGATIVE] SB1's EQUIVALENT `ChangePhase` and `TreatEventBase` do not become STRICT with Eggbert 2's STRICT
spellings: the SB2 forms give SB1 the opposite zero/PLAY allocation, and SB2's toggle form widens
TreatEventBase's register residue from 8 to 14 bytes (no losses elsewhere on the nine SB1 targets).

### Whole-image relink and context solvers (round 10)

[EVIDENCE] `tools/relink.py --target sb2/eggbert/en|sb2/blupi/en` links the verifier's objects with the
target's own linker (Eggbert 2: SP3 LINK 5.10; Blupi 2: RTM LINK 5.00, without `--historical-host`), CRT and
DX5 libraries, in oracle TU order with `/OPT:REF,ICF` (`scratch/w-relink/REPORT.md`). Both link with no
unresolved or duplicate symbols; the accepted-function gate passes; the import tables have the original
seven DLLs, order and symbol sets. Accepted functions at their original RVA: Eggbert 2 310/463, Blupi 2
339/455 (manifest TU order: 39; no ICF: 15/14). `.text` and `.rdata` keep their original RVAs and sizes;
`.data` is 9,792 bytes short in both (owner unknown, [OPEN]); `.rsrc` is absent (no SB2 resource script;
Eggbert 2's second Rich entry matches CVTRES 5.00.1668, [HYPOTHESIS]). SB1's default relink is unchanged
(`557ca92e…`).

[EVIDENCE] Resources (`scratch/w-rsrc/REPORT.md`): `tools/make_rc.py --target` generates
`games/sb2/res/sb2-{eggbert-en,blupi-en,blupi-de,blupi-fr}.rc` the way docs/RESOURCES.md built SB1's (cursors and
icon are SB1's Planet Blupi files, byte-identical to the SB2 payloads; strings, dialogs and version blocks
as script text from each original), and `tools/relink.py` links them. After normalising data-entry RVAs
(the section sits 8–12 KiB low behind the `.data` shortfall), Eggbert 2's `.rsrc` equals all 25,088 bytes;
Blupi 2 en/de/fr equal all bytes except 73 resource-directory timestamp fields each. SB1's `blupi.rc` and
relinked image are unchanged.

[EVIDENCE] Those timestamps come from VC5 RTM CVTRES's process clock (the `.res` file time has no effect;
RTM LINK keeps them). `tools/relink.py --historical-host` with the RTM linker now runs the pinned CVTRES and
LINK under `tools/histlink.py` at the original resource and PE timestamps (no output patching;
`scratch/w-rsrcts/REPORT.md`): the relinked `.rsrc` is raw byte-exact for Blupi 2 en/de/fr and for all six
Speedy Eggbert 1 languages. SB1's primary relink is unchanged.

[NEGATIVE] Context solvers over natural, byte-neutral respellings (`scratch/w-solve-decblupi`,
`w-solve-pixmap`): our compiles of decblupi.cpp and pixmap.cpp are byte-identical under SP3 and RTM and every
respelling shifts both compilers equally, but the two ORIGINALS differ in operand/register order for seven
decblupi predicates (IsWorld needs about +187 more entities on Eggbert 2 than Blupi 2; IsBridge, IsDoor,
IsTeleporte, IsOutWater about +90..+130 more on Blupi 2) and for BltFast/MouseBackDraw. Natural respellings
reach −5..+10, so no shared source satisfies both editions: the 2001 and 2004 sources differ by roughly
100–200 front-end entities before these functions (consistent with code the linker strips, which leaves no
trace in the binaries). Removing pixmap's nine bodiless inherited methods is inert.

### EQUIVALENT tier and data (round 11)

[EVIDENCE] A separate EQUIVALENT tier, as SB1's (docs/STANDALONE.md), certifies functions whose remaining
differences are operand or register order that natural source cannot fix for both editions
(`tools/equivalence_target.py`, `games/sb2/recon.json` `target_equivalent_functions`, certificates in
`evidence/equivalence/sb2-*`; `scratch/w-equiv/REPORT.md`). Blupi 2 en/de/fr: DrawButtons, GetBlupiInfo,
IsBridge, IsDoor, IsTeleporte, IsDeepWater, IsOutWater, BltFast(int,int,POINT,RECT,int), MouseBackDraw:
9 functions / 8,944 bytes each. Eggbert 2: IsWorld, IsDeepWater, IsMoveTraj, IsVentillo, MoveObjectSort:
5 functions / 1,200 bytes. They stay comparator MISMATCH and add zero STRICT bytes; every normal
`verify.py`/`relink.py` run re-proves the regions (symbolic proofs, liveness, branch alignment, relocations)
and checks the pinned certificates; a changed payload is rejected. SB1's checker and certificates are
untouched. [OPEN] ChangePhase (Blupi 2) and BlupiStep need loop/register invariants.

[EVIDENCE] `table_decor_quart` (decblock.cpp) has 7,056 entries in SB2 (441 icons × 16, all values read from
both executables; SB1 has 3,792). With it the relinked `.data` is 3,296 bytes short instead of 9,792
(`scratch/w-data/REPORT.md`); every function status is unchanged. [OPEN] The owner of the remaining bytes.

[NEGATIVE] The decblock/decmove solver (`scratch/w-solve-decblock`) found GetBlupiInfo and MoveObjectSort
need opposite contexts in the two editions, like the decblupi and pixmap predicates.

### ChangePhase and BlupiStep certified; atlas tables; Eggbert 1 link order (round 12)

[EVIDENCE] `scratch/w-equiv2/REPORT.md`: Blupi 2 en/de/fr ChangePhase (6,912 bytes) and BlupiStep on all four
SB2 targets (35,456 bytes) are certified EQUIVALENT. The BlupiStep proof pairs both control-flow graphs
(every branch arm and back edge, 10,457 / 10,435 boundaries), binds all 499 relocations bijectively, and
proves each differing window: constant 1/2/10 order, literal-1 promotion, Jeep entry, literal-10 stores,
commutative TEST orientation and, on Eggbert 2, the wind index expression (affine address arithmetic mod
2^32). ChangePhase's zero/PLAY register swap is proved across 30 windows with whole-CFG constant facts. Four
negative controls (an alternate XOR encoding, reversed TEST operands, PLAY or literal 10 changed by one) are
rejected. Assumptions as SB1's (ordinary stable RAM, declared ABI); local static equivalence, not STRICT.
EQUIVALENT tier now: Eggbert 2 6 functions / 36,656 bytes; Blupi 2 11 / 51,312 bytes per language.

[EVIDENCE] `scratch/w-data2/REPORT.md`: four truncated atlas tables in `pixtables.h` (complete 6-short
records decoded from both originals, 4,658 more bytes); all four compare byte for byte in both relinked
images; the relinked `.data` is now 1,376 (Eggbert 2) / 1,408 (Blupi 2) bytes LONGER than the original
([OPEN] which contribution); no missing BSS declaration found. Function statuses unchanged.

[EVIDENCE] `scratch/w-layout11/REPORT.md`: `tools/relink.py --order eggbert-jauge-before-menu` (linking
jauge.cpp before menu.cpp, so ICF keeps `CJauge::~CJauge` as the original does) puts 272/408 accepted Speedy
Eggbert 1 functions at their original RVA (oracle order: 235). [OPEN] Its `.text` is 16 bytes short (first at
`CEvent::GetPhase`) and every SB1 secondary `.data` 32 bytes long (zero fill).

Outside both tiers: Eggbert 2's CreateSoundBuffer (DX7 descriptor, line 64), SetDemo (owner ambiguous with
SetState), the GetHide/IsHost shared body.

### Data classification and certificate binding (round 13)

[EVIDENCE] Data ownership (`scratch/w-data4`, `w-data5`, supervisor reads of Eggbert2.exe `.data`): the 39
editor arrays that decblock.cpp defines under SB1-era names (`table_366b0`, `table_364a0`, ...) and that
decdesign.cpp duplicated as `sb2_design_*` lie inside decblock.cpp's data contribution, interleaved in source
order with its genuine tables (`table_design_wind` 0x437650, `table_366b0` 0x4376a8, `table_design_key`
0x4377b8): as in SB1, decblock.cpp defines them and the editor references them. decdesign.cpp's 39 copies
are removed and its references use decblock's tables (`scratch/w-data3`'s change; an earlier note here
called it wrong by reading the SB1-era names as addresses). Five decblock.cpp arrays absent from both
originals and unused are removed. With table_decor_quart, the atlas tables and these, both editions'
relinked `.data` has the original's exact size (virtual delta 0), and Blupi 2's `.idata` and `.rsrc` sit at
their original RVAs. Every function status is unchanged. [OPEN] 20 decdesign-only `sb2_design_*` arrays also
have addresses inside decblock's range (moving them would change decblock.cpp's declaration context), and the
`.data` contents still differ in places (Eggbert 2: 325,364 / 363,520 same-offset bytes equal).
[NEGATIVE] Removing decblock.cpp's copies instead (`scratch/w-data5`) changes GetBlupiInfo's context; a
subset chosen to keep it is not evidence.

[EVIDENCE] `tools/equivalence_target.py` certificates (`scratch/w-certfix/REPORT.md`) now bind the complete
payload, each relocation's site, symbol, type, addend and resolved original target, and the proof inputs, but
no longer the COFF placement of unrelated data in the object (which had made unrelated data fixes invalidate
BlupiStep's certificate). All 39 SB2 certificates were re-issued; six negative controls (payload mutation,
relocation retargeted, changed addend, ...) are rejected. SB1's checker and certificates are unchanged.

### Round 14: multi-change context solvers, `.data` ledger, function emission order (2026-10-24)

[NEGATIVE] `scratch/w-mc-egg/REPORT.md`: piecewise ID-shift solver for Eggbert 2's decblupi.cpp (fresh traces,
exact key model, shifts -2000..+3000, any number of change points). The cheapest profile that satisfies every
STRICT decision and fixes IsWorld, IsDeepWater and IsVentillo needs five compensating changes (+187 before
IsWorld and -187 before ActiveSwitch, +43 before IsDeepWater, -108 before IsOutWater, -109 before SetMoveTraj;
seven with BlupiStep's wind order). No authentic source explains it: attested `IsNormalJump` costs 288 bytes,
the C# port's indexed trajectory clears 192, DX7 `dsound.h` 272. MoveObjectSort's late operand-order decision
depends only on the header ID of `SortGetType`, so no body change of any size can fix it.

[NEGATIVE] `scratch/w-mc-b2/REPORT.md`: per-TU uniform solutions exist for Blupi 2 (decblock -3, pixmap +9..+30,
event +32, each periodic in 512/1024) but not for decblupi (four local changes) and not as one header shift.
`DIRECTDRAW_VERSION=0x0300` with `D3D_OVERLOADS` makes six functions STRICT (+8,352 bytes, DrawButtons
included) and costs MoveObjectSort 240 (re-provable EQUIVALENT); nothing in either executable attests those
settings, so it is a hash fit, not a recovered configuration, and is not adopted.

[EVIDENCE] `scratch/w-data6/REPORT.md`: every differing `.data` byte of both relinks is classified. All 168
inventoried tables have the original's contents (no value or length errors); the differences are placement:
numeric tables 14,697, atlas tables 10,245, event.cpp's table contribution 7,105, string literals 4,503,
text.cpp 1,251, CRT 661 (Eggbert 2; Blupi 2 alike). [OPEN] The placement rule for these contributions.

[EVIDENCE] `scratch/w-emitorder/REPORT.md`, msvc5kb `BLUPI-LAY-04`: VC5 C2 emits file-static and inline
functions only after a compiled reference needs them, so a `static` callback follows its caller whatever its
source position. All three originals place each DirectPlay callback before its caller, so their callbacks had
external linkage. games/sb2/src/network.h and network.cpp now declare `EnumProvidersCallback` and
`EnumSessionsCallback` without `static`: every function status of all four SB2 targets is unchanged
(`samestatus.py`: 0 differing; certificates pass), and in Eggbert 2's relink both callbacks and their callers sit
at their original RVAs (functions at original RVA 85 -> 87, same-offset `.text` +186 bytes). SB1 keeps its static
callbacks: there the same change moves an ICF survivor (GetHide/GetNbSessions) and the whole image gets worse
(docs/LAYOUT_ORDER.md).

### Round 15: `.data` contribution order (2026-10-24, `scratch/w-dataorder/REPORT.md`)

[EVIDENCE] Rules confirmed with 40 micro TUs on SP3 and RTM: ordinary initialized writable data follows definition
order (also after functions; file-static numeric tables are not deferred like BLUPI-LAY-04's functions);
explicit all-zero initializers go to `.bss`, while a nonzero array keeps its zero tail in `.data`; literal
COMDATs follow function emission and LINK pools identical literals at the earlier object's contribution.

[EVIDENCE] Both originals hold one 39,216-byte numeric band (Eggbert 2 `0x4364c8`, Blupi 2 `0x4354d0`) with
decblock.cpp's 99 tables, the 40 animation tables that dectables.cpp defined, the 20 decdesign-only editor arrays,
`table_adapt_fromage` and the vehicle speed table that decor.cpp kept as a function-local static, in an order
visible in both executables. decblock.cpp now defines all of them in that order (values and lengths unchanged,
no padding); their users declare them `extern`. The relinked band equals the original byte for byte in both
editions, and event.cpp's 290,820-byte table contribution (whose 14,023-byte zero tail is the "zero block" seen in
round 14) now sits at its original address. Same-offset `.data`: Eggbert 2 325,365 -> 346,879 / 363,520, Blupi 2
312,427 -> 345,641 / 349,184. All four SB2 targets keep every function status (0 differing) and every certificate.
[HYPOTHESIS] The original names of the speed array and of the band's TU are not recovered (the C# port's
Tables.cs and sb2-decomp's dectables.cpp give the semantic ancestry, not the filename).

[OPEN] The literal `"ab"` precedes decblock's first literal in both originals but is first used by
`CEvent::OutputNetDebug`; the later literal bands drift by 4-24 bytes from there. `.data` virtual size is 32 bytes
over the original in both editions.

### Round 16: SortGetType declaration identity (2026-10-24)

[EVIDENCE] The sb2-decomp/C# member order and original helper ABI support static
`CDecor::SortGetType` before `MoveObjectSort`. Its predicted ID 33269 has
F(ID).bit7=1, fixing the late header-only decision: Eggbert gains 240 STRICT bytes
(449 STRICT + 12 FOLDED, 136,432 bytes). Under the edition policy, Blupi en/de/fr
lose that one STRICT body (447 + 5 FOLDED, 121,360 bytes each), independently
re-proven EQUIVALENT; no other report changes beyond the helper rename. The shared
checker accepts both manglings under the unchanged 48-byte instruction theorem.
Official re-issuance changes only the checker hash in 22 SB1 and 40 SB2 existing
certificates, plus three new Blupi proofs. All four SB2 and ten SB1 gates exit 0;
reports are unchanged across the checker refresh (sandbox sound gate INCOMPLETE).
Evidence: `scratch/w-sb2rev/REPORT.md`, `scratch/w-sb2rev/integration/checker-rollover/`.

### Round 17: linker inputs, BlupiStep, CreateSoundBuffer (2026-10-24, `scratch/w-sb2link`, `w-sb2step`, `w-sb2small`)

[EVIDENCE] Linker: the LINK model replays every current SB2 ICF input and fold (837 SP3 / 831 RTM entries, 17 / 16
folds) and both import sorts. No SB1 function with a binary-proven body is missing from SB2's source: it already has
33 REF-dead definitions; only `CButton::Redraw`, `CButton::GetEnable` and `CPixmap::Fill` (Planet Blupi, SB2 header
declarations) are absent. Most of the placement gap is one ICF choice: the shared empty destructor survives in
button.cpp instead of jauge.cpp (+16 bytes through about 290 functions); pixmap's CacheAll/Benchmark slot and event's
BuildUp/Play* slot are source/emission order, which no dead population can fix. Fixed-order upper bound of the model:
405/463 (Eggbert 2), 401/455 (Blupi 2). [NEGATIVE] Restoring Redraw + GetEnable reaches 313 / 338 functions at their
original RVA (from 87 / 30) exactly as predicted, but changes `CButton::MouseMove` by seven bytes through its
declaration context in every edition (MouseMove matched only after round 3 removed them). [OPEN] What else differed in
the original button.cpp before MouseMove.

[EVIDENCE] BlupiStep: the key-mask TEST orientation is fixed in C2's post-reader input (memory AND versus a named
temporary under the door guard), before scheduling and allocation; a preregistered compiler-IR counterfactual changed
exactly the predicted byte (`+0x5b69`). The constant-group regions' first cost difference is localized; reversing the
first queue crossing is byte-inert. Three new natural source forms: no STRICT gain (pilot stopped).

[EVIDENCE] CreateSoundBuffer (Eggbert 2): DX7 `dsound.h` for sound.cpp alone, with one meaningful line before it
(`#define DIRECTSOUND_VERSION 0x0700`, or equally `#include <windows.h>`), reproduces the descriptor and line 64 with
every other payload unchanged; only `__FILE__` (historical path) stays unverified in the sandbox. [HYPOTHESIS] A 2004
project with a sound.cpp-specific DirectSound header; the import hints prove DX5 import libraries. Not integrated: the
line is not identified and a per-TU header environment needs evidence. After the IsHost attribution (round 16 notes),
the only uncovered bytes are this function (208, Eggbert 2) and the four DirectX import thunks (32 per edition).

### Round 18: per-edition matching (2026-10-24, `scratch/w-b2own`, `scratch/w-button`)

[EVIDENCE] Speedy Blupi 2 `MouseBackDraw` is STRICT: Blupi 2's own pixmap.cpp defines `CPixmap::Benchmark` before
`SetDebug`, where both originals emit it (definition order = emission order). Blupi 2: 452 STRICT + 5 FOLDED,
122,240 B per language; EQUIVALENT 9 functions / 50,800 B. Eggbert 2 unchanged (its shared pixmap.cpp keeps the old
order: restoring the full anchored order would cost QuickIcon). [NEGATIVE] SB1's binary-proven forms of GetBlupiInfo,
DecorNextAction clamps, IsDeepWater, IsOutWater, DrawButtons' number position and ChangePhase's music/WriteInfo
guards gain nothing on Blupi 2 (several are neutral, IsOutWater costs SetMoveTraj); declaration order is already
identical to SB1 for 178 CDecor, 98 CEvent and 45 CPixmap declarations.

[EVIDENCE] button.cpp conflict: each restored dead definition (`CButton::Redraw`, `GetEnable`) adds two front-end IDs;
four extra IDs flip a bit-3 operand-order decision in `MouseMove`. Neither alone gives the original jauge ICF
survivor. sb2-decomp's method order (MouseMove before MouseDown) keeps every STRICT payload with both methods and
reaches 312 (Eggbert 2) / 337 (Blupi 2) functions at their original RVA, but the originals place MouseDown before
MouseMove, so it contradicts the binary. [OPEN] The original needs about four fewer IDs before MouseMove alongside the
dead methods; no attested source shows where.

### Round 19: bounded BlupiStep pilot and Blupi 2 residues (2026-10-24, `scratch/w-step2`, `w-b2pred`, `w-b2event2`, `w-eggev`)

[EVIDENCE] BlupiStep key-mask TEST: a standalone positive control proves that a guarded named copy
(`(old = keys) && (old & mask)` after the door check) makes C2 keep a named key range and emit the original operand
order. Transferred as `(num3 = m_blupiCle) != 0` in BlupiStep it behaves exactly as predicted on SP3 and RTM (reader
+3 tuples, num3 in EAX, `85 f8` becomes the original `85 c7`, all 43 peer payloads unchanged), but keeps an extra
`cmp`/`je` (8 bytes) the original lacks. [OPEN] The original needs a guarded key value that survives into lowering
without an extra test and without living across OpenDoor; nested locals, comma assignments, references, pointers and
inline getters are all folded back into a memory AND. Separate mask initialization failed its control; no route to
the constant 1/2 split history. Pilot stopped by its bound.

[NEGATIVE] Blupi 2 predicates: fresh RTM traces confirm the needed profile (0 -> +189 before IsBridge -> -51 -> +64);
GetBlupiInfo needs both operand pairs reversed (nearest window +17..+20). No evidenced missing body or declaration
supplies them. DrawButtons: no independently evidenced earlier source/header difference puts it in a byte-preserving
window. ChangePhase: four candidates keep the zero/PLAY priorities (-65/-78); no source form crosses the 13-point
boundary. Eggbert 2 (read-only): sb2-decomp's earlier BlitzActif/SoundEnviron declarations (+5 entities) are
decompiler order, not independent evidence; nothing new for IsWorld, IsDeepWater, IsVentillo or the wind expression.

### DirectX header environment (round 5)

[EVIDENCE] Static reads of both executables (`scratch/w-sdkenv/REPORT.md`): both contain exactly the ten
DirectPlay GUIDs that DX5's `dplay.h` defines under `INITGUID`, in header order, and neither has
IID_IDirectPlay4/4A, which DX6, 6.1 and 7 define unconditionally: **both editions used DX5's `dplay.h`**.
`CreateSoundBuffer` passes a 0x24-byte DSBUFFERDESC in Eggbert 2 (DX7 `dsound.h`, `guid3DAlgorithm`) and
0x14 in Blupi 2 (DX5/6/6.1, or DX7 with `DIRECTSOUND_VERSION` < 0x700); Eggbert 2 logs line 64, Blupi 2
line 63 (ours: 63). DDSURFACEDESC, DPSESSIONDESC2, DPNAME and the joystick structures have the same size
in every SDK. Neither imports DirectInput.

[EVIDENCE] Front-end entities per header, relative to DX5, in a game-logic TU: `ddraw.h` +385 (DX6) /
+690 (DX7), `d3dtypes.h` +138 / +257, `dplay.h` +270 / +270, `dsound.h` 0 / +12; `decor.h` pulls
sound.h, pixmap.h and network.h, so every game-logic TU sees all of them. Full verifier runs with a scratch
include override (`scratch/sb2/sdkverify.py`, 26 runs in `scratch/w-sdkenv/suite.json`, gains/losses by
`scratch/sb2/gainloss.py`):
- No SDK, `DIRECTDRAW_VERSION`, `DIRECT3D_VERSION`, `DIRECTSOUND_VERSION` or `D3D_OVERLOADS` setting keeps
  every STRICT function of either target. DX5 stays the reproducing configuration of both.
- Blupi 2 with DX7 `ddraw.h` over DX5: +8,864 bytes (DrawButtons, BltFast, MouseBackDraw, IsBridge,
  IsDoor, IsTeleporte, IsDeepWater, IsOutWater), −2,336 (QuickIcon, MouseMove, CreateSoundBuffer, eight
  decblupi predicates). Whole DX7 is similar. [HYPOTHESIS] Blupi 2's remaining residues are a different
  entity count before these functions (2001 headers or bodies), which DX7's `ddraw.h` happens to
  approximate; the GUID table rules out a whole DX6/DX7 SDK with the current `network.cpp`.
- Eggbert 2 with DX7 `dsound.h` over DX5 (the only combination consistent with all of its data): −272
  bytes (GetBlupiInfo, SetBalleTraj, IsBalleTraj), +80 (SetMoveTraj); `CreateSoundBuffer` then differs
  only by its line (63 vs 64). [OPEN] Eggbert 2's extra line before line 64 of sound.cpp; adopting this
  environment needs genuine context for the three functions first.

## 9. Tools and limitations

- `verify.py`: `source_root`, whole-file `source_overlays`, own-manifest `--accept`, folded owners in the game's tree.
- `crossmap.py --audit [--candidates] [--results PATH]`: callers of each symbol in SB1 must call the SB2
  address; calls of STRICT functions must land on the same-named SB1 function, a documented rename
  (`source_correspondence`) or a shared body the run locates under that name.
- `target_coverage.py`: accepted unique bytes per group (`coverage_groups` in the effective target manifest).
- `fnprobe.py --target`; `buildpath.py` folder mode with a sandbox fallback.
- `relink.py` and `fnprobe.py` select the same effective edition tree as `verify.py`.
- Limitations: STRICT placement by unique bytes cannot name a renamed/split body without a compiled
  caller (the audit catches it); fold ownership flips with the set of compiled callers (cmpfunc is
  pinned); withdrawn entries stay forced at their RVA; `residues.py`/permuter retain SB1-specific defaults.

## 10. SB1 protection

[EVIDENCE] After each integration and at the end of round 2: the verifier output of all nine SB1
targets is byte-identical to the pre-SB2 baseline (equivalence certificates checked); the relinked
SB1 image is `557ca92e…`; nothing under `src/`, `port/`, `assets/` or `external/` changed.

## 11. Independent edition source (2026-10-24)

[EVIDENCE] Under the independent-target policy, Blupi 2 en/de/fr declare
`source_overlays: ["games/sb2/blupi"]` in targets.json. Eggbert 2 keeps
`games/sb2/src/`. An overlay contains whole files, with the last overlay taking precedence;
logical TU names, oracle identities and certificate declarations stay unchanged. Only four
files currently differ:

| Blupi overlay | Difference and evidence |
|---|---|
| `decor.h`, `decmove.cpp` | Free `SortGetType` declared after the class, exactly the pre-`9e41413` form. Its original 48-byte body/call ABI is unchanged; `MoveObjectSort` regains 240 STRICT bytes. The static member remains Eggbert's STRICT form. Source membership is not uniquely recoverable from that ABI; the static-member witnesses are sb2-decomp and the C# port. |
| `decblupi.cpp` | `IsMoveTraj` converts the used by-value `pos` directly, exactly the pre-`e1a2e09` form (`b2-continue`, `w-eggprimary` reports). It regains 112 STRICT bytes; Eggbert retains its used `POINT cel`. Both source spellings implement the binary-supported trajectory calculation. |
| `pixmap.cpp` | Nested palette-capacity guards in `Create`, the `w-solve-pixmap/candidate.diff` form. `Create` itself is byte-identical; Blupi's `BltFast` gains 240 STRICT bytes with no losses. This natural spelling is a reconstruction hypothesis, not an authenticated historical source line. |

[EVIDENCE] `tools/source_tree.py` copies the base and overlay files byte-for-byte to a fresh
private build tree. This is necessary: VC5 resolves quoted includes beside the including file
before searching `/I`, so an overlay include directory alone would leave shared TUs using the
wrong `decor.h`. Targets without overlays keep their original paths. `verify.py`, `relink.py`,
`fnprobe.py`, and c2trace capture helpers use this resolver. `crossmap.py` and
`equivalence_target.py` consume logical TU identities and objects, rather than resolving source
files; their checks need no change. SB1 and the SDL3 port retain their source trees.

[EVIDENCE] Accepted coverage, counted independently by `target_coverage.py`:

| Executable | Before STRICT / FOLDED / bytes | After STRICT / FOLDED / bytes | EQUIVALENT after |
|---|---:|---:|---:|
| Eggbert 2 en | 450 / 12 / 136,448 | 450 / 12 / 136,448 | 4 / 36,304 bytes |
| Blupi 2 en | 448 / 5 / 121,376 | 451 / 5 / 121,968 | 10 / 51,072 bytes |
| Blupi 2 de | 448 / 5 / 121,376 | 451 / 5 / 121,968 | 10 / 51,072 bytes |
| Blupi 2 fr | 448 / 5 / 121,376 | 451 / 5 / 121,968 | 10 / 51,072 bytes |

Bytes here include folded owners once. Blupi's three obsolete equivalence certificates per
language are removed. Only its three BlupiStep certificates require official re-issuance,
because `IsMoveTraj` is now a STRICT callee. The comparator and proof checker are unchanged.
The sandbox historical sound-path gate remains INCOMPLETE; accepted coverage retains the
existing 208-byte Blupi sound entry whose path-dependent relocation needs an outside-sandbox run.

[NEGATIVE] Complete pixmap order restoration gains Blupi's BltFast/MouseBackDraw (512 bytes)
but loses its QuickIcon (784 bytes); an edition split alone cannot accept that regression.
The nested guard gives the smaller proven gain without that loss. IsWorld/IsDeepWater/IsVentillo
withdrawals from replacing the inherited SB1 BlupiStep context cannot be restored by reinstating
the wrong game's body. DrawButtons, GetBlupiInfo and MouseBackDraw still need genuine context.
`DIRECTDRAW_VERSION=0x0300` plus `D3D_OVERLOADS` remains rejected evidence-free steering.

Detailed assessment, fresh per-function comparisons, certificate diffs and all gate receipts:
`scratch/w-sb2split/REPORT.md`. Earlier rounds above are historical observations and may show
superseded totals or policies.

## 12. Next targets (estimated bytes)

1. **BlupiStep**: 35,452 original body bytes / 14 diagnostic regions on Blupi 2; 35,449 / 20 on
   Eggbert 2. Remaining constant scheduling/promotion and key-mask operand orientation need authentic
   source evidence; Eggbert also has the different wind order. The measured windows are in section 8.
2. **Blupi 2's ChangePhase and DrawButtons**: 6,912 and 7,648 verifier bytes. ChangePhase needs the
   source of the now-observed constant allocation boundary; DrawButtons needs genuine declaration
   context. Neither is strict, despite the original-style constructor loop now matching beside them.
3. **Blupi 2's smaller residues**: GetBlupiInfo, IsBridge, IsDoor, IsTeleporte, IsOutWater, BltFast and
   MouseBackDraw; IsDeepWater remains withdrawn. Eggbert retains four withdrawals and
   its IsWorld/IsDeepWater/IsVentillo residues. MoveObjectSort and IsMoveTraj are now STRICT in both
   editions through their own source forms. Existing negative probes constrain new source hypotheses.
4. **Eggbert 2's sound environment**: DX5 plus DX7 dsound.h matches its data evidence, but needs genuine
   context for GetBlupiInfo/SetBalleTraj/IsBalleTraj and line 64. Blupi 2's sound helper is now accepted
   separately with its existing DX5 descriptor and line 63.
5. **Identity/ownership**: the 16-byte GetHide/IsHost shared body (Blupi 0x28150 / Eggbert 0x28100)
   remains outside acceptance. CEvent::SetDemo has no compiled caller and stays ambiguous. Build and
   CEvent's constructor are resolved and are no longer matching targets.
