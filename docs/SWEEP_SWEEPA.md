# Sweep A: unmatched functions

Verification used MSVC 5.0 SP3 (`/O2 /GX /ML`) with the `sweepa` build directory. The final run compiled the ten owned TU copies and left the accepted baseline intact. No header deltas remain.

## Function ledger

Target sizes are original oracle code bytes. `MISMATCH` means no strict candidate match; it does not imply the function was fully iterated. Generic `FUN_...` labels are Ghidra names where the seeded pseudocode did not recover a source name.

| RVA | Name | Status | Cause / notes |
|---|---|---|---|
| 0x2a30 | CButton::Create | MISMATCH (161 B) | Assignment-order probe produced 176 B and remained different; reverted with the other button edits. |
| 0x2b40 | CButton::Draw | MISMATCH (325 B) | Candidate remains 336 B; body not iterated beyond pseudocode/source comparison. |
| 0x2e10 | CButton::GetToolTips | MISMATCH (237 B) | Demo/submenu bounds reconstruction produced 240 B and regressed accepted `MouseMove`; reverted. |
| 0x2f70 | CButton::MouseDown | MISMATCH (84 B) | Early-guard rewrite remained 96 B; target calls the three-argument `CSound::PlayImage` body not available in this candidate TU. |
| 0x106b0 | FUN_004106b0 | MISMATCH (489 B) | SB2-derived body retained; no source-form iteration. |
| 0x144d0 | CDecor::AdaptBorder | MISMATCH (114 B) | Candidate instruction body spans the target range, but register allocation and block layout differ; hints are register/block-only. No semantic rewrite was justified. |
| 0x16220 | FUN_00416220 | MISMATCH (319 B) | SB2-derived body retained; no source-form iteration. |
| 0x164a0 | CEvent::Create | STRICT (90 B) | Swapping the independent `m_pNetwork` / `m_pMovie` stores matches the target; relocation to `ReadInfo` is proven. |
| 0x166b0 | CEvent::RestoreGame | STRICT (61 B) | Call sequence now matches `HideMouse(0)`, `WaitMouse(1)`, `WaitMouse(0)`, or `DisableMouse(1)`; all four call targets verified. |
| 0x16710 | FUN_00416710 | MISMATCH (479 B) | SB2-derived body retained; no source-form iteration. |
| 0x16a70 | FUN_00416a70 | MISMATCH (494 B) | SB2-derived body retained; no source-form iteration. |
| 0x16c60 | CEvent::ChangePause | STRICT (119 B) | Packet stores reordered to `data[0]`, `data[1]`, `data[2]`; all call targets proven. |
| 0x16d30 | CEvent::NetSearchPlayer | STRICT (43 B) | Added the `bIsPresent` guard and wrote the equality with `dpid` on the left to match the native compare. |
| 0x16d60 | FUN_00416d60 | MISMATCH (230 B) | SB2-derived body retained; no source-form iteration. |
| 0x16e50 | CEvent::NetSendBarePacket | STRICT (48 B) | A local four-byte packet struct matches the native stack layout; `Send` relocation is proven. |
| 0x18c70 | CEvent::MousePosToSprite | MISMATCH (123 B) | A nested phase guard collapsed to the same `je` layout; target has `jne` followed by `jmp` at `WM_PHASE_BYE`. |
| 0x191b0 | CEvent::GetWorld | STRICT (44 B) | Corrected `m_multi` displacement from `+200` to `+100`; exact bytes. |
| 0x19200 | CEvent::GetWorldGroup | MISMATCH (71 B) | Corrected the source to save the original mission before mutating it, as the oracle does; candidate remains 96 B. |
| 0x19290 | CEvent::CheckWorld1 | MISMATCH (113 B) | File/path logic agrees with pseudocode; candidate is 128 B, with branch/size differences still open. |
| 0x1ae30 | CEvent::IntroStep | MISMATCH (59 B) | Rewritten to the seeded decrement/check/phase/timer sequence; candidate is 64 B and still differs in branch layout. |
| 0x1ae70 | CEvent::MovieToStart | STRICT (77 B) | Exact code and verified calls to `HideMouse`, `StartMovie`, and `ChangePhase`. |
| 0x1aec0 | FUN_0041aec0 | MISMATCH (146 B) | SB2-derived body retained; no source-form iteration. |
| 0x1af60 | CEvent::CurrentRead | MISMATCH (87 B) | Oracle reuses the rank parameter as the private-flag output while preserving its original value; the source-level alias probe remains 96 B. |
| 0x1b090 | FUN_0041b090 | MISMATCH (181 B) | SB2-derived body retained; no source-form iteration. |
| 0x1b150 | FUN_0041b150 | MISMATCH (195 B) | SB2-derived body retained; no source-form iteration. |
| 0x1c0b0 | CEvent::StartMovie | MISMATCH (349 B) | Pseudocode shows movie checks, audio stop/cache, path append, and playback; no source-form iteration. |
| 0x1c280 | FUN_0041c280 | MISMATCH (413 B) | SB2-derived body retained; no source-form iteration. |
| 0x1c420 | FUN_0041c420 | MISMATCH (475 B) | SB2-derived body retained; no source-form iteration. |
| 0x1c6f0 | FUN_0041c6f0 | MISMATCH (230 B) | SB2-derived body retained; no source-form iteration. |
| 0x1c7e0 | FUN_0041c7e0 | MISMATCH (447 B) | SB2-derived body retained; no source-form iteration. |
| 0x1ca00 | FUN_0041ca00 | MISMATCH (188 B) | SB2-derived body retained; no source-form iteration. |
| 0x1cb40 | CEvent::ChatPop | MISMATCH (76 B) | Corrected the shift direction and copied each trailing NUL; candidate is 80 B and still differs in loop/register layout. |
| 0x1cb90 | CEvent::ChatPush | MISMATCH (123 B) | Candidate remains 144 B; no source-form iteration beyond pseudocode/source comparison. |
| 0x1cd70 | FUN_0041cd70 | MISMATCH (389 B) | SB2-derived body retained; no source-form iteration. |
| 0x1cf00 | CEvent::TextInputDelete | STRICT (83 B) | Indexed copy loop plus positive guard matches; candidate padding is excluded from unique original bytes. |
| 0x1f210 | FUN_0041f210 | MISMATCH (285 B) | Planet Blupi-derived body retained; no source-form iteration. |
| 0x1f410 | FUN_0041f410 | MISMATCH (324 B) | Planet Blupi-derived body retained; no source-form iteration. |
| 0x20cb0 | CNetwork::JoinSession | MISMATCH (237 B) | Candidate is 224 B; not iterated beyond source/pseudocode comparison. |
| 0x20dd0 | CNetwork::CreateSession | MISMATCH (234 B) | Candidate is 224 B; not iterated beyond source/pseudocode comparison. |
| 0x20ec0 | CNetwork::Send | MISMATCH (77 B) | A local `bGuaranteed` probe was optimized away; candidate stayed 80 B and the probe was reverted. |
| 0x20fe0 | CNetwork::Close | MISMATCH (18 B) | Guard-return form emitted the same 16-byte body as the original candidate; no code-shape change. |
| 0x22940 | CSound::CreateSoundBuffer | RELOC-UNVERIFIED (206 B) | Candidate code is 208 B; its `__FILE__` data names `scratch\sweepa\sound.cpp` instead of the oracle path `F:\Profil\sound.cpp`. |
| 0x22d60 | CSound::Create | MISMATCH (82 B) | All fixed bytes match. Candidate has a direct `REL32 _DirectSoundCreate@12` call to thunk 0x23890; verifier instead expects IAT 0x42c014 for this named import. |
| 0x236b0 | FUN_004236b0 | MISMATCH (144 B) | `DrawCharSingle` argument-width probe made this target strict but broke accepted `DrawTextA`; reverted to the canonical `char` signature. |

