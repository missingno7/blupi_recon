# SB1 ICF with external network callbacks

2026-10-24. **STATUS: partial. No integration candidate qualifies.**

[EVIDENCE, follow-up] The **Solver** section below adds a real 801-entry
source witness: 412/412 mapped functions at their oracle RVAs, 175,718
same-offset `.text` bytes with attested sound, and 596 unequal whole-file
bytes. Historical source identity and unchanged certificate acceptance are
not established; the original four-token candidate results below remain valid.

[EVIDENCE] The corrected callback order is sufficient to explain every observed
survivor change, without changing the COMDAT population, libraries, function
payloads, or object order. More precisely, **the sessions pair alone causes
all five survivor flips**; the providers pair changes none of the survivors.
This does not identify the original link's other missing/different inputs.

[NEGATIVE] Both callbacks external gives 48 functions at original RVA, versus
the guard's 131, and 40,064 same-offset `.text` bytes versus 54,919 with the
same attested historical sound object. It also fails the current accepted
function gate on all six Eggbert targets because of a reproducible comparator
fold-owner cycle. No unsupported source reordering, dummy input, library
addition, linker option, or binary modification is proposed.

## Rule, traced rather than inferred from MAP symbol ordering

[EVIDENCE] Pinned SP3 LINK SHA-256
`04c892989f4cc8076ef45e033215eb7aefbdf1459cbee9b13a81f8ddb718b579`,
historical MSVCRT SHA-256
`e390b0529fb9ee1214bb68302a4658b578134d92ab76bc5eb48f3badd843d210`.
All links use canonical `tools/histlink.py` host setup and the oracle clock.
The supplemental observer records contribution owners/flags and each actual
fold at LINK VA `0x432a4b`. It changes no LINK decision or output byte.
Both observer images equal their ordinary histlink images by whole-file hash.

[EVIDENCE] LINK appends eligible executable COMDAT supplemental records while
reading objects/archive members (`0x4604d8..0x46058b`, eligibility helper
`0x431f30`). In these inputs, order is module load order and COFF section
order within the module, including associative `.text$x` sections. There are
**793 entries: 790 `.text`, three unnamed `.text$x`**. All have COMDAT set;
588 have LINK's live bit `0x2000`, and 205 do not. Thus REF-dead executable
contributions participate in the sort. Non-executable GUIDs, constants, BSS,
and the data-only dectables TU supply no entries here. They could influence
ICF indirectly if they changed extraction of code-bearing library members.

[EVIDENCE] The ICF pass at `0x431fb0` calls the imported qsort with comparator
`0x432130`. Its key is ascending **effective contribution raw size**
(`CON+8 - CON+0x14`), then unsigned 16-bit relocation count (`CON+0x24`);
the special synthetic-module case substitutes zero relocations. The key
includes alignment bytes present in the contribution. It has no function
name, lexical symbol order, address, or module-name tie breaker. Names can
affect admission/extraction elsewhere; they are not sorting tie breakers.

[EVIDENCE] VC5 MSVCRT's unstable QSORT.C uses a middle pivot and shortsort for
at most eight elements. Replaying that implementation reproduces **all 793
positions**, not only winners, in the baseline, extern, both one-factor links,
default-library link, and the preserved-sound controls. Within each size/count
bucket LINK scans forward (`0x432970`), skips already folded/ineligible
contributions, tests section-selection/body/relocation compatibility, and
folds later equivalents into the first eligible equivalent. Its fold link is
`later.CON+0x10 = earlier.CON`; `0x4000` marks the later contribution folded.
Further ICF passes can resolve relocation-dependent equivalence. Matching
size/count alone is not equivalence. The observer records 14 actual folds in
seven classes in each endpoint, all agreeing with this rule.

[EVIDENCE] Full trace/replay data: [causality.json](causality.json),
[analysis.json](analysis.json), [facts.json](facts.json),
[link-icf-disassembly.txt](link-icf-disassembly.txt).
`build/w-sb1icf-trace-{base,extern}/historical-host.json` contains module,
contribution and fold records. `histlink-telemetry.diff` documents the private
in-memory observer changes. Module/CON pointers are process addresses and
are not cross-run identities; archive offsets are compared only for archive
members, not command-line modules.

## Exact deciding input difference

[EVIDENCE] All 793 baseline/extern keys and identities agree except these
two adjacent swaps (indices are zero-based):

| Slots | Baseline key/name | External key/name |
|---|---|---|
| 358,359 | EnumProviders `(80,5)`, callback `(112,0)` | callback, EnumProviders |
| 364,365 | EnumSessions `(176,8)`, callback `(128,0)` | callback, EnumSessions |

[EVIDENCE] Their object sections are 49/51 and 61/63, respectively. Ordinary
external definitions become ready before their callers under BLUPI-LAY-04.
GetHide and GetNbSessions remain in initial slots **308 and 366** in both
builds. The four getters remain at 360/381/382/431; the filters at 559/616;
the two `_strncnt` bodies at 646/728. None is moved as an input. Their
**sorted** positions change because the key sequence elsewhere changes.

| Linkage control | Hide winner | Four-way getter | Filter | `_strncnt` | Event tiny-method label |
|---|---|---|---|---|---|
| Both static | jauge | sound | frame | aw_map | PlayMove |
| Providers external only | jauge | sound | frame | aw_map | PlayMove |
| Sessions external only | network | network | ehvecdtr | aw_cmp | PlayUp |
| Both external | network | network | ehvecdtr | aw_cmp | PlayUp |

