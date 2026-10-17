# Reconstructed source provenance

**[EVIDENCE]** Planet Blupi references are from https://github.com/blupi-games/planetblupi, tag v1.8.0, commit 203ccc8, licensed GPLv3+. SB2 reconstruction material is from external/sb2-decomp and is GPLv3. Reference repositories are read-only. Provenance stays here so source comments do not shift observable __LINE__ values.

**[EVIDENCE]** The current manifest accepts 220 STRICT functions and two FOLDED functions. A clean combined verify covers 28,160 unique bytes of the 142,064-byte game `.text` range; folded bytes are counted under their owner once. The latest verified function-address placement establishes `CButton::~CButton` as folded into `CMenu::~CMenu` at `0x1f010`.

## PB-derived core files

- sound.cpp: Planet Blupi v1.8.0-derived Windows DirectSound source, retaining the matched source line numbers and TRY_DS sites. The core candidate's CSound::StopSound body is strict at 0x23050, but is not in canonical source: adding its declaration to shared sound.h changes seven bytes in accepted CButton::MouseMove, so the prior header context is retained.
- movie.cpp and ddutil.cpp: Planet Blupi v1.8.0-derived Windows DirectDraw/AVI source; the core candidates were identical to the already accepted files.
- misc.cpp: Planet Blupi v1.8.0-derived utility source. Its active AddUserPath folder literal is changed to c:\Speedy Blupi\ to match the SB1 path data.
- sound.h, movie.h, misc.h, ddutil.h, and resource.h: Planet Blupi-derived declarations and support headers. sound.h remains without the candidate-only StopSound declaration to preserve the accepted UI TU context.

## Network TU integration

- network.cpp: SB2 reconstruction DirectPlay code (GPLv3), adjusted to SB1 evidence. `TraceErrorDP` uses the target error set/labels; `EnumSessions` initializes the descriptor fields in target store order; `Receive` compares the sender DPID with each player's stored DPID. Thirteen inventoried functions and both `CALLBACK` enumeration bodies are STRICT in the combined run; `JoinSession`, `CreateSession`, `Send`, and `Close` remain mismatches.
- network.h: SB2-derived types with SB1 member order established by target offsets: `m_pDP` at `+0`, three lists at `+4..+0x1c`, `m_bHost` at `+0x1c`, `m_dpid` at `+0x20`, and players at `+0x24`. Both enumeration callback declarations use `CALLBACK`, matching their `ret 0x14` and `ret 0x10` bodies.
- event.cpp: `CEvent::NetEnumSessions` calls `CNetwork::EnumSessions`. This corrects the candidate's stale `EnumProviders` call: the accepted body at `0x16920` relocates to `0x20bd0`, the verified `EnumSessions` entry. `CEvent::NetDraw` and `CEvent::DrawMap` compile to the same 48-byte body at `0x16e80`; accept it once under `NetDraw`, with `DrawMap` excluded from duplicate promotion.

## Pixmap core integration

- pixmap.cpp: Planet Blupi-derived DirectDraw implementation, integrated from the core candidate. It yields 22 STRICT functions in the full multi-TU run, including CPixmap::~CPixmap.
- pixmap.h: the PB-derived class layout includes one BOOL m_bTrueColorBack after m_mouseType, at this+8. The offset is required by oracle accesses and shifts later members by four bytes. The name is a hypothesis taken from SB2; SB2's surrounding layout does not match SB1. This field also allows the destructor and other pixmap methods to match.

## UI cluster

