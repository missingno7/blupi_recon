# Full relink and initialized data (sol-relink, 2026-10-17)

## Round 2: source reconstruction after 11af3b8 / c1b2c29

[EVIDENCE] **Partial delivery:** `scratch/sol-relink/round2/relink2.patch`.
It applies against c1b2c29 with plain `git apply --check` and changes six files:
blupi.cpp, decblupi.cpp, decmove.cpp, event.cpp, pixmap.cpp and pixtables.h.
Canonical sources, recon.json and compiler catalog were not edited. The patch
preserves all **402 active accepted entries (399 STRICT, three FOLDED)**.
Sound has the expected INCOMPLETE historical-path exception; measured text
coverage remains **110,720 / 141,488**, with no new unique code coverage.

[EVIDENCE] The final natural link has **zero duplicate symbols** and three
unresolved methods: **DecorNextAction, BlupiRect and BlupiDead**. The latter
two are exposed dependencies of retained, unmatched SB2 helper callers after
removing their unsupported decblupi definitions. There is no successful EXE or
layout.json. Actual section/accepted-function RVA deltas therefore remain OPEN.
No forced link, padding, renamed duplicate owner, replacement stub or changed
source order to conceal a context regression is delivered.

### Delivered changes and evidence

- [EVIDENCE] Decblupi's two channel-method copies and five unsupported bodies
  (BlupiIsGround, BlupiRect, BlupiAdjust, BlupiBloque, BlupiDead) are removed.
  Its **20 remaining definitions are all STRICT**, beginning with BlupiStep
  at `0x9280` and ending with IsVentillo at `0x10480`. The target BlupiStep
  ends immediately before GetPosDecor at `0xf540`; there is no intervening
  BlupiDead slot. The leading four helpers likewise have no space before the
  block's first method. These bodies have SB2 ancestry and no independently
  established SB1 out-of-line location. This is evidence against retaining
  these copies as SB1 definitions, not proof about every possible original
  inline helper's source spelling. GetBlupiChannelActual/Standard remain owned
  by decblock at `0x42e0`/`0x4310`. IsDeepWater stays STRICT at `0x10290`
  without extern probes or added declarations.
- [EVIDENCE] BackgroundCache calls in blupi/event are calls to the existing
  `CPixmap::Cache` at `0x21870` in the oracle. Accepted WindowProc independently
  binds its former BackgroundCache call to exactly that address. The initial
  Planet Blupi pixmap source (`b0270ab`) has Cache and no BackgroundCache or
  eight-argument CacheAll. The unreferenced SB2 LoadImageFromDisc body is removed
  from event; it has no oracle slot or incoming compiled relocation. This
  eliminates the CacheAll dependency without inventing a pixmap method.
- [EVIDENCE] GetTrueColorBack/Decor return the sole SB1 true-color word,
  `m_bTrueColorBack` at `+4`. Constructor/Create stores establish that offset;
  both corresponding oracle getter calls land on the four-byte
  `mov eax,[ecx+4]; ret` body at `0x22dc0` (16-byte object span). The independent
  CSound::GetEnable body owns that location. GetTrueColorDecor is **FOLDED**
  with a verified LoadImages caller and the independent sound object witness.
  GetTrueColorBack remains **RELOC-UNVERIFIED**: its unmatched ChangePhase
  caller does not supply the same independent folding proof. Neither adds
  unique bytes or a newly accepted function.
- [EVIDENCE] The existing CDecor::PlaySound target at `0x5800` ends with
  `ret 0x10`: sound, two POINT components and bLocal. Every placed live caller
  uses that three-argument source ABI. The inherited two-argument references
  occur only in unmatched StartSploutchGlu, DynamiteStart, MoveObjectFollow and
  BlupiDead; no separate target overload body is identified. The three retained
  decmove helper calls now pass FALSE explicitly. BlupiDead is removed with
  the unsupported decblupi bodies. [HYPOTHESIS] FALSE is the ordinary broadcast
  behavior used by the surrounding SB1 effects; no strict identity or original
  caller-byte match is claimed for those retained SB2 helpers. Their source
  references now resolve to the real four-word callee rather than a fabricated
  two-argument wrapper. This changes no accepted caller's bytes.

