# SB1 class-header inventory and shared-view experiment

## Result

[EVIDENCE] A private full copy of `src/` was compiled with the pinned MSVC 5.0
SP3 settings from `recon.json` (`/W3 /GX /O2 /ML /Gd /Gy`) and compared as one
object set against the 321 accepted functions. Reproduce it with
`.pyenv\\python.exe scratch\\hdr\\run_verify.py`. The private manifest removed all
per-TU `SB1_EVENT_API` and `SB1_GAME_API` defines. The six gated blocks in
`button.h`, `decor.h`, `pixmap.h`, and `sound.h` were made unconditional. No
canonical source or manifest file was changed.

The owner-object results are 317 STRICT, 3 FOLDED, and one MISMATCH. The
MISMATCH is `CDecor::GetBlupiInfo` at `0x57b0` (`decblock.cpp`). It emits an
80-byte candidate code span; the target span has 67 bytes of instructions and
13 trailing NOP bytes. All other accepted owner functions remain STRICT or
FOLDED, covering 42,016 unique original bytes. `CButton::MouseMove` at
`0x2fd0` is STRICT in this current full-tree run.

[OPEN] This does not establish an authentic shared header set: the
unconditional candidate still loses `GetBlupiInfo`, and the missing original
declarations/order have not been identified. Do not integrate the four
unconditional candidate headers as final headers yet. Candidate copies are
under `scratch/hdr/src/`.

The current `MouseMove` result differs from the earlier per-block observations
in `docs/WORKFLOW.md`: in this full no-switch run it is STRICT. The canonical
headers now include the helper declarations resolved in the core-cluster work,
and the combined declaration context is non-additive. The isolated results
remain useful for their own manifest context, but they do not predict this
whole-tree result.

## Evidence and inventory method

[EVIDENCE] Function names and translation-unit membership come from
`oracle/functions.csv` and `oracle/tu_map.csv`. Anchored `best_source` names
identify methods directly; bracketed ranges use the TU's contiguous source
order from `docs/TU_MAP.md`. Candidate declarations, overloads, and their
current signatures are transcribed from the class headers in `src/`. The
source definitions in `src/*.cpp` confirm argument types where they exist.

[HYPOTHESIS] The name inventories below are the complete PB 1.8.0-derived
candidate declaration sets, in header/source-section order. They are useful
SB1 candidates, not proof that every inherited declaration appeared in the
1998 headers. The `best_source` anchors are a smaller, independently named
subset; an unanchored method is not upgraded to evidence just because it is
present in the reference source. Full parameter and return declarations
remain visible in the linked headers. Constructors and destructors are
included in each count.

For `CDecor`, the source sections follow the class header's `Decor`,
`DecBlock`, `DecDesign`, `DecBlupi`, `DecMove`, `DecNet`, and `DecIO` groupings.
For the other classes, names retain the declaration order in their current
headers. Repeated names are repeated explicitly to preserve order; `×N`
compresses N declarations, usually overloads.

## Method inventory by class

### CDecor — 206 declarations in [decor.h](../src/decor.h)

[HYPOTHESIS] Candidate declaration names, grouped in source order:

