# sol-event: event handler continuation

**Current status: partial (round 2).** [EVIDENCE] The new deliverable is
`scratch/sol-event/integration2/strict.patch`: **4 newly STRICT functions,
1,506 unique original code bytes**, no header deltas, and all **382** active
accepted entries preserved. `ChangePhase` remains MISMATCH. Full round-2
evidence and compiler findings are appended below. The first-round record
that follows is historical; its 13 gains were integrated in `18b6b58`.

**Round 1: integrated.** [EVIDENCE] The integration candidate adds 13 STRICT
functions, covering **2,882 unique original instruction bytes**, with no loss
of the baseline's 69 STRICT event symbols. `ChangePhase` remains unfinished.
No canonical source, manifest, comparator, compiler catalog, asset, reference
repository, or toolchain was modified. No game executable was run.

## Deliverable and verification

[EVIDENCE] Apply `scratch/sol-event/integration/strict.patch` for the matched
functions and the single declaration correction below. It excludes the
unfinished `ChangePhase` work and all unsuccessful controls. Both
`git apply --check` against the canonical tree and application to a scratch
copy succeed. The applied files equal the compiled delivery byte-for-byte.
The patch and source retain CP1252/CRLF.

[EVIDENCE] The compiled snapshot is `scratch/sol-event/integration/src/`.
All 20 manifest TUs were substituted together, using that one copied header
set, VC5 SP3 11.00.7022, `/W3 /GX /O2 /ML /Gd /Gy`, and the manifest defines.
The exact command is `integration/verify_command.json`; outputs are
`integration/verify.log`, `results.json`, `objects/`, and
`verification_summary.json`. Every compile used `verify.py --build-dir
solevent`; commands ran serially and have finished.

[EVIDENCE] The verifier exits zero; all **361 active accepted entries** retain
their required status, RVA, and section size. The event report is **82 STRICT,
1 RELOC-UNVERIFIED, 26 MISMATCH**. Baseline was 69/1/39. The withdrawn
`MoveObjectSort` entry remains outside the gate. These are comparator symbol
counts, including header and alias bodies, not a claim of 82 unique functions.
`DrawButtons` and `TreatEventBase` are byte-identical source bodies in the
canonical TU, working candidate, and delivery.

| Newly STRICT function | RVA | Original code bytes | COFF section bytes | Proven relocations |
|---|---:|---:|---:|---:|
| ReadInput | `0x16710` | 479 | 480 | 6 |
| NetAdjustLobbyButtons | `0x16a70` | 494 | 496 | 17 |
| NetStartPlay | `0x16d60` | 230 | 240 | 12 |
| GetWorldGroup | `0x19200` | 71 | 80 | 0 |
| CheckWorld1 | `0x19290` | 113 | 128 | 5 |
| TryInsert | `0x1ae30` | 59 | 64 | 2 |
| ReadAll | `0x1aec0` | 146 | 160 | 4 |
| CurrentRead | `0x1af60` | 87 | 96 | 2 |
| DemoRecStop | `0x1c6f0` | 230 | 240 | 10 |
| DemoStep | `0x1ca00` | 188 | 192 | 4 |
| ChatPop | `0x1cb40` | 76 | 80 | 0 |
| ChatPush | `0x1cb90` | 123 | 128 | 2 |
| TreatTextInput | `0x1cf60` | 586 | 592 | 3 |

[EVIDENCE] All locations above are distinct, and every listed relocation is
verified by the full comparator. Body sizes come from `oracle/functions.csv`;
alignment padding is excluded from the 2,882-byte gain. Intermediate raw-byte
diagnostics in `runs/*/summary.json` are not match certificates. In particular,
NetStartPlay first had matching instructions but failed two string payloads;
adding the oracle's CRLF suffixes was required before its STRICT result.

Delivery SHA-256:

- `integration/src/event.cpp`: `6e53f4542bba78d6faacd50abb1f830ad17a00b5f21ee0db9285ee76d04763d4`
- `integration/src/event.h`: `567859530a292b652c24eeaf9cf9a31cde8e84e33f49d5b66b5e80c7a4a503cc`
- `integration/strict.patch`: `be8323adc819070c10cecabc32e41f04d392855c466a0af5d0dd8008131b1a54`

