# Network and wave TU reconstruction

## Result

[EVIDENCE] The current 17 manifest TUs were compiled from `src/` with `tools/msvc.py` into `build/netwave/anchors/obj/`, using VC5 SP3 `11.00.7022`, `/W3 /GX /O2 /ML /Gd /Gy`, the manifest defines, `src/` headers, and DX6 headers. The network and wave candidates were compared with all 17 anchor objects by `tools/cmpfunc.py`. Per the supervisor's integration lock, `tools/verify.py` was not run and `build/verify/` was left untouched.

[EVIDENCE] The final network candidate has 13 strict inventory functions totaling 1,721 original bytes, plus strict enumeration callbacks at `0x209c0` and `0x20b50` (240 bytes; these callback rows are not in `oracle/functions.csv`). That is 15 strict bodies and 1,961 unique network bytes. `EnumSessions` became strict after its descriptor store order was corrected. Four inventory functions remain mismatches. The two extra getter lookalikes are `RELOC-UNVERIFIED` and excluded from all counts.

[EVIDENCE] The current `def.h` also emits 14 inline `POINT` operator bodies into `network.obj`; they are outside this block. Thus the full object summary is 35 functions, 15 strict, 2 relocation-unverified, and 18 mismatches (the four network mismatches plus those 14 header helpers).

[EVIDENCE] Three wrappers in the inferred wave block match strictly: `0x23b20` (21 bytes), `0x23bb0` (14 bytes), and `0x23dd0` (16 bytes). A VC5 SP3 CRT source probe also matches `sprintf` at `0x23b40` (104 target body bytes). Together with the network bodies, the measured strict coverage is 2,116 unique original bytes across 19 bodies. No folded or ambiguous duplicate bytes are included.

[EVIDENCE] The copied 1996 Microsoft DirectSound sample candidate in `scratch/tu/wave.cpp` builds three functions but gets zero wildcard hits, zero strict matches, and three mismatches. Of the 12 non-wrapper records, the `sprintf` entry is now matched to VC5 CRT source; the other 11 show CRT or exception-support structure, with one tiny transfer thunk still only hypothesized. The TU map labels the range `wave.cpp` only by inferred source-order placement; it does not prove those runtime functions came from the sample or even that the sample was linked.

## Network function status

Sizes are target body sizes from `oracle/functions.csv`, excluding alignment padding.

| Function | RVA | Bytes | Status |
|---|---:|---:|---|
| `TraceErrorDP` | `0x205d0` | 916 | STRICT |
| `CNetwork::CNetwork` | `0x20970` | 31 | STRICT |
| `CNetwork::~CNetwork` | `0x20990` | 36 | STRICT |
| `EnumProvidersCallback` | `0x209c0` | 112 | STRICT; callback outside CSV inventory |
| `CNetwork::EnumProviders` | `0x20a30` | 76 | STRICT |
| `CNetwork::GetProviderName` | `0x20a80` | 36 | STRICT |
| `CNetwork::CreateProvider` | `0x20ab0` | 111 | STRICT with `_IID_IDirectPlay2A=0x0042c230` mapping |
| `CNetwork::FreeProviderList` | `0x20b20` | 35 | STRICT |
| `EnumSessionsCallback` | `0x20b50` | 128 | STRICT; callback outside CSV inventory |
| `CNetwork::EnumSessions` | `0x20bd0` | 163 | STRICT after store-order correction |
| `CNetwork::GetSessionName` | `0x20c80` | 36 | STRICT |
| `CNetwork::JoinSession` | `0x20cb0` | 237 | MISMATCH; candidate 224 bytes |
| `CNetwork::FreeSessionList` | `0x20da0` | 35 | STRICT |
| `CNetwork::CreateSession` | `0x20dd0` | 234 | MISMATCH; candidate 224 bytes |
| `CNetwork::Send` | `0x20ec0` | 77 | MISMATCH; candidate 80 bytes |
| `CNetwork::Receive` | `0x20f10` | 207 | STRICT |
| `CNetwork::Close` | `0x20fe0` | 18 | MISMATCH; candidate 16 bytes |
| `CNetwork::FreeUnknownList` | `0x21000` | 35 | STRICT |
| `CNetwork::IsHost` | `0x21030` | 4 | STRICT |

[EVIDENCE] `JoinSession` now uses the target bounds check (`index >= nb`) and its session-record stride (`0x74`). Its remaining mismatch is in the `DPNAME`/`CreatePlayer` setup sequence. `CreateSession` has the target GUID, name, flags, and player limit, but its stores and argument setup still differ. `Send` has the target operations and flags normalization, with register allocation and call layout differences. `Close` has the target truth condition but still emits a different boolean materialization sequence.

[EVIDENCE] The extra candidate methods `GetNbProviders` and `GetNbSessions` remain `RELOC-UNVERIFIED` at byte hits `0x22dc0` and `0x1efc0`. Those target addresses are `CSound::GetEnable` and `CJauge::GetHide`, respectively; all inspected direct callers use the sound/jauge objects. The byte hits do not prove that either network getter exists in the executable.

