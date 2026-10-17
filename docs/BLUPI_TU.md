# `blupi.cpp` and `decnet.cpp` reconstruction

## Scope and candidate order

[EVIDENCE] The owned candidate sources are copied from the integrated `src/blupi.cpp` and `src/decnet.cpp`. Candidate-local headers are beside them in `scratch/tu/blupi_include/`; the ordinary verification run uses current `src/*.h` copies, with the documented `SPRITE_WAIT` experiment. A separate header copy in `scratch/tu/blupi_phase_include/` tests the SB1 phase table. No canonical source or toolchain file was changed by this worker.

The target order is:

`blupi.cpp`: `GetNum`, `ReadConfig`, `UpdateFrame`, `SetDecor`, `RestoreGame`, `FlushGame`, `WindowProc`, `FinishObjects`, `TimerStep`, `InitFail`, `Benchmark`, `WinMain`, `DoInit`.

`decnet.cpp`: `NetStopCloud`, `ObjectStart`, `ObjectDelete`, `ModifDecor`, `NetPlaySound`, `NetStopSound`.

## Comparison result

[EVIDENCE] The final candidate check used `.pyenv/python.exe tools/verify.py --tu src/blupi.cpp=scratch/tu/blupi_include/blupi.cpp --tu src/decnet.cpp=scratch/tu/blupi_include/decnet.cpp --build-dir blupi_reviewed`. It ran with VC5 SP3 11.00.7022, `/W3 /GX /O2 /ML /Gd /Gy`, the manifest defines and DX6 headers. It reports 10 STRICT functions for `blupi.cpp` and 5 for `decnet.cpp`; there are no newly introduced accepted-function regressions in this candidate run.

The table counts original target body bytes. Candidate spans can include matched NOP alignment bytes after a function body.

| Function | Target RVA / body bytes | Candidate span | Status |
|---|---:|---:|---|
| `GetNum` | `0x1000 / 37` | 48 | STRICT |
| `ReadConfig` | `0x1030 / 673` | 688 | MISMATCH |
| `UpdateFrame` | `0x12e0 / 528` | 528 | MISMATCH with default phase table |
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
| `ModifDecor` | `0x10910 / 100` | 112 | STRICT |
| `NetPlaySound` | `0x10980 / 59` | 64 | STRICT |
| `NetStopSound` | `0x109c0 / 50` | 64 | STRICT |

[EVIDENCE] The 15 distinct STRICT named bodies above cover 2,913 unique target bytes. `ModifDecor`'s candidate span contains 100 instruction bytes plus 12 matched NOPs; only the 100-byte target function body is counted. The other TUs also contain duplicate candidates for `ObjectDelete` and `ModifDecor`; those do not add target bytes.

[EVIDENCE] `ObjectDelete` at `0x108a0` is owned by `decnet.cpp` and is STRICT. It is not a folded duplicate; this resolves the engine worker's earlier folded suspicion. Its `MoveObjectSearch` and `NetMessagePush` call targets are proven.

[EVIDENCE] Baseline `WindowProc` occupies `0x1610-0x1b5f`, ending at `FinishObjects` (`0x1b60`). It is STRICT with the default header context. The phase-table experiment below changes a compiler-generated local-label mapping and needs supervisor integration before that context can be used for the whole TU.

## Globals and initialized data

[EVIDENCE] The initialized globals are `g_speedRate=1` at `.data+0`, `g_timerInterval=50` at `+4`, `g_mouseType=1` at `+8`, and `g_lastPhase=999` at `+0xc`. The target bytes at `0x42e030`, `0x42e034`, `0x42e038`, and `0x42e03c` match. Config, title, error, and benchmark strings also match.

[EVIDENCE] Target zero-initialized globals occur in this order: `g_hWnd` at `0x47e618`, `g_CDPath` at `0x47e628`, `g_updateTimer` at `0x47e72c`, `WindowProc`'s static `hInstance` at `0x47e734`, then `g_pEvent`, `g_pPixmap`, `g_pSound`, `g_pNetwork`, `g_pMovie`, `g_pDecor`, the flags, and timer through `0x47e764`. The candidate's static `hInstance` is zero initialized and written by `WindowProc` from its first argument.

[EVIDENCE] The current candidate BSS offsets relative to `g_hWnd` are `g_CDPath=8`, `g_updateTimer=0x10c`, static `hInstance=0x110`, and `g_pEvent=0x114`; later pointer and scalar globals retain their order. Target bytes at `0x47e620-0x47e627` and `0x47e730-0x47e733` are zero. A static scan found no direct absolute load/store operand in either range. The `ReadConfig` pseudocode's `(&DAT_0047e627)[i]` test is consistent with `g_CDPath[i-1]`; it does not prove an object at `0x47e627`.

[OPEN] The candidate omits the 8-byte and 4-byte zero-filled gaps, so later BSS symbols are respectively 8 and 12 bytes earlier than target. The references do not identify whether the gaps are unused globals/statics or linker layout. No padding member was added.

## Header evidence and deltas

[EVIDENCE] The final candidate header copy starts from current `src/*.h`. In the private `def.h`, `SPRITE_WAIT` is changed from 12 to 11: target `DoInit` passes immediate `0xb` to the wait-sprite helper, and the candidate's corrected instruction also passes `0xb`. This corrects one `DoInit` immediate but does not make the function STRICT.

[EVIDENCE] Current `decor.h` already declares both `MoveStep()` and `MoveStep(BOOL bFirst)`. With the SB1 phase table enabled, the `UpdateFrame` object calls decorated symbol `?MoveStep@CDecor@@QAEXH@Z`; the relocation is proven to target `0x4041d0`. No new `MoveStep` declaration is needed.

