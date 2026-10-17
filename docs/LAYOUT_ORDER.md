# Function definition order (sol-layout, 2026-10-17)

[EVIDENCE] Base: `b0c527a2538a5883ed85f127b2a1162fb49f2b1b`.
The source-only delivery is
`scratch/sol-layout/delivery/layout-order.patch`. It passes `git apply --check`
against the unchanged working sources; applying it under
`scratch/sol-layout/patchcheck/` reproduces all six candidate files byte-for-byte.
Canonical `src/`, headers and `recon.json` were not edited.

## Result

[EVIDENCE] Whole definitions, with adjacent comments, are reordered in button,
jauge, network, text, event and decdesign. Located, non-folded definitions
follow increasing oracle RVA; unplaced definitions retain their preceding
source neighbour. Folded bodies outside the owning TU are not used to order
that TU. No signature, body, constant or declaration is added or rewritten.
An audit compares every definition's complete original bytes, each file's
byte multiset and CRLF count, and all private headers against `src/`.
No affected source contains `__LINE__`. Decblock is byte-identical to HEAD:
its existing definitions already have the correct order.

| TU | Baseline warnings | Final warnings |
|---|---:|---:|
| button | 1 | 0 |
| jauge | 2 | 0 |
| network | 5 | 4 |
| text | 3 | 0 |
| event | 36 | 4 |
| decblock | 1 | 1 |
| decdesign | 3 | 0 |
| **Total** | **51** | **9** |

[EVIDENCE] All 396 active accepted entries (393 STRICT, three FOLDED) retain
their required result, RVA and size under the available gate. No accepted
function regresses, and no new STRICT function is gained. The sole changed
raw COFF function payload in the seven audited TUs is unmatched DrawButtons.
Measured comparator coverage is unchanged at 107,232 / 141,488 bytes in this
sandbox, rather than the handoff's 107,440. Sound's expected historical-path
mapping denial leaves its gate INCOMPLETE; accepted code-byte checks pass.

## Remaining warnings

[EVIDENCE] Network's four warnings concern the two static callbacks. Each
callback definition already precedes its caller in canonical source. In the
object, EnumProviders precedes its callback and EnumSessions precedes its
callback. A controlled second candidate moves only those two whole callback
definitions after their callers. The complete ordered sequence of function
names and raw function payloads is identical in the two objects; all accepted
functions pass and the same four warnings remain. The delivery keeps each
definition before its caller, consistent with increasing oracle RVA.
TraceErrorDP is moved to the beginning of the TU, resolving the fifth warning.

[HYPOTHESIS] Static callback section creation/emission is influenced by their
references, rather than simply by definition position. This is a measured
exception to applying the definition-order assumption to every COFF section.
Original callback linkage/packaging needs independent evidence before changing
the shared static declarations; no linkage or header change is proposed here.

[EVIDENCE] Event's four remaining overlaps are NetDraw/DrawMap selecting the
same `0x16e80` body, and BuildUp/PlayDown/PlayMove/PlayUp selecting the same
`0x1b030` body. The extra aliases are already promotion-excluded. Reordering
cannot create distinct oracle bodies or establish their source identities;
they are not extra strict coverage or newly proved folds.

[EVIDENCE] Decblock's remaining gap starts at `0x5670`, between DrawInfo and
KeyChange. It is the missing DecorNextAction, not an order error. The current
25 definitions are unchanged by the reorder algorithm. Inserting the recovered
`scratch/sol-build/action-reference.cpp` verbatim immediately before KeyChange
reproduces DecorNextAction STRICT at `0x5670`: 269 instruction bytes,
272-byte section, six verified relocations. GetBlupiInfo at `0x57b0` changes
from STRICT (67 instruction bytes, 80-byte section) to MISMATCH with four
fixed operand-load differences, first at `+0xe`. This rejected diagnostic
addition is omitted from the delivery. Correct order offers no new decblock
variable: the existing canonical and reordered decblock files are identical.

## Parked event measurements

[EVIDENCE] Canonical ChangePhase is an older 6,832-byte COFF section, not the
parked round-3 body. Two separate event-only builds transplant only the exact
ChangePhase definition from `scratch/sol-event3/delivery/event.cpp` into the
canonical and reordered event TUs. No parked function body is edited.

