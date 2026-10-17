# Event translation unit reconstruction

**Status: partial.** The scratch candidate now has 47 strict target bodies and an exact SB1 phase table. `CreateButtons`, the `TreatEvent` wrapper, and small helpers are strict; the larger drawing, input, phase, and dispatch bodies still differ.

## Scope and comparison

[EVIDENCE] The target block is `0x16220`-`0x1d230`, with 86 mapped functions and 34,724 Ghidra body bytes. The candidate is `scratch/tu/event.cpp`, derived mainly from SB2 `event.cpp`; candidate-only headers are in `scratch/tu/event_include/`. `src/` has no `event.cpp` or `event.h`.

[EVIDENCE] The latest candidate was compiled with VC5 SP3 (11.00.7022), `/O2 /GX /ML /Gd /Gy`, `nullptr=0`, and `TMP/TEMP=build/tmp`. The comparison used the event object together with the other reconstructed objects. Forced `--at` starts are assertions for comparison; only `STRICT` results with verified relocations count.

```powershell
.pyenv\python.exe tools\msvc.py scratch\tu\event.cpp --tree sp3 --out build\event\obj\event_work20_o2_ml_gy_gx.obj --include scratch\tu\event_include\sb2_compat --include external\sb2-decomp\bass\inc --include C:\tools\dx6sdk\INCLUDE --define nullptr=0 --flag /O2 --flag /GX --flag /ML --flag /Gd --flag /Gy
.pyenv\python.exe scratch\event\run_work20_compare.py
```

[EVIDENCE] The event object reports 109 candidate functions: 47 strict, 8 relocation-unverified, 54 mismatches, and no folded matches. There are 47 unique strict target entries totaling 2,986 original function-body bytes. Padding is excluded from body-byte coverage.

[EVIDENCE] A merge experiment using current `src/` headers plus about 73 SB2 declarations failed with over 100 compiler errors: phase/text identifiers and duplicate cases, `POINT`/`RECT` constructor differences, and changed `CDecor`, `CPixmap`, and read/write APIs. It was discarded; no `src/` header was edited.
[EVIDENCE] The focused hybrid-header compile reproduced concrete conflicts: `src/def.h` macros `CHBACK`, `CHOBJECT`, `WM_UPDATE`, `WM_DECOR1`-`WM_DECOR5`, and `WM_WRITE0`-`WM_WRITE9` expand inside same-named SB2 `def.h` enum declarations; `src/movie.h` has no include guard and conflicts with its inclusion through candidate `event.h`; `CNetwork` and `DPID` are undeclared when `event.h` precedes `network.h`. `src/pixmap.h` also declares four-argument `CPixmap::Create`, versus six arguments in the SB2 copy; candidate `CDecor::GetDoors/SetDoors(short*)` are absent from current `src/decor.h`. Keep one `def.h` and integrate the measured SB1 constants/layout instead of including both versions.

## Header and layout deltas

[EVIDENCE] `Phase::backName` must be 32 bytes (the SB2 header has 36). Target `ChangePhase` uses a `0x1bac` row stride; with the filename at `+4`, two dwords at `+36/+40`, and 40 table button records of 176 bytes, the prefix is 44 bytes and the row is `44 + 40*176 = 7084 (0x1bac)` bytes.

