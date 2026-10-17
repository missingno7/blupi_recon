# Reconstructed source provenance

**[EVIDENCE]** Planet Blupi references are from https://github.com/blupi-games/planetblupi, tag v1.8.0, commit 203ccc8, licensed GPLv3+. SB2 reconstruction material is from external/sb2-decomp and is GPLv3. Reference repositories are read-only. Provenance stays here so source comments do not shift observable __LINE__ values.

**[EVIDENCE]** The current manifest accepts 303 STRICT functions and three FOLDED functions. A clean combined verify covers 38,752 unique bytes of the 141,488-byte game `.text` range; folded bytes are counted under their owner once. The folded bodies are `CButton::~CButton`, `CJauge::~CJauge`, and `OutputDebug`, all folded into `CMenu::~CMenu` at `0x1f010`.

## PB-derived core files

- sound.cpp: Planet Blupi v1.8.0-derived Windows DirectSound source, retaining existing matched line numbers and TRY_DS sites. It now includes STRICT `PlaySoundDS(int,int,int,int)` (0x22f00), `PlaySoundDS(int,int,int)` (0x22fc0), `StopSound` (0x23050), `PlayImage` (0x230c0), and `Play` (0x23220).
- movie.cpp and ddutil.cpp: Planet Blupi v1.8.0-derived Windows DirectDraw/AVI source; the core candidates were identical to the already accepted files.
- misc.cpp: Planet Blupi v1.8.0-derived utility source. `AddUserPath` (0x1f560), `TraceErrorDD` (0x1f690), and `GetCurrentDir` (0x1f360) are STRICT and accepted. The verifier binds `__pgmptr` only through its unique VC5 SP3 CRT `__setargv` reference plus the separately compiled `_pgmptr` zero-filled definition; `ChangeSprite` remains MISMATCH.
- sound.h, movie.h, misc.h, ddutil.h, and resource.h: Planet Blupi-derived declarations and support headers. `sound.h` now declares the target helper overloads and `StopSound`; the full build keeps `CButton::TreatEvent`, `CButton::MouseMove`, and `CMenu::TreatEvent` STRICT. `StopAllSounds` is not declared in the accepted context.

## Network TU integration

- network.cpp: SB2 reconstruction DirectPlay code (GPLv3), adjusted to SB1 evidence. `TraceErrorDP` uses the target error set/labels; `EnumSessions` initializes the descriptor fields in target store order; `Receive` compares the sender DPID with each player's stored DPID. Thirteen inventoried functions and both `CALLBACK` enumeration bodies are STRICT in the combined run; `JoinSession`, `CreateSession`, `Send`, and `Close` remain mismatches.
- network.h: SB2-derived types with SB1 member order established by target offsets: `m_pDP` at `+0`, three lists at `+4..+0x1c`, `m_bHost` at `+0x1c`, `m_dpid` at `+0x20`, and players at `+0x24`. Both enumeration callback declarations use `CALLBACK`, matching their `ret 0x14` and `ret 0x10` bodies.
- event.cpp: `CEvent::NetEnumSessions` calls `CNetwork::EnumSessions`. This corrects the candidate's stale `EnumProviders` call: the accepted body at `0x16920` relocates to `0x20bd0`, the verified `EnumSessions` entry. `CEvent::NetDraw` and `CEvent::DrawMap` compile to the same 48-byte body at `0x16e80`; accept it once under `NetDraw`, with `DrawMap` excluded from duplicate promotion.

## Pixmap core integration

