# decdesign structural reconstruction (sol-decdesign, 2026-10-17)

## Review package and result

[EVIDENCE] Candidate: `scratch/sol-decdesign/decdesign.cpp`, with private
`scratch/sol-decdesign/dectables.h`. Only PlaceDecor4..7 bodies change;
CheatAction and BlupiSearchIcon definitions are added in their oracle TU positions.
Other existing bodies, including the accepted functions, retain their source text.
Canonical source, headers, manifest, compiler catalog, assets, reference repos and
toolchains were not edited. The existing inaccurate BlupiSearchIcon draft in
`src/decblupi.cpp` remains untouched; integration must remove that duplicate.

```powershell
.pyenv\python.exe tools\verify.py --tu src/decdesign.cpp=scratch/sol-decdesign/decdesign.cpp --build-dir sol-decdesign
```

[EVIDENCE] Final full-manifest verification exits **0**. Both newly STRICT bodies
and all active accepted entries survive. Evidence: `final-verify.log`,
`final-verify-exit.txt`, `final/results.json`, `final/score.json` and `audit.json`
under the candidate directory; the live report is
`build/verify-sol-decdesign/results.json`. Compiles use tools/verify.py and its
tools/msvc.py wrapper, pinned SP3 `/W3 /GX /O2 /ML /Gd /Gy`, default jobs/slots,
and repository-local TMP/TEMP. Shell campaigns were sequential.

| Final name | RVA | Oracle body | Candidate body / extent | Score | Fixed prefix | Full-run result |
|---|---:|---:|---:|---:|---:|---|
| PlaceDecor4 | `6530` | 3,619 | 3,619 / 3,632 | 1 | 3,632 | RELOC-UNVERIFIED, 41/52 relocations proved |
| PlaceDecor5 | `7360` | 1,980 | 1,980 / 1,984 | 0 | 1,984 | **STRICT, 17/17 relocations proved** |
| PlaceDecor6 | `7b20` | 2,077 | 2,075 / 2,080 | 471 | 27 | MISMATCH |
| PlaceDecor7 | `8340` | 478 | 478 / 480 | 1 | 480 | RELOC-UNVERIFIED, 5/7 relocations proved |
| CheatAction | `86a0` | 1,686 | 1,686 / 1,696 | 0 | 1,696 | **STRICT, 40/40 relocations proved** |
| BlupiSearchIcon | `8e80` | 1,020 | 1,020 / 1,024 | 1 | 1,024 | RELOC-UNVERIFIED, 3/10 relocations proved |

[EVIDENCE] **Two new STRICT functions, 3,666 unique original code bytes.** Three
additional functions have zero non-relocation differences, representing 5,117
original body bytes, but receive **no STRICT coverage credit**. Extents include
terminal alignment NOPs, which receive no code-byte credit. Scores come from
tools/asmscore.py and are diagnostics; the full verifier supplies relocation proof.
The snapshot utility's early `body_size` fields incorrectly included padding;
final/score.json corrects them by identifying the terminal instruction.

## Function identities and regions

[EVIDENCE] Earlier ENGINE_PROBE behavior labels used SB2's PlaceDecor6 for lifts,
PlaceDecor7 for treasure/starts, and PlaceDecor5 for bombs. They were hypotheses.
The original editor caller in `build/decomp/tu/event.c` dispatches menu IDs
`418/419/41a/41b` to `6530/7360/7b20/8340` respectively. Current source constants
WM_DECOR5..8 are WM_USER+24..27; their source caller invokes PlaceDecor4..7.
The final candidate therefore uses those names in contiguous source order.
Renumbering/reordering preserved every measured target body byte and status.
Earlier scratch trial names refer to the old behavior labels, not final names.

[EVIDENCE] PlaceDecor4: reject !bMix before grid bounds; select the palette by
iconLift (0..2 default; 3..9 and 21..24/27 overrides); edit an existing endpoint
before the creation branches; cycle enemy channels or palette icons at the start;
scale travel duration by the larger axis distance and synchronize the lift.
Creation indices 0..3 build four lift timings; 4/5 cycle crate palettes; 6..11
create bulldozer, fish, bird, moving bomb, helicopter enemy and tank enemy.
The SB2-only wasp/slime/conveyor actions and extra palette selections are absent.
All 52 relocation sites and their instruction positions agree.

[EVIDENCE] PlaceDecor5: indices 0..6 create treasure, egg, power, shield, drink,
bullet and charger with target coordinates/channels. Indices 7..10 update the four
start positions/directions; modifier bit 8 copies start zero to the other three.
Index 11 deletes old goal/key records before creating the new goal. AdaptBorder
is the final call. Unsupported personal-bomb/dynamite paths from SB2 are removed.

