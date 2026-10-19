# Status: Speedy Blupi 1 matching reconstruction (2026-10-19)

[EVIDENCE] Complete standalone source build validated without assets, oracle maps or the original EXE.
`CEvent::ChangePhase` and `CEvent::TreatEventBase` are now accepted as **EQUIVALENT** under pinned static
equivalence certificates, independently of STRICT. All strict peers still pass. See [STANDALONE.md](STANDALONE.md).
[OPEN] Remaining strict bytes and whole-image layout are deferred pending new evidence; they do not block
continuing from this source baseline. A bounded cnc-ddraw smoke test reaches the first playable level and exits
cleanly ([RUNTIME.md](RUNTIME.md)); this does not establish full-game runtime equivalence.

[EVIDENCE] Milestone accepted by the user after manual play (2026-10-19): the game
looks good. Remaining strict-match and linker-layout research is parked; the
complete source-build baseline, two EQUIVALENT certificates and runtime harness
are ready for subsequent work. Color depth is an explicit runtime option (8/16),
with the harness default corrected to 16 to match the supplied original settings.

Strict acceptance is unchanged: only STRICT counts, and every canonical change passed `tools/verify.py` with zero
accepted regressions. CDecor::Build became STRICT on 2026-10-19 (docs/RESIDUAL_CAUSES.md, "Build STRICT").

## Where it stands

| Area | State |
|---|---|
| Game code | 127,456 / 141,488 bytes STRICT (90.1 %). Every game function is STRICT or FOLDED except two. |
| Behavioral exceptions | 2 EQUIVALENT functions; 16 differing aligned instructions remain deferred strict work (3,907 / 3,923) |
| `.rsrc`, `.data`, file size, `.text` size, timestamp | `.rsrc` exact; `.data` 1 differing byte in a CRT pointer; file and `.text` sizes equal the oracle; timestamp equal under the historical host (fresh check: docs/BLOCKER_CHECKS.md) |
| Whole image (canonical historical relink) | 121,401 differing bytes, almost all a +16 shift from LINK keeping a different copy of a folded empty function (ICF survivor), plus the CRT filter survivor, import order and the open functions |

## The open functions

| Function | Remaining | Cause (first divergent decision) | Proven by diagnostic | Source cause |
|---|---|---|---|---|
| CDecor::Build (0x4350) | STRICT (0 bytes, 1237/1237, 61/61 relocations) | A/B homes follow the animated-boundary reload order (0x40fbfa mode 1, ascending fragment ID); the IDs are popped by the first group split (0x437e67) in ascending member LR order from a batched LIFO free list | Forced extra drop at 0x41a6f8, or the reload swap: 0 bytes | [EVIDENCE] Solved: the big-decor Y start written as `tinyPoint.y = 0 - posDecor.y % DIMOBJY; tinyPoint.y += 2 - DIMOBJY;` interns A's NEG(y%64) identity before the quotient B, so A's web is walked first and B gets the lower fragment ID. |
| CEvent::TreatEventBase (0x1d230) | 8 bytes, 1886/1894, register-only | One missing temp-rotor step at the rank-0 cheat toggle: early C2 backward reconstruction (0x40cbc8/0x40cd08) creates a short old-flag copy and 0x41ddfe folds it into the compare | Extra rotor assignment, or old-flag split: 0 bytes | [OPEN] Needs a copy that survives 0x41ddfe with the same MOV/XOR/TEST/SETcc. No STRICT function shows one (409 searched). Eliminated: toggle spellings, terminator forms, named/volatile/reference old values, clones, index reuse, else-if ranks, shared feedback. |
| CEvent::ChangePhase (0x19310) | 32 bytes (8 instructions) | With the oracle's test order the pooled zero takes EBX before PLAY; zero's pool identity is fixed at 0x414ba6, before web partition; earlier groups keep 3/2/1 free colours | Zero-only cut of the three late zero reads: 0 bytes | [OPEN] Needs a separate late-zero component with PLAY connected through WriteInfo. Eliminated: condition-order families (canonical keeps a reordered guard as the closest form), flags, memset, mission arguments, renamed/copied results, counter epochs, shared two-consumer values, sign-mask test, shared failure tail. |

## What closed this phase (all canonical, zero regressions)

- Build STRICT: the big-decor Y start is two statements, the plain grid reset `0 - posDecor.y % DIMOBJY` and then
  `+= 2 - DIMOBJY` (expression-identity order 0x42127d -> web LR order 0x416a40 -> fragment pop order 0x437e67 ->
  boundary reload order 0x40fbfa -> stack homes 0x4367f0). Found by measuring the free-list counts directly
  (`tools/c2trace/freelist.py`) after spelling and context sweeps proved the counts robust.
