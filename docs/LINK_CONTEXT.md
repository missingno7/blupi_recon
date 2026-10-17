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

| State | button | decblock (tables + DecorNextAction) | decmove | decblupi |
|---|---|---|---|---|
| canonical headers | 0 OK | 18-21, 26-29, 34-37 (12-15, 20-23 with tables) | 0 OK; with the 3 callers removed: >= 29 | 0-16, 81-96+ |
| enum removed | OK | 7-10, 15-18, 23-26, 31-34 | callers removed: 0-55 | 12-31 |
| enum + operators + 44 externs removed | 8-15 + 16k | 1-4, 9-12, 49-52, 57-60 | none in 0-64 | 0-53 |
| canonical def.h, 44 externs removed | OK | **0 OK** | as is: >= 29; callers removed: none in 0-64 | OK |

The last row is the nearest gate state: canonical def.h, decblock with its tables and DecorNextAction,
`table_adapt_decor` moved, the 44 externs removed. Only MockeryDetect regresses (DecorNextAction STRICT, +272 bytes).
decblock tolerates removing R externs with R mod 8 in 1..4; decmove tolerates R <= 15. A subset of 9-12 (or 1-4)
externs would satisfy both, but the 44 have identical evidence status, so choosing a subset would be count fitting.

## [OPEN] What would resolve it

- Evidence for SB1's real header contents (which tables dectables.h declared, which CDecor methods decor.h declared:
  35 declared methods have no body, mostly SB2-only), or for decmove-specific SB1 content before MockeryDetect.
- An authentic replacement of the `RECT(...)`/`POINT(...)` helper macros (local aggregates, as already done for
  DrawMap's Build call) is required for a Planet-Blupi-faithful def.h; its count effect is not yet measured.