- pixmap.cpp: Planet Blupi-derived DirectDraw implementation with the core residual source forms. The full build has 34 STRICT functions, including the six previously integrated `CPixmap::CPixmap` (0x21040), `MouseUpdate` (0x22370), `MouseBackClear` (0x22590), `MouseBackSave` (0x226c0), `MouseBackRestore` (0x227c0), and `MouseHotSpot` (0x22910), plus residual `DrawMap` (0x217a0), `DrawIcon` (0x21e30), `DrawPart` (0x22100), `MouseBackDraw` (0x225b0), and `MouseRectSprite` (0x22890). The four atlas base mappings are backed by oracle references and constrained to `.data`; atlas contents and `Create` remain open.
- pixmap.h: the SB1 layout places `m_bTrueColorBack` at +4 and `m_mouseType` at +8, supported by constructor stores and mouse accesses. The target `Create` declaration takes five arguments; `MouseRectSprite` takes `RECT*` and `POINT*`. The field name is a hypothesis from SB2; its offset is target evidence.

## UI cluster

- button.cpp: PB-derived routines with SB1 behavior/layout updates. SetIconMenu and SetToolTips use the SB2 reconstruction methods (GPLv3).
- button.h: PB-derived UI declaration set adjusted to the SB1 member layout established in docs/UI_CLUSTER.md. Keep the UI Create signature and m_bDemo field; the oracle's call cleanup and +0x18 access require them.
- jauge.cpp: PB-derived routines with the SB1 field layout; `CJauge::Draw` is strict at `0x1ee50`. GetLevel and GetType use SB2 reconstruction methods (GPLv3).
- jauge.h: UI declaration set with the CDecor* member omitted to match observed SB1 offsets; retain GetLevel and GetType declarations.
- menu.cpp and menu.h: PB-derived menu routines and declarations, adjusted to the observed SB1 member offsets and initialization behavior.
- text.cpp: PB-derived text routines plus SB2 DrawCharSingle, DrawTextLeft, and DrawChar code (GPLv3). The post-increment drawing loop matches SB1 at `0x23770`; `GetCharWidthA` matches at `0x23610`.
- texttables.h: the 13 `table_char` rank fields and `table_width[62]` / `[124]` values are corrected from oracle data evidence; the full table identity remains open.
- text.h: UI free-function declarations used by the candidate text TU.
- decor.h: three include names were lower-cased (DEF.H, SOUND.H, PIXMAP.H) to avoid VC5 reopening the same headers under different case spellings. This is a compile-context fix; declarations do not change.
- def.h: existing layout and line numbers retained; SB1 values include `CHLITTLE=7`, `CHTEXT=6`, `CHBUTTON=4`, and `CHJAUGE=5`. The candidate-only blank first line and `#undef DrawText` / `#undef GetCharWidth` blocks were dropped.

**[EVIDENCE]** Windows' ANSI macros expand the helper identifiers to `DrawTextA` and `GetCharWidthA`. With the undefines removed, both functions retain strict code and relocation matches at `0x23770` and `0x23610`; `recon.json` records the emitted macro-expanded symbols. No macro workaround is needed.

**[EVIDENCE]** The coherent UI/core header set preserves every previously accepted UI function in the clean combined build. The accepted `sound.h` declaration order adds the three helper overloads while retaining the two-DWORD overload, and does not declare `StopAllSounds`. `def.h` keeps game `CHBLUPI=4`; `CHPIXMAP_CURSOR=10` names CPixmap's separately evidenced cursor surface slot (`this+0x94`, array base `+0x6c`). Only pixmap cursor-surface references use that alias.

**[EVIDENCE]** `table_char` rank fields and `table_width[62]`/`[124]` now match the observed target bytes. The full glyph table identity and `DrawChar` body remain open. Per-function UI evidence and remaining differences are in docs/UI_CLUSTER.md.

## SB1 engine/game integration

