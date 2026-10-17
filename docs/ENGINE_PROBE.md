# SB engine probes

## Method

[EVIDENCE] Scratch candidates are `decblock.cpp`, `decdesign.cpp`, `decmove.cpp`, and `decblupi.cpp`, with private headers under `scratch/tu/engine_include/`. The SB2-derived source copies remain in original function order. `decnet.cpp` was also compiled as a diagnostic owner probe. No canonical source, manifest, oracle map, assets, or external reference files were changed.

[EVIDENCE] VC5 SP3 11.00.7022 compiled with the manifest defines and flags: `WIN32`, `NDEBUG`, `_WINDOWS`, `nullptr=0`, `/W3 /GX /O2 /ML /Gd /Gy`; `TMP` and `TEMP` point at `build/tmp`. Current full-TU checks use isolated `--build-dir` paths, including `engine_refreshed_baseline`, `engine_design_followup_final`, `engine_block_flags_followup`, and `engine_combined_final`.

## Canonical-header recheck (2026-10-17)

[EVIDENCE] The canonical `src/` headers are the baseline for this follow-up. Refreshed baseline source copies are in `scratch/tu/engine_current/`, which contains only `.cpp` files so quoted includes resolve to `src/`. Private-header experiments remain beside their candidates, including `scratch/tu/des_screen_local/` and `scratch/tu/engine_flag_reorder/`. Candidate builds used `tools/verify.py --tu ... --build-dir <worker>` with separate build directories.

[EVIDENCE] The refreshed canonical-header build preserves 34 unique strict bodies across the owned blocks: `decblock` 12, `decblupi` 7, `decmove` 11, and `decdesign` 4. `decmove.obj` also emits an identical `ObjectDelete` body owned by `decnet.cpp`; it is not an additional unique match. The scratch `decdesign` candidate documented below adds three strict bodies without modifying canonical source.

### Current decblupi per-function table

[EVIDENCE] The tables below distinguish refreshed canonical results from scratch-only fixes; no candidate changes were integrated into `src/` by this worker.

| Oracle RVA | Oracle bytes | Current candidate/result |
|---:|---:|---|
| `0x9280` | 25,269 | `BlupiStep` — MISMATCH |
| `0xf540` | 108 | `GetPosDecor` — STRICT |
| `0xf5b0` | 142 | `BlupiAddFifo` — STRICT |
| `0xf640` | 640 | `DecorDetect` — MISMATCH |
| `0xf8c0` | 833 | `TestPath` — STRICT; four call relocations proven |
| `0xfc10` | 439 | Target-derived scratch `int MoveObjectPollution` — MISMATCH (480); local-`POINT` variant 496. Canonical void body remains in `decmove` |
| `0xfdd0` | 75 | `MoveObjectPlouf` — STRICT |
| `0xfe20` | 94 | `MoveObjectTiplouf` — STRICT |
| `0xfe80` | 261 | `MoveObjectBlup` — STRICT |
| `0xff90` | 184 | Canonical `IsWorld` — MISMATCH (272); narrowed target-range scratch form — MISMATCH (192) |
| `0x10050` | 248 | `GetTypeBarre` — MISMATCH |
| `0x10150` | 88 | `IsLave` — MISMATCH (96 candidate bytes) |
| `0x101b0` | 94 | `IsRessort` — MISMATCH (96 candidate bytes) |
| `0x10210` | 115 | `IsSurfWater` — MISMATCH |
| `0x10290` | 71 | `IsDeepWater` — STRICT (80 candidate bytes) |
| `0x102e0` | 75 | `IsOutWater` — MISMATCH |
| `0x10330` | 63 | `IsPassIcon` — RELOC-UNVERIFIED; code bytes match, `table_decor_quart` definition is missing |
| `0x10370` | 55 | `IsBlocIcon` — RELOC-UNVERIFIED; code bytes match, `table_decor_quart` definition is missing |
| `0x103b0` | 200 | `SearchDistRight` — MISMATCH |
| `0x10480` | 486 | `IsVentillo` — MISMATCH |

[EVIDENCE] `IsDeepWater` is an automatic unique STRICT hit with canonical headers, resolving the former context-only diagnostic. `IsPassIcon` and `IsBlocIcon` match all code bytes but remain unverified because the external `table_decor_quart` relocation resolves to `0x00431aa0` without a unique reconstructed data definition.

### Current decdesign table

| Oracle RVA | Oracle bytes | Current candidate/result |
|---:|---:|---|
| `0x5a20` | 133 | Canonical `ScreenPosToCelPos` — MISMATCH (144); scratch local-result form — STRICT |
| `0x5ab0` | 46 | `SetCelHiliFromScreenPos` — STRICT |
| `0x5ae0` | 93 | Canonical `SetIconLiftAndStuff` — MISMATCH (128); scratch target-condition form — STRICT (96) |
| `0x5b40` | 90 | `DeleteCel` — STRICT |
| `0x5ba0` | 125 | Canonical `PlaceDecor` — MISMATCH (144); scratch `int*` helper/pointer-update form — STRICT (128) |
| `0x8520` | 7 | `GetMissionTitle` — STRICT |
| `0x8530` | 46 | `SetMissionTitle` — STRICT |

[EVIDENCE] The `0x5a20` body is the first decdesign function under the corrected boundary. The forced anchors at `0x5ab0` and `0x5b40` are ordinary decdesign bodies, not folds. `ObjectDelete` at `0x108a0` remains owned by `decnet.cpp`; the identical `decmove.obj` source copy is excluded as a second target function.

