# Event translation unit reconstruction

**Status: partial.** After refreshing from integrated `src/event.cpp`, the current scratch candidate has 54 STRICT symbols, 1 RELOC-UNVERIFIED symbol, and 54 MISMATCH symbols. STRICT bodies occupy 50 unique target RVAs and 3,232 unique original function-body bytes; the matching candidate spans total 3,584 bytes. The exact SB1 phase table remains intact. Current large-handler residuals are DrawButtons, ReadInput, TreatEventBase, and ChangePhase.

## Scope and comparison

[EVIDENCE] The target block is `0x16220`-`0x1d230`, with 86 mapped functions and 34,724 Ghidra body bytes. The integrated source is `src/event.cpp`; this worker candidate is `scratch/tu/event.cpp`, derived mainly from SB2 `event.cpp`. Its active quoted includes resolve to 19 byte-identical canonical header copies under `scratch/tu/event_include/current_src/`; stale flat `scratch/tu` headers and the older private SB1 override directory are not used. `src/` has the full game TU set and one canonical header set.

[EVIDENCE] The scratch candidate was refreshed byte-for-byte from the integrated `src/event.cpp` before the current round. Its active quoted includes point to 19 freshly synchronized copies of canonical `src/*.h` under `scratch/tu/event_include/current_src/`; there are no private header overrides in this run. The final rerun resynchronized late canonical changes to `def.h`, `pixmap.h`, and `sound.h`; the latter is included transitively by `decor.h`. Verification used VC5 SP3 (11.00.7022), `/O2 /GX /ML /Gd /Gy`, and `nullptr=0`.

```powershell
.pyenv\python.exe tools\verify.py --tu src/event.cpp=scratch\tu\event.cpp --build-dir event_sound_refresh
```

[EVIDENCE] The isolated run reports 109 event candidate functions: 54 STRICT, 1 RELOC-UNVERIFIED, 54 MISMATCH, and no FOLDED status. There are 50 unique STRICT target RVAs totaling 3,232 original function-body bytes; candidate spans account for 3,584 matched bytes. The shared bodies at `0x16e80` and `0x1b030` are each counted once.

[EVIDENCE] A merge experiment using current `src/` headers plus about 73 SB2 declarations failed with over 100 compiler errors: phase/text identifiers and duplicate cases, `POINT`/`RECT` constructor differences, and changed `CDecor`, `CPixmap`, and read/write APIs. It was discarded; no `src/` header was edited.
[EVIDENCE] The focused hybrid-header compile reproduced concrete conflicts: `src/def.h` macros `CHBACK`, `CHOBJECT`, `WM_UPDATE`, `WM_DECOR1`-`WM_DECOR5`, and `WM_WRITE0`-`WM_WRITE9` expand inside same-named SB2 `def.h` enum declarations; `src/movie.h` has no include guard and conflicts with its inclusion through candidate `event.h`; `CNetwork` and `DPID` are undeclared when `event.h` precedes `network.h`. `src/pixmap.h` also declares four-argument `CPixmap::Create`, versus six arguments in the SB2 copy; the SB2 declarations for `CDecor::GetDoors/SetDoors(short*)` were absent from the earlier canonical header and have since been integrated. Keep one `def.h` and integrate measured SB1 constants/layout instead of including both versions.

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

[EVIDENCE] The current canonical `src/event.h` already has the proven SB1 layout: `Phase::backName[32]`; mission slots at `+0x6d1c/+0x6d20/+0x6d24`; `CButton[40]` at `+0x6e70`; run-movie/key fields at `+0x9490/+0x9494`; and `CMenu` at `+0x9498`. The SB2-only `m_menuIndex`, `m_menuDecor[11]`, and `m_9840[32]` are absent; `m_oldMousePos` is at `+0x9810`. No CEvent-relative `CJauge` constructor or offset was proven, so no further `event.h` delta is requested.
[EVIDENCE] The tail layout after `m_oldMousePos` continues with `m_bMousePress +0x9818`, `m_bHili +0x981c`, file arrays at `+0x9820/+0x9848`, tooltip point at `+0x9870`, tooltip strings at `+0x9878/+0x98aa`, mouse sprite at `+0x98dc`, rank/cheat state at `+0x98f4/+0x98f8`, all-missions flag at `+0x9904`, highlighted-info flag at `+0x9908`, retry phase/count at `+0x990c/+0x9910`, and demo-play flag at `+0x9918`. The private copy retains later demo/chat members after those anchors.

[EVIDENCE] The current canonical `src/network.h` already puts `CNetwork::m_dpid` at `+0x20` and `m_players` at `+0x24`; `NetPlayer` stride is `0x20` (four records). The target reads the DPID at `+0x20` and addresses player records from `+0x24`. The private candidate `network.h` is byte-identical to this canonical header.

[EVIDENCE] The current canonical `src/decor.h` already declares `GetDoors(short*)`, `SetDoors(short*)`, and `short m_doors[100]`. Target functions at `0x8620` and `0x8650` copy 100 16-bit entries from/to `this+0xcdd4`; the byte size and following offsets are unchanged.

[EVIDENCE] The canonical `src/def.h` now contains the target phase values (`INSERT=0x607`, `GAMER=0x609`, `BYE=0x614`, `DOQUIT=0x634`, `INTRO1/INTRO2=0x635/0x636`) and cursor values `SPRITE_WAIT=11`, `SPRITE_EMPTY=12`; `WaitMouse` and `HideMouse` are STRICT with these headers. No event-specific `event.h` or `def.h` delta remains for infra to integrate. Canonical `event.h` also carries `Phase::backName[32]`, mission slots at `+0x6d1c/+0x6d20/+0x6d24`, `CButton[40]` at `+0x6e70`, and `CMenu` at `+0x9498`. The current `sound.h` exposes the real `PlayImage` and `PlaySoundDS` overloads used by the synchronized include set.

