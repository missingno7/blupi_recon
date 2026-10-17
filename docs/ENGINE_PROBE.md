# SB engine probes

## Method

[EVIDENCE] Scratch candidates are `decblock.cpp`, `decdesign.cpp`, `decmove.cpp`, and `decblupi.cpp`, with private headers under `scratch/tu/engine_include/`. The SB2-derived source copies remain in original function order. `decnet.cpp` was also compiled as a diagnostic owner probe. No canonical source, manifest, oracle map, assets, or external reference files were changed.

[EVIDENCE] VC5 SP3 11.00.7022 compiled with the manifest defines and flags: `WIN32`, `NDEBUG`, `_WINDOWS`, `nullptr=0`, `/W3 /GX /O2 /ML /Gd /Gy`; `TMP` and `TEMP` point at `build/tmp`. All five objects were compared together. The automatic result is `build/engine/results/engine_automatic.json`; forced comparisons are in `build/engine/results/engine_final.json`.

## Strict body matches

[EVIDENCE] Automatic comparison found 16 strict functions in the owned blocks: two in `decdesign`, eleven in `decmove`, and three in `decblupi`. All direct relocations are proven. The 16 unique target bodies cover 2,340 oracle bytes. Candidate COFF code sizes below may include alignment bytes beyond the Ghidra body size.

[EVIDENCE] `TestPushCaisse` direct calls resolve to `SearchLinkCaisse=0x00413020` and `TestPushOneCaisse=0x00412f30`; `TestPath` calls resolve to `DecorDetect=0x0040f640`. `ObjectDelete`'s forced calls resolve to `MoveObjectSearch=0x00413cb0` and `NetMessagePush=0x004113c0`.

| Candidate method | RVA | Oracle body bytes | Candidate COFF bytes |
|---|---:|---:|---:|
| `CDecor::GetMissionTitle` | `0x8520` | 7 | 16 |
| `CDecor::SetMissionTitle` | `0x8530` | 46 | 48 |
| `CDecor::AscenseurSynchro` | `0x12dd0` | 54 | 64 |
| `CDecor::UpdateCaisse` | `0x12e10` | 60 | 64 |
| `CDecor::TestPushCaisse` | `0x12e50` | 215 | 224 |
| `CDecor::TestPushOneCaisse` | `0x12f30` | 229 | 240 |
| `CDecor::AddLinkCaisse` | `0x131f0` | 73 | 80 |
| `CDecor::CaisseInFront` | `0x13240` | 119 | 128 |
| `CDecor::CaisseGetMove` | `0x132c0` | 89 | 96 |
| `CDecor::MoveObjectDelete` | `0x13a30` | 115 | 128 |
| `CDecor::MoveObjectFree` | `0x13ab0` | 63 | 64 |
| `SortGetType` | `0x13af0` | 34 | 48 |
| `CDecor::MoveObjectSearch` | `0x13cb0` | 153 | 160 |
| `CDecor::GetPosDecor` | `0xf540` | 108 | 112 |
| `CDecor::BlupiAddFifo` | `0xf5b0` | 142 | 144 |
| `CDecor::TestPath` | `0xf8c0` | 833 | 848 |

[EVIDENCE] The diagnostic `decnet` object has four strict anchors: `NetStopCloud` at `0x10670` (50 oracle bytes), `ObjectDelete` at `0x108a0` (112), `NetPlaySound` at `0x10980` (59), and `NetStopSound` at `0x109c0` (50). These anchors are for boundary analysis, not part of the owned-block count. `ObjectDelete` accounts for part of the gap between `NetStopCloud` and `NetPlaySound`; unmatched space remains on both sides.

[EVIDENCE] Three forced comparisons are also byte-strict with every relocation proven: `SetCelHiliFromScreenPos` at `0x5ab0` (46 oracle bytes; 48 COFF bytes), `DeleteCel` at `0x5b40` (90; 96), and the `decmove.obj` copy of `ObjectDelete` at `0x108a0` (112; 112). The two design bodies are new unique matches, bringing the owned-block total to 18 functions and 2,476 target bytes. The `ObjectDelete` bytes are already matched automatically by the diagnostic `decnet.obj`; count that target body once. The four diagnostic `decnet` anchors bring the combined total to 22 unique target bodies and 2,747 bytes.

