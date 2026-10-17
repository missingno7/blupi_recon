# Event translation unit reconstruction

**Status: partial.** The current scratch candidate has 52 STRICT symbol results, 2 RELOC-UNVERIFIED, and 55 MISMATCH. Those STRICT results occupy 48 unique target RVAs and cover 3,085 unique original event-body bytes after folded/alias duplicates are counted once. The exact SB1 phase table remains intact. The current residuals are DrawButtons, ReadInput, TreatEventBase, and ChangePhase.

## Scope and comparison

[EVIDENCE] The target block is `0x16220`-`0x1d230`, with 86 mapped functions and 34,724 Ghidra body bytes. The integrated source is `src/event.cpp`; this worker candidate is `scratch/tu/event.cpp`, derived mainly from SB2 `event.cpp`. Non-overridden headers are 19 byte-identical copies of canonical `src/` headers under `scratch/tu/event_include/current_src/`; only `def.h` and `network.h` are included from `scratch/tu/event_include/sb1_event/`. This explicitly avoids stale flat `scratch/tu` header copies. `src/` has the full game TU set and one canonical header set.

[EVIDENCE] The latest candidate was evaluated with `.pyenv\python.exe tools\verify.py --tu src/event.cpp=scratch\tu\event.cpp --build-dir event_clean`, using VC5 SP3 (11.00.7022), `/O2 /GX /ML /Gd /Gy`, and `nullptr=0`. The isolated build compiled the candidate with all other reconstructed objects. The run reports one accepted-function regression outside this worker: `CEvent::NetEnumSessions` resolves `CNetwork::EnumProviders` to `0x20bd0`, while the oracle relocation expects `0x20a30` after `network.cpp` integration.

```powershell
.pyenv\python.exe tools\verify.py --tu src/event.cpp=scratch\tu\event.cpp --build-dir event_clean
```

[EVIDENCE] The current event object reports 109 candidate functions: 52 STRICT, 2 RELOC-UNVERIFIED, 55 MISMATCH, and no FOLDED status. There are 48 unique STRICT target RVAs totaling 3,085 original function-body bytes; the duplicate aliases at `0x16e80` and `0x1b030` are counted once.

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

[EVIDENCE] The current `src/def.h` already defines `WM_DECOR1`-`WM_DECOR8`; its final phase override block yields the target SB1 values (including `INSERT=0x607`, `GAMER=0x609`, `BYE=0x614`, `DOQUIT=0x634`, `INTRO1/INTRO2=0x635/0x636`). The remaining event-specific delta is `SPRITE_WAIT: 12 to 11` and `SPRITE_EMPTY: 13 to 12`. A private copy with only those two changes makes `WaitMouse` and `HideMouse` STRICT. The private `def.h` differs from canonical `src/def.h` by only these two constants; infra should integrate them globally.

## Phase table

[EVIDENCE] The table at RVA `0x345d0` has 32 rows of `0x1bac` bytes (226,688 bytes), with 199 nonzero button records. `scratch/event/rebuild_phase_table.py` emits ordinary C++ aggregate initializers. VC5 SP3 output compares exactly to target; SHA-256 `47d4c002a0b46f72fa0f75ab0f4a92117d79c8b55e04a9f61b7492d9601b4f42`.

[HYPOTHESIS] `bCDrom` and `bLocked` are inherited field names. Their target positions and values are exact, but the semantic labels are not independently established.

## Strict function matches

[EVIDENCE] The current run has 48 unique strict target RVAs and 3,085 unique original body bytes. Candidate symbol spans can include bytes beyond Ghidra body boundaries; the table lists original body bytes. `NetDraw`/`DrawMap` share one STRICT body at `0x16e80` (36 bytes), and `BuildUp`/`PlayDown`/`PlayMove`/`PlayUp` share one STRICT body at `0x1b030` (8 bytes); each shared body is counted once.

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

## Current unresolved and alias classifications

[EVIDENCE] Current status comes from `event_clean`; only the following two event symbols remain RELOC-UNVERIFIED in that run. The other current requested cases are mismatches or duplicate aliases, not strict evidence for a separate target body.