### Explo's physical extent

[EVIDENCE] The initializer now occupies exactly **648 bytes / 324 shorts**,
RVA **`0x7afd8` through `0x7b25f`**, SHA-256
`ef5159e7a1a74477e84ed92ee707a014349da765e52794aeaa63b598a8b728cc`.
The independently located MouseHotSpot `int[8]` starts at `0x7b260`; the
preceding nonzero short at `0x7b25e` is 124. Thus the typed atlas payload
reaches that boundary without borrowing the next table's value 95.
The stored count is 54, but the available data contains 53 complete six-short
rows and five fields of the last row. The actual DrawIcon instructions read
height from `base + rank*12 + 12`; rank 53 reads **`0x7b260`**, the next object.
The patch preserves the count and the bounded numeric data; it does not append
95, normalize the count, pad the array, or overlap MouseHotSpot.

[HYPOTHESIS] A missing final initializer field/count mistake is the likely
source explanation. The exact 1998 declaration spelling cannot be recovered
from these addresses. The physical boundary and the cross-boundary target
read are measured independently of that explanation. Full-image data ordering
must eventually reproduce the neighbour relationship naturally.

[EVIDENCE] `datacmp.py` verifies all 648 new bytes without masking. Final
initialized data coverage is **267,474 / 334,706 (79.91%)**: `.data`
267,458 / 329,728 and `.rdata` 16 / 4,978. The report has 456 EXACT symbols;
ambiguous/unlocated data and the expected sound-path mismatch add no coverage.
The only proposed header delta is the numeric explo initializer in
pixtables.h; method declarations, constants outside that table, member layouts
and ABI are unchanged.

### Rejected context repairs / Compiler findings

All measurements used manifest SP3 flags, canonical headers unless explicitly
identified below, no PCH, and msvc.py/fnprobe.py/verify.py at default limits.
KB hints for IsDeepWater, GetBlupiInfo and MockeryDetect, and VC5-CTX-01, were
consulted. Artifacts are under `scratch/sol-relink/round2/measurements/`.

- [EVIDENCE; VC5-CTX-01..04; Blupi-specific] Removing only the two channel
  copies makes IsDeepWater evaluate x before y (80-byte span, 66 fixed-byte
  differences after the instruction-order shift). Removing the complete
  unsupported decblupi helper set restores its original y/x order and every
  remaining method's STRICT result. Removing just the unreferenced ground/
  adjust/bloque helpers, or just the dynamite/death component, is insufficient.
  The final improvement comes from source-backed TU cleanup, not the known
  six-extern diagnostic threshold.
- [EVIDENCE; VC5-CTX-01..04; negative] Removing StartSploutchGlu, DynamiteStart
  and MoveObjectFollow from decmove removes the orphan references to BlupiRect/
  BlupiDead, but regresses accepted MockeryDetect (`0x13320`): 544 -> 528-byte
  extent, with changed division/evaluation order at both coordinate lookups
  (175/191 aligned oracle instructions). Deleting the six unreferenced
  trajectory methods, the unused three-argument ObjectStart wrapper, and the
  extra AscenseurShift/MoveAscenseurDetect bodies does not repair it. Nor do
  scalar/POINT coordinate locals, C89 local placement, or an explicit stdlib
  include. A POINT-construction variant worsens the size to 512. These source
  deletions/rewrites are rejected; the final patch retains the existing
  decmove bodies and only corrects their unresolved sound-call ABI.
- [EVIDENCE; VC5-CTX-01..04; negative] DecorNextAction is inserted at its oracle
  source position, before KeyChange, with the recovered table in the data TU.
  Its 272-byte extent has zero fixed differences against the 269-byte body at
  `0x5670`, but GetBlupiInfo loses STRICT with four operand displacement bytes
  at `+0xe,+0x14,+0x2e,+0x34`: both commutative OR pairs swap load order.
  Restoring the direct SB2 GetBlupiInfo expression, SB2 action-local scopes,
  Planet-style includes, and putting the actual numeric table definitions at
  the head of the owner TU are inert. None is retained. The existing study's
  arbitrary extern/function count probes were not repeated or delivered.
