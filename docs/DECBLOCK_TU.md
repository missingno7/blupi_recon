# decblock.cpp reconstruction

## Current verification

**[EVIDENCE]** Refreshed candidate: `scratch/decblock/decblock.cpp`, copied from
`src/decblock.cpp`; `scratch/decblock/decor.h` is copied from the current
canonical header and contains the recovered member names. The latest complete
run used the pinned VC5 SP3 toolchain and the capped verifier:

```powershell
.\.pyenv\python.exe tools\verify.py --tu src/decblock.cpp=scratch/decblock/decblock.cpp --build-dir decblock
```

The decblock object has 17 STRICT functions, zero RELOC-UNVERIFIED functions,
and 52 MISMATCH functions; the run recognizes 960 unique original bytes in
this TU. `GetBlupiInfo` is STRICT in `14_decblock.obj`. The corrected canonical
tree has no competing `GetBlupiInfo` definition in `decblupi.cpp`. No canonical
source or header was changed.

| RVA | Function | Latest candidate result |
|---:|---|---|
| `0x39d0` | `Create` (196 B target) | 208 B, MISMATCH |
| `0x3aa0` | `LoadImages` (183 B target) | 192 B, MISMATCH |
| `0x3b80` | `InitDecor` (744 B target) | 752 B, MISMATCH; score 453 |
| `0x3e90` | `PlayPrepare` (703 B target) | 768 B, MISMATCH |
| `0x41d0` | `MoveStep` (261 B target) | 272 B, MISMATCH |
| `0x4350` | `Build(RECT, POINT)` (3,866 B target) | 4,624 B, MISMATCH; score 5,271; frame `0x48` vs target `0x44` |
| `0x5270` | `DrawInfo` (1,010 B target) | 992 B, MISMATCH; score 225 |
| `0x57b0` | `GetBlupiInfo` (67 B target) | 80 B compared section, STRICT in `14_decblock.obj` |
| `0x5950` | `ChangePhase` (208 B target) | 208 B, MISMATCH |

The score helper compares target section spans, including trailing bytes:
`InitDecor` 453, `Build` 5,271, `DrawInfo` 225, and `GetBlupiInfo` 0. For
`Build`, the target body is 3,866 B and the compared span is 3,872 B.

## `GetBlupiInfo` context checkpoints

**[EVIDENCE]** The target body reads offsets `0xc90c`, `0xc910 | 0xc914`,
`0xc918`, and `0xc91c | 0xc920` (`tools/decomp.py 0x57b0`). The refreshed
candidate is STRICT at `0x57b0` with the recovered shared CDecor layout. The
historical TU-context probes below explain why the tutorial loop identity is
kept; the current candidate's final full-TU result is STRICT.

| Candidate change | Normal decblock-object result | Diagnostic before-definition probe |
|---|---|---|
| Direct `POINT`-preserving member unions, before the Build signature correction | STRICT | Not run at that checkpoint |
| Add the call-site-backed `POINT` parameter to `Build` | MISMATCH | STRICT at extra counts 4-7, 12-15, and 20-23 |
| Reuse `DrawInfo`'s function-scope loop index in the tutorial walk | STRICT | Not rerun; normal full-TU result is STRICT |

The count probe above was sequential and diagnostic-only. It did not add
declarations to candidate source. The corrected tree removed the former
duplicate definition, and the refreshed full run reports `GetBlupiInfo`
STRICT.

## Recovered shared layout

**[EVIDENCE]** No header delta is proposed in this round. The shared
`src/decor.h` already has the recovered members: `m_blupiFront` at `0xc954`,
`m_blupiBullet` at `0xc958`, and `m_blupiPerso` at `0xc95c`. The refreshed
candidate uses those names directly in `Build`, `InitDecor`, and `DrawInfo`.
The `POINT` members `m_blupiPosHelico` and `m_blupiPosMagic` remain intact.
Offsets and supporting target evidence are recorded in `docs/LAYOUT_DEBT.md`;
the worker's private header is only a copy of the current canonical header.

