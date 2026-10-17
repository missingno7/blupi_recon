# Reconstructed source provenance

**[EVIDENCE]** Planet Blupi references are from https://github.com/blupi-games/planetblupi, tag v1.8.0, commit 203ccc8, licensed GPLv3+. SB2 reconstruction material is from external/sb2-decomp and is GPLv3. Reference repositories are read-only. Provenance stays here so source comments do not shift observable __LINE__ values.

## PB-derived core files

- sound.cpp: Planet Blupi v1.8.0-derived Windows DirectSound source, retaining the matched source line numbers and TRY_DS sites. The core candidate's CSound::StopSound body is strict at 0x23050, but is not in canonical source: adding its declaration to shared sound.h changes seven bytes in accepted CButton::MouseMove, so the prior header context is retained.
- movie.cpp and ddutil.cpp: Planet Blupi v1.8.0-derived Windows DirectDraw/AVI source; the core candidates were identical to the already accepted files.
- misc.cpp: Planet Blupi v1.8.0-derived utility source. Its active AddUserPath folder literal is changed to c:\Speedy Blupi\ to match the SB1 path data.
- sound.h, movie.h, misc.h, ddutil.h, and resource.h: Planet Blupi-derived declarations and support headers. sound.h remains without the candidate-only StopSound declaration to preserve the accepted UI TU context.

## Pixmap core integration

- pixmap.cpp: Planet Blupi-derived DirectDraw implementation, integrated from the core candidate. It yields 22 STRICT functions in the full multi-TU run, including CPixmap::~CPixmap.
- pixmap.h: the PB-derived class layout includes one BOOL m_bTrueColorBack after m_mouseType, at this+8. The offset is required by oracle accesses and shifts later members by four bytes. The name is a hypothesis taken from SB2; SB2's surrounding layout does not match SB1. This field also allows the destructor and other pixmap methods to match.

## UI cluster

- button.cpp: PB-derived routines with SB1 behavior/layout updates. SetIconMenu and SetToolTips use the SB2 reconstruction methods (GPLv3).
- button.h: PB-derived UI declaration set adjusted to the SB1 member layout established in docs/UI_CLUSTER.md. Keep the UI Create signature and m_bDemo field; the oracle's call cleanup and +0x18 access require them.
- jauge.cpp: PB-derived routines with the SB1 field layout; GetLevel and GetType use SB2 reconstruction methods (GPLv3).
- jauge.h: UI declaration set with the CDecor* member omitted to match observed SB1 offsets; retain GetLevel and GetType declarations.
- menu.cpp and menu.h: PB-derived menu routines and declarations, adjusted to the observed SB1 member offsets and initialization behavior.
- text.cpp: PB-derived text routines plus SB2 DrawCharSingle, DrawTextLeft, and DrawChar code (GPLv3). The SB2-style texttables.h glyph table is GPLv3; its equality with the SB1 table remains open.
- text.h: UI free-function declarations used by the candidate text TU.
- decor.h: three include names were lower-cased (DEF.H, SOUND.H, PIXMAP.H) to avoid VC5 reopening the same headers under different case spellings. This is a compile-context fix; declarations do not change.
- def.h: existing layout and line numbers retained; only SB1 channel values changed to CHLITTLE=7 and CHTEXT=6. The candidate-only blank first line was dropped.

**[EVIDENCE]** def.h does not undefine DrawText or GetCharWidth. Windows expands those names to DrawTextA and GetCharWidthA; object symbol names differ, while function body bytes match the version built with the candidate header's #undef lines. No source-level compile shim remains.

**[EVIDENCE]** The coherent UI/core header set preserves every previously accepted UI function in the clean combined build. sound.h is the exception to the core candidate header set: its StopSound declaration was held back because adding it regressed CButton::MouseMove by seven bytes.

**[OPEN]** DrawChar and the glyph table do not yet match the oracle; table identity is unproven. Per-function UI evidence and remaining differences are in docs/UI_CLUSTER.md.

## SB1 engine/game integration