[EVIDENCE] PlaceDecor6: indices 0/1 create bombs; 2..5 edit count-prefixed tile
palettes; 6/7 store 8a/d3; 8 cycles personal bomb types 200..203; 9..12 create
helicopter, skateboard, jeep and tank. Personal bombs default to channel **2**,
then types 201..203 explicitly select 11..13. Target skateboard y is tile y+5.
Call order and creation fields are reconstructed, but register choices and
instruction scheduling remain different. In particular, the candidate loads
the menu index into EAX and preserves incoming ECX; the target loads it into ECX
and needs an additional `mov ecx,esi` before the first delete call. Candidate
posX/posY calculation order and table-return reload scheduling also differ.

[EVIDENCE] PlaceDecor7: two tile palettes, then a unique key/goal record. The
target deletes both pre-existing type-7 and type-21 records before allocation,
creates type 21 at tile+(2,4), and calls AdaptBorder. All fixed bytes agree.

[EVIDENCE] CheatAction: cases 2, 6..10 and 12..18, in independent-if source order.
Case 2 converts eight enemy types to explosions, adjusts the current position by
-34 per axis, copies it to both endpoints, updates animation and plays sound.
Case 10 takes the successful-goal arm first; the insufficient-treasure arm follows.
Shield uses **m_jauges[1] at cd60**, not gauge zero. The state resets precede the
real POINT copy into m_blupiPosMagic. The final gauge/sound cleanup matches.

[EVIDENCE] BlupiSearchIcon: Vent/Helico, Jeep, Tank, Skate, Nage, Surf and Suspend
remappings follow the recovered layout. SB2 balloon/flattened/hovercraft paths
are absent. The table cursor starts at zero at function entry. Animation-limit
selection uses branch-local assignments, followed by the final table lookup.
The default sprite channel is 2; left mirroring is limited to icons 0..287;
the final special-channel adjustment rechecks direction. All fixed bytes agree.

## Complete stack-frame map

[EVIDENCE] Let E be entry ESP. `evidence.py` propagates exact stack heights over
every reachable oracle instruction, including callee cleanup. Every CFG join
agrees. The `stack-<rva>.json` and `.txt` files list **every explicit ESP-relative
operand**, its instruction, live stack height and normalized entry-relative slot.
All instructions in each oracle body are reached by this analysis.

| Body | Allocated locals / saved registers | Explicit normalized slots and roles |
|---|---|---|
| `6530` | 0 / 16 bytes | E+4: cell x, then rank and crate icon; E+8: cell y; E+c: menu index; E+14: bMix, then palette pointer / cached y |
| `7360` | 0 / 16 bytes | E+4/+8: cell x/y; E+c: menu index; E+10: modifier input; E+14: bMix, then control-bit and three-copy counter |
| `7b20` | 4 / 16 bytes | E-4: tile pointer spill, later posY; E+4: cell x, then address-taken int icon; E+8: cell y; E+c: menu index; E+10: flags, then posX; E+14: bMix, then personal-bomb type |
| `8340` | 0 / 16 bytes | E+4/+8: cell x/y; E+c: menu index; E+10: flags; E+14: bMix copied to EBP, then address-taken int icon |
| `86a0` | 4 / 16 bytes | E-4: 200-record loop counter in case 10; E+4: cheat selector |
| `8e80` | 0 / 16 bytes | No explicit ESP-relative memory operands; cursor/action/icon/state are register-resident |

[EVIDENCE] The final candidate agrees with each allocation/saved-register size.
The earlier bomb POINT temporary required eight local bytes; scalar coordinates
recover the target's single four-byte slot. Actual POINT endpoint copies remain
aggregate assignments, while initial position components use direct stores.

## Header and data handoff

[EVIDENCE] Private dectables.h differs from canonical by exactly six declarations,
all `extern int name[]`, inserted before its existing endif. No struct, constant,
member, method signature, packing or inline-body change is proposed.

| New declaration | Referenced data RVA | Typed payload |
|---|---:|---|
| table_design_lava | `2ee70` | `{1,68}` |
| table_design_marine | `2ee78` | `{2,91,92}` |
| table_design_fan | `2ee88` | `{4,126,129,132,135}` |
| table_design_wind | `2eea0` | `{4,110,114,118,122}` |
| table_design_key | `2efc0` | `{2,182,183}` |
| table_design_door | `2ef78` | `{17,158,159,160,161,162,163,164,165,174,175,176,177,178,179,180,181,184}` |

[EVIDENCE] These are placement palettes with a leading element count, not the
animation sequences table_36630, table_decor_ventillog or table_decor_ventg.
Using those animation symbols would give incorrect relocations/data contracts.
The unused canonical lava placement reference also conflicts with Build's use of
table_decor_lave as a five-frame animation, so the candidate uses distinct names.

[EVIDENCE] `recovered-tables.cpp` and `.json` propose **11 typed integer arrays**
for the data owner; they were not compiled into the candidate or canonical tree.
The six above plus existing declarations need definitions as follows:

- table_36980 at `2f0b0`: 21 integers, count 20 then icons 48..67.
- table_369c0 at `2f160`: `{3,32,33,34}`.
- table_369c8 at `2f170`: `{9,2,4,79,84,48,50,143,219,230}`.
- table_blupi at `2f198`: 2,320 integers, 64 action records and final zero at
  `315d4`; each record is action/frame-count/limit followed by that many icons.
  `animation-records.json` records the parse boundaries.
- table_mirror at `315d8`: 288 integers, bounded by the target's mirroring test.

[EVIDENCE] Each proposal includes its RVA, element count and payload SHA-256.
The 64 animation records end exactly one word before the mirror table. Values
are declarative game data recovered from indexed consumers, not copied code bytes.
No STRICT claim is made for these unintegrated definitions.

[EVIDENCE] Equal payloads do not prove shared object identity. Reusing canonical
table_36548 for the identical 20-icon palette produced conflicting global bindings
and regressed accepted PlaceDecor3: its established data address differs from
`2f0b0`. This experiment was reverted; table_36980 retains its distinct identity.

## Compiler findings

- [EVIDENCE; `VC5-SEM-01`, `VC5-ORD-01`; blupi-specific] Direct indexed
  MoveObject member stores, component coordinate assignments and true POINT
  copies recover the repeated target access pattern. With correct SB1 constants,
  palettes and missing regions, the lift body moves 1,280 -> 3,632 extent bytes,
  score 4,615 -> 1 (relocation-unverified); treasure/start body moves 1,376 ->
  1,984, score 2,131 -> 0 and STRICT. These are measured source corrections,
  not a universal pointer-versus-index compiler rule.
- [EVIDENCE; `VC5-ORD-01`; blupi-specific] Treasure start-position comparisons
  originally emitted memory-first CMPs. Writing `pos.x == member.x` and the
  corresponding y comparison emits the target register-first CMPs. Score 9 -> 0;
  all 17 full-run relocations verify. Compiler/flags and complete TU unchanged.
- [EVIDENCE; `VC5-ORD-01`, `VC5-CTL-03`; blupi-specific] CheatAction's successful
  goal arm first changes score 183 -> 13. Gauge index 1 fixes the real cd60
  displacement; component shield copies give score 5. A POINT copy **after**
  the state/timer stores yields target scheduling and STRICT, 40/40 relocations.
  Component copies before the resets score 9; aggregate before the timer scores 5;
  both alternatives were rejected. No padding or context probes were retained.
- [EVIDENCE; `BLUPI-REG-01`, `VC5-CTL-03`; blupi-specific fixed-byte result]
  Animation limit loaded into a local and then conditionally replaced leaves
  EDX live through the remainder join; the target joins in EAX with a two-byte
  transfer. Branch-local assignments from the original table entries recover
  that join and improve score 107 -> 1 with zero fixed differences. A distinct
  frame local was inert (107); computing modulo before the limit test worsened
  the score to 123. Final result is RELOC-UNVERIFIED, not STRICT.
- [EVIDENCE; `BLUPI-LOC-01`, `VC5-LOC-03`; blupi-specific allocation result]
  Bomb creation with function-scope POINT coordinates needs an eight-byte local
  frame. Scalar posX/posY permit reuse of dead incoming argument slots and give
  the correct four-byte frame, extent 2,080 and score 471. POINT variants can
  lower the heuristic score to 331 while restoring the wrong frame; rejected.
  This concerns scalar spills/aggregate identity, not a new array-layout rule.
- [EVIDENCE; `VC5-CTX-01..04`, `BLUPI-REG-02`; negative at this checkpoint]
  Sixteen sequential before-bomb-function extern-count probes, counts 0..15,
  leave extent 2,080, score 471 and instruction prefix 10 unchanged; every
  accepted gate passes. These scratch declarations are absent from the candidate.
  Block-local indices, separate search/free indices and a separate next-type
  local are also inert. Scope changes alone and split coordinate scaling are
  inert; copying/scaling a POINT gives extent 2,096, score 577 and wrong frame.
  This does not rule out other TU/backend context. The malformed `bomb-share`
  scratch experiment changed a member name and is invalid evidence, not a rule.

## Remaining work

[OPEN] Integrate the proven bodies and six declarations through the supervisor;
the data owner should review the 11 table definitions, then rerun the complete
verifier. This can establish or reject full relocation proofs for 5,117 currently
uncredited body bytes. Do not promote their score-1 results as STRICT today.

[OPEN] PlaceDecor6 still needs target-supported source/context work: correct
four-byte frame and call/field regions, two body bytes short, first fixed mismatch
at +1b. Stop the current local-form campaign at diminishing returns; continue
from the delivered scalar-coordinate body rather than the lower-score POINT
variants. Canonical PlaceDecor1..3 and the remaining tiny editor accessors were
not rewritten. No background process is left running and no commit was made.
