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
[EVIDENCE, 2026-10-24] Resolved mechanism (msvc5kb BLUPI-LAY-04, scratch/w-emitorder): C2 emits
file-static functions only once a compiled reference needs them. Dropping `static` from the
network.h prototypes and the definitions restores the original callback order in SB1 too, with
identical payloads, but SB1's historical relink then keeps a different GetHide/GetNbSessions ICF
survivor and loses functions at their original RVA (131 -> 48), so SB1 is unchanged. SB2 adopts it.
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

## Round 2: bounded context follow-up (2026-10-17)

[EVIDENCE] Fresh base: `6ad87f16ec0e0c93307fd938e7dc3a13cbc679eb`.
All `src/` inputs were copied anew to `scratch/sol-layout/r2/src/` and
`r2/candidate/`. The final candidates are again byte-identical to that fresh
snapshot. Canonical sources, headers, manifest, assets, reference repositories
and compiler catalog are unchanged. The source-only delivery
`scratch/sol-layout/r2/delivery/layout-context.patch` is **empty**, checked with
`git apply --check --allow-empty`. This round changes only this report.

### Gate and limits

[EVIDENCE] There were **25 serial verify runs**: initial canonical baseline,
parked/action control, 22 controlled variations and final canonical gate.
Every run used `--build-dir sollayout`, pinned SP3, manifest
`/W3 /GX /O2 /ML /Gd /Gy`, canonical defines, no PCH, default job/slot limits
and repo-local TMP/TEMP through verify.py/msvc.py. No direct compiler call,
filler declaration, padding, assembly or background campaign was used.
The available source-backed context leads reached diminishing returns before
the approximate 30-run ceiling; further arbitrary permutations were not run.

[EVIDENCE] The final gate exits **0** with all **396 active accepted entries
(393 STRICT, three FOLDED)** preserved. Sound has the expected INCOMPLETE
historical-path note and passes accepted code-byte checks. New STRICT gains:
**none**. Warnings remain **9**: network 4, event 4, decblock 1.
There are no proposed header deltas.

[EVIDENCE] Each experimental event TU uses HEAD's complete source plus only
the exact parked ChangePhase definition from
`scratch/sol-event3/delivery/event.cpp`; each experimental decblock TU adds
the exact recovered DecorNextAction definition/table from
`scratch/sol-build/action-reference.cpp`. A per-run audit proves every
function definition unchanged from that control, including all parked and
accepted peer bodies. These diagnostic additions are not delivered.

| Parked function | RVA / instruction bytes | Result across all 22 variations |
|---|---|---|
| ChangePhase | `0x19310` / target 6,919 | Candidate 6,922; section 6,928; 4,664 fixed differences, first `+0x84`; complete raw payload identical to control |
| DrawButtons | `0x16f70` / 7,419 | Section 7,424; 65 fixed differences, first `+0x19c7`; complete raw payload identical |
| TreatEventBase | `0x1d230` / 6,616 | Section 7,072 including switch data; eight fixed differences, first `+0x41b`; complete raw payload identical; additional relocation discrepancy below |
| DecorNextAction | `0x5670` / 269 | STRICT in every diagnostic run; 272-byte section and all six relocations verified; complete raw payload identical |
| GetBlupiInfo with DecorNextAction present | `0x57b0` / 67 | Four fixed differences in control; two in seven variations below, never zero; 80-byte section; always MISMATCH |

[EVIDENCE] Every diagnostic run exits 1 solely for accepted GetBlupiInfo.
No other accepted peer regresses. Removing the diagnostic action addition
restores GetBlupiInfo STRICT in the final unchanged canonical candidate.
The large shifted-byte ChangePhase count and relocation-excluded difference
counts are diagnostics, not partial strict coverage.

### Tested arrangements and header deltas