New strict coverage is 565 unique original bytes: `CEvent::Create` 90 B, `RestoreGame` 61 B, `ChangePause` 119 B, `NetSearchPlayer` 43 B, `NetSendBarePacket` 48 B, `GetWorld` 44 B, `MovieToStart` 77 B, and `TextInputDelete` 83 B. Alignment padding, folded functions, and ambiguous duplicates are excluded.

## Compiler findings

- **`CEvent::Create`, trigger: independent store order.** The oracle stores `m_pNetwork` before `m_pMovie`; source had the reverse. Reordering only those independent stores produced STRICT equality (90 unique bytes). **VC5-ORD-01; blupi-specific.**
- **`CEvent::ChangePause`, trigger: packet-byte store order.** Reordered the independent stores to size, message, pause byte; emitted STRICT equality (119 unique bytes). **VC5-ORD-01; blupi-specific.**
- **`CEvent::NetSearchPlayer`, trigger: missing presence condition and compare operand order.** Added the native `bIsPresent` check and expressed `dpid == player.dpid`; produced STRICT equality (43 unique bytes). **VC5-SEM-01; blupi-specific.**
- **`CEvent::NetSendBarePacket`, trigger: outgoing call stack layout.** A local `{BYTE, BYTE, short}` packet struct replaced the four-byte char array and produced STRICT equality (48 unique bytes). **VC5-ORD-01, VC5-LOC-02 (local-layout cue, transferred unverified); blupi-specific.**
- **`CEvent::TextInputDelete`, trigger: loop body and branch layout.** A direct indexed loop emitted the oracle's reload/store/test body; changing the guard from early-false to a positive block moved the false return to the target tail and produced STRICT equality (83 unique bytes). **VC5-CTL-01, BLUPI-REG-01; blupi-specific.**
- **`CEvent::MousePosToSprite`, trigger: `WM_PHASE_BYE` branch layout.** Splitting the phase checks into a nested guard compiled back to the same `je`; the target keeps `jne` plus `jmp`. **VC5-CTL-01; negative/inert on this probe.**
- **`CEvent::CurrentRead`, trigger: branch flip and parameter lifetime.** The oracle pseudocode uses the rank parameter as the private-flag output after saving the original rank. The equivalent alias probe remained 96 B versus the 87 B target. **VC5-CTL-01; negative on this probe.**
- **`CEvent::ChatPop`, trigger: loop direction and terminator handling.** The oracle shifts slots 0 through 4 upward and copies the NUL; the old descending `memcpy(strlen)` source did neither. Corrected behavior now compiles to 80 B versus 76 B, with register/loop layout still different. **VC5-CTL-01, BLUPI-REG-01; blupi-specific negative for exact matching.**
- **`CNetwork::Send`, trigger: 80 B candidate versus 77 B target.** A local `BOOL bGuaranteed = dwFlags != 0` was optimized away and left the object code unchanged; the source probe was reverted. **VC5-LOC-02; negative/inert.**
- **`CNetwork::Close`, trigger: control-flow size mismatch.** Guard-return and equivalent boolean-return forms all emitted a 16-byte body versus the 18-byte oracle; the scratch TU was restored. **VC5-CTL-03; negative.**
- **`CDecor::AdaptBorder`, trigger: same-sized body with register/block differences.** Hints show register allocation, block reordering, and displacements; no source-semantic probe was justified. **VC5-CTX-01; open/unverified.**