[EVIDENCE] `scratch/tu/des_screen_local/decdesign.cpp` with its private sibling `decor.h`, verified as `engine_design_followup_final`, has seven strict decdesign bodies. Its three additions over canonical source are `ScreenPosToCelPos` (133 bytes), `SetIconLiftAndStuff` (93), and global `PlaceDecor` (125), with all relocations proven.

## Strict body matches

[EVIDENCE] The refreshed canonical-header verification finds 34 unique strict functions in the owned blocks: 12 in `decblock`, seven in `decblupi`, eleven in `decmove`, and four in `decdesign`. They cover 3,403 oracle bytes; candidate COFF code sizes below may include alignment bytes beyond the Ghidra body size. The three scratch-only decdesign matches and scratch `AdaptMotorVehicleSound` add four bodies and 474 bytes, for 38 bodies and 3,877 bytes.

[EVIDENCE] `TestPushCaisse` direct calls resolve to `SearchLinkCaisse=0x00413020` and `TestPushOneCaisse=0x00412f30`; `TestPath` calls resolve to `DecorDetect=0x0040f640`. `ObjectDelete`'s forced calls resolve to `MoveObjectSearch=0x00413cb0` and `NetMessagePush=0x004113c0`.

| Candidate method | RVA | Oracle body bytes | Candidate COFF bytes |
|---|---:|---:|---:|
| `CDecor::GetMissionTitle` | `0x8520` | 7 | 16 |
| `CDecor::SetMissionTitle` | `0x8530` | 46 | 48 |
| `CDecor::InitGamer` | `0x3b60` | 31 | 32 |
| `CDecor::SetTime` | `0x3e70` | 13 | 16 |
| `CDecor::GetTime` | `0x3e80` | 7 | 16 |
| `CDecor::BuildPrepare` | `0x4150` | 109 | 112 |
| `CDecor::IsTerminated` | `0x41c0` | 7 | 16 |
| `CDecor::GetBlupiChannelActual` | `0x42e0` | 40 | 48 |
| `CDecor::GetBlupiChannelStandard` | `0x4310` | 30 | 32 |
| `CDecor::GetIconPerso` | `0x4330` | 26 | 32 |
| `CDecor::KeyChange` | `0x5780` | 13 | 16 |
| `CDecor::SetJoystickEnable` | `0x5790` | 13 | 16 |
| `CDecor::SetDemoPlay` | `0x57a0` | 13 | 16 |
| `CDecor::PlaySound` | `0x5800` | 124 | 128 |
| `CDecor::AdaptMotorVehicleSound` | `0x58d0` | 123 | 128 |
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
| `CDecor::IsDeepWater` | `0x10290` | 71 | 80 |
| `CDecor::MoveObjectPlouf` | `0xfdd0` | 75 | 80 |
| `CDecor::MoveObjectTiplouf` | `0xfe20` | 94 | 96 |
| `CDecor::MoveObjectBlup` | `0xfe80` | 261 | 272 |
| `CDecor::SetCelHiliFromScreenPos` | `0x5ab0` | 46 | 48 |
| `CDecor::DeleteCel` | `0x5b40` | 90 | 96 |
| `CDecor::ScreenPosToCelPos` (scratch) | `0x5a20` | 133 | 144 |
| `CDecor::SetIconLiftAndStuff` (scratch) | `0x5ae0` | 93 | 96 |
| `PlaceDecor` (scratch) | `0x5ba0` | 125 | 128 |

[EVIDENCE] The canonical `decnet` object has strict anchors at `NetStopCloud` (`0x10670`, 50 oracle bytes), `ObjectDelete` (`0x108a0`, 112), `NetPlaySound` (`0x10980`, 59), and `NetStopSound` (`0x109c0`, 50). These anchors are outside the three owned blocks and help establish the neighboring TU's boundaries. `ObjectDelete` accounts for part of the gap between `NetStopCloud` and `NetPlaySound`; unmatched space remains on both sides.

[EVIDENCE] A forced comparison confirms that `decmove.obj` also emits an identical `ObjectDelete` body at `0x108a0` (112 target bytes; relocations proven), but `recon.json` owns and accepts the target function in `decnet.cpp` and excludes the `decmove.cpp` source copy as a second target function. The 34 canonical owned-block bodies cover 3,403 unique target bytes. Adding the four unique `decnet` anchors gives 38 target bodies and 3,674 bytes; count `ObjectDelete` once. The three scratch-only decdesign matches and scratch `AdaptMotorVehicleSound` bring the comparison total to 42 bodies and 4,148 bytes.

[EVIDENCE; pre-merge scratch result, superseded] The earlier `engine_forced_complete.json` note reported seven forced `decblupi` bodies under a scratch header overlay. The refreshed canonical source now independently has seven strict decblupi bodies, including `MoveObjectPlouf`, `MoveObjectTiplouf`, and `MoveObjectBlup`; the `IsPassIcon` and `IsBlocIcon` rows remain relocation-unverified and are excluded from strict coverage. The earlier result is retained only as investigation history.

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
| `0x13b20` | 234 | `MoveObjectSort` — forced MISMATCH (256 candidate bytes with canonical headers) |
| `0x13c10` | 160 | `MoveObjectPriority` — forced MISMATCH |
| `0x13cb0` | 153 | `MoveObjectSearch` — STRICT |

[HYPOTHESIS] The broad `decmove` interval `0x10a00`–`0x13cb0` remains plausible from the function map and later strict anchors, but the start has no strict anchor. `ObjectDelete` at `0x108a0` is handled separately below because it falls in the mapped `decnet` interval.

