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

## Senior pass — sol-blupistep, 2026-10-17

### Result and reproduction

[EVIDENCE] `scratch/solblupi/decblupi.cpp` changes only `BlupiStep`; its prefix before that definition and suffix beginning with `BlupiDead` equal the starting canonical source. It uses a private `scratch/solblupi/decor.h`, described below. No canonical source/header, manifest, catalog, assets, or external repository was changed by this pass.

[EVIDENCE] Final verifier size is **25,024 bytes**, frame **`0xa4`**, raw identical prefix **21 bytes**, status **MISMATCH** at target RVA `0x9280`. The candidate's `ret` ends at offset `0x61bd` (25,021 bytes), followed by three NOPs in its COFF extent; the oracle ends at `0x62b5` (25,269 bytes). Thus the remaining body deficit is 248 bytes; the verifier-extent deficit is 245 bytes. Neither frame equality nor the call alignment is a STRICT match.

```powershell
.pyenv\python.exe tools\verify.py --tu src/decblupi.cpp=scratch\solblupi\decblupi.cpp --build-dir solblupi
.pyenv\python.exe scratch\solblupi\audit.py
.pyenv\python.exe scratch\solblupi\calls.py
.pyenv\python.exe scratch\solblupi\slots.py
```

[EVIDENCE] All candidate evaluations used that verifier with the full TU/object set, pinned SP3, `/O2 /GX /ML /Gd /Gy`, and repository TMP/TEMP. Final log: `scratch/solblupi/verify-final.log`; results: `build/verify-solblupi/results.json`. The run reports one accepted-function regression: unchanged canonical `DecorDetect`, whose member names encode the old header layout. This scratch candidate must not be promoted wholesale. Incidental unchanged peers `IsLave` (`0x10150`, 88 inventoried body bytes) and `IsRessort` (`0x101b0`, 94 body bytes) are STRICT; the comparator reports 96-byte extents including their oracle alignment bytes. Do not count the padding as recovered code or count either peer twice if already recovered elsewhere.

### Iteration log

Sizes below consistently use the verifier/COFF extent, as in the earlier log. Every BlupiStep result is MISMATCH. Snapshots and logs remain under `scratch/solblupi/`.

| Step | Change | Bytes | Frame | Prefix |
|---|---|---:|---:|---:|
| 00 | Fresh canonical copy | 29,216 | `bc` | 2 |
| 01 | Fixed collision rectangles; upper collision result evaluated before spring/fall gates | 28,912 | `ac` | 2 |
| 02 | Remove unsupported vehicle/state/terrain blocks; restore object detection after mockery | 24,304 | `a8` | 2 |
| 03 | Explicit SB1 predicates/death resets; remove helicopter wall helper | 24,256 | `a8` | 2 |
| 04 | Private layout; separate ammunition/inventory from timers | 24,480 | `a8` | 2 |
| 05 | Reuse saved point through path retries and old-position snapshot | 24,480 | `a8` | 2 |
| 06 | Four independent spring vehicle destructions; collision threshold/order | 24,800 | `a8` | 2 |
| 07 | Declare upper collision result at function scope | 24,800 | `a8` | 2 |
| 08 | Reuse `icon` for upper collision result | 24,800 | `a8` | 2 |
| 09 | Explicit sound locality/object multiplayer arguments | 24,944 | `a8` | 2 |
| 10 | Reuse saved point as GetPosDecor output | 24,944 | `a8` | 2 |
| 11 | Initialize a distinct block-local GetPosDecor output; reverted | 24,944 | `ac` | 2 |
| 12 | Spring input chain, vent eligibility, BOOL object-start success tests | 24,976 | `a8` | 2 |
| 13 | Fix lift/OUF4 index variable reuse | 24,960 | `a8` | 2 |
| 14 | Move the sole `str[100]` to function scope | 24,960 | `a8` | 2 |
| 15 | Restore network-cloud collision, including `NetPlayerCollide(..., &num)` | 26,336 | **`a4`** | **21** |
| 16 | Remove colored-key pickups and creature contact | 24,960 | `a4` | 21 |
| 17 | Motor sound transitions; remove jeep edge/slope behavior; combine magic trails | 24,768 | `a4` | 21 |
| 18 | Independent motor checks, treasure/glue/shield/personal-bomb responses, world/scroll constants | 24,928 | `a4` | 21 |
| 19 | Correct shield/power slots, skateboard jump speeds, vehicle pickup sounds/grace gates | 25,136 | `a4` | 21 |
| 20 | Explicit endpoint x/y sums instead of point copy plus increments; reverted | 25,120 | `a4` | 21 |
| Context | 1, 2, 3, 4, 7, 8, 12, 16 extern declarations immediately before the function; diagnostic only | 25,136 each | `a4` | 21 |
| 21 | Target input release/crate/hazard/mockery predicates; retained final | **25,024** | **`a4`** | **21** |
| 22 | Separate/reorder snapshot declarations while retaining initialization order; reverted | 25,024 | `a4` | 21 |

[EVIDENCE] Context probes were evaluated through the required verifier using `scratch/solblupi/probe.cpp`, never retained in the deliverable candidate. All eight emitted identical BlupiStep bytes. Their logs and `context-results.json` record the negative result. The entry residue is not resolved by this tested declaration-count family.