## Header delta

[EVIDENCE] The only private header change is `event.h`:
`int GetWorldGroup();` becomes `void GetWorldGroup();`, with the corresponding
definition changed to void. No constants, members, offsets, or other method
declarations change. The target at `0x19200` updates `m_mission` and returns
without constructing either of the SB2 reconstruction's integer results.
Removing those results with a void declaration produces exact code. The
canonical `blupi.cpp` call discards the result. Recompiling all TUs against
the corrected copied header passes the gate, including that caller.

[HYPOTHESIS] The original declaration was void; the SB2 int signature and
returns were artifacts of interpreting residual EAX values. Exact bytes and
the ignored call result support this reconstruction, but do not independently
recover an original source declaration. Infra must integrate this one header
delta together with the delivered definition; the worker did not edit `src/`.

## ChangePhase checkpoint

[EVIDENCE] `scratch/sol-event/event.cpp` is the separate working candidate.
`runs/wip_final/` preserves its source, private event header, object,
disassembly, and comparator results. At `0x19310`, it emits a **6,912-byte
section / 6,908 instruction bytes**, versus **6,919 target instruction bytes**
(6,928-byte aligned span). It is MISMATCH, with a 132-byte diagnostic fixed
prefix; no partial bytes are counted as coverage.

[EVIDENCE] The full target buffer map is now reproduced: netData `+0x1c`,
join data `+0x28`, res `+0x44`, str `+0xa8`, text `+0x1ac`, shared find/sort/
joystick scratch `+0x210`, and door `+0x3a4`, in the frame after saved registers.
The prologue uses the target `sub esp,0x45c`. No padding, wrapper aggregate,
unused array, or byte-forcing construct was added to obtain it.

[EVIDENCE] Restored target regions include the independent phase tests and
demo-stop condition; DOQUIT and SETUPp phases; demo world range 200..219;
DOSERVICE failure path; one retained SearchPhase result; old phase read after
the outgoing write; 16-bit background cache attempt followed by 8-bit fallback;
INFO limits; 24 region buttons and dimension alternatives; multiplayer world
offset 100; MESS_LEAVE and one-byte host status; full DPID store plus join-name
termination; SESSION row indexing; music filename with music-1; movie tests of
the incoming phase; and the final debug CRLF. These are source/behavior repairs,
not independently matched functions.

[EVIDENCE] Named POINT dimensions reproduce the cache-call argument staging.
Assigning `dim = GetDim()` rather than initializing it directly recovers use
of the returned temporary pointer. Four dimension alternatives, including
separate X/Y alternatives with the same button states, recover the target's
shared tails. Positive `if (Read(...)) DrawMap; else error` moves the error
block to the tail, matching the target region order; a negative test with an
explicit tail goto was re-laid inline by the compiler.

[OPEN] First difference is at `+0x84`: target clears EDI, candidate EBX; the
early PLAY constant consequently occupies the opposite register. Later scalar
slots still differ (target gamer count `+0x10` and presence pointer `+0x18`,
candidate `+0x18` and `+0x20`), despite the correct array map. SERVICE/SESSION
load scheduling and register choices, and later save/setup regions, remain.
The eleven-byte instruction-size shortfall is not an eleven-byte isolated
edit. More local-size guessing is not justified by the recovered frame.

## Compiler findings

All controls use the full event TU, canonical SP3 flags above, and the canonical
header set unless the void-return experiment is explicitly identified. Run
names refer to archived sources, objects, and strict comparator output under
`scratch/sol-event/runs/`.

