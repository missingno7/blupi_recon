# sol-decblock: remaining decblock / decor pass (2026-10-17)

## Deliverable and verification

[EVIDENCE] Main patch: `scratch/sol-decblock/integration.patch`. Candidate
sources: `scratch/sol-decblock/decblock.cpp` and `decor.cpp`. Canonical source,
headers and manifest were not edited. Source files retain CRLF. No commit was
made. Only this document, the owned scratch directory and the isolated verifier
build were written.

[EVIDENCE] Started from HEAD `18b6b58`, copying both canonical TUs fresh. During
the pass the supervisor advanced HEAD to `24ce276` (MouseDown integration).
Both owned canonical .cpp files remain byte-identical to the initial copies.
The delivery patch is generated against `24ce276`; it also applies to `18b6b58`.
The MouseDown gain is not credited to this worker.

[EVIDENCE] Final verification, pinned VC5 SP3, manifest
`/W3 /GX /O2 /ML /Gd /Gy`, canonical headers, no PCH:

```powershell
.pyenv\python.exe tools\verify.py --tu src/decblock.cpp=scratch\sol-decblock\decblock.cpp --tu src/decor.cpp=scratch\sol-decblock\decor.cpp --build-dir soldecblock
```

[EVIDENCE] Exit **0**; all **376 active accepted entries** pass. Evidence:
`final-verify.log`, `final-verify-exit.txt`, `delivery-audit.json`,
`final/results.json` and the two owned objects/disassemblies under
`scratch/sol-decblock/`; the complete final object set remains in
`build/verify-soldecblock/`. No promotion/accept command was run.

| New STRICT function | RVA | Original body bytes | COFF compared span | Proven relocations |
|---|---:|---:|---:|---:|
| Create | 0x39d0 | 196 | 208 | 7/7 |
| LoadImages | 0x3aa0 | 183 | 192 | 7/7 |
| PlayPrepare | 0x3e90 | 703 | 704 | 7/7 |
| MoveStep(BOOL) | 0x41d0 | 261 | 272 | 4/4 |
| ChangePhase | 0x5950 | 208 | 208 | 5/5 |
| AdaptDoors | 0x15e30 | 840 | 848 | 6/6 |

[EVIDENCE] **Six new STRICT functions, 2,391 unique original instruction
bytes, all 36 relocations proved.** No folded body is added. The 2,432-byte
COFF-span increment includes 41 alignment bytes and is not body-byte credit.
The final run's whole-game span total is 103,088, including the independent
96-byte MouseDown integration. GetBlupiInfo, InitDecor, DrawInfo and
BuildPrepare remain STRICT and are not counted as new.

[EVIDENCE] Thirty inherited non-owner helpers after the decblock block are
removed; their complete names are in `delivery-audit.json`. These were SB2
extras or drafts belonging to other SB1 TUs, including duplicate border and
collision routines. No function owned by those other TUs was edited.
AdaptMotorVehicleSound is moved before ChangePhase to follow oracle source
order. These changes preserve the entire accepted gate.

[EVIDENCE] Header deltas: **none**. The private `decor.h` is byte-identical to
canonical. MoveStep uses the already-declared BOOL overload; ChangePhase uses
the already-declared four-argument PlayImage overload. No layout, padding,
constant or shared declaration was invented.

## Retained source correction without a strict claim

[EVIDENCE] FileExist's mission-99 caption passes `open` before nbVies,
nbTresor and totalTresor. At target +0x14b, EDI (the door-count accumulator)
is pushed between the life count and the format pointer; the caller removes
0x18 argument bytes at +0x153. The previous five-argument sprintf merged with
the private-mission branch; that was a real missing format argument. The
main patch retains this correction. FileExist remains MISMATCH (556 candidate
body bytes versus 540); validation/cleanup layout is still unresolved.
CurrentRead's unsuccessful layout experiments are reverted to canonical.

## Recovered DecorNextAction: peer gate blocks integration

[EVIDENCE] `blocked-decblock.cpp` is the main candidate plus the authentic
DecorNextAction body and its ordinary initialized table. The body follows SB2
ancestry with the unsupported pause guard removed, as required by target
0x5670. Its 269 instruction bytes and all six relocations are STRICT in
`blocked-results.json`. This is a strict function result, not an integrated gain.

[EVIDENCE] Table `table_decor_action` is absent from canonical definitions.
The target at 0x2ea28 has two records (IDs 1 and 2, 32 coordinate pairs each)
and a terminating zero: 133 signed ints, 532 initialized data bytes. The
ancestral third record is absent. The numeric initializer is retained in
`table_decor_action.cpp`; its complete compiled payload and six references
are proved by the blocked full run. Executable bytes were not used as source.
Putting the table in the recovered decblock TU is a private proposal, not
proof of the original data-definition file.

[EVIDENCE] Adding this real body regresses accepted GetBlupiInfo: its two
OR operand-load pairs swap, leaving four fixed-byte differences. The blocked
run exits 1 solely for this peer; its transcript, report and decblock object
are preserved separately. The main patch omits this addition and passes.
The blocked source is not included in `integration.patch` and must not be
substituted as the accepted candidate.

[EVIDENCE] Sequential subset checks isolate the interaction:
Create+LoadImages+ChangePhase alone regress GetBlupiInfo; adding the genuine
MoveStep(BOOL) body restores it; adding corrected PlayPrepare preserves it;
adding DecorNextAction regresses it again. Full snapshots are `gate-a` through
`gate-d`. This is source-context evidence, not permission to add count padding.