- `blupi.cpp` is SB2-derived game/window code with SB1-targeted body edits recorded in [docs/BLUPI_TU.md](../docs/BLUPI_TU.md). The integrated object has 11 STRICT functions, including `UpdateFrame` under the SB1 phase override. `WindowProc` local labels are checked from their section-relative offsets; stale per-label address mappings were removed.
- `decnet.cpp` is SB2-derived networking/game code (5 STRICT functions). `decor.cpp` is assembled from SB2 `decor.cpp`, `decio.cpp`, and `decblock.cpp` methods (14 STRICT functions). `CDecor::Write` matches with `DescFile::reserve1` as `int[50]`; `AdaptMidBorder` uses an 80-entry initialized table whose bytes match the target. Its one-past table relocation is accepted only at the verified end of that same data object. The candidate's `LEGACY_CODE` guard was resolved into the unconditional SB1 store path in `OpenDoorsWin`; no build switch remains. Its `PlayImage` calls pass the fourth zero explicitly to select the target-proven four-word call ABI while `sound.h` retains the separate three- and four-argument overloads required by other TUs.
- `decblock.cpp`, `decdesign.cpp`, `decmove.cpp`, and `decblupi.cpp` contain SB2-derived engine methods; integrated STRICT counts are now 16, 36, 18 accepted, and 10 respectively. `PlaceDecor` uses the target-proven global `int*` helper signature. `decmove.cpp` also emits a duplicate `ObjectDelete` body owned by `decnet.cpp`. `IsPassIcon` and `IsBlocIcon` are STRICT with `table_decor_quart` defined and its initialized bytes checked against the oracle.
- These SB2-derived sources are GPLv3. `dectables.h`, `pixtables.h`, `network.h`, `obstacle.h`, and `wave.h` are SB2-derived support headers (GPLv3).
- `decor.h` is the coherent SB1 layout selected from the decor and engine probes. Strict code and target data establish `m_decor +0x10`, `m_bigDecor +0x4e30`, and `m_moveObject +0x9c50` with 48-byte records; SB2 trajectory arrays are absent before `m_moveObject`. The saved `DescGameFile` size is `0xd490`, and the target `doors` array is `short[100]`. `def.h` uses SB1 values `MAXNOTIF=4`, `CHBUTTON=4`, `CHJAUGE=5`, `ACTION_STOP=1`, `SPRITE_WAIT=11`, and `SPRITE_EMPTY=12`, supported by strict game/UI methods. `CaisseGetMove` proves `m_blupiPower` at `+0xc934` and `m_blupiShield` at `+0xc938`; `CurrentWrite` and `CurrentRead` map those members to the opposite named fields in the serialized structure, preserving the target byte order. The FIFO/no-barre word is at `+0xc978`, FIFO positions at `+0xc97c`, and start positions at `+0xc9cc`. The header aliases the word at `+0xc978` as `m_blupiFifoNb` and `m_blupiNoBarre`; which name matches the original SB1 declaration remains open.
- **[HYPOTHESIS]** The tail declarations after the last strictly observed engine member (including `m_lastDecorIndexes` and trajectory/detection members inherited from SB2) are compile context only. Their SB1 presence and ordering have not been established by the accepted methods.
- The original `blupi.cpp` include order (`event.h` before `network.h`) is retained; `event.h` forward-declares `CNetwork`, while `network.h` provides the full DPID/network declarations before definitions. `CDecor::Build(RECT, POINT)` is evidenced by strict `SetDecor` and target callee cleanup; `MoveStep(BOOL)` supplies the game call shape. `UpdateFrame` now matches under the SB1 phase override.
- `decnet.cpp` owns `CDecor::ObjectDelete` at `0x108a0` (strict). The same body emitted by `decmove.cpp` is a duplicate candidate and is excluded from promotion; the oracle function is accepted and counted only under `decnet.cpp`. `SetCelHiliFromScreenPos` at `0x5ab0` and `DeleteCel` at `0x5b40` are strict in `decdesign.cpp`, supporting the corrected `decdesign` boundary in [docs/TU_MAP.md](../docs/TU_MAP.md).