| Trigger and original construct | Controlled variation and measured effect | Strict result / rule / classification |
|---|---|---|
| ReadInput retained DL for buttons, used AL masks, and collapsed the intermediate button-three store | Replace branch assignments to `newKeyPress` plus the shared member assignment with direct member assignments in every branch. One control (`read_direct`) restores target AL, dword masks in other registers, and both member stores. Section stays 480 B. | STRICT 479 B; `VC5-LOC-01`, `VC5-ORD-01`; **confirmed-transferable** evidence that value-flow identity changes allocation and store combining. No context padding was needed. |
| CheckWorld1 and CurrentRead success-first source put the failure epilogue after success | Use an early failure return, then normal success work (`helpers_frame`). Target branch polarity, placement, and return-zero reuse are restored. | STRICT 113/87 B; `VC5-CTL-01`; **confirmed-transferable** branch-source-order evidence. REQUIRE_CD remains TRUE. |
| ReadAll used an invented address, three-argument save call, and `-1 < index` | Restore the measured presence member and two-argument CurrentWrite; then use `index >= 0`. The last comparison changes CMP -1/JLE to TEST/JL and removes one instruction byte (`phase_semantics`). | STRICT 146 B; `VC5-SEM-01` for member/call repairs; `VC5-CTL-01` **Blupi-specific** comparison-form evidence. |
| NetStartPlay loop ended with JGE exit/JMP continue rather than JL continue/JMP exit | Reverse DPID equality operands to restore load order, then use a bounded for scan with a found label and explicit no-match exit (`helpers2`, `helpers3`). Three remaining fixed instruction bytes become equal. CRLF payload correction (`net_literals`) then proves all relocations. | STRICT 230 B; `VC5-CTL-01`, `VC5-ORD-01`; **Blupi-specific** loop/control and operand-order evidence; literal repair is `VC5-SEM-01`. |
| ChatPush scan tested a row beyond its bounded loop and its sound call omitted the final flag | Bound the scan, retain an explicit destination for found/full paths, and use the four-argument PlayImage overload with flag 0. Four meaningful loop/destination controls culminate in `shared_enable`: the found branch calculates its row, while full-chat uses the constant last row. | STRICT 123 B; `VC5-CTL-01`, `VC5-SEM-01`; **Blupi-specific**. The direct index assignment and post-loop equality controls did not match. |
| DemoStep cached input as the KeyChange argument | Keep the input store, then pass `m_keyPress` instead of the local (`helpers2`). VC5 adds the target register copy and scheduling; section remains 192 B. | STRICT 188 B; `VC5-LOC-01`, `VC5-ORD-01`; **confirmed-transferable** repeated-member/value-flow evidence. |
| TreatTextInput key handlers were unreachable inside WM_CHAR; length was uninitialized on key paths | Restore independent message handling and actual strlen sites (`text_input`: 592 B, prefix 127). Enclose key handling in positive `if (message == WM_KEYDOWN)` and preserve LEFT's two return arms (`text2`): false epilogues join at the target tail and all instructions match. | STRICT 586 B; `VC5-SEM-01`, `VC5-CTL-01`, `VC5-ORD-01`; **confirmed-transferable** branch-layout effect after semantic repair. |
| NetAdjustLobbyButtons passed Boolean expressions; target pushes literal flags in branch arms | Repair the 10-world limit and relative button loop bound. Explicit ternary arguments (`lobby`) retain MOV/XOR/PUSH or SETcc. Five explicit if/else call pairs (`lobby_calls`) become the target branch-local pushes, restore later registers, and shrink section 512 -> 496 B. | STRICT 494 B; `VC5-CTL-01`, `BLUPI-REG-01`; **confirmed-transferable** source-call/branch shape affecting later allocation; **negative** for ternary substitution. |
| ChangePhase array slots differed despite earlier local-layout probes | Restoring real phase/cache regions changes str from +10c to target +a8 (`phase_semantics`), without changing its declaration or size. Moving the used gamer loop index to the root also restores netData +1c. Frame remains target 45c. | MISMATCH; `BLUPI-LOC-01`, `BLUPI-REG-01`; **Blupi-specific** evidence that the earlier size/scope-only model is incomplete. No general local-sort rule is inferred. |
| ChangePhase cache POINT macro staging and Read error block differed | Named POINT component assignments restore cache staging (`phase4`). Positive Read then/else restores tail placement (`phase_read_positive`); explicit tail goto (`phase3`) does not. | MISMATCH; `VC5-LOC-01`, `VC5-CTL-01`, `VC5-ORD-01`; **Blupi-specific positive regional evidence**, not a function match. |
| Remaining ChangePhase scalar/register differences | Shared cache bEnable versus block bCache, function-root versus inner num, distinct bHide versus bEnable are inert in the tested full TU. Hide init/conditional versus if/else changes twelve diagnostic bytes without improving the prefix (`phase_hide_order`). | MISMATCH; `BLUPI-REG-01`, `VC5-LOC-01`, `VC5-ORD-01`; **negative** for these exact families. One accidentally overbroad bHide replacement changed enable semantics; `phase_bhide` is invalid and excluded; `phase_bhide_correct` is the repaired control. |
| PutTextInputBox has target NEG/ADD versus candidate SUB at +26 | Negative half-width addition, split negative-then-add, LONG-to-int destination, and cumulative addition all retain the same SUB family. Reverted from delivery and working source. | MISMATCH; `VC5-EXP-03`, `VC5-ORD-01`; **negative**, four source controls. No arithmetic padding introduced. |
| MousePosToSprite has an extra target conditional/unconditional jump pair at +29 | Nested negative tests and positive phase alternatives retain the canonical instruction family. All controls reverted. | MISMATCH; `VC5-CTL-01`; **negative** for these forms. |