## Compiler findings

All trials below used the complete manifest and the SP3 flags above, through
`verify.py --build-dir soldecblock`. Hints/rules were consulted before tuning.

- [EVIDENCE; BLUPI-REG-02 / VC5-LOC-03; blupi-specific] Create's two POINT
  helper temporaries become one named position with x changed for the second
  gauge and y retained at 450. Shared height stays in EBX across calls; the
  body changes 199 -> 196 bytes and becomes STRICT. No header change.
- [EVIDENCE; VC5-CTL-03 / VC5-EXP-01; blupi-specific] LoadImages formerly had
  a success return inside the region guard and another afterward. Sharing the
  outer success return removes the neg/sbb/neg boolean materialization on the
  final Cache path: 185 -> 183 instruction bytes, STRICT.
- [EVIDENCE; VC5-SEM-01 / VC5-ORD-01; blupi-specific] PlayPrepare needed real
  state repairs before scheduling: forward 0..199 object traversal; six target
  random-phase types; no later-game resets; target inventory and timer slots;
  Suspend and Power/Cloud order. After these repairs its body was 703 bytes
  with eight differing bytes. Moving nbLinkCaisse reset before scrollPoint
  closes the remaining scheduling difference and makes it STRICT. Restoring
  the ancestral POINT(0,0) scrollAdd helper grows it to 711 and loses STRICT;
  component zero stores are retained.
- [EVIDENCE; VC5-SEM-01 / BLUPI-REG-02; blupi-specific] MoveStep's existing
  caller passes one BOOL, and the target ends in ret 4. Correcting the no-arg
  definition, using one step=50 local, and calculating the positive-direction
  bound before updating the coordinate yields 261 bytes, STRICT (was 271).
  Branch-local versus shared bound locals are byte-inert in this checkpoint.
- [EVIDENCE; VC5-SEM-01; blupi-specific] ChangePhase's resumed motor sound
  pushes a fourth argument of TRUE in the target. Adding that argument changes
  206 -> 208 bytes and makes the existing overload call fully STRICT.
- [EVIDENCE; VC5-SEM-01 / VC5-LOC-01 / VC5-CTL-03; blupi-specific]
  AdaptDoors uses channel 1, a reused loop index, and posCurrent POINT copies.
  These reduce the mismatch to the final player-start loop (838 versus 840).
  Replacing its destination-field ternary with an explicit if on newPosBlupi.x
  produces the target local comparison and 0x10 frame: STRICT, 840 bytes.
  The equivalent local-position ternary hoists the condition and gives a 0x14
  frame / 847-byte body instead; it is not retained.
- [EVIDENCE; VC5-SEM-01; blupi-specific] FileExist's missing `open` argument
  is a call-arity correction, not register tuning. It prevents an incorrect
  format-call merge; 530 -> 556 candidate body bytes, still MISMATCH.
- [EVIDENCE; VC5-CTL-03; negative] A guarded open/read followed by a combined
  validity check gives CurrentRead 1594 / FileExist 544 bytes before the format
  correction. A real bOk do/while validation form gives 1575 / 551 after it,
  preserving early cleanup but adding 11 flag-handling bytes versus target.
  Replacing that flag with success-break/error-label structure restores the
  old cold-cleanup output (1580 / 556). None is STRICT; all are reverted.
- [EVIDENCE; VC5-CTX-01..04 / BLUPI-REG-02; negative] Explicit OR temporaries,
  commuted OR operands and reuse of an existing temporary do not repair the
  blocked GetBlupiInfo loads. Compound writes through the output pointer add
  a store and grow 67 -> 69 bytes. Genuine local-sharing/scope changes and
  replacing DrawInfo's final POINT helper are also insufficient. Removing
  the unsupported no-arg MoveStep and one-arg Build declarations in a private
  header is inert for this peer; that header trial is reverted. No filler
  declaration, dummy body, volatile or forced-byte construct is retained.
- [EVIDENCE; BLUPI-REG-01 / BLUPI-REG-02; negative] Build's new trial combines
  shared remote-player locals, direct player-effect calls and exchanged genuine
  POINT identities. It grows 3846 -> 3898 and worsens the score 1489 -> 1851.
  Reusing only remote locals and spelling the foreground 8..11 bounds as in
  target changes the score to 1481, without closing the 20-byte gap or peer
  gate. These new variants are reverted. Prior documented background, count
  probe and direct-call campaigns were not rerun wholesale.

## Remaining work

[OPEN] Build is unchanged at 3846 versus 3866 instruction bytes, frame 0x44,
score 1489; first fixed difference +0xca. AdaptBorder remains eight register
bytes different; DrawMap first differs at +0x23e (120 fixed differences).
CurrentRead remains 1580 versus 1564 bytes; corrected FileExist 556 versus 540.
The earlier documentation's equivalent cleanup and point-spelling families
remain negatives. DoInit allocation-tail debt and ReadConfig were not touched.

[OPEN] Integrate the passing main patch and accept the six bodies through the
supervisor's normal process. Continue Build's source-region reconstruction;
then reassess the isolated, strict DecorNextAction addition together with
GetBlupiInfo. The peer gate is a real blocker to that addition, not a reason
to normalize its load bytes. No background process was started or left running.