- **Decor:** `CDecor`, `~CDecor`, `Create`, `LoadImages`, `InitGamer`, `InitDecor`, `SetTime`, `GetTime`, `PlayPrepare`, `BuildPrepare`, `IsTerminated`, `MoveStep ×2`, `GetBlupiChannelStandard`, `GetBlupiChannelActual`, `GetIconPerso`, `Build ×2`, `DrawInfo`, `DecorNextAction`, `KeyChange`, `SetJoystickEnable`, `SetDemoPlay`, `PlaySound ×2`, `StopSound`, `AdaptMotorVehicleSound`, `ChangePhase`, `GetRegion`, `SetRegion`, `GetMusic`, `SetMusic`, `GetDim`, `SetDim`, `GetNbVies`, `SetNbVies`, `GetPause`, `ChangePause`, `GetDoors`, `SetDoors`, `SetAllMissions`, `CheatAction`, `GetSuperBlupi`, `SetSuperBlupi`, `GetDrawSecret`, `SetDrawSecret`, `SetBuildOfficialMissions`, `GetNetPacked`, `SetNetPacked`, `GetNetMovePredict`, `SetNetMovePredict`, `GetNetDebug`, `SetNetDebug`, `OutputNetDebug`, `SetMulti`, `SetTeam`, `VoyageGetPosVie`, `VoyageInit`, `VoyageStep`, `VoyageDraw`, `DrawMap`.
- **DecBlock:** `BlitzActif`, `SoundEnviron`, `IsWorld`, `ActiveSwitch`, `GetTypeBarre`, `IsLave`, `IsPiege`, `IsGoutte`, `IsScie`, `IsSwitch`, `IsEcraseur`, `IsBlitz`, `IsRessort`, `IsTemp`, `IsBridge`, `IsDoor`, `IsTeleporte`, `SearchTeleporte`, `IsSurfWater`, `IsDeepWater`, `IsOutWater`, `IsPassIcon`, `IsBlocIcon`, `IsVentillo`, `ModifDecor`, `IsRightBorder`, `IsFromage`, `IsGrotte`, `AdaptMidBorder`, `AdaptBorder`.
- **DecDesign:** `ScreenPosToCelPos`, `SetCelHiliFromScreenPos`, `SetIconLiftAndStuff`, `DeleteCel`, `PlaceDecor`, `PlaceDecor1`, `PlaceDecor2`, `PlaceDecor3`, `PlaceDecor4`, `PlaceDecor5`, `PlaceDecor6`, `PlaceDecor7`, `PlaceDecor8`, `PlaceDecor9`, `PlaceDecor10`, `GetMissionTitle`, `SetMissionTitle`.
- **DecBlupi:** `BlupiSearchIcon`, `BlupiIsGround`, `BlupiRect`, `BlupiAdjust`, `BlupiBloque`, `BlupiStep`, `BlupiDead`, `GetPosDecor`, `BlupiAddFifo`, `DecorDetect`, `GetBlupiInfo`.
- **DecMove:** `TestPath`, `MoveObjectPollution`, `MoveObjectPlouf`, `MoveObjectTiplouf`, `MoveObjectBlup`, `FlushBalleTraj`, `SetBalleTraj`, `IsBalleTraj`, `FlushMoveTraj`, `SetMoveTraj`, `IsMoveTraj`, `SearchDistRight`, `StartSploutchGlu`, `ObjectStart ×2`, `ObjectDelete`, `MoveObjectStep`, `MoveObjectStepLine`, `MoveObjectStepIcon`, `DynamiteStart`, `AscenseurDetect`, `AscenseurVertigo`, `AscenseurShift`, `AscenseurSynchro`, `UpdateCaisse`, `TestPushCaisse`, `TestPushOneCaisse`, `SearchLinkCaisse`, `AddLinkCaisse`, `CaisseInFront`, `CaisseGetMove`, `MockeryDetect`, `BlupiElectro`, `MoveObjectFollow`, `MoveObjectDetect`, `MoveAscenseurDetect`, `MoveChargeDetect`, `MovePersoDetect`, `MoveBalleDetect`, `MoveObjectDelete`, `MoveObjectFree`, `SortGetType`, `MoveObjectSort`, `MoveObjectPriority`, `MoveObjectSearch`.
- **DecNet:** `NetStopCloud`, `NetPlaySound`, `NetStopSound`, `NetDataFlush`, `NetSendBarePacket`, `TreatNetData`, `DoNetSmooth`, `NetAdjustToLift`, `NetAscenseurSynchro`, `NetPlayerCollide`, `NetMessageIndexFlush`, `NetMessagePush`, `NetMessagePop`, `NotifFlush`, `NotifPop`, `NotifPush`, `NotifStep`.
- **DecIO and inline helpers:** `GetMissionPath`, `Write`, `Read`, `SomethingMissionPath`, `CurrentWrite ×2`, `CurrentRead`, `FileExist`, `SearchWorld`, `SearchDoor`, `AdaptDoors`, `OpenDoorsTresor`, `OpenDoor`, `OpenDoorsWin`, `OpenGoldsWin`, `DoorsLost`, `DeleteMission`, `IsValidCel`, `MoveObjectCopy`, `StopVehicleSound`, `IsDeadAction`, `StopBlupi`, `IsBlupiVehicle`, `IsBlupiMotorVehicle`, `IsBlupiStanding`.