- [EVIDENCE; VC5-CTX-01..04; negative] A coherent private header trial removes
  the same obsolete helper/API declarations from decor.h/event.h/pixmap.h.
  It reduces GetBlupiInfo to two differing bytes, but leaves MockeryDetect
  wrong and regresses IsDeepWater and MoveObjectSort. No trial header cleanup
  or synthetic per-TU header view is delivered. Further arbitrary count/order
  permutations have no source evidence and were not pursued.

[OPEN] The remaining natural-link blockers are genuine reconstruction/context
debt. Integrating the oracle-ordered DecorNextAction must first restore
GetBlupiInfo. Removing the unsupported decmove callers must first restore
MockeryDetect. Their source-counter/allocator context remains unidentified
after these bounded, source-supported tests; source fillers are not a fix.

### Reproduction and artifacts

```powershell
.pyenv\python.exe tools\verify.py --tu src/blupi.cpp=scratch/sol-relink/round2/delivery-src/blupi.cpp --tu src/event.cpp=scratch/sol-relink/round2/delivery-src/event.cpp --tu src/pixmap.cpp=scratch/sol-relink/round2/delivery-src/pixmap.cpp --tu src/decblupi.cpp=scratch/sol-relink/round2/delivery-src/decblupi.cpp --tu src/decmove.cpp=scratch/sol-relink/round2/delivery-src/decmove.cpp --tu src/dectables.cpp=scratch/sol-relink/round2/delivery-src/dectables.cpp --build-dir sol-relink2
git apply --check scratch/sol-relink/round2/relink2.patch
.pyenv\python.exe tools\relink.py --objects-dir build/verify-sol-relink2/objects --out-dir build/relink/round2
.pyenv\python.exe tools\datacmp.py --objects-dir build/verify-sol-relink2/objects --comparison build/verify-sol-relink2/results.json --source-dir scratch/sol-relink/round2/delivery-src --json-out build/relink/round2/data.json --csv-out build/relink/round2/data.csv
```

[EVIDENCE] The final verifier exits zero with no accepted regression and the
expected sound INCOMPLETE note. Dectables.cpp is a byte-identical candidate
copy solely to select the private corrected pixtables.h; it has no source
patch hunk. `gate-delivery.log`, `delivery-files.json`, `explo-evidence.json`,
the focused scripts/variant metrics, and `build/relink/round2/{gate,build,data}.json`
preserve inputs and results. No game/linked image was executed, and no commit
or background process was created.

## Round 1 historical report

The sections below retain the first delivery's measurements, before supervisor
integration and POINT-inline cleanup. Round 2 above is the current result.

## Result

[EVIDENCE] `tools/relink.py` builds the 20 manifest TUs serially through
`msvc.py`, then invokes the pinned SP3 linker without executing the result.
The canonical link stops with **16 distinct multiply defined symbols and
10 unresolved symbols**. The delivered data-only candidate reduces the latter
to **seven**. Neither run produces a successful `BLUPI.EXE`; there is no
whole-image match or measured linked section shift to report. Failure removes
any partial image/empty map rather than leaving a stale successful artifact.

[EVIDENCE] All **402 active accepted entries (399 STRICT, three FOLDED)**
retain their gate result, address and size in the available sandbox check.
Sound has the documented historical-path INCOMPLETE exception. Measured
object-code coverage is **110,720 / 141,488 bytes**, 208 below the authoritative
110,928-byte figure because `CreateSoundBuffer` cannot prove its path literal
here. No newly STRICT function is claimed. All **718 emitted function
sections**, including unmatched/header bodies, have identical raw payloads
before and after the proposed data change.

## Reproduce

[EVIDENCE] From the repository root, with the repo-local interpreter:

```powershell
.pyenv\python.exe tools\relink.py
.pyenv\python.exe tools\relink.py --link-only
.pyenv\python.exe tools\datacmp.py --comparison build/relink/results.json --csv-out build/relink/data.csv
```

