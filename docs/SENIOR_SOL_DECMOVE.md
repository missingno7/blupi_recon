# Senior decmove structural pass (sol-decmove, 2026-10-17)

[EVIDENCE] **Latest result: Sort is restored after duplicate Pollution removal;
all 39 oracle-mapped decmove functions are STRICT, and the proposed common header
passes the full accepted-function gate.** The repair moves the actual global
SortGetType declaration out of CDecor; no diagnostic declaration is retained.
See [the context repair](#context-repair-after-duplicate-pollution-removal).
The previous two passes below are historical and have been integrated.

## Context repair after duplicate Pollution removal

[EVIDENCE] Review patch: `scratch/sol-decmove/sol-decmove-context.patch`.
The header-only repair is also available as
`scratch/sol-decmove/sort-declaration.patch`. Canonical source and recon.json
were not changed. The final candidate is `scratch/sol-decmove/decmove.cpp`
with its adjacent private `decor.h`; `context-proof.json` records hashes and
the exact change audit. Every surviving function body is byte-for-byte source
identical to canonical source. Six duplicate definitions are removed, listed
below. No count probe, padding, unused variable or extra body is retained.
Both patch files pass `git apply --check` against the current canonical tree;
their hunk contents preserve the source files' CRLF line endings.

[EVIDENCE] The baseline after the supervisor removed MoveObjectPollution has
a 3ch Sort frame, correct ESI flag and EDI key registers, but evaluates the
next object's key before the current object's key. Target does current first.
The residue is 240-byte compared extent / score 17 / fixed prefix 130 bytes /
44 identical instructions. Moving the helper declaration to file scope gives
score 0 / fixed prefix 234 / 89 identical instructions. The body is **234 B**;
the six trailing NOPs receive no additional byte credit. Both call relocations
are STRICT and bind to SortGetType at `0x13af0`.

[EVIDENCE] Header delta: remove `int SortGetType(int type);` from CDecor's
member list and declare that same function in the existing file-scope helper
declaration block beside GetCel/GetVector. This declares the helper actually
defined by canonical decmove.cpp, instead of a nonexistent nonstatic member
definition. The target uses cdecl caller cleanup and a bare-ret callee;
canonical's global helper already matches those bytes. No constants, members,
record offsets, sizes, inline implementations or other declarations change.

[OPEN] Machine code does not uniquely recover an original prototype's spelling
or physical location, nor distinguish all possible source-level cdecl helpers.
The proposed declaration is an ordinary declaration of the real used function,
consistent with its existing canonical definition; it is not evidence that the
1998 header had this exact line at this exact location.

[EVIDENCE] Full common-header check: `common_header_gate.py` copies the flat
canonical sources/headers into the worker directory, changes only decor.h and
the six duplicate bodies, and substitutes every manifest TU in one verifier
run. `package_context.py` verifies all other copied files are byte-identical.
`common-header/verify.log`, `results.json` and archived `objects/` preserve the
result: **exit 0; every active accepted function preserved; decmove 39 STRICT**.
Sort remains formally withdrawn in canonical recon.json until the supervisor
integrates/promotes it. Full-run Icon is STRICT; its isolated asmscore result
still lacks the other data objects and is not the acceptance evidence.

```powershell
.pyenv\python.exe scratch/sol-decmove/common_header_gate.py
.pyenv\python.exe scratch/sol-decmove/package_context.py
```

### Context measurements and rejected candidates

[EVIDENCE] All runs use pinned VC5 SP3 `/W3 /GX /O2 /ML /Gd /Gy`, canonical
headers except explicitly named experiments, and private build-dir sol-decmove.
Trials and compiles were sequential. `context-ledger.jsonl` and trial directories
retain source/object/results, status, extent, score and instruction prefix.

| Probe on post-Pollution baseline | Sort result |
|---|---|
| Before includes: 0, 1, 4, 8 names | MISMATCH / 17 |
| Before includes: 9-12, 16-20, 24-26, 32-33, 40-41, 64, 128, 136 sampled | STRICT / 0 |
| Before includes: 137, 138, 140, 144, 152, 160, 192, 224, 256 | MISMATCH / 17 |
| Immediately before Sort: 1, 4, 8, 12, 16, 32, 128 | MISMATCH / 17 |
| End: 1, 8, 12, 16, 32, 128 | MISMATCH / 17 |
| Before decor/misc/dectables include, or NetDataFlush/TreatNetData/MoveObjectStep: 16 | STRICT / 0 |

[EVIDENCE] Measured adjacent boundaries are 8/9 and 136/137; the interior
was sampled, not exhaustively checked. Some larger failing top probes also
regress MockeryDetect. There is no claim of an observed internal counter or
proven periodicity. The later placement probes supersede the initial impression
that restoration required placement before includes.

[EVIDENCE] Removing only the stale member declaration is inert (`c02`). An
early declaration of the actual global helper restores Sort even while retaining
the stale member (`c08`); deleting the stale member and placing the global
prototype immediately before its definition is inert (`c09`). The final repair
uses the existing header helper-declaration block and removes the false member.
It works both with the six duplicate bodies still present (`c00-global-sort-decl`)
and with them removed (`c01-no-duplicates-global-sort-decl`, common-header gate).
Thus the duplicate cleanup is not compensation required by the declaration fix.

[EVIDENCE] Additional real duplicate definitions removed from the final candidate:
MoveObjectPlouf (`decblupi.cpp`, STRICT `0xfdd0`), MoveObjectTiplouf (`0xfe20`),
MoveObjectBlup (`0xfe80`), SearchDistRight (`0x103b0`), ObjectStart's four-argument
overload (`decnet.cpp`, target block `0x106b0`) and ObjectDelete (`decnet.cpp`,
STRICT `0x108a0`). Canonical owner files independently define those same method
signatures; two definitions could not link. ObjectStart remains an existing
unmatched decnet owner; removing its duplicate claims no new match. Their
removal alone leaves Sort at score 17 and preserves every active accepted entry.

[EVIDENCE] Removing the twelve unlocated SB2 helpers makes Sort STRICT but
regresses MockeryDetect (544 -> 528-byte extent, score 207). Isolated deletions
trace the Sort change to DynamiteStart; its deletion also reverses the two
grid-index division orders in Mockery. Other tested helper groups alone do not
restore Sort. These wider removals are rejected: absent linked bodies alone do
not prove absent original source. Their declarations/trajectory-tail cleanup,
natural Mockery returns, shared icon/all function-scope locals, anonymous or
shared bare-packet records, and natural MoveObject alignment padding do not close
the combined gate. Sort's one-key/two-key locals score 21; reversed comparison
also fails. None of these variants is retained. Private header experiments have
no proposed delta beyond the helper declaration described above.

### Compiler findings (context repair)

- [EVIDENCE; `BLUPI-CTX-01`, `VC5-CTX-01..04`; blupi-specific] Trigger:
  duplicate-body removal reverses evaluation of the two direct call operands
  in Sort, 240 B/17 with identical frame/register roles. Top and late count
  probes differ; early probes have measured adjacent boundaries 8/9, 136/137.
  Original/candidate: stale nonstatic member prototype versus the real used
  global helper declared in the header's file-scope block. Same SP3/full-TU
  flags: 240 B/0, 234 B fixed prefix, both relocations STRICT, all 39 functions
  and the complete accepted set pass with the header shared across all TUs.
  This confirms declaration timing/scope as an SB1 context repair; an internal
  numbering mechanism or universal window remains unverified.
- [EVIDENCE; `BLUPI-CTX-01/02`, `VC5-CTX-04`; negative controls] Removing the
  false member alone, or adding the real global prototype only immediately
  before its definition, leaves 240 B/17. An early global prototype alone
  suffices, so deleting an arbitrary class member is not the explanation.
  Six proven duplicate-body deletions alone are also inert. No filler is a fix.
- [EVIDENCE; `BLUPI-REG-01/02`, `VC5-CTX-07`; negative] Removing unlocated
  DynamiteStart changes Sort to STRICT but commutes Mockery's two signed grid
  divisions and changes allocation/size. Function-scope locals and natural
  return forms do not repair that combined candidate. Under these flags,
  a standalone strict gain cannot validate a TU context; the full gate rejects it.

[EVIDENCE] Next step: supervisor review/integration of the two-file patch and
reaccept Sort. No remaining decmove oracle miss or background process remains.

## Round 1 result and reproduction (historical)

[EVIDENCE] Review candidate: `scratch/sol-decmove/decmove.cpp`, SHA-256
`f10cbfb01c2e70e50353a3d048c6d5775f86d632444b92839c07913beb001d66`.
Only MoveObjectStep, MoveObjectStepLine and MoveObjectStepIcon are changed;
TreatNetData is added at its TU-order position between NetSendBarePacket and
DoNetSmooth. `scratch/sol-decmove/audit.py` removes these bodies and confirms
that the remaining candidate text equals canonical source. There are no
retained context probes, header overlays or header deltas. No canonical source,
manifest, compiler catalog, toolchain, asset or reference repository was changed.

```powershell
.pyenv\python.exe tools\verify.py --tu src/decmove.cpp=scratch/sol-decmove/decmove.cpp --build-dir sol-decmove
```

[EVIDENCE] Final full verifier exits **0**, preserving every active accepted
function. All compiles use tools/verify.py and tools/msvc.py, pinned VC5 SP3,
`/W3 /GX /O2 /ML /Gd /Gy`, canonical headers and repository-local TMP/TEMP.
Commands and trials were sequential. Reports, source/object snapshots, scores,
target/candidate disassembly and every raw ESP operand are retained under
`scratch/sol-decmove/final/`; live results are
`build/verify-sol-decmove/results.json`. No background process remains.

| Function | RVA | Oracle body | Compared extent | Result |
|---|---:|---:|---:|---|
| MoveObjectStep | `0x115c0` | 591 B | 592 B | STRICT; 18/18 relocations verified |
| MoveObjectStepLine | `0x11810` | 946 B | 960 B | STRICT; 5/5 relocations verified |
| MoveObjectStepIcon | `0x11bd0` | 4,165 B | 4,176 B | RELOC-UNVERIFIED; 60/62 relocations verified |
| TreatNetData | `0x10a90` | 1,545 B | 1,552 B | MISMATCH; score 117 |

[EVIDENCE] **Two new STRICT functions, 1,537 unique original body bytes.**
Terminal NOPs in comparator extents are not body-byte credit. The animation
body has zero non-relocated differences, but its two unproved data targets
prevent STRICT and contribute no strict coverage. No manifest promotion was
attempted. Whole-run coverage includes the verifier's padded extents and is
not used to compute the 1,537-byte credit here.

## Frame map before instruction tuning

[EVIDENCE] Let E be entry ESP and S be ESP after frame allocation and the four
saved registers. Raw operands shift during argument construction; the complete
per-instruction raw access inventories are `final/*-slots.json`.

MoveObjectStepIcon has only `push ecx` local allocation, so S = E-14h.
The saved `this` is S+10h = E-4; the first `[esp+8]` reads the incoming index
before all registers have been pushed. All later raw `[esp+10h]` accesses
reload saved `this`. The drawing/firing POINT components remain in registers.
There is no additional local frame or local array. Candidate and target agree.

MoveObjectStepLine allocates 44h bytes, so S = E-54h:

| Storage at S | Entry-relative | Role |
|---|---|---|
| `+10` | `E-44` | Lift/player intersection flag |
| `+14..1b` | `E-40..39` | 8-byte NetMessage; data1 remains unwritten in both target/candidate |
| `+1c,20` | `E-38,-34` | Saved lift position POINT |
| `+24..30` | `E-30..24` | Player foot RECT |
| `+34..40` | `E-20..14` | Lift RECT |
| `+44..50` | `E-10..4` | IntersectRect output RECT |
| `+58` | `E+4` | Incoming object index |

[EVIDENCE] The first structural candidate used inner-block RECT declarations
and allocated 3ch, overlaying the saved POINT with the expired player RECT.
Moving both real intersection RECT declarations to function scope restores
44h and every target displacement. The unneeded SB2 MoveObject local is
replaced by the real NetMessage required at both host-synchronization calls.

TreatNetData allocates 2b4h, so S = E-2c4h:

| Storage at S | Entry-relative | Role / reuse |
|---|---|---|
| `+10` | `E-2b4` | Received player index; address passed to Receive |
| `+14` | `E-2b0` | Outgoing delivery flag, later message pointer spill |
| `+18` | `E-2ac` | Incoming message-loop counter |
| `+1c` | `E-2a8` | Receive-loop counter |
| `+20..5b` | `E-2a4..269` | 60-byte NetPacket, including five 8-byte messages |
| `+5c..68` | `E-268..25c` | Four BOOL recent-sender entries |
| `+6c` | `E-258` | Death notification format, 100 bytes |
| `+d0` | `E-1f4` | Abandon notification text, 100 bytes |
| `+134` | `E-190` | Lost notification format, 100 bytes |
| `+198` | `E-12c` | Lost notification text, 100 bytes |
| `+1fc` | `E-c8` | Abandon notification format, 100 bytes |
| `+260` | `E-64` | Death notification text, 100 bytes |

[EVIDENCE] All six notification buffers are real separate function-scope
arrays. Snapshotting each incoming message's speed, POINT and type before its
independent dispatch tests restores the target's 2b4h frame and buffer offsets;
the prior candidate used 2ach and reloaded message fields at individual calls.

[EVIDENCE] MoveObjectStep allocates 10h, S = E-20h. Its index spill is S+10,
detected-object index S+14. The remaining eight bytes belong to POINT storage;
raw `+1c`/`+24` uses in the final collision arm include temporary argument-push
shifts. The target and final candidate agree byte-for-byte, including frame.

## Region reconstruction

[EVIDENCE] MoveObjectStepIcon retains the target's ordered independent-if
chain: down/up/moving bombs; four personal bombs; treasure/egg/goal/key/skate;
shield/power/charge/magic track; explosions 1..4; pollution/water effects;
bulldozer/fish/bird; enemy helicopter/tank and their shots; glue/clear/electro;
six static pickup/projectile types; phase increment. SB2-only conveyor lifts,
following bombs, extra keys, dynamite, invert effects, extra explosions,
sploutches, tentacle, bridge, wasp, creature, over, extra tracks and phase-wrap
logic are removed. Explosion 1 uses `phase/3%12`, shield uses `%14`, and the
target's explicit channels are 1 for water/charge and 2 for personal bomb 1.
Enemy firing is disabled in editor phase, uses multiplayer ObjectStart, tests
its BOOL result, and calls the real three-argument PlaySound overload with
FALSE. These changes reproduce all fixed bytes and call sites.

[EVIDENCE] MoveObjectStepLine tests only type 1 and `!m_blupiSuspend` at
`this+c928`, rather than SB2's jump-pain member at c92c. It intersects the
player's feet and lift, saves the lift position, then follows the four-state
motion chain. Stop-start and stop-end each enqueue a message at time zero only
on the network host. Interpolation uses separate x/y expressions so y reloads
the appropriate record fields after x is stored. The target deletes arriving
bubbles/bullets, freezes arriving glue, deletes completed doors and updates the
transport/vector from the initial intersection. SB2 following-bomb, conveyor,
trajectory-marking and no-ascenseur-time paths are absent.

[EVIDENCE] MoveObjectStep restores the SB1 bullet-collision arm missing from
canonical source. A bullet colliding with a moving enemy starts explosion 3,
plays the explosion sound, resets decor shaking, deletes both objects and
starts glue when the enemy was a helicopter or tank. The existing enemy-hit
and electro arms use explicit sound locality and multiplayer creation arguments.
Component vector clears and shared real POINT/index/type locals agree with the
oracle frame and scheduling. This body is fully STRICT.

[EVIDENCE] TreatNetData is absent from canonical decmove.cpp but declared in
the canonical header. Its SB2 ancestor lives in read-only
`external/sb2-decomp/src/decnet.cpp`; oracle TU order places it here. The candidate
reconstructs outgoing message reliability exclusions, at most five messages,
ten Receive iterations, player range/timestamp checks, channel and transport
assignment, component velocity calculation, message dispatch, six notification
buffers, signed-byte pause handling, and smoothing for absent senders.
Regions are outgoing send `10a90..10c4f`, sender reset `10c4f..10c65`, receive
and movement update `10c65..10dd9`, message dispatch `10dd9..10eec`, lifecycle
notifications/pause `10eec..11060`, loop tail `11060..11072`, smoothing/return
`11072..11099`. These are region bounds, not partial STRICT claims.

## Iteration ledger

[EVIDENCE] `scratch/sol-decmove/ledger.jsonl` records tools/asmscore.py scores,
extent, raw/fixed prefix and instruction prefix for each successfully measured
trial. Fixed prefix skips relocation operands only for diagnosis; it never
proves a target. Selected checkpoints:

| Trial | Change | Icon extent / score | Line extent / score | Net extent / score | Step extent / score |
|---|---|---|---|---|---|
| s00 | Canonical baseline | 6752 / 6387 | 1344 / 1261 | absent | 400 / 477 |
| s01 | First animation prune and semantic corrections | 4224 / 1921 | 1344 / 1261 | absent | 400 / 477 |
| s02 | Remove third key; reconstruct lift states/messages/components | 4192 / 1587 | 960 / 65 | absent | 400 / 477 |
| s03 | Remove hide-track arm; correct rectangle scopes and Suspend | 4160 / 303 | 960 / 0 | absent | 400 / 477 |
| s04 | Explicit sound overload/locality | 4176 / 1 | 960 / 0 | absent | 400 / 477 |
| s05a | Initial network body and six buffers | 4176 / 1 | 960 / 0 | 1568 / 1193 | 400 / 477 |
| s06 | Component comparison/velocity, flag initialized at send | 4176 / 1 | 960 / 0 | 1552 / 833 | 400 / 477 |
| s07 | Incoming argument snapshots, positive-delta branch first | 4176 / 1 | 960 / 0 | 1552 / 301 | 400 / 477 |
| s08 | Snapshot speed; component sound position and complete call | 4176 / 1 | 960 / 0 | 1552 / 117 | 400 / 477 |
| s10 / final | Restore bullet-collision region | 4176 / 1 | 960 / 0 | 1552 / 117 | 592 / 0 |

[EVIDENCE] Final raw/fixed/instruction prefixes are Icon 606/4165/981,
Line 139/946/239, Net 262/583/151, Step 63/591/210. Line and Step relocations
are proved by the full verifier; Icon score 1 reflects its missing data proofs.

## Compiler findings

- [EVIDENCE; `BLUPI-LOC-01`, `VC5-LOC-03`; blupi-specific] Lift rectangle
  lifetime/scope was the frame mismatch: inner-block rectangles permitted
  POINT overlap and 3ch allocation, function-scope real rectangles produced
  the target 44h. Together with the independently evidenced c928 member,
  MoveObjectStepLine reaches STRICT, 5/5 relocations. No padding local remains.
- [EVIDENCE; `VC5-SEM-01`, `VC5-ORD-01`; blupi-specific] SB2 point-operator
  interpolation cached values and emitted a larger state body. Component
  expressions, real host messages and pruning unsupported regions reduced
  Line 1344 -> 960 compared bytes; the final scope/member corrections close it.
  Tested in the complete TU with canonical headers and pinned SP3 flags.
- [EVIDENCE; `VC5-SEM-01`; blupi-specific] Animation's last three instruction
  differences were missing FALSE sound-locality arguments, not a register rule.
  Selecting the real overload changes 4160 -> 4176 extent and score 303 -> 1;
  every fixed byte agrees but two table definitions remain unproved.
- [EVIDENCE; `BLUPI-REG-01`, `VC5-CTX-02`; blupi-specific] Removing the genuine
  SB2-only third-key block changes earlier treasure-channel scheduling from
  register materialization/store to the target immediate store followed by
  materialization. Prefix improves 365 -> 904 fixed bytes after the combined
  s02 corrections. This demonstrates nonlocal source-context sensitivity;
  the block must be removed for target semantics regardless of that effect.
- [EVIDENCE; `BLUPI-LOC-01`, `VC5-ORD-01`; blupi-specific] Per-message snapshots
  replace repeated field loads and establish Net's target 2b4h frame. Snapshot
  speed plus component sound-coordinate updates remove the remaining helper
  call and add its real fourth PlayImage argument; score 301 -> 117. Net is
  still MISMATCH, so this is measured progress, not a strict compiler rule.
- [EVIDENCE; `VC5-CTX-01..04`; bounded negative] Before-TreatNetData scratch
  extern-count diagnostics at 1, 4, 8 and 12 leave all four target bodies
  byte-identical to s10. These samples do not disprove other context effects.
  No probe is retained. The full accepted gate passes for all four probes.
- [EVIDENCE; `BLUPI-REG-02`, `VC5-ORD-01`; negative] Net's POINT constructor
  versus component incoming-position assignment (s09), reusing type/speed
  scalar roles (s11), merging the nested player/time predicate (s12), and
  chaining the zero-velocity stores (s14) are byte-inert at the final context.
  Zeroing velocity through POINT(0,0) (s13) worsens score 117 -> 411. Rejected
  experiments are archived; final restores the explicit s10 source.

## Data-owner handoff and remaining work

[EVIDENCE] **No header delta.** The recovered canonical CDecor and MoveObject
layout suffice. Existing method declarations, byte NetMessage.data1, NetPacket,
network player layout and Sound overloads are used unchanged.

[EVIDENCE] Icon's only unverified relocations are offset 719 to table_shield
at VA 0042e6e8 and offset 775 to table_power at VA 0042e730. Canonical headers
declare both arrays but canonical dectables.cpp defines neither. Static oracle
reads and the animation's `%14` / `%8` indexing establish these payloads:

- table_shield: `144,145,146,147,148,149,150,151,150,149,148,147,146,145`.
- table_power: `136,137,138,139,140,141,142,143`.

[EVIDENCE] The SB2 shield reference instead has 16 entries including
266..273, and the SB2 power reference has a leading count word 8. Neither
can be copied unchanged: the SB1 referenced ranges above are plain animation
values. Adjacent SB1 words at 2e720 are `151,150,151,150`, not a power count.
The data owner should reconstruct/prove these definitions in canonical
dectables.cpp, check all consumers, then reverify this candidate. They were not
inserted into this worker's source or hardcoded into relocation mappings.

[OPEN] TreatNetData's remaining differences are confined to register choices
and scheduling in `10cd6..10dd9`: received-player member stores, zero
materialization and previous-position copy. Frame, fixed-byte stream before
that region and the remaining dispatch/notification stream agree. No exact
source form has been established for that region; further guesses reached
diminishing returns. Resume from final/s10, preserving both new strict bodies
and Icon's fixed-byte equality. No additional helper or count-padding source
should be retained to conceal this residue.

[EVIDENCE] `scratch/sol-decmove/final/negative-audit.json` independently checks
object-body equality for the inert variants and four diagnostic probes. It
also counts **137 non-relocated byte differences** in Net, first at RVA
`10cd7`, last at `10dd4`. This count includes changed instruction positions
within that scheduling region; it is not a count of distinct source defects.

## Follow-up after canonical integration

### Review package and validation

[EVIDENCE] Refreshed `scratch/sol-decmove/decmove.cpp` directly from canonical
`src/decmove.cpp` before this pass. The canonical MoveObjectStep, MoveObjectStepLine
and MoveObjectStepIcon bodies were not edited. The final candidate SHA-256 is
`bdb5d5545124bacebc0873d0d1fb324f37d4e70d2b549b49a9c97036e03f7d38`.
`scratch/sol-decmove/audit_round2.py` proves the only source differences from the
refresh snapshot are the thirteen bodies listed below. There is no header delta,
private header overlay, retained probe or change to any other canonical input.

[EVIDENCE] Reproduction uses the same full verifier command above. Final evidence
is archived at `scratch/sol-decmove/round2-final/`: source, object, `results.json`,
transcript, exit status, scores, disassemblies, complete raw stack-operand maps,
`audit.json`, and `decmove.patch`. All compilation was sequential through the
repository tools, pinned SP3 and canonical manifest flags. No commit was made
and no background process remains.

| New STRICT function | RVA | Original body bytes | COFF extent | Verified relocations |
|---|---:|---:|---:|---:|
| TreatNetData | `10a90` | 1,545 | 1,552 | 21/21 |
| DoNetSmooth | `110a0` | 187 | 192 | 1/1 |
| NetAscenseurSynchro | `111b0` | 259 | 272 | 0/0 |
| NetPlayerCollide | `112c0` | 217 | 224 | 1/1 |
| AscenseurDetect | `12c20` | 324 | 336 | 2/2 |
| AscenseurVertigo | `12d70` | 96 | 96 | 0/0 |
| SearchLinkCaisse | `13020` | 453 | 464 | 3/3 |
| MockeryDetect | `13320` | 532 | 544 | 1/1 |
| BlupiElectro | `13540` | 151 | 160 | 1/1 |
| MoveObjectDetect | `135e0` | 538 | 544 | 1/1 |
| MovePersoDetect | `138a0` | 190 | 192 | 1/1 |
| MoveBalleDetect | `13960` | 208 | 208 | 1/1 |
| MoveObjectSort | `13b20` | 234 | 240 | 2/2 |

[EVIDENCE] **13 new STRICT bodies, 4,934 unique original bytes.** All 39 rows
assigned to decmove.cpp in oracle/tu_map.csv have exactly one STRICT owner in
the candidate object, totaling **12,846 original body bytes**. NOP padding is
excluded from these totals. Extra SB2 helper bodies still present in canonical
source are not claimed as SB1 matches merely because they compile.

[EVIDENCE] The final whole-tree verifier exits **1**, solely for the independent
canonical `src/decblupi.cpp` IsDeepWater regression at `10290`. A control run
substituting an untouched canonical decmove copy also exits 1 for that exact
function; see `scratch/sol-decmove/control.log` and `control-results.json`.
The final audit checks that each active accepted failure has the same status
in the control. There are **no additional accepted-function regressions from
this candidate**, and all accepted decmove functions remain STRICT.

[EVIDENCE] During r07/r08, unrelated integration temporarily left
`src/dectables.h`'s table_design_door declaration without its semicolon, stopping
the full build. It was corrected outside this worker's scope; neither that header
nor the independent IsDeepWater failure was edited or masked here. No manifest
acceptance or withdrawal was attempted. Promotion should follow a clean shared
tree check after the independent failure is resolved.

### Frames and regions

[EVIDENCE] The first-pass TreatNetData 2b4h frame map remains exact, as do the
integrated large-body frame maps. New target/candidate maps were made before
instruction tuning. E is entry ESP; S is ESP after local allocation and all
ordinary prologue saves. Every literal ESP operand, including shifted argument
views, is recorded in `round2-final/*-slots.json`.

| Function | Frame / S | Local storage roles |
|---|---|---|
| DoNetSmooth | 8h; initial two saved registers | End POINT at E-8/E-4; EBP/EBX are saved only inside prediction arm; RECT is built directly on the outgoing argument stack |
| NetAscenseurSynchro | 8h; S=E-18h | S+10 cached int type, S+14 vertical distance; horizontal distance in EBP |
| NetPlayerCollide | 30h; S=E-40h | S+10 remote RECT, +20 incoming player's RECT, +30 intersection output |
| AscenseurDetect | 30h; S=E-40h | S+10 swept player RECT, +20 lift RECT, +30 intersection; incoming oldpos.x is reused for index, newpos.x for pointer spill |
| AscenseurVertigo | no locals | Three register saves only; incoming output pointers remain arguments |
| SearchLinkCaisse | 44h; S=E-54h | S+10 outer index, +14 BOOL flag, +18 current linked object, +1c/+20 saved POINT, +24/+34/+44 three RECTs; dead incoming rank slot reused for pointer induction |
| MockeryDetect | 34h; S=E-44h | S+10 saved this, +14 player RECT, +24 moving-object RECT, +34 intersection; tile-test POINT stays in registers |
| BlupiElectro | 30h; S=E-30h | S+0 cloud RECT, +10 incoming RECT, +20 intersection output |
| MoveObjectDetect | 40h; S=E-50h | S+10 object RECT, +20 player RECT, +30 near RECT, +40 intersection; dead incoming pos.x reused for index |
| MovePersoDetect / MoveBalleDetect | 30h; S=E-40h | S+10 incoming RECT, +20 moving-object RECT, +30 intersection |
| MoveObjectSort | 3ch; S=E-4ch | S+10 saved this, +14 loop counter, +18 loop limit, +1c..4b 48-byte MoveObject temporary; change flag ESI and first sort-key result EDI |

[EVIDENCE] Region corrections, in TU-map order:

- TreatNetData keeps the earlier packet regions and all 21 calls. Its final
  motion-store region sets recentSenders before copying the old timestamp in
  source, then writes the received coordinates component by component. The
  scheduler emits the target store order; all fixed bytes and relocations agree.
- DoNetSmooth computes the end POINT from member coordinates/velocity, builds
  the rectangle from those members and passes the member POINT to TestPath.
  Removing the saved start/POINT operator restores the 8-byte frame and reloads.
- NetAscenseurSynchro snapshots int type, horizontal and vertical absolute
  travel distances, then checks each same-type peer's distances before the
  four timing fields. It uses indexed records, not SB2 pointer-local addressing.
- NetPlayerCollide always constructs the peer rectangle, then expands its
  existing sides by 40 for SEC_CLOUD. The old SEC_HIDE gate is unsupported.
- AscenseurDetect removes the SB2 no-lift timer and conveyor variants. The
  sweep step is +/-30 from the signed y difference, and both rectangle edges
  advance by that step. Three real RECTs share function scope. AscenseurVertigo
  ends after its two edge tests; the conveyor-shift aftermath is absent.
- SearchLinkCaisse uses function-scope real loop/scalar/RECT locals and array
  indexing. Its rectangles derive right/bottom from the already assigned
  left/top plus 65. The repeat flag is BOOL. This restores the 44h frame and
  the target's reload/store schedule without pointer or padding locals.
- MockeryDetect checks icon 68 in both cells below an airborne player. Extra
  SB2 enemy kinds and initialized output rectangles are removed. Both direction
  paths reach the same reverse-mockery return through an ordinary goto label.
- BlupiElectro tests the recovered Cloud member and returns a normalized BOOL;
  the output RECT is not initialized. The cloud RECT declaration precedes the
  incoming-player RECT, matching stack order.
- MoveObjectDetect uses direct player bounds, excludes the target's five
  inactive types, skips the crate during push/pop, and ignores the matching
  vehicle at m_blupiPosHelico. It restores SB1 up-bomb/crate/bird/projectile
  bounds. A bullet's right side is x+40 (DIMOBJX-24), and failure leaves pbNear
  unwritten. Rectangle declaration order follows the recovered frame.
- MovePersoDetect uses x+16..x+44 for personal bombs. MoveBalleDetect indexes
  each record through all 200 entries, drops the SB2 wasp and omits destination
  initialization. Their oracle geometry and call order are fully reproduced.
- MoveObjectSort calls the already defined global ::SortGetType, rather than
  the stale same-named CDecor declaration. Its two calls are the operands of
  the comparison directly, preserving target evaluation order and allocation.

### Follow-up score ledger

[EVIDENCE] Complete extent, score, raw/fixed prefix and instruction-prefix
measurements are in ledger.jsonl and each trial's scores.json. Scores are
diagnostic; the full-run relocation proofs above determine acceptance.

| Target | Key checkpoints: compared bytes / score |
|---|---|
| TreatNetData | r00 1552/117; r01 flag after timestamp 1552/25 rejected; r02 components in that order 1552/109; r03 zero-delta arm first 1552/125; r16 flag before previous timestamp 1552/93; r17 plus component stores **1552/0** |
| DoNetSmooth | r00 208/227 -> r04 **192/0** |
| NetAscenseurSynchro | r00 128/363 -> r04 208/301 -> r05 272/33 -> r06 **272/0** |
| NetPlayerCollide | r00 176/171 -> r04 208/137 -> r05 **224/0** |
| AscenseurDetect / Vertigo | r00 416/447 and 176/217 -> r09 **336/0 and 96/0** |
| SearchLinkCaisse | r00 464/367 -> r09 464/371 -> r10 480/361 -> r11 496/211 -> r12 reversed comparisons 496/215 -> r13 **464/0** |
| MockeryDetect | r00 512/471 -> r09 560/49 -> r10 544/37 -> r11 **544/0** |
| BlupiElectro / MovePersoDetect / MoveBalleDetect | r00 176/129,224/175,256/237 -> r09 **160/0,192/0,208/0** |
| MoveObjectDetect | r00 608/561 -> r10 544/13 -> r11 **544/0** |
| MoveObjectSort | r00 256/175 -> r10 reversed comparison 256/175 -> r14 global calls 240/21 -> r15/r18 scope/control variants 240/21 -> r19 direct comparison **240/0** |

### Compiler findings (follow-up)

- [EVIDENCE; `VC5-ORD-01`, `BLUPI-REG-01`; blupi-specific] Net's original
  117-score residue was source scheduling, not solved by the earlier count
  probes. Flag-first source alone gives 93; component coordinates in that
  context give STRICT, 21/21 relocations. Component assignment had been inert
  in the earlier flag order. The lower-score r01 variant is rejected because
  it emits a different actual store order. This is a context-dependent combined
  source-form finding, not a universal assignment-order rule.
- [EVIDENCE; `BLUPI-LOC-01`, `VC5-LOC-03`; blupi-specific] Sync's main distance
  POINT reserves an unused component slot when only y spills: frame 0ch,
  otherwise matching instruction stream, score 33. Real scalar dx/dy variables
  remove that slot, restore frame 8h, and produce STRICT. The peer-distance
  POINT stays in registers. No artificial declaration is retained.
- [EVIDENCE; `BLUPI-LOC-01`, `VC5-ORD-01`; blupi-specific] SearchLinkCaisse's
  scoped locals and explicit pointer loops kept a 48h frame. Real function-scope
  variables and indexed loops produce 44h, but direct member-derived rectangle
  endpoints remain 496 B/211. Restoring the ancestor's left/top-derived right
  and bottom expressions yields 464 B/0 and STRICT. Comparison-operand reversal
  (r12) worsens score to 215 and is rejected.
- [EVIDENCE; `VC5-CTL-03`, `VC5-ORD-01`; blupi-specific] Mockery's semantically
  equivalent immediate reverse-mockery returns duplicate an epilogue. A shared
  authentic goto return removes that layout difference: 544 B/37 -> 544 B/0,
  STRICT. Its earlier two-direction restructuring alone did not suffice.
- [EVIDENCE; `VC5-SEM-01`; blupi-specific] MoveObjectDetect's last 13-score
  differences were the actual bullet extent (DIMOBJX-24, not DIMBLUPIX-24)
  and the y comparison's operand order. Correcting both yields STRICT; neither
  was dismissed as a register-only residue.
- [EVIDENCE; `VC5-SEM-01`, `BLUPI-REG-01`; blupi-specific] Sort's member/global
  name collision selected thiscall and omitted caller cleanup. Explicit global
  qualification restores cdecl and reduces 256 B/175 -> 240 B/21. Replacing
  separate sort-key locals with the direct two-call comparison swaps flag/key
  registers to target ESI/EDI and reaches STRICT with both calls proved. Moving
  those locals to function scope (r15) and wrapping the final region instead
  of an early return (r18) were byte-inert at their checkpoints. No filler or
  header declaration removal was required.

[EVIDENCE] All findings use the full canonical-header TU and pinned SP3 manifest
flags. Every retained gain was checked against the integrated large matches and
the active accepted set. The only unresolved verification issue is the independently
reproduced IsDeepWater failure outside this worker's ownership. Next action is
supervisor integration of the thirteen-body patch followed by the clean shared
tree verifier; there is no remaining unmatched oracle function in this TU block.