## Phase table

[EVIDENCE] The table at RVA `0x345d0` has 32 rows of `0x1bac` bytes (226,688 bytes), with 199 nonzero button records. `scratch/event/rebuild_phase_table.py` emits ordinary C++ aggregate initializers. VC5 SP3 output compares exactly to target; SHA-256 `47d4c002a0b46f72fa0f75ab0f4a92117d79c8b55e04a9f61b7492d9601b4f42`.

[HYPOTHESIS] `bCDrom` and `bLocked` are inherited field names. Their target positions and values are exact, but the semantic labels are not independently established.

## Strict function matches

[EVIDENCE] The current run has 50 unique strict target RVAs and 3,232 unique original function-body bytes. Candidate spans can extend beyond Ghidra body boundaries; the table reports both target body bytes and candidate symbol span. `NetDraw`/`DrawMap` share the body at `0x16e80` (36 bytes), and `BuildUp`/`PlayDown`/`PlayMove`/`PlayUp` share the body at `0x1b030` (8 bytes); aliases are counted once.

| Candidate function | Target RVA | Original body bytes | Candidate symbol bytes |
|---|---:|---:|---:|
| `CEvent::~CEvent` | `0x16360` | 113 | 128 |
| `CEvent::OutputNetDebug` | `0x163e0` | 84 | 96 |
| `CEvent::GetMousePos` | `0x16440` | 59 | 64 |
| `CEvent::SetFullScreen` | `0x16480` | 13 | 16 |
| `CEvent::SetMouseType` | `0x16490` | 13 | 16 |
| `CEvent::GetButtonIndex` | `0x16500` | 80 | 80 |
| `CEvent::GetState` | `0x16550` | 48 | 48 |
| `CEvent::SetState` | `0x16580` | 47 | 48 |
| `CEvent::SetEnable` | `0x165b0` | 47 | 48 |
| `CEvent::SetHide` | `0x165e0` | 47 | 48 |
| `CEvent::GetMenu` | `0x16610` | 48 | 48 |
| `CEvent::SetMenu` | `0x16640` | 47 | 48 |
| `CEvent::SetIconMenu` | `0x16670` | 52 | 64 |
| `CEvent::FlushInput` | `0x166f0` | 24 | 32 |
| `CEvent::NetCreate` | `0x168f0` | 41 | 48 |
| `CEvent::NetEnumSessions` | `0x16920` | 34 | 48 |
| `CEvent::CreateButtons` | `0x16950` | 278 | 288 |
| `CEvent::NetSendLobby` | `0x16ce0` | 78 | 80 |
| `CEvent::NetDraw` | `0x16e80` | 36 | 48 |
| `AddCheatCode` | `0x16eb0` | 109 | 112 |
| `CEvent::DrawTextCenter` | `0x16f20` | 74 | 80 |
| `CEvent::MouseSprite` | `0x18cf0` | 68 | 80 |
| `CEvent::WaitMouse` | `0x18d40` | 111 | 112 |
| `CEvent::HideMouse` | `0x18db0` | 121 | 128 |
| `CEvent::DisableMouse` | `0x18e30` | 111 | 112 |
| `CEvent::MouseOnButton` | `0x190d0` | 153 | 160 |
| `CEvent::SearchPhase` | `0x19170` | 50 | 64 |
| `CEvent::IsPrivate` | `0x191e0` | 7 | 16 |
| `CEvent::IsMulti` | `0x191f0` | 7 | 16 |
| `CEvent::SetMission` | `0x19250` | 59 | 64 |
| `CEvent::GetPhase` | `0x1ae20` | 7 | 16 |
| `CEvent::CurrentWrite` | `0x1afc0` | 97 | 112 |
| `CEvent::BuildUp` | `0x1b030` | 8 | 16 |
| `CEvent::MouseRelease` | `0x1b040` | 19 | 32 |
| `CEvent::MouseCapture` | `0x1b060` | 19 | 32 |
| `CEvent::IsMouseRelease` | `0x1b080` | 7 | 16 |
| `CEvent::BuildMove` | `0x1c070` | 55 | 64 |
| `CEvent::StopMovie` | `0x1c210` | 82 | 96 |
| `CEvent::IsMovie` | `0x1c270` | 7 | 16 |
| `CEvent::ClearGamer` | `0x1c600` | 79 | 80 |
| `CEvent::SetNbVies` | `0x1c650` | 13 | 16 |
| `CEvent::GetSpeed` | `0x1c660` | 7 | 16 |
| `CEvent::SetSpeed` | `0x1c670` | 23 | 32 |
| `CEvent::DemoRecStart` | `0x1c690` | 94 | 96 |
| `CEvent::DemoPlayStop` | `0x1c9a0` | 92 | 96 |
| `CEvent::DemoRecEvent` | `0x1cac0` | 87 | 96 |
| `CEvent::ChatFlush` | `0x1cb20` | 21 | 32 |
| `CEvent::ChatSend` | `0x1cc10` | 343 | 352 |
| `CEvent::GetLastMousePos` | `0x1d1b0` | 24 | 32 |
| `CEvent::TreatEvent` | `0x1d1d0` | 89 | 96 |

## Current unresolved and alias classifications

[EVIDENCE] In `event_sound_refresh`, only `SetDemo` is RELOC-UNVERIFIED. Its 48-byte body has four byte-identical hits at `0x16580`, `0x165b0`, `0x165e0`, and `0x16640`, all owned by strict setter wrappers. The verifier reports `function location is ambiguous` before using its `CButton::SetDemo` relocation to choose among them. This is a precise comparator location-selection limitation; do not count it as a separate strict body.

[EVIDENCE] `CEvent::~CEvent` is now STRICT at `0x16360` (113 target body bytes, 128 candidate bytes). The current verifier proves the local `$L37042` bytes, treats all three `__except_list` relocations as the VC5 absolute `FS:0` exception-chain symbol, and verifies the destructor/helper call targets. The earlier external-data verifier limitation is cleared; no tool change is requested.