## Body reconstruction and compiler findings

### `InitDecor` (`0x3b80`)

**[EVIDENCE]** The candidate clears both 100x100 icon maps, seeds three tiles,
clears 200 move-object type fields, initializes the two default moving
objects and start positions, then sets Blupi, decor, scroll, treasure, energy,
and phase state. With the recovered shared layout it is 752 B versus 744 B;
current score 453.

- **[EVIDENCE; negative, `VC5-SEM-01`]** Replacing the two `-1` map sentinels
  with target-spelled `0xffff` produced no codegen change: still 752 B, score
  457, and 33 exact matching instructions. The explicit literal is retained
  because the target pseudocode stores `0xffff`; SB2 source uses `-1`.
- **[EVIDENCE; negative, `VC5-SEM-01`]** Replacing the four scalar
  `m_posDecor`/`m_dimDecor` stores with two `POINT` aggregate assignments,
  as in SB2, worsened the score from 457 to 487 and exact instruction matches
  from 33 to 24, with no size change. The experiment was reverted.
- **[EVIDENCE; negative, `VC5-ORD-01`]** Moving the inventory reset after the
  energy reset also worsened the score and was reverted.
- **[EVIDENCE; negative, `VC5-CTX-01..04` / `VC5-SEM-01`]** After refreshing
  the corrected header, replacing the indexed nested map clear with a
  decrementing pointer walk through `m_bigDecor` and its preceding `m_decor`
  storage grew `InitDecor` from 752 to 768 B and worsened its score 453 -> 565
  (11 exact instructions versus 34). The full-TU run also changed following
  `GetBlupiInfo` from STRICT to MISMATCH. The source was reverted; the current
  752 B `InitDecor` remains MISMATCH and `GetBlupiInfo` is STRICT. Classification:
  **negative, blupi-specific**.

The 457/487 scores above are historical scores from the prior header context;
the refreshed baseline score with the recovered layout is 453.

### `Build` (`0x4350`)

**[EVIDENCE]** The candidate follows the target background and map passes,
start markers, multiplayer/local Blupi drawing, moving objects, dynamic decor,
overlays, `DrawInfo`, `VoyageDraw`, clipping restore, and time update. It
remains 4,624 B versus 3,866 B, with score 5,271 and a `0x48` frame versus the
target `0x44`.

Target call-site bytes resolve the parameter shape. At caller `0x14f0`, the
code pushes two point coordinates, reserves and fills a 16-byte `RECT`, then
calls `0x4350`; the target returns with `ret 0x18`. The candidate now declares
`Build(RECT rect, POINT mousePos)`, matching the existing shared declaration
and both target call sites. The Ghidra body does not use the point, and the
candidate does not consume it. This correction changed the score from 5,275
to 5,271 and emitted the expected 24-byte stack cleanup; the frame and body
size did not change.

The initial SB2-derived body measured 5,152 B. Target-backed pruning removed
the official-mission gate, extra 384/385 layer, newer particle classes, and
SB2-only type filters. The target static-tile predicate and build movement
endpoint overlays remain. A previous shared-local identity probe did not
improve the score or frame and was reverted.

The refreshed candidate uses the recovered `m_blupiFront` member directly.
An exhaustive seven-compile `tools/permute.py` pass over the early background
and map-loop lines tested independent assignment swaps and `for`-to-`while`
forms. All six mutations retained the 5,271 score and 4,624 B size, so the
candidate was left unchanged. The campaign ran alone at the default capped
jobs setting.

### `DrawInfo` (`0x5270`)

**[EVIDENCE]** The candidate draws notifications, lives, bullet/person counts,
treasure text, gauges, tutorial overlay, build highlight, and playtest text.
It is 992 B versus 1,010 B; score 225. The source comparison
`cel.x >= left` is retained; it improved the prior score from 229 to 225
without changing size.