[EVIDENCE] The completed `decblupi` tail candidate was compared with all engine objects in `build/engine/results/engine_forced_complete.json`. Seven forced `decblupi` bodies are STRICT with all relocations proven: the three earlier anchors (`GetPosDecor`, `BlupiAddFifo`, `TestPath`) plus four tail methods (`IsLave`, `IsRessort`, `IsPassIcon`, `IsBlocIcon`). This raises the unique owned total to 22 bodies and 2,776 target bytes; adding the four unique diagnostic `decnet` anchors gives 26 unique bodies and 3,047 bytes. The automatic pass sees these seven as exact-byte but relocation-unverified hits; do not describe them as automatic strict placements.

## Decmove per-function table

[EVIDENCE] The table lists every target entry currently assigned to `decmove.cpp` in `oracle/tu_map.csv`. “No unique hit” means the automatic pass did not locate a strict body for that oracle entry; it is not a forced comparison at that RVA. Eleven automatic strict anchors concentrate in the crate/lift and move-object helpers near the end of the block.

| Oracle RVA | Oracle body bytes | Candidate/result |
|---:|---:|---|
| `0x10a00` | 86 | No unique hit |
| `0x10a60` | 45 | No unique hit |
| `0x10a90` | 1,545 | No unique hit |
| `0x110a0` | 187 | No unique hit |
| `0x11160` | 72 | No unique hit |
| `0x111b0` | 259 | No unique hit |
| `0x112c0` | 217 | No unique hit |
| `0x113a0` | 21 | No unique hit |
| `0x113c0` | 96 | No unique hit |
| `0x11420` | 97 | No unique hit |
| `0x11490` | 31 | No unique hit |
| `0x114b0` | 86 | No unique hit |
| `0x11510` | 130 | No unique hit |
| `0x115a0` | 23 | No unique hit |
| `0x115c0` | 591 | No unique hit |
| `0x11810` | 946 | No unique hit |
| `0x11bd0` | 4,165 | No unique hit |
| `0x12c20` | 324 | No unique hit |
| `0x12d70` | 96 | No unique hit |
| `0x12dd0` | 54 | `AscenseurSynchro` — STRICT |
| `0x12e10` | 60 | `UpdateCaisse` — STRICT |
| `0x12e50` | 215 | `TestPushCaisse` — STRICT |
| `0x12f30` | 229 | `TestPushOneCaisse` — STRICT |
| `0x13020` | 453 | `SearchLinkCaisse` — forced MISMATCH (464 candidate bytes) |
| `0x131f0` | 73 | `AddLinkCaisse` — STRICT |
| `0x13240` | 119 | `CaisseInFront` — STRICT |
| `0x132c0` | 89 | `CaisseGetMove` — STRICT |
| `0x13320` | 532 | No unique hit |
| `0x13540` | 151 | No unique hit |
| `0x135e0` | 538 | `MoveObjectDetect` — forced MISMATCH (608 candidate bytes) |
| `0x13800` | 159 | No unique hit |
| `0x138a0` | 190 | No unique hit |
| `0x13960` | 208 | No unique hit |
| `0x13a30` | 115 | `MoveObjectDelete` — STRICT |
| `0x13ab0` | 63 | `MoveObjectFree` — STRICT |
| `0x13af0` | 34 | `SortGetType` — STRICT |
| `0x13b20` | 234 | `MoveObjectSort` — forced MISMATCH (240 candidate bytes) |
| `0x13c10` | 160 | `MoveObjectPriority` — forced MISMATCH |
| `0x13cb0` | 153 | `MoveObjectSearch` — STRICT |

[HYPOTHESIS] The broad `decmove` interval `0x10a00`–`0x13cb0` remains plausible from the function map and later strict anchors, but the start has no strict anchor. `ObjectDelete` at `0x108a0` is handled separately below because it falls in the mapped `decnet` interval.

## Decblupi probes

[EVIDENCE] The full candidate preserves the current SB2 `decblupi.cpp` function order, then appends the table and movement helpers extracted from the scratch `decblock.cpp`/`decmove.cpp` copies in target order. A combined forced comparison confirms seven strict bodies; locations and sizes are listed below. `TestPath`'s four calls resolve to `DecorDetect=0x0040f640`; both table helper relocations resolve to `table_decor_quart=0x00431aa0`.