[EVIDENCE] `NetEnumSessions` is STRICT at `0x16920`; its `CNetwork::EnumSessions` relocation resolves to `0x20bd0`, which matches the oracle. `CheckWorld1` remains a MISMATCH under canonical `REQUIRE_CD=TRUE`; the scratch-only `FALSE` variant yields a 16-byte unconditional-true body with duplicate hits at `0x13b0c` and `0x21520`, both outside the event block. `MovieToStart` and `RestoreGame` remain mismatches.

[OPEN] The remaining verifier limitation in the current event run is `SetDemo`'s ambiguous identical-body location. There is no remaining destructor or `__FILE__`-like data limitation.

## Per-function target status

This inventory includes the current event run and the cross-TU/alias overrides above. Each row is the SB1 function boundary from `oracle/tu_map.csv`. `MISMATCH (forced)` means the candidate was compared at that RVA but the code did not match; it is not a match.

| Target function / RVA | Oracle body bytes | Candidate status |
|---|---:|---|
| `FUN_00416220` / `0x16220` | 319 | NO HIT |
| `CEvent::~CEvent` / `0x16360` | 113 | STRICT: `~CEvent` (128-byte candidate span) |
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
| `FUN_00416E80` / `0x16e80` | 36 | STRICT shared body: `NetDraw` / `DrawMap` (count once) |
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
| `FUN_0041AFC0` / `0x1afc0` | 97 | STRICT: `CurrentWrite` |
| `FUN_0041B030` / `0x1b030` | 8 | STRICT shared body: `BuildUp` / `PlayDown` / `PlayMove` / `PlayUp` (count once) |
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

## Large handlers: current measurements

| Function | Target body | Candidate span | Current finding |
|---|---:|---:|---|
| `CreateButtons` | 278 B | 288 B | STRICT at `0x16950`. |
| `DrawButtons` | 7,419 B | 7,392 B | Candidate is 27 B shorter. The prologue and `0xe4` frame match; private layout fixes and INFO title assignment order grow the fixed prefix to 4,886 bytes. First fixed divergence is `+0x1316`: target clears EDX before the suffix copy and loads page/file indices after it; candidate loads page offset into EDX before the copy. |
| `ReadInput` | 479 B | 480 B | Still +1 B in total span, but not a one-byte code difference. Target and candidate first differ in a downstream branch displacement at `+0x22`; the first substantive button-mask difference is at `+0x114`: target loads the low byte into AL and tests AL, while candidate uses DL. Signed axis casts fixed four condition mismatches; earlier BYTE, mask, temp-store, and TU-context probes remain negative. |
| `LoadImageFromDisc` | n/a | 176 B | Candidate-only helper; target image-cache work is inline in `ChangePhase` under the target `[LoadImages]` path. |
| `TreatEventBase` | 6,616 B | 6,944 B | Residual +328 B. The counted player scan matches through `+0x94`. Target-shaped Escape/Return phase chains flip the `+0xac` SIB and grow the fixed prefix to 730 bytes; first raw difference is the `ja` displacement at `+0x2da`. Candidate's intervening `<= 0x100` message-dispatch region is 13 bytes larger than target. |
| `ChangePhase` | 6,919 B | 6,848 B | Residual -71 B. The prologue now matches the target `0x45c` frame after moving the 12-byte lobby packet into `WM_PHASE_DP_CANCELMULTI` scope. The early-exit comparison now uses target phase `0x634`. The first divergence is at `+0x1c`: candidate `str` `[esp+0x10c]`, target `str` `[esp+0xa8]`. Controlled local-array probes document behavior but do not predict the target's mixed slot map. |
| `TreatEvent` | 89 B | 96 B | STRICT at `0x1d1d0`; 89 target body bytes. |

[EVIDENCE] Fresh isolated verification gives `DrawButtons` 7,392/7,419 MISMATCH with a 4,886-byte fixed prefix and first difference at `+0x1316`; `ReadInput` is 480/479 MISMATCH, `TreatEventBase` is 6,944/6,616 MISMATCH, and `ChangePhase` is 6,848/6,919 MISMATCH. `TreatEvent` and `CreateButtons` remain STRICT.

## Compiler findings