## Pre-merge decblupi probes (historical)

[EVIDENCE; pre-merge scratch result, superseded for current coverage] This full candidate preserved the then-current SB2 `decblupi.cpp` function order, then appended helpers from scratch `decblock.cpp`/`decmove.cpp` copies. Its combined forced comparison reported seven strict bodies under a private header overlay; those results are historical and are not current canonical-header coverage. `TestPath`'s four calls resolve to `DecorDetect=0x0040f640`; the two table helper relocations resolve to `table_decor_quart=0x00431aa0` but are not independently proven.

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
| `0x10150` | 88 | `IsLave` — MISMATCH (96 candidate bytes with canonical headers) |
| `0x101b0` | 94 | `IsRessort` — MISMATCH (96 candidate bytes with canonical headers) |
| `0x10210` | 115 | `IsSurfWater` — forced MISMATCH (128 candidate bytes) |
| `0x10290` | 71 | `IsDeepWater` — STRICT (80 candidate bytes with canonical headers) |
| `0x102e0` | 75 | `IsOutWater` — forced MISMATCH (96 candidate bytes) |
| `0x10330` | 63 | `IsPassIcon` — RELOC-UNVERIFIED (64 code bytes match; table definition not reconstructed) |
| `0x10370` | 55 | `IsBlocIcon` — RELOC-UNVERIFIED (64 code bytes match; table definition not reconstructed) |
| `0x103b0` | 200 | `SearchDistRight` — forced MISMATCH (272 candidate bytes) |
| `0x10480` | 486 | `IsVentillo` — forced MISMATCH (544 candidate bytes); `BlupiDead` is not the target method |

[EVIDENCE] `IsPassIcon`/`IsBlocIcon` use the target-compatible `MAXQUART=237` (the limit compares to `0xed`); their code is exact, but the data relocation is unverified. `IsLave` uses an early bounds guard and full-width `BOOL` return, yet its current merged-header code still differs in division/register scheduling. `IsRessort` checks icon `0xd3` at the point shifted by `(30,60)`; its current candidate also remains a mismatch.

[EVIDENCE] The four target bodies between `TestPath` and `IsWorld` (`0xfc10`, `0xfdd0`, `0xfe20`, `0xfe80`) were absent from SB2 `decblupi.cpp`; the scratch candidate now reconstructs all four by behavior. `MoveObjectPlouf`, `MoveObjectTiplouf`, and `MoveObjectBlup` are STRICT; `MoveObjectPollution` remains a 480-byte MISMATCH against 439. No placeholder declarations were added.

## Oracle-first CDecor block reconstruction (2026-10-17)

[EVIDENCE] The target interval assigned by `oracle/tu_map.csv` to `decblock.cpp` contains 26 CDecor methods from `0x3910` through `0x5950`. Their field offsets, callees, and strings align much more closely with behavior in SB2 `external/sb2-decomp/src/decor.cpp` than with SB2 `decblock.cpp`; this is behavior/source ancestry evidence only. The SB1 TU filename remains a link-order hypothesis. I did not rename any source or edit the map.

[EVIDENCE] The oracle-first block candidate is `scratch/tu/engine_oracle_block/decblock.cpp`; it was verified with canonical headers using `tools/verify.py --tu src/decblock.cpp=scratch/tu/engine_oracle_block/decblock.cpp --build-dir engine_oracle_block_final`. It contains 12 STRICT target bodies totaling 426 unique oracle bytes. The 12 are `InitGamer` (`0x3b60`, 31), `SetTime` (`0x3e70`, 13), `GetTime` (`0x3e80`, 7), `BuildPrepare` (`0x4150`, 109), `IsTerminated` (`0x41c0`, 7), `GetBlupiChannelActual` (`0x42e0`, 40), `GetBlupiChannelStandard` (`0x4310`, 30), `GetIconPerso` (`0x4330`, 26), `KeyChange` (`0x5780`, 13), `SetJoystickEnable` (`0x5790`, 13), `SetDemoPlay` (`0x57a0`, 13), and `PlaySound` (`0x5800`, 124). All relocations on these functions are proven, and the run retained the previously accepted functions at their accepted RVAs. A separate current-source scratch candidate, `scratch/tu/engine_adapt_sound/decblock.cpp`, has 13 STRICT decblock bodies because `AdaptMotorVehicleSound` now matches `0x58d0` (123 target bytes; 128-byte candidate section) with its calls proven to `CSound::Play` at `0x23220`. `PlayPrepare` remains a 784-byte mismatch against 703; target-backed field/loop changes reduce it to 768 but it remains MISMATCH. `GetBlupiInfo` remains 80 against 67.