[EVIDENCE] `relink.py` writes objects, isolated compile logs, linker transcript,
response file and input/tool/library hashes under `build/relink/`. It uses
manifest flags `/W3 /GX /O2 /ML /Gd /Gy`, manifest defines and DirectX headers;
`sound.cpp` goes through the existing `msvc.py --build-path` mechanism only.
`--link-only` reuses those objects. `--objects-dir` accepts a separate verified
object run. `--linker 7255|1592|1691|1197` selects other documented 5.10
candidates while keeping SP3 LIBC; `--opt ref,noicf` and `--order manifest`
support controlled future comparisons. No alternate linker is claimed to
solve a source-definition failure.

[EVIDENCE] `relink.py` calls the existing verifier's comparator, CRT-evidence
and regression-check functions on its objects before attempting the link,
writing `results.json`, `gate.json` and `verify.log`. A real accepted-function
regression prevents linking. The two supplemental CRT evidence objects are
never link inputs. The source
candidate was evaluated using the required CLI:

```powershell
.pyenv\python.exe tools\verify.py --tu src/dectables.cpp=scratch/sol-relink/atlas/dectables.cpp --build-dir sol-relink-data
git apply --check scratch/sol-relink/atlas/atlas.patch
```

[EVIDENCE] The verifier exited zero with its expected sound INCOMPLETE note.
Its generated directory was subsequently moved into the assigned
`build/relink/atlas-gate/`; `results-original.json` retains the original report,
and a fresh comparison at the archived object paths again has no regressions.
The original verifier transcript is `scratch/sol-relink/atlas-gate.log`.

```powershell
.pyenv\python.exe tools\datacmp.py --objects-dir build/relink/atlas-gate/objects --comparison build/relink/atlas-gate/results.json --source-dir scratch/sol-relink/atlas --json-out build/relink/data-atlas.json --csv-out build/relink/data-atlas.csv
.pyenv\python.exe tools\relink.py --objects-dir build/relink/atlas-gate/objects --out-dir build/relink/atlas-link
```

## Exact link command and order

[EVIDENCE] The command is:

```text
C:\tools\msvc5_sp3\VC\BIN\LINK.EXE @D:\Prog\blupi_recon\build\relink\link.rsp
```

[EVIDENCE] `link.rsp` contains the following arguments, one per line. Object
filenames retain manifest indices, but their link order follows the observed
TU blocks in `oracle/tu_map.csv`, not the manifest's historical insertion
order. The two reconstruction-only data TUs have no oracle code block and
remain at the tail in manifest order. Their original data ownership/order is
OPEN; this ordering is not presented as a recovered original project file.

```text
/nologo
/machine:i386
/subsystem:windows,4.0
/entry:WinMainCRTStartup
/base:0x400000
/align:4096
/fixed
/incremental:no
/nodefaultlib
/opt:ref
/opt:icf
/out:D:\Prog\blupi_recon\build\relink\BLUPI.EXE
/map:D:\Prog\blupi_recon\build\relink\BLUPI.map
D:\Prog\blupi_recon\build\relink\objects\10_blupi.obj
D:\Prog\blupi_recon\build\relink\objects\00_button.obj
D:\Prog\blupi_recon\build\relink\objects\01_ddutil.obj
D:\Prog\blupi_recon\build\relink\objects\14_decblock.obj
D:\Prog\blupi_recon\build\relink\objects\15_decdesign.obj
D:\Prog\blupi_recon\build\relink\objects\17_decblupi.obj
D:\Prog\blupi_recon\build\relink\objects\11_decnet.obj
D:\Prog\blupi_recon\build\relink\objects\16_decmove.obj
D:\Prog\blupi_recon\build\relink\objects\12_decor.obj
D:\Prog\blupi_recon\build\relink\objects\13_event.obj
D:\Prog\blupi_recon\build\relink\objects\02_jauge.obj
D:\Prog\blupi_recon\build\relink\objects\03_menu.obj
D:\Prog\blupi_recon\build\relink\objects\04_misc.obj
D:\Prog\blupi_recon\build\relink\objects\05_movie.obj
D:\Prog\blupi_recon\build\relink\objects\06_network.obj
D:\Prog\blupi_recon\build\relink\objects\07_pixmap.obj
D:\Prog\blupi_recon\build\relink\objects\08_sound.obj
D:\Prog\blupi_recon\build\relink\objects\09_text.obj
D:\Prog\blupi_recon\build\relink\objects\18_dectables.obj
D:\Prog\blupi_recon\build\relink\objects\19_obstacle.obj
C:\tools\msvc5_sp3\VC\LIB\LIBC.LIB
C:\tools\msvc5_sp3\VC\LIB\OLDNAMES.LIB
C:\tools\msvc5_sp3\VC\LIB\kernel32.lib
C:\tools\msvc5_sp3\VC\LIB\user32.lib
C:\tools\msvc5_sp3\VC\LIB\gdi32.lib
C:\tools\msvc5_sp3\VC\LIB\winmm.lib
C:\tools\dx6sdk\lib\dsound.lib
C:\tools\dx6sdk\lib\ddraw.lib
C:\tools\dx6sdk\lib\dplayx.lib
C:\tools\dx6sdk\lib\dxguid.lib
```

