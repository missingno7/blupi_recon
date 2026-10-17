# Sweep MIDA: mid-size non-engine functions

`[EVIDENCE]` Candidates are isolated under `scratch/mida`; the final full-manifest run used VC5 SP3 and `tools/verify.py --build-dir mida`. It preserved the accepted baseline and reports 42,096 strict/folded game bytes. Six owned functions became STRICT: 2,550 oracle body bytes total. Their emitted object spans total 2,576 bytes, including 26 exact alignment bytes.

## Function ledger

Candidate size is shown before target body size. `STRICT` spans can include matching alignment bytes after the oracle function body; the body byte count is the target size from `oracle/functions.csv`.

| RVA | Function | Status | Candidate / target | Prefix or size trend; cause |
|---:|---|---|---:|---|
| 0x1030 | ReadConfig | MISMATCH | 688 / 673 | One fixed byte remains at `+0x89`: SIB base/index order. Reordered addition probe emitted the same bytes. |
| 0x2b40 | CButton::Draw | MISMATCH | 336 / 325 | Candidate is 11 bytes larger; first difference is branch layout. No source iteration. |
| 0x106b0 | CDecor::ObjectStart | MISMATCH | 480 / 489 | Candidate is 9 bytes shorter; call/register and SIB differences remain around `MoveObjectStepIcon`. |
| 0x140c0 | CDecor::IsRightBorder | MISMATCH | 784 / 775 | Target cell access is `[x][y]`; target-backed correction and short-local probe remain unmatched. See `DECOR_TU.md`. |
| 0x14550 | CDecor::DrawMap | MISMATCH | 800 / 805 | Five bytes shorter; target calls an unmapped one-argument pixmap helper after `Build(0,0,640,480)`. |
| 0x14af0 | CDecor::Read | MISMATCH | 608 / 610 | Current SB2 guard form is two bytes shorter; target version-guard variant was 624 bytes. Neither matched. |
| 0x153d0 | CDecor::CurrentRead | MISMATCH | 1,584 / 1,564 | Twenty bytes larger after target field stores; cleanup and validation branches differ. |
| 0x159f0 | CDecor::FileExist | MISMATCH | 544 / 540 | Four bytes larger; validation and shared cleanup layout differ. |
| 0x15e30 | CDecor::AdaptDoors | MISMATCH | 848 / 840 | Eight bytes larger in direct-index form; pointer-local form was 720 and also mismatched. |
| 0x16220 | CEvent constructor | STRICT | 320 / 319 | Exact body plus one alignment byte; initializes `m_saveIndex` and uses target store order. |
| 0x16710 | CEvent::ReadInput | MISMATCH | 480 / 479 | One byte larger; previously tested mask/temp/context variants remain negative. |
| 0x16a70 | CEvent::NetAdjustLobbyButtons | MISMATCH | 512 / 494 | 256 fixed bytes differ, first at `+0x55`; target-shaped byte offset/do-while and enable call are present, but register and branch layout remain different. |
| 0x18ea0 | CEvent::EventButtons | STRICT | 560 / 547 | Exact body plus 13 alignment bytes after replacing cursor polling with `ConvLongToPos(lParam)`. |
| 0x1bd50 | `[HYPOTHESIS]` CEvent::BuildDown | MISMATCH | 1,072 / 799 | Target body handles build/delete state and decor cells; inherited candidate is 273 bytes larger. |
| 0x1c0b0 | CEvent::StartMovie | STRICT | 352 / 349 | Exact body plus three alignment bytes; target return predicate and four-argument audio call now match. |
| 0x1c280 | CEvent::WriteInfo | STRICT | 416 / 413 | Exact body plus three alignment bytes after field assignment reordering. |
| 0x1c420 | CEvent::ReadInfo | STRICT | 480 / 475 | Exact body plus five alignment bytes after assignment reordering and removal of an extra decor setter call. |
| 0x1c7e0 | CEvent::DemoPlayStart | STRICT | 448 / 447 | Exact body plus one alignment byte after correcting demo numbering, open-failure flow, and music path formatting. |
| 0x1cd70 | `[HYPOTHESIS]` CEvent::PutTextInputBox | MISMATCH | 400 / 389 | 311 fixed bytes differ, first at `+0x26`; target and candidate arithmetic/control-flow layouts remain different after two behavior corrections. |
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
- `[EVIDENCE; negative/incomplete; BLUPI-LOC-01, BLUPI-REG-01, VC5-SEM-01]` `NetAdjustLobbyButtons`: the target advances a player byte offset by `0x20`, advances the button ID, and enables the ready button for the local DPID. The pointer-walk candidate fell from 496 to 480 bytes before the missing enable call was restored; the semantically corrected candidate is 512 bytes with 256 fixed differences. No strict match or general compiler rule is inferred.
- `[EVIDENCE; negative/incomplete; VC5-SEM-01]` `PutTextInputBox`: target pseudocode leaves `m_textHiliEnd` unchanged and advances the draw x-coordinate by the selected-text width. Applying those behavior corrections reduced fixed differences from 314 to 311, but the 400-byte candidate remains different from the 389-byte target. The equivalent centering expression rewrite emitted unchanged code.
- `[EVIDENCE; negative/inert; VC5-CTX-01..04 as triage context only]` `ReadConfig`: changing the addition operand order did not alter the one-byte SIB residue at `+0x89`. This probe confirms no source-form effect in the current TU; no new context rule is inferred.

## Header deltas

`[EVIDENCE]` The only private-header difference is `scratch/mida/sound.h`: `friend class CEvent;` grants access to the existing protected four-argument `CSound::PlaySoundDS` declaration used by the target `StartMovie` call. This changes no layout. The other 18 copied headers match `src/` byte for byte. No canonical header was edited.

## Open work

`[OPEN]` Seventeen of the 23 owned functions remain mismatches. Their current sizes and documented negatives are in the ledger; no further probe was justified for the broad control-flow/register mismatches in this pass. The next useful targets are the one-byte `ReadConfig` SIB residue and the decor/pixmap cases with small size gaps, after the existing cluster notes are consulted.
