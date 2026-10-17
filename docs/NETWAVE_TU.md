# Network TU reconstruction

## Verification baseline

[EVIDENCE] `scratch/tu/network.cpp` was refreshed from canonical `src/network.cpp` before this pass. The private same-directory `network.h`, `misc.h`, and `def.h` copies are byte-identical to current `src/` headers; no header delta is proposed. Verification used `.pyenv/python.exe tools/verify.py --tu src/network.cpp=scratch/tu/network.cpp --build-dir netwave_baseline`, compiling the full manifest with VC5 SP3 and keeping `build/verify/` untouched.

[EVIDENCE] The network block has 15 STRICT bodies totaling 1,961 unique target bytes, including its two callbacks outside `oracle/functions.csv`. `EnumSessions` is STRICT at `0x20bd0` (163 target bytes); its descriptor store-order fix is already integrated. Four methods remain MISMATCH. `GetNbProviders` and `GetNbSessions` are extra byte hits with unverified identity and are excluded from the strict count.

[EVIDENCE] The verifier reports 35 functions in `network.obj`: 15 STRICT, 2 RELOC-UNVERIFIED, and 18 MISMATCH. The 18 mismatches are the four open network methods and 14 inline `POINT` operators emitted from `def.h`; those inline bodies have no proved target mapping in this block.

## Network function status

Target body lengths below are from `oracle/functions.csv`; callback lengths are from the verified target bodies. Candidate lengths are from the isolated baseline object.

[EVIDENCE] Some candidate COFF extents are longer than the corresponding CSV function size (for example, `TraceErrorDP` is 928 versus 916 bytes); the strict placement is still proved by the verifier. Unique target-byte totals below use the oracle function extents, not the longer candidate COFF extents.

| Function | RVA | Target bytes | Candidate bytes | Status |
|---|---:|---:|---:|---|
| `TraceErrorDP` | `0x205d0` | 916 | 928 | STRICT |
| `CNetwork::CNetwork` | `0x20970` | 31 | 32 | STRICT |
| `CNetwork::~CNetwork` | `0x20990` | 36 | 48 | STRICT |
| `EnumProvidersCallback` | `0x209c0` | 112 | 112 | STRICT |
| `CNetwork::EnumProviders` | `0x20a30` | 76 | 80 | STRICT |
| `CNetwork::GetProviderName` | `0x20a80` | 36 | 48 | STRICT |
| `CNetwork::CreateProvider` | `0x20ab0` | 111 | 112 | STRICT |
| `CNetwork::FreeProviderList` | `0x20b20` | 35 | 48 | STRICT |
| `EnumSessionsCallback` | `0x20b50` | 128 | 128 | STRICT |
| `CNetwork::EnumSessions` | `0x20bd0` | 163 | 176 | STRICT |
| `CNetwork::GetSessionName` | `0x20c80` | 36 | 48 | STRICT |
| `CNetwork::JoinSession` | `0x20cb0` | 237 | 224 | MISMATCH |
| `CNetwork::FreeSessionList` | `0x20da0` | 35 | 48 | STRICT |
| `CNetwork::CreateSession` | `0x20dd0` | 234 | 224 | MISMATCH |
| `CNetwork::Send` | `0x20ec0` | 77 | 80 | MISMATCH |
| `CNetwork::Receive` | `0x20f10` | 207 | 208 | STRICT |
| `CNetwork::Close` | `0x20fe0` | 18 | 16 | MISMATCH |
| `CNetwork::FreeUnknownList` | `0x21000` | 35 | 48 | STRICT |
| `CNetwork::IsHost` | `0x21030` | 4 | 16 | STRICT |

[EVIDENCE] `JoinSession` has the target signed bounds check and `0x74` session-record stride; the remaining difference is the local `DPNAME`/`CreatePlayer` setup. `CreateSession` has the target descriptor values and call sequence but differs in descriptor/player-name stack stores and boolean tests. `Send` passes the target arguments and normalizes `dwFlags`; its candidate uses a different register/stack layout. `Close` returns the same success condition but uses `neg`/`sbb`/`inc`, while the target uses `test`/`sete`.

[EVIDENCE] `GetNbProviders` is a 16-byte hit at `0x22dc0`, which is assigned to `CSound::GetEnable`; `GetNbSessions` is a 16-byte hit at `0x1efc0`, assigned to `CJauge::GetHide`. No network call-site or relocation proof identifies either address as a network getter, so both remain RELOC-UNVERIFIED.

## Source experiments this pass

All variants were verified with a separate `--build-dir`; the scratch candidate was restored to canonical `src/network.cpp` afterward.

