# German build comparison

Static comparison of the English oracle (`assets/Speedy Blupi/BLUPI.EXE`) with the ISO copy at `build/oracle/iso/DEUTSCH/BLUPI.EXE`. No executable was run.

## Findings

**[EVIDENCE] One mapped function accounts for the German `.text` differences.** Both `.text` sections start at RVA `0x1000` and have raw size `0x2b000`; 4,540 aligned byte positions differ. The function map assigns 4,537 of them to `FUN_00416f70` (`0x16f70`, 7,419 bytes, end `0x18c6b`), in `event.cpp` with `bracketed` confidence and status `none`. Its phase-driven drawing body is consistent with `CEvent::DrawButtons` at [event.cpp:1081](../src/event.cpp#L1081), but that method identity is not a strict source match. The final three changed bytes are the German function's longer epilogue: its `ret` is at `0x18c6d` instead of `0x18c6a`; the next mapped entry remains at `0x18c70`. Every other mapped function body is byte-identical at its English RVA.

**[EVIDENCE] This is code divergence, not a global address shift.** `.rdata` and `.data` are byte-identical, import sets match, and functions after the changed body retain their addresses. The German code changes branch layout, register use, and immediates inside the body: for example, the branch at `0x179ae` targets `0x17a8c` rather than `0x17a89`; at `0x17a02`, the instruction/register sequence and immediate differ (`0x8c` vs `0x78`). A call in this region still targets `0x41f1f0`. The following function and other TUs do not move. Three changed epilogue bytes occupy padding after the English body range.

**[EVIDENCE] The resource section contains the translations.** English, German, and French each have 329 nonempty `RT_STRING` IDs. German changes 309 IDs, including ID 100, `training %d, time %d` becoming `Training %d, Zeit %d`; French changes 313 IDs. German `.rsrc` raw size grows from `0x5c00` to `0x6600` (2,560 bytes), exactly the executable file-size increase. French has the same 538,112-byte file size but byte-identical `.text`.

**[EVIDENCE] Link metadata is consistent with closely related builds.** English and German have the same image base, entry RVA, linker version 5.10, and Rich entries `(0, 270)` and `(0x60684, 1)`. Their COFF timestamps are 1998-10-14 07:17:39 UTC and 07:19:17 UTC. This supports a nearby rebuild using the same toolchain family; it does not prove identical compiler flags or source revision.

## Interpretation and open questions

**[HYPOTHESIS]** A German-specific source/build variant in the event drawing routine is more likely than a broad later revision: translated resources differ, while code changes are isolated to one UI function and the rest of `.text`, `.data`, and `.rdata` remain stable. The changed immediates and control flow could reflect localized layout or text handling. The binary evidence cannot distinguish an original language `#if` from a German-only edit or a small revision to that method.

**[EVIDENCE]** Current `src/event.cpp` has product-feature guards (`_VALUE`, `_DEMO`, `_EGAMES`) in `DrawButtons`, but no German/French/English language guard. The mapped function is not accepted STRICT in `recon.json`; the current source therefore cannot demonstrate an `#if GERMAN` that reproduces both bodies. `kb.py hints DrawButtons --rva 0x16f70 --exe "assets\Speedy Blupi\BLUPI.EXE"` only returned keyword matches, so there is no function-specific compiler rule to transfer from that query.

**[EVIDENCE]** The German image gives no evidence about the `decblock.cpp` residual: code at `0x57b0` is unchanged. It also shows no TU-order or data-layout movement. The useful event clue is limited to the alternate body at `0x16f70`; the other event functions remain unchanged.

## Artifacts

- `build/german/text_diff_summary.json`: PE sections, hashes, byte counts, resource counts.
- `build/german/text_diff_functions.csv`, `text_diff_runs.csv`: function attribution and changed RVA runs.
- `build/german/text_diff_neighborhood.txt`: focused English/German disassembly around the first divergence and epilogue.
- `build/german/resource_string_diff.csv`: changed string IDs and texts.
- `scratch/german/analyze_german.py`: reproducible static comparison script.


## Language-dependent region in CEvent::DrawButtons (follow-up)

**[EVIDENCE]** Aligning the English and German DrawButtons bodies isolates the difference to the `WM_PHASE_HELP` text
layout: the branch displacement first differs at RVA `0x179b0` (`+0xa40`), the blocks diverge through EN `0x17a87` /
DE `0x17a8a`, and the remaining 1,412-instruction suffix aligns at a +3 RVA shift. English draws from y=140 with the
second string at x=230 on the same row, advancing 20 per row; German starts at y=120, draws the second string at x=160
and y+13, advancing 18.

**[HYPOTHESIS]** The original source selected this layout with a compile-time language conditional; a scratch
conditional reproduces both variants' fixed bytes, but the original guard name is unknown, so canonical `src/` keeps
the English layout without a guard. The open DrawButtons residue (EN `+0x1316` / DE `+0x1319`) lies in the shared
suffix and is not language-dependent.