| CEvent offset | Field | Evidence |
|---:|---|---|
| `+0x6d14` | `m_nbVies` | SetNbVies |
| `+0x6d18` | `m_speed` | GetSpeed / SetSpeed |
| `+0x6d1c` | `m_mission` | SetMission |
| `+0x6d20` | `m_private` mission slot | SetMission |
| `+0x6d24` | `m_multi` mission slot | SetMission / lobby handlers |
| `+0x6d28` | `m_phase` | GetPhase / event handlers |
| `+0x6d2c` | `m_index` | ChangePhase table access |
| `+0x6d30` | unknown integer | ClearGamer; semantic label open |
| `+0x6d34` | `m_bDrawMap` | TreatEventBase / ChangePhase |
| `+0x6d38` | `m_bPrivate` | IsPrivate / SetMission |
| `+0x6d3c` | `m_bMulti` | IsMulti / network paths |
| `+0x6d40` | `m_bBuildOfficialMissions` | Build toolbar and mission paths |
| `+0x6d44` | `m_bFullScreen` | SetFullScreen |
| `+0x6d48` | `m_mouseType` | SetMouseType |
| `+0x6d4c` | `m_hInstance` | Create / constructor |
| `+0x6d50` | `m_hWnd` | GetMousePos / StopMovie |
| `+0x6d54` | `m_pPixmap` | member accesses |
| `+0x6d58` | `m_pDecor` | FlushInput / event handlers |
| `+0x6d5c` | `m_pSound` | member accesses |
| `+0x6d60` | `m_pNetwork` | network handlers |
| `+0x6d64` | `m_pMovie` | StopMovie / IsMovie |
| `+0x6d68` | `m_movieToStart[260]` | MovieToStart |
| `+0x6e6c` | `m_phaseAfterMovie` | target member access |
| `+0x6e70` | `m_buttons[40]` | constructor array call; stride `0xf4` |
| `+0x9490` | `m_bRunMovie` | IsMovie |
| `+0x9494` | `m_keyPress` | FlushInput |
| `+0x9498` | `m_menu` | constructor call |

[EVIDENCE] Required `event.h` changes: use `Phase::backName[32]`; retain mission slots at `+0x6d1c/+0x6d20/+0x6d24`; place `CButton[40]` at `+0x6e70`, run-movie/key fields at `+0x9490/+0x9494`, and `CMenu` at `+0x9498`. Remove the SB2-only `m_menuIndex`, `m_menuDecor[11]`, and `m_9840[32]` between `m_96b8[344]` and `m_oldMousePos`; target `m_oldMousePos` is at `+0x9810`. No CEvent-relative `CJauge` constructor or offset was proven.
[EVIDENCE] The tail layout after `m_oldMousePos` continues with `m_bMousePress +0x9818`, `m_bHili +0x981c`, file arrays at `+0x9820/+0x9848`, tooltip point at `+0x9870`, tooltip strings at `+0x9878/+0x98aa`, mouse sprite at `+0x98dc`, rank/cheat state at `+0x98f4/+0x98f8`, all-missions flag at `+0x9904`, highlighted-info flag at `+0x9908`, retry phase/count at `+0x990c/+0x9910`, and demo-play flag at `+0x9918`. The private copy retains later demo/chat members after those anchors.

[EVIDENCE] Required `network.h` change: `CNetwork::m_dpid` is at `+0x20`, `m_players` starts at `+0x24`, and `NetPlayer` stride is `0x20` (four records). The target reads the local DPID at `+0x20` and addresses player records from `+0x24`.

[EVIDENCE] Required `decor.h` change: `GetDoors` and `SetDoors` take `short*`; the internal 200-byte region is `short m_doors[100]`. Target functions at `0x8620` and `0x8650` copy exactly 100 16-bit entries from/to `this+0xcdd4`. This leaves the member byte size and following offsets unchanged.

[EVIDENCE] Required `def.h` changes: the target build toolbar uses `WM_DECOR1`-`WM_DECOR8` (`0x414`-`0x41b`); `src/def.h` stops at five. The target phase messages use the SB1 sequence beginning `INIT/PLAY/BUILD = 0x5f4/0x5f5/0x5f6` and ending `INTRO1/INTRO2 = 0x635/0x636`; the current `src/def.h` phase ordering differs. These numeric values are target evidence; several enum names in the private copy are inherited SB2 labels and remain hypotheses.

## Phase table

[EVIDENCE] The table at RVA `0x345d0` has 32 rows of `0x1bac` bytes (226,688 bytes), with 199 nonzero button records. `scratch/event/rebuild_phase_table.py` emits ordinary C++ aggregate initializers. VC5 SP3 output compares exactly to target; SHA-256 `47d4c002a0b46f72fa0f75ab0f4a92117d79c8b55e04a9f61b7492d9601b4f42`.

[HYPOTHESIS] `bCDrom` and `bLocked` are inherited field names. Their target positions and values are exact, but the semantic labels are not independently established.

## Strict function matches