[EVIDENCE] The two middle rows were first predicted in the model and then
compiled/linked as real controls. Their entire sort vectors agree with the
prediction. Button's empty return and `_CallMemberFunction0` survive in
every row. Partial-linkage controls are not proposed source corrections:
both original callback/caller sequences require the full external candidate.

| Input category | What this experiment establishes |
|---|---|
| Object order | Unchanged. Every game module remains in the current oracle-anchor order. An alternative original TU partition/dead-input placement is not recoverable from this experiment. |
| Section order | The sessions section swap is necessary and sufficient for the five baseline-to-extern flips with the other 791 positions fixed. Providers order is inert for these winners. |
| Section/symbol names | The comparator never reads them. No name or symbol-table tie-break hypothesis is needed. Changing symbol order could still affect library extraction in a different input. |
| COMDAT population | Count, identities, sizes and relocation counts are unchanged. Therefore a population change is **not** the cause of this regression. Unknown original dead code can still change the original permutation. |
| Data | No non-executable data contribution occurs in this ICF vector. Moving opaque BSS or the data-only TU supplies no direct ICF lever. |
| CRT/library order | The complete loaded module/member sequence, archive offsets/stamps, and both import-sort input/output vectors agree between endpoints. IDE default-library selection gives a byte-identical extern image. |

## Remaining original-survivor differences, including all alias groups

[EVIDENCE] Locations below are RVAs. Both the fresh sandbox and preserved-sound
links have these same code placements. Actual winner identities come from
fold events, not MAP ordering of aliases.

| Class | Oracle physical band / RVA | Baseline survivor / RVA | Both external survivor / RVA | Remaining question |
|---|---|---|---|---|
| GetHide / GetNbSessions | jauge / `0x1efc0` | GetHide / `0x1efd0` | GetNbSessions / `0x20c80` | Wrong owner and RVA; sessions swap reverses sorted ranks 181<187 into 196<207 in network's favor. |
| Four empty returns: button/menu/jauge destructors, OutputDebug | menu / `0x1f010` | button / `0x2a30` | button / `0x2a30` | Wrong owner in both; button remains sorted rank 165. |
| GetNbProviders / GetTrueColorBack / GetTrueColorDecor / GetEnable | sound / `0x22dc0` | GetEnable / `0x22dc0` | GetNbProviders / `0x20a70` | Wrong owner and RVA; first sorted sound rank 177 becomes network rank 169. |
| ArrayUnwindFilter / FrameUnwindFilter | ehvecdtr / `0x243f0` | frame / `0x27390` | ehvecdtr / `0x243f0` | Corrected by the sessions swap; not remaining debt. |
| Two static `_strncnt` bodies | aw_map band / `0x285c0` | aw_map / `0x285c0` | aw_cmp / `0x2b900` | Wrong physical band; initial slots 646/728 unchanged, first sorted rank changes from 646's body to 728's body. |
| BuildUp / PlayDown / PlayMove / PlayUp | event / `0x1b030` | PlayMove / `0x1b040` | PlayUp / `0x1b040` | Label is not identifiable from the stripped executable: four identical adjacent sections in one TU. +16 placement is upstream empty-return debt. |
| Three `_CallMemberFunction*` bodies | trnsctrl / `0x23e20` | function 0 / `0x23e20` | function 0 / `0x23e20` | Actual ICF, unchanged. Original individual label is not identifiable from identical adjacent sections. MAP display order does not establish another survivor. |
| `__alloca_probe` / `__chkstk` | chkstk / `0x2b630` | `0x2b630` | `0x2b600` | **Not ICF:** two symbols at offset zero in one ordinary non-COMDAT section. -48 is the removed aw_map slot before chkstk, restored later in aw_cmp. |

[EVIDENCE] Read-only archive inspection in [crt.json](crt.json) proves that
`__alloca_probe` and `__chkstk` share section 1, offset 0, flags `0x60500020`.
The three trnsctrl sections are separate 7-byte COMDATs and fold in the live
trace. The two full 48-byte `_strncnt` sections have no relocations and a
single unmasked oracle hit at `0x285c0`; neighboring CRT member placement
identifies the aw_map band, rather than the indistinguishable body's bytes
alone proving a source filename. These are static diagnostics, not new CRT
acceptance certificates.

## Fresh layout measurements and guarded-sound controls

[EVIDENCE] All images have the oracle's `.text` RVA and virtual size (176,019
initialized bytes). First CRT `_strstr` remains `0x238b0`. Fresh links have
the same sandbox sound-path gate INCOMPLETE and `.data` +32. Preserved-sound
controls reuse attested historical objects and pass the relink gate; they
are not represented as fresh compiles.

| Historical-host input | At original RVA | Byte-exact functions at linked RVA | Same-offset `.text` bytes | Entry |
|---|---:|---:|---:|---|
| Fresh baseline | 131 | 275 | 54,320 | `0x24d20` |
| Fresh providers-only control | 133 | 275 | 54,507 | `0x24d20` |
| Fresh sessions-only control | 48 | 275 | 39,600 | `0x24d40` |
| Fresh both external | 48 | 275 | 39,604 | `0x24d40` |
| Fresh both external, IDE default libs | 48 | 275 | 39,604 | `0x24d40` |
| Attested-sound guard baseline | 131 | 283 | 54,919 | `0x24d20` |
| Same attested inputs, header-dependent candidate objects | 48 | 280 | 40,064 | `0x24d40` |

