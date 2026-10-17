# decblock.cpp reconstruction

## Current verification

**[EVIDENCE]** Candidate: `scratch/decblock/decblock.cpp`, compiled with the VC5 SP3 shared-header build:

```powershell
.\.pyenv\python.exe tools\verify.py --tu src/decblock.cpp=scratch/decblock/decblock.cpp --build-dir decblock
```

The latest run reports 16 STRICT functions in this TU and one RELOC-UNVERIFIED function. The three reconstructed bodies
remain mismatches. The worker candidate contributes 880 unique target bytes to this run; no new strict bytes come from
the three bodies.

| RVA | Target function | Candidate / latest result |
|---:|---|---|
| `0x39d0` | `Create`, 196 B | 208 B, MISMATCH |
| `0x3aa0` | `LoadImages`, 183 B | 192 B, MISMATCH |
| `0x3b80` | `InitDecor`, 744 B | 768 B, MISMATCH |
| `0x3e90` | `PlayPrepare`, 703 B | 768 B, MISMATCH |
| `0x41d0` | `MoveStep`, 261 B | 272 B, MISMATCH |
| `0x4350` | `Build`, 3,866 B | 4,704 B, MISMATCH |
| `0x5270` | `DrawInfo`, 1,010 B | 992 B, MISMATCH |
| `0x5670` | `DecorNextAction`, 269 B | 272 B, RELOC-UNVERIFIED |
| `0x57b0` | `GetBlupiInfo`, 67 B | 80 B, MISMATCH in latest run |
| `0x5950` | `ChangePhase`, 208 B | 208 B, MISMATCH |

The private header beside the candidate contains two layout-preserving aliases used only in this worker:

- `BlupiPosMagicInventory` overlays the `POINT` name with two `int` count names at the same offset. `DrawInfo`
  pseudocode reads those values at `0xc958` and `0xc95c`.
- `m_blupiPosHelicoFront` overlays the second `POINT` word with the `blupiFront` flag. `Build` reads the flag at
  `0xc954`; `InitDecor` initializes that word. The zeroes at `0xc8e4/0xc8e8` are already emitted by
  `m_blupiVitesse = POINT(0, 0)`.

No shared-header delta is proposed. `src/` is unchanged.

## `GetBlupiInfo` context checkpoints

**[EVIDENCE]** These are normal full-TU results unless marked as count probes. A count probe adds scratch declarations
before `GetBlupiInfo` only to measure the context window; those declarations are diagnostics and are not candidate code.

| Checkpoint | `GetBlupiInfo` result | Count-probe strict windows when run |
|---|---|---|
| Before the new bodies | MISMATCH | 3–6, 11–14, 19–22, 27–30 |
| After inserting `InitDecor` | STRICT | Not needed at that checkpoint |
| After inserting the first `Build` body | MISMATCH | 4–7 |
| After first `DrawInfo` draft, before tutorial overlay | MISMATCH | 1–4, 9–12, 17–20 |
| After adding tutorial overlay and target-shaped table walk | STRICT | Not needed at that checkpoint |
| After subsequent `Build` corrections | MISMATCH | Latest probe: 3–6, 11–14, 19–22 |

The latest candidate is close to a strict probe window, but it does not match in its actual TU context. The strict result
after the tutorial walk is evidence of a context effect, not evidence that either `DrawInfo` or `GetBlupiInfo` is a
strict match. Keep `GetBlupiInfo` withdrawn from the shared regression gate until a normal full-TU run reports STRICT.

## Body reconstruction and findings

### `InitDecor` (`0x3b80`)

**[EVIDENCE]** Added before `SetTime`, in target order. It clears the two 100×100 icon maps and 200 move-object type
fields, sets the three seed tiles and two default moving objects, initializes four start positions, and initializes the
target-backed Blupi, decor, scroll, treasure, energy, and phase fields. The target writes the front flag at `0xc954`;
the candidate does not initialize the unrelated first word at `0xc950`. The current body is 768 B versus 744 B.

### `Build` (`0x4350`)

**[EVIDENCE]** Added in target order before `DecorNextAction`. The body follows the target background and map passes,
start markers, multiplayer and local Blupi drawing, moving-object passes, dynamic decor, overlays, `DrawInfo`,
`VoyageDraw`, clipping restore, and time update. Current size is 4,704 B versus 3,866 B; its frame is `0x4c` versus
the target `0x44`.

