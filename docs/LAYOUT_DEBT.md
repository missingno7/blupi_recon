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

## Follow-up 2: CDecor allocation tail — unresolved

[EVIDENCE] The first-round layout and BlupiStep have been integrated. This
follow-up snapshots the new canonical tree in `scratch/layout/tail-baseline/`.
The manifest has 361 active entries (358 STRICT, 3 FOLDED). A full shared-header
copy under `scratch/layout/integration2/src/` passes that complete gate. No
canonical input or catalog was edited in this follow-up.

[EVIDENCE] **DoInit is still MISMATCH.** Its 2,801-byte oracle body / 2,816-byte
compared extent differs only at +9a3/+9a4/+9a5, inside the immediate of the
`push` at RVA 2892. Oracle allocation is 258d4. The present SP3 sizeof(CDecor)
is **dbd8**, not dbec; the present gap is **17cfc = 97,532 bytes**. The earlier
approximate dbec figure would give 97,512. This distinction is measured rather
than adjusted through a source constant. The 235 DoInit relocations are not
certified by the fixed-byte near match.

### Static access and computed-address audit

[EVIDENCE] `scratch/layout/tail_scan.py` scans every operand in all **407
inventory functions before the CRT boundary at 238b0: 39,693 instructions**,
including all **154 functions in the six CDecor TU blocks**. Capstone decodes
every byte of those inventory function bodies. Output:
`build/layout/tail-scan/scan.json`. The initial raw scan over all 579 oracle
functions, including CRT, is also retained as `build/layout/tail-raw-scan.json`.

[EVIDENCE] There are **24 based-memory displacement hits >=d208 and <40000**
in game functions, all in CurrentWrite (14da0, 12) and CurrentRead (153d0, 12).
They are accesses to the **separately malloc'd DescGameFile buffer**, not the
CDecor object. At 14dfd the target pushes d490, calls malloc at 14e02, and
saves the returned pointer in EBX; at 15429 it allocates the same file buffer.
CurrentWrite keeps CDecor in EBP. Its `rep stosd` at 14e20 zeros d490 bytes
through **EDI=EBX**, while subsequent source addresses use EBP. The largest
false-positive file displacement is d3c4. No direct based-memory displacement
at or beyond the current class end dbd8 was found.

[EVIDENCE] `scratch/layout/tail_cfg.py` additionally follows register copies,
LEA, add/sub, global g_pDecor loads (VA 47e74c), CFG branches/backedges and VC5
indexed switch tables. After switch expansion, all 39,693 decoded instructions
are reached. It records **4,856 possible this-derived memory/address uses**,
including indexed forms and addresses accumulated before dereference, and
**39 this-rooted bulk-copy/fill instructions**. None has a retained base anchor
>=d208. The highest explicit recovered anchor is d07c at 83ea, an element
address in the existing last-decor-index array. Output:
`build/layout/tail-scan/cfg-aliases.json`.

[OPEN] This CFG pass is triage, not a complete pointer-range proof: stack-spill
alias recovery, arbitrary pointer loads, returned-pointer summaries and numeric
index bounds are not modeled. Widened index expressions keep their base anchor.
Negative output therefore does not prove that no conceivable indirect access
exists. Raw operands, target pseudocode and the following loop checks provide
the concrete evidence; no new member is inferred solely from a taint label.

[EVIDENCE] The audit also inspected inventory gaps. There are 381 gaps totaling
5,528 bytes; nine contain something besides alignment. Their code portions
include the window procedure at 1610, timer callback at 1c50, small constructor
at 29f0 and network callbacks at 209c0/20b50; others contain switch tables.
A bytewise candidate scan is retained in
`build/layout/tail-scan/gap-bytewise-hits.json`. Its apparent large operands
decode from instruction interiors or table bytes: e.g. 2a0f lies inside the
real `mov [eax+dc],ecx` at 2a0a. No validated tail access was recovered there.
Gap extents are in `build/layout/tail-scan/gaps.json`.

### Bulk arrays and constructor/destructor constraints