| Candidate | Current result | Classification and evidence |
|---|---|---|
| `CEvent::~CEvent` | RELOC-UNVERIFIED at `0x16360` | Target body is 113 bytes; the candidate span is 128 bytes. Function calls resolve (`WriteInfo`, `CMenu::~CMenu`, `CButton` destruction, array destructor). Remaining DIR32 relocations are a local `$L37050` to `0x42bf89` and three `__except_list` references (`resolved_symbol_va=0`). `cmpfunc.py` cannot verify external data targets without a unique reconstructed definition, even though the non-relocated bytes and code-call targets match. |
| `SetDemo` | RELOC-UNVERIFIED, four hits | Its 48-byte candidate body hits `0x16580`, `0x165b0`, `0x165e0`, and `0x16640`, which are already strict wrappers (`SetState`, `SetEnable`, `SetHide`, `SetMenu`). The comparator reports `function location is ambiguous` before using the candidate `CButton::SetDemo` call relocation to disambiguate those wrappers. Do not count it as a strict function. |
| `CheckWorld1` | MISMATCH with `REQUIRE_CD=TRUE` | The canonical SB1 configuration produces a 128-byte body with no hit. A scratch-only `REQUIRE_CD=FALSE` variant becomes a 16-byte unconditional-true body but is RELOC-UNVERIFIED at both `0x13b0c` and `0x21520`, outside the event block. This is a config mismatch/duplicate-body ambiguity, not evidence for a strict event mapping. |
| `NetEnumSessions` | MISMATCH at `0x16920` in the full run | The event candidate's `CNetwork::EnumProviders` relocation resolves to `0x20bd0`, while the oracle expects `0x20a30`. This began after `network.cpp` was integrated and is a cross-TU accepted-function regression for infra to resolve; no event-source change was made for it. |

[EVIDENCE] The prior relocation-unverified set is now classified: `OutputNetDebug`, `AddCheatCode`, `DrawTextCenter`, `MouseOnButton`, `SearchPhase`, `NetCreate`, `BuildMove`, `StopMovie`, and `ClearGamer` are STRICT; `PlayDown`/`PlayMove`/`PlayUp` are STRICT aliases of the same 8-byte `BuildUp` body; `NetEnumSessions` is the cross-TU MISMATCH above; `MovieToStart` is MISMATCH; `CEvent::~CEvent` and `SetDemo` remain RELOC-UNVERIFIED; `CheckWorld1` is MISMATCH under the SB1 macro. `RestoreGame` is also MISMATCH. `NetDraw` and `DrawMap` are additional names sharing one STRICT 36-byte body at `0x16e80` and must be counted once.

[OPEN] The destructor data-symbol limitation is precisely the verifier gap to route to infra. `__except_list` has no image target VA, and `$L37050` has no unique reconstructed data definition; do not edit `tools/`. `SetDemo` has a separate ambiguous-location limitation: the comparator does not use call relocations to disambiguate multiple identical masked bodies. No case is caused by unresolved `__FILE__`/`__LINE__` data.

## Per-function target status

This inventory includes the current event run and the cross-TU/alias overrides above. Each row is the SB1 function boundary from `oracle/tu_map.csv`. `MISMATCH (forced)` means the candidate was compared at that RVA but the code did not match; it is not a match.

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
| `CEvent::NetEnumSessions` / `0x16920` | 34 | MISMATCH: cross-TU `EnumProviders` relocation |
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
| `FUN_0041AFC0` / `0x1afc0` | 97 | NO HIT |
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

## Large handlers: earlier and current measurements

| Function | Target body | Candidate body | Current finding |
|---|---:|---:|---|
| `CreateButtons` | 278 B | 288 B | Strict; removing the SB2-only icon-menu increment matched. |
| `DrawButtons` | 7,419 B | 7,648 B | Still +229 B. The scratch candidate removes the non-target `DECOMP -- __DATE__` draw (removed 48 B) and fixes the split-line buffer index from `line2-res` to `line2-m_filenameBuffer[i]` (+48 B); the net span is unchanged, but the pointer arithmetic now follows the target pseudocode. The prologue still differs: target reserves `0xe4` and keeps `this` in EBP; candidate reserves `0xdc` and keeps `this` in EBX. |
| `ReadInput` | 479 B | 480 B | Still +1 B in span, but not a one-byte code difference: target and candidate diverge from about offset `0x114` through the return. Signed joystick-axis casts fixed four condition mismatches. Target loads button state into AL and uses dword key-mask operations; candidate uses DL and several byte masks. Explicit `BYTE buttons`, mask spelling, and button-three store variants remained mismatches (480 B); a local-temp form was 464 B and also mismatched. |
| `LoadImageFromDisc` | — | 176 B | Candidate-only helper; target image-cache work is inline in `ChangePhase` under the target `ChangePhase [LoadImages]` path. |
| `TreatEventBase` | 6,616 B | 6,880 B | Current residual +264 B with canonical headers and `m_players +0x24`. The first host player-slot availability compare/branch still differs. Indexed do/goto, indexed while, and pointer-loop source variants all remained 6,880 B and mismatched. |
| `ChangePhase` | 6,919 B | 6,848 B | Still 71 B shorter. Correcting the gamer door buffer to 100 shorts matches the target 16-bit loop. Moving that live buffer to function scope changes the frame from `0x39c` to `0x464`; target is `0x45c`, leaving an 8-byte frame difference. |
| `TreatEvent` | 89 B | 96 B | Strict wrapper at `0x1d1d0`; target body-byte coverage remains 89 B. |