[EVIDENCE] SP3 recognizes `/OPT:REF` and `/OPT:ICF`. An initial
`/FILEALIGN:512` argument was rejected as unrecognized and removed; 512 is the
documented controlled-probe default, not an option silently presumed applied.
`/ALIGN:4096` is recognized but emits LNK4108. The startup **symbol** is used;
forcing numerical entry RVA `0x24d40` would conceal layout errors. The final
RVA must emerge naturally after source sizes, extraction and folding match.

[HYPOTHESIS] Import-library order follows the oracle descriptor order:
KERNEL32, USER32, GDI32, WINMM, DSOUND, DDRAW, DPLAYX. The earlier controlled
5.10 test distinguishes descriptor order from the DLL-sorted IAT. Full-game
descriptor/IAT equality cannot be checked until a successful link exists.

## Link blockers and section comparison

[EVIDENCE] The 16 duplicate names are the 14 ordinary POINT operators in
`def.h` (emitted in 17 TUs), plus `GetBlupiChannelActual` and
`GetBlupiChannelStandard` defined in both decblock and decblupi. `/Gy` places
the ordinary operators in COMDAT sections with selection 1 (NODUPLICATES);
`/OPT:REF,ICF` does not make these duplicate definitions legal. No
`/FORCE:MULTIPLE`, unresolved stubs, aliases, object editing or forced RVAs
were used. Header/linkage repair must pass the shared-header regression gate.

[EVIDENCE] The seven unresolved names after the data patch are
`CPixmap::BackgroundCache(int,const char*,POINT,POINT,BOOL)`, the eight-argument
`CPixmap::CacheAll`, `CPixmap::GetTrueColorBack`,
`CPixmap::GetTrueColorDecor`, `CDecor::DecorNextAction`, the **two-argument**
`CDecor::PlaySoundA(int,POINT)`, and `table_icon_explo`. The three-argument
PlaySound body exists; the extra two-argument declaration has no body.
`OLDNAMES.LIB` resolves the initial `_ftime` and `_mkdir` failures without a
source change. Removing unused VFW/OLE/ADVAPI/COMDLG library inputs adds no
new unresolved symbols. Logs and exact decorated names are in each
`build.json` and `link.log`.

| Section | Oracle RVA / virtual bytes | Measured state |
|---|---|---|
| `.text` | `0x1000` / `0x2af93` | [EVIDENCE] object-code gate passes; linked layout unavailable. Game/CRT boundary `0x238b0` and startup `0x24d40` remain targets. |
| `.rdata` | `0x2c000` / `0x1372` | [EVIDENCE] 16 game-object bytes exact: APP_GUID at `0x2c2c0`. Imports/IAT, linked CRT constants and final offsets OPEN. |
| `.data` | `0x2e000` / `0x51e1c` | [EVIDENCE] `0x50800` file-backed initialized bytes; object payload coverage below. Linked symbol order/offsets and the `0x161c` virtual tail OPEN. |
| `.rsrc` | `0x80000` / `0x5b88` | [OPEN] deliberately omitted; no original resource bytes copied. |