| Oracle RVA | Oracle bytes | Behavior-derived source/result |
|---:|---:|---|
| `0x3910` | 158 | CDecor constructor: initializes the embedded gauge and selected state fields; no candidate yet |
| `0x39b0` | 22 | CDecor destructor: destroys the embedded gauge; no candidate yet |
| `0x39d0` | 196 | `Create` behavior: stores four object pointers, initializes decor and gauges, then invokes two helpers; no candidate yet |
| `0x3aa0` | 183 | `LoadImages`: target strings are `image16\\decor\\3d.blp` and `image08\\decor\\3d.blp`; no candidate yet |
| `0x3b60` | 31 | `InitGamer` — STRICT |
| `0x3b80` | 744 | `InitDecor`: clears the 100×100 maps and initializes object/default state from fixed values; no candidate yet |
| `0x3e70` | 13 | `SetTime` — STRICT |
| `0x3e80` | 7 | `GetTime` — STRICT |
| `0x3e90` | 703 | Canonical `PlayPrepare` — MISMATCH (784); target-store/loop scratch form — MISMATCH (768) |
| `0x4150` | 109 | `BuildPrepare` — STRICT |
| `0x41c0` | 7 | `IsTerminated` — STRICT |
| `0x41d0` | 261 | Tick/input helper: calls target RVAs `0x115c0`, `0x9280`, `0x11160`, and `0x115a0` by phase, and updates scroll coordinates from key bits; no candidate yet |
| `0x42e0` | 40 | `GetBlupiChannelActual` — STRICT after target channel constant correction |
| `0x4310` | 30 | `GetBlupiChannelStandard` — STRICT after target channel constant correction |
| `0x4330` | 26 | `GetIconPerso` — STRICT |
| `0x4350` | 3,866 | `Build`/decor drawing behavior; calls `DecorNextAction`, gauge and pixmap helpers; no candidate yet |
| `0x5270` | 1,010 | `DrawInfo`/HUD behavior; no candidate yet |
| `0x5670` | 269 | `DecorNextAction` (hidden POINT return) behavior; no candidate yet |
| `0x5780` | 13 | `KeyChange` — STRICT as a direct key-state setter |
| `0x5790` | 13 | `SetJoystickEnable` — STRICT |
| `0x57a0` | 13 | `SetDemoPlay` — STRICT |
| `0x57b0` | 67 | `GetBlupiInfo` behavior is identified from four output stores; candidate MISMATCH (80 bytes) |
| `0x5800` | 124 | `PlaySound` — STRICT; target-relative coordinate subtraction and explicit locality argument |
| `0x5880` | 68 | `StopSound` behavior is clear, but the merged `CSound` API has no `StopSound` declaration/body; no compilable candidate |
| `0x58d0` | 123 | Canonical `AdaptMotorVehicleSound` — MISMATCH (144); scratch coordinate-based `CSound::Play` calls — STRICT |
| `0x5950` | 208 | `ChangePhase` sound/phase behavior identified; target sound stop helper is absent from the merged `CSound` API |

[EVIDENCE] The four target `decblupi` bodies were matched by behavior against SB2 `decmove.cpp`, independent of SB2 filename placement. Candidate `scratch/tu/engine_oracle_blupi/decblupi.cpp` and its private header were verified with `tools/verify.py --tu src/decblupi.cpp=scratch/tu/engine_oracle_blupi/decblupi.cpp --build-dir engine_oracle_blupi_final`; the run retained the previously accepted functions at their accepted RVAs.

| Oracle RVA | Oracle bytes | Behavior/result |
|---:|---:|---|
| `0xfc10` | 439 | Periodic pollution emitter. Oracle-derived candidate returns either the timer quotient or `ObjectStart`; it removes SB2's over-state branch and shrinks from 624 to 480 candidate bytes, but remains MISMATCH |
| `0xfdd0` | 75 | `MoveObjectPlouf` — STRICT |
| `0xfe20` | 94 | `MoveObjectTiplouf` — STRICT |
| `0xfe80` | 261 | `MoveObjectBlup` — STRICT |

[EVIDENCE] The oracle-derived `0xfc10` body tests action value 3, emits pollution for helicopter/jeep/tank states, uses phase periods and residues from the seeded pseudocode, offsets x by 22/32/35, and returns the four-argument object-start result. It has no `m_blupiOver` branch. The private candidate changes `MoveObjectPollution`'s return type from the canonical `void` to `int` because the target returns timer quotients and the object-start result; this signature remains a hypothesis because target names/signatures are not reliable. The private return-type delta was not integrated.

### CDecor tail reference inventory

[EVIDENCE] `build/decomp/tu/blupi.c` shows `DoInit` allocating `0x258d4` bytes before calling the CDecor constructor at `0x3910`. The merged CDecor layout is `0xdbec`; the unexplained allocation tail is therefore `0x17ce8` bytes (97,512 bytes) beyond the known class end.

[EVIDENCE] I scanned all seeded pseudocode in `decblock`, `decblupi`, `decdesign`, `decmove`, `decnet`, and `decor` for fixed `this`/first-object-parameter displacements greater than `+0xc5a8`, including STRICT and near-miss functions. Full rows are in `build/engine/cdecor_member_offsets.tsv`: decblock has 19 functions (maximum `+0xcee4` at `0x5670`), decblupi 5 (`+0xcee4` at `0x9280`), decdesign 27 (`+0xd07c` at `0x8340`), decmove 20 (`+0xcee4` at `0x115c0`), decor 12 (`+0xcee4` at `0x153d0`), and decnet has no fixed direct reference above `+0xc5a8`. No scanned function directly references a CDecor displacement above the current `+0xdbec` size. The apparent `+0xd3c4` in save/read pseudocode is a temporary `DescGameFile` buffer, not `this`.

[HYPOTHESIS; proposed header delta for infra] Append an opaque `BYTE m_unmappedTail[0x17ce8]` after the currently known CDecor fields to represent the allocation size. The target code references do not establish any member boundaries or purpose in this interval. SB2's `dummy[100000]` supports the existence of a large tail reservation, but its length differs by 2,488 bytes and does not prove SB1's internal layout. There is no evidence to label this span as path, undo/history, or demo-recording storage.

## Decblock ownership probes (historical, superseded by oracle-first section)

[EVIDENCE; historical] The following zero-hit conclusion predates the oracle-first candidate and is superseded by the 12 STRICT body results above. Its SB2-only candidates and ownership cautions remain useful context.

