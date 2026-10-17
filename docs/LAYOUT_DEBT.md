# decor.h layout debt — resolved proposal

[EVIDENCE] The integration candidate passes all **324 active accepted entries**
(321 STRICT, 3 FOLDED) in the unchanged current manifest and makes **BlupiStep
STRICT at 0x9280: 25,269 original body bytes, 25,280 compared bytes, frame a4,
zero fixed differences, 350/350 verified relocations**. All 20 TUs use the same
candidate header directory. No canonical source, manifest, compiler catalog,
asset, reference repository, or toolchain was changed. No acceptance withdrawal
or per-TU declaration view is needed.

[EVIDENCE] The canonical snapshot has 117 non-relocated byte differences in
that same 25,269-byte BlupiStep body (`probe-baseline-00`); the final proposal
has zero. Both have frame a4 and 25,280-byte compared extents.

## Review package and reproduction

[EVIDENCE] Full source/header snapshot: `scratch/layout/integration/src/`.
Apply only the five changes listed in `scratch/layout/integration/inventory.json`:

| Canonical file | Proposed change |
|---|---|
| `src/decor.h` | Recovered CDecor state/inventory/FIFO layout; plain byte NetMessage payload; matching saved-game names; remove unused SB2 fields |
| `src/decblupi.cpp` | Senior winning BlupiStep body with proper members; DecorDetect Helico/Nage/Surf; offset-preserving name migration in unfinished surrounding bodies |
| `src/decblock.cpp` | Front/Bullet/Perso and other member names at their actual offsets, retaining current InitDecor/Build/DrawInfo work and accepted GetBlupiInfo |
| `src/decor.cpp` | VoyageStep uses Perso; CurrentWrite/CurrentRead use corresponding runtime/file members |
| `src/decmove.cpp` | Offset-preserving names in existing state references |

[EVIDENCE] `scratch/layout/integration/integration.patch` passes `git apply
--check`. The inventory records before/after SHA-256 values. Unchanged source
lines preserve their original bytes and line endings. `sound.cpp`, which uses
`__LINE__`, and `button.cpp` are byte-identical to canonical inputs. The manifest
already has the corrected int GetTypeBarre and NetSendBarePacket signatures;
the earlier senior package's signature migration is unnecessary in this round.

```powershell
.pyenv\python.exe scratch\layout\run.py final integration/src
.pyenv\python.exe scratch\layout\package.py --audit
git apply --check scratch/layout/integration/integration.patch
```

[EVIDENCE] `run.py` invokes the required `tools/verify.py --build-dir layout`
CLI with all 20 `--tu src/X.cpp=scratch/layout/integration/src/X.cpp`
substitutions. The verifier necessarily stages `build/verify-layout/`; the
worker archives reports and objects in `build/layout/<experiment>/`.
`build/layout/final/command.json` contains the exact CLI, `verify.log` the full
transcript, and `results.json` all comparisons. `scratch/layout/integration/audit.json`
independently checks the accepted gate, key bodies, fixed bytes and relocations.
Compiles use tools/msvc.py, SP3, `/W3 /GX /O2 /ML /Gd /Gy`, default jobs/slots,
and repository-local TMP/TEMP. Campaigns were sequential.

## What moved MouseMove

[EVIDENCE] The close header changes more than member order. Relative to the
canonical protected state block it removes four SB2 flags from that block,
adds Front/Bullet/Perso, and gives NoBarre its own dword while replacing
`union { int m_blupiFifoNb; int m_blupiNoBarre; };` with a scalar FifoNb.
The senior version retains the four removed flags at the compatibility tail:
its net named-member change is **+3**, with **one fewer anonymous union type**.
Power/Shield are reordered; the other relevant types, arrays, methods, and
inline bodies are unchanged. There are no new bitfields or nested structs.

[EVIDENCE] The following controls isolate the anonymous union declaration.
They all used the complete verifier and archived accepted-set checks:

| Experiment under `build/layout/` | MouseMove | Other observation |
|---|---|---|
| `probe-baseline-00` | STRICT | Canonical snapshot, accepted gate passes |
| `control-power-order` | STRICT | Wrong offsets regress CurrentWrite/CaisseGetMove as expected |
| `control-layout-only` | STRICT | Moving four flags without replacement breaks storage offsets elsewhere |
| `control-union-single` | MISMATCH | Only FIFO union becomes scalar; NoBarre uses become FifoNb; all other accepted entries pass |
| `control-fifo-single-union` | STRICT | Remove the alias member but retain `union { int m_blupiFifoNb; };`; full accepted gate passes |
| `proposal-initial` | MISMATCH | Full corrected layout; BlupiStep and GetBlupiInfo STRICT; sole accepted failure MouseMove |
| `control-speed-single-union` | MISMATCH | Remove unused moveObjectSpeed member but retain anonymous char union; no context repair |
| `proposal-unused-speed-alias` | STRICT | Replace that char union with `char data1;`; all accepted entries and BlupiStep STRICT |