## Wave block source attribution

[EVIDENCE] The DirectSound sample's three candidate functions (`WAVE_LoadResource`, `wave_ParseWaveMemory`, and `LoadWave`) have no corresponding body in the inspected target range. The 12 non-wrapper records have no RIFF `fmt `/`data` parser, resource-loading sequence, or DirectSound `CreateSoundBuffer`/`Lock`/`Unlock` path. `0x23b40` is now matched to CRT `sprintf`; the other 11 remain open and their observed code supports CRT/runtime attribution, not a different sample version or calling convention.

| Target | RVA | Bytes | Evidence-based source attribution |
|---|---:|---:|---|
| `FUN_00423b20` | `0x23b20` | 21 | STRICT wrapper to `0x423af0` (mapped to `text.cpp` by TU order); exact source name remains open |
| `FUN_00423b40` | `0x23b40` | 104 | STRICT: VC5 SP3 CRT `sprintf`, C `__cdecl`; see CRT probe below |
| `FUN_00423bb0` | `0x23bb0` | 14 | STRICT wrapper to `0x424480`; exact source name remains open |
| `FUN_00423bc0` | `0x23bc0` | 326 | VC5 CRT `fread`, C `__cdecl`; target algorithm follows `FREAD.C` |
| `FUN_00423d10` | `0x23d10` | 187 | VC5 CRT `_ftime`, C `__cdecl`; target fills `_timeb` fields |
| `FUN_00423dd0` | `0x23dd0` | 16 | STRICT wrapper to `0x4244f0`; exact source name remains open |
| `FUN_00423de0` | `0x23de0` | 45 | `/GX` exception funclet: restores `ExceptionList`, then transfers to a handler |
| `FUN_00423e20` | `0x23e20` | 7 | HYPOTHESIS: register/stack tail-transfer thunk between exception helpers |
| `FUN_00423e30` | `0x23e30` | 86 | SEH unwind helper using `RtlUnwind` and `ExceptionList` |
| `FUN_00423e90` | `0x23e90` | 60 | VC5 CRT `___CxxFrameHandler` wrapper to `___InternalCxxFrameHandler` at `0x426ea0` |
| `FUN_00423ed0` | `0x23ed0` | 91 | Exception-frame setup around `__CallSettingFrame_12` |
| `FUN_00423f30` | `0x23f30` | 42 | Exception-handler adapter to `___InternalCxxFrameHandler` |
| `FUN_00423f60` | `0x23f60` | 202 | `/GX` exception-frame setup and dispatch |
| `FUN_00424030` | `0x24030` | 124 | `/GX` exception dispatch/unwind helper |
| `FUN_004240c0` | `0x240c0` | 127 | Exception-frame chain search over `0x14`-byte records |

[EVIDENCE] The CRT sources in the pinned SP3 tree identify `sprintf` (`SPRINTF.C`), `fread` (`FREAD.C`), and `_ftime` (`FTIME.C`) as C `__cdecl` routines. Compiling `SPRINTF.C` with `/D_CRTBLD` and the SP3 flags produces a 112-byte object body that compares `STRICT` at `0x23b40`, with both relocations proven to `__output` at `0x425890` and `__flsbuf` at `0x425760`. A same-flags `VSPRINTF.C` probe mismatches at the forced RVA, distinguishing the actual varargs `sprintf` wrapper from `vsprintf`. The target function body is 104 bytes followed by eight matching alignment NOPs; only 104 original function bytes are counted.

[EVIDENCE] The `FREAD.C` body has the same buffered-copy / `_read` / `_filbuf` flow as `0x23bc0`; a standalone SP3 source compile is 320 bytes and mismatches the 326-byte target body, so the exact library build variant is not established. The `_ftime` source compile is 192 bytes versus 187 target bytes and is `RELOC-UNVERIFIED` because its `_timezone` data relocation has no unique reconstructed definition; its `_tzset`, `GetLocalTime`, and `mktime` targets resolve to `0x4267c0`, the IAT slot, and `0x426570`.

[EVIDENCE] Eight entries from `0x23de0` through `0x240c0` visibly use exception records, `ExceptionList`, `RtlUnwind`, `__CallSettingFrame_12`, or the VC5 internal frame handler. `0x23e20` is a seven-byte tail-transfer thunk adjacent to these helpers; its exact role remains a hypothesis. `docs/TOOLCHAIN.md` independently identifies the 60-byte `___CxxFrameHandler` at `0x23e90` and the 208-byte `___InternalCxxFrameHandler` at `0x26ea0` as CRT code. This cluster is compiler/runtime support associated with `/GX`, not DirectSound resource-sample code. Exact source object names for the remaining eight exception helpers remain open.

[HYPOTHESIS] The three strict wrappers have different callees (`0x423af0`, `0x424480`, and `0x4244f0`); their byte matches do not establish original source text or prove membership in a source file named `wave.cpp`.

## Private header deltas

