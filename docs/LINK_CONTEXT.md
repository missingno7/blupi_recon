# Link blockers and declaration context (2026-10-18)

The natural link (docs/RELINK.md) stops on three unresolved methods: `CDecor::DecorNextAction`, `BlupiRect` and
`BlupiDead`. All three are blocked by TU declaration-count context, not by code: every candidate body is already
byte-identical or has no SB1 counterpart. This note records the evidence and the measured context windows.
Candidates live in `scratch/linkfix/`; nothing here is integrated because no evidence-backed subset passes the gate.

## Evidence-backed source facts

- [EVIDENCE] **DecorNextAction** (oracle 0x5670, between DrawInfo and KeyChange) compiles byte-identical (0 fixed
  differences, 272-byte extent). With it present, accepted `GetBlupiInfo` (0x57b0) swaps both commutative OR operand
  pairs (4 bytes).
- [EVIDENCE] **The decor tables belong to decblock.cpp.** In the oracle `.data`, the table block (table_shield ...
  0x2e6e8, `table_decor_quart` 0x31aa0, a second table block, `table_adapt_decor` 0x34298) lies directly after
  ddutil.cpp's data and before decblock.cpp's own strings (0x343d8); data follows object (= TU) order. The
  reconstruction's `dectables.cpp`/`obstacle.cpp` data TUs and decor.cpp's `table_adapt_decor` are conveniences.
  Likewise the icon atlases (0x78dd8..0x7b260) are at the start of pixmap.cpp's data.
- [EVIDENCE] `table_decor_action` (133 ints) is at 0x2ea28 inside that block (the relink worker's values match).
- [EVIDENCE] StartSploutchGlu, DynamiteStart and MoveObjectFollow have **no oracle slot**: every decor-TU slot is
  filled except Build, DecorNextAction and CDecor::DrawMap. They are SB2-only; removing them removes the BlupiRect /
  BlupiDead references. Of the three, only removing DynamiteStart moves accepted `MockeryDetect`; with
  DynamiteStart gone, 29 or more front-end symbols before MockeryDetect restore it.
- [EVIDENCE] def.h's `namespace Object { enum Object { 442 enumerators } }` is an SB2-decompilation construct (Planet
  Blupi's initial def.h has no enum); only decor.cpp used it (7 references, replaced by 174/181/182/158/165 in
  candidates). The 14 POINT operators are not in Planet Blupi's initial def.h either (later SDL-era additions; unused).
  With both removed, our def.h equals Planet Blupi's initial def.h plus the SB2 `MAKE_RECT`/`MAKE_POINT` helpers
  behind the `RECT(...)`/`POINT(...)` macros (about 45 uses).
- [EVIDENCE] dectables.h declares 44 SB2 tables (plus `table_decor_action`) that are defined nowhere, used nowhere,
  and have no room in SB1's decblock data (the table range is covered except about 170 bytes of known
  ventilator/wind tables and table_decor_action).

## Measured windows (docs/COMPILER_STUDY.md cost model; probes are diagnostics, never source)

`tools/ctxsweep_tu.py` (one TU, probes at the top or before a function) and `tools/ctxsweep_all.py` (all TUs at
once, simulating a shared-header shift) put N `extern int` probes on the first line and list accepted functions that
stop matching. GetBlupiInfo flips in windows of 4 with period 8 (a gauge of the exact count), button's MouseMove in
windows of 8 with period 16, IsDeepWater and MockeryDetect have thresholds.