[EVIDENCE] A fresh SP3 build of the full scratch `decblock.cpp` candidate still reports 44 functions, zero strict hits, and zero wildcard hits (`decblock_current_auto.json`). The current source starts with `BlitzActif` and `SoundEnviron`; its body set does not explain the first target entries at `0x3910`–`0x3b80`.

[EVIDENCE] Three probes distinguish source resemblance from current TU evidence. First, the read-only SB2 `decor.cpp` diagnostic object mismatches the target constructor, destructor, `Create`, `LoadImages`, `InitGamer`, and `InitDecor` at `0x3910`–`0x3b80` after the layout correction. Second, it is STRICT at `SetTime=0x3e70`, `GetTime=0x3e80`, and `IsTerminated=0x41c0`, with no relocations; SB2 defines these methods in `decor.cpp`. Third, the full current `decblock.cpp` object has zero exact-byte and wildcard hits automatically. The strict diagnostic bodies do not establish the target-emitting TU, and alphabetical TU order places `decor.cpp` after `decblock.cpp`; they do not justify moving these map rows or calling them folds. The SB2 `decblock.cpp` body set differs from this target region. With no SB1 source or object-order anchor, stop probing and report the block unresolved.

[OPEN] `oracle/tu_map.csv` still assigns `0x3e70`, `0x3e80`, and `0x41c0` to `decblock.cpp`; this worker proposes no correction. The `0x3910`–`0x3b80` initialization sequence remains mismatched, and target source generation or TU assignment is unresolved. No oracle map rows were edited.

## CDecor layout and private header deltas

[EVIDENCE] Oracle accesses place `m_decor` at `+0x10`, `m_bigDecor` at `+0x4e30`, and the 48-byte `m_moveObject` records at `+0x9c50`. The target has no two 1,251-element trajectory arrays before `m_moveObject`; moving them after `m_lastDecorIndexes` in the private header removes their combined `0x9c8` displacement. The tail placement is still unproven.

[EVIDENCE] Target accesses put `m_blupiPower` at `+0xc934`, `m_blupiShield` at `+0xc938`, `m_blupiFifoNb` at `+0xc978`, and `m_blupiFifoPos` at `+0xc97c`. In the scratch header, moving FIFO count and positions ahead of the no-barre/timer fields shifted them back by `0x1c`; placing Power before Shield shifted Power by four bytes. These offsets are supported by strict `CaisseGetMove` and `BlupiAddFifo` bodies.

[EVIDENCE] The canonical header declares `MoveObjectDelete` as `void`, matching the target body at `0x13a30`, which has no result value. The independent start/end checks compile to a strict body. Treat the signature as an SB1 source inference supported by this match.

[EVIDENCE; hypothesis] Scratch variants use `MAXQUART=237`, matching the target comparison against `0xed` in `IsPassIcon` and `IsBlocIcon`. Their code bytes match, but the `table_decor_quart` relocation is not proven, so this does not establish the reconstructed data definition or the constant independently.

[EVIDENCE] The canonical inline `MoveObjectCopy` uses `sizeof(*dest)` (48 bytes), matching target pseudocode for twelve dwords per record. The helper change fixes the inherited SB2 pointer-size copy; `MoveObjectSort` remains a mismatch, so this fact alone is not a strict anchor.

[EVIDENCE; pre-merge header comparison, superseded] The decor worker's former private header agreed that trajectory arrays do not precede `m_moveObject`, but disagreed on the later Blupi field identities and order. The canonical `src/decor.h` now agrees with the engine offsets and places those arrays after the engine fields; no header delta is outstanding from this worker.

[EVIDENCE] The canonical `src/decor.h` agrees with the target-supported layout: `m_decor=+0x10`, `m_bigDecor=+0x4e30`, and `m_moveObject=+0x9c50`; no trajectory arrays precede `m_moveObject`; Power is at `+0xc934`, Shield at `+0xc938`, the FIFO-count/no-barre union at `+0xc978`, and FIFO positions at `+0xc97c`. The tail placement of the trajectory arrays remains unproven. The earlier disagreement with the decor worker's private header has been resolved in the canonical header merge; I did not edit decor-owned files.

[HYPOTHESIS; private header delta for infra] Moving `m_blupiOver` after `m_blupiSurf` would align the target-used flag slots as Helico `+0xc90c`, Jeep `+0xc910`, Tank `+0xc914`, Skate `+0xc918`, Nage `+0xc91c`, Surf `+0xc920`, and Over `+0xc924`. This maps the `GetBlupiInfo` output grouping and the three vehicle checks in `MoveObjectPollution` to their target offsets. A scratch header with this order does not by itself make either function STRICT (`GetBlupiInfo` remains 80 vs 67 bytes; `MoveObjectPollution` remains 480 vs 439), so treat the member identities as a proposal pending reconciliation, not an integrated layout correction.

[EVIDENCE] The private `SortGetType` declaration is global `int __cdecl`, rather than the SB2 member returning `char`. In `scratch/tu/des_screen_local/decor.h`, the global `PlaceDecor` helper's first parameter is `int*`, matching target symbol `?PlaceDecor@@YAXPAH00HHH@Z`; the short icon members are passed through explicit `int*` casts. That private declaration and source form produce the 125-byte strict body at `0x5ba0`; no canonical header was changed.

## TU boundary and ownership findings

