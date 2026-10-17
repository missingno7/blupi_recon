# Sweep MIDA: mid-size non-engine functions

`[EVIDENCE]` Candidates are isolated under `scratch/mida`; the final full-manifest run used VC5 SP3 and `tools/verify.py --build-dir mida`. It preserved the accepted baseline and reports 42,096 strict/folded game bytes. Six owned functions became STRICT: 2,550 oracle body bytes total. Their emitted object spans total 2,576 bytes, including 26 exact alignment bytes.

## Function ledger

Candidate size is shown before target body size. `STRICT` spans can include matching alignment bytes after the oracle function body; the body byte count is the target size from `oracle/functions.csv`.

| RVA | Function | Status | Candidate / target | Prefix or size trend; cause |
|---:|---|---|---:|---|
| 0x1030 | ReadConfig | MISMATCH | 688 / 673 | One fixed byte remains at `+0x89`: target SIB `06`, candidate `30`. `pText[nb]` is codegen-inert; wide end/top/before context probes did not flip it. |
| 0x2b40 | CButton::Draw | MISMATCH | 336 / 325 | Candidate is 11 bytes larger; first difference is branch layout. No source iteration. |
| 0x106b0 | CDecor::ObjectStart | MISMATCH | 480 / 489 | Candidate is 9 bytes shorter; call/register and SIB differences remain around `MoveObjectStepIcon`. |
| 0x140c0 | CDecor::IsRightBorder | MISMATCH | 784 / 775 | Target cell access is `[x][y]`; target-backed correction and short-local probe remain unmatched. See `DECOR_TU.md`. |
| 0x14550 | CDecor::DrawMap | MISMATCH | 800 / 805 | 311 fixed bytes differ, first at `+0x1c1`. Target pseudocode uses a 522x400 map viewport; candidate now follows those bounds and target-backed local/store order. |
| 0x14af0 | CDecor::Read | MISMATCH | 608 / 610 | Current SB2 guard form is two bytes shorter; target version-guard variant was 624 bytes. Neither matched. |
| 0x153d0 | CDecor::CurrentRead | MISMATCH | 1,584 / 1,564 | Twenty bytes larger after target field stores; cleanup and validation branches differ. |
| 0x159f0 | CDecor::FileExist | MISMATCH | 544 / 540 | Four bytes larger; validation and shared cleanup layout differ. |
| 0x15e30 | CDecor::AdaptDoors | MISMATCH | 848 / 840 | Eight bytes larger in direct-index form; pointer-local form was 720 and also mismatched. |
| 0x16220 | CEvent constructor | STRICT | 320 / 319 | Exact body plus one alignment byte; initializes `m_saveIndex` and uses target store order. |
| 0x16710 | CEvent::ReadInput | MISMATCH | 480 / 479 | One byte larger; previously tested mask/temp/context variants remain negative. |
| 0x16a70 | CEvent::NetAdjustLobbyButtons | MISMATCH | 512 / 494 | 210 fixed bytes differ, first at `+0x55`; target-shaped post-increment loop condition reduced fixed differences 256 -> 210. Explicit true/false call branches regressed to 327 differences and were reverted. |
| 0x18ea0 | CEvent::EventButtons | STRICT | 560 / 547 | Exact body plus 13 alignment bytes after replacing cursor polling with `ConvLongToPos(lParam)`. |
| 0x1bd50 | `[HYPOTHESIS]` CEvent::BuildDown | MISMATCH | 1,072 / 799 | Target body handles build/delete state and decor cells; inherited candidate is 273 bytes larger. |
| 0x1c0b0 | CEvent::StartMovie | STRICT | 352 / 349 | Exact body plus three alignment bytes; target return predicate and four-argument audio call now match. |
| 0x1c280 | CEvent::WriteInfo | STRICT | 416 / 413 | Exact body plus three alignment bytes after field assignment reordering. |
| 0x1c420 | CEvent::ReadInfo | STRICT | 480 / 475 | Exact body plus five alignment bytes after assignment reordering and removal of an extra decor setter call. |
| 0x1c7e0 | CEvent::DemoPlayStart | STRICT | 448 / 447 | Exact body plus one alignment byte after correcting demo numbering, open-failure flow, and music path formatting. |
| 0x1cd70 | `[HYPOTHESIS]` CEvent::PutTextInputBox | MISMATCH | 400 / 389 | 311 fixed bytes differ, first at `+0x26`; target uses `neg` + `add`, while equivalent unary-negation and split-assignment forms still compile to `sub`. |
| 0x1cf60 | `[HYPOTHESIS]` CEvent::TreatTextInput | MISMATCH | 176 / 586 | Target pseudocode handles `WM_CHAR`/`WM_KEYDOWN`; candidate body is much smaller and remains unresolved. |
| 0x1f410 | AddCDPath | MISMATCH | 336 / 324 | Twelve bytes larger; storage/register order remains open. |
| 0x21190 | CPixmap::Create | MISMATCH | 912 / 902 | Ten bytes larger; first prologue allocation differs (`xor ebx` versus target `push edi`); current member offsets are retained. |
| 0x21b20 | `[HYPOTHESIS]` CPixmap clipped icon blit | MISMATCH | 800 / 770 | Thirty bytes larger; table read order and inline channel-helper probes remain negative. |

## Compiler findings