[EVIDENCE] The active network header changes remain private under `scratch/tu/netwave_include/sb1`; canonical `src/` headers were not edited. Private `def.h` and `misc.h` match current `src/` byte-for-byte. The unused flat `def.h`, `misc.h`, and `network.h` copies also match `src/`; flat `wave.h` duplicates the sample header.

- `CNetwork`: `m_pDP` at `+0`; provider count/pointer at `+4/+8`; session list at `+12/+16`; unknown list at `+20/+24`; `m_bHost` at `+28`; `m_dpid` at `+32`; `m_players` at `+36`. `IsHost` reads `+0x1c`, player creation uses `+0x20`, and `Receive` accesses players from `+0x24`.
- `NetPlayer`: 32-byte record; byte fields at `+0..+3`, `DPID` at `+4`, team at `+8`, name at `+10`.
- `NamedGUID`: 116 bytes (`GUID` 16 plus name 100); target allocation `0x2d50` supports 100 records and `(*pList)[index]` indexing.
- `MAXNETPLAYER=4` agrees with the four-player loop and target session descriptor.
- Both callback declarations use `CALLBACK` (`__stdcall`), matching callback returns `ret 0x14` and `ret 0x10`.
- The private wave header is the copied DirectSound sample header with no layout delta established by SB1 target evidence.
- A one-off probe header at `scratch/netwave/closebool_include/network.h` changes only `CNetwork::Close` from `BOOL` to `bool`; it emitted the same code and is not an integration delta.

## Compiler findings

Build findings below use VC5 SP3 `11.00.7022`, `/W3 /GX /O2 /ML /Gd /Gy`. Rule IDs are references only; `msvc5kb/catalog.json` was not changed.

| ID / rule | Trigger, variation, and result | Classification |
|---|---|---|
| `BLUPI-NET-001` / `VC5-SEM-01` | `TraceErrorDP`: adding `DPERR_BUFFERTOOSMALL` and `DPERR_NOCAPS` and correcting `DPERR_CAPSNOTAVAILABLEYET`'s label string matches 916 bytes strictly at `0x205d0`. | blupi-specific |
| `BLUPI-NET-002` / calling-convention source form | Declaring both enumeration callbacks `CALLBACK` instead of `/Gd` default `__cdecl` matches `0x209c0`/112 and `0x20b50`/128 strictly and proves the callback relocation. | blupi-specific |
| `BLUPI-NET-003` / `VC5-SEM-01` | `Receive`: compare sender DPID with `m_players[i].dpid`, not loop index; matches target offsets and all 207 bytes at `0x20f10`. | blupi-specific |
| `BLUPI-NET-004` / `VC5-SEM-01` | `JoinSession`: correcting `index >= nb` and 0x74-byte record indexing fixes target condition and field offset; body remains a mismatch. | blupi-specific; codegen unresolved |
| `BLUPI-NET-005` / `VC5-ORD-01` | `EnumSessions`: placing `desc.dwSize = sizeof(desc)` before `desc.guidApplication = APP_GUID` removes the store-order mismatch and yields STRICT at `0x20bd0` (163 bytes). | confirmed-transferable |
| Negative / `VC5-ORD-01` | Reordering `JoinSession` DPNAME assignments and trying aggregate DPNAME initialization did not match; aggregate initialization grew the candidate body. | negative |
| Negative / `VC5-CTL-03` | Rewriting `Close` as an explicit success/failure branch did not match the target `test`/`sete`; candidate remains 16 bytes versus 18. | negative |
| Negative / `VC5-NEG-01` | Changing `Close`'s return type from Win32 `BOOL` to C++ `bool` emitted the same `neg`/`sbb`/`inc` sequence and did not match. The original candidate signature was restored. | negative |
| Negative / `VC5-EXP-04` | Splitting `Send`'s comma-expression assignment from its error test retained the 80-byte mismatch versus the 77-byte target. | negative |
| Negative / `VC5-FLG-04` | The copied DirectSound sample TU, compiled as C++ with `/GX`, has zero wildcard hits and three mismatches; its resource parser and DirectSound calls are absent from the target block. | negative |

## Open work

[OPEN] Remaining network mismatches are `JoinSession`, `CreateSession`, `Send`, and `Close`. The last `kb.py hints` pass flags displacement/native-stack differences for the session methods, a displacement-only difference for `Send`, and a size/structure difference for `Close`; no forced-byte or dummy-context source changes were used.

[OPEN] `GetNbProviders` and `GetNbSessions` remain relocation-unverified byte hits with no target call-site proof. Do not count them as matches until a unique relocation target or direct call/reference establishes identity.

[OPEN] `FREAD.C` is the strongest source-family match for `0x23bc0`, but the standalone body differs by six bytes. `_ftime` is identified by its `_timeb` fields and call graph, but its timezone relocation remains unverified. Exact object provenance for the other eight entries in the exception-support cluster (including the tentative transfer thunk) and the three wrappers is not fully mapped.

[OPEN] The target's `wave.cpp` label is inferred from block order. There is no evidence that SB1 linked the 1996 DirectSound sample body; the currently compiled sample copy is rejected as the source for these 15 target functions.