| Trigger | Source change and measured effect | Strict result | Rule / classification |
|---|---|---|---|
| SB2 phase row stride differed from the target table | `backName[36]` to `[32]`; regenerated normal aggregate initializers. | Exact 226,688-byte table equality. | `VC5-SEM-01`; blupi-specific layout fact. |
| `CreateButtons` emitted an SB2-only icon-menu count branch | Removed the conditional count adjustment absent in SB1. | `CreateButtons` strict at `0x16950`. | `VC5-SEM-01`; blupi-specific. |
| `ReadInput` compared DWORD axes as unsigned | Added signed `int` casts for X/Y threshold checks. | Four branch-condition mismatches resolved; body remains 480/479 B. | `VC5-SEM-01`; blupi-specific behavior. |
| `ReadInput` button-three update collapsed an intermediate store | Tried `m_keyPress &= ~KEY_UP` against the local-value expression. | No code change; store and byte-width differences remain. | `VC5-NEG-02`; negative result. |
| `ReadInput` retained the same body under TU symbol-count context changes | Scratch-only VC5 count probes at top/end/before the function used 0-2 filler declarations; top/before also tried 7-14. | Every probed body remained MISMATCH at 480 B; the target AL/dword button-mask pattern was not restored. | `VC5-CTX-01..04`; negative context-probe result. |
| `ReadInput` button-one mask used enum complements | Replaced `~(KEY_DOWN | KEY_UP)` with the target-equivalent `0xfffffff3`; separately tested reusing `newKeyPress` plus a member store. | Explicit mask was codegen-inert (480 B); the local-temp form changed register allocation and reduced the body to 464 B, still mismatched against 479 B. The target AL load and dword mask/store pattern was not recovered. | `VC5-EXP-03`; negative source-form/width result. |
| `TreatEventBase` player-slot loop differed at its first compare | Tested indexed/pointer `while` and do/goto forms. | Same 6,880-B body; no target match. | `VC5-CTL-01`; negative source-form result. |
| `TreatEventBase` Escape phase dispatch and the `+0xac` SIB residue | With VC5 SP3 `/W3 /GX /O2 /ML /Gd /Gy`, `vc5probe` end counts 0-64 step 1 and 64-512 step 16, plus top/before:`TreatEventBase` counts 0-64 step 1, never flipped the SIB. Changing the same function's Escape block from a phase `switch` to the target-shaped PLAY `if` / exclusion `else if` chain flipped the SIB; a target-shaped nested VK_RETURN `if` chain with the shared QUITPLAYTEST action then retained it. | At instruction `+0xaa`, the SIB byte at `+0xac` changes from `c6 44 10 25 00` to target `c6 44 02 25 00`; fixed prefix grows 172 -> 730 bytes. The body is 6,944/6,616 B, still MISMATCH. | `BLUPI-REG-01`; confirmed SB1 same-function control-flow-shape effect on register allocation. TU-context hypothesis VC5-CTX-05 is negative over all stated ranges. |
| `TreatEventBase` `ja` at `+0x2da` spans a message-dispatch block | Target bytes `0f 87 39 09 00 00` branch to `+0xc17`; candidate bytes `0f 87 46 09 00 00` branch to `+0xc24`. These are the corresponding entries into the later `WM_MOUSEMOVE`/`0x200` dispatcher. | The `WM_KEYDOWN` arm begins at the same `+0x32f` entry in both objects and occupies 13 more bytes in the candidate before the next `0x200` message dispatcher. Target-shaped nested VK_RETURN conditions plus sharing the QUITPLAYTEST action reduced the earlier candidate destination from `+0xcef` to `+0xc24` (216 -> 13 extra bytes); fixed prefix remains 730. Body is 6,944/6,616 B, MISMATCH. | `VC5-CTL-01`; branch layout follows source block order; residual block-size/source-order difference remains unverified. |
| `TreatEventBase` rank-zero cheat toggle had register/store-order mismatch at `+0x41a` | Reordering `bEnable = !member; member = bEnable;` to `member = !member; bEnable = member;` was codegen-inert. Then tested an explicit `if (member) member = FALSE; else member = TRUE; bEnable = member;` form. | The if/else form moves the member store before the ESI copy, matching target order; register choices remain inverted (target input EDX/result ECX, candidate input ECX/result EDX). Fixed-byte differences from `+0x41a` through `+0x6f2` drop 12 -> 6. Body remains 6,944/6,616 B and the first mismatch stays `+0x2da`; no strict match. | `VC5-ORD-01`; measured SB1 condition-shape effect, target source form unverified; negative for simple assignment reordering. |
| `ChangePhase` gamer door table was treated as 200 bytes | Used `short[100]` and changed private `GetDoors`/`SetDoors`/member types to 100 shorts from target `0x8620/0x8650`. Moved the used door buffer to function scope. | Correct 16-bit scan; frame improved from 192 B under target to 8 B over target; function still mismatch. | `VC5-LOC-03`, `VC5-SEM-01`; blupi-specific structure/type evidence. |
| `DrawButtons` showed SB2 version and phase exclusions | Changed to `Version 1.0` and target Insert/Bye exclusions. | 16-B reduction; residual +229 B. | `VC5-SEM-01`; blupi-specific. |
| `AddUserExportPath` contained a malformed backslash literal | Escaped the source backslash before `User`. | Target string at RVA `0x77ec8` matches; no strict function established. | `VC5-SEM-01`; blupi-specific data correction. |
| `ChangePhase` target array slots looked like function/inner/later scope runs | Three 9-array VC5 probes varied root first-use and whether a nested door-owning block overlapped the scratch sub-blocks. First-use order left slots unchanged; a sibling `door` reused `text`, and an owning block prevented reuse but still did not produce the target map. Moving existing `oldPhase` to function scope while retaining its initializer point also left code bytes/frame unchanged. | Candidate scope application remains MISMATCH, 6,848/6,919 B; frame is 8 B over target and the first difference is the prologue. | `BLUPI-LOC-01`; negative transfer to this function / unverified grouping hypothesis. |

| ChangeButtons ready-byte SIB at +0x5fb | Under VC5 SP3 /W3 /GX /O2 /ML /Gd /Gy, tested five source spellings of m_players[playerId].ready (pointer addition, arrow, row pointer, local row pointer, byte-offset indexing), a nested ready-range condition, and target-shaped later GAMEWRITE/READDESIGN blocks. Wide TU context probes were already negative. | Access forms and nested range all retain 3,024 B and prefix 1,531 with SIB 8a 54 0e 25 vs target 8a 54 31 25. Target-shaped later blocks leave the SIB/prefix unchanged; both together yield 3,040/2,854 B. No STRICT match. | BLUPI-REG-01; negative same-function probes for this function; rule remains confirmed transferable. |

### Follow-up compiler findings

