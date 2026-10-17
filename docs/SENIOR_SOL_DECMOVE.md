# Senior decmove structural pass (sol-decmove, 2026-10-17)

## Result and reproduction

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