[EVIDENCE] The function at `0x5950` has a 208-byte body ending at `0x5a20`. The sequence `ScreenPosToCelPos` (`0x5a20`), `SetCelHiliFromScreenPos` (`0x5ab0`), `SetIconLiftAndStuff` (`0x5ae0`), `DeleteCel` (`0x5b40`), and the `PlaceDecor` helper (`0x5ba0`) follows the `decdesign.cpp` candidate order. Forced comparisons make `SetCelHiliFromScreenPos` and `DeleteCel` strict from `decdesign.obj`; `decblock.obj` produces neither body. This is evidence for ordinary `decdesign` ownership, not a fold.

[EVIDENCE] The canonical `decdesign.cpp` still mismatches at `0x5a20`, `0x5ae0`, and `0x5ba0`. The source-form candidate `scratch/tu/des_screen_local/decdesign.cpp` strictly matches all three under the corrected `0x5a20` boundary, confirming ordinary decdesign ownership rather than folding.

[EVIDENCE] The canonical `oracle/tu_map.csv` now starts `decdesign.cpp` at `0x5a20` and assigns the sequence through `0x5ba0` to `decdesign.cpp`. The strict `0x5ab0` and `0x5b40` bodies and candidate order support that boundary. This worker did not edit the map.

[EVIDENCE] `SetCelHiliFromScreenPos` and `DeleteCel` are ordinary `decdesign` functions under the accepted boundary: their bodies are emitted in `decdesign.obj` and not in `decblock.obj`. They are not folds.

[EVIDENCE] `ObjectDelete` at `0x108a0` is owned and accepted in `decnet.cpp`. The forced `decmove.obj` source copy has identical target bytes and proven calls to `MoveObjectSearch=0x13cb0` and `NetMessagePush=0x113c0`, but `recon.json` explicitly excludes it as a second target function. Keep the map row in `decnet.cpp`; no additional folded target body is counted here.

[EVIDENCE] The identical forced comparison proves code identity across the two candidate objects, but the accepted manifest models one target function owned by `decnet.cpp`, not a folded second target entry. The `decmove` block start at `0x10a00` remains provisional because it has no strict anchor at that first RVA.

[OPEN] The decdesign candidate has two strict mission-title anchors near `0x8520`; the interval between its corrected start and those anchors is still mostly unmatched. The decblupi anchors at `0xf540`, `0xf5b0`, and `0xf8c0` support that block, but the large `0x9280` body remains a substantial source or TU-context mismatch.

## Compiler findings