[EVIDENCE] No linked section shift is inferred from a failing map. In
particular, missing DecorNextAction at `0x5670` is already documented in
LAYOUT_ORDER.md, including its peer regression; that investigation was not
repeated here. Several source bodies/order/linkage decisions remain nonfinal.

[HYPOTHESIS] With the retained sections known, each changed function's aligned
size changes subsequent TU addresses; dead stripping and ICF alter the set of
retained sections as well. The accumulated `.text` extent moves `.rdata` only
when it crosses a 4096-byte boundary, and `.data` follows the aligned `.rdata`
end. Changed data order moves string/table/global VAs and therefore relocated
code fields even in independently STRICT bodies. Summing unstripped COFF
sections cannot establish those final shifts. On a successful link,
`layout.json` records section deltas, exact same-section-offset equality,
import/IAT order and each accepted function's mapped RVA delta and raw linked
equality. Shifted equality is never labeled a whole-image strict match.

## Initialized-data comparison and coverage

[EVIDENCE] `datacmp.py` reports each initialized COFF symbol's location, span,
location evidence, status and byte equality in JSON/optional CSV. It uses
independently placed fixed-byte-equal code references, existing manifest
address evidence, or a unique full-payload hit in the corresponding PE section
at natural alignment. Conflicting/ambiguous observations remain unresolved.
The current code bytes are rechecked before reusing a comparison location.

[EVIDENCE] Ordinary numeric source array sizes are used only after their full
packed initializer matches the COFF prefix; this removes inter-symbol padding
from those arrays. Other extents are explicitly labeled COFF spans, with the
existing comparator's count-prefix/string-terminator rules. Some spans can
include compiler padding. Every counted interval matches in full. Coverage
is the **union** of oracle intervals, excluding ambiguous duplicate locations,
uninitialized symbols, virtual zero-fill and raw file padding beyond section
virtual size. The denominator includes CRT/import data even though this
object run supplies game definitions only; it is not a game-only data total.

[EVIDENCE] Relocations are encoded using independently established targets
before full byte comparison. Unknown/unsupported pointers are
RELOC-UNVERIFIED and contribute nothing. The tool never reads a pointer field
and then uses that same value to certify itself. `--image EXE --map MAP`
additionally reports linked symbol locations/deltas and actual linked payload
equality; this requires the corresponding game objects for symbol sizes.

| Input | `.data` exact / 329,728 | `.rdata` exact / 4,978 | Unique initialized total / 334,706 |
|---|---:|---:|---:|
| Canonical objects | 258,116 (78.28%) | 16 (0.32%) | **258,132 (77.12%)** |
| Three-atlas candidate | 266,810 (80.92%) | 16 (0.32%) | **266,826 (79.72%)** |

[EVIDENCE] Final reports: `build/relink/data.json` (452 EXACT symbols) and
`data-atlas.json` (455 EXACT). The large exact phase-table span at `0x345d0`
is 226,688 bytes. Other anchors include `table_blupi` at `0x2f198` (9,280),
`table_decor_quart` at `0x31aa0` (7,584), and `table_char` at `0x7b530` (1,536).
The sole located MISMATCH is the expected sound source-path literal. Fifteen
ambiguous and four unlocated definitions add no coverage. In particular the
unreferenced global `table_width[128]`, unlike GetCharWidth's separate local
byte tables, has no proven target location; it is not silently treated as the
target font-width data. `IID_IDirectPlay2A` is reported as absent from the game
objects; DXGUID.LIB supplies it for linking, so this is not an unresolved link
symbol or a request to duplicate its definition in source.

## Proposed data patch and extent limit

[EVIDENCE] Delivery: `scratch/sol-relink/atlas/atlas.patch`, applicable with
plain `git apply` (check already passed), changes only `src/pixtables.h` and
`src/dectables.cpp`. Source/manifest/shared headers have not been edited.
The existing SB2-derived header's three ordinary short-array initializers are
reconstructed as typed numeric rows, using its established schema: count,
then x/y, offset x/y, width/height for each icon. No original machine-code or
resource bytes enter source. Existing ancestry/license provenance applies.

