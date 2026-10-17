# BlupiStep structural match

## Current result

[EVIDENCE] The oracle inventory has one `FUN_00409280` body, 25,269 bytes, at RVA `0x9280`; forced comparisons place the candidate at that entry. Its `functions.csv` end field is `0xf535` (exclusive), consistent with the size. The current point-scoped candidate is 29,200 bytes, frame `0xbc`, raw prefix 2, and `MISMATCH`; it is 3,931 bytes over the target. The target frame is `0xa4`. No new STRICT function was produced; the decblupi TU retains its seven accepted STRICT bodies and two relocation-unverified bodies.

[EVIDENCE] All candidate verification used VC5 SP3, `/O2 /GX /ML /Gd /Gy`, the full decblupi TU and all run objects, with `--build-dir bigblupi`; `cmpfunc.py` forced `?BlupiStep@CDecor@@QAEXXZ=0x9280`. `scratch/bigblupi/decblupi.cpp` and its private header are the only source inputs changed for this work. The full run leaves the existing accepted functions unchanged.

## Prefix and size log

Raw prefix is byte equality from entry. The first mismatch remains the stack-allocation immediate (`bc` candidate versus `a4` target). The earlier localized comparison at RVA `0x92c8-0x9331` found 102 equal non-relocation bytes in a 106-byte span; it is a local match, not a STRICT function.

| Candidate step | Bytes (target 25,269) | Raw prefix | Frame | Result |
|---|---:|---:|---:|---|
| Canonical-header baseline | 31,264 | 2 | `0xc8` | MISMATCH |
| Entry snapshot cleanup | 31,232 | 2 | `0xc8` | MISMATCH |
| Fixed first collision rectangle, no point reuse | 31,200 | 2 | `0xc8` | MISMATCH |
| Private vent/surf layout and safe point reuse | 31,264 | 2 | `0xc4` | MISMATCH |
| Remove `MOCKERYp`/`NON` switch cases | 31,184 | 2 | `0xc4` | MISMATCH |
| Ordered independent-if action chain | 31,488 | 2 | `0xc4` | MISMATCH |
| Remove delayed `MOCKERYp` path | 31,472 | 2 | `0xc4` | MISMATCH |
| Remove dynamite pickup/place lifecycle | 31,008 | 2 | `0xc4` | MISMATCH |
| Collapse added death variants to `CLEAR1` | 30,784 | 2 | `0xc4` | MISMATCH |
| Remove teleport state and switch animation | 30,480 | 2 | `0xc4` | MISMATCH |
| Remove development `sprintf` overlay | 30,448 | 2 | `0xc4` | MISMATCH |
| Set mockery timer to oracle value 200 | **30,432** | **2** | **`0xc4`** | **MISMATCH** |
| Scope `tinyPoint2` to its two sibling blocks | 30,432 | 2 | `0xbc` | MISMATCH |
| Read `m_detectIcon` directly at the sound call | 30,416 | 2 | `0xbc` | MISMATCH |
| Replace SB2 FIFO gate with target predicate; remove its helper chain | 30,112 | 2 | `0xbc` | MISMATCH |
| Remove unmatched `IsSwitch` / `ActiveSwitch` branch | 29,520 | 2 | `0xbc` | MISMATCH |
| Match oracle `MockeryDetect` caller eligibility gate | **29,488** | **2** | **`0xbc`** | **MISMATCH** |
| Supervisor's canonical refresh from `src/decblupi.cpp` | 29,216 | 2 | `0xbc` | MISMATCH |
| Scope sibling `tinyPoint` temporaries without moving later source lines | **29,200** | **2** | **`0xbc`** | **MISMATCH** |

[EVIDENCE] Replacing the pruned action switch with ordered independent `if` blocks added 304 bytes. That shape matches the oracle's direct compare chain; its cost is retained. The current body is 3,931 bytes larger than the oracle.

## Frame map

The raw-access tables are from Capstone over the oracle bytes and current SP3 COFF body. “First” is the instruction offset from function entry (add `0x9280` for an oracle RVA); each listed operand is 4 bytes. Modes are `R`/`W` and `A` for address-only `lea`. Displacements are literal `[esp+N]` operands, including call arguments and reused storage. Candidate symbol locations below come from the source-annotated SP3 listing, expressed relative to entry ESP (`E-N`).

### Oracle raw `[esp+N]` operands