[EVIDENCE] Target CDecor accesses establish `m_decor` at `this+0x10` and `m_moveObject` at `this+0x9c50`, with `MoveObject` stride `0x30`. Target `MoveObjectFree` loops over 200 entries, so that array ends at `0xc1d0`.

[EVIDENCE] Target `DoInit` allocation operands are `CPixmap=0x1b94`, `CSound=0x2b0`, `CNetwork=0xa4`, `CMovie=0x14`, `CDecor=0x258d4`, and `CEvent=0x9b90`. The current canonical candidate emits the `CPixmap`, `CSound`, `CNetwork`, `CMovie`, and `CEvent` sizes exactly. Its `CDecor` size is `0xdbec`; target CDecor has a further `0x19704` bytes after the measured 200-entry move-object array, while the current candidate has `0x1a1c`. The target's extra member layout is not established by this evidence.

[OPEN] Do not fill the CDecor size difference with an unknown member. The existing offset and stride facts locate the front of the class, but do not identify the missing tail layout.

## Compiler findings

- [EVIDENCE] `BLUPI-BENCH-01` - replacing the SB2-style benchmark body with the target ten-call loop, millisecond wrap handling, elapsed-time division by ten, exact `CheckTime = %d\r\n` output, and guarded `data\\time.blp` write makes `Benchmark` STRICT at `0x1d60` under SP3 `/O2 /Gy /GX /ML /Gd`. Classification: **Blupi-specific**.
- [EVIDENCE] `BLUPI-SETDECOR-ARGS-01` - adding the mouse point to `Build(RECT, POINT)` matches `SetDecor` at `0x14f0`; the relocation resolves to `0x404350`, whose target callee uses `ret 0x18`. Classification: **Blupi-specific**.
- [EVIDENCE] `BLUPI-UPDATEFRAME-PHASE-01` / `VC5-SEM-01` - baseline phase constants produce immediate mismatches in `UpdateFrame`. Defining the existing `SB1_EVENT_API` override in a private `def.h` copy supplies the target phase values and makes all 528 bytes STRICT at `0x12e0`, including the proven `MoveStep(BOOL)` relocation. In that same run, accepted `WindowProc` becomes MISMATCH because relocation `$L32708` resolves to `0x40190d` while its current explicit mapping expects `0x40198d`. Classification: **Blupi-specific** phase/header context; local-label mapping needs supervisor review.
- [EVIDENCE] `BLUPI-READCONFIG-SIB-01` / `VC5-CTX-01` / `VC5-CTX-03` - after the earlier boundary correction from `g_benchmark > 3099` to `g_benchmark >= 3100`, active target and candidate code are both 673 bytes. `kb.py` tags `regalloc_only`, `block_reorder`, and `sib_order`; the remaining instruction orders the SIB registers as `[eax+esi]` versus `[esi+eax]`. Context counts 0-2 at TU start, end, and before the function did not remove it. Changing the local drive buffer from 10 to 12 bytes also emitted identical code. Classification: context and buffer-size probes **negative**; final SIB residue **unverified**.
- [EVIDENCE] `BLUPI-DOINIT-SPRITE-01` / `VC5-SEM-01` - changing only private `SPRITE_WAIT` from 12 to 11 changes the target call immediate from `0xc` to `0xb`. The function remains MISMATCH; target and candidate CDecor allocation sizes are `0x258d4` and `0xdbec`. Other current class allocation sizes match. Classification: sprite constant **Blupi-specific and confirmed**; CDecor layout **open**.
- [EVIDENCE] `BLUPI-OBJECTSTART-01` / `VC5-LOC-03` / `VC5-SEM-01` - current `ObjectStart` is 480 bytes versus the 489-byte target, with six fewer decoded instructions. Direct `POINT(0,1)` direction temporaries emitted the same mismatch. Earlier probes that mutated `speed`, nested comparisons, or split the point ABI were negative. The packet fields and move-object offsets agree with target evidence; no source-supported change has closed the gap. Classification: tested source forms **negative**; remaining codegen/source form **unverified**.
- [EVIDENCE] `BLUPI-MODIFDECOR-STRICT-01` - against the current canonical header set, unchanged `ModifDecor` is STRICT at `0x10910`; its 100 instruction bytes and 12 trailing NOPs match. `kb.py` tags are empty. Classification: **Blupi-specific strict result**.
- [EVIDENCE] `WindowProc` is STRICT at `0x1610` for 1,360 target bytes with the default phase table; no source rewrite was needed.

## Remaining work and artifacts

[OPEN] Current source-level mismatches are `ReadConfig`, `UpdateFrame` under the default phase table, `DoInit`, and `ObjectStart`. The phase-table variant strictly matches `UpdateFrame`, but requires resolving the `WindowProc` local-label mapping before it can be used for the whole TU. `ModifDecor` is now STRICT with the canonical headers. The CDecor tail layout and the two BSS gaps remain unproven.

- Final candidate inputs and private header delta: `scratch/tu/blupi_include/`.
- SB1 phase-table diagnostic candidate: `scratch/tu/blupi_phase_include/`.
- Final verification output: `build/verify-blupi_reviewed/`.
- Phase-table verification output: `build/verify-blupi_phase_review/`.
- ObjectStart direct-point probe: `build/verify-decnet_pointdirs/`.
- Target pseudocode: `build/decomp/tu/blupi.c`; targeted function output from `tools/decomp.py`.
- No game executable or ISO content was executed.