[EVIDENCE] Named `best_source` anchors include `SetCelHiliFromScreenPos`,
`DeleteCel`, `GetMissionTitle`, `SetMissionTitle`, `TestPath`, `NetStopCloud`,
`ObjectDelete`, `NetPlaySound`, `NetStopSound`, `VoyageGetPosVie`,
`GetMissionPath`, `CurrentWrite`, `CurrentRead`, `FileExist`, and `SearchDoor`.
The anchor source TUs are `decdesign.cpp`, `decblupi.cpp`, `decnet.cpp`, and
`decor.cpp`; see those rows in `oracle/tu_map.csv` for RVAs and evidence text.

### CPixmap — 54 declarations in [pixmap.h](../src/pixmap.h)

[HYPOTHESIS] Candidate names: `CPixmap`, `~CPixmap`, `SetDebug`, `Create ×2`,
`Flush`, `Restore`, `InitSysPalette`, `IsPalette`, `Fill`, `SavePalette`,
`RestorePalette`, `SearchColor`, `Cache ×2`, `BackgroundCache`, `Cache`, `Flush`,
`SetTransparent`, `SetTransparent2`, `SetClipping`, `GetClipping`,
`IsIconPixel`, `DrawIcon`, `DrawIconDemi`, `DrawIconPart`, `DrawPart`,
`DrawImage`, `DrawMap`, `QuickIcon`, `CacheAll`, `GetBenchmarkSuccess`,
`SetTrueColorBack`, `SetTrueColorDecor`, `GetTrueColorBack`,
`GetTrueColorDecor`, `BuildIconMask`, `Display`, `SetMousePosSprite`,
`SetMousePos`, `SetMouseSprite`, `MouseShow`, `MouseInvalidate`,
`MouseBackClear`, `MouseBackDraw`, `RestoreAll`, `BltFast ×2`, `MouseUpdate`,
`MouseQuickDraw`, `MouseBackSave`, `MouseBackRestore`, `MouseRectSprite`,
`MouseHotSpot`.

[EVIDENCE] Named anchors include `SetDebug`, `Restore`, `InitSysPalette`,
`RestoreAll`, both `BltFast` overloads, `SavePalette`, `RestorePalette`,
`Cache`, `Flush`, `SetTransparent`, `SetTransparent2`, `SetClipping`,
`GetClipping`, `DrawImage`, `Display`, `SetMousePosSprite`, `SetMouseSprite`,
`MouseShow`, `MouseQuickDraw`, `MouseInvalidate`, and the constructor/destructor.
The target TU is `pixmap.cpp`.

### CSound — 30 declarations in [sound.h](../src/sound.h)

[HYPOTHESIS] Candidate names: `CSound`, `~CSound`, `Create`, `SetState`,
`GetEnable`, `SetAudioVolume`, `GetAudioVolume`, `SetMidiVolume`,
`GetMidiVolume`, `CacheAll`, `Cache`, `Flush`, `Play`, `StopSound`,
`PlayImage ×2`, `PlayMusic ×2`, `RestartMusic`, `SuspendMusic`, `StopMusic`,
`IsPlayingMusic`, `AdaptVolumeMusic`, `SetSuspendSkip`, `CreateSoundBuffer`,
`ReadData`, `CreateBufferFromWaveFile`, `PlaySoundDS ×3`.

