# decblock.cpp reconstruction

## Current verification

**[EVIDENCE]** Candidate: `scratch/decblock/decblock.cpp`, with its private
`scratch/decblock/decor.h` overlay. The latest complete run used the pinned VC5
SP3 toolchain and the capped verifier:

```powershell
$env:BLUPI_JOBS='4'
.\.pyenv\python.exe tools\verify.py --tu src/decblock.cpp=scratch/decblock/decblock.cpp --build-dir decblock
```

The decblock object has 17 STRICT functions, one RELOC-UNVERIFIED function,
and 52 MISMATCH functions; the run recognizes 960 unique original bytes in
this TU. `GetBlupiInfo` is STRICT in `14_decblock.obj`. A second definition
from `17_decblupi.obj` still mismatches at the same target RVA, so the shared
regression gate keeps the function withdrawn. No canonical source or header
was changed.

| RVA | Function | Latest candidate result |
|---:|---|---|
| `0x39d0` | `Create` (196 B target) | 208 B, MISMATCH |
| `0x3aa0` | `LoadImages` (183 B target) | 192 B, MISMATCH |
| `0x3b80` | `InitDecor` (744 B target) | 752 B, MISMATCH |
| `0x3e90` | `PlayPrepare` (703 B target) | 768 B, MISMATCH |
| `0x41d0` | `MoveStep` (261 B target) | 272 B, MISMATCH |
| `0x4350` | `Build(RECT, POINT)` (3,866 B target) | 4,624 B, MISMATCH; frame `0x48` vs target `0x44` |
| `0x5270` | `DrawInfo` (1,010 B target) | 992 B, MISMATCH |
| `0x5670` | `DecorNextAction` (269 B target) | 272 B, RELOC-UNVERIFIED; six table relocations remain unproved |
| `0x57b0` | `GetBlupiInfo` (67 B target) | 80 B candidate section, STRICT in `14_decblock.obj` |
| `0x5950` | `ChangePhase` (208 B target) | 208 B, MISMATCH |

The score helper compares the target section span, including its trailing
bytes: `InitDecor` 457, `Build` 5,271, `DrawInfo` 225, and `GetBlupiInfo` 0.
For `Build`, the target body is 3,866 B and the compared span is 3,872 B.

## `GetBlupiInfo` context checkpoints

**[EVIDENCE]** The target body reads offsets `0xc90c`, `0xc910 | 0xc914`,
`0xc918`, and `0xc91c | 0xc920` (`tools/decomp.py 0x57b0`). The decblock
candidate body has those field semantics; its STRICT/MISMATCH transitions
track TU context. The duplicate in `decblupi.obj` has different field
semantics and remains MISMATCH.

| Candidate change | Normal decblock-object result | Diagnostic before-definition probe |
|---|---|---|
| Direct `POINT`-preserving member unions, before the Build signature correction | STRICT | Not run at that checkpoint |
| Add the call-site-backed `POINT` parameter to `Build` | MISMATCH | STRICT at extra counts 4-7, 12-15, and 20-23 |
| Reuse `DrawInfo`'s function-scope loop index in the tutorial walk | STRICT | Not rerun; normal full-TU result is STRICT |

The count probe above was sequential and diagnostic-only. It did not add
declarations to candidate source. The last full run still reports the
decblock-object `GetBlupiInfo` as STRICT; the separate `decblupi.obj` copy is
why the shared regression gate remains withdrawn.

## Shared-header delta proposal

**[EVIDENCE]** The worker's private header now expresses these layout-preserving
aliases directly while retaining both existing `POINT` member names:

```cpp
union
{
    POINT m_blupiPosHelico;
    struct { int x; int blupiFront; } m_blupiPosHelicoFront;
};
union
{
    POINT m_blupiPosMagic;
    struct { int bulletCount; int persoCount; } m_blupiInventory;
};
```

Propose the same delta in `src/decor.h`; do not integrate it per TU. Target
`Build` reads `this+0xc954` as the front flag, and target `InitDecor` writes
zero there. Target `DrawInfo` reads `this+0xc958` and `this+0xc95c` as counts;
target `InitDecor` clears both offsets. The existing `m_blupiPosHelico` and
`m_blupiPosMagic` members are used as `POINT`s by `src/decblupi.cpp` and
`src/decor.cpp`, so the overlay preserves their names and 8-byte layout. The
target pseudocode evidence is in `scratch/decblock/target_4350.c`,
`target_3b80.c`, and `target_5270.c`. `src/decor.h` remains untouched.

## Body reconstruction and compiler findings

### `InitDecor` (`0x3b80`)

**[EVIDENCE]** The candidate clears both 100x100 icon maps, seeds three tiles,
clears 200 move-object type fields, initializes the two default moving
objects and start positions, then sets Blupi, decor, scroll, treasure, energy,
and phase state. It is 752 B versus 744 B; score 457.

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
  window was +4..7, +12..15, and +20..23. The final decblock-object body is
  STRICT, while the duplicate in `decblupi.obj` remains MISMATCH. This confirms
  context sensitivity for this TU, not a transferable count-padding recipe.
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

## Next work

**[OPEN]** Continue comparing Build's early map-loop shape and `0x48` frame
against `target_4350.c`; keep the two-argument signature and the tutorial
index reuse. Improve `InitDecor` store/register shape and `DrawInfo`'s remaining
18-byte size difference only with target or ancestry evidence. Re-run the
capped full verifier after each source edit. Do not promote a mismatch or a
relocation-unverified function as a strict match.