| Oracle RVA | Oracle body bytes | Candidate/result |
|---:|---:|---|
| `0x9280` | 25,269 | `BlupiStep` — forced MISMATCH (31,264 candidate bytes) |
| `0xf540` | 108 | `GetPosDecor` — STRICT (112 candidate bytes) |
| `0xf5b0` | 142 | `BlupiAddFifo` — STRICT (144 candidate bytes) |
| `0xf640` | 640 | `DecorDetect` — forced MISMATCH (816 candidate bytes) |
| `0xf8c0` | 833 | `TestPath` — STRICT (848 candidate bytes; four call relocations proven) |
| `0xfc10` | 439 | No current source candidate; target behavior not mapped to a method |
| `0xfdd0` | 75 | No current source candidate; target behavior not mapped to a method |
| `0xfe20` | 94 | No current source candidate; target behavior not mapped to a method |
| `0xfe80` | 261 | No current source candidate; target behavior not mapped to a method |
| `0xff90` | 184 | `IsWorld` — forced MISMATCH (272 candidate bytes) |
| `0x10050` | 248 | `GetTypeBarre` — forced MISMATCH (256 candidate bytes) |
| `0x10150` | 88 | `IsLave` — STRICT (96 candidate bytes) |
| `0x101b0` | 94 | `IsRessort` — STRICT (96 candidate bytes) |
| `0x10210` | 115 | `IsSurfWater` — forced MISMATCH (128 candidate bytes) |
| `0x10290` | 71 | `IsDeepWater` — forced MISMATCH (80 candidate bytes); a declaration-count context probe is strict, diagnostic only |
| `0x102e0` | 75 | `IsOutWater` — forced MISMATCH (96 candidate bytes) |
| `0x10330` | 63 | `IsPassIcon` — STRICT (64 candidate bytes; table relocation proven) |
| `0x10370` | 55 | `IsBlocIcon` — STRICT (64 candidate bytes; table relocation proven) |
| `0x103b0` | 200 | `SearchDistRight` — forced MISMATCH (272 candidate bytes) |
| `0x10480` | 486 | `IsVentillo` — forced MISMATCH (544 candidate bytes); `BlupiDead` is not the target method |

[EVIDENCE] `IsPassIcon`/`IsBlocIcon` required private `MAXQUART=237` (target compares the limit to `0xed`); the inherited value 441 mismatched. `IsLave` required the target's early bounds guard and full-width `BOOL` return. The inherited `IsPiege` candidate was replaced with `IsRessort`: the target checks icon `0xd3` and adds `(30,60)` to the point, matching the spring behavior. These four tail functions are forced matches, not inferred from wildcard hits.

[OPEN] The four actual target functions between `TestPath` and `IsWorld` (`0xfc10`, `0xfdd0`, `0xfe20`, `0xfe80`) are not present in SB2 `decblupi.cpp` and remain absent from the candidate. They are real missing function bodies before `IsDeepWater` and may account for some TU symbol-count difference, but no exact count contribution is proven. Their source names, intended TU, and source forms remain open; do not add declarations or placeholders solely to reproduce the context probe.

## Decblock ownership probes

[EVIDENCE] A fresh SP3 build of the full scratch `decblock.cpp` candidate still reports 44 functions, zero strict hits, and zero wildcard hits (`decblock_current_auto.json`). The current source starts with `BlitzActif` and `SoundEnviron`; its body set does not explain the first target entries at `0x3910`–`0x3b80`.

[EVIDENCE] Three probe results separate assignment from source-generation problems. First, the read-only SB2 `decor.cpp` diagnostic copy (`decor_init_probe.obj`) mismatches the target CDecor constructor, destructor, `Create`, `LoadImages`, `InitGamer`, and `InitDecor` at `0x3910`–`0x3b80` after the engine layout correction. Second, that same object is STRICT when forced at `SetTime=0x3e70`, `GetTime=0x3e80`, and `IsTerminated=0x41c0`, with no relocations. Third, the current `decblock.cpp` object has no exact-byte or wildcard hits in its automatic pass. The forced bodies support `decor.cpp` as a source-family hypothesis for at least three rows currently assigned to `decblock.cpp`, but do not prove TU placement or exclude a folded owner; the initialization methods still differ beyond the corrected member offsets.

[HYPOTHESIS] Propose moving the `0x3e70`, `0x3e80`, and `0x41c0` map entries to `decor.cpp`, subject to the supervisor's contiguous TU-order reconciliation. The current `0x3910`–`0x3b80` initialization sequence appears to be `decor.cpp` behavior but is not a strict source match. Report the remaining decblock block as unresolved rather than spending more probes without a new SB1 source or object-order anchor. No oracle map rows were edited.