**[EVIDENCE]** The oracle-first `decblock.cpp` candidate contributes 12 STRICT methods (426 unique oracle bytes), with `AdaptMotorVehicleSound` at `0x58d0` added from the residual candidate. `decdesign.cpp` adds `ScreenPosToCelPos` (`0x5a20`), `SetIconLiftAndStuff` (`0x5ae0`), and `PlaceDecor` (`0x5ba0`) after the already matched design methods. `decblupi.cpp` has STRICT `MoveObjectPlouf` (0xfdd0), `MoveObjectTiplouf` (0xfe20), and `MoveObjectBlup` (0xfe80), bringing that TU to seven STRICT methods. The candidate `int MoveObjectPollution()` definition was excluded because it conflicts with the existing `void` member definition. The existing `DoInit` allocation of `0x258d4` bytes remains **[OPEN]**: no unexplained `0x17ce8` CDecor tail padding was added; computed-indexed storage or a non-`sizeof(CDecor)` allocation remain possibilities. A later proposed move of `m_blupiOver` after `m_blupiSurf` was rejected: it regressed accepted `CDecor::CurrentWrite`; the canonical field order remains in place.

## Event TU integration

- `event.cpp` and the SB2-derived `event.h` compatibility layout are GPLv3 ancestry. `Phase::backName[32]` and the CEvent offsets in the compatibility header follow the SB1 target; the SB2-only `m_menuIndex`, `m_menuDecor[11]`, and `m_9840[32]` fields are omitted so `m_oldMousePos` is at `+0x9810`. The phase table is the worker's exact SB1 table.
- `sound.h` retains the event call-compatible `PlayMusic(HWND,int)` overload and distinct `PlayImage` declarations: the three-argument form used by the PB-derived sound TU and the four-argument game form. The target `VoyageInit` pushes five stack words to the callee at `0x230c0`, which returns with `ret 0x14`; `decor.cpp` passes the fourth word explicitly so VC5 selects that ABI without changing the other TUs' declaration context.
- **[EVIDENCE]** Follow-up 17 removes the former per-TU API gates; their shared declarations are now unconditional, and the manifest has no per-TU `SB1_*` definitions. The withdrawn `GetBlupiInfo` record and its probe evidence are documented below. `event.h` has a guard and forward-declares `CNetwork`; `event.cpp` includes `network.h` after `event.h`, so DPID and the full network declaration are available before method definitions.
- **[EVIDENCE]** The integrated shared-header run reports 54 STRICT event names and 3,584 unique event code bytes. The three `Play*` names alias `BuildUp` at one RVA; only `BuildUp` is accepted. `NetSendLobby`, `WaitMouse`, `HideMouse`, and `ChatSend` are strict in the integrated source. `CEvent::~CEvent` is strict after verifying its `__except_list` absolute `FS:0` references, `.text$x` funclet bytes/relocations, and linked `.xdata$x` exception table. The exception cleanup's `CButton::~CButton` function-address relocations bind its identical 16-byte body to the independently matched `CMenu::~CMenu` owner at `0x1f010`, so it is reported FOLDED and counted under the menu TU.
- **[OPEN]** `SetDemo` remains excluded: its 48-byte body has four identical-byte hits and none of the 18 built objects contains a direct incoming relocation to its symbol, so call-based placement has no witness. The identical `PlayDown`/`PlayMove`/`PlayUp` aliases are excluded. `CheckWorld1` has duplicate bodies outside the event block. Integrated functions that still mismatch are intentionally not accepted.

**[EVIDENCE]** `event.h` now uses the worker-proven SB1 name/file layout: `m_name[96]`; ten name rows with a 4-byte prefix and 96-byte text; `m_bNamesExist` at `+0x8d8`; one dword at `+0x900`; and `m_filenameBuffer[256][100]` at `+0x904`, leaving `m_nbFiles` at `+0x6d04`. `CEventNameRow` converts to its `text` member so existing event call sites pass the target string address without changing row storage.