| Trigger | Source change and measured effect | Strict result | Rule / classification |
|---|---|---|---|
| Four UI helpers missed before header integration | Canonical `network.h` plus integrated `SPRITE_WAIT=11` and `SPRITE_EMPTY=12` restore `NetSendLobby`, `ChatSend`, `WaitMouse`, and `HideMouse`. | STRICT at `0x16ce0`/78, `0x18d40`/111, `0x18db0`/121, and `0x1cc10`/343 original bytes; 653 unique bytes total. | `VC5-SEM-01`; SB1 layout/constant evidence. |
| `DrawButtons` included a development overlay and undefined pointer subtraction | Removed `DECOMP -- __DATE__`; changed split index to `line2 - m_filenameBuffer[i]` per the target copy into a local buffer. The removal saves 48 candidate bytes; the corrected indexing adds 48. | Still 7,648/7,419 B and MISMATCH; source now follows target behavior more closely. | `VC5-SEM-01`; blupi-specific semantic correction. |
| `ChangePhase` frame exceeded target by 8 bytes | Moved `char netData[12]` from function scope into the `WM_PHASE_DP_CANCELMULTI` case block. | Frame changed `0x464` -> target `0x45c`; body remains 6,848/6,919 B, with the next mismatch at the string-buffer offset. | `VC5-LOC-03`; blupi-specific scope/lifetime evidence. |
| `ChangePhase` early-exit used the wrong configured phase | Replaced the `BYE_SCREEN` conditional (`WM_PHASE_BYE=0x614` in this build) with the target `WM_PHASE_DOQUIT=0x634` check. | The compare immediate now matches target; function remains 6,848/6,919 B. | `VC5-SEM-01`; blupi-specific behavior. |
| `DrawButtons` built the same `(2,2)` point as two separate temporaries | Reused existing `pos` for the background draw and following text draw; assigned the RECT fields through the existing local. | Target setup through byte `+0x13e` now matches aside from relocations; first fixed divergence is RECT staging at `+0x13f`; body remains 7,648/7,419 B. | `VC5-LOC-01`; unverified source-form/local-reuse probe. |
| INFO mission-title centering had an ECX/EDX register-order residue after the private layout correction | Before: `pos.y = 267; pos.x = 250 - lg / 2;` After: `pos.x = 250 - lg / 2; pos.y = 267;` in `event.cpp` under VC5 SP3 `/W3 /GX /O2 /ML /Gd /Gy`. | The native register sequence now matches at `+0xd35`; fixed prefix grows 3,381 -> 4,886 bytes (`+0x1316`). Candidate is 7,392/7,419 B and remains MISMATCH. | `VC5-ORD-01`; confirmed-transferable on SB1. |
| `DrawButtons` highlight index read was scheduled before the session ellipsis copy at `+0x1316` | Kept the inline index expression and tested a post-copy `sessionIndex` local, reversed `m_filePageOffset + i` order, and `&m_filenameBuffer[...][0]`; tested `sprintf(text + 50, "...")` for the copy. VC5 SP3 `/W3 /GX /O2 /ML /Gd /Gy`, canonical `event.h`. | The local, operand order, and explicit buffer address are codegen-inert (prefix 4,886, body 7,392/7,419 B). `sprintf` regresses to prefix 4,245 and body 7,360 B. Target scans the fixed literal then uses `rep movsd/movsb` into `text + 50`, matching the `strcpy` intrinsic; a `strcat` destination scan is inconsistent with that sequence. Earlier top/before-function extern counts 0-64 also left `+0x1316` unchanged. MISMATCH remains. | `VC5-CTX-01`, `VC5-ORD-01`; negative source/context probes. |
| Alternate VC5 compiler builds were proposed as the cause of the commuted `ChangeButtons` / `TreatEventBase` SIBs | Compiled the current `scratch/ev_drawbuttons/event.cpp` with the canonical flags and defines under SP3, SP2, RTM, NT5 1592/1691, SP3 DEU, and SP3 JPN. All seven emit `ChangeButtons` as 3,040 B with prefix 1,531 and `8a 54 0e 25` versus target `8a 54 31 25`; current `TreatEventBase` remains 6,944/6,616 B with prefix 730 and instruction `c6 44 02 25 00`, matching target. Also compiled the earlier switch form from `scratch/ev_treateventbase/event_baseline.cpp` read-only under all seven trees: each remains 6,880 B, prefix 172, instruction `c6 44 10 25 00` versus target `c6 44 02 25 00`. | No tree flips either SIB; both target functions remain non-strict. The 50 accepted event functions have identical fixed code bytes and relocation signatures across all seven trees. | `BLUPI-FLG-01`; negative alternate-tree result for these SIBs. |

### `TreatEventBase` isolated worker follow-up

[EVIDENCE] The isolated candidate is `scratch/ev_treateventbase/event.cpp`, derived from the current `src/event.cpp` and changed only within `CEvent::TreatEventBase`. At RVA `0x1d230` the current best source form remains MISMATCH at 6,944 candidate bytes versus 6,616 target bytes; all 54 previously strict event functions remain strict in the verification run.

[EVIDENCE] Replacing the indexed search loop with a counted `for` loop and explicit `goto` exits reproduces the target scan bytes from `+0x81` through `+0x94`. The first fixed-byte divergence moved from `+0x37` (55-byte prefix) to `+0xac` (172-byte prefix), a gain of 117 bytes. The first remaining byte is in a commuted SIB address for a player record (`candidate 0x10`, target `0x02`); both encode the same effective address.

[EVIDENCE] Three store spellings—`(&m_players[i])->field`, `base + sizeof(NetPlayer) * i`, and `sizeof(NetPlayer) * i + base`—all compiled to the same 6,880-byte body and retained the `+0xac` SIB difference. A pointer-walk loop changed the body to 6,832 bytes and moved the first fixed difference back to `+0x37`; it did not preserve the target loop layout or establish a matching SIB form.

[EVIDENCE] `vc5probe.py probe` with VC5 SP3, `/W3 /GX /O2 /ML /Gd /Gy`, and extern counts 0, 1, and 2 produced the same result at `end`, `top`, and `before:TreatEventBase`: every body remained MISMATCH at 6,880 bytes, with candidate SIB bytes `10 25` versus target `02 25` at `+0xac`. The tested TU-count placements did not flip the SIB order.