- button.cpp: PB-derived routines with SB1 behavior/layout updates. SetIconMenu and SetToolTips use the SB2 reconstruction methods (GPLv3).
- button.h: PB-derived UI declaration set adjusted to the SB1 member layout established in docs/UI_CLUSTER.md. Keep the UI Create signature and m_bDemo field; the oracle's call cleanup and +0x18 access require them.
- jauge.cpp: PB-derived routines with the SB1 field layout; `CJauge::Draw` is strict at `0x1ee50`. GetLevel and GetType use SB2 reconstruction methods (GPLv3).
- jauge.h: UI declaration set with the CDecor* member omitted to match observed SB1 offsets; retain GetLevel and GetType declarations.
- menu.cpp and menu.h: PB-derived menu routines and declarations, adjusted to the observed SB1 member offsets and initialization behavior.
- text.cpp: PB-derived text routines plus SB2 DrawCharSingle, DrawTextLeft, and DrawChar code (GPLv3). `DrawText` uses the post-increment loop form that matches SB1 at `0x23770`; `DrawChar` and the SB2-style GPLv3 `texttables.h` glyph table remain open, including table identity.
- text.h: UI free-function declarations used by the candidate text TU.
- decor.h: three include names were lower-cased (DEF.H, SOUND.H, PIXMAP.H) to avoid VC5 reopening the same headers under different case spellings. This is a compile-context fix; declarations do not change.
- def.h: existing layout and line numbers retained; only SB1 channel values changed to CHLITTLE=7 and CHTEXT=6. The candidate-only blank first line was dropped.

**[EVIDENCE]** `def.h` undefines the Windows `DrawText` and `GetCharWidth` macros after the Windows headers. VC5 otherwise rewrites those source identifiers to `DrawTextA` and `GetCharWidthA`; retaining the source names reproduces the original object symbols. The undefines are a compile-context shim and do not change emitted function bodies.

**[EVIDENCE]** The coherent UI/core header set preserves every previously accepted UI function in the clean combined build. sound.h is the exception to the core candidate header set: its StopSound declaration was held back because adding it regressed CButton::MouseMove by seven bytes.

**[OPEN]** DrawChar and the glyph table do not yet match the oracle; table identity is unproven. Per-function UI evidence and remaining differences are in docs/UI_CLUSTER.md.

## SB1 engine/game integration

- `blupi.cpp` is SB2-derived game/window code with SB1-targeted body edits recorded in [docs/BLUPI_TU.md](../docs/BLUPI_TU.md). The integrated object has 11 STRICT functions, including `UpdateFrame` under the SB1 phase override. `WindowProc` local labels are checked from their section-relative offsets; stale per-label address mappings were removed.
- `decnet.cpp` is SB2-derived networking/game code (5 STRICT functions). `decor.cpp` is assembled from SB2 `decor.cpp`, `decio.cpp`, and `decblock.cpp` methods (12 STRICT functions). The candidate's `LEGACY_CODE` guard was resolved into the unconditional SB1 store path in `OpenDoorsWin`; no build switch remains. Its `PlayImage` calls pass the fourth zero explicitly to select the target-proven four-word call ABI while `sound.h` retains the separate three- and four-argument overloads required by other TUs.
- `decblock.cpp`, `decdesign.cpp`, `decmove.cpp`, and `decblupi.cpp` contain SB2-derived engine methods; integrated STRICT counts are 0, 4, 11 accepted, and 4 respectively. `decmove.cpp` also emits a duplicate `ObjectDelete` body owned by `decnet.cpp`. `IsPassIcon` and `IsBlocIcon` remain relocation-unverified because `table_decor_quart` has no unique reconstructed data definition.
- These SB2-derived sources are GPLv3. `dectables.h`, `pixtables.h`, `network.h`, `obstacle.h`, and `wave.h` are SB2-derived support headers (GPLv3).
- `decor.h` is the coherent SB1 layout selected from the decor and engine probes. Strict code and target data establish `m_decor +0x10`, `m_bigDecor +0x4e30`, and `m_moveObject +0x9c50` with 48-byte records; SB2 trajectory arrays are absent before `m_moveObject`. The saved `DescGameFile` size is `0xd490`, and the target `doors` array is `short[100]`. `def.h` uses SB1 values `MAXNOTIF=4`, `CHBUTTON=4`, `CHJAUGE=5`, `ACTION_STOP=1`, `SPRITE_WAIT=11`, and `SPRITE_EMPTY=12`, supported by strict game/UI methods. `CaisseGetMove` proves `m_blupiPower` at `+0xc934` and `m_blupiShield` at `+0xc938`; `CurrentWrite` and `CurrentRead` map those members to the opposite named fields in the serialized structure, preserving the target byte order. The FIFO/no-barre word is at `+0xc978`, FIFO positions at `+0xc97c`, and start positions at `+0xc9cc`. The header aliases the word at `+0xc978` as `m_blupiFifoNb` and `m_blupiNoBarre`; which name matches the original SB1 declaration remains open.
- **[HYPOTHESIS]** The tail declarations after the last strictly observed engine member (including `m_lastDecorIndexes` and trajectory/detection members inherited from SB2) are compile context only. Their SB1 presence and ordering have not been established by the accepted methods.
- The original `blupi.cpp` include order (`event.h` before `network.h`) is retained; `event.h` forward-declares `CNetwork`, while `network.h` provides the full DPID/network declarations before definitions. `CDecor::Build(RECT, POINT)` is evidenced by strict `SetDecor` and target callee cleanup; `MoveStep(BOOL)` supplies the game call shape. `UpdateFrame` now matches under the SB1 phase override.
- `decnet.cpp` owns `CDecor::ObjectDelete` at `0x108a0` (strict). The same body emitted by `decmove.cpp` is a duplicate candidate and is excluded from promotion; the oracle function is accepted and counted only under `decnet.cpp`. `SetCelHiliFromScreenPos` at `0x5ab0` and `DeleteCel` at `0x5b40` are strict in `decdesign.cpp`, supporting the corrected `decdesign` boundary in [docs/TU_MAP.md](../docs/TU_MAP.md).