### Corrected complete target stack map

[EVIDENCE] `slots.py` walks the instruction CFG and accounts for pushes, explicit ESP arithmetic, and each callee's `ret N` (including the sound overload ABI). For the oracle it visits **all 6,505 instructions with no unknown call cleanup and no conflicting stack depths**. The earlier raw-displacement table must not be treated as a fixed-local map: arguments pushed before an access change the literal displacement.

[EVIDENCE] Coordinates below are relative to ESP after `sub esp,0xa4` and the four saved-register pushes, i.e. entry ESP minus `0xb4`. The final candidate has the same set of effective local offsets. Access counts are target/final candidate, excluding indirect reads through returned pointers.

| Fixed offset | Entry-ESP coordinate | Source role | Direct uses T/C |
|---|---|---|---:|
| `10` | `E-a4` | `end.x` | 122 / 120 |
| `14` | `E-a0` | `end.y` | 104 / 103 |
| `18` | `E-9c` | `num`: tile/object/lift/network index; address escapes to NetPlayerCollide | 72 / 66 |
| `1c` | `E-98` | `bIsOpen` | 20 / 20 |
| `20` | `E-94` | `bIsOpen2` | 19 / 19 |
| `24` | `E-90` | `bAirStart` | 16 / 16 |
| `28`, `2c` | `E-8c`, `E-88` | Saved attempted endpoint, then old Blupi position; later point-return storage | 13, 8 / 13, 8 |
| `30` | `E-84` | Four-byte `BOOL bNear`, passed to MoveObjectDetect | 8 / 8 |
| `34` | `E-80` | Rectangle left component stays in registers; no effective direct access | 0 / 0 |
| `38` | `E-7c` | `rect.top` scalar spill across calls | 16 / 16 |
| `3c` | `E-78` | Rectangle right component stays in registers; no effective direct access | 0 / 0 |
| `40` | `E-74` | `rect.bottom` scalar spill across calls | 11 / 11 |
| `44` | `E-70` | `lastBlupiAction`, later vertical scroll distance | 5 / 5 |
| `48` | `E-6c` | `bLastBlupiAir`, later object-position/point-return x storage | 8 / 8 |
| `4c` | `E-68` | Corresponding point y storage | 1 / 1 |
| `50..b3` | `E-64..E-1` | `str[100]` | 2 / 2 addresses |

[EVIDENCE] This corrects two earlier interpretations: `local_84[2]` is not proof of an eight-byte output local, and the target does not lack an ordinary 16-byte RECT. Its left/right components need no stack reads; top/bottom spill. The effective local area has 56 directly accessed bytes plus two four-byte rectangle component positions, before the 100-byte buffer. The upper collision result is kept in `ebp` in the target, rather than requiring another persistent scalar slot. BLUPI-LOC-01 has no second source array to order here; moving the single text buffer's scope was inert.

### Region reconciliation

[EVIDENCE] With the following corrections, `calls.py` aligns **346 target calls and 346 candidate calls in identical order**, with no inserted/deleted call. It maps unresolved target names by the established behavior/RVA correspondences; this is an analysis aid, not a relocation proof or a normalized match claim.

| Oracle region (seeded pseudocode) | Senior-pass source correction |
|---|---|
| Entry/fall/spring, lines 45–311 | Replace BlupiRect aggregate-return calls with the target fixed rectangles. Evaluate upper collision before state changes. Destroy helicopter/jeep/tank/skate separately on springs. Restore skateboard-specific jump speeds `-17/-13`, the `-13` injury threshold, shared collision sound call, BOOL object-start success tests, and the actual returned lift index. |
| Input, lines 455–870 | Expand SB1 state predicates; key releases exclude state flags without press-time action exclusions. Crate push requires down plus the appropriate horizontal key, with the opposite key/jump clear. |
| Vehicles, lines 871–1277 | Remove hovercraft/balloon/crushed-state blocks. Restore independent high/low motor-channel transitions, helicopter acceleration of one, and jeep up/down exit gate. Remove SB2 helicopter wall helper and jeep edge/slope acceleration. |
| Water/platform/path/timers, lines 1278–2054 | Use target rectangle dimensions and swimming/surf rectangle branch. Reuse one saved point across path retries/old-position restore. Remove AscenseurShift checks and inverse/balloon/crushed timers. Power or shield produces one common TYPE_MAGICTRACK trail. |
| Hazards/pickups, lines 2051–2670 | Remove trap/drop/saw/crusher/bridge/colored-door, creature, colored-key, and inverse pickup behavior absent from this body. Restore object detection after mockery, vehicle grace-point gates/pickup sounds, separate ammunition count, shield deletion, ordinary glue-contact expansion, treasure coordinates without SB2 tracks/door helper, team-relative personal bombs and GetIconPerso. |
| Missing network contact, immediately before crate continuation | Add NetPlayerCollide, cloud-sector test, NetStopCloud, electro state, conditional vehicle ejections/channel stops and sound. The escaped `&num` is essential to the target's allocation. |
| Death/end/scroll tail | Separate front-display flag from shield timer; use target world 99, remove the respawn x-minus-30 and scroll endpoint plus-30 additions, and send PK_LOST on falling beyond the world. |