| State | button | decblock (tables + DecorNextAction unless stated) | decmove | decblupi | pixmap |
|---|---|---|---|---|---|
| canonical headers | 0 OK | 18-21, 26-29, 34-37 (12-15, 20-23 with tables) | 0 OK; with the 3 callers removed: >= 29 | 0-16, 81-96+ | not measured |
| enum removed | OK | 7-10, 15-18, 23-26, 31-34 | callers removed: 0-55 | 12-31 | not measured |
| enum + operators + 44 externs removed | 8-15 + 16k | 1-4, 9-12, 49-52, 57-60 | none in 0-64 | 0-53 | not measured |
| canonical def.h, 44 externs removed | OK | **0 OK** | as is: >= 29; callers removed: none in 0-64 | OK | not measured |
| authentic def.h + aggregates (sol-macros) | 4-11 + 16k | no tables/DNA: none in 0-64 | 2-64 | 0-21, 86-105 | 0-64 |
| same + tables + DecorNextAction | 4-11 + 16k | none in 0-64 | 2-64 | 0-21, 86-105 | 0-64 |
| same + 44 externs removed | 4-11 + 16k | no tables/DNA: none in 0-64 | 35-64 | 2-65, 130-149 | 0-64 |
| same + tables + DecorNextAction + 44 externs removed | 4-11 + 16k | **0-64** | 35-64 | 2-65, 130-149 | 0-64 |

The last canonical-def.h row is the nearest gate state: canonical def.h, decblock with its tables and DecorNextAction,
`table_adapt_decor` moved, the 44 externs removed. Only MockeryDetect regresses (DecorNextAction STRICT, +272 bytes).
decblock tolerates removing R externs with R mod 8 in 1..4; decmove tolerates R <= 15. A subset of 9-12 (or 1-4)
externs would satisfy both, but the 44 have identical evidence status, so choosing a subset would be count fitting.

## Authentic aggregate experiment (sol-macros, 2026-10-18)

- [EVIDENCE] Private trees against HEAD `a998026` are under `scratch/sol-macros/{tree,extern44,tables,tables44}`.
  The Object enum, 14 POINT operators, MAKE helpers and constructor macros are absent. All **48** TU uses are replaced:
  blupi 4, decblock 6, decdesign 1, decmove 2, decor 3, event 32. Existing locals use field assignments where their bytes
  permit it; other arguments use local aggregates. No padding declarations or unused locals are retained. The three
  SB2-only decmove callers remain present in every state.
- [EVIDENCE] An existing CButton::Create static symbol supplies a counter anchor without probes: `$S34693` canonical,
  `$S34250` enum removed, `$S34188` enum/operators removed, `$S34176` MAKE helpers also removed. Measured reductions:
  **443 + 62 + 12 = 517**, before TU-local aggregate replacements.
- [EVIDENCE] Full `verify.py --build-dir sol-macros` runs substitute **every** manifest TU, using private headers:

  | State | Every accepted regression at N=0 |
  |---|---|
  | enum/operators removed, helpers retained (control) | MouseMove, GetBlupiInfo |
  | authentic def.h + aggregates | MouseMove, GetBlupiInfo, MoveObjectSort |
  | + tables + DecorNextAction | MouseMove, GetBlupiInfo, MoveObjectSort |
  | + 44 externs removed | MouseMove, GetBlupiInfo, IsDeepWater, MockeryDetect, MoveObjectSort |
  | + tables + DecorNextAction + 44 externs removed | MouseMove, IsDeepWater, MockeryDetect, MoveObjectSort |

  DecorNextAction is STRICT, extent 272, in both table states. The sound build path is unavailable in the sandbox as
  expected; no sound byte regression was reported. No zero-regression authentic state was found.
- [EVIDENCE] New table rows give **joint non-relocation-byte windows**, not strict acceptance of probed sources.
  Bounds: 0-64 for button/decblock/decmove/pixmap; 0-112 for base decblupi, 0-160 with 44 externs removed. Button and
  pixmap inputs are identical across states and do not include dectables.h. Decmove/decblupi inputs are unchanged by
  table relocation, so corresponding rows share measurements. Decdesign also has no accepted byte differences in
  0-64. Before invoking `ctxsweep_tu.py`, private headers are copied to `build/tmp/rx/tusweep/`; its generated TU would
  otherwise fall back to canonical headers.
- [EVIDENCE] Individual windows: MoveObjectSort 2-64 in both header states; MockeryDetect 0-64 with externs retained,
  35-64 with them removed. IsDeepWater 0-21 and 86-112 (base), 2-65 and 130-160 (44 removed). **New peer: IsVentillo**
  stays equal through 105 (base) or 149 (44 removed), then differs through the tested end. QuickIcon is equal in 0-64.
  GetBlupiInfo has 2 differing non-relocation bytes throughout 0-64 in the first three states and zero throughout
  0-64 in the tables+44 state; the prior periodic windows must not be assumed here.