[EVIDENCE] 47 unique target functions are strict, covering 2,986 original body bytes. Candidate symbol spans can include bytes beyond the Ghidra function body; the table shows the comparator byte span separately from original body coverage. `BuildUp` is counted once at `0x1b030`; the identical PlayDown/PlayMove/PlayUp candidates remain unresolved aliases.

| Candidate function | Target RVA | Original body bytes | Candidate symbol bytes checked |
|---|---:|---:|---:|
| `OutputNetDebug` | `0x163e0` | 84 | 96 |
| `GetMousePos` | `0x16440` | 59 | 64 |
| `SetFullScreen` | `0x16480` | 13 | 16 |
| `SetMouseType` | `0x16490` | 13 | 16 |
| `GetButtonIndex` | `0x16500` | 80 | 80 |
| `GetState` | `0x16550` | 48 | 48 |
| `SetState` | `0x16580` | 47 | 48 |
| `SetEnable` | `0x165b0` | 47 | 48 |
| `SetHide` | `0x165e0` | 47 | 48 |
| `GetMenu` | `0x16610` | 48 | 48 |
| `SetMenu` | `0x16640` | 47 | 48 |
| `SetIconMenu` | `0x16670` | 52 | 64 |
| `FlushInput` | `0x166f0` | 24 | 32 |
| `NetCreate` | `0x168f0` | 41 | 48 |
| `NetEnumSessions` | `0x16920` | 34 | 48 |
| `CreateButtons` | `0x16950` | 278 | 288 |
| `NetSendLobby` | `0x16ce0` | 78 | 80 |
| `AddCheatCode` | `0x16eb0` | 109 | 112 |
| `DrawTextCenter` | `0x16f20` | 74 | 80 |
| `MouseSprite` | `0x18cf0` | 68 | 80 |
| `WaitMouse` | `0x18d40` | 111 | 112 |
| `HideMouse` | `0x18db0` | 121 | 128 |
| `DisableMouse` | `0x18e30` | 111 | 112 |
| `MouseOnButton` | `0x190d0` | 153 | 160 |
| `SearchPhase` | `0x19170` | 50 | 64 |
| `IsPrivate` | `0x191e0` | 7 | 16 |
| `IsMulti` | `0x191f0` | 7 | 16 |
| `SetMission` | `0x19250` | 59 | 64 |
| `GetPhase` | `0x1ae20` | 7 | 16 |
| `BuildUp` | `0x1b030` | 8 | 16 |
| `MouseRelease` | `0x1b040` | 19 | 32 |
| `MouseCapture` | `0x1b060` | 19 | 32 |
| `IsMouseRelease` | `0x1b080` | 7 | 16 |
| `BuildMove` | `0x1c070` | 55 | 64 |
| `StopMovie` | `0x1c210` | 82 | 96 |
| `IsMovie` | `0x1c270` | 7 | 16 |
| `ClearGamer` | `0x1c600` | 79 | 80 |
| `SetNbVies` | `0x1c650` | 13 | 16 |
| `GetSpeed` | `0x1c660` | 7 | 16 |
| `SetSpeed` | `0x1c670` | 23 | 32 |
| `DemoRecStart` | `0x1c690` | 94 | 96 |
| `DemoPlayStop` | `0x1c9a0` | 92 | 96 |
| `DemoRecEvent` | `0x1cac0` | 87 | 96 |
| `ChatFlush` | `0x1cb20` | 21 | 32 |
| `ChatSend` | `0x1cc10` | 343 | 352 |
| `GetLastMousePos` | `0x1d1b0` | 24 | 32 |
| `TreatEvent` | `0x1d1d0` | 89 | 96 |

[EVIDENCE] `CreateButtons` is strict at `0x16950`; its target body is 278 bytes; the candidate symbol span checked is 288 bytes, ten beyond the Ghidra body boundary. `TreatEvent` is strict at `0x1d1d0` (89 original body bytes). `NetSendLobby` and `ChatSend` are now strict after unique body and callee evidence was supplied.

## Relocation-unverified and ambiguous functions

[EVIDENCE] The previous 17-entry relocation-unverified set has been reduced to these eight current cases. None is counted as strict.