The tutorial walk now reuses the function-scope `i`, matching the SB2 loop
identity and target pseudocode. This did not change `DrawInfo` bytes or score,
but it restored `GetBlupiInfo` to STRICT in the following full-TU run.

## Compiler findings

- **[EVIDENCE]** The source-form experiments below used the pinned VC5 SP3
  `/O2 /GX /ML /Gd /Gy` build in the complete decblock TU; each candidate
  change was followed by a full capped verification.
- **[EVIDENCE; blupi-specific context effect, `VC5-CTX-01..04`]** Changing the
  genuine `Build` formal list changed `GetBlupiInfo` from STRICT to MISMATCH;
  reusing the existing tutorial index restored STRICT. The intervening probe
  window was +4..7, +12..15, and +20..23. The refreshed decblock-object body is
  STRICT; the former `decblupi.cpp` duplicate has since been removed from the
  canonical tree. This confirms context sensitivity for this TU, not a
  transferable count-padding recipe.
- **[EVIDENCE; source/ABI correction, `VC5-SEM-01`]** Two target call sites
  pass a 16-byte `RECT` plus a two-word `POINT`; the target `ret 0x18` confirms
  24 bytes of callee cleanup. Updating the candidate signature produced
  `ret 0x18` and score 5,275 -> 5,271, but Build remains MISMATCH at 4,624 B.
  Classification: **blupi-specific**.
- **[EVIDENCE; target-backed body pruning, `BLUPI-REG-01` / `VC5-ORD-01`]**
  Removing SB2-only drawing paths reduced Build from 5,152 B to 4,624 B. The
  body remains MISMATCH; these are behavior corrections, not a strict compiler
  rule. Classification: **blupi-specific**.
- **[EVIDENCE; source-form result, `VC5-ORD-01`]** Reusing `DrawInfo`'s
  function-scope index in the tutorial walk left its machine code unchanged
  but restored following `GetBlupiInfo` to STRICT. The point comparison
  `cel.x >= left` independently improved the score 229 -> 225. Classification:
  **blupi-specific**.
- **[EVIDENCE; negative, `VC5-SEM-01`]** `POINT` aggregate initialization in
  `InitDecor` worsened score 457 -> 487; the 752 B function remains MISMATCH.
  Changing `-1` to `0xffff` had no codegen effect and also remains MISMATCH.
  Classification: **negative** for those source-form probes.
- **[EVIDENCE; negative, `VC5-ORD-01`]** The seven-compile early-`Build`
  permutation campaign tested independent assignment ordering and loop forms
  against `0x4350` in the authentic decblock TU. Every variant remained
  MISMATCH at 4,624 B and score 5,271; no strict result or score improvement.
  Classification: **negative for those local source-form changes**.

## Next work

**[OPEN]** The corrected layout and names are now reflected in the refreshed
candidate, but `Build`, `InitDecor`, and `DrawInfo` remain MISMATCH. Continue
only with target- or ancestry-supported body changes; keep the two-argument
`Build` signature and the `DrawInfo` tutorial index reuse. Re-run the full
verifier after each source edit and check `GetBlupiInfo` each time. Do not
promote a mismatch or a relocation-unverified function as a strict match.

## Senior pass (sol-decblock, 2026-10-17)

### Result and reviewable candidate

[EVIDENCE] This section supersedes the earlier mismatch results for InitDecor
and DrawInfo. Candidate: `scratch/soldecblock/decblock.cpp`. Only InitDecor,
Build and DrawInfo differ from the current canonical source; a text comparison
with these three bodies removed passed. No canonical source, header, manifest,
catalog, external reference or original asset was changed by this pass.

[EVIDENCE] Final verification used the complete manifest, pinned VC5 SP3,
`/W3 /GX /O2 /ML /Gd /Gy`, canonical headers, and:

```powershell
.pyenv\python.exe tools\verify.py --tu src/decblock.cpp=scratch\soldecblock\decblock.cpp --build-dir soldecblock
```

