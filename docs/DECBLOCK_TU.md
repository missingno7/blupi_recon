# decblock.cpp reconstruction

## Current verification

**[EVIDENCE]** The worker candidate is `scratch/decblock/decblock.cpp`; it was checked with VC5 SP3 and the shared headers using:

```powershell
.\.pyenv\python.exe tools\verify.py --tu src/decblock.cpp=scratch\decblock\decblock.cpp --build-dir decblock
```

The final run reports 16 STRICT decblock functions and one RELOC-UNVERIFIED candidate. The 16 strict functions retain their baseline results. No new unique target bytes are counted from this pass.

| RVA | Target body | Candidate / result |
|---:|---|---|
| `0x39d0` | `Create`, 196 B | 208 B, MISMATCH |
| `0x3aa0` | `LoadImages`, 183 B | 192 B, MISMATCH |
| `0x3b80` | `InitDecor`, 744 B | No candidate in the worker source |
| `0x3e90` | `PlayPrepare`, 703 B | 768 B, MISMATCH |
| `0x41d0` | Tick/input helper (`MoveStep` candidate), 261 B | 272 B, MISMATCH |
| `0x4350` | `Build`, 3,866 B | No candidate in the worker source |
| `0x5270` | `DrawInfo`, 1,010 B | No candidate in the worker source |
| `0x5670` | `DecorNextAction`, 269 B | 272 B, RELOC-UNVERIFIED at a unique location |
| `0x57b0` | `GetBlupiInfo`, 67 B | 80 B, MISMATCH at the forced target RVA |
| `0x5950` | `ChangePhase`, 208 B | 208 B, MISMATCH |

**[EVIDENCE]** The `DecorNextAction` candidate follows the target method order after `GetIconPerso`. The target pseudocode has an early return when `m_decorAction == 0`; it does not test `m_bPause`. Removing the SB2 ancestor's pause test reduced the candidate from 304 to 272 bytes. All non-relocation bytes match at `0x5670`; six relocations reference `table_decor_action` at `0x42ea28`, but the run has no unique reconstructed table definition, so the result is not STRICT. The table payload remains unresolved.

**[EVIDENCE]** `GetBlupiInfo` stayed 80 B versus the 67 B oracle body in the baseline run, after adding `DecorNextAction` in source order, and after removing the target-absent pause check from that method. The function is still withdrawn and is not accepted as a match. Earlier probes in [HEADERS.md](HEADERS.md#whole-tree-and-isolation-experiments) and [ENGINE_PROBE.md](ENGINE_PROBE.md) remain the relevant findings: its bitwise-OR candidate is better than the tested logical-OR form, and the required earlier front-end context has not been identified.

## Prior probes used

**[EVIDENCE]** The oracle-first and SB2-derived probes in [ENGINE_PROBE.md](ENGINE_PROBE.md) already cover the incomplete initialization and play-preparation bodies. The forward 200-record `PlayPrepare` reconstruction with the target-backed narrowed randomization list is 768 B versus 703 B and remains MISMATCH. The prior `Build` probe removed the SB2-only debug overlay and measured 5,152 B with a `0x48` frame versus the target's 3,866 B and `0x44` frame; only the initial two prologue bytes matched. The direct `InitDecor` and `DrawInfo` transfers were not compilable against the shared SB1 declarations. These remain open rather than being filled with offset-only or dummy source constructs.

**[EVIDENCE]** `Create`, `LoadImages`, `MoveStep`, and `ChangePhase` were forced to their assigned RVAs in a `cmpfunc.py` diagnostic using all objects from the final run. `Create` and `MoveStep` diverge in the entry sequence; `ChangePhase` has matching initial instructions but remains a body mismatch. The `LoadImages` candidate has the target-sized `0x34` stack allocation and matching first instructions, but is still MISMATCH. No source rewrite from these diagnostics is promoted.

## Compiler findings

- **[EVIDENCE; Blupi-specific, `VC5-SEM-01`]** `DecorNextAction` mismatch: SB2 source tests `m_decorAction == 0 || m_bPause`; target pseudocode tests only `m_decorAction == 0`. With VC5 SP3 `/O2 /GX /ML /Gd /Gy` and the shared headers, removing the pause condition reduced the body from 304 to 272 B and made every non-relocation byte match at `0x5670`. The six data relocations remain unverified because `table_decor_action` has no unique reconstructed definition; final status is RELOC-UNVERIFIED, not a match.

- **[EVIDENCE; Blupi-specific, `VC5-CTX-01..04`]** Inserting a genuine, target-ordered `InitDecor` body before `GetBlupiInfo` changed `GetBlupiInfo` from MISMATCH to STRICT in the normal full-TU verification. The 784-byte `InitDecor` candidate still mismatches its 744-byte target. A scratch-only baseline `before:GetBlupiInfo` probe had strict windows at filler counts 3–6, 11–14, 19–22, and 27–30; because the normal post-insertion build is already strict, no post-insertion count sweep was needed. This establishes the context effect, not an `InitDecor` body match.

## Header and source deltas

**[EVIDENCE]** No shared-header delta is proposed. `src/` was not modified. The candidate and build artifacts remain under `scratch/decblock/` and `build/verify-decblock/`.

**[EVIDENCE]** The follow-up added a behavior-backed `InitDecor` body before `SetTime`, in target order. Verification reports `InitDecor` MISMATCH (784 candidate bytes / 744 target bytes) and `GetBlupiInfo` STRICT at `0x57b0` in the decblock object (80 candidate bytes). The latter is withdrawn from the shared regression gate, but this worker's object comparison is strict.

**[OPEN]** Continue reconstructing `Build` and `DrawInfo` from target pseudocode and the SB2 ancestor, in target order, then improve `InitDecor` toward strict. Identify and reconstruct the action table payload. `Create`, `LoadImages`, and `ChangePhase` also remain mismatches.