**[EVIDENCE]** `CEvent::DrawButtons` uses the improved `scratch/ev_drawbuttons/event.cpp` reconstruction. It remains MISMATCH at 7,392 candidate bytes versus 7,419 oracle bytes, with 4,886 leading bytes identical; this is the best known source body and is intentionally not accepted. `TreatEventBase` was subsequently integrated in Follow-up 11 as the best known non-strict reconstruction.

## Follow-up 11

**[EVIDENCE]** `def.h` sets `WM_DIMS1..4` to `1255..1258` (`0x4e7..0x4ea`), matching target IDs.

**[EVIDENCE]** `decor.h` names the early `CDecor` slots `+0xc964..+0xc974` as `m_blupiTimeShield`, `m_blupiTimeFire`, `m_blupiTimeMockery`, `m_blupiTimeOuf`, and `m_blupiActionOuf`; `decblock.cpp`, `decblupi.cpp`, and `decor.cpp` uses follow those meanings. The five later, same-sized source-ancestry fields are retained as `m_sb2*` placeholders so subsequent offsets stay fixed; their SB1 meanings are not established.

**[OPEN]** The `m_blupiVent`/`m_blupiSurf` order remains as in the canonical header. The worker's proposed reversal is supported by the non-strict `BlupiStep` analysis, not yet by a STRICT function.

**[EVIDENCE]** `event.cpp` now carries the best known `TreatEventBase` (6,944 bytes versus the 6,616-byte target; 730-byte fixed prefix) and `ChangeButtons` (3,024 bytes versus 2,854; 1,531-byte fixed prefix) bodies. Both remain mismatches and are guarded only by the accepted-function gate.

**[EVIDENCE]** The non-strict `BlupiStep` reconstruction was integrated because it removes target-absent behavior supported by oracle analysis and shortens the body. The original canonical body was 31,264 bytes, frame `0xc8`, with a 2-byte raw prefix; the worker candidate before field rebinding was 29,488 bytes, frame `0xbc`, also with a 2-byte prefix. The integrated object after queue-field rebinding is 29,216 bytes, still MISMATCH; the oracle body is 25,269 bytes with frame `0xa4`.

## Follow-up 12

**[EVIDENCE]** The accepted sweep adds 37 STRICT functions (1,204 unique bytes) from `decblock.cpp`, `decdesign.cpp`, `decmove.cpp`, and `decblupi.cpp`: the `CDecor` constructor/destructor and `StopSound`; the network message/notification queue methods; the design getters/setters including `GetNetDebug`/`SetNetDebug`; and `IsPassIcon`/`IsBlocIcon`. The sweep's `decdesign.cpp` includes `<stdio.h>` for the target formatting calls. The shared `decor.h` removes `m_bBuildOfficialMissions` and inserts `m_bNetDebug` after `m_bNetMovePredict`; target accessor code places these fields at `+0xcdb4` through `+0xcdc4`.

**[EVIDENCE]** The integrated `RestoreGame` (`0x166b0`), `GetWorld` (`0x191b0`), and `MovieToStart` (`0x1ae70`) event bodies are STRICT. The engine9 `PlaceDecor1/2/3` bodies (`0x5c20`, `0x5e80`, `0x61f0`) and `DecorDetect` (`0xf640`) are STRICT after their table data relocations were resolved and checked. The `DecorDetect` source keeps the canonical field order: `CurrentWrite` proves the existing order, because moving `m_blupiOver` after `m_blupiSurf` regressed that accepted method. Its source expression uses the retained member names that compile to the target offsets.

**[OPEN]** `DecorDetect`'s three target flag offsets are byte-proven, but their semantic member labels remain unresolved where the engine9 reorder conflicts with the accepted `CurrentWrite` layout.