[EVIDENCE] Expanded `vc5probe.py probe` for the `+0xac` SIB used `/W3 /GX /O2 /ML /Gd /Gy` and the event TU's current defines/includes. At `end`, counts 0-64 step 1 and 64-512 step 16 (94 objects; count 64 overlaps) all remained 6,880 bytes with instruction bytes `c6 44 10 25 00`; at `top` and `before:TreatEventBase`, every count 0-64 (65 objects each) had the same body size and bytes. The target instruction is `c6 44 02 25 00`; all probe objects' first fixed-byte divergence is `+0xac` (172 identical prefix bytes). None of these TU-context counts flips the SIB. The negative context result rules out VC5-CTX-05 for these ranges; the positive same-function control-flow result is recorded as `BLUPI-REG-01`.

[EVIDENCE] The target pseudocode clears `m_rankCheat` and `m_posCheat` on every keydown path that does not return from completed cheat/text input; the initial candidate cleared them only on an active-cheat letter mismatch. Moving the reset after the cheat-input block removes the fixed differences at `+0x6cc` through `+0x6f2`; the next nearby mismatch is register selection for the HWND load at `+0x6f3`. The body remains 6,880/6,616 B and MISMATCH.

[EVIDENCE] In the earlier switch candidate, the target Escape path maps `WM_PHASE_GAMEREAD` (`0x632`) to `WM_PHASE_STOP` (`0x62e`). Moving it into the STOP group corrected the behavior; that 6,880/6,616-B version still had its first fixed difference at `+0xac`.

[EVIDENCE] The target pseudocode's Escape flow is an outer PLAY test, an `else if` excluding phases `GAMEREAD`, `GAMEREADp`, and `GAMEWRITE`, followed by direct phase checks and a shared STOP action. This source shape matches the target compare order at `+0x7fd` and flips the earlier `+0xac` SIB to the target instruction bytes `c6 44 02 25 00`; the fixed prefix grows from 172 to 730 bytes. A target-shaped nested VK_RETURN chain and a shared QUITPLAYTEST action reduce the current branch destination from `+0xcef` to `+0xc24`. At `+0x2da`, target bytes `0f 87 39 09 00 00` reach `+0xc17`, while candidate bytes `0f 87 46 09 00 00` reach `+0xc24`: the intervening `<= 0x100` message-dispatch region (including `WM_KEYDOWN`) is 13 bytes larger in the candidate. Current body is 6,944/6,616 B; fixed prefix remains 730, and no strict match is established.

[EVIDENCE] Reordering the rank-zero toggle to store the member before copying it to `bEnable` was codegen-inert. An explicit if/else toggle stores before copying, matching target store order at `+0x41a`; target still uses EDX as input and ECX as the boolean result, while candidate uses ECX and EDX. This reduces fixed-byte differences in `+0x41a..+0x6f2` from 12 to 6 without changing the 6,944-byte body or the `+0xc24` destination. At `+0x6f3`, target bytes `8b 95 50 6d 00 00 52` load the HWND into EDX and push EDX; candidate bytes `8b 85 50 6d 00 00 50` use EAX. No semantics-supported source fix has been confirmed for the remaining register residues.

[EVIDENCE] No dummy declarations, padding, unused code, forced bytes, or assembly were added. The candidate is scratch-only; target code and headers in `src/` were not changed.

## VC5 local array layout

[EVIDENCE] Controlled probes are under `scratch/ev_changephase/micro/`; each `.cpp` is an independent minimal translation unit with `probe` (the EH pair also emits its throwing helper and destructor), and each corresponding object/disassembly is under `build/ev_changephase_micro/obj/` and `scratch/ev_changephase/micro/disasm/`. The compiler was VC5 SP3 11.00.7022 with `/W3 /GX /O2 /ML /Gd /Gy`, `/DWIN32 /DNDEBUG /D_WINDOWS /Dnullptr=0`, `/c`; no project headers were included. `sink(void*)` remained external so array addresses had observable escapes. Frame figures are the `sub esp, imm` allocation; probe byte counts are COFF function bytes.