[OPEN] Call-order agreement does not prove the remaining branch nesting, conditions, source forms, or relocation bindings. The first byte residue is instruction offset `0x15`: candidate reads the saved action into ECX, target into EDX; dependent position/vector load scheduling also differs. The final effective local map agrees, but code is still 248 bytes short through `ret`. Continue with actual instruction/basic-block comparison, especially the six residual `num` access differences and world/water/input conditions. Do not preserve count probes or manufacture declarations to change register choices.

### Private header deltas for infra

[EVIDENCE] The private header starts from this round's canonical header, which already has TimeShield/TimeFire/TimeMockery/TimeOuf/ActionOuf at `c964..c974`. Those five placements are retained. The main function's explicit calls select existing PlaySound/ObjectStart overloads; no new method declaration was added.

| Target offsets | Private member mapping | Evidence |
|---|---|---|
| `c90c`, `c910`, `c914`, `c918` | Helico, Jeep, Tank, Skate | Four spring destructions; distinct movement, firing and pickup blocks; object types 13/19/28/24. |
| `c91c`, `c920`, `c924`, `c928` | Nage, Surf, Vent, Suspend | Movement table references, water changes, vent tile test and bar/hanging tests. |
| `c92c`, `c930`, `c934`, `c938`, `c93c` | JumpAie, Shield, Power, Cloud, Hide | Jump collision, hazard immunity, powered velocities, charge and invisibility lifecycle. Shield precedes Power, unlike canonical. |
| `c940/c944`, `c948/c94c` | PosHelico, PosMagic | Thirty-pixel vehicle grace box and forty-pixel magic-trail distance. |
| `c950`, `c954`, `c958`, `c95c`, `c960` | Restart, Front, Bullet, Perso, NoBarre | Respawn gate; front flag set at win/byes and cleared at death completion; ammunition pickup/fire; team bomb stock; five-tick hanging-bar inhibition. |
| `c978` | FifoNb alone | FIFO save/reset operation is separate from NoBarre at `c960`; remove their canonical union alias. |

[EVIDENCE] Private changes remove Over, Invert, Balloon and Ecrase from this target flag region; add Front/Bullet/Perso/NoBarre before the existing timers; split the FIFO alias; and swap Shield/Power. Over/Invert/Balloon/Ecrase declarations are kept in the existing SB2 compatibility tail solely to compile untouched non-owned functions. **[OPEN] Their tail positions and the resulting whole-class size are not SB1 layout claims.** No new constants are introduced.

[EVIDENCE] `DecorDetect` explains the final verifier rejection: canonical source tests `m_blupiHelico || m_blupiSkate || m_blupiNage`, which under the old header encoded target offsets `c90c/c91c/c920`. In the proposed correct naming those are Helico/Nage/Surf; leaving that non-owned body unchanged makes it read `c90c/c918/c91c`. Infra must reconcile the header and all affected source names together. This worker did not edit that body or waive the regression.

### Compiler findings

- `[VC5-SEM-01; Blupi-specific]` Trigger: extra aggregate-return calls and frame `bc`. Fixed coordinate construction in place of BlupiRect returns, plus target-timed upper detection, changed 29,216/`bc` to 28,912/`ac`. Target evidence is direct rectangle argument arithmetic, not a smaller RECT type. Full TU/SP3 flags above; MISMATCH.
- `[VC5-SEM-01; Blupi-specific, measured allocation effect]` Trigger: one remaining frame dword and wrong index allocation. Restoring the genuinely missing network-cloud contact block, including `NetPlayerCollide(...,&num)`, changed 24,960/`a8`/prefix 2 to 26,336/`a4`/prefix 21. Address-taking moves `num` to the target fixed slot and affects code throughout the body. This is a source omission, not a prescription to add an arbitrary address-taking call. MISMATCH; all final effective slots subsequently agree.
- `[VC5-LOC-03; negative]` Saved-point reuse, declaring collision output at function scope, reusing `icon`, and assigning GetPosDecor into the saved point were individually inert for frame/size at their tested stages. A separate initialized block-local POINT increased `a8 -> ac` without reducing code; reverted. This narrows the remaining scope explanation; do not repeat those probes.
- `[BLUPI-LOC-01; negative scope test]` Single `str[100]`, block scope versus function scope: 24,960 bytes and frame `a8` in both tested builds. Its effective address remains `E-64`. No array-order family is available.
- `[VC5-CTX-01..04; negative]` At step 19, eight before-definition extern-count probes (1/2/3/4/7/8/12/16) all emitted byte-identical 25,136-byte BlupiStep bodies, frame `a4`, prefix 21. Scratch diagnostic only; no authentic declaration inferred, no STRICT gain.
- `[VC5-SEM-01; source-form effect, unverified for exact original form]` Point copy plus component increments versus explicit endpoint sums changed 25,136 to 25,120 bytes and the load/SIB schedule, while frame/prefix stayed `a4`/21. Explicit sums were reverted because the copy/increment form follows the target entry shape more closely. Separating/reordering the two snapshot declarations at step 21 was byte-inert and reverted.
- `[VC5-CTL-03; Blupi-specific source form]` Independent motor transition checks retain target rechecks after sound calls; if/else versions merged PlaySound sites. The final independent checks restore the target call sequence. This pass has no full-function STRICT result and does not claim a general compiler rule from that effect.