The verifier exited **0**, preserving the accepted-function gate. Evidence is
in `build/verify-soldecblock/results.json`, with the transcript and exit code
copied to `scratch/soldecblock/final-verify.log` and `final-verify-exit.txt`.
The final source, object, disassemblies and measurements are archived under
`scratch/soldecblock/final/`.

| Function | RVA | Original body bytes | Candidate body / compared extent | Final proof |
|---|---:|---:|---:|---|
| InitDecor | `0x3b80` | 744 | 744 / 752 | STRICT; no relocations |
| Build | `0x4350` | 3,866 | 3,846 / 3,856 | MISMATCH; frame `44`, score 1,489 |
| DrawInfo | `0x5270` | 1,010 | 1,010 / 1,024 | STRICT; 25/25 relocations proved |
| GetBlupiInfo, unchanged source | `0x57b0` | 67 | 67 / 80 | STRICT; no relocations |

[EVIDENCE] **Two new STRICT bodies, 1,754 unique original bytes.** GetBlupiInfo
is preserved, not counted as new coverage. The comparator includes terminal
alignment NOPs in these COFF extents; their bytes are not function-body credit.
DrawInfo's isolated `asmscore` result is 1 because the single-object run lacks
the tutorial table's definition. The complete verifier proves all 25 relocations
and reports STRICT; this is not a relocation-unverified claim.

### Stack map before instruction tuning

[EVIDENCE] Build's target prologue allocates `44` bytes and saves four registers.
Let E be entry ESP and S = E-54h, the steady body ESP. Pushes and temporary
argument reservations change literal `[esp+N]` displacements; the table uses S.
The target and final candidate agree on the frame and aggregate storage below.
Scalar spill roles are recovered from their defining arithmetic and subsequent
uses, not Ghidra's unreliable local names.

| Target storage | Effective address | Recovered source role / reuse |
|---|---|---|
| `S+10` | `E-44` | Animated-map inner index multiplied by seven; scalar spill |
| `S+14` | `E-40` | Cached `posDecor.x/64`, later map base/count spill |
| `S+18` | `E-3c` | Start-direction cursor, network-player byte offset, then `i*7` / `i*13` |
| `S+1c` | `E-38` | Cached horizontal remainder, later inner-loop/count spill |
| `S+20` | `E-34` | Initial tile-y index, then network-position cursor |
| `S+24` | `E-30` | Outer loop count, then network icon/security cursor |
| `S+28` | `E-2c` | Background vertical remainder `y`, start-label cursor adjustment, later grid bound |
| `S+2c,30` | `E-28,-24` | `posDecor.x,y` |
| `S+34..40` | `E-20..-14` | 16-byte GetClipping return temporary; later `tinyPoint.x` and reusable spills |
| `S+44..50` | `E-10..-4` | Saved `lastClip` RECT |
| `S+58` | `E+4` | Incoming RECT's dead first word reused for loop counts and POINT-return scratch |

[EVIDENCE] Final candidate also spills foreground `tinyPoint.y` at `S+38`
where the target keeps that value in a register. This reuses the dead clipping
temporary and does not enlarge the frame. The remaining raw operands above
`S+50` in call construction are shifted views of saved locals or incoming
arguments, not additional local buffers. Complete raw access lists are in
`final/Build-{target,candidate}-slots.txt`.

[EVIDENCE] InitDecor has no allocated local frame. DrawInfo already had the
correct `74`-byte frame: its 100-byte text buffer begins at steady `esp+20`
(`E-64`), highlight icon at `esp+10`, outer highlight index at `esp+14`, and
cached highlight y at `esp+1c`. No buffer-size or packing change was needed.

### Region reconstruction

[EVIDENCE] InitDecor now uses component stores for the two initial moving
objects' starting coordinates and the four player starts, with actual POINT
copies for posEnd, posCurrent, valid position and scroll point. Unsupported
clears at `c924`, `c944`, `c948` and `c94c` are removed. Suspend is cleared in
the target state sequence; Power precedes Cloud. Inventory, energy, velocity
and treasure stores follow the target's source dependencies. Keeping
`m_totalTresor = 1` after the treasure reset, rather than near Focus, closes
the final six-instruction scheduling difference and produces STRICT.