| Operand | Bytes | Access | First | Uses |
|---|---:|---|---:|---:|
| `+0x10` | 4 | RWA | `0x0048` | 102 |
| `+0x14` | 4 | RW | `0x004c` | 106 |
| `+0x18` | 4 | RWA | `0x0112` | 86 |
| `+0x1c` | 4 | RWA | `0x0040` | 15 |
| `+0x20` | 4 | RWA | `0x0044` | 25 |
| `+0x24` | 4 | RW | `0x01de` | 15 |
| `+0x28` | 4 | RW | `0x2f83` | 10 |
| `+0x2c` | 4 | RWA | `0x2f91` | 10 |
| `+0x30` | 4 | RWA | `0x0b0f` | 9 |
| `+0x34` | 4 | W | `0x0900` | 3 |
| `+0x38` | 4 | WA | `0x07e7` | 4 |
| `+0x3c` | 4 | RW | `0x07fc` | 2 |
| `+0x40` | 4 | W | `0x2f7b` | 1 |
| `+0x44` | 4 | RW | `0x0030` | 11 |
| `+0x48` | 4 | RW | `0x0026` | 8 |
| `+0x4c` | 4 | RW | `0x2c04` | 5 |
| `+0x50` | 4 | A | `0x4fe2` | 3 |
| `+0x54` | 4 | RW | `0x2c0b` | 4 |
| `+0x5c` | 4 | R | `0x2fa5` | 6 |

### Candidate raw `[esp+N]` operands

| Operand | Bytes | Access | First | Uses |
|---|---:|---|---:|---:|
| `+0x10` | 4 | RWA | `0x0048` | 124 |
| `+0x14` | 4 | RW | `0x004c` | 114 |
| `+0x18` | 4 | RWA | `0x0040` | 21 |
| `+0x1c` | 4 | RWA | `0x0044` | 35 |
| `+0x20` | 4 | RW | `0x01fb` | 16 |
| `+0x24` | 4 | RW | `0x50d5` | 7 |
| `+0x28` | 4 | RW | `0x01d2` | 7 |
| `+0x2c` | 4 | WA | `0x021e` | 9 |
| `+0x30` | 4 | RW | `0x0225` | 5 |
| `+0x34` | 4 | W | `0x0233` | 4 |
| `+0x38` | 4 | RW | `0x01fb` | 12 |
| `+0x3c` | 4 | W | `0x30d0` | 1 |
| `+0x40` | 4 | RWA | `0x01c4` | 14 |
| `+0x44` | 4 | RWA | `0x010e` | 17 |
| `+0x48` | 4 | RWA | `0x01ce` | 16 |
| `+0x4c` | 4 | R | `0x34a4` | 5 |
| `+0x50` | 4 | R | `0x34aa` | 6 |
| `+0x54` | 4 | RWA | `0x30b0` | 5 |
| `+0x58` | 4 | RWA | `0x01a7` | 10 |
| `+0x64` | 4 | RWA | `0x0030` | 4 |
| `+0x68` | 4 | A | `0x6dfa` | 2 |

[EVIDENCE] The target Ghidra locals are `local_a4`, `local_a0`, `local_9c`, `local_98`, `local_94`, `local_90`, `local_8c`, `local_88`, `local_84[2]`, `local_7c`, `local_74`, `local_70`, `local_6c`, `local_68`, and `local_64[100]`. The first fourteen span the target's 64-byte local region before the 100-byte notification buffer. Pseudocode roles: endpoint x/y; index/pointer; three flags; saved point; an 8-byte output; and five scalar scratch values. The target buffer is addressed at raw `+0x50` (effective entry-ESP address `E-0x64`).

[EVIDENCE] The current SP3 listing has frame allocation `0xbc`; its fixed prologue consumes `0xcc` bytes including saved registers. Candidate locals are `POINT end` at `E-0xbc` (8); `bIsOpen` at `E-0xb4` (4); `bIsOpen2` at `E-0xb0` (4); `bLastBlupiAir`, scoped `POINT tinyPoint2`, and several sibling scoped `POINT tinyPoint` temporaries at `E-0xac` (point temporaries occupy 8 bytes and share the first dword with `bLastBlupiAir` by lifetime; union extent 8); `RECT rect` at `E-0xa4` (16); `bAirStart` at `E-0x94` (4); `bNear` at `E-0x8c` (4); `num`/`num27` and one scoped `POINT tinyPoint` at `E-0x88` (the point occupies 8 bytes and shares the first dword with `num` by lifetime); `POINT blupiPos` and compiler temporaries at `E-0x78`; `lastBlupiAction` at `E-0x68` (4); and `char str[100]` at `E-0x64`. `icon` has no stack symbol in the listing. The candidate buffer operand is raw `+0x68`, resolving to `E-0x64`; the target buffer operand is raw `+0x50`, also resolving to `E-0x64`.