- [EVIDENCE] **`VC5-EXP-03` / `SRC-03`, `SortGetType`:** target is a global `int __cdecl` helper over type values 2, 3, and 12. A `char` return emitted a 32-byte candidate and mismatched; changing the private helper return to `int` emitted a 48-byte window matching the 34-byte target body plus alignment. The width effect is confirmed-transferable; helper semantics/signature are Blupi-specific.
- [EVIDENCE] **`VC5-CTL-01` and `VC5-CTL-03`:** with VC5 SP3 `/O2 /GX /ML /Gd /Gy` in `decblupi`, changing the inherited `GetPosDecor` block order to test zero width/height before the clamp changed the branch layout and made all 108 target bytes strict. Classification: Blupi-specific source form; this SB1 example validates the branch-order effect.
- [EVIDENCE] **`BLUPI-LAY-02`:** in the same flags/TU context, moving FIFO count and positions ahead of the no-barre/timer fields changed their accesses from `+0xc994/+0xc998` to target `+0xc978/+0xc97c`; `BlupiAddFifo` became strict (142 target bytes). Classification: Blupi-specific layout.
- [EVIDENCE] **`BLUPI-LAY-03`:** moving `m_blupiPower` before `m_blupiShield` changed the Power access from `+0xc938` to target `+0xc934`; `CaisseGetMove` became strict (89 target bytes). Classification: Blupi-specific layout.
- [EVIDENCE] **`VC5-EXP-03` and `VC5-SEM-01`:** in `decmove`, `TestPushOneCaisse` changed from `return DecorDetect(rect);` to `return DecorDetect(rect) != 0;`, matching target pseudocode's normalized boolean return. With the `DecorDetect` relocation mapped, the 229-byte target body became strict. Classification: Blupi-specific source semantics.
- [EVIDENCE; negative] **`VC5-SEM-01`:** at `SearchLinkCaisse` (`0x13020`), removing `{0,0,0,0}` initialization from the `IntersectRect` output local reduced the candidate from 480 to 464 bytes; the target body is 453 bytes and its frame is `0x44`, versus candidate frame `0x48`. Changing the flag from `BOOL` to `bool` and iterating the two integer arrays with pointers did not reduce the code further. The forced pair remains MISMATCH. Classification: negative for these variants as a complete fix; the uninitialized output and pointer walks follow target pseudocode.
- [EVIDENCE; negative, `VC5-CTL-03`]: `MoveObjectPriority` at `0x13c10` has the right member offsets and record stride, but the inherited body differs. The decompiled `while` form and two natural equivalent `for`/`do-while` loop forms were compiled in the same `decmove` context; none matched the 160-byte target. The `for` form added a duplicate bound check and the `do-while` form retained a different loop exit layout. Classification: negative for these loop-form variants; the target-supported leading-ball scan remains.
- [EVIDENCE; negative] **`VC5-SEM-01` / `BLUPI-CTX-01` / `VC5-CTX-01..04`:** `MoveObjectSort` (`0x13b20`) copies twelve dwords per 48-byte record; the canonical header's inline helper uses `sizeof(*dest)`. With merged headers the candidate is 256 bytes versus the 234-byte target and still has control/displacement residues as well as the register difference at `+0x6d`. Fresh `vc5probe.py` probes with the canonical header tested top-level declaration counts 0–128 and before-method counts 0–32; every probe remained MISMATCH at 256 bytes. Generic symbol-count context is inert for this current body. Classification: negative for the tested context family; do not add dummy declarations. The remaining source/control-flow difference is open.
- [EVIDENCE; negative, `VC5-EXP-03` / `VC5-CTX-01..04`]: `IsLave` at `0x10150` has the target early bounds guard and full-width `BOOL` result in the canonical candidate, but remains MISMATCH at 96 candidate bytes versus an 88-byte body. A scalar-local/index rewrite retained 96 bytes and the mismatch. Target/candidate disassembly agrees on the field offset and conditions; remaining instructions differ in register and division order. Canonical-header probes at top (0–32 declarations) and immediately before the method (1–32) all remained MISMATCH at 96 bytes. The earlier private-overlay strict claim is not reproduced, so no return-width rule is confirmed by this function.
- [EVIDENCE; negative, `VC5-EXP-03`]: changing both `IsLave` and `IsRessort` declarations/definitions from `BOOL` to `bool`, following Ghidra's inferred return type, left both forced comparisons mismatching at 96 candidate bytes (`IsLave` target 88; `IsRessort` target 94). A top-level declaration-count probe for `IsRessort` at 0 and 32 declarations also remained MISMATCH. Classification: negative for return-width/signature as the complete fix.
- [EVIDENCE; negative then confirmed, `VC5-CTX-01..04` / `VC5-SEM-01`]: `ScreenPosToCelPos` (`0x5a20`) remained MISMATCH at 144 candidate bytes in top-level declaration-count probes from 0 through 128. In the same VC5 SP3 `/O2 /GX /ML /Gd /Gy` decdesign context, changing the direct `POINT(-1,-1)` return to a local `POINT result` with separate x/y assignments in the valid and invalid branches produced a STRICT 133-byte body. The generic context family was negative; the source-form change is Blupi-specific and target-backed.
- [EVIDENCE; negative, `VC5-EXP-03`]: `GetTypeBarre` at `0x10050` was compiled with both `char` and `int` return declarations in isolated scratch variants. Both forced comparisons remain MISMATCH at 256 candidate bytes versus a 248-byte target body. Return width alone does not account for the target's call/branch layout; classification: negative for the return-width variants, with the source signature unverified because pseudocode types are unreliable.
- [EVIDENCE; confirmed-transferable, `BLUPI-CTX-01` / `VC5-CTX-01..04`]: `IsDeepWater` at `0x10290` is an automatic unique STRICT hit at 80 candidate bytes versus a 71-byte target body with canonical headers and full natural TU context. This supersedes the diagnostic-only declaration-count result; the verified source contains no probe declarations.
- [EVIDENCE] **`VC5-SEM-01`:** `MoveObjectDelete` (`0x13a30`) inherited an `int` return accumulator and `else if` from SB2. The target body independently checks both start and end cells and has no returned result. Changing the private declaration/definition to `void` and making the second check independent reduced the candidate from 144 to 128 COFF bytes and made all 115 target bytes strict in the same `/O2 /GX /ML /Gd /Gy` `decmove` context. Classification: Blupi-specific semantic/source correction; the combined variation does not isolate the effect of either change.
- [EVIDENCE; negative] **`VC5-SEM-01` / `VC5-CTL-03`:** for `MoveObjectDetect` (`0x135e0`), grouping the helicopter action filter, adding the four creature/player-position skips from target pseudocode, and leaving the `IntersectRect` output uninitialized changed the candidate from 512 to 608 bytes; the canonical-header candidate remains 608 bytes versus the 538-byte target. A local `short type` had no further size effect. Classification: negative for these changes as a complete reconstruction; target parameter/load order and remaining source form are unresolved.
- [EVIDENCE; confirmed-transferable, `VC5-SEM-01`]: `SetIconLiftAndStuff` (`0x5ae0`) became STRICT (93 oracle bytes) after matching the target's `menu == 2 && index != 3` condition, dropping SB2 exclusions for 9/10, and omitting the SB2-only `menu == 5` assignment. `PlaceDecor` (`0x5ba0`) became STRICT (125 bytes) after the helper's first parameter changed from `short*` to `int*`, icon-field pointers were cast to `int*`, and the target increment was expressed as `(*pLastIndex)++` before wrapping against `pTable[0]`. Both matches use the private scratch header and have all relocations proven; these are source/API differences specific to SB1, not general compiler rules.
- [EVIDENCE; confirmed-transferable, `VC5-SEM-01`]: In oracle-first `decblock`, under VC5 SP3 `/O2 /GX /ML /Gd /Gy`, the behavior-derived `BuildPrepare` body matches `0x4150` exactly (109 oracle bytes; 112-byte candidate section including alignment), with no relocations. `InitGamer`, `SetTime`, `GetTime`, `IsTerminated`, and the three direct setter methods also match at their target RVAs. Classification: confirmed-transferable source bodies; no new compiler rule.
- [EVIDENCE; Blupi-specific, `VC5-SEM-01`]: In the same decblock flags/TU context, the SB2 channel helpers with inherited `CHBLUPI=4` did not represent `0x42e0`/`0x4310`. Replacing the default/guard channel value with target value `2` made both helpers STRICT (40 and 30 oracle bytes). Target pseudocode supports the constant difference. Classification: Blupi-specific semantic constant.
- [EVIDENCE; Blupi-specific, `VC5-SEM-01`]: In the same decblock context, the target `0x5780` body has no SB2 `KeyChange` inversion branch; a direct `m_keyPress = keyPress` setter is STRICT (13 oracle bytes). At `0x5800`, direct point subtraction from `m_posDecor` and four-argument `PlayImage(..., bLocal)` match the target (124 oracle bytes; relocations proven). Classification: Blupi-specific source/API form.
- [EVIDENCE; negative, `VC5-SEM-01` / `VC5-CTX-01..04`]: In the decblock candidate with VC5 SP3 flags, `GetBlupiInfo` at `0x57b0` has four target output stores, including two OR-combined flags. The bitwise-OR candidate is 80 bytes against 67; changing both joins to logical `||` produces 112 bytes and still mismatches. A private header reorder aligns the flag offsets but leaves an 80-byte mismatch; a `before:GetBlupiInfo` context probe with zero declarations also remains MISMATCH. Neither construct nor layout hypothesis is a complete fix; retain bitwise OR as the better behavior candidate. Classification: negative for both source-form variants and the tested context point.
- [EVIDENCE; negative, `VC5-SEM-01`]: `PlayPrepare` at `0x3e90` targets a 200-record walk from `m_moveObject[0]` and only the six random-phase types shown by pseudocode. A scratch decblock candidate changed the inherited reverse/off-by-one loop to a forward 0–199 loop, used a `MoveObject*` countdown loop, omitted the two resets absent from target stores under the private flag layout, and narrowed the randomization list. The section fell from 784 to 768 bytes but remains MISMATCH against 703. The target-backed changes are not a complete reconstruction; field identity/order remains partly hypothetical.
- [EVIDENCE; negative, `VC5-SEM-01`]: In the decblupi candidate with VC5 SP3 `/O2 /GX /ML /Gd /Gy`, the target pollution emitter at `0xfc10` returns timer quotients/object-start status and lacks SB2's over-state branch. The oracle-derived `int` return candidate shrinks the SB2 body from 624 to 480 bytes against the 439-byte target, but remains MISMATCH. Direct modulo expressions instead of local temporaries made no size change. Classification: negative as a complete reconstruction; source signature/form remains unverified.
- [EVIDENCE; negative, `VC5-SEM-01`]: A second `MoveObjectPollution` source form (`POINT pos = m_blupiPos`, then adjust `pos.x` before `ObjectStart`) emitted 496 bytes against the 439-byte target. Both scratch forms use the target-supported DIR_RIGHT check and private flag-order hypothesis; neither is STRICT. This rules out the local-POINT form as a complete fix, not as the target's exact source form.
- [EVIDENCE; negative, `VC5-SEM-01`]: The oracle-derived `IsWorld` behavior narrows SB2's many World/Level cases to three icon ranges (`0x9e..0xa5`, `0xa6..0xad`, `0xae..0xb5`) returning 1–8, plus `0xb8 -> 99`, with other values returning -1. This reduced the candidate from 272 to 192 bytes against the 184-byte target. Separate-int and reversed-argument signature probes did not change the result. Classification: negative for the tested source form; target behavior is supported, but return/control-flow code generation remains open.
- [EVIDENCE; confirmed-transferable, `VC5-SEM-01`]: The scratch `decdesign` candidate `scratch/tu/des_screen_local/decdesign.cpp`, built with VC5 SP3 `/O2 /GX /ML /Gd /Gy`, strictly matches `ScreenPosToCelPos` (133 bytes), `SetIconLiftAndStuff` (93), and `PlaceDecor` (125). Their target-backed changes are listed above. No canonical source or shared header was edited.
- [EVIDENCE; confirmed-transferable, `VC5-SEM-01`]: In the decblock candidate with VC5 SP3 `/O2 /GX /ML /Gd /Gy`, changing the four guarded calls from `PlayImage(sound, POINT)` to `CSound::Play(sound, x, y)` reduced `AdaptMotorVehicleSound` from 144 to a 128-byte section and made the 123-byte target at `0x58d0` STRICT. Each call relocation resolves to the already-STRICT `CSound::Play` body at `0x23220`; no header delta was needed. Classification: Blupi-specific source/API correction.