## CDecor layout and private header deltas

[EVIDENCE] Oracle accesses place `m_decor` at `+0x10`, `m_bigDecor` at `+0x4e30`, and the 48-byte `m_moveObject` records at `+0x9c50`. The target has no two 1,251-element trajectory arrays before `m_moveObject`; moving them after `m_lastDecorIndexes` in the private header removes their combined `0x9c8` displacement. The tail placement is still unproven.

[EVIDENCE] Target accesses put `m_blupiPower` at `+0xc934`, `m_blupiShield` at `+0xc938`, `m_blupiFifoNb` at `+0xc978`, and `m_blupiFifoPos` at `+0xc97c`. In the scratch header, moving FIFO count and positions ahead of the no-barre/timer fields shifted them back by `0x1c`; placing Power before Shield shifted Power by four bytes. These offsets are supported by strict `CaisseGetMove` and `BlupiAddFifo` bodies.

[EVIDENCE] In the private engine header only, `MoveObjectDelete` now returns `void` instead of the inherited `int`. Its target body at `0x13a30` has no result value; the new signature and independent start/end checks compile to a strict body. Treat the signature as a source inference from the body until the shared header owner reconciles it.

[EVIDENCE] The engine worker's private header now uses `MAXQUART=237`, supported by the strict `IsPassIcon` and `IsBlocIcon` bodies and their proven table relocation. The independent decor worker header still has 441.

[EVIDENCE] The private inline `MoveObjectCopy` now uses `sizeof(*dest)` (48 bytes) instead of `sizeof(dest)` (4-byte pointer). Target pseudocode for `MoveObjectSort` copies twelve dwords per record. This fixes the inherited SB2 helper's copy length; the function remains a mismatch, so this private helper change is not yet a strict anchor.

[EVIDENCE] The independent `scratch/tu/decor_include/decor.h` agrees that trajectory arrays do not precede `m_moveObject`. Its present Blupi field declarations still put Shield before Power, put FIFO positions after `m_blupiNoBarre`, and do not declare `m_blupiFifoNb` by that name. It therefore does not yet reproduce the offsets established by the engine matches. I did not edit the decor worker's files.

[EVIDENCE] The latest `docs/DECOR_TU.md` confirms agreement on `m_decor=+0x10`, `m_bigDecor=+0x4e30`, and `m_moveObject=+0x9c50`, and independently supports no trajectory arrays before `m_moveObject`. The current headers disagree on later field identities/order: the decor header labels `+0xc978` as `m_blupiNoBarre`, while the engine evidence uses `+0xc978` for the FIFO count; both place FIFO positions at `+0xc97c`. The target byte evidence also requires Power at `+0xc934` before Shield at `+0xc938`, opposite the decor header's declaration order. The engine header retains the trajectory arrays at the class tail, which is still unproven. The decor worker's source and header were not edited.

[EVIDENCE] The private `SortGetType` declaration is global `int __cdecl`, rather than the SB2 member returning `char`. The private `PlaceDecor` declaration was also changed to test the global helper form, but its body at `0x5ba0` remains a mismatch. These are scratch-only declaration changes.

## TU boundary and ownership findings

[EVIDENCE] The function at `0x5950` has a 208-byte body ending at `0x5a20`. The sequence `ScreenPosToCelPos` (`0x5a20`), `SetCelHiliFromScreenPos` (`0x5ab0`), `SetIconLiftAndStuff` (`0x5ae0`), `DeleteCel` (`0x5b40`), and the `PlaceDecor` helper (`0x5ba0`) follows the `decdesign.cpp` candidate order. Forced comparisons make `SetCelHiliFromScreenPos` and `DeleteCel` strict from `decdesign.obj`; `decblock.obj` produces neither body. This is evidence for ordinary `decdesign` ownership, not a fold.

[EVIDENCE] `ScreenPosToCelPos` at `0x5a20`, `SetIconLiftAndStuff` at `0x5ae0`, and the `PlaceDecor` helper at `0x5ba0` remain mismatches. `SetIconLiftAndStuff` still has a different branch/store sequence after the shared member offsets were corrected.

[HYPOTHESIS] Proposed map correction: end `decblock.cpp` at the function ending `0x5a20`, and begin `decdesign.cpp` at `0x5a20`; move the entries at `0x5a20` through `0x5ba0` into `decdesign.cpp`. `oracle/tu_map.csv` was not edited. The current `decdesign` start at `0x5c20` is not supported by these object-order matches.

