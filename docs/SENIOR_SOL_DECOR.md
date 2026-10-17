# sol-decor matching pass (2026-10-17)

## Delivered candidates and verification

[EVIDENCE] Candidate files are `scratch/sol-decor/decor.cpp` and
`scratch/sol-decor/blupi.cpp`; focused integration diffs are beside them as
`decor.patch` and `blupi.patch`. Only the assigned target bodies differ from
canonical source. No canonical source, header, table, manifest, toolchain,
asset, or reference repository was edited. No commit was made.

[EVIDENCE] Final checks used the current canonical headers, manifest VC5 SP3
flags, and one TU substitution per invocation:

```
.pyenv/python.exe tools/verify.py --tu src/decor.cpp=scratch/sol-decor/decor.cpp --build-dir sol-decor
.pyenv/python.exe tools/verify.py --tu src/blupi.cpp=scratch/sol-decor/blupi.cpp --build-dir sol-decor
```

Both final commands exited zero and preserved all non-withdrawn accepted
functions. Logs and copied reports are `scratch/sol-decor/final-decor.log`,
`final-decor-results.json`, `final-blupi.log`, and `final-blupi-results.json`.
The final canonical tree had an independently withdrawn MoveObjectSort;
that is not a candidate regression. Other workers integrated changes during
this pass, so global coverage totals across intermediate runs are not a
measurement of this worker's gains.

| Function | RVA | Original instruction bytes | Compared COFF span | Result |
|---|---:|---:|---:|---|
| IsRightBorder | 0x140c0 | 775 | 784 | **New STRICT**, zero relocations |
| Read | 0x14af0 | 610 | 624 | **New STRICT**, 15 proven relocations |
| AdaptMidBorder | 0x143d0 | 253 | 256 | Already STRICT; unchanged |
| DrawMap | 0x14550 | 805 | 816 | MISMATCH; 120 fixed differences, first +0x23e |
| CurrentRead | 0x153d0 | 1,564 | 1,584 | MISMATCH; validation/cleanup layout |
| FileExist | 0x159f0 | 540 | 544 | MISMATCH; validation/cleanup and format-call layout |
| AdaptBorder | 0x144d0 | 114 | 128 | MISMATCH; eight register bytes |
| AdaptDoors | 0x15e30 | 840 | 848 | MISMATCH; spills/address scheduling |
| DoInit | 0x1ef0 | 2,801 | 2,816 | MISMATCH; **three fixed bytes in allocation operand only** |
| ReadConfig | 0x1030 | 673 | 688 | MISMATCH; one SIB byte at +0x89 |

[EVIDENCE] The two new strict bodies cover **1,385 unique original instruction
bytes**. The verifier's 1,408-byte span increment also contains 23 alignment
bytes; these are not counted as original function code here. The decor object
has 16 STRICT functions in the final candidate. No folded or ambiguous body
is counted again.

## Source facts and pending integration

[EVIDENCE] Read skips bigDecor only for version 1.0. For major version 1 and
minor <=1, it clears the entire 200-entry move-object array and reads 100
records; otherwise it reads 200 records without the clear. These guards and
the placement of memset reproduce the complete target body, including its
EBX/EBP allocation. The previous SB2-style guards were semantically different.

[EVIDENCE] DrawMap's viewport is 522 by 400: the horizontal center subtraction
is 261, its large-world maximum is 5878, and the vertical maximum is 6000.
It passes mouse point (320,240) to Build(RECT,POINT), then channel **8** to
CPixmap::DrawMap. The candidate also follows the target's saved-state order,
`i == team` comparison, and remote secondary/icon store order. Code through
+0x23d agrees on fixed bytes; the drawing tail remains non-STRICT. These
source corrections are retained for review, not claimed as a matched body.

[EVIDENCE] Reusing `POINT totalDim, iconDim` across DoInit's Cache calls,
with ordinary component assignments, removes the preceding register and
scheduling differences. The remaining allocation is at DoInit+0x9a2:
candidate `push sizeof(CDecor)` = 0xdbd8, oracle = 0x258d4. Its three differing
immediate bytes are real layout evidence, not normalization candidates.
No artificial class tail or allocator-size override was added.

[OPEN] The missing CDecor tail is still unidentified. DoInit's code bytes
outside this operand agree after relocation fields are excluded, but this is
not STRICT and is not a claim that all DoInit relocations are proven.

[EVIDENCE] Header deltas: **none**. Table deltas: **none**; the existing
80-entry table and accepted AdaptMidBorder were preserved. CHMAP remains
unchanged in the shared header; the evidenced SB1 channel 8 is local to
the pending DrawMap body.