| Factor and exact pair | Count / conditions | Frame and observed slots | Result |
|---|---|---|---|
| Declaration order: `decl_forward` vs `decl_reverse` | Same seven function-scope objects: `char[12]`, `char[28]`, `char[100]`, `char[260]`, `char[100]`, `char[404]`, `short[100]`; sink order fixed. | Both `0x450` frame / 128 function bytes. By variable in sink order: offsets `0, 0x0c, 0x8c, 0x1b8, 0x28, 0x2bc, 0xf0`. | Reversing all seven declarations changed no slot or code byte. |
| First-use order: `use_forward` vs `use_reverse` | Same declarations and seven calls; only the call order was reversed. | Both `0x450` / 128 bytes. Each variable kept its forward-case address; the reversed LEA sequence was `0xf0, 0x2bc, 0x8c, 0x1b8, 0x28, 0x0c, 0`. | Placement did not sort by first reference. |
| Scope depth: function scope vs one enclosing block | Same seven declarations and calls, all in one nested block in the second source. Also moved each array alone into its `if (selector)` block while keeping that branch/call site fixed in both variants (14 objects across seven function/block pairs). | One enclosing block: both `0x450` / 128 bytes. Each conditional function/block pair: both `0x450` / 144 bytes, with the same seven per-variable slots. | Scope depth alone did not move a slot in these shapes. |
| Disjoint lifetimes: `overlap` vs `disjoint_blocks` | Same seven objects and sink order. First source has all seven function-scope; second places each declaration and its sink call in a separate sequential sibling block. Separate controls use sibling `char[12]` + `char[28]`, then `char[12]` + `char[100]`. | Main pair: `0x450` / 128 bytes vs `0x1bc` / 112 bytes. The 12- and 28-byte arrays stay at `0` and `0x0c`; the five arrays sized 100, 260, 100, 404, and 200 all use `0x28`. Small controls use distinct `0` and `0x0c` slots (frames `0x28` and `0x70`). | Five of seven objects share the 404-byte slot; the frame shrinks by 660 bytes. In these probes the two small arrays do not share, even with a disjoint 100-byte local. |
| Case scope: `switch_case` | Seven braced `case` arms, one object per arm, selected by `selector`; same sizes as above. | `0x1bc` frame / 224 function bytes. After accounting for the saved `esi`, the 12- and 28-byte slots are `0` and `0x0c`; the five larger objects all use `0x28`. | Same 5-of-7 sharing pattern as sibling blocks; case-local scope does not make the two small slots share. |
| Size: `size_res_100` vs `size_res_101` | One function-scope `char[100]` changed to `char[101]`; all other objects and uses fixed. | Frame `0x450` -> `0x454`; function stays 128 bytes. The enlarged object rounds to 104 bytes; later slots shift by 4 (`str 0x1b8->0x1bc`, scratch `0x2bc->0x2c0`, door `0xf0->0xf4`). | Stack allocation is 4-byte aligned; the added byte consumes four frame bytes. |
| Address escape: `address_local` vs `address_passed` | `volatile char res[100]` is indexed in both; second source additionally passes `res` to `sink`. | Both `0x450` frame; `res` stays at normalized `0x28`; function bytes `128` -> `144`. | Passing the address added a call but did not change local placement. |
| Destructor/EH: `try_no_destructor` vs `try_with_destructor` | Same seven arrays, `try/catch`, and throwing `mayThrow()` call. Second source adds `Guard` with a user destructor inside the `try`; `/GX` in both. | Both probe functions 208 bytes; frame `0x450` -> `0x454`. `.text$x` grows 10 -> 18 bytes and `.xdata$x` grows 88 -> 96; a 16-byte destructor function is emitted. Array slots after the guard shift by 4. | A live destructor adds an EH cleanup state and one four-byte frame slot. |
| Aggregate type: `scratch_array` vs `scratch_struct` | Same seven objects; only `char scratch[404]` becomes `struct Scratch { char bytes[404]; } scratch`, and the same first-byte address is passed. | Both `0x450` / 128 bytes; all seven slots identical. | Plain aggregate vs array made no layout or code-size difference. |

[EVIDENCE] The target map is `netData +0x1c`, `data +0x28`, `res +0x44`, `str +0xa8`, `text +0x1ac`, shared scratch `+0x210`, and `door +0x3a4`. A target-shaped scope partition (function-scope `str/text/res/door`; block-scope `netData/data/scratch`) compiled to offsets by sink order `netData 0`, `data 0x0c`, `res 0x8c`, `str 0x1b8`, `text 0x28`, `scratch 0x2bc`, `door 0xf0`; it does not predict the target map. The micro rule established here is limited: in the all-function-scope probe, storage follows ascending array size (ties stayed stable under declaration and use reversal); large arrays in the tested sibling/case scopes share a slot, while the two small arrays do not. It does not explain the target's mixed local ordering.

[EVIDENCE] Before the grouped-scope follow-up, candidate `13_event.obj` mapped `netData +0x14`, `data +0x28`, `res +0x44`, `text +0xa8`, `str +0x10c`, scratch `+0x210`, and `door +0x3a4`. The first forced instruction divergence was at ChangePhase `+0x1c`, where the initial log call formed the `str` pointer.

### Grouped-scope follow-up probes

[EVIDENCE] Three additional independent minimal functions used the same VC5 SP3 flags and external `sink(void*)`. Each had nine arrays: function-scope `netData[12]`, `data[28]`, `res[100]`, `str[260]`; block `text[100]`; three `char[404]` scratch arrays; and `short door[100]`. The ten LEAs count `text` twice because the probe passed it at both ends of its lifetime.

| Probe | Scope/use condition | Function bytes / `sub esp` | LEA slots by variable (hex) |
|---|---|---:|---|
| `group_inner_then_later_door` | Text encloses three sibling scratch blocks; door is a later sibling block; root arrays first-used after the group. | 176 / `0x3ec` | `netData 0`, `data 0x0c`, `res 0xf0`, `str 0x2e8`; `text 0x28`; each scratch `0x154`; `door 0x28`. |
| `group_roots_first` | Same declarations/scopes as above; only first-use order changed so root arrays are passed before the inner group. | 176 / `0x3ec` | `netData 0`, `data 0x0c`, `res 0xf0`, `str 0x2e8`; `text 0x28`; each scratch `0x154`; `door 0x28`. |
| `group_nested_door_owner` | Roots first-used first; text block encloses a nested door-owning block, which encloses three sibling scratch blocks. Door remains live across the scratch sub-blocks. | 176 / `0x450` | `netData 0`, `data 0x0c`, `res 0x8c`, `str 0x34c`; `text 0x28`; `door 0x284`; each scratch `0xf0`. |

[EVIDENCE] Reversing first-use order did not change any slot in the first two probes. They do not establish that inner-block arrays follow a contiguous function-scope run: `text` occupies `0x28` before later root arrays, and the later sibling `door` reuses `text`'s slot. Making door's owning block contain the scratch sub-blocks prevents that sharing, but still does not predict the target map. Target offsets normalized to `netData` are `netData 0`, `data 0x0c`, `res 0x28`, `str 0x8c`, `text 0x190`, scratch `0x1f4`, and `door 0x388`; none of these probe layouts matches it.

[EVIDENCE] Applied the grouping hypothesis in `scratch/ev_changephase/event.cpp`: `netData` and `data` are function-scope, `text` is in an inner block enclosing file/joystick processing, and a nested door-owning block keeps `door` live across those scratch sub-blocks. The candidate builds with frame `0x464` (target `0x45c`) and remains 6,848 B versus 6,919 B. Its first divergence is now the prologue at `+0x0` (`sub esp,0x464` vs `sub esp,0x45c`), so this grouping is not a strict match and supersedes the earlier 28-byte identical prefix measurement.