- DrawButtons STRICT: loop-invariant title `pos.x = 330` with a separate number-label `POINT` (allocator cloning at
  0x451ac3), and a named `int cutLength` (operand key order at 0x408d58).
- Build 2,328 -> 8 bytes: `long screenY` with two real consumers (early forwarding 0x40b7b9; reproduces two of the three
  diagnostic edits), and the reverse-moving loop written with the cursor in the for step and three `continue` guards
  (changes the latch predecessor order and the allocator's region choice; the three-guard spelling keeps GetBlupiInfo's
  symbol-ID window).
- Method that worked: trace the actual compiler decision, find the earliest source-controllable input, confirm it in a
  small file, then transfer. The wins came from real value relationships (a value with two consumers, a loop-invariant
  field with a separate local, a named intermediate, loop control structure), never from spelling searches.

## Linker side

- Import order (15/128), Rich count (268 vs 270) and the two ICF survivors depend on LINK's input population/order,
  which the image does not identify. The historical MSVCRT qsort, comparator keys and extraction order are modelled
  exactly (docs/LINK_ICF.md, docs/HISTLINK.md).
- Experiment kept as evidence only (docs/experiments/FIFO_LIBORDER.md): Planet Blupi fifo.cpp as a REF-dead input plus
  library order `kernel32 user32 gdi32 winmm dsound LIBC OLDNAMES ddraw dplayx` reproduces both ICF survivors and cuts the
  whole-image difference to 40,288 bytes, but leaves the CRT -0x10 offset, Rich 269 and the import order unexplained.
- LINK 5.10.7287 (Blood II's linker) is possible for Blupi: an EXE without exports records no linker build.

## Best next discriminators (only with new evidence)

1. TreatEventBase: a rank-0 block that is rotor-neutral (result in ECX, rotor left at EDX; a one-slot shift before
   temp assignment 21 gives 0 bytes). Routes: an old-flag copy that survives 0x41ddfe/0x409c5c, a terminator byte
   that reaches temp assignment, or a preference for the select result (rank 1 gets one because its new value is
   forwarded to a push; rank 0's forwarded forms coalesce with bEnable instead).
2. ChangePhase: a source property that separates the three late zero reads into their own component before 0x414ba6,
   or raises PLAY/9 by 14 at the oldPhase dispatch. The recovered cost and region model leaves no code-visible input
   for either, so it is an IL property the machine code does not show (docs/RESIDUAL_CAUSES.md, model closure tests).
3. Link: original project data (a .dsp, map or build log) or LINK 5.10.7287 itself.

Closed in round 4 (docs/RESIDUAL_CAUSES.md, "Final targeted experiments"): symbol-ID operand order in
ChangePhase blocks 3..99 (480 comparisons, none decided by a hash bit) and inline-helper preference for the
TreatEventBase rank-0 toggle (five helper forms reduce to the direct IL or coalesce with bEnable).

Details and provenance: docs/RESIDUAL_CAUSES.md, docs/COMPILER_STUDY.md, docs/SYMBOL_ORDER.md, docs/LINK_ICF.md,
msvc5kb/catalog.json.

## Fresh blocker checks (2026-10-19)

[EVIDENCE] Full gate PASS, including the historical sound path: 127,456 bytes, zero accepted regressions.
Seventeen declaration controls produced no improvement. A compiler-host check observed 1,151 C1XX qsort calls;
using historical qsort, then all 66 CL / 101 C1XX historical CRT bindings, preserved every event.cpp function
payload, relocation and symbol order. Scope and reproducible scratch commands: [BLOCKER_CHECKS.md](BLOCKER_CHECKS.md).
The fresh canonical historical relink still differs in 121,401 bytes. No source or acceptance change was made.

## Independent Planet Blupi controls (2026-10-19)

[EVIDENCE] [PLANET_ORACLE.md](PLANET_ORACLE.md) authenticates official 1.7/1.8 binaries and the 2017 source import.
Two unchanged 1.8 functions distinguish RTM C2 from SP3; the same Display test favors SP3 for SB1. A real member
toggle keeps rotor EDX unchanged through an EAX preference, and a 688-byte STRICT Planet function supplies a
pooled-constant control. Binary-evidenced version corrections give complete masked hits for both Planet event
functions, with remaining relocation/data limits. Outcome C for SB1's two blockers; no canonical or acceptance change.