[EVIDENCE] Guard hash reproduced exactly:
`557ca92e6ec8c71cfaf5e9f2d55969d3ebcd614c771c9010e074c6ad82ba3533`.
Corresponding extern hash:
`8791bd46d2ef75d34042e8507340dad0813d146c0603bd5063c033dfefdecf6a`.
Fresh baseline/extern hashes and all section/function layout fields are in
`facts.json`. Entry equality after the corrected filter is real, but does not
make this a qualifying layout improvement. Same-offset bytes and function
placement counts are layout diagnostics, never whole-image strict proof.

## All nine verifier runs: additional acceptance obstruction

[EVIDENCE] Each baseline and candidate was compiled fresh through canonical
verify/msvc. Both callback declarations and definitions change; all **12**
transitively header-dependent TUs use private header/source copies. Local
angle includes become quoted includes to prevent the canonical include root
from shadowing the candidate. Sound and other unaffected TUs remain canonical.
All nine pairs have **460 unchanged function payloads and symbolic relocation
tuples**. No candidate is accepted or promoted.

| Target | Baseline / extern exit | Complete per-symbol function records identical? | Certificates |
|---|---|---|---|
| sb1/blupi/us | 0 / 0 | Yes | Both pass |
| sb1/blupi/de | 0 / 0 | Yes | Both pass |
| sb1/blupi/fr | 0 / 0 | Yes | Both pass |
| sb1/eggbert/en | 0 / 1 | No: two getter records | All pass |
| sb1/eggbert/de | 0 / 1 | No: two getter records | All pass |
| sb1/eggbert/es | 0 / 1 | No: two getter records | All pass |
| sb1/eggbert/fr | 0 / 1 | No: two getter records | All pass |
| sb1/eggbert/it | 0 / 1 | No: two getter records | All pass |
| sb1/eggbert/pt | 0 / 1 | No: two getter records | All pass |

[EVIDENCE] Primary complete per-symbol record files are byte-identical,
SHA-256 `7aee281609454ca58b740e21d6759d9e7e9c9980aaebfbfcb720e4668da1b14f`.
The files are `function-records-by-symbol.json` in each verifier output.
This comparison retains all function fields, including matches, relocations,
status, placement basis and disassembly diagnostics. Raw results.json envelopes
contain different object paths and enumeration order, so literal whole-file
results.json byte identity is not claimed. No executable-byte equality is
inferred from this record serialization.

[EVIDENCE] In all six Eggbert candidates, GetNbSessions changes placement
basis from `forced by --at` to `placed by TU order`, at its unchanged single
oracle hit (English `0x20be0`). This makes it eligible as an owner in
cmpfunc.py's cross-TU fold loop, specifically the owner-location condition
around lines 1720-1724. GetHide changes STRICT -> FOLDED into GetNbSessions;
GetNbSessions remains FOLDED into GetHide. The reports now contain a cycle.
Only those two complete function records differ. The accepted gate rejects
GetHide; no byte, relocation or matched RVA changes. This is demonstrated
verifier order dependence, not a strict result to bypass by shuffling source.
The pinned comparator and inventories were not modified.

[EVIDENCE] All baseline runs and the three Blupi candidates explicitly report
sound-path INCOMPLETE. Eggbert candidates return for the getter regression
before printing that final sound notice; their compile logs still attest the
same unavailable `F:\Profil\sound.cpp` mapping. The expected sound limitation
does not excuse their additional accepted-function regression. All certificate
checkers finish successfully (Eggbert DoInit remains RELOCATIONS-VERIFIED,
excluded from STRICT/EQUIVALENT as in its baseline).

## What would decide the remaining inputs

[NEGATIVE] There is no qualifying change among the evidenced linkage
correction and ordinary IDE default-library selection. The original ordering
of live functions is already constrained by unique oracle anchors. The only
other file-static functions are FinishObjects and DoInit; their observed
positions already agree with the original. No additional BLUPI-LAY-04 order
correction is evidenced by that census.

[HYPOTHESIS] The original has a different executable contribution population
or order outside the two corrected pairs, or a different historical LINK/CRT
implementation. The observed swaps disprove the stronger idea that an ICF
winner diagnoses population differences specifically: an unchanged population
with two changed keys in its input order changes several winners nonlocally.

[EVIDENCE] Both complete loaded inventories remain 268 stamp-less modules plus
one resource module, against the oracle's 270 plus one. Rich's net +2 does not
name those inputs or establish that either contributed any code. Existing
FIFO/removed-body/alias permutations in LINK_ICF.md are not original-source
evidence and are not adopted or reintroduced. The ftime/mkdir and library-order
experiments already contradict other original CRT/hint-name positions; an ICF
win alone would not remove that contradiction.

[HYPOTHESIS] A precise recovery needs the original project/make response and
its full object/archive list; original network/header source; original objects
or a map/PDB exposing COMDAT/section order; or independently justified original
dead-code/TU ancestry. Those inputs would decide all four remaining wrong
physical owners by the validated forward rule. Source/project evidence is
especially necessary for REF-dead sections, which the final PE omits. A map
with symbols is needed to distinguish equal adjacent same-TU labels at all.
The exact original LINK build and host CRT would settle the implementation
alternative: PE 5.10 alone does not prove SP3 7303; the previously documented
unavailable 7287 possibility is still open, not tested or asserted here.

[EVIDENCE] Separately, a supervisor-approved target-wrapper/ownership correction
would be needed to remove the demonstrated Eggbert verifier cycle before
claiming unchanged nine-target acceptance. This worker has no authorization
to change canonical tools or certificates and supplies no such patch.

## Reproduction, boundaries and delivery

[EVIDENCE] Integration source diff: [extern.diff](extern.diff) (four storage
class deletions only). `providers-only.diff` and `sessions-only.diff` are
mechanism controls. They are **not** acceptable partial source fixes. The
private include rewrites are experiment isolation, not integration edits.