## Remaining work

[OPEN] Continue front-to-back from `DrawButtons` RECT staging, the remaining `ChangePhase` local map (`netData` and the `str`/`text` slots), and the `TreatEventBase` player-slot loop; the controlled VC5 probes narrow local-array behavior but do not explain the target's mixed-scope ordering. Revisit `ReadInput` only if a new source-form hypothesis is supported by target evidence. `SetDemo` remains ambiguous and `CheckWorld1` remains a config mismatch; the destructor and `NetEnumSessions` relocation cases are resolved in the integrated run.


### `ChangeButtons` worker follow-up

[EVIDENCE] RVA `0x1b220` (2,854 target bytes) is `CEvent::ChangeButtons(int message)`, confirmed by the target phase/button handling and the SB2 ancestor. The scratch candidate uses the canonical headers plus private `def.h`; it is 3,024 bytes and remains MISMATCH. The fixed prefix grew `2 -> 77 -> 421 -> 561 -> 673 -> 685 -> 844 -> 971 -> 1,270 -> 1,531` bytes. The local-buffer candidate uses shared function-scope `file[256]`/`out[256]` with `netData[12]`, matching the target `sub esp,0x20c` frame (initial candidate: `0x270`).

[EVIDENCE] Source-supported fixes that advanced the prefix: reversed the gamer equality operands to `m_gamer - 1 == i` (77 -> 421); corrected the region button upper bound to `WM_BUTTON24` (`0x4e0`); changed the dimension path to assign a block-local `POINT` and call `SetDim` once after the four cases (421 -> 561); reordered the first dimension assignments before `SetState` (561 -> 673); expressed the file-page decrements as `m_filePageOffset = m_filePageOffset + -6` (673 -> 685); used explicit `bHide` assignments for SERVICE and reversed the hide comparison operands to target order (685 -> 971); corrected the SESSION selected-row test to `m_filePageOffset + i == m_fileIndex` (no prefix change at that point), then used the same branch-based hide form in SESSION (971 -> 1,270); incremented the selected network player's team before wrapping at `MAXTEAM` (1,270 -> 1,531). The team-cycle correction matches target behavior; the previous source only reset an already-out-of-range team.

[EVIDENCE] Header delta submitted to infra at that stage: the private def.h changed WM_DIMS1..4 from 1265..1268 to 1255..1258, matching target IDs 0x4e7..0x4ea. Infra later integrated these values in canonical src/def.h; the refreshed worker candidate uses canonical headers and has no private header.

[EVIDENCE] The first fixed difference after the 1,531-byte prefix is the ready-byte SIB at `+0x5fb`: candidate `8a 54 0e 25`, target `8a 54 31 25` (`[esi+ecx+0x25]` vs `[ecx+esi+0x25]`, same effective address). With VC5 SP3 `/W3 /GX /O2 /ML /Gd /Gy`, extern-count probes at the TU top and immediately before `ChangeButtons`, counts 0..64, all retained candidate `3,024` bytes, prefix `1,531`, and the same SIB bytes. This is a negative `VC5-CTX-01` result; no semantic rewrite was made for the commuted address.

[EVIDENCE] A `ReadInput` probe introduced a local `BYTE buttons = (BYTE)joyInfo.dwButtons` and reused it for all four checks. Generated code was unchanged: 480 candidate bytes vs 479 target, 34-byte fixed prefix, first substantive mismatch at `+0x114` (AL/DWORD target vs DL/byte-mask candidate). It was reverted when the candidate was refreshed from `src/event.cpp`; existing prior negative probes remain above.

[OPEN] `ChangeButtons` is not strict; no function became newly strict in this follow-up. `DrawButtons` remains 7,392/7,419 bytes with a 4,886-byte fixed prefix and first difference `+0x1316`.


### ChangeButtons continuation after canonical integration

[EVIDENCE] Refreshed scratch/ev_drawbuttons/event.cpp from src/event.cpp after commit a07e15e. The candidate uses canonical headers; no private event.h or def.h remains. ReadInput was left untouched. Full verification with VC5 SP3 and --build-dir ev_drawbuttons reports the event TU at 54 STRICT, 1 RELOC-UNVERIFIED, and 54 MISMATCH; DrawButtons remains canonical and non-strict.

[EVIDENCE] ChangeButtons remains 3,040 candidate bytes versus 2,854 target bytes, with a 1,531-byte fixed prefix. The first fixed divergence is +0x5fb: candidate 8a 54 0e 25, target 8a 54 31 25. These load the same ready byte from the network player record, with the SIB base/index fields commuted. If that byte is skipped, the next fixed mismatch is the +0x62b branch span (candidate 0x185, target 0x18f).

[EVIDENCE] Five byte-access spellings were compiled under VC5 SP3 /W3 /GX /O2 /ML /Gd /Gy: *(m_players + playerId), (m_players + playerId)->ready, (&m_players[0])[playerId].ready, a local NetPlayer pointer, and byte-offset indexing from the player array. Each retained the 3,024-byte body, 1,531-byte prefix, and candidate SIB. The wide TU context probes reported by the supervisor were already negative.

[EVIDENCE] For BLUPI-REG-01, tested a nested ready-button range and target-pseudocode control-flow in later blocks: the GAMEWRITE button range and the positive READDESIGN row test. None changed the early SIB or prefix. The range correction adds 16 bytes; the two target-shaped source corrections together compile to 3,040 bytes and remain non-strict. They are source-supported behavior/control-flow corrections, but did not transfer the SIB result in this function.

[OPEN] Continue from the +0x5fb SIB only with new target-backed source or layout evidence; the tested access spellings and same-function control-flow probes were negative. Header delta this turn: none. No canonical source or other worker files were edited.