[EVIDENCE] The supervisor's refreshed canonical candidate is six dwords above the target (`0xbc` vs `0xa4`). Scoping sibling point temporaries leaves the frame unchanged but reduces the body by 16 bytes (29,216 to 29,200). The current candidate's local extent below `str` is 88 bytes versus the target's 64 bytes. `str[100]` is the only source array in candidate `BlupiStep`; no other candidate arrays need ordering under BLUPI-LOC-01, and both notification buffers start at the same effective stack address.

[EVIDENCE] Extent accounting for the current candidate gives 68 bytes of distinct named stack storage after lifetime sharing and 20 bytes of holes within the 88-byte local region. The target pseudocode names 60 bytes of locals in its 64-byte region, leaving 4 bytes unused. The candidate holes are `E-0x90..E-0x8c` (4 bytes), `E-0x80..E-0x78` (8), and `E-0x70..E-0x68` (8): 16 bytes more gap than the target. The remaining 8-byte difference is named-object extent; `RECT rect` is the only 16-byte source local, while the target pseudocode has an 8-byte `local_84[2]` scratch aggregate. This is a plausible two-dword excess, but the aggregate mapping is not proven. The 4-dword padding difference is caused by current local layout; its specific source lifetime cause is unresolved.

[OPEN] The raw target table gives every decoded `[esp+N]` access (width, access mode, first instruction, and use count), and the candidate table is regenerated from the verified object. The candidate currently has 17 dwords of named storage versus 15 named target dwords. `end`, the two entry-zero flags, `bLastBlupiAir`, and `lastBlupiAction` all have plausible target live-value counterparts from their initialization/use patterns, so they are not established as SB2-only. The two extra named dwords and the four dwords of excess holes have not been assigned confidently to specific source locals. A probe that split the RECT into 19 use-block locals cut 32 bytes from code but increased the frame `0xbc -> 0xc4`; it was reverted. Do not delete or retag a local solely to force the frame.

## Control-flow and behavior map

[EVIDENCE] A static Capstone scan of the 25,269-byte oracle body found 6,505 instructions and 94 direct jumps; there is no indirect jump dispatch. Thus the oracle has no `m_blupiAction` jump table. Its early action handlers are a sequence of independent comparisons. The references to `.data` RVAs `0x31a58`, `0x31a68`, and `0x31a88` are movement-speed lookup tables, not dispatch tables; their contents match SB2's `table_vitesse_march`, `table_vitesse_nage`, and `table_vitesse_surf`. A separate short-table reference occurs at `.data` RVA `0x31aa0`; its SB2 source symbol is unresolved.