[EVIDENCE] `SetCelHiliFromScreenPos` and `DeleteCel` are ordinary `decdesign` functions under the proposed boundary correction: their bodies are emitted in `decdesign.obj` and not in `decblock.obj`. They are not folds.

[EVIDENCE] `ObjectDelete` at `0x108a0` is a genuine cross-TU duplicate under the current map. The diagnostic `decnet.obj` emits the 112-byte body at its mapped entry with both relocations verified; the forced `decmove.obj` body at the same RVA is also strict, with calls to `MoveObjectSearch=0x13cb0` and `NetMessagePush=0x113c0` verified. This supplies the second-TU body required for folding. The one target body is counted once; keep its map row in `decnet.cpp` to preserve the contiguous source-order assignment.

[EVIDENCE] With `/Gy` and identical-COMDAT folding established for this toolchain, the two strict `ObjectDelete` bodies support an actual COMDAT fold. If later source-order anchors move the decnet boundary, re-evaluate the row; the diagnostic decnet anchors and inferred `decmove` start at `0x10a00` remain provisional.

[OPEN] The decdesign candidate has two strict mission-title anchors near `0x8520`; the interval between its corrected start and those anchors is still mostly unmatched. The decblupi anchors at `0xf540`, `0xf5b0`, and `0xf8c0` support that block, but the large `0x9280` body remains a substantial source or TU-context mismatch.

## Compiler findings