| Candidate | Target hit | Classification and evidence |
|---|---|---|
| `CEvent::~CEvent` | `0x16360` | All function calls resolve, including `WriteInfo`, `CMenu::~CMenu`, `CButton` destruction, and the array destructor. Remaining relocations are `$L38353` to `0x42bf89` and three `__except_list` references. `$L38353` is at the end of the mapped `Unwind@0042bf7b` body; no unique reconstructed definition proves the address. `__except_list` has no target VA in the executable. |
| `SetDemo` | `0x16580`, `0x165b0`, `0x165e0`, `0x16640` | Ambiguous wildcard hits against the already strict `SetState`, `SetEnable`, `SetHide`, and `SetMenu` bodies; the candidate wrapper cannot be assigned uniquely. |
| `NetDraw`, `DrawMap` | `0x16e80` | Wildcard relocation collision only. Target pseudocode at `0x16e80` performs the SB1 team update via `NetSearchPlayer`/`CDecor::SetTeam`; the SB2-derived candidate aliases do not prove this mapping. |
| `PlayDown`, `PlayMove`, `PlayUp` | `0x1b030` | Identical short candidate bodies also match strict `BuildUp`; ambiguous aliases, counted once under `BuildUp`. |
| `CheckWorld1` | `0x13b0c`, `0x21520` | Same bytes occur at two addresses outside the event block; no unique event-TU placement evidence. |

[OPEN] The destructor issue is a precise comparator limitation to route to infra: `cmpfunc.py` cannot bind an external runtime data symbol with no unique reconstructed definition, even when all code bytes and function-call relocations are proven. `__except_list` is the unresolved process/thread exception-chain symbol (`resolved_symbol_va=0`); `$L38353` lacks a unique target definition. No `tools/` changes were made.
[EVIDENCE] None of the eight current cases is an unresolved `__FILE__`/`__LINE__` relocation; the candidate source and its private headers contain neither token.

[EVIDENCE] The supervisor’s previous 17 relocation-unverified candidates now classify as follows: ten are STRICT at their current target entries (`OutputNetDebug` `0x163e0`, `AddCheatCode` `0x16eb0`, `DrawTextCenter` `0x16f20`, `MouseOnButton` `0x190d0`, `SearchPhase` `0x19170`, `NetCreate` `0x168f0`, `NetEnumSessions` `0x16920`, `BuildMove` `0x1c070`, `StopMovie` `0x1c210`, `ClearGamer` `0x1c600`) after unique body/callee/data evidence; `MovieToStart` is now MISMATCH because its `HideMouse` call participates in conflicting global helper bindings (the same conflict affects `RestoreGame`); the other six remain relocation-unverified for the reasons above (`CEvent::~CEvent`, `SetDemo`, `PlayDown`, `PlayMove`, `PlayUp`, `CheckWorld1`). `NetDraw` and `DrawMap` are two additional current wildcard cases, not members of that prior 17.

## Per-function target status

Each row is the SB1 function boundary from `oracle/tu_map.csv`. `MISMATCH (forced)` means the candidate was compared at that RVA but the code did not match; it is not a match.