## Senior pass follow-up - header reconciliation and byte alignment

[EVIDENCE] This section supersedes the preceding senior pass's 248-byte deficit and next-step text. All evaluations use the required `tools/verify.py --tu src/decblupi.cpp=<candidate> --build-dir solblupi`, pinned SP3 and the manifest flags. Only owned scratch files and this document were edited. No catalog, canonical source/header, assets, toolchain or external repository was changed.

### DecorDetect layout conflict resolved

[EVIDENCE] The proposed state/inventory/FIFO layout is retained exactly as tabulated above. There is no honest alternative member permutation that makes both the BlupiStep skate access at `this+c918` and the stale DecorDetect source name `m_blupiSkate` mean the target's `this+c91c`. DecorDetect requires the source predicate `m_blupiHelico || m_blupiNage || m_blupiSurf`. In the private header these read `c90c/c91c/c920`.

[EVIDENCE] With the private layout, the unchanged predicate reads `c90c/c918/c91c`. Exactly two fixed bytes differ in DecorDetect: function offsets `+0x57` (`18 -> 1c`) and `+0x61` (`1c -> 20`), in the displacements of instructions at `+0x55/+0x5f`. Changing only the two stale names restores DecorDetect STRICT at `0xf640`, 640 body bytes, all three relocations proved. The same full verifier run keeps IsLave STRICT (`0x10150`, 88 body bytes, 96-byte extent) and IsRessort STRICT (`0x101b0`, 94 body bytes, 96-byte extent). Neither helper accesses the state fields: their relevant layout dependencies are decor base `+0x10` and dimensions/bounds. Their strictness must not be cited as proof of a different skate/swim mapping. First proof: `followup/f01-headerproof`; the later diagnostic candidates retain the three STRICT results.

[EVIDENCE] Final state delta remains: Air `c908`; Helico/Jeep/Tank/Skate `c90c/c910/c914/c918`; Nage/Surf/Vent/Suspend `c91c/c920/c924/c928`; JumpAie/Shield/Power/Cloud/Hide `c92c/c930/c934/c938/c93c`; PosHelico `c940/c944`; PosMagic `c948/c94c`; Restart/Front/Bullet/Perso/NoBarre `c950/c954/c958/c95c/c960`; timers/action `c964..c974`; FifoNb `c978`; FIFO points `c97c` onward. Remove Over/Invert/Balloon/Ecrase from that region, swap Shield/Power, add Front/Bullet/Perso/NoBarre, and split the old FifoNb/NoBarre union. Compatibility-tail positions remain unproved. No new numeric header constant was introduced.

### Measurement and iteration ledger

[EVIDENCE] `tools/asmscore.py` is used as supplied. Its oracle span includes 11 alignment bytes (25,280 versus 25,269 body bytes), and its relocation projection/difflib score is diagnostic, never STRICT evidence. `raw prefix` stops at a literal COFF relocation: after entry is fixed it reaches 149, the first call operand. `instruction prefix` is asmscore's relocation-normalized count. `fixed.py` separately locates differing non-relocated bytes; those ignored-relocation spans are not matches. Every BlupiStep row below remains MISMATCH. Frame is `0xa4` in every successful row. Objects, headers, verifier logs and metrics are archived in `scratch/solblupi/followup/<label>/`.