[EVIDENCE] ObjectStart is in `src/decnet.cpp`, not either assigned source
file. It was not edited because this worker owns only decor.cpp/blupi.cpp
scratch copies. Existing investigation is in `docs/BLUPI_TU.md`.

## Compiler findings

All measurements below use VC5 SP3 with manifest `/W3 /GX /O2 /ML /Gd /Gy`,
the same canonical header set for each controlled comparison, no PCH, and
isolated `build/verify-sol-decor`. Hints/rules were consulted using kb.py.

- [EVIDENCE; blupi-specific, VC5-CTL-01 / BLUPI-REG-01] IsRightBorder's
  separate bounds returns emitted duplicated return blocks; combining the
  four bounds conditions gives the target shared tail. Expressing each
  directional diagonal group as `if (dx == direction || dy == 1) return
  FALSE; return TRUE;` closes the remaining branch/register differences.
  The combined measured result is STRICT (775 instruction bytes). Flattening
  those groups into fallthrough tests instead shortened the body to 681
  instruction bytes and did not match; it is not retained. No universal
  prediction about register names is inferred.
- [EVIDENCE; blupi-specific, VC5-SEM-01] Read's target version conditions
  and legacy-only clear, together in one controlled candidate, make the
  624-byte object span STRICT. The frame was already 0x114; the semantic
  correction also restored target register allocation. This is evidence
  for fixing version semantics before treating the EBX/EBP swap as context.
- [EVIDENCE; blupi-specific, VC5-LOC-01 / BLUPI-REG-02] DoInit originally
  supplied independent MAKE_POINT temporaries to Cache. Reusing two named
  points across all Cache calls reduced 209 fixed-byte differences to three
  at the class-size operand, with the same 2,816-byte span and 0x38 local
  allocation. A simultaneous named RECT passed through SetRect expanded the
  local allocation to 0x48 and regressed; reverting only that RECT change
  produced the three-byte checkpoint. Related local-identity rule is
  supported here, but the whole function remains unverified.
- [EVIDENCE; negative, VC5-CTL-01] Three CurrentRead validation/cleanup
  shapes (nested opening guard plus success label, combined bad-input guard
  with inline error label, and fully nested success) all remained non-STRICT.
  The first two retained the 1,584-byte span and late error block; the fully
  nested form also removed the initial EBX zeroing. Reverted. Subsequent
  reading of DECOR_TU.md showed this overlaps prior negative investigations;
  no further cleanup variants were pursued.
- [EVIDENCE; negative, BLUPI-REG-01 / VC5-LOC-01] AdaptBorder's four inline
  POINT temporaries versus one component-updated POINT local are byte-inert
  for the eight register residues. AdaptDoors aggregate posStart/posEnd and
  posCurrent copies change scheduling and shorten the section to 832, but
  remain non-STRICT. Both probes are reverted.
- [EVIDENCE; negative, VC5-LOC-01 / BLUPI-LOC-01] DrawMap explicit mouse
  point, explicit Build rectangle, reused source rectangle, separate source
  and destination initializers, nested rectangle scope, and function-root
  declarations do not close the drawing tail. Named source/destination
  rectangles expand the frame from 0x28 to 0x38; the 107-difference inline
  rectangle checkpoint was tested; the final retained inline-point/rectangle
  form has 120 fixed differences in the final canonical context. Constant/call/store-order corrections
  preceding it are target facts, not general compiler rules.
- [EVIDENCE; negative, BLUPI-REG-02] Four ReadConfig local-identity probes:
  separate pointer-difference local, reuse nb as the loop index, separate
  loop index, and separate pointer difference plus nb for TrueColor parsing.
  All completed verification without accepted regressions, all remain
  MISMATCH with 688-byte spans. Baseline is retained; no new context-count
  campaign or address-spelling sweep was repeated. Early campaign retries
  that failed Windows cleanup due to a still-open results.json handle were
  discarded and rerun with Path.read_text; they are not compiler evidence.

## Remaining work

[OPEN] Integrate the two strict bodies and review the non-STRICT DrawMap and
DoInit improvements independently using the focused patches. The highest
value next step is static identification of the missing CDecor tail members;
that is now the only fixed-code blocker for this DoInit candidate. CurrentRead
and FileExist need new control-flow/source evidence before more equivalent
cleanup rewrites. AdaptBorder needs authentic TU/source context evidence;
prior count probes are documented in SWEEP_RESWEEP.md. No background process
was started or left running.