**[EVIDENCE]** New `dectables.cpp` defines 92 ordinary initialized tables using SB2's GPLv3 `dectables.cpp`/`dectables.h` as source ancestry. Seventy-three initializers retain SB2 values with whole-payload matches in SB1 `.data`; 19 numeric tables were corrected to the target values. The corrected names are `table_36418`, `table_36458`, `table_36488`, `table_364a0`, `table_364b0`, `table_364d0`, `table_364e8`, `table_364f8`, `table_36510`, `table_36520`, `table_36530`, `table_36538`, `table_36548`, `table_36550`, `table_36570`, `table_365a8`, `table_365d0`, `table_365d8`, and `table_365f0`; their integer initializers are taken from target `.data` bytes. New `obstacle.cpp` defines `table_decor_quart` as `short[3792]`, matching the target payload at `0x431aa0` through the next anchored table at `0x433840`; the longer SB2 array is not used. `dectables.h` and `obstacle.h` are SB2-derived GPLv3 declarations.

**[EVIDENCE]** `cmpfunc.py` resolves an otherwise unbound single-definition initialized data symbol from a unique exact payload hit in the same oracle section and at natural alignment. It then validates the full initialized payload for references. Numeric `table_` arrays use their count prefix to exclude only compiler alignment padding from the compared data extent. No per-symbol VA mapping was added for these tables. This proves the tables used by `PlaceDecor1/2/3`, `DecorDetect`, `IsPassIcon`, and `IsBlocIcon`.

**[EVIDENCE]** A full-initializer scan packed each of the remaining 57 SB2 arrays using its declared element type and searched the complete byte sequence in oracle `.data`; none had an exact hit. **[OPEN]** These arrays remain omitted until their SB1 values can be reconstructed and checked: `table_36670`, `table_366a0`, `table_36748`, `table_36980`, `table_36988`, `table_369a0`, `table_369b0`, `table_369c0`, `table_369c8`, `table_adapt_decor`, `table_adapt_fromage`, `table_blitz`, `table_blupi`, `table_bridge`, `table_chenille`, `table_chenillei`, `table_cle1`, `table_cle2`, `table_cle3`, `table_creature_left`, `table_creature_right`, `table_creature_turn2`, `table_decor_action`, `table_decor_eau1`, `table_decor_eau2`, `table_decor_ecraseur`, `table_decor_goutte`, `table_decor_lave`, `table_decor_piege1`, `table_decor_piege2`, `table_decor_scie`, `table_decor_temp`, `table_drinkeffect`, `table_drinkoffset`, `table_dynamitef`, `table_explo1`, `table_explo5`, `table_explo7`, `table_follow1`, `table_follow2`, `table_guepe_left`, `table_guepe_right`, `table_guepe_turn2l`, `table_invert`, `table_invertpanel`, `table_invertstop`, `table_mirror`, `table_power`, `table_shield`, `table_shield_blupi`, `table_shieldloop`, `table_shieldtrack`, `table_sploutch1`, `table_sploutch2`, `table_sploutch3`, `table_tentacule`, and `table_tresortrack`.

## Follow-up 13

**[EVIDENCE]** The integrated sweepa event bodies add STRICT `CEvent::Create` (`0x164a0`, 96 bytes), `ChangePause` (`0x16c60`, 128 bytes), `NetSearchPlayer` (`0x16d30`, 48 bytes), `NetSendBarePacket` (`0x16e50`, 48 bytes), and `TextInputDelete` (`0x1cf00`, 83 oracle function bytes). `TextInputDelete` tests `m_textInput[start - 1]` directly after incrementing `start`, removing the redundant local pointer that prevented the match. Its emitted 96-byte object span includes 13 matching trailing `0x90` bytes.