[EVIDENCE] Complete before-MouseMove extern-count diagnostics, counts 0–15:

| Header | Counts producing STRICT MouseMove |
|---|---|
| Canonical `baseline` | 0–3, 12–15 |
| Only FIFO union replaced by scalar | 3–10 |
| Senior corrected layout, `proposal` | 3–10 |

[EVIDENCE] Records are `build/layout/probe-<base>-NN/audit.json` and matching
objects. `scratch/layout/probe.py` inserts diagnostics only into separate probe
copies. No probe declaration is present in the integration snapshot. The
senior header and union-only header have identical response vectors: the three
extra scalar names and member moves do not explain this residue. Deleting the
five unused m_sb2 timer declarations, then four unsupported compatibility flags,
also leaves the same unprobed MouseMove failure while BlupiStep stays STRICT.

[HYPOTHESIS] The response is consistent with removing the anonymous union
changing preceding frontend numbering by **−7 modulo 16** (equivalently +9),
with the optimizer using a phase of that numbering. The single-member-union
controls locate the effective change in retaining/removing the anonymous UDT,
rather than merely retaining/removing an alias name. These experiments do not
expose VC5's internal absolute counter or prove its hidden-symbol implementation;
do not report seven particular compiler symbols as observed facts. A declaration
line count or a count of member names alone is insufficient here.

## Source-supported repair

[EVIDENCE] NetMessage already has an 8-byte wire shape: type byte at 0, data1
byte at 1, and data2/data3/data4 words at 2/4/6. Current ObjectStart writes speed
through `data1`; every canonical `.cpp` has zero uses of `moveObjectSpeed`.
The char union's second name comes from the read-only SB2 reconstruction and
adds no distinct storage or operation. The proposal uses the existing `char
data1` field directly and removes that unused alias/type. It adds no declaration,
padding, byte forcing, or new overlay. The other NetMessage unions remain:
current ModifDecor uses posX/posY/decorIcon while other senders use data2/3/4.

[EVIDENCE] Simplifying all four NetMessage unions restored MouseMove but
regressed GetBlupiInfo's commutative loads (`proposal-wire-message`). Restoring
the direct ancestor GetBlupiInfo expression did not repair it
(`proposal-wire-ancestry`). Simplifying the NetPacket union too regressed both
functions (`proposal-wire-all`). Those wider cleanups are not proposed. The
chosen char-field simplification removes the union with **no used alternate
name**, keeping the overlays actually used by this tree.

[OPEN] Exact original source spellings and whether the 1998 header contained
this particular union cannot be recovered uniquely from machine code. The
proposed plain char is the minimal field representation supported by all current
uses and target byte accesses. No claim of unique original syntax is made.

## Header deltas and offset evidence

[EVIDENCE] `build/layout/offsets-integration-src/offsets.json` contains SP3
compiler-evaluated offsets emitted by a separate diagnostic data table in a
dectables candidate. That table is absent from the integration source. The
baseline measurement is `build/layout/offsets-baseline/offsets.json`.

| CDecor member(s) | Proposed offset(s) | Target evidence |
|---|---|---|
| Air; Helico/Jeep/Tank/Skate | c908; c90c/c910/c914/c918 | BlupiStep state/vehicle paths; GetBlupiInfo 57b0 |
| Nage/Surf/Vent/Suspend | c91c/c920/c924/c928 | BlupiStep water/vent/bar paths; DecorDetect tests c90c/c91c/c920 |
| JumpAie/Shield/Power/Cloud/Hide | c92c/c930/c934/c938/c93c | BlupiStep injury, protection and pickup paths |
| PosHelico; PosMagic | c940/c944; c948/c94c | BlupiStep saved-position reads/writes |
| Restart/Front/Bullet/Perso/NoBarre | c950/c954/c958/c95c/c960 | BlupiStep; InitDecor 3b80; Build 4350 front flag; DrawInfo 5270 counts |
| TimeShield/TimeFire/TimeMockery | c964/c968/c96c | BlupiStep countdowns; TimeShield is int, not a flag |
| TimeOuf/ActionOuf | c970/c974 | BlupiStep pending-action counter/value |
| FifoNb/FifoPos | c978/c97c | BlupiStep/BlupiAddFifo; NoBarre has separate c960 storage |
| StartPos; jauges | c9cc; cd30 | Unchanged; existing strict functions retain later offsets |

[EVIDENCE] The four unsupported SB2 state members Over/Invert/Balloon/Ecrase
are removed entirely in the final header. Surrounding unfinished bodies receive
offset-preserving member-expression migrations. The five unused m_sb2 timer
members are also removed. No constants, method signatures, bitfields or opaque
tail arrays are added. Existing TimeNoAsc/trajectory/lastDecorIndexes tail
reconstruction remains inherited and unproved as original SB1 layout; this work
claims the state block above and accepted accesses, not sizeof(CDecor).

