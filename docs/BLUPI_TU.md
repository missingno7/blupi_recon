# `blupi.cpp` and `decnet.cpp` reconstruction

## Scope and candidate order

[EVIDENCE] The owned candidates are `scratch/tu/blupi.cpp` and `scratch/tu/decnet.cpp`, with private headers under `scratch/tu/blupi_include/`. `src/`, `assets/`, and `external/` were not changed. Candidate order follows the SB1 block:

`GetNum`, `ReadConfig`, `UpdateFrame`, `SetDecor`, `RestoreGame`, `FlushGame`, `WindowProc`, `FinishObjects`, `TimerStep`, `InitFail`, `Benchmark`, `WinMain`, `DoInit`.

`decnet.cpp` order is `NetStopCloud`, `ObjectStart`, `ObjectDelete`, `ModifDecor`, `NetPlaySound`, `NetStopSound`.

## Comparison result

[EVIDENCE] Both objects use VC5 SP3 11.00.7022 with `/W3 /GX /O2 /ML /Gd /Gy`, defines `WIN32 NDEBUG _WINDOWS nullptr=0`, the private include overlay, the SB2 BASS include, and DX6 headers. `TMP` and `TEMP` point to `build/tmp`. The final `cmpfunc.py` run compares both objects together, forces the target RVAs, and maps cross-TU calls and `WindowProc` local labels to proven oracle addresses. It reports 11 STRICT `blupi.cpp` functions and 4 STRICT `decnet.cpp` functions, with zero `RELOC-UNVERIFIED` functions. Their unique oracle body coverage is 3,341 bytes. No folded or ambiguous duplicate is counted.

| Function | Oracle RVA / body bytes | Candidate span bytes | Status |
|---|---:|---:|---|
| `GetNum` | `0x1000 / 37` | 48 | STRICT |
| `ReadConfig` | `0x1030 / 673` | 688 | MISMATCH |
| `UpdateFrame` | `0x12e0 / 528` | 528 | STRICT |
| `SetDecor` | `0x14f0 / 203` | 208 | STRICT |
| `RestoreGame` | `0x15c0 / 34` | 48 | STRICT |
| `FlushGame` | `0x15f0 / 18` | 32 | STRICT |
| `WindowProc` | `0x1610 / 1360` | 1360 | STRICT |
| `FinishObjects` | `0x1b60 / 234` | 240 | STRICT |
| `TimerStep` | `0x1c50 / 64` | 64 | STRICT |
| `InitFail` | `0x1c90 / 197` | 208 | STRICT |
| `Benchmark` | `0x1d60 / 204` | 208 | STRICT |
| `WinMain` | `0x1e30 / 191` | 192 | STRICT |
| `DoInit` | `0x1ef0 / 2801` | 2816 | MISMATCH |
| `NetStopCloud` | `0x10670 / 50` | 64 | STRICT |
| `ObjectStart` | `0x106b0 / 489` | 480 | MISMATCH |
| `ObjectDelete` | `0x108a0 / 112` | 112 | STRICT |
| `ModifDecor` | `0x10910 / 100` | 112 | MISMATCH |
| `NetPlaySound` | `0x10980 / 59` | 64 | STRICT |
| `NetStopSound` | `0x109c0 / 50` | 64 | STRICT |

[EVIDENCE] All relocations for the 15 STRICT functions are proven. Explicit function mappings include `DoInit=0x401ef0`, `Benchmark=0x401d60`, `TimerStep=0x401c50`, `Build=0x404350`, `MoveObjectSearch=0x413cb0`, `MoveObjectFree=0x413ab0`, `MoveObjectStepIcon=0x411bd0`, `SearchDistRight=0x4103b0`, `MoveObjectPriority=0x413c10`, `NetMessagePush=0x4113c0`, and the folded `OutputDebug` owner at `0x41f010`; imports and data targets are checked against the PE/IAT.

[EVIDENCE] `ObjectDelete` at `0x108a0` is owned by `decnet.cpp` and is STRICT for 112 bytes. It is not a linker-folded duplicate; this resolves the engine worker's folded suspicion. Its `MoveObjectSearch` and `NetMessagePush` call targets are proven.

[EVIDENCE] `WindowProc` occupies `0x1610–0x1b5f`, ending at `FinishObjects` (`0x1b60`), for 1,360 bytes. The entry and extent are confirmed by the target code and the STRICT comparison. The earlier mismatch was caused by stale mappings for compiler-generated local labels after preceding TU edits; remapping those labels restored a full strict match without changing the function source.

## Globals and initialized data

[EVIDENCE] Candidate `.data` symbol order and values are `g_speedRate=1` at offset `0`, `g_timerInterval=50` at `4`, `g_mouseType=1` at `8`, and `g_lastPhase=999` at `0xc`. Oracle bytes at `0x42e030`, `0x42e034`, `0x42e038`, and `0x42e03c` match those values exactly. The target's config, title, error, and benchmark string payloads also match the candidate literals.