[EVIDENCE] Ancestry inputs were read-only SB2 `event.h`, `decor.h`,
`network.h/.cpp`, `decor.cpp`, `decblock.cpp`, and `dectables.h`, plus PB
v1.8.0 `event.h/.cpp` and `decor.h/.cpp` fetched from the local git object
store. Their source/header arrangements are hypotheses for SB1, not proof
that SB1 used the same declarations. Method reorders preserve each complete
declaration, access section, member-field layout and declaration set.

| Runs | Ordinary context variation | Result versus parked/action control |
|---|---|---|
| `02`, `03` | In private event.h, order common method declarations by PB ancestry, or by recovered SB1 definition order within each access section | All five target payloads identical |
| `04`, `05`, `06` | In private decor.h, use PB common-method order, recovered source/TU order, or move GetBlupiInfo's existing declaration beside its SB1 decblock peers before PlaySound | All five target payloads identical |
| `07`, `08` | Restore SB2 event.cpp's network.h-last include order; then also restore PB's explicit ddraw/pixmap/sound/button/menu/jauge includes | All five target payloads identical |
| `09`, `10` | Restore SB2 decor core include order, retaining required SB1 headers; compare PB's explicit core includes | `09`: GetBlupiInfo 4 -> 2, first `+0x2e`; `10`: unchanged four. Other target payloads identical |
| `11`, `19` | Move the action table definition to the TU tail with a nearby extern declaration, or to the existing dectables.cpp data TU using the already present ancestral dectables.h declaration | All five target payloads identical; full initialized table and action relocations remain STRICT |
| `12` | Move the two static CALLBACK prototypes from private network.h to network.cpp immediately before their existing definitions, preserving signatures/linkage | GetBlupiInfo 4 -> 2, first `+0x2e`; other targets identical; complete network function sequence/payloads identical |
| `13`, `21` | Combine private callback prototypes with SB2 decor include order, or with just its event.h include | GetBlupiInfo still two differences, now first `+0xe`; no strict repair |
| `14`, `15` | Add the actual SB2 decor event.h include after misc.h, or after dectables.h | GetBlupiInfo 4 -> 2, first `+0x2e`; isolates the include responsible for `09` |
| `16`, `18`, `23` | Move unchanged DescInfo typedef near its first serializer use; group it with file-format types before CEvent in private event.h; combine header placement with the SB2 decor event.h include | `16`/`18`: unchanged four; `23`: two, first `+0x2e`. All parked event payloads identical |
| `17` | Use the oracle-proven 22-row cheat table extent and omit the three unsupported SB2 tail literals, without changing any function body | Event payloads identical; the 25-iteration loop remains wrong; diagnostic table change reverted |
| `20` | Move the complete unchanged phase-table definition to immediately before GetButtonIndex, its first user | All five target payloads identical |
| `22` | Move existing GetCel/GetVector/SortGetType free prototypes before CDecor, leaving declarations and inline bodies unchanged | All five target payloads identical |

[EVIDENCE] Tested header changes are limited to the event.h/decor.h method
reorders, GetBlupiInfo prototype position, network.h prototype removal with
the same prototypes moved into network.cpp, unchanged DescInfo placement in
event.h, and decor.h free-prototype position. No member field, constant,
signature, access level or ABI is changed. All private headers are restored
byte-for-byte to fresh src/ in the delivery. No trial header is proposed for
shared integration; passing unrelated canonical TUs is not claimed as a
whole-tree test of those private trial headers.

### Remaining warnings: no supported order correction

[EVIDENCE] The two static callback definitions already precede their callers
in the source, as in SB2 ancestry; both original CALLBACK signatures are
target-proven. Round 1's before/after definition move and this round's private
prototype placement both leave the ordered network function payloads
identical, with the same four warnings. Moving the source further cannot be
justified from these warnings. Original linker/COMDAT packaging or linkage
details remain OPEN; changing static linkage without evidence is not a fix.

[EVIDENCE] Event's four overlaps still identify duplicate source placeholders:
DrawMap shares NetDraw's `0x16e80` body, and three Play* names share BuildUp's
`0x1b030` body. The extra names remain promotion-excluded. They do not prove
four distinct original definitions at those addresses; sorting them again
cannot remove overlap. Their original identities remain OPEN. No artificial
alias or folding claim is introduced to silence the warnings.