**[EVIDENCE]** `cmpfunc.py` verifies a plain external function `REL32` call only when its resolved target is a `.text` thunk with exact `ff 25 <IAT VA>` bytes for that same imported function. `__imp_` relocations continue to require the exact IAT slot. The named imports `DirectSoundCreate` and `DirectDrawCreate` use their normal import names; the pinned DPLAYX import library and oracle ordinal imports bind `DirectPlayCreate` to ordinal 1 and `DirectPlayEnumerateA` to ordinal 2. This makes `CSound::Create` STRICT at `0x22d60` (96 bytes) and validates the existing strict `CNetwork::CreateProvider` / `EnumProviders` thunk calls. `CPixmap::Create` remains MISMATCH because its function body does not match; the import thunk alone does not establish a match.

**[EVIDENCE]** The clean combined verifier reports 309 accepted STRICT functions and 3 FOLDED functions, with 39,264 reported game `.text` bytes covered out of 141,488. The six new STRICT functions are the five event methods above and `CSound::Create`.
## Follow-up 14

**[EVIDENCE]** The sweepb round 2 integration adds STRICT `CDecor::GetBlupiInfo` (`0x57b0`), engine-side `CDecor::NetSendBarePacket` (`0x10a60`), and `CDecor::IsSurfWater` (`0x10210`). `GetBlupiInfo` is 67 oracle bytes (80-byte candidate span); the packet routine is 45 oracle bytes (48-byte span); `IsSurfWater` is 115 oracle bytes (128-byte span). The candidate `LoadImages` reconstruction is retained as non-strict best-known source (183 oracle bytes, 192 candidate bytes); `<stdio.h>` supports its `sprintf` calls. At that integration stage, `CPixmap::GetTrueColorDecor` had a game-only declaration and an overlapping event declaration. Follow-up 16 merged these into one unconditional member declaration; its getter body remains evidenced at `0x22dc0`. The final clean build and promotion gate report 312 accepted STRICT functions plus 3 FOLDED functions, covering 39,520 / 141,488 game `.text` bytes; all earlier accepted functions remain STRICT.
## Follow-up 15

**[EVIDENCE]** The mida integration adds six STRICT `CEvent` methods: constructor (`0x16220`, 319 oracle bytes / 320-byte emitted span), `EventButtons` (`0x18ea0`, 547 / 560), `StartMovie` (`0x1c0b0`, 349 / 352), `WriteInfo` (`0x1c280`, 413 / 416), `ReadInfo` (`0x1c420`, 475 / 480), and `DemoPlayStart` (`0x1c7e0`, 447 / 448).

**[EVIDENCE]** `PutTextInputBox` and `NetAdjustLobbyButtons` retain mida's target-informed source updates as best-known non-strict reconstructions. They remain MISMATCH (400 candidate / 389 target bytes and 512 / 494 respectively), but the clean combined gate preserves every accepted function.

**[EVIDENCE]** `CEvent` calls the four-argument `CSound::PlaySoundDS`. A shared `friend class CEvent;` in `sound.h` changes VC5 TU context and regresses accepted `GetBlupiInfo`, while declaring the playback/buffer helpers `public` (no added declaration) keeps every accepted function strict with no per-TU switch. The access level is not observable in the binary (only in decorated names), so `public` is the evidence-backed choice; the earlier TU-scoped `SB1_EVENT_SOUND_ACCESS` friend was removed by the supervisor.

**[EVIDENCE]** The final clean build and promotion gate report 318 accepted STRICT functions plus 3 FOLDED functions, covering 42,096 / 141,488 game `.text` bytes.

## Build-context debt

**[EVIDENCE]** Follow-up 17 integrates the four shared headers from
`scratch/hdr/src/`. They contain the complete candidate declarations with the
six former `SB1_GAME_API`/`SB1_EVENT_API` blocks unconditional. `recon.json`
has no per-TU definitions, and no `#ifdef SB1_*` block remains in the source
headers. This removes the synthetic per-TU header views; it does not prove
that every inherited candidate declaration appeared in the original 1998
headers.

**[EVIDENCE; historical baseline]** Follow-up 17 had 317 STRICT and 3 FOLDED
functions, with `CButton::MouseMove` STRICT and `CDecor::GetBlupiInfo`
withdrawn. Earlier count probes linked that mismatch to missing preceding
`decblock.cpp` context; their evidence remains in
`docs/HEADERS.md#whole-tree-and-isolation-experiments`.