[EVIDENCE] The named zero-initialized globals keep their target order: `g_hWnd` (`0x47e618`), `g_CDPath` (`0x47e628`), `g_updateTimer` (`0x47e72c`), `WindowProc`'s static `hInstance` (`0x47e734`), `g_pEvent` (`0x47e738`), `g_pPixmap` (`0x47e73c`), `g_pSound` (`0x47e740`), `g_pNetwork` (`0x47e744`), `g_pMovie` (`0x47e748`), `g_pDecor` (`0x47e74c`), `g_bFullScreen` (`0x47e750`), `g_bTrueColorBack` (`0x47e754`), `g_benchmark` (`0x47e758`), `g_timer` (`0x47e75c`), `g_bActive` (`0x47e760`), and `g_bTermInit` (`0x47e764`). Static inspection confirms zero bytes at these addresses; the BSS object symbols and the static local `hInstance` are also zero initialized.

[EVIDENCE] Candidate BSS symbol offsets are `g_hWnd=0`, `g_CDPath=8`, `g_updateTimer=0x10c`, static `hInstance=0x110`, `g_pEvent=0x114`, then the pointer globals in the target order through `g_pDecor=0x128`; the scalar flags follow through `g_bTermInit=0x140`. Target code writes the static `hInstance` at `0x47e734` from `WindowProc`'s first argument, and the candidate has the matching static-local symbol.

[EVIDENCE] The target bytes at `0x47e620–0x47e627` and `0x47e730–0x47e733` are zero-filled. A static scan of `.text` finds no absolute load/store operand in either range. `ReadConfig` pseudocode represents its last-character test as `(&DAT_0047e627)[i]` with `i > 0`, which is consistent with `g_CDPath[i-1]`; it does not establish a stored object at `0x47e627`.

[OPEN] The candidate omits both zero-filled gaps, so its BSS layout is not exact: the path and following globals are 8 bytes early, and the static `hInstance` and later globals are 12 bytes early relative to `g_hWnd`. The gap addresses are 4-byte aligned on both sides, so alignment alone does not identify the missing 8-byte and 4-byte spans. They may be unused zero-initialized globals/statics or linker section padding/order; stripped target symbols and absent direct references do not distinguish these cases. No dummy or padding declaration was added.

## Private header deltas

[EVIDENCE] The private include closure changes `button.h`, `ddutil.h`, `decor.h`, `def.h`, `jauge.h`, `menu.h`, `misc.h`, `movie.h`, `pixmap.h`, `resource.h`, `sound.h`, and `text.h` relative to the copied current headers. It adds SB2 support headers `dectables.h`, `event.h`, `network.h`, `obstacle.h`, `pixtables.h`, and `wave.h`. The full overlay remains isolated from `src/`; the other copied SB2 declarations are compile context and remain unverified for SB1.

- [EVIDENCE] `decor.h`: `m_decor` begins at `this+0x10`; `m_moveObject` begins at `this+0x9c50` with stride `0x30`. Two SB2 trajectory arrays were removed to preserve the measured object offset. `CDecor::Build` is declared in this private header as `Build(RECT, POINT)` for the exact `SetDecor` call shape; the target callee at `0x404350` returns with `ret 0x18`, confirming six stack words.
- [EVIDENCE] `decor.h`: `CDecor::MoveStep(BOOL bFirst)` carries the first-step flag passed by `UpdateFrame`; the full `UpdateFrame` body is STRICT at `0x12e0` for 528 bytes with this declaration.
- [EVIDENCE] `pixmap.h`: `CPixmap::Create` takes one true-color flag followed by `mouseType`, as required by target `DoInit`.
- [EVIDENCE] `event.h` / `decor.h`: target-shaped `Create` declarations match the `DoInit` call sequence: `CDecor::Create(hwnd, sound, pixmap, network)` and `CEvent::Create(instance, hwnd, pixmap, decor, sound, network, movie)`.
- [EVIDENCE] `def.h`: `REQUIRE_CD` is true; `MAXCELX/MAXCELY=100` and `DIMOBJX/DIMOBJY=64`, matching the target config and cell-index evidence.
- [EVIDENCE] `misc.h`: `OutputDebug(char*)` agrees with `src/misc.h` and target decoration `?OutputDebug@@YAXPAD@Z`; `DoInit`'s three debug call sites resolve to the folded bare `ret` at `0x1f010`.
- [EVIDENCE] Target `DoInit` allocation immediates establish sizes `CPixmap=0x1b94`, `CSound=0x2b0`, `CNetwork=0xa4`, `CMovie=0x14`, `CDecor=0x258d4`, and `CEvent=0x9b90`. The current private declarations emit `CPixmap=0x1ba4`, `CSound=0x284`, `CDecor=0x2595c`, and `CEvent=0x9be0`; the `CNetwork` and `CMovie` sizes match. These are aggregate layout differences, not evidence identifying which members are wrong.
- [OPEN] The remaining declarations, constants, and member layouts copied from SB2/PB are not claimed as SB1 facts without target offsets or strict bodies.