[EVIDENCE] Remaining straightforward repairs also matched: TryInsert restores
CheckWorld1 and a 40-tick retry; ChatPop copies successive rows forward with
strcpy, including the terminator; DemoRecStop formats a MAX_PATH filename
using GetWorld instead of the fixed typo-like `demo.3d.blp` literal. These are
`VC5-SEM-01` reconstruction corrections, not new optimizer rules.

## Remaining limits and next step

[EVIDENCE] SetDemo is still RELOC-UNVERIFIED, with identical instruction hits
at `0x16580`, `0x165b0`, `0x165e0`, and `0x16640`, each already owned by a
setter. There is no new incoming-reference proof, so no alias is promoted or
counted. Candidate-only helpers and header operators are not assigned invented
oracle locations.

[OPEN] Integrate the matched patch and its one header declaration together.
Resume ChangePhase from `runs/wip_final`, concentrating on the scalar spill
graph and the remaining SERVICE/SESSION and save/setup source forms. The frame
and array ordering are no longer the initial blocker. PutTextInputBox and
MousePosToSprite need new source/context evidence; the listed equivalent-form
families have diminishing returns. The two parked handlers remain untouched.

## Round 2: rebased continuation, 2026-10-17

[EVIDENCE] Rebuilt the working candidate from current `src/event.cpp` and
`src/event.h`, carrying only unfinished ChangePhase / PutTextInputBox /
MousePosToSprite bodies from `runs/wip_final`. The latter two were initially
identical to canonical. No old private `decor.h` was retained; the canonical
file-scope SortGetType declaration was used. `round2_base/` records the initial
inputs. All source outside the target bodies, including DrawButtons and
TreatEventBase, stayed canonical. No canonical source or header was edited.

[EVIDENCE] Deliver **`scratch/sol-event/integration2/strict.patch`** against
HEAD `575b139eb3260cfa1b18b4955a55c2061231810b`. It changes only the following
four bodies. ChangePhase and unsuccessful probes are excluded. `git apply
--check` passes, and application to `patchcheck2/` reproduces the compiled
event.cpp byte-for-byte. The patch preserves CP1252/CRLF.

| Newly STRICT function | RVA | Original code bytes | COFF section bytes | Proven relocations |
|---|---:|---:|---:|---:|
| MousePosToSprite | `0x18c70` | 123 | 128 | 1 |
| CopyMission | `0x1b150` | 195 | 208 | 10 |
| BuildDown | `0x1bd50` | 799 | 800 | 33 |
| PutTextInputBox | `0x1cd70` | 389 | 400 | 8 |