| Method | Source-supported variation | Result |
|---|---|---|
| `CreateSession` | Move `dwMaxPlayers` assignment before `dwSize` and `dwFlags`. | Changed some bytes, but remained MISMATCH at 224 candidate bytes. |
| `CreateSession` | Use `if (hr)` instead of `if (hr != DP_OK)` for both error checks. | COFF body byte-identical to baseline; no code effect. |
| `JoinSession` | Full aggregate `DPNAME` initialization was tested. | VC5 rejected a flat initializer because the ANSI name pointer is in an anonymous union; no candidate was produced. Earlier `{0}` plus field assignment and field-order variants did not match. |
| `Send` | Split the comma-expression assignment and introduce a local normalized flag. | COFF body byte-identical to baseline; remained MISMATCH. |
| `Close` | Store the comparison in a `BOOL` local; use a `TRUE`/`FALSE` conditional expression. | COFF body byte-identical to baseline; remained MISMATCH. Earlier explicit-branch and `bool` return-type probes also failed. |

## CRT/runtime region formerly labeled `wave.cpp`

[EVIDENCE] The `0x23b20-0x240c0` block is statically linked CRT/runtime support, not a DirectSound `wave.cpp` translation unit. It contains CRT `sprintf` (`0x23b40`), `fread` (`0x23bc0`), `_ftime` (`0x23d10`), and `/GX` exception support. The CRT C helpers use `__cdecl`; the unwind entries are VC5 C++ EH runtime. A VC5 SP3 `SPRINTF.C` probe strictly matches `0x23b40` (104 target bytes, both relocations proved). `FREAD.C` and `_ftime` identify the source family but retain byte/relocation differences. The copied DirectSound sample is not the source of this block; no further wave TU work is open.

## Header/layout evidence

[EVIDENCE] The current canonical `network.h` uses `CALLBACK` for both enumeration callbacks, matching their `ret 0x14` and `ret 0x10` cleanup. Observed class layout is `m_pDP +0`, provider list `+4/+8`, session list `+12/+16`, unknown list `+20/+24`, host flag `+28`, `m_dpid +32`, and players `+36`. `NetPlayer` is 32 bytes with `DPID +4`, team `+8`, and name `+10`; `NamedGUID` is 116 bytes. These agree with the verified callback, allocation, and receive accesses. No header change is pending.

## Compiler findings

Builds used VC5 SP3 `11.00.7022`, `/W3 /GX /O2 /ML /Gd /Gy`. Findings are recorded here; `msvc5kb/catalog.json` and `docs/MSVC5KB_VALIDATION.md` were not edited.

| ID / rule | Trigger, variation, and result | Classification |
|---|---|---|
| `BLUPI-NET-001` / `VC5-SEM-01` | `TraceErrorDP`: correcting the missing error labels and strings yields STRICT at `0x205d0`. | Blupi-specific |
| `BLUPI-NET-002` / `BLUPI-DIA-01` | Declaring enumeration callbacks `CALLBACK` matches both callback bodies and proves their relocations. | Blupi-specific |
| `BLUPI-NET-003` / `VC5-SEM-01` | `Receive`: comparing sender DPID with `m_players[i].dpid` fixes field use and matches STRICT. | Blupi-specific |
| `BLUPI-NET-004` / `VC5-SEM-01` | `JoinSession`: `index >= nb` and 0x74-byte session indexing fix the target condition and member offsets; the body remains open. | Blupi-specific; codegen unresolved |
| `BLUPI-NET-005` / `VC5-ORD-01` | `EnumSessions`: storing `dwSize` before `guidApplication` yields STRICT at `0x20bd0`. | Blupi-specific |
| Negative / `VC5-ORD-01` | Reordering `CreateSession` descriptor assignments changed store order but did not produce STRICT code. | Negative |
| Negative / `VC5-NEG-01` | `if (hr)` versus `hr != DP_OK`, local boolean/result forms, and the `Send` local normalized flag emitted byte-identical method bodies. | Negative |
| Negative / `VC5-CTL-03` | The tested explicit `Close` branch did not match the target `sete` materialization. | Negative |
| Negative / `VC5-FLG-04` | The DirectSound sample TU has no body matching the target CRT/runtime region. | Negative |

## Open work

[OPEN] `JoinSession`, `CreateSession`, `Send`, and `Close` remain MISMATCH. The isolated baseline tags are respectively size/displacement/native-stack, size/displacement/native-stack, displacement, and size/structural. Previous source-supported assignment-order, aggregate, branch, return-type, and split-call probes did not resolve them; further work needs a stronger source-form clue.

[OPEN] The two 16-byte getter hits remain RELOC-UNVERIFIED and excluded from match counts. The former wave block is attributed to CRT/runtime and is closed.