[EVIDENCE] DescGameFile now names the same semantic state/inventory fields at
its existing file offsets. Its Helico starts at cc00, Front/Bullet/Perso are
cc48/cc4c/cc50, NoBarre cc54, stored timers/pending action cc58..cc64, and FifoNb
cc68. The file format omits TimeMockery. The later FIFO/start arrays remain at
cc6c/cd84; sizeof(DescGameFile) remains **d490**. CurrentWrite still passes every
byte and relocation with the corrected LHS/RHS names. CurrentRead's corresponding
names are migrated but its existing full body remains MISMATCH.

## Whole-tree verification report

[EVIDENCE] Final CLI exit 0; 20/20 TUs compile; all 324 active accepted entries
retain their status, location, compared size, and folded owner. The raw candidate
inventory reports 327 STRICT, 3 FOLDED, 6 RELOC-UNVERIFIED and 415 MISMATCH
records, including inline/duplicate candidates; these are not new acceptance
counts. Only **25,269 newly strict unique original bytes** are credited here.
The accepted set's deduplicated oracle-body intervals total 40,754 bytes;
including BlupiStep they total **66,023 bytes**.
The verifier's 70,432-byte interval total includes boundary padding and must not
be presented as original function-body coverage.

| Function | RVA | Original body / compared bytes | Final proof |
|---|---|---|---|
| CButton::MouseMove | 2fd0 | 201 / 208 | STRICT, 1/1 relocations |
| CDecor::GetBlupiInfo (decblock owner) | 57b0 | 67 / 80 | STRICT, no relocations |
| CDecor::BlupiStep | 9280 | 25,269 / 25,280 | STRICT, 350/350 relocations, frame a4, zero fixed differences |
| CDecor::DecorDetect | f640 | 640 / 640 | STRICT, 3/3 relocations |
| CDecor::IsLave | 10150 | 88 / 96 | STRICT |
| CDecor::IsRessort | 101b0 | 94 / 96 | STRICT |

[EVIDENCE] BlupiStep's former ten register bytes were already solved in the
senior source; this round transplants that winning body onto the current tree
and removes the shared-header gate. `audit.json` lists excluded duplicate/event
aliases separately; no folded body or identical address is credited twice.
Promotion remains the supervisor's step. Suggested new manifest entry is in
`scratch/layout/integration/blupistep-acceptance.json`; no existing entry changes.

## Compiler findings

- **Anonymous UDT context, not visible member count.** Trigger: five MouseMove
  load/register/SIB residues at +6e/+71/+74/+7e/+80 after layout recovery.
  Original/candidate: two-member anonymous FIFO union versus same-offset int;
  single-member union controls retain STRICT. SP3/full 20-TU flags above.
  Measured count windows shift 0–3,12–15 to 3–10; corrected full layout has the
  identical vector. Result: union removal alone regresses MouseMove; all other
  accepted entries pass. `VC5-CTX-01..04`, `BLUPI-CTX-01`.
  **Classification: blupi-specific measured context effect; −7-mod-16 counter
  interpretation unverified.**
- **Unused char overlay removal closes the shared gate.** Original/candidate:
  NetMessage anonymous char union with unused moveObjectSpeed alias versus
  existing char data1 alone. Same flags/whole-tree context. Size and offsets
  unchanged; all 324 accepted entries plus BlupiStep STRICT together. Retaining
  a single-member anonymous char union does not repair MouseMove. No filler
  declarations. `BLUPI-CTX-01`, `VC5-CTX-04`.
  **Classification: confirmed SB1 repair; not a general rule to delete unions.**
- **Scalar/tail and serialization controls.** Removing five unused timer fields,
  then four unsupported flags, does not repair the original MouseMove residue;
  correcting the saved-game state names also leaves it. Once the char overlay
  is removed, each cleanup and their combined final form pass the accepted set
  and preserve STRICT BlupiStep. `VC5-CTX-01`, `VC5-SEM-01`.
  **Classification: negative for scalar-member-count explanation; blupi-specific
  layout/name repair.**
- **Broad wire-union cleanup breaks a peer.** Removing all four NetMessage
  unions makes MouseMove STRICT but swaps both GetBlupiInfo operand loads;
  direct ancestry expressions are inert. Also removing the NetPacket union
  regresses MouseMove. Same compiler/flags/full-tree gate. `VC5-CTX-04`,
  `BLUPI-CTX-01`. **Classification: negative; rejected candidates.**

[OPEN] InitDecor, Build, DrawInfo and other unfinished functions remain
MISMATCH; their semantic/source reconstruction is separate from this resolved
accepted-header gate. Their inherited SB2 behaviors are not certified by member
renaming. There is no remaining blocker to reviewing/integrating this five-file
proposal and accepting BlupiStep. Final checks found no compiler/Python worker
left running; no background campaign is intentionally retained.