| Function | Original RVA / instruction bytes | Before order correction | After order correction |
|---|---|---|---|
| ChangePhase, parked | `0x19310` / 6,919 | 6,922 instruction bytes; 6,928-byte section; 4,664 fixed differences; first `+0x84` | Identical complete COFF payload and metrics |
| DrawButtons | `0x16f70` / 7,419 | 7,419 instruction bytes; 7,424-byte section; 78 fixed differences; first `+0x19c7` | Same sizes; 65 fixed differences; same first residue |
| TreatEventBase | `0x1d230` / 6,616 | 6,616 instruction bytes; 7,072-byte section including switch data; eight fixed differences; first `+0x41b` | Identical complete COFF payload and metrics |

[EVIDENCE] DrawButtons has the same 78-to-65 change in the canonical-body
reorder and the parked-ChangePhase pair. All three remain MISMATCH. The large
ChangePhase fixed-address difference count includes shifted instruction
regions; it is diagnostic, not a partial match or similarity certificate.
The delivery retains HEAD's ChangePhase body, not the unmatched parked WIP.

## Compiler findings

All trials use VC5 SP3 11.00.7022, manifest `/W3 /GX /O2 /ML /Gd /Gy`,
canonical headers/defines and no PCH, through verify.py/msvc.py with repo-local
TMP/TEMP. Shell commands and compile campaigns run serially at default limits.
Hints for ChangePhase, DrawButtons, GetBlupiInfo and EnumProvidersCallback,
plus VC5-CTX-01 and BLUPI-CTX-01, were consulted. No catalog was edited.

- [EVIDENCE; VC5-CTX-01..04 / BLUPI-CTX-01/02; Blupi-specific]
  Trigger: wrong TU definition order, with parked register/pointer residues.
  Variation: reorder complete definitions only. DrawButtons stays 7,419
  instruction bytes and improves from 78 to 65 fixed differences, without
  becoming STRICT. Every other raw function section in the seven audited
  TUs is identical; all accepted peers pass. This confirms sensitivity to
  genuine preceding source context, not an authentic original declaration
  count or a general allocation formula. Keeping unplaced methods beside
  their preceding source neighbour gives the same DrawButtons result as
  leaving them in their original occupied slots.
- [EVIDENCE; VC5-CTX-01..04 / BLUPI-CTX-01/02; negative]
  Trigger: parked ChangePhase's +3-byte size residue and TreatEventBase's
  eight register bytes. The same verbatim order correction is completely
  byte-inert for both, in separately compiled original/reordered parked TUs.
  Definition order alone does not close these residues.
- [EVIDENCE; VC5-CTX-01..04 / BLUPI-CTX-01/02; negative]
  Trigger: DecorNextAction integration regresses GetBlupiInfo. Canonical
  decblock is already ordered; restoring the authentic missing body/table
  at its correct position still gives STRICT DecorNextAction and the same
  four-byte GetBlupiInfo regression. Addition is rejected; no fillers,
  header changes or expression rewrites are retained.
- [EVIDENCE; TU-order assumption; Blupi-specific]
  Trigger: callback layout warnings despite correct source definition order.
  Swapping only the static callback definitions across their callers leaves
  their COFF function order and every network function payload identical.
  No existing KB rule specific to this packaging case was found; original
  linkage remains OPEN. No claim of a new universal compiler rule follows.

## Reproduction and artifacts

[EVIDENCE] Final command (exit 0; expected sound INCOMPLETE note):

```powershell
.pyenv\python.exe tools\verify.py `
  --tu src/button.cpp=scratch/sol-layout/button.cpp `
  --tu src/jauge.cpp=scratch/sol-layout/jauge.cpp `
  --tu src/network.cpp=scratch/sol-layout/network.cpp `
  --tu src/text.cpp=scratch/sol-layout/text.cpp `
  --tu src/event.cpp=scratch/sol-layout/event.cpp `
  --tu src/decdesign.cpp=scratch/sol-layout/decdesign.cpp `
  --build-dir sollayout
```

[EVIDENCE] `scratch/sol-layout/` retains baseline, grouped, callback-after-controlled,
parked-before, parked-after, action-original-order and final snapshots. Each
contains objects/results/metrics; matching top-level `.log` files retain the
verifier transcript. `delivery/audit.json` records the body/header audits,
acceptance totals, warnings, changed raw sections and exact scratch patch
application. Original trailing whitespace is deliberately retained in moved
functions, so git apply can warn about inherited whitespace. Nothing from the
game/ISO was executed; no commit or background process is left.

[OPEN] Next useful work is source evidence for the three event residues and
the decblock context conflict, plus callback linkage/section-order evidence.
The nine remaining warnings cannot be removed by the tested whole-definition
order correction alone.