[EVIDENCE] DrawInfo uses the ordinary six-word tutorial-array loop with the
existing `i`. The joystick branches each call LoadString with their own
argument expression. Highlight icons use channel 1, and phase is rechecked
after highlight drawing before loading the build caption. Existing `pos`
component assignments replace fixed-coordinate POINT helper invocations
without changing DrawInfo bytes. The final body is fully STRICT.

[EVIDENCE] Build now follows these target regions in order: saved clipping;
four background parts; large-map tiles; editor starts; remote players;
local player and effects; reverse moving-object pass; static tiles; lifts
and movement endpoints; animated tiles; foreground object classes 8..11;
front-layer player; DrawInfo; VoyageDraw; clipping restore; time increment
and reset at 32768. All **44 call sites** remain present, although instruction
and cold-block placement still differ.

[EVIDENCE] Target-backed changes remove trap, drop, crusher, saw, temporary
tile and lightning animation paths absent from this body. Lava uses
`(i*13+j*7+time/3)%5`; both water families use the corresponding `%4`.
Shield uses four frames and a single image at y+2; the extra SB2 shield-loop
image is removed. The local flag sequence is Shield/Power/Cloud/Hide, using
the recovered canonical members. Hide's visible arm composes all three
temporary images and displays temporary icon zero. The remote player's saved
channel is remapped only when it is 2 and the team is positive. Secret tiles
are suppressed only during ordinary play when secret drawing is disabled.

[EVIDENCE] The editor-start loop begins at 4 and includes zero in the oracle.
Its label x adjustment is emitted as +10 followed by `i*8-12`; the candidate
retains that arithmetic. [OPEN] Why the target includes index four, crossing
the currently proven four-element start arrays, remains unexplained. No
array extension or shared-layout change is proposed to conceal it.

### Iteration ledger

[EVIDENCE] Full per-function score, extent, raw prefix and normalized
instruction-prefix measurements for each successful trial are in
`scratch/soldecblock/ledger.csv`; each step directory retains source/object.
Selected Build checkpoints follow. Extents include padding; oracle extent
is 3,872, body 3,866. Fixed prefix skips COFF relocation operands and is only
a diagnostic: it does not prove those relocations or a partial STRICT match.

| Step | Change | Extent | Frame | Score | Fixed / instruction prefix |
|---|---|---:|---:|---:|---:|
| s00 | Canonical baseline | 4,624 | 48 | 5,271 | 2 / 0 |
| s03 | Target regions, effects, clipping and time tail | 4,080 | 50 | 3,429 | 2 / 0 |
| s04 | Component POINT arithmetic, shared drawing point | 3,920 | 4c | 2,629 | 2 / 0 |
| s12 | One final foreground QuickIcon with selected icon/channel | 3,904 | 4c | 2,509 | 2 / 0 |
| s13 | Background bottom outside inner loop | 3,856 | 48 | 2,145 | 2 / 0 |
| s18 | Scalar cropped-background coordinates | 3,856 | 44 | 2,087 | 102 / 34 |
| s26 | Target start marker branch and geometry | 3,840 | 44 | 1,679 | 102 / 34 |
| s27 | Shared sprite/channel/inner-index locals | 3,840 | 44 | 1,679 | 102 / 34 |
| s28 | Scale into POINT, then scalar remainders | 3,856 | 44 | 1,507 | 163 / 52 |
| s32 / final | Assign cropped left before computing vertical remainder | 3,856 | 44 | 1,489 | 202 / 67 |

[EVIDENCE] s02 first made DrawInfo STRICT. InitDecor scores were 453 at
baseline, 123 after semantic/component fixes (s05), 49 with POINT copies
(s06), 13 with copy placement restored (s07), and 0 with late totalTreasure
assignment (s08). Later retained changes preserve both STRICT bodies.