**[EVIDENCE]** `tools/verify.py` keeps withdrawn functions in the manifest,
reports their observed status separately, and does not fail the gate for
them. A later `--accept` reactivates a withdrawn function only when its
candidate becomes STRICT or FOLDED, moving the withdrawal record to
`withdrawal_history` instead of discarding that history.
## Follow-up 18: event helper ABI and ChangeButtons

**[EVIDENCE]** Integrated only `CEvent::ChangeButtons` from the sol-sib candidate. It matches the 2,854-byte
oracle body at `0x1b220`; the object span is 2,864 bytes including 10 alignment NOPs, and all 83 relocations
are verified. The source correction shares one player-index local across the commuted SIB expressions, as
confirmed by the worker micro-experiment. Evidence: `docs/SOLSIB.md`.

**[EVIDENCE]** `CEvent::CopyMission(char*, char*)` and `CEvent::AddUserExportPath(char*, char*)` are now
declared `static`, matching their caller-cleanup ABI at oracle helpers `0x1b150` and `0x1b090`. The shared
`event.h` declaration change was checked with a full-tree verification because several TUs include it.
## Follow-up 19

**[EVIDENCE]** `decblock.cpp` now contains the best-known `InitDecor`, `Build`,
and `DrawInfo` reconstructions. They remain MISMATCH (752/744, 4,624/3,866,
and 992/1,010 candidate/oracle bytes). The source uses the existing
`POINT` components for the target slots `+0xc954/+0xc958/+0xc95c`; the
`+0xc960` reset uses the existing member at that physical offset. These
source aliases preserve the accepted shared-header context. `GetBlupiInfo`
is defined only in `decblock.cpp`, STRICT at `0x57b0` (67 oracle body bytes;
80-byte candidate extent), and was re-accepted through `verify.py --accept`.
Its previous withdrawal is preserved under `withdrawal_history` in
`recon.json`.

**[EVIDENCE]** The best BlupiStep body is integrated using source-level member
aliases into the retained shared layout. It compares as 25,269 body bytes
(25,280-byte extent), frame `0xa4`, score 49, with 12 instruction edits and
350 relocations. The prior canonical body scored 24,675 under the same
relocation-aware score; this candidate remains MISMATCH. The remapping uses
existing members at the oracle-proven offsets and adds no per-TU header view.
`GetTypeBarre` now returns `int`; `NetSendBarePacket` takes an `int` second
argument, and the accepted decorated symbol was migrated at `0x10a60`. The
source-side duplicate `GetBlupiInfo` definition was removed from `decblupi.cpp`.

**[EVIDENCE]** The integrated shared build accepts `IsLave` and `IsRessort`
at `0x10150` and `0x101b0` (96-byte object extents each); `DecorDetect` remains
STRICT. `event.cpp` keeps the strict `ChangeButtons` body and uses the
sol-sib `TreatEventBase` reconstruction. `def.h` defines `WM_BUTTON40..49` as
`WM_USER+240..249`. `TreatEventBase` remains MISMATCH: its best-known body has
the exact target size and retains eight register-byte differences at
`+0x41a` and `+0x6f3`.

**[OPEN]** The proposed reordered `decor.h` state layout is not integrated.
With it, the full shared-header build regressed accepted `CButton::MouseMove`
at `0x2fd0`; removing or moving `decor.h` in `button.cpp` did not resolve the
mismatch. The retained shared layout plus the evidence-backed prototype
changes passes the accepted gate. The BlupiStep body therefore uses
offset-equivalent legacy member expressions until a coherent header can encode
the corrected semantic names without that regression. `decor.cpp` retains its
previous strict `VoyageStep`/`CurrentWrite` source forms.

\r\n