| Iteration / construct | COFF bytes | Raw prefix | Instruction prefix | asmscore |
|---|---:|---:|---:|---:|
| f00-baseline | 25,024 | 21 | 8 | 12,623 |
| f01-headerproof | 25,024 | 21 | 8 | 12,623 |
| f02-shared-icon3 | 25,024 | 21 | 8 | 12,623 |
| f03-shared-icon4 | 25,024 | 21 | 8 | 12,623 |
| f04-shared-scroll | 25,024 | 21 | 8 | 12,623 |
| f05-grid-point | 25,024 | 21 | 8 | 9,683 |
| f06-grid-scalars | 25,024 | 21 | 8 | 12,623 |
| f07-grid-yfirst | 25,024 | 21 | 8 | 12,535 |
| f08-end-assign | 25,024 | 21 | 8 | 9,683 |
| f09-spring-if | 25,040 | 21 | 8 | 9,699 |
| f10-jump-if | 25,088 | 21 | 8 | 9,781 |
| f11-snapshot-order | 25,088 | 16 | 7 | 9,789 |
| f12-grid-localpoint | 25,088 | 21 | 8 | 9,945 |
| f13-shared-savedpoint | 25,088 | 21 | 8 | 9,945 |
| f14-shared-destpoint | 25,088 | 21 | 8 | 9,781 |
| f15-sucette-phase | 25,072 | 21 | 8 | 9,705 |
| f16-grid-sharedints | 25,072 | 21 | 8 | 10,029 |
| f17-injury | 25,088 | 21 | 8 | 9,581 |
| f18-surfstop | 25,120 | 21 | 8 | 8,379 |
| f19-exclusive | 25,200 | 21 | 8 | 9,505 |
| f20-emptyammo | 25,232 | 21 | 8 | 8,949 |
| f21-sharedcrate | 25,232 | 21 | 8 | 8,577 |
| f22-walkguards | 25,248 | 21 | 8 | 8,697 |
| f23-pickups | 25,296 | 21 | 8 | 7,983 |
| f24-shared-netindex | 25,328 | 21 | 8 | 7,961 |
| f25-stopguards | 25,312 | 21 | 8 | 7,499 |
| f26-mask-bomb-path | 25,344 | 21 | 8 | 13,719 |
| f27-idleform | 25,344 | 21 | 8 | 8,039 |
| f28-hazards | 25,312 | 21 | 8 | 5,839 |
| f30-shared-count | 25,312 | 21 | 8 | 5,839 |
| f31-grid-scalars-yfirst | 25,312 | 21 | 8 | 5,839 |
| f29b-shared-collision | 25,312 | 21 | 8 | 5,839 |
| f31b-grid-scalars | 25,312 | 21 | 8 | 6,851 |
| f32-charge-front | 25,296 | 21 | 8 | 5,833 |
| f33-electro | 25,296 | 21 | 8 | 5,791 |
| f34-swimform | 25,280 | 21 | 8 | 5,553 |
| f35-firstmask | 25,280 | 21 | 8 | 5,293 |
| f36b-independent-idle | 25,280 | 21 | 8 | 3,321 |
| f37-getbarre-prototype | 25,280 | 21 | 8 | 2,959 |
| f38-helicopter-form | 25,280 | 21 | 8 | 559 |
| f39-semantic | 25,280 | 21 | 8 | 395 |
| f40-endfirst | 25,280 | 149 | 2050 | 343 |
| f41-endafteraction | 25,280 | 21 | 8 | 395 |
| f42-action-later | 25,280 | 21 | 8 | 395 |
| f43-heli-exit | 25,280 | 149 | 2992 | 331 |
| f44-bar-storeorder | 25,280 | 149 | 3563 | 319 |
| f45-edge-order | 25,280 | 149 | 3554 | 303 |
| f46-edge-second | 25,280 | 149 | 4611 | 279 |
| f47-mockery-order | 25,280 | 149 | 5812 | 271 |
| f48-netpoints | 25,280 | 149 | 5812 | 271 |
| f49-netshared | 25,280 | 149 | 5812 | 271 |
| f50-netlocals | 25,280 | 149 | 5812 | 271 |
| f51-packet-width | 25,280 | 149 | 5812 | 49 |
| f52-fifo-shield | 25,280 | 149 | 5812 | 45 |
| f53-netvehicle-points | 25,296 | 41 | 11 | 1,117 |
| f54-netheli-yfirst | 25,280 | 149 | 5812 | 45 |
| f55-default-prototypes | 25,280 | 149 | 5812 | 45 |
| f56-ctx-top-32 | 25,280 | 149 | 5812 | 45 |
| f56-ctx-top-64 | 25,280 | 149 | 5812 | 45 |
| f56-ctx-before-32 | 25,280 | 149 | 5812 | 45 |
| f56-ctx-before-64 | 25,280 | 149 | 5812 | 45 |
| f56-ctx-end-32 | 25,280 | 149 | 5812 | 45 |
| f56-ctx-end-64 | 25,280 | 149 | 5812 | 45 |
| f57-body-only-final | 25,280 | 149 | 2920 | 2,885 |
| f58-layout-final | 25,280 | 149 | 2920 | 2,885 |

[EVIDENCE] The f29 and f36 intermediate files had transcription compile errors, corrected in f29b/f36b; they provide no compiler evidence. f31's intended substitution did not apply (a no-op); the actual scalar-coordinate test is f31b. Rejected variants are retained only as diagnostic evidence.

### Structural reconciliation in this follow-up

[EVIDENCE] Call order alone hid real differences. The retained body now restores: explicit spring/jump if/else stores; severe injury `<= -14` with branch-local sounds (the compiler shares the call opcode); the first lift top at `end.y+25`; SUCETTE phase 36; exclusive left/right movement; two independent swim/surf idle-stop predicates; missing empty personal-bomb-stock action/sound; team-relative bomb types; separate rectangle construction in both swimming/path arms; power/drink eligibility checks; skateboard turn phase 7, no-input phase 130 and up/down exit; hanging-bar input mask/store order; vehicle edge rectangle dimensions; surf sound 25 and upward water probe 60 pixels; bullet destination x=450; mockery flag order; GLU completion phase 90; electro state 57; and FIFO immunity testing Shield rather than JumpAie.

[EVIDENCE] The multiplayer count loop uses the same escaped `num` as its loop index; its following win sound reads `m_moveObject[num].posCurrent` (num reaches 4), not the player's position. This surprising source behavior is required by the oracle's fixed `esp+18` index reload and `+9c6c/+9c70` reads. The old unconditional death-completion Front reset has no target store and was removed. Lava clears Air and snaps y before gauges, with Restart true.

### Additional prototype evidence and integration boundary

[EVIDENCE] GetTypeBarre at `0x10050` returns full EAX: `mov eax,1` at `0x100e3`, `mov eax,2` at `0x10100`. Its BlupiStep callers compare EAX at `BlupiStep+2bce/+417f` vicinity, whereas canonical `char GetTypeBarre(POINT)` introduces `movsx eax,al` at the first caller and a byte comparison at the second. Diagnostic declaration AND definition return type `int` remove those width differences. The callee body itself remains MISMATCH and no new strict callee is claimed.