[EVIDENCE] Decblock's one gap is still the absent `0x5670` action body.
Its correctly positioned diagnostic definition is STRICT but blocks the peer
gate. This is incomplete reconstruction/context, not an out-of-order body.

### New evidence: the eight-byte TreatEventBase score omits a real residue

[EVIDENCE] The oracle loop at `0x1d230 + 0x3b2` starts its cheat-table pointer
at VA `0x434480`, advances by 15 bytes per iteration, and compares at `+0x3bf`
against VA `0x4345ca`. The difference is **330 bytes / 22 rows**. The source's
first 22 rows match all 330 oracle bytes exactly. Six zero alignment bytes
follow; the fully matched phase table starts at RVA `0x345d0`.

[EVIDENCE] The candidate loop's relocation at `+0x3c0` has addend **375**
(`25 * 15`), while the oracle requires **330**. The source explicitly loops
to 25 and retains three SB2-only tail rows: IOVERCRAFT, UDYNAMITE, WELLKEYS.
Thus even correcting the eight fixed register bytes would not make this
function STRICT. Merely truncating the initializer to 22 rows does not change
the compiled loop, its addend or any parked event payload. That diagnostic
change is reverted, and the loop body is deliberately untouched under this
round's instruction. No raw original bytes were copied into source.

### Compiler findings

- [EVIDENCE; VC5-CTX-01..04 / BLUPI-CTX-01/02; negative]
  Trigger: order correction previously reduced DrawButtons 78 -> 65.
  Variation: ancestor/source-ordered existing member declarations, original
  explicit includes, real typedef/table/prototype placement and removal of
  three unsupported initializer literals. All 22 controls leave every parked
  event raw function payload identical. The two alternate data-TU/table
  arrangements also leave both decblock target payloads identical. This
  rules out the tested ordinary arrangements, not all possible TU context.
- [EVIDENCE; VC5-CTX-01..04 / BLUPI-CTX-01; Blupi-specific]
  Trigger: authentic DecorNextAction addition swaps two GetBlupiInfo OR-load
  pairs. Adding SB2's genuine event.h include, or making its static network
  callback prototypes private to their defining TU, changes four fixed bytes
  to two at unchanged 67 instruction bytes. Combined arrangements change
  which pair is wrong but never yield STRICT. No extra placeholder declaration
  or arbitrary count sweep was used. All other accepted peers pass; none of
  these incomplete gains is retained.
- [EVIDENCE; VC5-SEM-01; confirmed-transferable]
  Trigger: relocation-excluded scoring concealed the cheat-loop bound.
  Candidate `i < 25` produces a relocation addend 375; the oracle uses 330
  and has exactly the matching 22-row data before the next array. The
  table-only 22-row control leaves code unchanged, proving that moving or
  trimming literals does not fix this source-level loop-bound residue.
  TreatEventBase remains MISMATCH; body correction is outside this round.
- [EVIDENCE; callback packaging case from Round 1; negative]
  Moving the two existing static CALLBACK prototypes into their defining
  source changes decblock context but neither network code bytes nor function
  emission order. The four layout warnings persist. No new universal ordering
  rule or original-linkage claim follows from this result.

[EVIDENCE] `r2/runs/<label>/` retains exact candidates, all private headers,
command/exit records, complete verifier logs, objects, results and target
metrics. `r2/delivery/audit.json` indexes all 25 runs, their trial header
deltas and sole accepted regression, verifies every parked definition and
raw event payload, checks the cheat-table/loop evidence, and audits the final
396-entry gate. `r2/delivery/verify.log` and `results.json` preserve the final
canonical build. No catalog edit, commit or background process remains.

[OPEN] Next useful work is the explicit 22-row/22-iteration cheat correction
in a round permitting body changes, followed by the two known scalar-register
regions; and independent source/header evidence for the decblock context
conflict. Further already-inert member-order or literal-order permutations
are not supported by this sweep.