## Compiler findings

- [EVIDENCE] `BLUPI-BENCH-01` — the SB2-style benchmark body was replaced with the target's ten-call loop, millisecond wrap handling, elapsed-time division by ten, exact `CheckTime = %d\r\n` output, and guarded `data\\time.blp` write. Under SP3 `/O2 /Gy /GX /ML /Gd`, `Benchmark` is STRICT at `0x1d60`; candidate span 208 bytes covers the 204-byte target body. Classification: **Blupi-specific**.
- [EVIDENCE] `BLUPI-SETDECOR-ARGS-01` — baseline `Build(RECT)` was a 208-byte MISMATCH. Passing `posMouse` as the second value and using the private `Build(RECT, POINT)` declaration made all 208 bytes STRICT at `0x14f0`; the `Build` relocation resolves to `0x404350`. The target callee ends with `ret 0x18`. Classification: **Blupi-specific**.
- [EVIDENCE] `BLUPI-UPDATEFRAME-01` / `VC5-SEM-01` — the target flow omits SB2 intro-phase handling, and `CDecor::MoveStep` receives the first-step boolean. With that source flow and private `MoveStep(BOOL)` declaration, the SP3 `/O2 /GX /ML /Gd /Gy` candidate is STRICT at `0x12e0` for 528 bytes. Classification: **Blupi-specific**.
- [EVIDENCE] `VC5-SEM-01` / `VC5-CTX-01` / `VC5-CTX-03` — `ReadConfig` initially emitted `cmp eax,0xc1b` / `jle`; target bytes are `cmp eax,0xc1c` / `jl`. Replacing `g_benchmark > 3099` with `g_benchmark >= 3100` removed those immediate and condition mismatches. A separate delta local did not improve the original code. Counts `0–2` at TU start, TU end, and immediately before `ReadConfig` all left exactly one fixed-byte SIB-order difference at offset `0x89`; the current candidate remains MISMATCH (`688`-byte span versus the `673`-byte target body). This bounded probe does not rule out other context counts. Classification: boundary change **Blupi-specific**; declaration-count/context probes **negative in the tested range**; SIB residue **unverified**.
- [EVIDENCE] `VC5-SEM-01` — target `DoInit` pseudocode fixes the windowed/fullscreen `CreateWindowEx` styles; the candidate now uses those exact values and preserves the target debug strings/calls. It remains MISMATCH. Target `new` sizes are `CPixmap 0x1b94`, `CSound 0x2b0`, `CDecor 0x258d4`, and `CEvent 0x9b90`; the overlay emits `0x1ba4`, `0x284`, `0x2595c`, and `0x9be0`. Repeated register/order differences remain, but the size immediates establish real header-layout differences first. Classification: **open layout dependency**.
- [EVIDENCE] `VC5-SEM-01` / `VC5-LOC-03` — `ObjectStart` remains `480` bytes versus `489`. Mutating `speed` instead of retaining `num2`, nesting the target comparisons, and splitting the `POINT` parameters in a scratch ABI probe did not improve the match; the nested probe increased fixed-byte differences and the split-parameter probe reduced the span to `464`. The `size_differs` KB tag had no actionable transfer. Classification: tested source/ABI variants **negative**; remaining source form **unverified**.
- [EVIDENCE] `BLUPI-DECNET-DIV-01` / `VC5-SEM-01` — `ModifDecor` remains `112` bytes versus `100`. Reversing the flattened coordinate expression emitted identical bytes; direct 2D indexing and `(pos+63)/64` did not match; an int-pair ABI probe retained the 112-byte span and added a frame. Classification: tested expression/ABI variants **negative**.
- [EVIDENCE] `WindowProc` is STRICT at `0x1610` for its full 1,360-byte span after remapping its compiler-generated branch labels to target addresses. `kb.py` still emits `native_fewer_loads` / `native_more_stack` tags on that strict row; the mapped byte and relocation comparison proves there is no remaining code mismatch. No source rewrite was needed. Classification: **confirmed strict**.

## Remaining work and artifacts

[OPEN] The remaining mismatches are `ReadConfig`, `DoInit`, `ObjectStart`, and `ModifDecor`. `ReadConfig` has one fixed-byte SIB-order residue after the benchmark-boundary fix. `DoInit` exposes four target/private class-size differences; `ObjectStart` and `ModifDecor` need further source evidence. The two zero-filled BSS gaps also remain unexplained at symbol level.

- Candidate TUs: `scratch/tu/blupi.cpp`, `scratch/tu/decnet.cpp`.
- Private header overlay: `scratch/tu/blupi_include/`.
- Compiler copies/logs: `scratch/blupi/`, `build/blupi/logs/`.
- Final strict comparison: `build/blupi/results/blupi_decnet_final_current.json`.
- Pseudocode consulted: `build/decomp/tu/blupi.c`; targeted calls at `0x404350`, `0x4113c0`, and related helper entries.
- No game executable or ISO content was executed; no `src/`, `assets/`, or `external/` content was modified.