[EVIDENCE] The life-count packet argument at `BlupiStep+5de7` loads full ECX from `this+ce9c`; the canonical `NetSendBarePacket(BYTE,short)` emits `mov cx,word ptr [...]`, one byte longer. Private `NetSendBarePacket(BYTE,int)` restores that load and the exact 25,269-byte body length. [OPEN] This changes the decorated symbol from `...EF@Z` to `...EH@Z`; the owning definition in decmove.cpp and all declarations/callers require coordinated integration and relocation proof. The diagnostic run's overall verifier exit 0 means no accepted regression; BlupiStep remains MISMATCH and its new packet binding is not proved. No canonical prototype was changed.

[EVIDENCE] To respect BlupiStep-only source ownership, `scratch/solblupi/decblupi.cpp` is the body-only deliverable, with its surrounding canonical text preserved. The stronger diagnostic candidates under `argwidth/` additionally change only the DecorDetect predicate and GetTypeBarre definition signature, using a separate private header with the two prototype experiments. Do not promote one entire diagnostic TU without these explicit integration edits.

### Compiler findings

- `[VC5-LOC-03 / BLUPI-REG-01; confirmed source-order effect, BlupiStep still unverified]` Trigger: saved action ECX versus target EDX at entry, plus early index/call register differences. Moving the real `POINT end = m_blupiPos` initializer before the saved-state locals (f40), with identical behavior and no dummy declaration, fixes all early residues through `+1f21`. Raw prefix 21 -> 149; normalized instruction prefix 8 -> 2,050; score 395 -> 343. Moving it only after the saved action or moving only the saved-action initialization later is inert (f41/f42). SP3/full TU/header-prototype context as archived. This is actual initializer order, not a local-name trick.
- `[VC5-ORD-01 / VC5-CTL-03; Blupi-specific source form]` Trigger: common sound temporary makes injury branch four bytes short; ternary velocity stores use neg/sbb arithmetic where target branches. Branch-local literal sound calls tail-share one call opcode; explicit velocity if/else stores restore target branches. Inverting the outer up/jump test in swimming/surfing and helicopter control restores target down/neutral/upper arm placement. Most notably f37 -> f38: score 2,959 -> 559, frame and extent unchanged. Full function remains MISMATCH.
- `[VC5-ORD-01; Blupi-specific]` Trigger: Focus store precedes vehicle-position copy, hanging-bar stores reorder, and second edge predicate branches differ. StopBlupi() plus the genuine later Focus=true assignment, target-timed bar fields, and second edge predicate `bIsOpen2 && !bIsOpen` advance normalized prefixes 2,050 -> 2,992 -> 3,563 -> 4,611. Swapping BOTH edge predicates regresses the first difference; only the second swap is retained. Flags/context unchanged; MISMATCH.
- `[VC5-SEM-01; Blupi-specific]` Fixed constants, offsets, masks, inventory source omissions and prototype widths were corrected from oracle instructions, not treated as allocator noise. Independent idle-stop blocks with the actual full mask restore target common-value reuse; putting the swim stop inside the vehicle idle outer block does not. No individual semantic correction is promoted to a compiler rule.
- `[VC5-LOC-03 / BLUPI-REG-01; negative local-reuse families]` Sharing icon with num3/num4/scroll scratch/OUF4 result/count is inert at the tested stages; grid POINT versus actual scalar locals is worse; shared saved/destination points do not improve entry. At the ten-byte network residue, explicit components, y-first heli components, separate block POINTs, and existing tinyPoint5 reuse are inert; caching direct jeep/tank position arguments in tinyPoint regresses entry to 41 and size to 25,296. No artificial construct was retained.
- `[VC5-CTX-01..04; negative declaration experiment]` Replacing the two hack overload declarations by ordinary default arguments is byte-inert for BlupiStep (f55); it is a diagnostic header experiment, not an integration recommendation. Wide top/before/end count probes are recorded separately below and are never claimed source.


### Final preserved artifacts and remaining work

[EVIDENCE] Strongest diagnostic: `scratch/solblupi/argwidth/f52-fifo-shield.cpp`, private `argwidth/decor.h`, archived full verification in `followup/f52-fifo-shield`. BlupiStep body is **25,269 bytes**, COFF extent 25,280, frame `a4`, raw prefix **149**, normalized instruction prefix **5,812**, asmscore **45** (22 instruction edits; 6,505 of 6,516 normalized instructions match). `fixed.py` finds exactly **ten** differing non-relocated bytes, first at `+58c8`, in ten instructions at `+58c7/+58d2`, `+58e2/+58e8/+58f2/+58f3`, and `+5959/+595f/+5966/+5967`. They are register encodings in jeep ObjectStart, tank ObjectStart and electro PlaySound; all literal offsets, conditions and body length now agree. Every target/candidate call remains in the same 346-call order and at the same relative position when packet declaration names are mapped by behavior. That mapping does not prove the new packet symbol binding.

[EVIDENCE] Six wide count probes (32/64 declarations at top, immediately before BlupiStep, and TU end) are **byte-identical** to f52, not merely equal scores. No context probe or overload-removal experiment is retained as proposed source. The tested local-identity and context families have diminishing returns for the ten-byte residue.

