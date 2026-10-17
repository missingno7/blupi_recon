# Sweep B: unmatched engine functions

[EVIDENCE] Candidate work stayed in `scratch/sweepb/`; no canonical `src/`, `assets/`, or `external/` files were edited. Final verification compiled all four candidate TUs with `tools/verify.py --build-dir sweepb`; output is in `build/verify-sweepb/results.json`.

[EVIDENCE] Forty requested target functions are strict, covering 1,431 unique oracle bytes. The two tile-mask helpers are strict through the integrated payload-verified table binding in the default verifier.

## Per-function results

| TU | RVA | Name | Oracle bytes | Status | Cause / notes |
|---|---:|---|---:|---|---|
| `decblock.cpp` | `0x3910` | `CDecor::CDecor` | 158 | **STRICT** | Constructor code and CJauge array-constructor relocation match. |
| `decblock.cpp` | `0x39b0` | `CDecor::~CDecor` | 22 | **STRICT** | Array-member destructor call matches. |
| `decblock.cpp` | `0x39d0` | `CDecor::Create` | 196 | **MISMATCH** | SB2-derived candidate is 208 bytes; stores and member displacements differ. |
| `decblock.cpp` | `0x3aa0` | `CDecor::LoadImages` | 183 | **OPEN** | Target selects `image16\decor%.3d.blp` then `image08\decor%.3d.blp`; the private game-only getter declaration compiles, but the 192-byte candidate still differs in call/branch layout. |
| `decblock.cpp` | `0x41d0` | `CDecor::MoveStep` | 261 | **MISMATCH** | 272-byte SB2-derived body; y-limit predicate was corrected from >0 to !=0, but displacement/load differences remain. |
| `decblock.cpp` | `0x5670` | `FUN_00405670` | 269 | **OPEN** | Scroll/interpolation routine; target table identity and source method are not reconstructed. |
| `decblock.cpp` | `0x57b0` | `CDecor::GetBlupiInfo` | 67 | **STRICT** | Refreshed canonical body matches the full candidate span at this target; no relocations. |
| `decblock.cpp` | `0x5880` | `CDecor::StopSound` | 68 | **STRICT** | Sequential flag clears match; `CSound::StopSound` relocation is verified at VA `0x00423050`. |
| `decblock.cpp` | `0x5950` | `CDecor::ChangePhase` | 208 | **MISMATCH** | SB2-derived candidate is 208 bytes but target bytes still differ. |
| `decdesign.cpp` | `0x8340` | `CDecor::PlaceDecor7 [hypothesis]` | 478 | **MISMATCH** | Inherited placement body is 1376 bytes; target has a three-action placement path. |
| `decdesign.cpp` | `0x8560` | `CDecor::GetRegion` | 7 | **STRICT** | Getter/setter cluster matches with proven field offsets. |
| `decdesign.cpp` | `0x8570` | `CDecor::SetRegion` | 13 | **STRICT** | Getter/setter cluster matches with proven field offsets. |
| `decdesign.cpp` | `0x8580` | `CDecor::GetMusic` | 7 | **STRICT** | Getter/setter cluster matches with proven field offsets. |
| `decdesign.cpp` | `0x8590` | `CDecor::SetMusic` | 13 | **STRICT** | Getter/setter cluster matches with proven field offsets. |
| `decdesign.cpp` | `0x85a0` | `CDecor::GetDim` | 24 | **STRICT** | Getter/setter cluster matches with proven field offsets. |
| `decdesign.cpp` | `0x85c0` | `CDecor::SetDim` | 23 | **STRICT** | Getter/setter cluster matches with proven field offsets. |
| `decdesign.cpp` | `0x85e0` | `CDecor::GetNbVies` | 7 | **STRICT** | Getter/setter cluster matches with proven field offsets. |
| `decdesign.cpp` | `0x85f0` | `CDecor::SetNbVies` | 13 | **STRICT** | Getter/setter cluster matches with proven field offsets. |
| `decdesign.cpp` | `0x8600` | `CDecor::GetPause` | 7 | **STRICT** | Getter/setter cluster matches with proven field offsets. |
| `decdesign.cpp` | `0x8610` | `CDecor::ChangePause` | 13 | **STRICT** | Getter/setter cluster matches with proven field offsets. |
| `decdesign.cpp` | `0x8620` | `CDecor::GetDoors` | 35 | **STRICT** | Accessor matches; four-byte loop bounds and door field layout proven. |
| `decdesign.cpp` | `0x8650` | `CDecor::SetDoors` | 35 | **STRICT** | Accessor matches; four-byte loop bounds and door field layout proven. |
| `decdesign.cpp` | `0x8680` | `CDecor::SetAllMissions` | 32 | **STRICT** | Code and AdaptDoors relocation match. |
| `decdesign.cpp` | `0x8d40` | `CDecor::GetSuperBlupi` | 7 | **STRICT** | Getter/setter cluster matches with proven field offsets. |
| `decdesign.cpp` | `0x8d50` | `CDecor::SetSuperBlupi` | 13 | **STRICT** | Getter/setter cluster matches with proven field offsets. |
| `decdesign.cpp` | `0x8d60` | `CDecor::GetDrawSecret` | 7 | **STRICT** | Getter/setter cluster matches with proven field offsets. |
| `decdesign.cpp` | `0x8d70` | `CDecor::SetDrawSecret` | 13 | **STRICT** | Getter/setter cluster matches with proven field offsets. |
| `decdesign.cpp` | `0x8d80` | `CDecor::GetNetPacked` | 7 | **STRICT** | Getter/setter cluster matches with proven field offsets. |
| `decdesign.cpp` | `0x8d90` | `CDecor::SetNetPacked` | 13 | **STRICT** | Getter/setter cluster matches with proven field offsets. |
| `decdesign.cpp` | `0x8da0` | `CDecor::GetNetMovePredict` | 7 | **STRICT** | Getter/setter cluster matches with proven field offsets. |
| `decdesign.cpp` | `0x8db0` | `CDecor::SetNetMovePredict` | 13 | **STRICT** | Getter/setter cluster matches with proven field offsets. |
| `decdesign.cpp` | `0x8dc0` | `CDecor::GetNetDebug` | 7 | **STRICT** | New target field accessor matches at +0xcdc4. |
| `decdesign.cpp` | `0x8dd0` | `CDecor::SetNetDebug` | 30 | **STRICT** | Field store and remove("debug.txt") relocation match. |
| `decdesign.cpp` | `0x8df0` | `CDecor::OutputNetDebug` | 112 | **STRICT** | Formatting code and string/call relocations match. |
| `decdesign.cpp` | `0x8e60` | `CDecor::SetMulti` | 13 | **STRICT** | Getter/setter cluster matches with proven field offsets. |
| `decdesign.cpp` | `0x8e70` | `CDecor::SetTeam` | 13 | **STRICT** | Getter/setter cluster matches with proven field offsets. |
| `decmove.cpp` | `0x10a00` | `CDecor::NetDataFlush` | 86 | **MISMATCH** | Target-directed field loop reduced the candidate to 96 bytes; target code still differs. |
| `decmove.cpp` | `0x10a60` | `CDecor::NetSendBarePacket` | 45 | **STRICT** | Local size/type/number packet fields produce an exact candidate span; the `CNetwork::Send` call relocation resolves to `0x420ec0`. |
| `decmove.cpp` | `0x110a0` | `CDecor::DoNetSmooth` | 187 | **MISMATCH** | SB2-derived candidate is 208 bytes. |
| `decmove.cpp` | `0x11160` | `CDecor::NetAdjustToLift` | 72 | **MISMATCH** | Candidate is 80 bytes. |
| `decmove.cpp` | `0x111b0` | `CDecor::NetAscenseurSynchro` | 259 | **MISMATCH** | Candidate is 128 bytes; target synchronization body is larger. |
| `decmove.cpp` | `0x112c0` | `CDecor::NetPlayerCollide` | 217 | **MISMATCH** | Candidate is 176 bytes; collision return and hidden-player path differ. |
| `decmove.cpp` | `0x113a0` | `CDecor::NetMessageIndexFlush` | 21 | **STRICT** | Three index stores match. |
| `decmove.cpp` | `0x113c0` | `CDecor::NetMessagePush` | 96 | **STRICT** | Queue bound, copy, and wraparound code match. |
| `decmove.cpp` | `0x11420` | `CDecor::NetMessagePop` | 97 | **STRICT** | Queue empty check, copy, and wraparound code match. |
| `decmove.cpp` | `0x11490` | `CDecor::NotifFlush` | 31 | **STRICT** | Notification clear loop and timer store match. |
| `decmove.cpp` | `0x114b0` | `CDecor::NotifPop` | 86 | **STRICT** | Three-string shift, final clear, and timer store match. |
| `decmove.cpp` | `0x11510` | `CDecor::NotifPush` | 130 | **MISMATCH** | Candidate is 144 bytes; target and candidate branch counts differ. |
| `decmove.cpp` | `0x115a0` | `CDecor::NotifStep` | 23 | **STRICT** | Timer/expiry branch matches. |
| `decmove.cpp` | `0x12c20` | `CDecor::AscenseurDetect` | 324 | **MISMATCH** | Candidate is 416 bytes; intersection path differs. |
| `decmove.cpp` | `0x12d70` | `CDecor::AscenseurVertigo` | 96 | **MISMATCH** | Candidate is 176 bytes. |
| `decmove.cpp` | `0x13020` | `CDecor::SearchLinkCaisse [hypothesis]` | 453 | **MISMATCH** | Candidate is 464 bytes. |
| `decmove.cpp` | `0x13540` | `CDecor::BlupiElectro [hypothesis]` | 151 | **MISMATCH** | Candidate is 176 bytes. |
| `decmove.cpp` | `0x13800` | `CDecor::MovePersoDetect [hypothesis]` | 159 | **MISMATCH** | Candidate is 224 bytes; target only tests a subset of move-object types. |
| `decmove.cpp` | `0x138a0` | `FUN_004138a0` | 190 | **OPEN** | Collision query over a restricted type range; no unique source-method identity established. |
| `decmove.cpp` | `0x13960` | `CDecor::MoveChargeDetect [hypothesis]` | 208 | **MISMATCH** | Candidate is 176 bytes; target type set and collision box differ. |
| `decmove.cpp` | `0x13b20` | `CDecor::MoveObjectSort` | 234 | **MISMATCH** | Candidate is 256 bytes; target sort/copy loop differs. |
| `decmove.cpp` | `0x13c10` | `CDecor::MoveObjectPriority` | 160 | **MISMATCH** | Candidate is 160 bytes; `MoveObjectCopy` inline-helper variant still mismatches. |
| `decblupi.cpp` | `0xfc10` | `FUN_0040fc10` | 439 | **OPEN** | Progression/action selector; current source identity is not established. |
| `decblupi.cpp` | `0xff90` | `CDecor::IsWorld [hypothesis]` | 184 | **MISMATCH** | Candidate IsWorld is 272 bytes. |
| `decblupi.cpp` | `0x10050` | `CDecor::GetTypeBarre` | 248 | **MISMATCH** | Candidate is 256 bytes; raw-coordinate rewrite preserved size and did not match. |
| `decblupi.cpp` | `0x10150` | `CDecor::IsLave` | 88 | **MISMATCH** | Candidate is 96 bytes; KB tags displacement difference. |
| `decblupi.cpp` | `0x101b0` | `CDecor::IsRessort` | 94 | **MISMATCH** | Candidate is 96 bytes. |
| `decblupi.cpp` | `0x10210` | `CDecor::IsSurfWater` | 115 | **STRICT** | Caching the upper cell icon before the lower-cell short-circuit test matches the full candidate span at this target. |
| `decblupi.cpp` | `0x102e0` | `CDecor::IsOutWater` | 75 | **MISMATCH** | Candidate is 96 bytes. |
| `decblupi.cpp` | `0x10330` | `CDecor::IsPassIcon` | 63 | **STRICT** | All bytes match after cmpfunc maps table_decor_quart to VA 0x00431aa0, evidenced by Ghidra DAT_00431aa0. |
| `decblupi.cpp` | `0x10370` | `CDecor::IsBlocIcon` | 55 | **STRICT** | All bytes match after cmpfunc maps table_decor_quart to VA 0x00431aa0, evidenced by Ghidra DAT_00431aa0. |
| `decblupi.cpp` | `0x103b0` | `CDecor::SearchDistRight` | 200 | **MISMATCH** | Candidate is 272 bytes. |
| `decblupi.cpp` | `0x10480` | `CDecor::IsVentillo` | 486 | **MISMATCH** | Candidate is 560 bytes. |