- [EVIDENCE] Compared with the helper-retaining control, the six changed TUs retain all function non-relocation bytes
  except context-sensitive GetBlupiInfo/MoveObjectSort and already-unmatched DrawMap. Build's separate zero-POINT
  aggregates preserve its body; existing-pos reuse changes 8 bytes (rejected). DrawMap's two RECT locals enlarge its
  frame 0x28 -> 0x38 (159 non-relocation differences against the oracle). Same-block, field-assignment and clip-reuse
  forms are inert. All four aggregates before Build restore frame 0x28 but still differ by 146 bytes. No DrawMap form
  is claimed matching.
- [OPEN] MouseMove has no constructor-macro use and regresses in every state. Aggregate choices in other TUs cannot
  repair it; further authentic header evidence is required. No patch is proposed. Scripts, full logs, results JSON,
  source variants and the exact 44-extern list are under `scratch/sol-macros/`.

## [OPEN] What would resolve it

- Evidence for SB1's real header contents (which tables dectables.h declared, which CDecor methods decor.h declared:
  35 declared methods have no body, mostly SB2-only), or for decmove-specific SB1 content before MockeryDetect.
- An authentic, byte-preserving replacement of DrawMap's remaining two RECT arguments is still open; the shared
  helper-removal count effect and accepted-function regressions are now measured above.

## New evidence: DirectX 5 SDK headers (2026-10-18)

[EVIDENCE] The oracle's `.rdata` GUIDs show the original compiled against DirectX 5 SDK headers with `INITGUID` in
network.cpp (docs/RELINK.md, "DirectX SDK version"). Our manifest uses DX6 headers, which declare more (DirectPlay4,
DirectDraw4/7-era, DirectSound additions) in every TU that includes ddraw.h, dsound.h or dplay.h. All windows above
were measured with DX6 headers; they must be re-measured with DX5 headers before any context conclusion is final.

## DirectX 5.0 SDK installed and verified (2026-10-18)

[EVIDENCE] `C:	ools\dx5sdk\{INCLUDE,LIB}` holds `sdk\inc` and `sdk\lib` from archive.org item `ms-dx5-sdk`
("Microsoft DirectX Foundation 5.0 SDK", 1997), file `DirectX5SDK.iso`, 277,858,304 bytes, MD5
`40a5aceb9683e56e6e6451d39daa3686` (verified); extracted with 7-Zip, nothing executed. Its dplay.h defines exactly ten
GUIDs whose 160 bytes equal the oracle's `.rdata` at 0x2c220 (IID_IDirectPlay2 .. IID_IDirectPlay). The original SDK is
therefore identified.

[EVIDENCE] Switching only the manifest include directory from dx6sdk to dx5sdk (no source change) keeps every TU
compiling but regresses nine accepted functions, all context-type: CButton::MouseMove; decblupi's IsWorld,
GetTypeBarre, IsLave, IsRessort, IsOutWater, IsVentillo (x/y evaluation order, the IsDeepWater family); decnet's
ModifDecor; decmove's MoveObjectSort. DX5 plus the evidence-backed set above: eleven (adds GetBlupiInfo, IsDeepWater).

[EVIDENCE] Windows on DX5 + evidence set (tools/ctxsweep_tu.py, fixed version): button 1-8 + 16k; decblock 10-13,
18-21, 26-29, 34-37; decmove >= 15; decnet >= ~180 (none below 174); decblupi none in 0-216. button, decblock and
decmove share 18-21 and 34-37; decblupi and decnet need other, possibly negative, shifts.

[OPEN] The canonical manifest stays on DX6 headers until a source/header state passes the gate under DX5; the DX5
header set is the correct baseline for any future context reconstruction.

[EVIDENCE] Tool fix: earlier ctxsweep runs after the sol-macros worker were contaminated by private headers left in
the probe folder (quoted includes resolve next to the probe file first). Both ctxsweep tools now empty their folder,
delete the stale object before each compile and report failed compiles; measurements before the worker are unaffected.