## Import-call and header evidence

- **[EVIDENCE]** The compile command adds `C:\tools\dx6sdk\INCLUDE` with `/I`. That exact `dsound.h` declares `DirectSoundCreate` as `extern HRESULT WINAPI` at line 196, without `__declspec(dllimport)`; VC5's bundled `dsound.h` line 107 is also plain `extern`. The pinned DX6 `ddraw.h` and `DPLAY.H` likewise declare `DirectDrawCreate`, `DirectPlayEnumerateA`, and `DirectPlayCreate` without `dllimport`. This local header evidence differs from the follow-up hint's stated DX6 declaration.
- **[EVIDENCE]** Oracle `CSound::Create` calls 0x23890, whose thunk is `jmp [0x42c014]` (`DSOUND!DirectSoundCreate`). Every candidate non-relocation byte matches; the COFF relocation is direct `REL32 _DirectSoundCreate@12`, but `verify.py` reports MISMATCH because its named-import resolver expects the IAT slot 0x42c014.
- **[EVIDENCE]** Oracle `CPixmap::Create` calls thunk 0x23896 (`jmp [0x42c000]`, `DDRAW!DirectDrawCreate`). Matched `CNetwork::EnumProviders` and `CreateProvider` call thunks 0x2389c and 0x238a2; their candidate COFF objects use direct `REL32` relocations to `_DirectPlayEnumerateA@8` and `_DirectPlayCreate@12`, and the verifier proves those thunk targets. The corresponding DPLAYX imports are ordinal #2 and #1.
- **[HYPOTHESIS]** The project used plain function declarations/import-library thunks for DirectSound, DirectDraw, and DirectPlay. The exact pinned headers already have that declaration style, so no local declaration or header delta is needed. Do not classify `CSound::Create` as STRICT until the candidate relocation is accepted by the verifier.

## Additional TU triage

- **[EVIDENCE]** No unaccepted `blupi.cpp` target is below 673 B (the other is 2,801 B), so no cheap Blupi target remained. The pending `pixmap.cpp` targets are 770 B and 902 B; `CPixmap::Create` was inspected for the DirectDraw thunk call but not rewritten. The tiny pixmap functions checked in the current source are already accepted.

## Header deltas

None. All private headers in `scratch/sweepa` match the copied canonical headers.