Run serially from `D:/Prog/blupi_recon`:

```powershell
.pyenv\python.exe scratch/w-sb1icf/run.py prepare
.pyenv\python.exe scratch/w-sb1icf/suite.py
.pyenv\python.exe scratch/w-sb1icf/run.py relink sb1/blupi/us link-base build/w-sb1icf-us-base/objects
.pyenv\python.exe scratch/w-sb1icf/run.py relink sb1/blupi/us link-extern build/w-sb1icf-us-extern/objects
.pyenv\python.exe scratch/w-sb1icf/trace_link.py base
.pyenv\python.exe scratch/w-sb1icf/trace_link.py extern
.pyenv\python.exe scratch/w-sb1icf/causality.py
.pyenv\python.exe scratch/w-sb1icf/run.py verify sb1/blupi/us us-providers-only providers-only
.pyenv\python.exe scratch/w-sb1icf/run.py relink sb1/blupi/us link-providers-only build/w-sb1icf-us-providers-only/objects
.pyenv\python.exe scratch/w-sb1icf/run.py verify sb1/blupi/us us-sessions-only sessions-only
.pyenv\python.exe scratch/w-sb1icf/run.py relink sb1/blupi/us link-sessions-only build/w-sb1icf-us-sessions-only/objects
.pyenv\python.exe scratch/w-sb1icf/run.py relink sb1/blupi/us link-extern-defaultlibs build/w-sb1icf-us-extern/objects --default-libs
.pyenv\python.exe scratch/w-sb1icf/guard.py
.pyenv\python.exe scratch/w-sb1icf/run.py relink sb1/blupi/us link-guard-base build/w-sb1icf-guard-base/objects
.pyenv\python.exe scratch/w-sb1icf/run.py relink sb1/blupi/us link-guard-extern build/w-sb1icf-guard-extern/objects
.pyenv\python.exe scratch/w-sb1icf/recheck.py
.pyenv\python.exe scratch/w-sb1icf/analyze.py
.pyenv\python.exe scratch/w-sb1icf/crt.py
.pyenv\python.exe scratch/w-sb1icf/facts.py
```

[EVIDENCE] `run.py` refuses an existing verifier output directory. For a new
compile, choose a fresh label. Exact expanded verifier argv, elapsed time, exit
code and full logs are in `verify-*.command.json`/`verify-*.log`; links have
matching `relink-*` receipts, response files and build/layout/gate JSON.
`suite.json` and `recheck.json` retain all nine-target comparisons. Guard input
hash/source provenance is in `guard-inputs.json`; no canonical sound object
was rebuilt, overwritten or patched.

[EVIDENCE] The stock verifier CLI only permits `build/verify-NAME`. To honor
the worker output prefix, `run.py` invokes canonical main with a directory
creation hook pointing at a fresh `build/w-sb1icf-*`; it does not alter compile,
comparison, regression or certificate logic. Python bytecode writes are
disabled in the worker drivers and children.

[EVIDENCE; operational deviation] Initial secondary runs also refreshed the
stock cmpfunc_target.py wrapper's generated inventory copies under
`build/targets/<target>/root/oracle/{functions,tu_map}.csv`, outside the worker
prefix. Their contents are the unchanged canonical inventories. This side
effect was discovered during tool review. `cmpfunc_target_local.py` now
relocates only that generated shadow under `build/w-sb1icf-inventory`, leaving
the pinned comparator intact. `recheck.py` reran the canonical comparisons and
certificates on all 16 secondary fresh object sets with the isolated shadow;
every complete results list is identical to its initial run. The unavoidable
msvc wrapper's shared `build/tmp` and `build/locks` were used normally. No
outside generated inventory was deleted or manually modified to conceal this.

[EVIDENCE] Final audit HEAD `e12537017bb1a839201703a313184288ed7a0f97`, tracked
git status clean. Canonical source/manifests/tools/docs/catalog and certificate
hashes are recorded in `facts.json`; this worker makes no tracked changes,
commit, original/rebuilt game execution, or asynchronous compile/link run.
Every historical launcher reports process wait 0, zero active job processes,
and unchanged tool files. All deliberate worker artifacts are below the two
requested prefixes, with the stock-tool side effects disclosed above.

## Solver — supervisor follow-up, 2026-10-24

[EVIDENCE] **All requested physical survivors can be reproduced by real C++
inputs.** Restore eight REF-dead CDecor methods, in their historical definition
positions, together with both external callbacks. The actual LINK trace has
801 entries and exactly the predicted input order and complete qsort output.
All eight additions are REF-dead in LINK's trace. No dummy reference, extra TU,
library permutation, `/ORDER`, output patch, or forced byte was used.

[HYPOTHESIS] This is a constructive source/input witness, **not recovery of
the original dead source**. Its helper definitions come from this repository's
earlier SB2-derived reconstruction. Their presence and tail-field layout in
SB1 are not independently proven. The current, binary-validated SB2 forms
provide a useful negative control below. The successful witness also changes
generated symbolic relocation names and therefore fails the saved certificate
pins. It is not an integration recommendation under the current SB2-inheritance
and unchanged-report/certificate rules.

### Search space, exact minimal vector families, and their limits

[EVIDENCE] `solver.py` starts exclusively from the **both-external** 793-entry
vector. It constrains GetHide→jauge, empty return→menu, getter→sound,
filter→ehvecdtr, `_strncnt`→aw_map, and keeps `_CallMemberFunction0`. Its
physical-band mode permits any of the indistinguishable adjacent event
methods, while retaining an event-method body. Its conservative mode also
keeps today's PlayUp label. The successful real source witness satisfies
**both** modes; no previously correct class is sacrificed.