[EVIDENCE] Ownership-preserving deliverable: `scratch/solblupi/decblupi.cpp` (f57) has only BlupiStep changed relative to current canonical text. With the still-canonical char/short prototypes its body is 25,272 bytes, frame a4, raw prefix 149, score 2,885; those prototype residues explain why this version is weaker than f52. Its unchanged DecorDetect retains the known two-byte regression under the proposed layout. `scratch/solblupi/header-proof.cpp` (f58) changes only the two additional DecorDetect member names; full verifier exit 0 and all three peers STRICT reproduce the final layout solution independently of the prototype experiments. Last worker build is f58. No BlupiStep bytes are credited as STRICT.

[OPEN] Next integration: retain the state/inventory/FIFO layout and correct DecorDetect to Helico/Nage/Surf; coordinate GetTypeBarre's int return declaration/definition and NetSendBarePacket's int argument declaration/definition across owning TUs; then continue from the ten register bytes in f52 and prove every relocation, especially the new decorated callee names. GetTypeBarre itself and the new NetSendBarePacket signature are not certified STRICT by this worker. No background worker processes remain.
# Senior pass: post-restart closure and combined integration audit

[EVIDENCE] All resumed verification used `--build-dir solblupi`, with
`BLUPI_JOBS=4` and one sequential campaign at a time. The interrupted evaluations
from the earlier host failure were not counted as results. The winning source
is `scratch/solblupi/close/r04-sound-copy-init.cpp`; the coordinated payload is
`scratch/solblupi/package/decblupi.cpp`. Package instructions and the exact
integration file list are in `scratch/solblupi/package/README.md`.

[EVIDENCE] **BlupiStep is now STRICT in the combined 20-object comparison:**
RVA `0x9280`, **25,269 unique body bytes**, frame `0xa4`, compared extent 25,280
(11 boundary NOPs), **all 350 relocations verified**. The result was read from
`build/verify-solblupi/results.json` after the private-header combined run.
The complete-run summary is retained in `package/verify-button-both-aggregate.log`.
Standalone asmscore on the winning one object is 1 / RELOC-UNVERIFIED, zero
instruction edits, normalized instruction prefix 6,516: only four external
walking/swimming/surfing table relocations lack definitions in that one-object
invocation. The combined run includes `dectables.obj` and proves all four.
Raw prefix remains 149 because the first call's unlinked relocation begins
there; it is not a remaining instruction mismatch.

| Resumed iteration | Frame | COFF bytes | Raw prefix | Normalized instruction prefix | asmscore |
|---|---:|---:|---:|---:|---:|
| r01-index-player | a4 | 25280 | 149 | 5812 | 45 |
| r02-index-num3 / index-num4 / index-icon | a4 | 25280 | 149 | 5812 | 45 |
| r02-position-ref / position-pointer | a4 | 25280 | 149 | 5812 | 45 |
| r02-end-reuse | a4 | 25312 | 149 | 5756 | 967 |
| r02-aggregate-points | a4 | 25280 | 149 | 5860 | 17 |
| r03-sound-xy / sound-scope / sound-tinypoint5 | a4 | 25280 | 149 | 5860 | 17 |
| r03-sound-yx | a4 | 25280 | 149 | 5859 | 21 |
| r03-sound-end | a4 | 25280 | 149 | 5756 | 797 |
| **r04-sound-copy-init** | **a4** | **25280** | **149** | **6516** | **1 standalone; STRICT combined** |
| r04-sound-tiny-xy / sound-const / sound-scalars | a4 | 25280 | 149 | 5860 | 17 |
| r04-sound-tiny-yx | a4 | 25280 | 149 | 5859 | 21 |

[EVIDENCE] Exact former ten-byte map (ours -> target): `+58c8 96->86`,
`+58d2 52->50` (jeep x EDX->EAX); `+58e3 86->8e`, `+58e9 8e->86`,
`+58f2 50->51`, `+58f3 51->50` (tank y EAX->ECX, x ECX->EAX);
`+595a 96->8e`, `+5960 86->96`, `+5966 52->51`, `+5967 50->52`
(electro sound y EDX->ECX, x EAX->EDX). Three branch-local aggregate POINT
initializers fixed eight. Replacing ONLY the sound initializer by
`POINT pos = m_blupiPos` fixed the final two at `+5960/+5967`.

## Compiler findings

- [BLUPI-REG-02 / VC5-LOC-03; blupi-specific source-form effect in this
  aggregate argument pattern] SP3 complete decblupi TU, canonical flags `/W3 /GX /O2
  /ML /Gd /Gy`, private corrected layout/prototypes. Direct member POINT call
  arguments versus branch-local `POINT pos = {m_blupiPos.x,m_blupiPos.y}`
  produce the same loads and pushes but different physical registers.
  Aggregate initialization at jeep/tank/sound reduces ten fixed bytes to two,
  score 45 -> 17, without frame/body/branch/call changes. The final sound's
  actual POINT copy initialization instead of aggregate construction fixes
  the remaining x-register identity: all fixed bytes equal, combined STRICT.
  This is source aggregate/local identity evidence; it does not make arbitrary
  scalar register residues instances of the shared-index SIB rule.