| Array | Proven RVA | SB2 count -> SB1 count | Payload bytes |
|---|---:|---:|---:|
| `table_icon_blupi` | `0x78dd8` | 336 -> 291 | 3,494 |
| `table_icon_object` | `0x79b80` | 441 -> 246 | 2,954 |
| `table_icon_element` | `0x7a710` | 289 -> 187 | 2,246 |

[EVIDENCE] Each array ends before the next independently anchored atlas.
All 8,694 bytes match without masking; addresses come from existing strict
pixmap-reference evidence in recon.json/CORE_CLUSTER.md. Hashes, element
counts, schema and recovery procedure are in `atlas/evidence.json` and
`scratch/sol-relink/recover_atlas.py`. `dectables.cpp` includes the corrected
data header; it has no functions. No declaration ABI/member/offset changes
are proposed. The pixtables header delta consists of those initializer/count
corrections and replacement of the unsupported explo initializer by its
existing extern declaration.

[OPEN] `table_icon_explo` is 54 in the oracle count field, versus the inherited
100. Treating it as 54 complete six-short rows would consume 650 bytes through
`0x7b262`. Independently STRICT MouseHotSpot references locate its 32-byte
int table at **`0x7b260`**, where the word is 95. The naive extraction would
borrow that neighbour's low halfword as the last icon height. This was detected
before delivery; no such definition or overlap contributes candidate coverage.
The patch removes the wrong SB2 initializer and leaves the symbol undefined.
Whether SB1 had a short initializer, count bug, or another source declaration
requires further evidence; none is chosen merely to close the link.

## Compiler findings and validation

- [EVIDENCE; VC5-SEM-01; confirmed-transferable] Trigger: inherited atlas
  counts/coordinates differ and all four definitions are missing from objects.
  Replacing three SB2 initializers with independently located SB1 numeric rows
  and including their data header in the data-only TU yields 8,694 exact bytes,
  zero accepted-function regressions and all 718 raw function sections
  unchanged, under manifest SP3 flags/headers with no PCH. This is source-data
  reconstruction, not a register-allocation workaround or a new compiler rule.
- [EVIDENCE; BLUPI-LNK-01; negative] Trigger: /Gy ordinary external POINT
  definitions conflict across TUs. `/OPT:REF /OPT:ICF` still reports LNK2005
  for selection-1 COMDATs; demonstrated empty-body folding does not license
  duplicate ordinary definitions. No source inline/static experiment was
  performed and no header change is proposed from this observation alone.
- [EVIDENCE; linker option/library finding; Blupi-specific] SP3 ignores
  `/FILEALIGN:512`; it was removed. Explicit SP3 OLDNAMES.LIB resolves
  `_ftime`/`_mkdir`, reducing 12 initial unresolved names to ten before the
  data patch. These results do not identify the exact historical link command.

[EVIDENCE] `scratch/sol-relink/check_tools.py` passes focused checks for
overlap-union counting, BSS exclusion, unknown/known/wrong pointer handling,
correct/wrong linked map addresses, VC5 map parsing, a static oracle
self-comparison of section reporting, and every raw game function section.
The fixture checks do not constitute a successful game relink. `git apply --check`
passes. No game executable, linked image or ISO program was executed; no
commit or background process was created. Compiler catalog edits were not
made by this worker.

[OPEN] Next: integrate the three-array patch through the supervisor's normal
gate; resolve the duplicate definitions and six missing method bodies/ABIs
without losing accepted matches; establish explo's physical extent. Then rerun
the natural full link and measure CRT extraction/folding, DLL/IAT order and
section shifts. Resource reconstruction and exact 5.10 linker patch remain
separate open items.

## Diagnostic link of the evidence candidates (2026-10-18)

[EVIDENCE] `tools/relink.py --diagnostic` links even when accepted functions regress, writes gate FAIL to gate.json
and prints a banner; the image is for layout measurement only (never a match claim, never executed). Objects:
`verify.py --build-dir linkdiag` with scratch/linkfix candidates (decor tables + table_decor_action +
DecorNextAction in decblock.cpp, the three SB2-only decmove callers removed, table_adapt_decor moved, pixtables-only
dectables.cpp, empty obstacle.cpp; docs/LINK_CONTEXT.md). Gate: GetBlupiInfo and MockeryDetect regress (context).