| Target function / RVA | Oracle body bytes | Candidate status |
|---|---:|---|
| `FUN_00416220` / `0x16220` | 319 | NO HIT |
| `CEvent::~CEvent` / `0x16360` | 113 | RELOC-UNVERIFIED: `~CEvent` |
| `CEvent::OutputNetDebug` / `0x163e0` | 84 | STRICT: `OutputNetDebug` |
| `CEvent::GetMousePos` / `0x16440` | 59 | STRICT: `GetMousePos` |
| `CEvent::SetFullScreen` / `0x16480` | 13 | STRICT: `SetFullScreen` |
| `CEvent::SetMouseType` / `0x16490` | 13 | STRICT: `SetMouseType` |
| `FUN_004164A0` / `0x164a0` | 90 | NO HIT |
| `FUN_00416500` / `0x16500` | 80 | STRICT: `GetButtonIndex` |
| `FUN_00416550` / `0x16550` | 48 | STRICT: `GetState` |
| `FUN_00416580` / `0x16580` | 47 | STRICT: `SetState`; unverified aliases: `SetDemo` |
| `FUN_004165B0` / `0x165b0` | 47 | STRICT: `SetEnable`; unverified aliases: `SetDemo` |
| `FUN_004165E0` / `0x165e0` | 47 | STRICT: `SetHide`; unverified aliases: `SetDemo` |
| `FUN_00416610` / `0x16610` | 48 | STRICT: `GetMenu` |
| `FUN_00416640` / `0x16640` | 47 | STRICT: `SetMenu`; unverified aliases: `SetDemo` |
| `CEvent::SetIconMenu` / `0x16670` | 52 | STRICT: `SetIconMenu` |
| `CEvent::RestoreGame` / `0x166b0` | 61 | MISMATCH (forced): `RestoreGame` |
| `CEvent::FlushInput` / `0x166f0` | 24 | STRICT: `FlushInput` |
| `FUN_00416710` / `0x16710` | 479 | MISMATCH (forced): `ReadInput` |
| `CEvent::NetCreate` / `0x168f0` | 41 | STRICT: `NetCreate` |
| `CEvent::NetEnumSessions` / `0x16920` | 34 | STRICT: `NetEnumSessions` |
| `FUN_00416950` / `0x16950` | 278 | STRICT: `CreateButtons` |
| `FUN_00416A70` / `0x16a70` | 494 | NO HIT |
| `FUN_00416C60` / `0x16c60` | 119 | NO HIT |
| `FUN_00416CE0` / `0x16ce0` | 78 | STRICT: `NetSendLobby` |
| `FUN_00416D30` / `0x16d30` | 43 | NO HIT |
| `FUN_00416D60` / `0x16d60` | 230 | NO HIT |
| `FUN_00416E50` / `0x16e50` | 48 | NO HIT |
| `FUN_00416E80` / `0x16e80` | 36 | RELOC-UNVERIFIED: `NetDraw`, `DrawMap` |
| `AddCheatCode` / `0x16eb0` | 109 | STRICT: `AddCheatCode` |
| `CEvent::DrawTextCenter` / `0x16f20` | 74 | STRICT: `DrawTextCenter` |
| `FUN_00416F70` / `0x16f70` | 7419 | MISMATCH (forced): `DrawButtons` |
| `FUN_00418C70` / `0x18c70` | 123 | NO HIT |
| `FUN_00418CF0` / `0x18cf0` | 68 | STRICT: `MouseSprite` |
| `FUN_00418D40` / `0x18d40` | 111 | STRICT: `WaitMouse` |
| `FUN_00418DB0` / `0x18db0` | 121 | STRICT: `HideMouse` |
| `FUN_00418E30` / `0x18e30` | 111 | STRICT: `DisableMouse` |
| `FUN_00418EA0` / `0x18ea0` | 547 | NO HIT |
| `FUN_004190D0` / `0x190d0` | 153 | STRICT: `MouseOnButton` |
| `FUN_00419170` / `0x19170` | 50 | STRICT: `SearchPhase` |
| `FUN_004191B0` / `0x191b0` | 44 | NO HIT |
| `CEvent::IsPrivate` / `0x191e0` | 7 | STRICT: `IsPrivate` |
| `CEvent::IsMulti` / `0x191f0` | 7 | STRICT: `IsMulti` |
| `FUN_00419200` / `0x19200` | 71 | NO HIT |
| `CEvent::SetMission` / `0x19250` | 59 | STRICT: `SetMission` |
| `FUN_00419290` / `0x19290` | 113 | NO HIT |
| `CEvent::ChangePhase` / `0x19310` | 6919 | MISMATCH (forced): `ChangePhase` |
| `CEvent::GetPhase` / `0x1ae20` | 7 | STRICT: `GetPhase` |
| `FUN_0041AE30` / `0x1ae30` | 59 | NO HIT |
| `CEvent::MovieToStart` / `0x1ae70` | 77 | MISMATCH (forced): `MovieToStart` |
| `FUN_0041AEC0` / `0x1aec0` | 146 | NO HIT |
| `FUN_0041AF60` / `0x1af60` | 87 | NO HIT |
| `FUN_0041AFC0` / `0x1afc0` | 97 | NO HIT |
| `FUN_0041B030` / `0x1b030` | 8 | STRICT: `BuildUp`; unverified aliases: `PlayDown`, `PlayMove`, `PlayUp` |
| `FUN_0041B040` / `0x1b040` | 19 | STRICT: `MouseRelease` |
| `FUN_0041B060` / `0x1b060` | 19 | STRICT: `MouseCapture` |
| `FUN_0041B080` / `0x1b080` | 7 | STRICT: `IsMouseRelease` |
| `FUN_0041B090` / `0x1b090` | 181 | NO HIT |
| `FUN_0041B150` / `0x1b150` | 195 | NO HIT |
| `FUN_0041B220` / `0x1b220` | 2854 | NO HIT |
| `FUN_0041BD50` / `0x1bd50` | 799 | NO HIT |
| `CEvent::BuildMove` / `0x1c070` | 55 | STRICT: `BuildMove` |
| `FUN_0041C0B0` / `0x1c0b0` | 349 | NO HIT |
| `CEvent::StopMovie` / `0x1c210` | 82 | STRICT: `StopMovie` |
| `CEvent::IsMovie` / `0x1c270` | 7 | STRICT: `IsMovie` |
| `FUN_0041C280` / `0x1c280` | 413 | NO HIT |
| `FUN_0041C420` / `0x1c420` | 475 | NO HIT |
| `CEvent::ClearGamer` / `0x1c600` | 79 | STRICT: `ClearGamer` |
| `CEvent::SetNbVies` / `0x1c650` | 13 | STRICT: `SetNbVies` |
| `CEvent::GetSpeed` / `0x1c660` | 7 | STRICT: `GetSpeed` |
| `CEvent::SetSpeed` / `0x1c670` | 23 | STRICT: `SetSpeed` |
| `FUN_0041C690` / `0x1c690` | 94 | STRICT: `DemoRecStart` |
| `FUN_0041C6F0` / `0x1c6f0` | 230 | NO HIT |
| `FUN_0041C7E0` / `0x1c7e0` | 447 | NO HIT |
| `FUN_0041C9A0` / `0x1c9a0` | 92 | STRICT: `DemoPlayStop` |
| `FUN_0041CA00` / `0x1ca00` | 188 | NO HIT |
| `FUN_0041CAC0` / `0x1cac0` | 87 | STRICT: `DemoRecEvent` |
| `FUN_0041CB20` / `0x1cb20` | 21 | STRICT: `ChatFlush` |
| `FUN_0041CB40` / `0x1cb40` | 76 | NO HIT |
| `FUN_0041CB90` / `0x1cb90` | 123 | NO HIT |
| `FUN_0041CC10` / `0x1cc10` | 343 | STRICT: `ChatSend` |
| `FUN_0041CD70` / `0x1cd70` | 389 | NO HIT |
| `FUN_0041CF00` / `0x1cf00` | 83 | NO HIT |
| `FUN_0041CF60` / `0x1cf60` | 586 | NO HIT |
| `FUN_0041D1B0` / `0x1d1b0` | 24 | STRICT: `GetLastMousePos` |
| `FUN_0041D1D0` / `0x1d1d0` | 89 | STRICT: `TreatEvent` |
| `FUN_0041D230` / `0x1d230` | 6616 | MISMATCH (forced): `TreatEventBase` |

