# sol-decblupi: complete oracle block and restore IsDeepWater

## Result and integration

[EVIDENCE] Final candidate: `scratch/sol-decblupi/decblupi.cpp`, SHA-256
`5670680872b9e05dfc8bb081e44d905a3635d8c336f09291cb2ffeda873b1463`.
It is a copy of the current canonical TU with changes limited to the four
unmatched target bodies and insertion of the missing `MoveObjectPollution`.
The target bodies are ordered as in `oracle/tu_map.csv`; `IsWorld` follows
`MoveObjectBlup`, and `SearchDistRight` follows `IsBlocIcon`.

[EVIDENCE] Final command (exit **0**):

```powershell
.pyenv\python.exe tools\verify.py --tu src/decblupi.cpp=scratch/sol-decblupi/decblupi.cpp --build-dir sol-decblupi
```

[EVIDENCE] All **20** oracle rows assigned to decblupi match STRICT, at unique
locations, with every relocation verified: **29,540 unique original body
bytes**. The console's 29,680-byte TU total includes COFF alignment extents;
it is not the body-byte count. The active accepted-function regression gate
passes. The withdrawn `IsDeepWater` is STRICT, but its manifest withdrawal
has not been changed by this worker.

| New or restored STRICT body | RVA | Original body bytes | COFF extent | Relocations |
| --- | --- | ---: | ---: | ---: |
| MoveObjectPollution | `0xfc10` | 439 | 448 | 2 |
| IsWorld | `0xff90` | 184 | 192 | 0 |
| GetTypeBarre | `0x10050` | 248 | 256 | 2 |
| IsDeepWater (restored) | `0x10290` | 71 | 80 | 0 |
| SearchDistRight | `0x103b0` | 200 | 208 | 1 |
| IsVentillo | `0x10480` | 486 | 496 | 2 |

[EVIDENCE] Five new bodies contribute **1,557** bytes; restoring IsDeepWater
adds **71**, for **1,628** bytes beyond this worker's baseline. The existing
14 strict bodies remain strict, including the 25,269-byte `BlupiStep`.
No unmatched or relocation-unverified body remains in the assigned oracle block.

[EVIDENCE] No header, constant declaration, member offset, table, canonical
source, manifest, or compiler catalog was edited. The pollution method retains
the existing `void` declaration. No new data payload is needed: the existing
canonical `table_decor_quart` definition already proves its relocations.

[EVIDENCE] Evidence artifacts, all in worker-owned directories:

- `scratch/sol-decblupi/order.log`: final verification after oracle-order restoration.
- `scratch/sol-decblupi/final-results.json`: complete final comparator results.
- `scratch/sol-decblupi/audit.json`: all 20 RVAs, original sizes, relocation counts,
  and the candidate hash; `audit.py` checks the TU-map coverage and creates this file.
- `scratch/sol-decblupi/candidate.patch`: focused canonical-source patch.
- `build/verify-sol-decblupi/`: final compiled objects and comparison report.

[OPEN; supervisor integration] Apply the candidate patch and reactivate
IsDeepWater through the normal clean verification/acceptance workflow. The
canonical `decmove.cpp` still contains the inherited, different
`MoveObjectPollution` definition; the oracle owner is decblupi at `0xfc10`.
Its removal belongs to the owning worker/supervisor, not this worker. The
pre-existing unlocated helpers and inline bodies still emitted by the candidate
object were left alone; they are not extra original functions or extra coverage.

## Target reconstruction

[EVIDENCE] `build/decomp/tu/decblupi.c` supplies the target control flow,
constants, member offsets, and calls. Static disassembly confirms the actual
instruction schedules. No executable from the original media was run.

[EVIDENCE] Pollution tests action 3, helicopter/jeep/tank state, the target
timer/phase residues, and offsets 22/32/35. It captures `m_blupiPos` at entry.
The direction branches adjust x, assign speed -20 or 20, and reach one source
`ObjectStart(pos, TYPE_POLLUTION, speed, TRUE)` expression. The compiler
duplicates that call into the two terminal paths, exactly as in the target.
There is no SB2-only over-state or accelerated-jeep path.

[EVIDENCE] `IsWorld` has only the three ranges 158–165, 166–173, and 174–181,
each returning 1–8, plus 184 returning 99. Other values return -1. The SB2
World_9/World_10..14/Level and 199 return paths are absent from the target.

[EVIDENCE] `GetTypeBarre` retains the ancestor's `POINT pos2 = pos`, adjusted
parameter, and early guards. The target returns 2 when the cell below is
**not** passable: the inherited `IsPassIcon` condition was reversed. Its
collision rectangle uses original-position offsets (12,48,58,59), without
calling `BlupiRect`. `int` return and the canonical header produce STRICT;
no pseudocode-derived `char` signature is needed.