- [EVIDENCE] **`VC5-EXP-03` / `SRC-03`, `SortGetType`:** target is a global `int __cdecl` helper over type values 2, 3, and 12. A `char` return emitted a 32-byte candidate and mismatched; changing the private helper return to `int` emitted a 48-byte window matching the 34-byte target body plus alignment. The width effect is confirmed-transferable; helper semantics/signature are Blupi-specific.
- [EVIDENCE] **`VC5-CTL-01` and `VC5-CTL-03`:** with VC5 SP3 `/O2 /GX /ML /Gd /Gy` in `decblupi`, changing the inherited `GetPosDecor` block order to test zero width/height before the clamp changed the branch layout and made all 108 target bytes strict. Classification: Blupi-specific source form; this SB1 example validates the branch-order effect.
- [EVIDENCE] **`BLUPI-LAY-02`:** in the same flags/TU context, moving FIFO count and positions ahead of the no-barre/timer fields changed their accesses from `+0xc994/+0xc998` to target `+0xc978/+0xc97c`; `BlupiAddFifo` became strict (142 target bytes). Classification: Blupi-specific layout.
- [EVIDENCE] **`BLUPI-LAY-03`:** moving `m_blupiPower` before `m_blupiShield` changed the Power access from `+0xc938` to target `+0xc934`; `CaisseGetMove` became strict (89 target bytes). Classification: Blupi-specific layout.
- [EVIDENCE] **`VC5-EXP-03` and `VC5-SEM-01`:** in `decmove`, `TestPushOneCaisse` changed from `return DecorDetect(rect);` to `return DecorDetect(rect) != 0;`, matching target pseudocode's normalized boolean return. With the `DecorDetect` relocation mapped, the 229-byte target body became strict. Classification: Blupi-specific source semantics.
- [EVIDENCE; negative] **`VC5-SEM-01`:** at `SearchLinkCaisse` (`0x13020`), removing `{0,0,0,0}` initialization from the `IntersectRect` output local reduced the candidate from 480 to 464 bytes; the target body is 453 bytes and its frame is `0x44`, versus candidate frame `0x48`. Changing the flag from `BOOL` to `bool` and iterating the two integer arrays with pointers did not reduce the code further. The forced pair remains MISMATCH. Classification: negative for these variants as a complete fix; the uninitialized output and pointer walks follow target pseudocode.
- [EVIDENCE; negative] **`VC5-CTL-03`:** `MoveObjectPriority` at `0x13c10` has the right member offsets and record stride, but the inherited body differs. The decompiled `while` form and two natural equivalent `for`/`do-while` loop forms were compiled in the same `decmove` context; none matched the 160-byte target. The `for` form added a duplicate bound check and the `do-while` form retained a different loop exit layout. Keep the target-supported leading-ball scan; further loop experiments are low yield without new source evidence.
- [EVIDENCE; negative] **`VC5-SEM-01` / `BLUPI-CTX-01` / `VC5-CTX-01..04`:** `MoveObjectSort` (`0x13b20`) inherited a 4-byte `memcpy` length from `sizeof(dest)`; target pseudocode copies twelve dwords per 48-byte record. Changing the private helper to `sizeof(*dest)`, removing the SB2-only `UpdateCaisse`/link-count tail, and sequencing the two `SortGetType` calls made the candidate frame match the target (`0x3c`) and aligned the first 109 fixed bytes, but the 240-byte candidate still mismatches the 234-byte body. A `bool` pass flag grew it to 256 bytes, so the candidate is back to `BOOL`. The first remaining fixed-byte difference is register allocation at `+0x6d`. A scratch `vc5probe.py` sweep of 0–128 top-level extern-int declarations and 0–32 declarations immediately before the method left the 240-byte candidate unchanged (same SHA-256); generic count context is inert here. Classification: negative for the tested declaration-count family, not a license to add fillers. The record-copy correction is target-supported; actual missing or extra source declarations remain open.
- [EVIDENCE; Blupi-specific, `VC5-EXP-03`]: `IsLave` at `0x10150` becomes STRICT with a full-width `BOOL` return and early bounds guard. The byte-return variant leaves `mov al,dl`, while the target uses `mov eax,edx`; the final candidate is 96 COFF bytes around an 88-byte target body. The combined source correction is Blupi-specific; the return-width residue confirms the VC5 result-width effect.
- [EVIDENCE; negative, `VC5-EXP-03`]: `GetTypeBarre` at `0x10050` was compiled with both `char` and `int` return declarations in isolated scratch variants. Both forced comparisons remain MISMATCH at 256 candidate bytes versus a 248-byte target body. Return width alone does not account for the target's call/branch layout; the signature is still unverified because pseudocode types are unreliable.
- [EVIDENCE; diagnostic only, `BLUPI-CTX-01` / `VC5-CTX-01..04`]: `IsDeepWater` at `0x10290` is MISMATCH with the candidate's natural TU context (80 COFF bytes versus a 71-byte target body). `vc5probe.py` top-placement probes with 32 or 64 extern-int declarations produce the same 80-byte body and a STRICT comparison; counts 0–16 mismatch. The byte change only swaps the `y` then `x` division/load order to match target. This validates context sensitivity on SB1 but does not identify the actual declarations or functions missing from the candidate; the diagnostic declarations were not copied into source.
- [EVIDENCE] **`VC5-SEM-01`:** `MoveObjectDelete` (`0x13a30`) inherited an `int` return accumulator and `else if` from SB2. The target body independently checks both start and end cells and has no returned result. Changing the private declaration/definition to `void` and making the second check independent reduced the candidate from 144 to 128 COFF bytes and made all 115 target bytes strict in the same `/O2 /GX /ML /Gd /Gy` `decmove` context. Classification: Blupi-specific semantic/source correction; the combined variation does not isolate the effect of either change.
- [EVIDENCE; negative] **`VC5-SEM-01` / `VC5-CTL-03`:** for `MoveObjectDetect` (`0x135e0`), grouping the helicopter action filter correctly, adding the four creature/player-position skips from the target pseudocode, and leaving the `IntersectRect` output uninitialized changed the SB2 candidate from 512 to 608 bytes; the 538-byte target still mismatches. A local `short type` matching the pseudocode's saved type value had no further size effect. Classification: negative for these changes as a complete source reconstruction; target parameter/load order and remaining source form are unresolved.
- [EVIDENCE; negative] **`VC5-SEM-01`:** a global cdecl `PlaceDecor` helper still mismatches at `0x5ba0`; target pseudocode copies a dword through an `int *`, while the candidate uses a `short *` and word accesses. No source-supported type correction has been tested. Classification: unverified source hypothesis.

## Current limits and next step

[OPEN] The current full `decblupi` object has seven forced strict bodies; automatic placement remains unproved for its added tail methods. The four target entries between `TestPath` and `IsWorld` are still unmapped, and `IsDeepWater` needs the real declaration context before its diagnostic strict result can transfer. `decmove` retains 11 automatic strict anchors; `MoveObjectDetect` (608/538), `SearchLinkCaisse` (464/453), `MoveObjectSort` (240/234), and `MoveObjectPriority` (160/160) remain forced mismatches. The decblock candidate has zero hits, but three functions in its presumed interval are strict from the SB2 `decor.cpp` object, so revisit the map only with contiguous TU-order evidence. The proposed decblock/decdesign boundary at `0x5a20` remains unchanged.