- `[EVIDENCE; confirmed Blupi-specific; VC5-SEM-01, VC5-ORD-01]` `CEvent::CEvent`: target pseudocode and member accesses show `m_saveIndex=-1` at `+0x6d10`. Adding the missing initialization and ordering independent field stores, including the `m_bDisableMouse`/`m_bShowMouse` stores, produced STRICT equality (319 body bytes; 320-byte emitted span).
- `[EVIDENCE; confirmed Blupi-specific; VC5-SEM-01]` `CEvent::EventButtons`: target calls `ConvLongToPos(lParam)` at `0x18ea0`; inherited source polled the cursor with `GetMousePos()`. The target-backed source correction produced STRICT equality (547 body bytes; 560-byte emitted span), with the call relocation verified.
- `[EVIDENCE; confirmed Blupi-specific; VC5-SEM-01]` `CEvent::StartMovie`: target returns false when `CMovie::Play` returns zero, then calls the four-argument playback helper at `0x22f00`. Reversing the inherited return test and calling `PlaySoundDS(SOUND_MOVIE,0,0,0)` directly produced STRICT equality (349 body bytes; 352-byte emitted span), with relocations proven.
- `[EVIDENCE; confirmed Blupi-specific; VC5-SEM-01]` `CEvent::DemoPlayStart`: target formats the demo path with `m_demoNumber+200`, uses the same offset for the mission, returns directly on failed `fopen`, and formats music with `music-1` before the string `PlayMusic` overload. These target-backed corrections produced STRICT equality (447 body bytes; 448-byte emitted span).
- `[EVIDENCE; confirmed Blupi-specific; VC5-ORD-01]` `CEvent::WriteInfo`: changing only the order of independent `DescInfo` assignments aligned target loads/stores. STRICT equality is 413 body bytes in a 416-byte emitted span.
- `[EVIDENCE; confirmed Blupi-specific; VC5-ORD-01, VC5-SEM-01]` `CEvent::ReadInfo`: target assignment order and absence of a `SetBuildOfficialMissions` call are visible in the pseudocode. Reordering field assignments and removing the extra setter produced STRICT equality (475 body bytes; 480-byte emitted span).
- `[EVIDENCE; negative/incomplete; VC5-ORD-01, VC5-SEM-01]` `NetAdjustLobbyButtons`: changing the target-shaped do-while tail from explicit `button++` plus `button < WM_BUTTON11 + MAXNETPLAYER` to `button++ - WM_BUTTON10 < MAXNETPLAYER` reduced fixed differences 256 -> 210 with the 512-byte body and retained the first mismatch at `+0x55`. Duplicating `SetEnable` in true/false branches regressed to 327 fixed differences (first `+0x2a`) and was reverted. No strict match or general compiler rule is inferred.
- `[EVIDENCE; negative/incomplete; VC5-SEM-01, VC5-ORD-01]` `PutTextInputBox`: target pseudocode leaves `m_textHiliEnd` unchanged and advances the draw x-coordinate by the selected-text width. Those behavior corrections reduced fixed differences 314 -> 311. Both `pos.x + (-(lg/2))` and a split `posD = -(lg/2); posD += pos.x;` compile to the same `sub ebx,eax` bytes; target uses `neg eax; add ebx,eax`. No strict match or new rule is inferred.
- `[EVIDENCE; negative; BLUPI-REG-01, BLUPI-NEG-02]` `ReadConfig`: the target-backed spelling `pText[nb] = *pText` and the earlier commuted addition spelling leave the one-byte SIB residue at `+0x89` unchanged (`candidate 0x30`, target `0x06`). Wide scratch-only VC5 SP3 context probes used 93 end counts (0-64 by 1, then 64-512 by 16), 65 top counts, and 65 before-function counts (all 0-64); every probe stayed 688 bytes with one fixed difference at `+0x89`. TU count did not flip the SIB; no other target-backed block-shape difference was found, so this source line is left as the target pseudocode's indexed form.
- `[EVIDENCE; negative/incomplete; VC5-ORD-01, VC5-SEM-01]` `CDecor::DrawMap`: target evidence gives a horizontal center offset of 261 and clamp bounds for a 522x400 map, while the inherited source used the 640x480 window dimensions. Following the target local-save order, `i == team`, zeroing the network blupi seconds before the icon assignment, and the 522x400 centering/clamp arithmetic improved the body from 800 bytes with 343 fixed differences (first `+0x19`) to 800 bytes with 311 (first `+0x1c1`). The target pixmap call resolves to the strict `CPixmap::DrawMap(int, RECT, RECT)` at `0x217a0`; Ghidra omits the by-value RECT parameters in its call rendering. The function remains MISMATCH; no compiler rule is inferred.

## Header deltas

`[EVIDENCE]` The only private-header difference is `scratch/mida/sound.h`: `friend class CEvent;` grants access to the existing protected four-argument `CSound::PlaySoundDS` declaration used by the target `StartMovie` call. This changes no layout. The other 18 copied headers match `src/` byte for byte. No canonical header was edited.

## Open work

`[OPEN]` Seventeen of the 23 owned functions remain mismatches. Their current sizes and documented negatives are in the ledger; no further probe was justified for the broad control-flow/register mismatches in this pass. The next useful targets are the one-byte `ReadConfig` SIB residue and the decor/pixmap cases with small size gaps, after the existing cluster notes are consulted.