[EVIDENCE] All 45 CDecor-block MOVS/STOS instructions were inspected, including
the six involving only stack/file buffers. Their families are:

| Runtime storage | Evidenced range/size | Representative target sites |
|---|---|---|
| decor / bigDecor grids | 10..4e30 / 4e30..9c50, 20,000 bytes each | 14e49/14e5c copy out; 15491/154a4 copy in |
| MoveObject[200] and record temporaries | 9c50..c1d0, 200 x 48 bytes; 48-byte copies | 14cbb clear; 13ae8, 13b4e, 13c6d..13c92 |
| rank/link caisse arrays | 200 ints each at c284/c5a8 | 14f0d/14f2c, 15555/15574 |
| FIFO / start positions | 10 POINTs at c97c; 4 POINTs at c9cc | 15155/15168, 15797/157b0 |
| mission/notification strings | c218[100]; four 100-byte notifications at cb9c | 8550/8557, 114df/114e7, 11554/11560 |
| doors[100] | cdd4..ce9c, 200 bytes | 3b7b, 15271/158c1, 16210/16214 |
| lastDecorIndexes initialization | cee8..d208, **200 dwords = 800 bytes** | constructor 3930..3945 |
| DescGameFile local heap buffer | d490 bytes on a separate allocation | 14dfd/14e20, 15429/1543f |

[EVIDENCE] Constructor 3910 (STRICT, 158 body / 160 compared bytes) constructs
only its two CJauge elements at cd30, stride 30; it clears the 200 indices with
`lea edi,[esi+cee8]; mov ecx,c8; rep stosd`. The last byte of that clear is
**d207**. Destructor 39b0 (STRICT, 22 / 32) destroys only those two jauges.
These bodies are compatible with extra trivial/uninitialized members, but
identify neither their element types nor their dimensions. The constructor's
clear proves an initialized extent, not that no longer unused array could exist.

[EVIDENCE] `scratch/layout/tail_measure.py` emits a separate, diagnostic offset
table through the required whole-tree verifier. It changes no class declaration.
`build/layout/tail-size-probe/layout.json` records:

| Current declaration | Offset | Size | SB1 tail evidence |
|---|---:|---:|---|
| m_lastDecorIndexes[200] | cee8 | 320 | Clear of exactly 200 dwords and editor element accesses |
| m_blupiTimeNoAsc | d208 | 4 | Inherited compatibility member; no target offset evidence recovered |
| m_balleTraj | d20c | 4e3 | Inherited 1,251-byte SB2 bitmap; no target offset evidence recovered |
| m_moveTraj | d6ef | 4e3 | Same limitation |
| m_detectIcon | dbd4 | 4 | Inherited compatibility member; no target offset evidence recovered |

[EVIDENCE] The known index declaration ends at d208. Existing unproved tail
declarations occupy 9d0 = 2,512 bytes including alignment; target capacity after
that initialized extent is **258d4 - d208 = 186cc = 100,044 bytes**. Treating
the current compatibility members as a proved prefix of a new 97,532-byte tail
would introduce an additional unsupported layout assumption.

### Ancestor comparison and rejected size-only partitions

[EVIDENCE] The read-only SB2 reconstruction ends its class with:
`m_lastDecorIndexes[200]`, `int reserve[11]`, `BYTE dummy[100000]`.
Removing the current compatibility tail and borrowing those placeholders would
give the exact arithmetic:

`cee8 + 200*4 + 11*4 + 100000 = 258d4`.

[HYPOTHESIS] This coincidence may explain where a prior reconstruction chose
its size placeholder. It does **not** establish 11 original int fields, a
100,000-byte original member, or their purposes. SB2's README explicitly
describes a binary reconstruction with defects/missing features; its dummy
declaration is not an independently recovered SB1 member. No filler variant
was compiled or proposed as a match.