### Compiler findings

- [EVIDENCE; `VC5-SEM-01`, `VC5-ORD-01`; blupi-specific] InitDecor's
  constructor-helper coordinate assignments cached values where the target
  reloads member coordinates. Explicit start components plus real POINT
  copies, removing unsupported clears and restoring late treasure assignment,
  reach 744 body bytes and STRICT. Moving the valid/scroll copies early gives
  score 49; restoring their later source positions gives 13; moving total
  treasure back after the reset gives 0. Same complete TU/SP3 flags throughout.
- [EVIDENCE; `VC5-ORD-01`, `VC5-CTL-03`; blupi-specific] DrawInfo's shared
  stringId temporary produced one LoadString argument path where the oracle
  branches to two argument preparations sharing the call. Array indexing,
  branch-local calls, the real channel and phase recheck restore 1,010 body
  bytes and STRICT, including all 25 full-run relocations. This does not claim
  that duplicating every call is a transferable optimization rule.
- [EVIDENCE; `BLUPI-LOC-01`, `VC5-LOC-03`; blupi-specific allocation effect]
  Build's frame maps identify a reserved cropped-coordinate POINT field and
  excess scalar spills. Moving bottom outside the inner loop changes frame
  `4c -> 48`; scalar cropped coordinates change `48 -> 44`. Scaling into the
  existing drawing POINT before scalar modulo calculation improves prefix
  102 -> 163 without frame growth. Assignment of the left crop before the
  vertical remainder improves prefix to 202. Build remains MISMATCH.
- [EVIDENCE; `VC5-CTX-01..04`, `BLUPI-REG-02`; blupi-specific context effect]
  s26's independent sprite/channel/inner-index locals versus s27's genuinely
  shared locals produce **byte-identical Build bodies**, but GetBlupiInfo
  changes MISMATCH -> STRICT. Both share the correct frame and extent. No
  unused declarations or count-padding probes were retained or used in this
  pass. Earlier broad sharing (s09) was inert at its earlier context; local
  identity must be evaluated in the actual final TU.
- [EVIDENCE; `VC5-ORD-01`, `BLUPI-REG-01`; negative families] Splitting scalar
  division/modulo statements (s10/s19), aggregate background initialization
  (s15), and moving invariant bottom outside both loops (s30 versus s28) were
  inert at their checkpoints. Global icon/channel sharing was also byte-inert
  for Build itself. A copied background POINT (s16), temporary BOOL rendering
  selection (s17/s21), goto completion (s20), icon sentinel (s25), and repeated
  direct QuickIcon calls (s29) did not recover the target CFG/register layout;
  these variants are not retained. A late-channel sentinel (s33) lowers the
  heuristic score to 1,293 but emits an extra compare/branch absent from the
  target; it is rejected despite the lower score. No family produces STRICT
  Build.

### Header deltas and remaining work

[EVIDENCE] **No header delta.** Canonical `m_netBlupiTransport[i]` already
occupies the channel slot read at `ca54+i*4`; the candidate uses that name.
The target's channel-1 drawing arguments are explicit in these bodies rather
than changing the shared SB2-era CHOBJECT constant. The recovered state-member
layout is used without private header overlays.

[OPEN] Build's first remaining fixed difference is `+ca`: background height
materialization and subsequent loop scheduling. Foreground rendering still
loads icon/channel into locals before its common call; the target pushes
coordinates/icon before GetBlupiChannelActual. This also affects cold-block
placement and point-register spills. Continue from final/s32, preserving
GetBlupiInfo and the two new STRICT bodies; do not resurrect the rejected
sentinel solely for its lower score. Several Build table definitions
(`table_shield_blupi`, lava, water) are still absent from canonical dectables;
their values and relocation ownership will need proof before STRICT can be
claimed even after fixed bytes agree. These are future data-owner work, not
header changes made by this worker. No background process is left running.