[EVIDENCE] These are four distinct original ranges, totaling **1,506 unique
code bytes**, with every relocation proven. Alignment padding is excluded.
All 20 manifest TUs were compiled against the single current copied header
set in `integration2/src/`. Every one of the **382 active accepted entries**
retains its required status, RVA and size; no losses. Event comparator counts
are **86 STRICT / 1 RELOC-UNVERIFIED / 22 MISMATCH**, including header bodies
and candidate-only functions. Exact command, compiler log, all objects,
comparator output and checked summary are in `integration2/verify_command.json`,
`verify.log`, `objects/`, `results.json`, `verification_summary.json`.

[EVIDENCE] **Header deltas: none.** The private event.h is byte-identical to
canonical. CopyMission's `_flag` tests use the existing VC5 FILE definition
(`_flag` at +0xc, `_IOERR` = 0x20), matching both oracle loads; no invented
layout or declaration is needed. The return is TRUE on error and FALSE after
EOF, as shown by target stack stores; no modern ferror compatibility branch
is retained in this historical source.

### Compiler findings

All controls use the complete event TU, VC5 SP3 11.00.7022,
`/W3 /GX /O2 /ML /Gd /Gy`, manifest defines and canonical headers. Each named
run under `scratch/sol-event/runs/` contains source, object, oracle/candidate
disassembly, and full verifier results. Diagnostics are never treated as
STRICT evidence.

| Trigger / original construct | Controlled variation and code effect | Strict result / rule / classification |
|---|---|---|
| BuildDown pushes bMix/fwKeys before its nested GetMenu call and includes three SB2-only placement arms | Remove absent WM_DECOR9..11 arms and assign the actually used `icon = GetMenu(...)` before each PlaceDecor1..7 call. Restores the oracle call/argument order (`r2_small`). | STRICT 799 B; source-arm repair is `VC5-SEM-01`; call staging is `VC5-ORD-01` / `VC5-LOC-02`, **confirmed-transferable**. |
| PutTextInputBox emits SUB where target has NEG/ADD at +0x26 | `posD = pos.x + lg / -2` is inert. `pos.x -= lg / 2; posD = pos.x;` makes the actual point component the destination and reproduces all bytes (`r2_text_pos_write`). Dividing and negating lg separately remains inert; negating GetTextWidth before dividing changes earlier scheduling but does not match. | STRICT 389 B; `VC5-EXP-03`, `VC5-ORD-01`; **Blupi-specific** destination/value-flow finding, not a claim that equivalent negative arithmetic generally changes code. |
| MousePosToSprite lacks the target JNE/JMP pair after the BYE comparison | Within the three-phase negative exclusion, retain an explicit BYE arm assigning SPRITE_POINTER, followed by else-if MouseOnButton (`r2_mouse_negative_else`). Positive ancestor-style outer arms recover the prefix but not the pair; goto and switch controls fail (`mouse4.py`). | STRICT 123 B; `VC5-CTL-01`, **confirmed-transferable** evidence for the conditional-arm structure. |
| CopyMission do/while versions duplicate fread and rotate the loop, or eliminate the final signed count test | Use signed int num, initialize it to 1, and a pretested while with a positive-count write arm; error gotos bypass the EOF success store (`r2_copy_while_positive`). The equivalent initialized for version also matches. This restores src EBP, dest EBX, buffer EDI, count ESI and the single read path. | STRICT 195 B; `VC5-CTL-01`, `BLUPI-REG-01`, **confirmed-transferable** loop-source-form evidence. Signedness/error semantics are `VC5-SEM-01`. |
| CopyMission alternative loop/error forms | EOF-break and positive/negative if/else do-loops, explicit back-label, size_t plus signed casts, and both compound tail-condition orders still duplicate reads or omit the retained count test. A pretested loop with an early EOF break has one read but removes the bottom count test and hoists 0x20 into BL (`r2_copy_for`). | MISMATCH controls; `VC5-CTL-01`, **negative** for these exact families. The initialized loop AND positive write arm are needed in the successful controls. |
| ChangePhase gamer count/presence pointer spill slots are reversed | Reuse the root num for both door count and joystick-device count (`r2_shared_num`). Ten displacement bytes change: count +0x18 -> target +0x10, presence pointer +0x10 -> target +0x18. Body size stays 6,908 instruction bytes at this stage. | MISMATCH; `BLUPI-REG-01/02`, **Blupi-specific** scalar-identity evidence. This corrects an earlier progress claim that sharing num was inert; the fixed-offset diff count hid the ten raw changes. |
| ChangePhase setup contains an extra pretest before its counted for loop | Remove the redundant outer `if (num > 0)` and retain the for's own test (`r2_phase_setup`). Restores XOR EBX before the joystick count call and the oracle setup loop, removing twelve instruction bytes. | MISMATCH; `VC5-CTL-01`, `BLUPI-REG-01`, **Blupi-specific** regional evidence. |
| ChangePhase pagination calls use SETcc rather than branch/MOV Boolean flags | Initialize bHide FALSE and conditionally assign TRUE in SERVICE/SESSION and read/write design; materialize the previous-page and nonempty-file flags similarly (`r2_phase_bool`). Restores the target branch families; body reaches 6,922 instruction bytes / 6,928 section bytes. | MISMATCH; `VC5-CTL-01`, `VC5-ORD-01`, **Blupi-specific** regional evidence. |
| Remaining ChangePhase register/scheduling residues | Shared root loop indices alter later code (`r2_shared_indices`). Sharing SearchPhase's index with i, moving index/music declarations to root, and UINT oldPhase are inert (`phase7.py`). Rounded-limit temporaries, predicate reversal, explicit state locals, ternary state arguments, and sibling-loop indices do not restore the complete body (`phase6.py`, `phase8.py`). | MISMATCH; `BLUPI-REG-01/02`, `VC5-LOC-01`, `VC5-EXP-06`; **negative** for the tested families. No artificial declarations were retained. |
| Possible TU count sensitivity of ChangePhase | Earlier sampled top/before/end extern-count controls leave the then-current body identical (`context2/`). After the source repairs, all top counts 0..15 leave the complete raw body identical, SHA-256 `24f4baa3e63c8ea03fa44d40d36893cfcd437eb55f282fd50b09096fcf1f4d36` (`context3/`). Every probe is restored in finally. | MISMATCH; `VC5-CTX-01..04`, **negative bounded context evidence**, not proof that every possible context is inert. |