## Event TU integration

- `event.cpp` and the SB2-derived `event.h` compatibility layout are GPLv3 ancestry. `Phase::backName[32]` and the CEvent offsets in the compatibility header follow the SB1 target; the SB2-only `m_menuIndex`, `m_menuDecor[11]`, and `m_9840[32]` fields are omitted so `m_oldMousePos` is at `+0x9810`. The phase table is the worker's exact SB1 table.
- `sound.h` retains the event call-compatible `PlayMusic(HWND,int)` overload and distinct `PlayImage` declarations: the three-argument form used by the PB-derived sound TU and the four-argument game form. The target `VoyageInit` pushes five stack words to the callee at `0x230c0`, which returns with `ret 0x14`; `decor.cpp` passes the fourth word explicitly so VC5 selects that ABI without changing the other TUs' declaration context.
- Compile-context adaptations are scoped by manifest defines: `SB1_GAME_API` supplies game-candidate `CPixmap`, `CSound`, and `CDecor` signatures; `SB1_EVENT_API` supplies event-only `CButton::SetDemo`, the event `CPixmap`/`CSound`/`CDecor` overloads, and the SB1 event phase/resource constants. `event.h` has a guard and forward-declares `CNetwork`; `event.cpp` includes `network.h` after `event.h`, so DPID and the full network declaration are available before method definitions. This preserves the source include order without importing duplicate SB2 enums or changing unrelated TUs.
- **[EVIDENCE]** The integrated shared-header run reports 54 STRICT event names and 3,584 unique event code bytes. The three `Play*` names alias `BuildUp` at one RVA; only `BuildUp` is accepted. `NetSendLobby`, `WaitMouse`, `HideMouse`, and `ChatSend` are strict in the integrated source. `CEvent::~CEvent` is strict after verifying its `__except_list` absolute `FS:0` references, `.text$x` funclet bytes/relocations, and linked `.xdata$x` exception table. The exception cleanup's `CButton::~CButton` function-address relocations bind its identical 16-byte body to the independently matched `CMenu::~CMenu` owner at `0x1f010`, so it is reported FOLDED and counted under the menu TU.
- **[OPEN]** `SetDemo` remains excluded: its 48-byte body has four identical-byte hits and none of the 18 built objects contains a direct incoming relocation to its symbol, so call-based placement has no witness. The identical `PlayDown`/`PlayMove`/`PlayUp` aliases are excluded. `CheckWorld1` has duplicate bodies outside the event block. Integrated functions that still mismatch are intentionally not accepted.