[EVIDENCE] Named anchors include `CreateSoundBuffer`, `ReadData`,
`CreateBufferFromWaveFile`, the constructor/destructor, `GetEnable`, `SetState`,
audio and MIDI volume accessors, `CacheAll`, `Cache`, `Flush`, `PlayMusic`,
`RestartMusic`, `SuspendMusic`, `StopMusic`, `IsPlayingMusic`,
`AdaptVolumeMusic`, and `SetSuspendSkip`. The target TU is `sound.cpp`.

### CEvent — 99 declarations in [event.h](../src/event.h)

[HYPOTHESIS] Candidate names: `CEvent`, `~CEvent`, `OutputNetDebug`,
`GetMousePos`, `Create`, `SetFullScreen`, `SetMouseType`, `GetWorld`,
`GetWorldGroup`, `SetMission`, `ChangePhase`, `LoadImageFromDisc`, `GetPhase`,
`MovieToStart`, `ReadInput`, `TryInsert`, `CopyMission`, `RestoreGame`,
`GetButtonIndex`, `GetState`, `SetState`, `GetEnable`, `SetEnable`, `GetDemo`,
`SetDemo`, `SetIconMenu`, `GetHide`, `SetHide`, `GetMenu`, `SetMenu`,
`IsMouseRelease`, `AddUserExportPath`, `ChangePause`, `NetSendLobby`,
`DrawMap`, `DrawButtons`, `MousePosToSprite`, `MouseSprite`, `WaitMouse`,
`HideMouse`, `DisableMouse`, `GetLastMousePos`, `TreatEvent`, `TreatEventBase`,
`StartMovie`, `StopMovie`, `IsMovie`, `FlushInput`, `SetSpeed`, `GetSpeed`,
`GetPause`, `GetDoors`, `IsMulti`, `IsPrivate`, `DemoStep`, `IntroStep`,
`SetNbVies`, `ReadAll`, `AddUserExportPath`, `ClearGamer`, `DrawTextCenter`, `CreateButtons`,
`EventButtons`, `MouseOnButton`, `SearchPhase`, `ChangeButtons`, `BuildDown`,
`BuildMove`, `BuildUp`, `PlayDown`, `PlayMove`, `PlayUp`, `WriteInfo`,
`ReadInfo`, `LoadState`, `PutTextInputBox`, `DemoRecStart`, `DemoRecStop`,
`DemoPlayStart`, `DemoPlayStop`, `DemoRecEvent`, `ChatFlush`, `ChatPop`,
`ChatPush`, `ChatSend`, `TreatTextInput`, `TextInputDelete`, `NetCreate`,
`NetEnumSessions`, `NetSearchPlayer`, `NetStartPlay`, `NetSendBarePacket`,
`NetDraw`, `MouseRelease`, `MouseCapture`, `CheckWorld1`,
`NetAdjustLobbyButtons`, `CurrentRead`, `CurrentWrite`.

[EVIDENCE] Named anchors include `OutputNetDebug`, `GetMousePos`,
`SetFullScreen`, `SetMouseType`, `SetIconMenu`, `FlushInput`, networking and
draw helpers, `SetMission`, `GetPhase`, `SetNbVies`, `GetSpeed`, `SetSpeed`,
and `ChatSend`. The target TU is `event.cpp`; its complete function order is
represented in `oracle/tu_map.csv`.

### CButton — 23 declarations in [button.h](../src/button.h)

[HYPOTHESIS] Candidate names: `CButton`, `~CButton`, `Create`, `SetIconMenu`,
`SetToolTips`, `Draw`, `Redraw`, `GetState`, `SetState`, `GetMenu`, `SetMenu`,
`GetEnable`, `SetEnable`, `SetDemo`, `GetHide`, `SetHide`, `TreatEvent`,
`MouseOnButton`, `GetToolTips`, `Detect`, `MouseDown`, `MouseMove`, `MouseUp`.