[EVIDENCE] `SearchDistRight` lacks both SB2's special-type return of 500 and
its `SetBalleTraj` call. Updating `cel.x` and `cel.y` separately reproduces
the target's induction variables without the inherited out-of-line POINT
operator call.

[EVIDENCE] `IsVentillo` uses an uninitialized icon-number local, assigned in
each of the four activating direction branches. A `while (TRUE)` loop with
explicit mismatch/bounds breaks reproduces the single target iteration region.
The frame is 12 bytes; the false flag occupies entry-ESP minus 12, and the
POINT delta occupies the remaining local slots/spills.

## Compiler findings

All experiments used VC5 SP3 and manifest `/W3 /GX /O2 /ML /Gd /Gy`, canonical
headers, no PCH, no added context declarations, and a full-object-set verifier.
Byte counts below are COFF extents unless explicitly labeled original bytes.

- [EVIDENCE; confirmed-transferable, `BLUPI-CTX-01`, `VC5-CTX-01..04`]
  Baseline IsDeepWater computes x before y and mismatches at 80 bytes after
  removal of the non-original selector duplicate. Inserting the actual missing
  pollution definition before it restores the target y-before-x schedule, with
  its own body unchanged. This occurred on the first pollution candidate and
  survived all following target fixes and oracle-order restoration. Final:
  STRICT, 71 original bytes. This is natural missing-body context, with no
  count probes or filler source.

- [EVIDENCE; Blupi-specific, `VC5-SEM-01`]
  Pollution source-form family, four compiles: a late local POINT and direct
  literal-speed calls emit 432 mismatching bytes. Moving the POINT capture to
  entry still emits 432 but restores every instruction through target offset
  `+0x179`, apart from exit displacements. Initializing a long-lived speed to
  20 and negating it in the right branch emits 448 mismatching bytes with an
  extra saved register. Declaring speed without an initializer and assigning
  -20/20 in the branches emits 448 bytes with the target schedule, STRICT for
  all 439 original bytes and both relocations. The prior int-return hypothesis
  in ENGINE_PROBE is unnecessary: division quotients and call results left in
  EAX are incidental in this matching void body. The other source forms are
  negative as complete reconstructions, not general compiler rules.

- [EVIDENCE; confirmed-transferable source correction, `VC5-SEM-01`]
  One oracle-derived IsWorld candidate removes the SB2-only icon cases and
  return values: 272 -> 192 bytes, STRICT for 184 original bytes. Earlier
  ENGINE_PROBE reports of a 192-byte mismatch used different TU context;
  this run does not isolate which earlier context discrepancy caused that result.

- [EVIDENCE; Blupi-specific, `VC5-SEM-01`, `BLUPI-REG-01`]
  One combined GetTypeBarre correction changes the nested scalar-coordinate
  form to ancestor-style POINT capture/parameter updates/early guards and
  reverses the below-cell passability check. Extent remains 256; the target
  register allocation, branch layout, call list, and all 248 original bytes
  become STRICT. The individual changes were not isolated; no return-width
  or register-allocation rule is claimed.

- [EVIDENCE; confirmed-transferable source correction, `VC5-SEM-01`]
  SearchDistRight, two compiles: removing the SB2-only return/call regions
  changes 272 -> 176 bytes, still MISMATCH because `cel += dir` calls the
  out-of-line POINT operator. Explicit `cel.x += dir.x; cel.y += dir.y;`
  produces target induction variables and 208 bytes, STRICT for 200 original
  bytes and the IsBlocIcon relocation. This is an overloaded-operator call
  difference, not an array-index spelling experiment.

- [EVIDENCE; Blupi-specific, `BLUPI-LOC-01`, `BLUPI-REG-01`]
  IsVentillo baseline: initialized `num` plus do/while emits 544 bytes, an
  8-byte frame, and a duplicated first-iteration region. Removing the dead
  initializer and using while(TRUE)/explicit bounds break emits 496 bytes,
  a 12-byte frame, and STRICT for 486 original bytes with both relocations.
  Two isolated negative controls: restoring only the initializer retains the
  496-byte extent but changes the frame back to 8 and mismatches; restoring
  only do/while emits 544 and mismatches. Both source differences are required
  in this TU context. Scalar spill placement is measured here; this does not
  extend the array-specific BLUPI-LOC-01 rule into a universal scalar rule.

- [EVIDENCE; negative/inert, `BLUPI-CTX-01`]
  Reordering the completed IsWorld and SearchDistRight definitions into exact
  oracle order preserves all 20 strict bodies and the zero-regression gate.
  It is not necessary to retain their inherited misplaced definition order
  to preserve IsDeepWater or any accepted body.