## Large handlers and residuals

| Function | Target body | Candidate body | Current finding |
|---|---:|---:|---|
| `CreateButtons` | 278 B | 288 B | Strict; removing the SB2-only icon-menu increment matched. |
| `DrawButtons` | 7,419 B | 7,648 B | Still +229 B. Version text is now `Version 1.0`; the top cheat-text gate excludes only Insert and Bye, saving 16 B. The first mismatch is the prologue: target reserves `0xe4` bytes and uses EBP as `this`; candidate reserves `0xdc` and uses EBX. Remaining differences include branches, loads, calls, and conditions. |
| `ReadInput` | 479 B | 480 B | Still +1 B. Signed joystick-axis casts fixed four signed/unsigned branch mismatches. Target button checks load the byte to AL and update the key mask with dword registers; baseline candidate uses DL and several byte-AL masks. The local-temp probe shrank to 464 B but stayed mismatched. |
| `LoadImageFromDisc` | — | 176 B | Candidate-only helper; target image-cache work is inline in `ChangePhase` under the target `ChangePhase [LoadImages]` path. |
| `TreatEventBase` | 6,616 B | 6,880 B | Still +264 B. Player-record layout and lobby map offset are corrected; the first player-search compare/branch layout remains different. |
| `ChangePhase` | 6,919 B | 6,848 B | Still 71 B shorter. Correcting the gamer door buffer to 100 shorts matches the target 16-bit loop. Moving that live buffer to function scope changes the frame from `0x39c` to `0x464`; target is `0x45c`, leaving an 8-byte frame difference. |
| `TreatEvent` | 89 B | 96 B | Strict wrapper at `0x1d1d0`; target body-byte coverage remains 89 B. |