| Oracle pseudocode region | Oracle behavior/evidence | SB2 counterpart | Coverage |
|---|---|---|---|
| Lines 45-94 | Snapshot air/action; endpoint and `TestPath`; vent tile offsets and flag at `this+0xc924`; floor test | `decblupi.cpp:389-462` | Strong. First-path bytes include the localized match above; private header swaps vent/surf order. |
| Lines 95-251 | Ground, spring, jump setup, vehicle ejection and associated sounds | `:463-560` | Strong behavior match by flags, action values, tile tests, and sound calls. |
| Lines 253-311 | Air/fall update, landing, wall collision, lift detection | `:561-660` | Strong behavior match by vertical velocity, `FUN_00412c20` lift lookup, and `AscenseurDetect` equivalent. |
| Lines 312-454 | Ordered action phase completions: 16 values `0x24,0x38,0x3a,0x2c,0x2d,0x2e,0x2f,0x30,0x41,0x3f,0x40,0x3c,0x3d,0x3e,0x31,0x37` | `:662-803` | High. Candidate now uses ordered `if`s; `MOCKERYp` (`0x53`) and `NON` (`0x54`) were removed. |
| Lines 455-870 | Up/down and left/right input, turning, march movement and sound/state transitions; march table at `.data 0x31a58` | `:804-1174` | High; target compares action/direction fields directly and uses the same speed-table data. |
| Lines 871-1277 | Helicopter/over movement and action transitions; vehicle-state checks | `:1266-1962` | Broad behavior coverage for helicopter, over, balloon, crushed, jeep, tank, and skate blocks; exact branch order still needs alignment. |
| Lines 1278-1450 | Water/surf movement, vertical alignment, speed tables at `.data 0x31a68/0x31a88` | `:1963-2160` | Strong data/behavior correspondence to `table_vitesse_nage/surf`; confirm member mapping during alignment. |
| Lines 1455-2054 | Lift/platform release, path collision retries using the saved position, and contact/turn responses | `:1151-1265`, `:2190-2420` | Partial; target `TestPath` retries and candidate endpoint/snapshot logic correspond, but branch nesting and local lifetimes differ. |
| Lines 2051-2670 | Object, hazard, door and pickup interactions; short-table at `.data 0x31aa0`; sounds and effects | `:2860-3724` | Broad mapping by object type, movement helpers, sound IDs and inventory effects. SB2-only sub-blocks are listed below. |
| Lines 2672-2746 | Delayed action queue: values `0x2c`, `0x2d`, `0x41`, `0x2f`, `0x30`, `0x3f/0x40`; counter/timer fields | `:3725-3807` | Behavior counterpart found. Current header offsets/names do not match target queue storage; see header delta. |
| Lines 2747-3709 | Death completion, world/game end, multiplayer, scroll/focus, notification buffer | `:3807-3965` | Broad mapping. Target `LoadStringA(0x11f, local_64, 100)` and notification call correspond to SB2 `LoadString(287, str, 100)`/`NotifPush`; the SB2 `time %d` / `act %d` overlay is absent from oracle and was removed. |

[EVIDENCE] No direct comparison/assignment literal for target action field `this+0xc8d8` uses `0x4a`, `0x4b-0x54`, `0x56`, or `0x57`; `0x0b` is used for the common death action. These absent values are SB2 teleport, extra death, switch/mockery/non, and dynamite states. The exact direct-literal set is recorded in the decompilation and can be regenerated from `build/decomp/tu/decblupi.c`.

[EVIDENCE] Oracle pseudocode lines 2132-2140 call `FUN_00413320` and, for nonzero result, assign it to `+0xc974` and clear `+0xc970`. This is the behavior counterpart of SB2 `CDecor::MockeryDetect`: target helper `0x13320` reads the `+0xc96c` timer, tests air tile icon `0x44` for result `0x40`, then intersects the Blupi rectangle with object types `2, 0x10, 4, 0x14, 0x17, 0x20, 0x21`. SB2 `MockeryDetect` has extra tile/object cases, including icon `317`, `TYPE_CREATURE`/`ACTION_MOCKERYp`, and added object types. The caller gate is now the target set (`+0xc90c/+0xc910/+0xc914/+0xc918` clear and focus `+0xc904` set); the helper implementation is in `decmove.cpp`, outside this worker's source ownership.

[EVIDENCE] After the removals, every live `ACTION_*` constant in the scratch `BlupiStep` maps to a value present in the oracle's direct `this+0xc8d8` literal set; only the commented `ACTION_NON` line remains outside the compiled function.

[EVIDENCE] The target has no indirect jump in the whole body. Its 16 action-phase values are handled by ordered comparisons; there is no `m_blupiAction` or state jump table. The scratch action phase now uses ordered `if` blocks in the same value order.

### SB2-only blocks removed in the scratch candidate

