# sol-decor-3: remaining decor functions

## Prior evidence and disproven hypotheses

[EVIDENCE] Starting revision: `999fdebf58d54fc466596f6c72fa1ac25e38507b`.
Fresh byte-preserving copies of `src/` are under `scratch/sol-decor3/`.
Ownership is limited to that directory, this document, and the required
`build/verify-soldecor3/` verifier output. Canonical source is untouched.

[EVIDENCE] Read before testing: `SENIOR_SOL_DECOR.md`, both rounds of
`SENIOR_SOL_DECBLOCK.md`, `LAYOUT_DEBT.md`, `DECOR_TU.md`, all four
`SWEEP_*.md` target references, `WORKFLOW.md`, `MATCHING.md`, and the KB README.
The following tested families do not solve these targets and will not be
repeated:

- AdaptBorder: inline versus one reused POINT; four independent neighbor
  POINTs; walking/restoring the incoming coordinate; top counts 7/14 and
  before-function counts 7/14/23/30; changing AdaptMidBorder's POINT ABI.
- DrawMap: named mutable rectangles, independent source/destination
  initializers, reused rectangle, nested/root declaration scopes, explicit
  mouse point, and const aggregate rectangles/point. Target viewport, map
  channel, Build signature, saved-state order, and remote stores are already
  corrected in canonical source.
- CurrentRead/FileExist: error labels, combined validity tests after separate
  opening/allocation, nested success guards, immediate cleanup returns,
  bOk single-pass validation, success-break/error-label validation, and
  initializing a persistent fread count to zero before guarded reads. These
  do not produce the early shared cleanup. Explicit dword-copy loops also
  fail. FileExist's missing mission-99 `open` argument is already corrected.

[OPEN] CDecor allocation-tail debt is outside this assignment. AdaptBorder
has no new authentic context hypothesis after the prior point/count trials.

## Compiler findings

[EVIDENCE] All experiments use pinned VC5 SP3,
manifest `/W3 /GX /O2 /ML /Gd /Gy`, canonical header snapshots, no PCH, and
`tools/verify.py --tu src/decor.cpp=scratch/sol-decor3/decor.cpp --build-dir soldecor3`.

- [EVIDENCE; negative; VC5-CTL-01 / VC5-CTL-03] Trigger: CurrentRead and
  FileExist's oracle error cleanup precedes success, but canonical separate
  assignment/goto guards compile to cold error tails. New candidate:
  one ordinary short-circuit OR expression containing fopen, malloc and
  fread assignments, followed by the saved-length check and explicit cleanup
  return. Prediction: the expression's shared failure region might retain
  the early cleanup. One substituted full-manifest compile (`r01-short-circuit`)
  exits 0, retaining the accepted gate; both target function data payloads are
  byte-identical to baseline and remain MISMATCH. This measured inert result
  extends the existing cleanup negatives; it establishes no new compiler rule.

## Outcome and delivery

[EVIDENCE] No new STRICT function, no new unique original bytes, and no
header delta. AdaptBorder (0x144d0, 114 original bytes), DrawMap (0x14550,
805), CurrentRead (0x153d0, 1564), and FileExist (0x159f0, 540) remain
MISMATCH. None is described as a relocation-unverified byte match.

[EVIDENCE] AdaptBorder and DrawMap were not recompiled in target-specific
experiments: their prior source/context families are exhausted and this pass
found no new evidence-backed hypothesis. The single new reader experiment
did not change their baseline residues (eight bytes for AdaptBorder;
DrawMap first fixed difference +0x23e, 120 differences).

[OPEN] Status: blocked at diminishing returns. Further work needs independent
source/CFG evidence for the shared early failure region, or authentic context
evidence for the border/map register allocation. CDecor tail/DoInit is untouched.

[EVIDENCE] `scratch/sol-decor3/integration.patch` is empty because no source
change qualifies for integration. Its no-op applicability check uses
`git apply --check --allow-empty`. The delivered `decor.cpp` is restored
byte-for-byte to the fresh canonical snapshot; all copied headers are unchanged.
Trial sources, logs, function disassemblies, and comparator reports are retained
in the owned scratch directory. No commit or background process was created.

[EVIDENCE] Final restored-source verification exits **0** and preserves all
**394 active accepted entries**. `src/sound.cpp` has the expected historical
drive-mapping INCOMPLETE note; its accepted code bytes pass the degraded gate.
Final target candidate body sizes (excluding trailing NOPs) are 114, 805,
1580, and 556 bytes respectively. `delivery-audit.json` records the complete
result, identical header/source checks, and empty patch. A separate byte check
confirms that the restored source retains exclusively CRLF line endings.
The baseline and trial contain identical payloads for every decor function.
There was one new-hypothesis compile and two baseline/final-restoration runs.