[EVIDENCE] There are 257 distinct existing comparison keys. Positive integer
size/16-bit relocation keys partition into **429 nonempty order cells**
(equal-key cells and intervals between keys). Inserting one distinct unknown
body anywhere in a cell gives the same comparison outcomes. The search tests
all 794 insertion boundaries in every cell: **340,626 cases**. It also tests
all **793 deletions**, **628,056 single-entry moves**, and **309,291 two-entry
deletions** that retain the six specified physical/retained-label owners.
All four event members are kept available unless the tested edit removes
individual members; no solution can discard their whole class.

[EVIDENCE] These are minima in **vector operations**, not in C++ edits: one
definition can emit associative funclets, and one source/reference change can
extract several library members. The eight-method minimum below applies only
to the explicitly enumerated historical twelve-method source pool.

[EVIDENCE] The fast search performs exactly the original partition/shortsort
swaps, omitting only recursion over key ranges containing no constrained class.
Its constraint signatures were checked against full replay on the real
endpoint plus 600 seeded insertion/removal/move cases. Every shortlisted real
link is checked with full, unpruned replay. Generated vectors are diagnostics
only and are never supplied to LINK.

| Edit family | Exhaustive minimal result | Constraint/evidence interpretation |
|---|---|---|
| One insertion | Any key in `[ (1,0), (6,1) )`, exactly `(7,0)`, or `( (7,0), (16,0) )`, inserted before any original slot **322..380**, restores all physical bands. | 59 positions per key cell. None keeps the arbitrary PlayUp label; all retain an identical event body. Relocation bounds/body legality still have to be established by a real object. |
| One deletion | No solution. | Includes every current executable contribution, live or dead. |
| One move, all current labels preserved | `_DDSetDebug`, original slot **37**, moved to reduced-vector slot **281..307**. | Key `(16,1)`. Destination is between the event tail and GetHide, rather than Microsoft's ddutil block. Its retained oracle body at `0x3100` excludes this as an original placement. |
| One move, physical bands | Also button's destructor, **15→381** in the reduced vector. | Its own definition position is lost to ICF and cannot be inferred from its final bytes. Both source ancestries place it in button.cpp, and pixmap has no CButton reference that would explain deferred inline emission there. No source evidence supports this move. |
| Two deletions | **21** physical-band solutions; only one preserves all current labels: MouseBackClear **418** plus `_srand` **560**. | Every pair removes a retained game function and a retained CRT function/alias. No pair consists of two REF-dead game functions. These contradict independently retained bodies or leave real call references undefined. |
| Whole nonempty-module removal/move | No removal solution among **231** executable-COMDAT-bearing modules. Five move families, eight destinations total. | Move button, ddutil, decmove, movie, or frame. Game moves contradict live TU anchors. Moving frame to reduced boundary 519 or 531 gives the physical bands but changes the event label and moves independently located live CRT routines. |

[EVIDENCE] All move destination coordinates above mean “remove the source,
then insert before destination index.” Insertion coordinates refer to the
unaltered 793-entry vector. Slot 322 is CMenu::IsError; 380 is CPixmap::Flush.
The complete exact families, including second-deletion intervals and member
destinations, are in `solver-{constraints,insert,remove,remove2,move,members}.json`.
The whole-module search uses **module indices**, not equal DLL names: import
archive members with the same name remain separate inputs.

[EVIDENCE] The single-insertion solution requires an effective size **below
16**; the ordinary VC5 C++ functions examined here have at least a 16-byte
contribution. All SP3/SP2/RTM LIBC members were read: the only sub-16 executable
COMDATs in each archive are the same three existing seven-byte trnsctrl
helpers. The six-byte, one-relocation import-thunk key `(6,1)` is explicitly
**not** a solution. No unloaded tiny CRT helper supplies this family, and
ordinary library extraction puts library contributions after the game objects,
outside 322..380. Adding another copy of a same-name COMDAT is not adding a
new arbitrary ICF entry.

[EVIDENCE] A generic unknown body is assumed distinct from the known classes.
Real body equivalence, COMDAT selection, eligibility and liveness must be
checked afterward. A live added body can change section extents/RVAs even
when these survivor labels agree. The successful source witness below avoids
that limitation because LINK confirms every addition dead and every mapped
game RVA equal. An unused file-static function normally never reaches LINK
under BLUPI-LAY-04; ordinary external members do, without dummy references.

[NEGATIVE] Conditional SP2/RTM member-key replacements, retaining today's
loaded-member order, alter three/seven keys respectively and fail the target
constraints. SP3 archive reconstruction reproduces every current CRT COMDAT
block exactly. This is explicitly **not** a full alternate-library extraction
model or proof of code equivalence. See `solver-libraries.json`. No alternate
CRT was linked on the strength of a failing model.

[EVIDENCE] The only remaining file-static functions, FinishObjects and DoInit,
were compiled with both their declarations and definitions external. Storage
changes 3→2, but their sections remain 38 and 54; all function bytes, symbolic
relocations and section order are identical. They were already defined after
their first callers. See `solver-static-probe.json`. No further evidenced
BLUPI-LAY-04 correction is available from that census.

### Mapping the families to real source

[EVIDENCE] Eight complete contexts were compiled through tools/msvc.py:
historical Planet Blupi `b0270ab` fifo (4 COMDATs), arrange (14), obstacle (45),
action (4), sb2-decomp wave (3), two historical SB1 decmove contexts (51 and
43), and historical text (14). Their **real** keys and exact compiler argv are
in `solver-source-keys.json`, with transcripts in
`build/w-sb1icf-source-keys/`. The parent contexts are ancestry probes, not
validated SB1 layouts.