[EVIDENCE] **First successful natural link** (`build/relink/diag/`): zero unresolved and zero duplicate symbols.
- `.text` virtual size is only 48 bytes below the oracle; `.rdata` is 5,884 bytes larger, which moves `.data` up one
  page (+0x1000); `.data` is 48,964 bytes smaller; `.rsrc` is absent (not reconstructed).
- Accepted functions: every function up to DrawInfo (0x5270) links at its oracle address; the first drift is
  Build's 10-byte shortfall (-16 after alignment). 212 / 402 accepted functions are byte-exact at their linked address;
  the others differ only through shifted addresses.
- `??1CButton` (accepted under button.cpp) links 432 bytes away: the oracle has it as the first function of the
  menu.cpp block, consistent with an inline (COMDAT) destructor first supplied by menu.obj. [HYPOTHESIS]
- network.cpp: the oracle places EnumProvidersCallback/EnumSessionsCallback before EnumProviders/EnumSessions; our
  objects emit them after, whether `static` or not and whether defined before or after (prototype). [OPEN]
- `.rdata` growth and the `.data` deficit are the next layout items to explain (imports, CRT constants, uninitialized
  data). The ordering items above are free of any accepted-function byte change.

### DirectX SDK version and GUID definitions ([EVIDENCE])

The diagnostic link's extra `.rdata` (+5,884 bytes) is dxguid.lib's single object (6,064 bytes, every DirectX GUID).
The oracle instead holds ten DirectPlay GUIDs at 0x2c220..0x2c2bf, in dplay.h declaration order, immediately before
network.cpp's `APP_GUID` (0x2c2c0): IID_IDirectPlay2, 2A, 3, 3A, CLSID_DirectPlay, DPSPGUID_IPX/TCPIP/SERIAL/MODEM,
IID_IDirectPlay. That is the DX6 dplay.h list **without IID_IDirectPlay4/4A** (VC5's own DPLAY.H is DirectX 3 and has
only IID_IDirectPlay). So network.cpp defined `INITGUID` before including a **DirectX 5 SDK** dplay.h, and dxguid.lib
was not linked. The manifest therefore uses `C:\tools\dx5sdk\INCLUDE` since the DX5 integration (zero regressions,
docs/LINK_CONTEXT.md); link with the DX5 libraries and without dxguid.lib.
[EVIDENCE] src/network.cpp now starts with `#define INITGUID` (2026-10-18): it emits the ten dplay.h GUIDs and
APP_GUID in the oracle's order and bytes (IID_IDirectPlay2 first, APP_GUID at +0xa0), all 19 accepted network.cpp
functions stay STRICT, and tools/relink.py links C:	ools\dx5sdk\LIB without dxguid.lib. Remaining unresolved link
methods: BlupiRect and BlupiDead. [HYPOTHESIS] The define's position (top of file vs `#include <initguid.h>`) is not
distinguishable; both give identical objects.

[EVIDENCE] 2026-10-18: MoveObjectFollow (dead SB2 helper, the BlupiRect caller) is removed with the CDecor tail
representation (docs/LAYOUT_DEBT.md). Remaining unresolved link method: BlupiDead (called from decmove.cpp).

[EVIDENCE] 2026-10-18: the four remaining caller-less SB2 decmove bodies (DynamiteStart, StartSploutchGlu, AscenseurShift,
the 3-argument ObjectStart overload) are removed; the oracle's decmove block is fully strict and has no slot for them.
Their decor.h declarations stay. Removing the declarations (and BlupiRect/BlupiDead's) shifts symbol IDs and regresses
GetBlupiInfo, IsDeepWater and MoveObjectSort (measured). **The canonical objects now link with no unresolved externals**
(LINK 5.10, DX5 libraries, no dxguid): `.rdata` virtual size equals the oracle's, `.text` is 32 bytes short (the open
functions' size differences), `.data` is 48,964 bytes short, `.rsrc` is not built yet.