[EVIDENCE] Planet Blupi v1.8.0 has an undo **pointer** m_pUndoDecor near the
start of CDecor; UndoOpen mallocs its grid and UndoCopy/UndoBack memcpy that
separate allocation (`external/planetblupi/src/decor.cxx`, UndoOpen..IsUndo).
Its m_decorMem grid is also near the class start, not a tail continuation.
Its real tail contains m_cheminWork, m_cheminNbPos, m_cheminPos/m_cheminRank
and fill-search state. The original-style local v1.7.0 `decor.h` was also read
through `git show` and saved under `build/layout/pb17-decor.h`; it has the undo
pointer and path/fill tail, with no 100,000-byte dummy. PB field existence does
not establish their presence or layout in SB1.

[HYPOTHESIS; unsupported] With SB1's 100x100 grid and measured Cellule size 2,
five grids use 100,000 bytes. One POINT grid plus one Cellule grid also uses
100,000 bytes. Eleven more ints would make either partition fit 100,044 bytes.
Neither interpretation has a corresponding address, copy, count or editor-undo
operation in the recovered target evidence. Choosing between them by the size
operand alone would manufacture source members. PB's path-work byte grid would
be 10,000 bytes under SB1 dimensions, not a uniquely determined remainder.

[EVIDENCE] Demo buffers have another evidenced owner: accepted CEvent::DemoPlayStart
at 1c7e0 uses m_pDemoBuffer and a **separate 40,000-byte allocation**
(MAXDEMO=10000, DemoEvent two shorts). Canonical event.h/event.cpp agree with
that STRICT body. No evidence moves those records into CDecor.

### Deliverable status and verification

[EVIDENCE] `scratch/layout/integration2/audit.json` and
`build/layout/tail-final/` retain the complete 20-TU CLI, objects, results
and accepted gate. A second whole-tree diagnostic run (`tail-size-probe`)
also preserves the entire accepted set. No new strict bytes are claimed.
BlupiStep remains STRICT; DoInit remains the three-byte allocation near miss.

[OPEN] **No evidence-backed C++ tail proposal is available.** The requested
`scratch/layout/integration2/integration.patch` is deliberately empty and its
README marks it **partial / not an integration change**. The accompanying
`src/decor.h` is an unchanged canonical snapshot, and `tail-constraints.json`
records measured sizes plus the missing evidence. There is no substituted size,
opaque byte array, fake undo buffer, or acceptance claim to integrate.

[OPEN] Closing DoInit now requires independent member/type evidence (for example
original headers, debug types, or an actual target-derived tail-access/copy
pattern). The allocation operand proves requested storage capacity; by itself
it cannot name or partition unaccessed storage. Further array-size permutations
would only satisfy that operand and are not a valid matching investigation.

### Compiler findings — tail follow-up

- **Size operand is genuine layout debt.** Trigger: three bytes in DoInit's
  `new CDecor` immediate. Original/candidate: allocation 258d4 versus the
  unchanged canonical class's measured dbd8. SP3 `/W3 /GX /O2 /ML /Gd /Gy`,
  complete 20-TU copies; both the normal run and diagnostic offset-table run
  preserve all 361 accepted entries. DoInit remains MISMATCH with 2,816-byte
  extent and 235 relocations not certified by this near match. No artificial
  header/context or allocation override was tested. `VC5-SEM-01`.
  **Classification: blupi-specific evidence; no new compiler transformation.**

## Follow-up 3: the tail is a real, fixed block ([EVIDENCE], 2026-10-18)

The SB1 demo (1998-06), the eGames build (1999) and Speedy Blupi 2 (2001) were compared statically (docs/ORACLE.md).
`new CDecor` is 0x258d4 in all SB1 builds and 0x26320 in SB2. SB2's earlier members are 0xa4c bytes larger, and the
unreferenced block after `m_lastDecorIndexes[200]` is exactly 100,044 bytes in both games, as predicted from sb2-decomp's
header history. The block is a genuine part of the original class, not a reconstruction artifact. Data members cost no
front-end symbol IDs (measured; docs/SYMBOL_ORDER.md), so declaring it would not shift any TU context.
[OPEN] Its member names and types. The only evidence is its size (100,044 = 4 x 25,011). Whether an opaque
declaration of a proven-size original block is acceptable source is a policy decision for the owner. No such
declaration is in canonical source.