[EVIDENCE] Latest isolated verification (`event_clean`) results for the requested large handlers: `CreateButtons` 278/288 STRICT; `DrawButtons` 7,419/7,648 MISMATCH; `ReadInput` 479/480 MISMATCH; `TreatEventBase` 6,616/6,880 MISMATCH; `ChangePhase` 6,919/6,848 MISMATCH; `TreatEvent` 89/96 STRICT. `LoadImageFromDisc` remains candidate-only (176 B); target cache work is inline in `ChangePhase`.

[EVIDENCE] For `TreatEventBase`, indexed do/goto, indexed while, and pointer loop variants all emitted 6,880-byte bodies; the early player-search compare remained different. The latest source retains the original indexed do/goto form. These loop-form trials were negative under VC5 SP3 `/O2 /GX /ML /Gd /Gy`.

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
| `ChangePhase` gamer door table was treated as 200 bytes | Used `short[100]` and changed private `GetDoors`/`SetDoors`/member types to 100 shorts from target `0x8620/0x8650`. Moved the used door buffer to function scope. | Correct 16-bit scan; frame improved from 192 B under target to 8 B over target; function still mismatch. | `VC5-LOC-03`, `VC5-SEM-01`; blupi-specific structure/type evidence. |
| `DrawButtons` showed SB2 version and phase exclusions | Changed to `Version 1.0` and target Insert/Bye exclusions. | 16-B reduction; residual +229 B. | `VC5-SEM-01`; blupi-specific. |
| `AddUserExportPath` contained a malformed backslash literal | Escaped the source backslash before `User`. | Target string at RVA `0x77ec8` matches; no strict function established. | `VC5-SEM-01`; blupi-specific data correction. |

### Follow-up compiler findings

| Trigger | Source change and measured effect | Strict result | Rule / classification |
|---|---|---|---|
| Four UI helpers missed under canonical headers | `NetSendLobby`/`ChatSend` match with canonical `network.h`; private `SPRITE_WAIT=11` and `SPRITE_EMPTY=12` (canonical `12/13`) make `WaitMouse`/`HideMouse` match. | STRICT at `0x16ce0`/78, `0x18d40`/111, `0x18db0`/121, `0x1cc10`/343 original bytes; 653 unique bytes total. | `VC5-SEM-01`; SB1 layout/constant evidence. |
| `DrawButtons` included a development overlay and undefined pointer subtraction | Removed `DECOMP -- __DATE__`; changed split index to `line2 - m_filenameBuffer[i]` per the target copy into a local buffer. The removal saves 48 candidate bytes; the corrected indexing adds 48. | Still 7,648/7,419 B and MISMATCH; source now follows target behavior more closely. | `VC5-SEM-01`; blupi-specific semantic correction. |

[EVIDENCE] No dummy declarations, padding, unused code, forced bytes, or assembly were added. The candidate is scratch-only; target code and headers in `src/` were not changed.

## Remaining work

[OPEN] Continue from the first unresolved handler difference: inspect `DrawButtons` local live ranges/prologue and the button-mask instruction pattern in `ReadInput`, then revisit `TreatEventBase` and `ChangePhase` using current canonical offsets. Route the `NetEnumSessions` relocation regression and destructor external-data symbols to infra; leave `SetDemo` and `CheckWorld1` out of strict counts until unique target evidence exists.