- [EVIDENCE] Removed the `ACTION_MOCKERYp` and `ACTION_NON` action handlers, then removed the delayed `MOCKERYp` queue and sound-stop path. Target queue/state values stop at `MOCKERY`/`MOCKERYi` (`0x3f/0x40`).
- [EVIDENCE] Removed dynamite pickup (`TYPE_DYNAMITE`/`ACTION_TAKEDYNAMITE`), placement (`ACTION_PUTDYNAMITE`), and their completion paths. Target has no `0x56/0x57` action state; target `this+0xc974` is used as the queued action field.
- [EVIDENCE] Collapsed SB2 `CLEAR2..CLEAR8` action variants and phase completions to `CLEAR1`; target death paths set action `0x0b` and use its phase `0x46` completion. The helper calls now pass only `ACTION_CLEAR1` for these hazards.
- [EVIDENCE] Removed the `IsSwitch`/`ActiveSwitch` tile branch and its `ACTION_SWITCH` transition. The target body has no action `0x52`, no call behaviorally identified with those helpers, and no matching icon literals `384/385`; this branch had no identified oracle counterpart.
- [EVIDENCE] Removed the `ACTION_TELEPORTE` animation and its FIFO-gate exclusion. The target body has no action `0x4a`, no call behaviorally identified as `IsTeleporte`, and no teleporter icon IDs `0x7e/0x81/0x84/0x87` in this function.
- [EVIDENCE] Replaced the SB2 FIFO gate with the target predicate: focus on; air, helicopter, jump-injury, both local open flags off; transport `-1`. It restores the first FIFO position only when queue count is positive, then calls `BlupiAddFifo`. This removes SB2 helper checks for ground, balloon, crushed, shield/hide, lava/trap/drop/saw/bridge/teleporter and trajectory states; target pseudocode uses only fields `+0xc904`, `+0xc908`, `+0xc90c`, `+0xc930`, two locals, `+0xc900`, `+0xc978`, and `+0xc97c/+0xc980` here.
- [EVIDENCE] Removed the active `sprintf("time %d")` / `sprintf("act %d")` development overlay and timer update. No corresponding call or string use appears in the target pseudocode.

[OPEN] Absence of corresponding action IDs and identifiable teleporter/switch helper calls is established for this function. Whether SB1 handles those map tiles through a different object interaction path remains unproven; do not add a replacement branch without matching target table/offset/callee evidence.

## Header delta

[EVIDENCE] Target clears/sets the field at `this+0xc924` around the vent-icon test and tests `this+0xc928` as a separate state flag. In the private header, `m_blupiVent` is at `0xc924` and `m_blupiSurf` at `0xc928`; the canonical header order is reversed. This scratch-only swap preserves class size and fixes the early field offsets.

[EVIDENCE] Target pseudocode and direct object offsets establish this five-dword mapping:

| Offset | Target behavior | Current `src/decor.h` member at this offset | Target-shaped member name |
|---|---|---|---|
| `+0xc964` | 100-count shield timer; values/thresholds 100, 25, 20, 10 and countdown | `m_blupiFront` | `m_blupiTimeShield` |
| `+0xc968` | 10-count fire cooldown; initialized/decremented | `m_blupiBullet` | `m_blupiTimeFire` |
| `+0xc96c` | mockery timer, set to 200 in action phases `0x3f/0x40`, then decremented | `m_blupiCle` | `m_blupiTimeMockery` |
| `+0xc970` | delayed-action counter, incremented and compared with phase thresholds | `m_blupiPerso` | `m_blupiTimeOuf` |
| `+0xc974` | pending action values and reset-to-zero logic | `m_blupiDynamite` | `m_blupiActionOuf` |

[EVIDENCE] A VC5 SP3 wrapper-offset probe compiled both `src/decor.h` and the private `scratch/bigblupi/decor.h`; both report the current early names and tail offsets shown here. The canonical header currently places the corresponding time/action members later: `m_blupiTimeShield` `+0xd208`, `m_blupiTimeFire` `+0xd20c`, SB2-only `m_blupiTimeNoAsc` `+0xd210`, `m_blupiTimeMockery` `+0xd214`, `m_blupiTimeOuf` `+0xd218`, and `m_blupiActionOuf` `+0xd21c`. Thus the integration delta is to represent the target meanings at `+0xc964..+0xc974` and not leave those five meanings only in the tail. The current names at those five earlier slots must be relocated, aliased, or removed according to whole-header evidence; the SB1 offsets of `m_blupiFront`, `m_blupiBullet`, `m_blupiCle`, `m_blupiPerso`, and `m_blupiDynamite` remain open. `m_blupiTimeNoAsc` has no proven SB1 counterpart. The correct placement of displaced SB2 fields is a whole-TU/header integration decision.

## Compiler findings