**[EVIDENCE]** The initial SB2-derived body measured 5,152 B. Target-backed pruning removed the official-mission gate,
the extra 384/385 layer, newer particle classes (types 53 and 90–100), and SB2-only type filters. The target's
build-mode movement endpoint overlays (icons 31 and 30) were added from the pseudocode. Replacing the SB2 static-tile
switch skip list with the target predicate (skip 68, 91, 92, and 110–137) reduced the candidate from 4,880 B to
4,704 B. These changes improve behavior and size, but the body remains MISMATCH.

### `DrawInfo` (`0x5270`)

**[EVIDENCE]** Added after `Build`. It renders notifications, lives, bullet/person counts, treasure text, gauges, the
tutorial overlay, build highlight, and playtest text. The first version omitted the tutorial overlay and measured 720 B;
adding the target-backed overlay grew it to the current 992 B versus 1,010 B.

**[EVIDENCE]** The lives and bullet loops now use the target's single guarded loop shape rather than a redundant outer
guard plus loop guard. Tutorial rows advance by six entries through the flat table; the candidate carries the lower x
bound separately and walks the remaining row fields with a pointer, following the target pseudocode. This change
temporarily restored `GetBlupiInfo` to STRICT in a normal full-TU verification. `DrawInfo` itself remains MISMATCH.

### Existing neighboring methods

**[EVIDENCE]** `DecorNextAction` follows target method order and omits the SB2-only pause test. All non-relocation bytes
match at `0x5670`; six relocations to `table_decor_action` are not proven because the table has no unique reconstructed
definition. It is RELOC-UNVERIFIED, not a match.

## Compiler findings

- **[EVIDENCE; confirmed SB1 context effect, `VC5-CTX-01..04`]** Trigger: `GetBlupiInfo` mismatched while the preceding
  bodies were absent. Adding a genuine `InitDecor` body before it changed the normal full-TU result to STRICT; adding
  the tutorial walk in `DrawInfo` also restored STRICT at one checkpoint. Later source corrections moved the result
  back to MISMATCH; the latest before-definition probe is strict at counts 3–6, 11–14, and 19–22. Flags/TU: VC5 SP3,
  `/O2 /GX /ML /Gd /Gy`, complete `decblock.cpp`. Classification: **blupi-specific**; this confirms context
  sensitivity, not a transferable source rewrite.

- **[EVIDENCE; SB1 behavior correction, `BLUPI-REG-01` / `VC5-ORD-01`]** Trigger: the SB2 `Build` body was 5,152 B
  versus 3,866 B. Removing target-absent drawing paths and using the pseudocode's reverse move-object walk, static
  tile predicate, and build endpoint overlays reduced it to 4,704 B. The resulting body is still MISMATCH, so these
  are target-backed behavior corrections, not a strict compiler rule. Classification: **blupi-specific**.

- **[EVIDENCE; negative, `BLUPI-LOC-01` / `VC5-SEM-01`]** Trigger: `Build` retained a 0x4c frame versus the target
  0x44 after pruning. Moving the initial `tinyPoint.x = 0` after `DecorNextAction` matches target statement order, but
  the frame remains 0x4c and the function remains MISMATCH. The unused SB2 local `num` was removed under the no-unused
  code rule. Classification: **negative** for that source-form probe.

- **[EVIDENCE; SB1 source-shape probe, `VC5-ORD-01`]** Trigger: `DrawInfo` emitted a redundant count guard before the
  lives and bullet loops. Removing the outer guards retained the loop semantics and made the candidate instruction
  prefix match the target shape. The body remains 992 B / MISMATCH. Classification: **blupi-specific**.

- **[EVIDENCE; layout evidence, `VC5-SEM-01`]** `DrawInfo` uses private aliases for values that target pseudocode reads
  at `0xc958/0xc95c`; `Build` reads the front flag at `0xc954`. These offsets explain the worker header view, but no
  canonical header change is proposed. Classification: **unverified** pending cross-TU header integration.

## Next work

**[OPEN]** Continue comparing `Build` source shape against `target_4350.c`, starting with the `0x44` versus `0x4c`
frame and the large early-background/map-loop divergence. Improve `DrawInfo` around its tutorial-table loop and
`InitDecor` around initialization store order. Re-run the exact full-TU verification after each source revision; retain
the latest count-probe window only as context evidence. Do not promote any function until all non-relocated bytes match
and every relocation target is proven.