## Header deltas

- `scratch/sweepb/decor.h` matches integrated `src/decor.h`; the prior private field-layout delta is now in the canonical header.
- `scratch/sweepb/pixmap.h` adds a game-build declaration for `CPixmap::GetTrueColorDecor()` after the event-only accessor block. This lets the private `LoadImages` candidate compile under `SB1_GAME_API`. The target calls a four-byte getter body at RVA `0x22dc0` through `CDecor::m_pPixmap`; the getter identity is supported by the current header and call receiver, but `LoadImages` remains mismatched, so no canonical header delta is proposed.

## Compiler findings

- `[VC5-SEM-01]` **Blupi-specific, confirmed:** removing the inherited mission-build flag and adding the target net-debug flag restores 26 strict `CDecor` accessors/debug methods (484 oracle bytes) plus strict constructor/destructor and `SetAllMissions` references. Evidence is the target field displacements and the full candidate-TU comparisons.
- `[VC5-SEM-01]` **Blupi-specific, confirmed:** `IsPassIcon` and `IsBlocIcon` are strict in the default verifier after integration of the payload-verified `table_decor_quart` binding at `0x00431aa0` (7,584 initialized bytes).
- `[BLUPI-LOC-01]` / `[VC5-SEM-01]` **Classification: blupi-specific, confirmed:** the byte-array and two-word packet candidates for `NetSendBarePacket` did not match. Under MSVC 5.0 SP3 `/O2 /GX /ML` in the `decmove.cpp` TU, a local `{ BYTE size; BYTE type; short num; }` payload with field assignments produced a 48-byte exact span at `0x10a60` (45-byte oracle function extent), STRICT; the `CNetwork::Send` relocation is strict at `0x420ec0`.
- `[VC5-ORD-01]` / `[VC5-SEM-01]` **Classification: blupi-specific, confirmed:** `IsSurfWater` stayed mismatched when the lower-cell icon load preceded the upper-cell load. Under MSVC 5.0 SP3 `/O2 /GX /ML` in the `decblupi.cpp` TU, caching the upper-cell icon before the short-circuit lower-cell test produced a 128-byte exact span at `0x10210` (115-byte oracle function extent), STRICT.
- `[VC5-ORD-01]` / `[BLUPI-LOC-01]` **Classification: negative:** under MSVC 5.0 SP3 `/O2 /GX /ML` in the `decblock.cpp` TU, `LoadImages` direct dimensions expressions and block-scoped `POINT` locals both emitted 192-byte candidates; moving those locals to function scope emitted 208 bytes. All differ from the 183-byte target. These scope/order variations did not recover its register setup or cache-result control flow.
- `[VC5-CTL-01]` / `[VC5-SEM-01]` **Negative:** `GetTypeBarre` was rewritten to retain input coordinates for the final rectangle instead of keeping a second `POINT`. It stayed 256 bytes against a 248-byte target and remained mismatched; KB tags branch and displacement differences.
- `[VC5-SEM-01]` **Negative/near miss:** target-directed `NetDataFlush` clearing only the observed arrays/counters reduced the candidate from 128 to 96 bytes, but the 86-byte target remains unmatched.
- `[VC5-INL-03]` **Negative:** replacing `MoveObjectPriority` structure assignments with the existing `MoveObjectCopy` inline helper kept a 160-byte candidate and remained mismatched at 160 target bytes; branch/return structure differs.
- `[VC5-SEM-01]` **Open:** the SB2-derived `MoveStep`, `Create`, `ChangePhase`, and network interpolation bodies were checked against target pseudocode but remain mismatched. No transferable compiler rule was inferred from those results.

## Verification notes

- Latest four-TU verification completed successfully in `build/verify-sweepb`; no accepted strict functions regressed. `Create`, `LoadImages`, `MoveStep`, `ChangePhase`, the remaining network/intersection methods, and the other listed terrain helpers remain mismatches or open source identities as shown above.
- The default verifier now proves `IsPassIcon` / `IsBlocIcon` against the integrated 7,584-byte initialized table payload at `0x00431aa0`; no explicit symbol mapping was needed.
- The relocation recheck keeps `StopSound` strict through `CSound::StopSound` at VA `0x00423050`; `NetSendBarePacket` is strict through `CNetwork::Send` at VA `0x00420ec0`.
- The three newly strict helpers compare full candidate spans of 80 bytes (`GetBlupiInfo`), 48 bytes (`NetSendBarePacket`), and 128 bytes (`IsSurfWater`) at their target RVAs. The oracle function extents recorded in the table remain 67, 45, and 115 bytes respectively.