- [VC5-ORD-01 / BLUPI-REG-01; negative] Explicit sound x/y component stores,
  extra lexical scope, existing tinyPoint5 reuse, const POINT and scalar
  initializer intermediates retain the two-byte residue. y-before-x regresses
  to score 21. Existing end reuse changes the use graph and regresses scores
  to 797/967 (the wider family adds 32 extent bytes). No variant retained.
- [BLUPI-REG-02; negative] Player-collision index identities num3/num4/icon
  and point reference/pointer aliases are byte-inert at the ten-byte checkpoint.
  The previously measured shared-index rule does not predict these scalar
  member-to-call register choices by itself.
- [BLUPI-CTX-01 / VC5-CTX-01..04; new global integration limitation] Applying
  the private decor.h to every TU (rather than just the changed CDecor TUs)
  regresses accepted CButton::MouseMove to five load/register/SIB instruction
  residues. Natural arithmetic association/subtraction forms, block/root
  scalar caches, snapshots and const qualifiers, branch/return forms,
  point references/copies and genuine include-order permutations did not
  restore STRICT. Removing the five unused duplicate m_sb2 timer declarations
  also did not restore it; that diagnostic header was reverted. No filler,
  asm, volatile access or accepted-function withdrawal is proposed. Two late
  grouped-header declaration controls stalled without a completed result and
  are not compiler evidence.

## Integration package and remaining gate

[EVIDENCE] `package/README.md` enumerates exactly six proposed canonical files:
`src/decblupi.cpp`, `src/decor.h`, `src/decmove.cpp`, `src/decblock.cpp`,
`src/decor.cpp`, and `recon.json`. Header offsets/evidence and the existing
unproved compatibility tail are explicitly recorded there. Duplicate
GetTypeBarre definitions in BOTH decblupi and decblock must change to int.
The NetSendBarePacket definition's second argument changes to int while its
serialized packet field remains short; the new `...EH@Z` callee is STRICT at
`0x10a60`, 48 bytes. Migrate only the accepted old `...EF@Z` symbol name at
the same RVA/size; do not withdraw the function. The canonical manifest has
322 entries, one already withdrawn; all 321 active entries were checked.

[EVIDENCE] Corrected `DecorDetect` tests Helico/Nage/Surf at c90c/c91c/c920,
STRICT `0xf640` / 640 body bytes. `IsLave` and `IsRessort` in decblupi are
STRICT at `0x10150` / 88 and `0x101b0` / 94 body bytes (96-byte extents each).
Under the new header, VoyageStep must increment Perso at c95c rather than the
stale PosMagic.y name. CurrentWrite RHS names must follow the original physical
offsets, retaining the inherited DescGameFile LHS names; both functions are
STRICT again at `0x13ef0` / 352 and `0x14da0` / 1584 compared bytes.

[OPEN] The raw full gate reports MouseMove plus the expected old packet-name
lookup failure. The single proved packet-name migration leaves one actual
accepted regression: MouseMove `0x2fd0` / 201 body bytes, 208-byte extent.
The package is therefore reviewable but **not an all-accepted passing
integration** yet. Preserve the strict BlupiStep body and solve authentic shared
header/declaration context for MouseMove; do not hide the regression using a
per-TU header or dummy declarations.

[OPEN] The last sequential two-header control and subsequently unrelated
read-only shell probes stopped responding. No new compiles were submitted
after detecting that stall. The polling channel still worked, but Ctrl-C did
not confirm termination of session 12441. Host-side process cleanup and the
stalled control's completion remain unconfirmed; incomplete results are not
matches. This is separate from the preserved completed STRICT proof above.

### Final clean payload verification

[EVIDENCE] The interrupted session subsequently ended. The ordinary command
broker's shell probes remained unresponsive, but the available Node command
channel worked; a fresh exact `tools/verify.py --tu ... --build-dir solblupi`
20-TU build completed there with `BLUPI_JOBS=4`. No further header experiments
were mixed into that run. The final payload is under
`scratch/solblupi/package/payload/`; `integration.patch` and `inventory.json`
record exactly the six proposed canonical changes. `finalize.py` reproducibly
assembles the candidate, performs the complete verifier CLI, and audits the
unchanged and one-name-migrated accepted gates. `evidence/results.json` archives
the full comparison; `audit.json` records **STRICT**, 25,269 body / 25,280
compared bytes, frame a4, zero fixed differences and **350/350 verified
relocations** for BlupiStep. All 321 active accepted entries are checked:
the raw gate has MouseMove and the old packet-name lookup; after the single
proved packet-name migration, **320 pass and MouseMove alone fails**.

[EVIDENCE] Process checks found no cl/c1xx/c2/Python worker remaining after the
interrupted campaign. The fresh final verification has also completed; no
background compiler campaign is intentionally left running. The command-channel
incident is resolved for package production and does not weaken the recorded
STRICT proof. The sole unresolved integration issue is the authentic shared
header context needed to keep accepted MouseMove STRICT.

[EVIDENCE] `git apply --check scratch/solblupi/package/integration.patch`
passes against the current canonical files. A final process check after the
clean build found no cl/c1xx/c2/Python worker. No canonical source, manifest,
compiler catalog or validation document was edited by this worker.