[NEGATIVE] None of the five complete predecessor/SB2 TUs, inserted at **any**
ICF-vector position, satisfies all target bands. None of their noncolliding
one/two-function subsets at a game-module boundary does either. The three
removed big-number text functions and all subsets of the four later-removed
decmove helpers also fail at their historical definition positions. Old source
and history patches are retained under `solver-sources/`; no source/data was
added merely to adjust Rich.

[EVIDENCE] The context before `60c7fab` contains twelve removed helpers, with
**all 39 current decmove function payloads and symbolic relocations unchanged**
when compiled there. Exhausting their 4,095 nonempty subsets, retaining their
historical definition order and every existing function key, yields **42**
solutions. Every solution has exactly **eight** additions and preserves all
seven conservative class labels. Every solution contains FlushBalleTraj,
FlushMoveTraj and AscenseurShift; no solution contains the three-argument
ObjectStart wrapper.

[EVIDENCE] Requiring normal CDecor function-dependency closure leaves **one**
of those 42 sets. StartSploutchGlu needs the absent three-argument ObjectStart;
DynamiteStart needs absent BlupiRect/BlupiDead; MoveObjectFollow needs absent
BlupiRect. The remaining set is the complete trajectory six plus the two
elevator helpers. It needs no undefined-method stub or extra reference:

| Added definition, historical order | Measured historical-context key | Current SB2-form key compiled against the scratch SB1 view |
|---|---|---|
| FlushBalleTraj | `(32,0)` | `(32,0)` |
| SetBalleTraj | `(96,0)` | `(80,0)` |
| IsBalleTraj | `(160,0)` | `(112,0)` |
| FlushMoveTraj | `(32,0)` | `(32,0)` |
| SetMoveTraj | `(96,0)` | `(80,0)` |
| IsMoveTraj | `(160,0)` | `(112,0)` |
| AscenseurShift | `(48,0)` | `(48,0)` |
| MoveAscenseurDetect | `(224,1)` | `(192,1)` |

[EVIDENCE] All eight current SB2 definitions have accepted bodies in
`games/sb2/recon.json`: the trajectory six at `0x14c90..0x14de0`,
AscenseurShift `0x18960`, MoveAscenseurDetect `0x19520`. This demonstrates
real family functions, not invented helpers. Source paths, accepted sizes and
coordinates are in `solver-sb2forms-provenance.json`.

[HYPOTHESIS] That does not prove their old reconstructed forms existed dead
in SB1. In particular, the successful history-view arrays are the inherited
**1,251-byte** form; SB2 binary evidence later establishes **1,300-byte** maps
in a different class location. The scratch SB1 candidate refines only the
existing proven 100,044-byte opaque tail with TimeNoAsc/two maps and its
remaining opaque extent. Those names, offsets and array lengths have no SB1
access evidence. The independent total class storage size is retained; no
runtime caller is added. This remains an explicitly unproved view of storage.

[NEGATIVE] A stronger source control uses the **current SB2 forms** and
1,300-byte arrays, preserving the same eight SB1-history definition positions.
It keeps six target labels but selects **aw_cmp**, not aw_map. The actual
historical link agrees with this prediction. Moving the six trajectory
definitions to their SB2 decblupi grouping, before the next shared function
without reordering existing SB1 code, gives the same failing survivor
signature in the model. A separate mixed-key screen (eight current measured
keys plus the four historical measured keys) has 24 solutions but none closed
under the known CDecor dependencies. This bounded result is not a search of
every possible later SB2 body or missing TU.

[NEGATIVE] The nine SB1 baseline reconstructed sets each contain 460 functions.
The positive verified-call census found no secondary-only witness for the 41
primary-link REF-dead game contributions. Both eight-helper candidates give
eight MISMATCH/unlocated new records in **every** SB1 target; no other SB1
build supplies a strict live helper body here. Missing witnesses do not prove
absence in unmatched callers or a different class layout. These limits are in
`solver-sb1-family-refs.json`, rather than an unsupported claim that no such
original function could exist.

### Real links, full replay, and acceptance

[EVIDENCE] The successful historical-form candidate is `solver-tail8.diff`.
It restores exact historical bodies and relative positions, seven missing
method declarations (AscenseurShift was still declared), the labelled
hypothetical tail view, and the four callback storage-class deletions.
Private local-include rewrites only isolate headers. No linkage or order option
differs from the baseline. Its 801-entry trace has all eight additions not live,
the same loaded module population, and the predicted 14 folds in seven classes.

| Historical-host input | At original RVA / 412 mapped | Same-offset `.text` bytes | Byte-exact functions at linked RVA | Gate |
|---|---:|---:|---:|---|
| Fresh static-callback baseline | 131 | 54,320 | 275 | INCOMPLETE sound path |
| Fresh both-external baseline | 48 | 39,604 | 275 | INCOMPLETE sound path |
| Declarations/tail-view only, no added bodies | 48 | 39,604 | 275 | FAIL certificate pins |
| Eight historical-form helpers | **412** | **174,934** | 356 | FAIL certificate pins |
| Eight current SB2-form helpers | **412** | **162,537** | 356 | FAIL certificate pins; aw_cmp remains |
| Attested-sound original guard | 131 | 54,919 | 283 | PASS |
| Same attested sound, historical-form helpers | **412** | **175,718** | 372 | FAIL certificate pins |