[EVIDENCE] Named anchors include `SetIconMenu`, `SetToolTips`, state/menu
accessors, `SetEnable`, `SetHide`, `TreatEvent`, `MouseOnButton`, `Detect`, and
`MouseUp`. `MouseMove` is accepted STRICT at `0x2fd0`. The target TU is
`button.cpp`.

### CMenu — 16 declarations in [menu.h](../src/menu.h)

[HYPOTHESIS] Candidate names: `CMenu`, `~CMenu`, `Create`, `Update`, `Delete`,
`Draw`, `GetSel`, `GetRank`, `IsError`, `IsExist`, `Message`, `TreatEvent`,
`Detect`, `MouseDown`, `MouseMove`, `MouseUp`.

[EVIDENCE] Named anchors include the constructor, `Delete`, `TreatEvent`,
`Detect`, `MouseDown`, `MouseMove`, and `MouseUp`. The target TU is `menu.cpp`.

### CJauge — 13 declarations in [jauge.h](../src/jauge.h)

[HYPOTHESIS] Candidate names: `CJauge`, `~CJauge`, `Create`, `Draw`, `Redraw`,
`SetLevel`, `GetLevel`, `SetType`, `GetType`, `GetHide`, `SetHide`, `GetPos`,
`SetRedraw`.

[EVIDENCE] Named anchors include `Create`, level/type getters and setters, and
hide getter/setter. The target TU is `jauge.cpp`.

### CMovie — 13 declarations in [movie.h](../src/movie.h)

[HYPOTHESIS] Candidate names: `CMovie`, `~CMovie`, `Create`, `GetEnable`,
`IsExist`, `Play`, `Stop`, `playMovie`, `fileOpenMovie`, `fileCloseMovie`,
`positionMovie`, `termAVI`, `initAVI`.

[EVIDENCE] Named target anchors in `movie.cpp` include the constructor,
`Create`, `GetEnable`, `IsExist`, `Play`, `Stop`, and all five AVI helper
methods. The destructor declaration is present in the candidate header but is
not independently named by a `best_source` anchor.

### CNetwork — 18 declarations in [network.h](../src/network.h)

[HYPOTHESIS] Candidate names: `CNetwork`, `~CNetwork`, `EnumProviders`,
`GetNbProviders`, `GetProviderName`, `CreateProvider`, `FreeProviderList`,
`EnumSessions`, `GetNbSessions`, `GetSessionName`, `JoinSession`,
`FreeSessionList`, `CreateSession`, `Send`, `Receive`, `Close`,
`FreeUnknownList`, `IsHost`.

[EVIDENCE] Named anchors include constructor/destructor, provider/session
enumeration and accessors, `CreateProvider`, `FreeProviderList`, `Receive`,
`FreeUnknownList`, and `IsHost`. The target TU is `network.cpp`.

## Gated API signatures

The current six preprocessor blocks contain seven declarations (the two pixmap
draw helpers share one block). These signatures are shown in the private
unconditional headers and are [HYPOTHESIS] as original SB1 declarations unless
otherwise noted:

| Header block | Candidate declaration | Evidence and limit |
|---|---|---|
| `button.h` / `SB1_EVENT_API` | `void CButton::SetDemo(BOOL bDemo)` | `[EVIDENCE]` Called from `CEvent::SetDemo(int,BOOL)` in `event.cpp`. `[HYPOTHESIS]` The `void` return is inherited from the candidate header; the call does not consume a return value, and no SB1 target body is anchored. |
| `decor.h` / `SB1_EVENT_API` | `BOOL CDecor::CurrentWrite(int gamer, int mission, BOOL bUser)` | `[EVIDENCE]` `event.cpp` calls this overload with three arguments and tests its return. `[HYPOTHESIS]` The overload has no candidate definition; the two-argument sibling is defined in `decor.cpp`. |
| `pixmap.h` / `SB1_EVENT_API` | `BOOL CPixmap::Create(HWND hwnd, POINT dim, BOOL bFullScreen, int mouseType, BOOL bTrueColor, BOOL bTrueColorDecor)` | `[EVIDENCE]` `event.cpp` passes six arguments and consumes a Boolean result. The candidate declaration provides exact types. No separate SB1 body is anchored. |
| `pixmap.h` / `SB1_GAME_API` | `BOOL CPixmap::BackgroundCache(int channel, const char *pFilename, POINT totalDim, POINT iconDim, BOOL bUsePalette)` | `[EVIDENCE]` Calls appear in `blupi.cpp` and `event.cpp`, including a tested Boolean return. The candidate declaration supplies exact types; no candidate method body exists. |
| `pixmap.h` / `SB1_GAME_API` | `BOOL CPixmap::DrawMap(int channel, RECT src, RECT dest)`; `void CPixmap::QuickIcon(int channel, int rank, POINT pos)` | `[EVIDENCE]` Both definitions and call sites are present in `pixmap.cpp` / `decor.cpp`. Their class signatures are directly supported by those definitions. |
| `sound.h` / `SB1_EVENT_API` | `BOOL CSound::PlayMusic(HWND hWnd, int music)` | `[EVIDENCE]` `event.cpp` calls with an integer music ID; the same class has a separate filename overload. `[HYPOTHESIS]` The integer overload has no candidate definition or anchored target body. |

For member calls, the `this` pointer is passed in ECX; parameter widths from the
call sites imply these expected MSVC x86 callee-pop sizes: `SetDemo` 4,
`CurrentWrite` 12, six-argument `Create` 28, `BackgroundCache` 28,
`CPixmap::DrawMap` 36, `QuickIcon` 16, and integer `PlayMusic` 8 bytes. The
`DrawMap` body at `0x217a0` is a named STRICT target anchor; the other values
are ABI-derived expectations where no target body establishes `ret n`. A
caller that ignores a result does not establish the declaration's return
type. In particular, the undefined overloads and `SetDemo` must remain
hypotheses until an SB1 target body, import, or stronger ancestry evidence is
found.

## Whole-tree and isolation experiments

[EVIDENCE] Full no-switch run: all six guard blocks were removed and all
per-TU API defines were removed in the private manifest. `GetBlupiInfo`
(`0x57b0`) is the only accepted owner mismatch; `MouseMove` (`0x2fd0`) is
STRICT. There are extra non-owner inline/COMDAT candidates for some methods in
other objects; the accepted comparison uses each function's recorded TU owner
and does not count those duplicate copies as separate original functions.

[EVIDENCE] Isolated runs retained the manifest's existing per-TU defines and
unguarded one event-API declaration at a time. Removing the guard from
`CDecor::CurrentWrite(int,int,BOOL)`, six-argument `CPixmap::Create`, or
`CSound::PlayMusic(HWND,int)` caused both `CButton::MouseMove` and
`CDecor::GetBlupiInfo` to regress. The combined no-switch run has `MouseMove`
STRICT, showing that declaration-count effects interact across headers and
TUs. Do not add the isolated effects together as if they were additive.

[EVIDENCE] A separate diagnostic count probe for `GetBlupiInfo` inserted
scratch-only `extern int` declarations at the top of `decblock.cpp`, just
before its definition, or at end of file. For counts 0 through 32, top and
before placements produced STRICT windows at 24–27 and 32; end placement had
no STRICT count. These are context probes, not proposed declarations. They
show that compiler context and source position affect the output; they do not
identify authentic methods.