### Unfinished ChangePhase and stopping point

[EVIDENCE] Resume from **`runs/r2_wip_final/event.cpp`** (canonical private
event.h alongside it). This contains the four new STRICT bodies plus the
unfinished ChangePhase; current `scratch/sol-event/event.cpp` is the same WIP.
Its frame and full array map match, and gamer scalar slots now match. The
candidate has **6,922 instruction bytes**, versus **6,919** target bytes,
and remains MISMATCH. Source-form repairs have not changed the first residue:
at +0x84, zero is allocated to EBX rather than EDI, and the early PLAY constant
gets the opposite register. The candidate retains zero across SearchPhase,
loses the PLAY constant, and substitutes a four-byte-longer immediate compare
there; its zero argument push saves one byte, producing the net +3. This is
not an isolated three-byte patch opportunity.

[OPEN] SERVICE/SESSION and design pagination retain different load/register
scheduling despite the restored Boolean branches. Multiplayer host/string
argument scheduling also differs. Target-ordered save and joystick regions
now agree in their instruction shapes. The bounded declaration, scalar and
expression families above have diminishing returns. Next work needs a new
source-form/value-flow lead for these regions; further array-size guessing
is not supported by the already correct frame and map.

[EVIDENCE] SetDemo remains RELOC-UNVERIFIED with its four already-owned setter
hits and no new incoming-reference proof. It is neither promoted nor counted.
DrawButtons and TreatEventBase remain untouched, even where their existing
compiled bodies mismatch. No game or ISO executable was run, no compiler
catalog was edited, no git commit was made, and no background process remains.