[EVIDENCE] For `TreatEventBase`, indexed do/goto, indexed while, and pointer loop variants all emitted 6,880-byte bodies; the early player-search compare remained different. The latest source retains the original indexed do/goto form. These loop-form trials were negative under VC5 SP3 `/O2 /GX /ML /Gd /Gy`.

## Compiler findings

| Trigger | Source change and measured effect | Strict result | Rule / classification |
|---|---|---|---|
| SB2 phase row stride differed from the target table | `backName[36]` to `[32]`; regenerated normal aggregate initializers. | Exact 226,688-byte table equality. | `VC5-SEM-01`; blupi-specific layout fact. |
| `CreateButtons` emitted an SB2-only icon-menu count branch | Removed the conditional count adjustment absent in SB1. | `CreateButtons` strict at `0x16950`. | `VC5-SEM-01`; blupi-specific. |
| `ReadInput` compared DWORD axes as unsigned | Added signed `int` casts for X/Y threshold checks. | Four branch-condition mismatches resolved; body remains 480/479 B. | `VC5-SEM-01`; blupi-specific behavior. |
| `ReadInput` button-three update collapsed an intermediate store | Tried `m_keyPress &= ~KEY_UP` against the local-value expression. | No code change; store and byte-width differences remain. | `VC5-NEG-02`; negative result. |
| `ReadInput` button-one mask used enum complements | Replaced `~(KEY_DOWN | KEY_UP)` with the target-equivalent `0xfffffff3`; separately tested reusing `newKeyPress` plus a member store. | Explicit mask was codegen-inert (480 B); the local-temp form changed register allocation and reduced the body to 464 B, still mismatched against 479 B. The target AL load and dword mask/store pattern was not recovered. | `VC5-EXP-03`; negative source-form/width result. |
| `TreatEventBase` player-slot loop differed at its first compare | Tested indexed/pointer `while` and do/goto forms. | Same 6,880-B body; no target match. | `VC5-CTL-01`; negative source-form result. |
| `ChangePhase` gamer door table was treated as 200 bytes | Used `short[100]` and changed private `GetDoors`/`SetDoors`/member types to 100 shorts from target `0x8620/0x8650`. Moved the used door buffer to function scope. | Correct 16-bit scan; frame improved from 192 B under target to 8 B over target; function still mismatch. | `VC5-LOC-03`, `VC5-SEM-01`; blupi-specific structure/type evidence. |
| `DrawButtons` showed SB2 version and phase exclusions | Changed to `Version 1.0` and target Insert/Bye exclusions. | 16-B reduction; residual +229 B. | `VC5-SEM-01`; blupi-specific. |
| `AddUserExportPath` contained a malformed backslash literal | Escaped the source backslash before `User`. | Target string at RVA `0x77ec8` matches; no strict function established. | `VC5-SEM-01`; blupi-specific data correction. |

[EVIDENCE] No dummy declarations, padding, unused code, forced bytes, or assembly were added. The candidate is scratch-only; target code and headers in `src/` were not changed.

## Remaining work

[OPEN] Continue the large-handler pass from the earliest residual: reconcile `DrawButtons` against SB1 behavior and source order, then address the remaining `TreatEventBase` and `ChangePhase` branch/layout differences. `ReadInput` source-form trials above did not improve the 480/479-byte residual. Route the destructor external-symbol limitation to infra; keep the seven ambiguous alias/wildcard functions out of strict counts unless new location evidence appears.