[EVIDENCE] The SB2 header comparison exposed candidate additions beyond the
current class lists. A no-switch variant tested `CDecor::GetDoors(BYTE*)`,
`SetDoors(BYTE*)`, and the SB2 `PlaceDecor(short*,...)` overload; CPixmap
`Benchmark`, `HudIcon`, `DrawChar`, `SetBenchmarkSuccess`, `MouseBackDebug`;
`CButton::GetDemo`; and CSound `ErrorSomething`, `SetCDAudio`, `PlayCDAudio`,
`StopAllSounds`. The combined variant yielded 316 STRICT, 3 FOLDED, and two
MISMATCH owner functions: `GetBlupiInfo` (`0x57b0`) and `IsDeepWater`
(`0x10290`), covering 41,936 unique original bytes. `MouseMove` was STRICT.

The additions were non-additive in focused no-switch builds. `DrawChar` alone
and the three CDecor additions alone each regressed `MouseMove` and
`GetBlupiInfo`; a full guide variant without the three CDecor additions also
regressed those two. The earlier guide subset without `DrawChar`, expanded
with the four CSound methods, restored `MouseMove` but left `GetBlupiInfo`
mismatched. These context-sensitive results do not identify which guide
declarations appeared in SB1. The additions are not included in the proposed
candidate headers. The SB2-only `GetTypeBarre` and `MoveObjectDelete` return
type differences remain signature hypotheses; the current source and target
mapping do not establish the SB1 return types. SB2's `PlaceDecor(short*,...)`
does not replace the target-proven `int*` helper at `0x5ba0`.

## Compiler findings

- **Missing header context changes bodies.** Trigger: unguarded shared class
  declarations. Variation: retain per-TU defines and unguard one event API
  declaration versus remove all gates and per-TU defines. Effect: isolated
  changes regress `MouseMove` and `GetBlupiInfo`; the combined context restores
  `MouseMove` but leaves `GetBlupiInfo` mismatched. Flags/TU: pinned MSVC 5.0
  SP3 `/O2 /GX /ML /Gd /Gy`, complete object set; key owner is `decblock.cpp`.
  Strict result: 317 STRICT + 3 FOLDED + one accepted mismatch in the full
  no-switch candidate. Related `BLUPI-CTX-01`, `VC5-CTX-01..04`.
  Classification: **confirmed SB1 context effect; declaration cause
  unverified**.
- **Count and placement probes are non-source diagnostics.** Trigger: the same
  `GetBlupiInfo` context mismatch. Variation: 24–27 and 32 dummy externs at
  top/before-definition restore STRICT; end-of-file probes do not. Flags/TU:
  same compiler flags, one-TU scratch builds of `decblock.cpp`. Strict result:
  count/position windows are observed, but no real source declaration is
  proven. Related `VC5-CTX-01..04`. Classification: **negative for identifying
  authentic declarations; unverified context mechanism**.
- **SB2-guide declaration sets do not explain the residual.** Trigger:
  `GetBlupiInfo` mismatch. Variation: add guide-only declarations to the
  all-unconditional variant. Effect: `DrawChar` alone and CDecor overloads
  each regress `MouseMove` and `GetBlupiInfo`; a full union adds
  `IsDeepWater` as another mismatch. An earlier sound-expanded subset restored
  `MouseMove` but left `GetBlupiInfo` mismatched. Flags/TU: pinned full-tree
  builds with no per-TU API defines. Strict result: none of these sets restores
  the residual without another regression. Related `BLUPI-CTX-01`,
  `VC5-CTX-01..04`. Classification: **negative for these guide sets**.

## Candidate header files for integration review

The current no-switch candidates are:

- `scratch/hdr/src/button.h`
- `scratch/hdr/src/decor.h`
- `scratch/hdr/src/pixmap.h`
- `scratch/hdr/src/sound.h`

They differ from canonical headers only by removing the six API guard blocks.
This is a reproducible experiment, not a final recommendation: integrating
them would change `CDecor::GetBlupiInfo` from STRICT to MISMATCH. The final
shared declarations and their original order remain [OPEN].