- `[VC5-LOC-03; confirmed candidate effect, SB1 transfer unverified]` Trigger: eight-dword frame excess. Moving `POINT tinyPoint2` from function scope into its two sibling use blocks changed its stack slot to a shared lifetime slot and reduced allocation `0xc4 -> 0xbc` (8 bytes); code size stayed 30,432. SP3 `/O2`, full decblupi TU, `/GX /ML /Gd /Gy`; whole function remained MISMATCH. `kb.py hints BlupiStep` returned keyword matches only. The remaining six dwords are not accounted for yet.
- `[VC5-LOC-03; negative scope probe, SB1 transfer unverified]` Scoping sibling `POINT tinyPoint` temporaries with line-preserving block declarations reduced `BlupiStep` by 16 bytes (29,216 to 29,200) but left the frame at `0xbc` and raw prefix at 2; full decblupi TU stayed MISMATCH with accepted functions strict. No local extent was reclaimed by this probe.
- `[VC5-LOC-03; negative scope probe, SB1 transfer unverified]` Splitting the long-lived rectangle into 19 block-local `RECT` declarations reduced code 32 bytes but expanded frame `0xbc -> 0xc4`; reverted. This use-block split did not satisfy the target's frame reuse pattern.
- `[BLUPI-LOC-01; no ordering probe needed]` Candidate has only one source array in `BlupiStep`, `char str[100]`; there is no second candidate array to reorder by ascending size or sibling scope. It occupies the same effective stack address as the target notification buffer. The target pseudocode's `local_84[2]` is an 8-byte output, not a second source array in this candidate.
- `[VC5-SEM-01; Blupi-specific source correction]` Trigger: candidate's FIFO save-position gate contained SB2-only helper checks. Replacing it with the exact oracle predicate/queue operation reduced code by 304 bytes (30,416 to 30,112), frame and prefix unchanged. Flags/TU: VC5 SP3, full decblupi TU, `/O2 /GX /ML /Gd /Gy`; full function remained MISMATCH. Evidence: oracle pseudocode lines 2869-2878 and offsets `+0xc904/+0xc908/+0xc90c/+0xc930/+0xc900/+0xc978/+0xc97c/+0xc980`.
- `[VC5-SEM-01; Blupi-specific, counterpart absent in this function]` Trigger: SB2 branch called `IsSwitch`/`ActiveSwitch`, used tile IDs 384/385, and assigned action `0x52`. Removing it reduced code by 592 bytes (30,112 to 29,520), frame/prefix unchanged. Oracle body has no action `0x52`, corresponding helper call, or those icon values; function remains MISMATCH. Whether SB1 reaches a similar effect through another path is open.
- `[VC5-SEM-01; Blupi-specific source correction]` Trigger: the `MockeryDetect` caller guard excluded SB2 states beyond the oracle. Replaced it with the target predicate (focus set; helicopter, over, jeep, tank clear), reducing the caller body by 32 bytes (29,520 to 29,488), frame/prefix unchanged. The oracle call is `FUN_00413320` at pseudocode lines 2135-2140; its return values and tile/object tests correspond by behavior. VC5 SP3, full decblupi TU, `/O2 /GX /ML /Gd /Gy`; no STRICT match.
- `[VC5-SEM-01; Blupi-specific source correction]` Target sets its mockery timer at `this+0xc96c` to 200 in both `0x3f` and `0x40` phase handlers; SB2 source used 300. Changing both values reduced code by 16 bytes (30,448 to 30,432), frame/prefix unchanged; full function remained MISMATCH.
- `[VC5-SEM-01; Blupi-specific source correction]` Replacing the `detectIcon` local snapshot with a direct `m_detectIcon` read at the sound call reduced code by 16 bytes (30,432 to 30,416), frame/prefix unchanged; it did not help frame alignment and the function remained MISMATCH.
- `[VC5-CTX-01/03; negative probe]` A prior diagnostic candidate with seven top-level `extern` declarations did not change `BlupiStep` size, frame, or prefix. The probe was removed and is not claimed source.
- `[BLUPI-specific; source correction]` Removing `BlupiAdjust`/entry initializer and using the target's first fixed collision rectangle gave the localized 102-byte non-relocation match, but no full STRICT match.

## Next alignment steps

[OPEN] Resolve the six-dword frame excess by matching candidate local lifetimes/slots to the 15 target dwords and four-byte target gap; the tested point and rectangle scope families did not produce that layout. Once the frame is `0xa4`, align from the prologue forward. The target's 16-action compare order, movement tables, and major callees are mapped; continue removing only SB2 blocks lacking an oracle counterpart by tables, offsets, callees, sounds, or action values, and reconstruct any differing switch/teleporter behavior from the oracle bytes. The five queue-field names/offsets above are a precise header delta for infra to integrate across TUs; no canonical header was edited here.