## Current limits and next step

[EVIDENCE] The canonical manifest remains untouched by this worker. The refreshed `src/` candidates have 34 unique strict bodies across decblock (12), decblupi (7), decmove (11), and decdesign (4), totaling 3,403 bytes. Scratch `des_screen_local` adds three strict decdesign bodies (351 bytes), and `engine_adapt_sound` adds strict `AdaptMotorVehicleSound` (123 bytes), for a 38-body / 3,877-byte engine candidate set. The former zero-hit decblock result is superseded by the oracle-first table above. The 26-function block behavior maps to SB2 `decor.cpp` methods, but the assigned SB1 TU filename/source generation remains a hypothesis; no map edit is proposed.

[OPEN] `PlayPrepare` (768–784/703) and `GetBlupiInfo` (80/67) remain mismatches; `MoveObjectPollution` remains 480/439 (496 with a local `POINT`). `AdaptMotorVehicleSound` is STRICT in the scratch candidate. `StopSound` and `ChangePhase` behavior is clear from the oracle, but the merged `CSound` API still lacks their target direct-stop helper; their scratch attempts were discarded after compile errors. Constructor/Create/LoadImages/InitDecor/Build/DrawInfo/DecorNextAction and the tick/input body still need oracle-derived source. The CDecor allocation tail is sized but semantically unpartitioned; see the proposed opaque tail delta above. The `decblock`/`decdesign` boundary remains `0x5a20`, and no `oracle/tu_map.csv` change was made by this worker.