- `blupi.cpp` is SB2-derived game/window code with SB1-targeted body edits recorded in [docs/BLUPI_TU.md](../docs/BLUPI_TU.md). The integrated object has 10 STRICT functions; `ReadConfig`, `UpdateFrame`, and `DoInit` remain mismatches. `WindowProc` is strict after its 12 local-label bindings are recorded in the manifest.
- `decnet.cpp` is SB2-derived networking/game code (5 STRICT functions). `decor.cpp` is assembled from SB2 `decor.cpp`, `decio.cpp`, and `decblock.cpp` methods (8 STRICT functions in the shared-header run). `VoyageInit`, `VoyageStep`, `CurrentWrite`, and `OpenDoorsWin` matched under the decor-only private overlay but are mismatches with the canonical merged header; their exact compile-context cause remains open, so the engine-proven offsets below are retained.
- `decblock.cpp`, `decdesign.cpp`, `decmove.cpp`, and `decblupi.cpp` contain SB2-derived engine methods; integrated STRICT counts are 0, 4, 11 accepted, and 4 respectively. `decmove.cpp` also emits a duplicate `ObjectDelete` body owned by `decnet.cpp`. `IsPassIcon` and `IsBlocIcon` remain relocation-unverified because `table_decor_quart` has no unique reconstructed data definition.
- These SB2-derived sources are GPLv3. `dectables.h`, `pixtables.h`, `network.h`, `obstacle.h`, and `wave.h` are SB2-derived support headers (GPLv3).
- `decor.h` is the coherent SB1 layout selected from the decor and engine probes. Strict code and target data establish `m_decor +0x10`, `m_bigDecor +0x4e30`, and `m_moveObject +0x9c50` with 48-byte records; SB2 trajectory arrays are absent before `m_moveObject`. The saved `DescGameFile` size is `0xd490`, and the target `doors` array is `short[100]`. `def.h` uses the SB1 `MAXNOTIF=4` value supported by `m_jauges +0xcd30`; `CHLITTLE=7` and `CHTEXT=6` retain the earlier SB1 UI evidence. Strict game functions establish `Power +0xc934` before `Shield +0xc938`; the FIFO/no-barre word is at `+0xc978`, FIFO positions at `+0xc97c`, and start positions at `+0xc9cc`. The header aliases the word at `+0xc978` as `m_blupiFifoNb` and `m_blupiNoBarre`; which name matches the original SB1 declaration remains open.
- **[HYPOTHESIS]** The tail declarations after the last strictly observed engine member (including `m_lastDecorIndexes` and trajectory/detection members inherited from SB2) are compile context only. Their SB1 presence and ordering have not been established by the accepted methods.
- The original `blupi.cpp` include order (`event.h` before `network.h`) is retained; `event.h` forward-declares `CNetwork`, while `network.h` provides the full DPID/network declarations before definitions. The 12 `WindowProc` local-label bindings are recorded by relocation offset to the statically inspected SB1 branch destinations in `recon.json`. `CDecor::Build(RECT, POINT)` is evidenced by strict `SetDecor` and the target callee cleanup; `MoveStep(BOOL)` is declared for the game call shape, but `UpdateFrame` remains a mismatch in the merged canonical header context and is not accepted.
- `decnet.cpp` owns `CDecor::ObjectDelete` at `0x108a0` (strict). The same body emitted by `decmove.cpp` is a duplicate candidate and is excluded from promotion; the oracle function is accepted and counted only under `decnet.cpp`. `SetCelHiliFromScreenPos` at `0x5ab0` and `DeleteCel` at `0x5b40` are strict in `decdesign.cpp`, supporting the corrected `decdesign` boundary in [docs/TU_MAP.md](../docs/TU_MAP.md).

## Event TU integration

- `event.cpp` and the SB2-derived `event.h` compatibility layout are GPLv3 ancestry. `Phase::backName[32]` and the CEvent offsets in the compatibility header follow the SB1 target; the SB2-only `m_menuIndex`, `m_menuDecor[11]`, and `m_9840[32]` fields are omitted so `m_oldMousePos` is at `+0x9810`. The phase table is the worker's exact SB1 table.
- `sound.h` also retains the event call-compatible `PlayMusic(HWND,int)` overload and the game `PlayImage(int,POINT,int,int)` declaration. The target `VoyageInit` pushes five stack words to the callee at `0x230c0`, which returns with `ret 0x14`; these declarations encode that ABI evidence, while each method body remains subject to the verifier.
- Compile-context adaptations are scoped by manifest defines: `SB1_GAME_API` supplies game-candidate `CPixmap`, `CSound`, and `CDecor` signatures; `SB1_EVENT_API` supplies event-only `CButton::SetDemo`, the event `CPixmap`/`CSound`/`CDecor` overloads, and the SB1 event phase/resource constants. `event.h` has a guard and forward-declares `CNetwork`; `event.cpp` includes `network.h` after `event.h`, so DPID and the full network declaration are available before method definitions. This preserves the source include order without importing duplicate SB2 enums or changing unrelated TUs.
- **[EVIDENCE]** The shared-header run accepts 44 unique event bodies (2,736 unique code bytes). The candidate comparator reports 47 STRICT names because three `Play*` names alias `BuildUp` at one RVA; only `BuildUp` is accepted. `NetSendLobby`, `WaitMouse`, `HideMouse`, and `ChatSend` were strict in the isolated worker overlay but are mismatches with the integrated shared headers.
- **[OPEN]** Event acceptance excludes `SetDemo` and the identical `PlayDown`/`PlayMove`/`PlayUp` aliases. The destructor's `$L` runtime-data and `__except_list` relocations remain unverified; `CheckWorld1` has duplicate bodies outside the event block. Integrated functions that still mismatch are intentionally not accepted.