[EVIDENCE] Every row has the original `.text` RVA/virtual size. Both eight-helper
links put startup at the original `0x24d40`. The historical-form witness has:
GetHide `0x1efc0`, menu return `0x1f010`, sound getter `0x22dc0`, ehvecdtr
filter `0x243f0`, aw_map `_strncnt` `0x285c0`, event tiny body `0x1b030`,
trnsctrl `0x23e20`, and the ordinary chkstk aliases `0x2b630`.
The manifest has 413 entries; the layout tool uniquely maps 412 and omits
NetDraw from this metric. Thus 412/412 refers to mapped entries, not an
independent placement proof for every manifest entry.

[EVIDENCE] The declarations-only control is **whole-file byte-identical** to
the old extern candidate, SHA-256
`e8a601aaf3d60c5f7276d880be96df34d583459948f8e233738133628b281e2f`.
Thus the new dead contributions, rather than the changed header names or
data placement, cause the layout improvement.

[EVIDENCE] The attested-sound historical-form image SHA-256 is
`882bf00828a20e546619b1e7ed36a0d4c104c0bc3298e4b59cb9175fb09ebbec`.
It has the same **535,552-byte** file length and exactly **596 unequal bytes**:

| Location | Unequal bytes | Direct diagnostic attribution |
|---|---:|---|
| `.text` | 301 | 261 bytes in references that identify the same PE imports, plus ChangePhase's 32 and TreatEventBase's 8 nonrelocated differences |
| `.rdata` | 294 | Entirely inside the INT/IAT thunk arrays; zero other `.rdata` differences |
| `.data` | 0 | Full raw section byte-exact |
| `.rsrc` | 0 | Full raw section byte-exact |
| Outside sections | 1 | File offset `0x94`, Rich stamp-less count 268 versus 270 |

[EVIDENCE] This is an unnormalized whole-file comparison. Import-reference
attribution is read-only and does not alter code/IAT bytes. Rich's two unknown
loaded modules and the original import-sort input remain unresolved; neither
is repaired by restoring eight functions inside an existing object. Fresh
sandbox and attested-sound scores are kept separate. Full evidence is in
`solver-audit.json` and the four link directories' layout/build/host JSON.

[EVIDENCE] Both historical/current-form candidates were compiled fresh for
**all nine targets**. For each candidate, all original **460 raw function
payloads remain identical**. Each adds eight MISMATCH records. Generated
symbolic relocations change in 16 existing functions; full per-symbol records
change in 13 existing functions on Blupi and 12 on Eggbert. Every changed record
is retained, not normalized away, in `solver-suite.json` and
`solver-suite-sb2forms.json`.

| Targets, individually covered | Added records | Changed existing records | Raw payload changes | Accepted regressions relative to their manifest |
|---|---:|---:|---:|---|
| sb1/blupi/us, sb1/blupi/de, sb1/blupi/fr | 8 each | 13 each | 0 | 0 |
| sb1/eggbert/en, /de, /es, /fr, /it, /pt | 8 each | 12 each | 0 | Existing GetHide owner-cycle failure, one each |

[EVIDENCE] **Every official candidate verifier exits 1** at the ChangePhase
certificate pin. Seven added method declarations shift C1-generated identifiers
by 14: for example `_table$S33911→S33925`, `_cheat_code$S33910→S33924` and
`$L36000→$L36014`. The primary fresh proof still checks all 231/273
relocations and all semantic windows. Its current certificate dictionaries
differ from the saved certificates **only in payload_sha256**, which deliberately
binds raw bytes **and symbolic names**. Both primary pins fail. No hash
normalization, symbol renaming, pinned-file edit, or promotion is performed.
Literal unchanged reports/certificates are therefore not achieved.

[EVIDENCE] Canonical relink aborts at this pin before linking, even in its
diagnostic mode. `diagnostic_link.py` retains the failure as
`diagnostic-gate.json: FAIL_CERTIFICATE`, freshly runs the comparator against
the actual supplied objects, and invokes unchanged histlink with the original
response options/order for **layout inspection only**. Both fresh and attested
candidate accepted-function regression lists are empty; this does not override
their failed certificates. All tool processes finish with zero active jobs.

### Eggbert owner cycle: cause and an unapplied principled correction

[EVIDENCE] All six Eggbert targets inherit one shared base inventory,
`oracle/sb1-eggbert-en/tu_map.csv`, which currently calls the shared getter at
**`0x20be0` jauge.cpp**, with CJauge::GetHide as best_source. The original
neighbouring EnumSessions and GetSessionName bodies place this slot physically
inside the network band. The inventory's unique identical getter signature
does not identify the winning source copy. The primary US inventory's jauge
ownership at `0x1efc0` is not changed by this proposal.

[EVIDENCE] In cmpfunc.py's collect_global_state:

1. The unique GetHide hit remains `unique`, agreeing with the current row's
   jauge attribution.
2. The network copy lies outside that attributed TU. With static callbacks its
   same-TU ordering proof fails, so its forced accepted location is
   `forced by --at`. With external callbacks the original network sequence is
   now monotonic, and it becomes `placed by TU order`.
3. The fold loop accepts owner bases `unique` **or** `placed by TU order`
   (around lines 1720-1724). Each getter has verified callers, identical bytes
   and the same target. Each can therefore become the other's owner. The loop
   does not establish one independently owned root for the equivalence class.
   This produces GetHide→GetNbSessions→GetHide. A first-edge/skip-already-folded
   fix would merely make the result depend on traversal order.

[EVIDENCE, PROPOSAL ONLY] The smallest local data correction is one semantic
row in that shared tu_map.csv: attribute `0x20be0` to **network.cpp**, with the
neighbour-based evidence, and make all six target expectations:

- GetNbSessions **STRICT**, removing its inherited folded-into-GetHide entry.
- GetHide **FOLDED into GetNbSessions** at the same RVA/extent.

This transfers the one strict owner of a byte-identical class, rather than
weakening acceptance or changing an address. The unapplied artifacts are
`solver-eggbert-inventory.diff` and `solver-eggbert-owner-overrides.json`.

[EVIDENCE] Private inventory/manifest copies were tested with the **unchanged
pinned comparator** on all six fresh extern object sets. Exactly the two getter
records change; GetNbSessions becomes STRICT, GetHide becomes FOLDED, and all
six accepted-function regression lists become empty. Every target-local semantic
proof completes. Current/saved certificate dictionaries differ **only in
inventory_sha256**. This intentionally remains **FAIL_PIN_PROPOSAL_ONLY**,
not passing pinned acceptance. Original inventory files, targets.json, checkers
and certificates remain untouched. See `solver-owner-proposal.json` and each
private proposal directory's explicit gate/receipt.

[HYPOTHESIS, TOOL ALTERNATIVE] A general fold-root repair should form a body/RVA
class and select a single root from independent physical-TU ownership evidence.
At minimum, when a trusted TU row exists, an owner candidate should belong to
that TU; being a `placed by TU order` copy in a different TU should not make it
an independent owner. Otherwise leave ownership unresolved. That removes this
mutual-owner failure, but retaining the erroneous Eggbert row would still pick
the wrong physical band. The inventory correction is the narrower preferred
proposal. Any checker edit also changes pinned checker hashes and needs full
re-proving; no such edit is supplied or applied here.

### Reproduce the solver and remaining boundary

[EVIDENCE] Run serially from the repo root; the fresh-label restriction in
run.py still applies. All compiles use tools/msvc.py via the shown drivers,
with default job/slot caps and repository-local temp. The full compiler argv
and logs are saved, not just these abbreviated driver commands:

```powershell
.pyenv\python.exe scratch/w-sb1icf/solver.py insert
.pyenv\python.exe scratch/w-sb1icf/solver.py remove
.pyenv\python.exe scratch/w-sb1icf/solver.py remove2
.pyenv\python.exe scratch/w-sb1icf/solver.py move
.pyenv\python.exe scratch/w-sb1icf/solver.py members
.pyenv\python.exe scratch/w-sb1icf/source_census.py
.pyenv\python.exe scratch/w-sb1icf/compile_sources.py
.pyenv\python.exe scratch/w-sb1icf/source_models.py
.pyenv\python.exe scratch/w-sb1icf/library_models.py
.pyenv\python.exe scratch/w-sb1icf/static_probe.py
.pyenv\python.exe scratch/w-sb1icf/prepare_tail8.py
.pyenv\python.exe scratch/w-sb1icf/solver_suite.py
.pyenv\python.exe scratch/w-sb1icf/diagnostic_link.py solver-link-tail8 build/w-sb1icf-solver-us-tail8/objects
.pyenv\python.exe scratch/w-sb1icf/prepare_solver_guard.py
.pyenv\python.exe scratch/w-sb1icf/diagnostic_link.py solver-link-tail8-guard build/w-sb1icf-solver-tail8-guard/objects
.pyenv\python.exe scratch/w-sb1icf/trace_link.py solver-tail8 build/w-sb1icf-solver-link-tail8
.pyenv\python.exe scratch/w-sb1icf/run.py verify sb1/blupi/us solver-us-declarations-only solver-declarations-only
.pyenv\python.exe scratch/w-sb1icf/diagnostic_link.py solver-link-declarations-only build/w-sb1icf-solver-us-declarations-only/objects
.pyenv\python.exe scratch/w-sb1icf/prepare_sb2forms.py
.pyenv\python.exe scratch/w-sb1icf/solver_suite.py solver-tail8-sb2forms
.pyenv\python.exe scratch/w-sb1icf/extended_models.py
.pyenv\python.exe scratch/w-sb1icf/diagnostic_link.py solver-link-sb2forms build/w-sb1icf-solver-us-sb2forms/objects
.pyenv\python.exe scratch/w-sb1icf/owner_proposal.py
.pyenv\python.exe scratch/w-sb1icf/sb1_family_refs.py
.pyenv\python.exe scratch/w-sb1icf/solver_audit.py
.pyenv\python.exe scratch/w-sb1icf/final_receipt.py
```

[EVIDENCE] The proposal collector records pin differences instead of raising
on the first one so that all semantic proofs can be inspected. It writes no
replacement canonical certificate; its explicit proposal gate remains failed.
The diagnostic links likewise grant no acceptance. No original or rebuilt
game executable was run, and no background tool remains. This follow-up uses
the already-isolated target wrapper, so it adds no stock build/targets shadow
write outside the worker prefix; mandatory msvc temp/slot side effects remain.

[OPEN] The survivor/layout solver now has a full constructive witness. To call
it original-source recovery still requires independent evidence for the dead
SB1 helper roster/forms/TU grouping and its unaccessed field declarations.
Only then should the changed symbolic payload pins and, separately, corrected
Eggbert inventory pins be reviewed and re-proven. The remaining Rich +2 and
import ordering need original loaded-input/host evidence. No scratch source,
proposal, status, or certificate has been integrated. Final static audit HEAD
`aa3dfd685ac4374fb61a4f16ff72e87b7196a4a8`, tracked status clean.
`solver-final-receipt.json` verifies both nine-target result sets, all six
private owner proposals, unchanged hashes for all 64 initial `src/` files,
and `git apply --check -p0` success for both source diffs and the single-base-row
inventory proposal. The patches remain unapplied.
