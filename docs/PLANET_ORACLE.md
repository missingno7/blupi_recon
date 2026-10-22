# Planet Blupi as an independent compiler oracle (2026-10-19)

Cleanup note: extracted oracle executables, scripts and compact reports are retained
in the local research archive; bulk distributions and compiler intermediates are
quarantined. See [RECOVERY_MILESTONE.md](RECOVERY_MILESTONE.md) for restoration.

**[EVIDENCE] Outcome C for the two remaining SB1 functions.** This investigation
does distinguish a real RTM/SP3 back-end difference in Planet 1.8, reproduces a
natural rotor-neutral member toggle, and authenticates a substantial pooled-
constant control. It does **not** identify a missing compiler that fixes SB1 or
a source relationship that closes its last 40 non-relocated bytes. Canonical
`src/`, `recon.json`, and the 127,456-byte accepted state are unchanged.

## Inputs and provenance

[EVIDENCE] Downloads came from the [official Epsitec archive](https://www.blupi.org/):
[multilingual disc](https://www.blupi.org/download/img/PlanetBlupi-en.en_US.fr.de.iso.7z)
and [Italian disc](https://www.blupi.org/download/img/PlanetBlupi-it.iso.7z).
The archive calls these 1.7/1.8 (1997), but the Italian executable's PE timestamp
is **January 1998**. README versions, not the stale `0,4,0,0` version resources,
identify the releases. EN and US are identical files, not independent witnesses.
Only archive extraction, PE parsing and disassembly were performed; no original
program or installer was executed.

| Executable | Bytes | PE timestamp (UTC) | Linker field | SHA-256 |
|---|---:|---|---|---|
| 1.7 EN / US | 685056 | 1997-07-29 11:58:02 | 3.0 | `9db17a002f312d7f34035e0c3d538a455852353116dc2efe969cb8ab6400736c` |
| 1.7 FR | 686080 | 1997-07-29 11:56:42 | 3.0 | `4ca2e3d68df20705bc2ae812f16ed29a5f3985b4718af9848e66648d303a06cf` |
| 1.7 DE | 686592 | 1997-07-29 11:58:49 | 3.0 | `bbe71cc9413a08cfb0e91fe8a6de42535d01d7a608918387faae39f687e2d2c2` |
| 1.8 IT | 688128 | 1998-01-28 21:54:39 | 5.0 | `667a6285099191ea25f687b3d774e66f8ac03edfdd46fe59a4533eb84b7a4fbc` |

[EVIDENCE] Archive SHA-256: multilingual 7z
`cc014d9ea53304631238df27cfb038a775f402fe74c7674a6310878c8e72d9b0`,
ISO `e028f83ba176fb47984b773266e3e45092bfee2e34c0f8215cf68121f77f8983`;
Italian 7z `175a254fb3d0e35eaeb1471be515b8c40f44846942a509a08a1fe324b084d254`,
ISO `42aed5ddc4d65f76a1ae9075c4560518217019340817423662be9199b1ef6e62`.

[EVIDENCE] Neither family has a Rich header or PE debug directory. Both import
KERNEL32, USER32, GDI32, WINMM, DSOUND and DDRAW, with no CRT DLL import.
EN has 111 imports; IT has 114. IT adds VirtualAlloc/VirtualFree/HeapDestroy/
GetCurrentProcess/TerminateProcess and removes HeapSize/HeapReAlloc.
Section virtual RVA/size pairs:

| Section | 1.7 EN | 1.8 IT |
|---|---|---|
| .text | `1000 / 2a00c` | `1000 / 299eb` |
| .rdata | `2c000 / 1f8` | `2b000 / 540` |
| .data | `2d000 / 83514` | `2c000 / 85818` |
| .idata | `b1000 / b92` | `b2000 / be4` |
| .rsrc | `b2000 / 4538` | `b3000 / 4a1c` |
| .reloc | `b7000 / 175a` | absent |

[EVIDENCE] Source is the [2017 initial import](https://github.com/blupi-games/planetblupi/commit/b0270ab432ac5c65337a6f16dd525a11d371f4b8),
`b0270ab432ac5c65337a6f16dd525a11d371f4b8`. Its 47 root C++/header files
are identical to tag `v1.8.0`, commit `203ccc8b5908d4075e97be4f958fdfc1c42f862c`.
The tag explicitly says "same sources". It is not pristine 1997 build input:
`misc.cpp` contains `nullptr`, and the event comparisons below prove version
differences. No original project settings were recovered.

[EVIDENCE] Private preparation lowercases quoted includes in 14 files to avoid
VC5's case-sensitive `#pragma once` alias problem. No bodies or layouts are
changed in the baseline. Original/adapted hashes are in generated `source.json`.
Original `event.cpp`: `ba1ca8a90a622cc1185ec5236abd3eb8cce7668cbbd39f2bb63c59bca571a4b0`;
`pixmap.cpp`: `f3257ec83f7f18429d2f13ea6649a9b1c7cbc5d6f27567383c4e93c48e6af66a`.

## Experiment and compiler fingerprint

[EVIDENCE] Built full event, pixmap, button and decor TUs, not extracted functions,
with `/W3 /GX /O2 /ML /Gd /Gy`, WIN32/NDEBUG/_WINDOWS/`nullptr=0`, no PCH,
normal default inlining, one stable source path, DX5 headers first and explicit
SP3 VC headers for **every** compiler. The initial native-header matrix was
repeated with these fixed headers and gave the same changed-function set.
All 27 baseline source TUs were also compiled with SP3 to supply independent
call/data evidence. This is not a repeat of the old misc/fifo family test.

| C1XX/C2 pair tested | Changed payloads vs SP3, out of 204 functions | Distinguishing 1.8 controls |
|---|---:|---|
| RTM | 5 | pointer-overload BltFast and Display match |
| SP2 (7149/7153) | 1 | neither matches |
| SP3 (7303/7303) | baseline | neither matches |
| NT5 1592 (7156) | 1 | neither matches |
| NT5 1691 (7156) | 1 | neither matches |

[EVIDENCE, 2026-10-20] The subsequently obtained full English SP1 C1XX/C2 files
are byte-identical to the SP2 pair above; this is the same compiler control, not
an additional build or compile. Provenance: [CONTEXT_REVISIT.md](CONTEXT_REVISIT.md).

[EVIDENCE] Differences are event TreatEventBase plus pixmap Create, pointer
BltFast, SearchColor and Display for RTM; only SearchColor for SP2/NT5.
"Changed" compares code plus symbolic relocation lists; it does not mean a
binary match. SP3's four TUs have 179/204 relocation-masked hits in IT, RTM
181/204. The report deliberately does not count all these as STRICT.

[EVIDENCE] **Two useful compiler fingerprints**, with unchanged imported bodies:

| Function | IT RVA / extent | RTM | SP3 |
|---|---|---|---|
| `CPixmap::BltFast(LPDIRECTDRAWSURFACE,...)` | `21fb0 / 96` | STRICT, 1 relocation | code mismatch |
| `CPixmap::Display` | `22d70 / 160` | STRICT, 3 relocations | code mismatch |

[EVIDENCE] Crossing components resolves the cause: SP3 C1XX + RTM C2 reproduces
both controls; RTM C1XX + SP3 C2 does not. Thus this result follows the **back
end**, under the fixed input context. RTM BltFast forms the Boolean in EAX,
copies it to EDI and retains a vtable entry in EBP; SP3 forms it in EBX and
reloads the indirect call slot. These are different live ranges, not a linker
address difference. The C2 SHA-256 values are RTM
`6d1406af5f62647347cfa7175cd7e108b838d6841a546396f29252198a14d73d`
and SP3 `e75aecaf4073b0817ffb638cae5fff636b2e2d1a090daabf8f68dbc954515fae`.
Full hashes of all tested front/back ends are in [TOOLCHAIN.md](TOOLCHAIN.md).

[EVIDENCE] A separate static LIBC control supports RTM for IT: `__setmbcp`
matches RTM's 496-byte body at `28ce0`, not SP2/SP3; `__output` at `25fe0`
matches RTM/SP2, not SP3. These are **relocation-masked CRT fingerprints**.
The 416-byte WinMainCRTStartup body at `254c0` matches all three single-thread
LIBC candidates. CRT identity is not automatically application C2 identity,
but here the independent observations agree. Five CRT members were examined;
this is not an exhaustive CRT census.

[EVIDENCE] 1.7 cannot serve as the same source-compatible VC5 witness yet.
Its code uses INT3 inter-function padding where the VC5 probes use NOPs.
Removing that padding *for diagnosis only* raises the four-TU hit count from
0 to 45/204; only simple controls such as the four-byte GetPhase body agree.
FlipOutline/BltFast/Display still do not. No tested full CRT member matches
1.7. [HYPOTHESIS] An older toolchain/source revision is likely; linker 3.0
alone does not identify that compiler. A VC4 comparison was not performed.

## Positive controls and source-version separation

[EVIDENCE] The following unmodified imported bodies pass independent relocation
verification against IT. The Planet comparison disables the SB1 function and
TU maps; SB1 addresses cannot certify Planet calls.

| Control | IT RVA | Bytes / relocations | What it demonstrates |
|---|---|---|---|
| `CDecor::FlipOutline` | `14f30` | 48 / 0 | member SETcc store with ordinary rotor assignment |
| `CDecor::SetInfoMode`, `GetInfoMode` | `198f0`, `19910` | 32 / 0, 16 / 0 | genuine getter/setter field correspondence |
| `CPixmap::BltFast(int,int,...)` | `21ec0` | 240 / 1 | ECX preference despite next rotor EAX |
| `CDecor::BlupiCreate` | `9830` | 688 / 6 | competing pooled constants with real calls/live ranges |

[EVIDENCE] Read-only SP3 observations use the existing booltemps/allocator
addresses, with normal-output audits. In BlupiCreate, pooled zero has 29
references and priority +119, takes ECX; pooled -1 has 9 references and +108,
takes EDX. Both choices occur in a complete STRICT body. This is an independent
Planet gameplay control, not an SB1 utility transplanted under a new name.

[EVIDENCE] **Natural rotor-neutral member toggle:** EventButtons contains
`m_bInfoHelp = !m_bInfoHelp; if (m_bInfoHelp) ...`. At IT RVA `1b993`, the exact
21 bytes through the member store load the old flag into EDX and form/store
the result in EAX. The result receives a copy-into-EAX preference at C2 line
146; its assignment takes return path `435db5`. Rotor EDX is observed both
before assignment and at SET, so it was not consumed. The newly added
`rotor_at_set` and `assignment_return` fields make this independently checkable.
All 70 event, 45 pixmap and 69 decor function payloads and symbolic relocations
survive the observations unchanged.

[EVIDENCE] EventButtons has a full 1072-byte relocation-masked hit, but its phase
table data and some callees do not verify: **the whole function is not STRICT**.
The cited 21-byte toggle region has no relocations. Its immediate Boolean
branch/return use differs from SB1's later `bEnable` feedback. Planet's four
cheat toggles instead have EDI preferences, which cannot satisfy the byte-register
constraint; they take rotor/skip paths. A named Boolean is not sufficient to
obtain a usable preference. None supplies SB1's required ECX result, EDX rotor
and ESI feedback relationship without changing other instructions.

[EVIDENCE] The imported ChangePhase/TEB bodies differ from IT in concrete
version behavior. The small [version-control patch](../tools/fixtures/planet_event_v18.patch)
removes the intro timer reset, the two intro key handlers and two intro switch
labels, and restores the movie guard on Return (also present on Escape).
Prediction: these changes remove the extra actions/dispatch entries, preserving
the cheat toggles. Falsifier: residual non-relocated code/table differences.
Afterward **both RTM and SP3 have complete relocation-masked hits**:
ChangePhase `1bc30 / 2720`, TreatEventBase `1e9d0 / 4240`.
Before removing the case labels, instructions already aligned 1239/1239 but
96 non-relocated switch-data bytes differed; that was not accepted as a match.

[EVIDENCE] These are binary-evidenced reconstructions, not pristine-source
proofs. ChangePhase still fails phase-table data validation; TreatEventBase
still has unresolved external relocations. Neither is called STRICT here.
The phase experiment nevertheless isolates a real source contribution to cost:
removing the intro reset changes initial zero priority -213 -> -203 (87 -> 86
references) and PLAY -374 -> -370 (six references), while the selected zero
component stays +12 in EBX (20 -> 19 references). This is a positive cost-change
control and a negative allocation-flip control; it does not reproduce SB1's
zero/PLAY competition or its required cut. A separate 688-byte STRICT control
above establishes that the stock allocator can reproduce pooled constants.

## What transfers to Speedy Blupi

[EVIDENCE] Recompiled SB1's already accepted Display body with RTM and SP3.
SP3 has zero non-relocated differences at `22240`; RTM has 16 bytes different
in zero materialization/RECT-store scheduling (65/68 aligned instructions).
Thus the Planet RTM fingerprint does **not** justify switching SB1 to RTM.
eGames has the SP3 body at `221a0` and no RTM hit; that reused body is weak
additional evidence. The June demo and SB2 have neither complete body hit
with these SB1 objects; different source/layout context prevents attribution.
Existing [ORACLE.md](ORACLE.md) evidence still applies: the June demo lacks
SB1's problematic rank-0 cheat, while retail/eGames reuse the toggle code;
eGames ChangePhase is a distinct body with small known source differences.

[HYPOTHESIS] **A, missing compiler:** possible, but not strengthened for SB1 by
this study. Known C2 releases demonstrably differ on a fixed authentic input,
yet SB1 favors SP3 on the new discriminator. PB's two event controls agree on
both RTM and SP3 once version differences are addressed. Neither observation
demonstrates an unavailable compiler's required behavior.

[HYPOTHESIS] **B, missing source relationships:** remains viable. The member
toggle supplies an authentic retained EAX preference without advancing EDX,
and the phase source delta changes measured costs. However neither has the
particular consumer/lifetime topology needed in SB1. This is evidence for the
mechanisms' availability, not a historical SB1 source recovery.

[EVIDENCE] Bounded compiler hunt: rechecked installed component hashes and the
sibling Blood II `docs/toolchain.md`, `docs/online_projects_survey.md` and
cross-project linker corrections. Blood II's **7287 is a linker export-object
stamp**, not a C1XX/C2 version. Historical CRT already explains the formerly
suspected import-order discrepancy. The referenced VB5 C2 7182 tree is absent
locally. Two bounded web-search batches found no new component hash or usable
compiler distribution. No new toolchain was downloaded or tested, and no
claim of an exhaustive archive search is made.

[OPEN] Next useful evidence: an authentic SB1 header/TU/preprocessor revision
that changes the Boolean's consumer/preference to ECX while preserving ESI
feedback; a source relationship that splits the late zero component while
keeping PLAY connected through WriteInfo; or an actual separately hashed C2
drop showing either decision on fixed inputs. Original PB1.7 project/compiler
files and source matching its layouts would make the earlier build a stronger
independent control. More declaration padding or unnamed "7287 compiler"
speculation is not supported by these results.

## Reproduction and retained evidence

[EVIDENCE] Focused driver: [tools/planet_oracle.py](../tools/planet_oracle.py).
All generated material is under ignored `build/planet-forensics/`; all compiler
invocations go through `tools/msvc.py`, serially, with repo-local TMP/TEMP.
Download with a browser User-Agent and the archive page as Referer (bare curl
received HTTP 403), extract the two ISOs into `distribution/`, then only game
executables/READMEs into `disc/` and `disc-it/`. Never execute them.

```powershell
.pyenv/python.exe tools/planet_oracle.py prepare
.pyenv/python.exe tools/planet_oracle.py inventory
.pyenv/python.exe tools/planet_oracle.py compile
.pyenv/python.exe tools/planet_oracle.py scan
```

Repeat `compile` with `--tree rtm`, `sp2`, `nt5-1592`, `nt5-1691`, then `matrix`.
Repeat `scan` with the same tree arguments. For independent callee/data evidence
in the STRICT controls, populate all SP3 objects before comparison:

```powershell
$planetUnits = (Get-ChildItem build/planet-forensics/source/*.cpp).BaseName
.pyenv/python.exe tools/planet_oracle.py compile --units $planetUnits
.pyenv/python.exe tools/planet_oracle.py compare
.pyenv/python.exe tools/planet_oracle.py compare --tree rtm
```

`--units` selects full TUs. The hybrid commands use `--units pixmap --label
rtm-front --extra-flag=/BxC:\tools\msvc5_rtm\VC\BIN\c1xx.dll` and analogous
`rtm-back` with `/B2...\c2.exe`. `version` applies the retained patch to a new
private `version/event.cpp`; reproduce its final event-only controls as follows:

```powershell
.pyenv/python.exe tools/planet_oracle.py version
.pyenv/python.exe tools/planet_oracle.py compile --units event --source build/planet-forensics/version --label version2-sp3
.pyenv/python.exe tools/planet_oracle.py compile --tree rtm --units event --source build/planet-forensics/version --label version2-rtm
.pyenv/python.exe tools/planet_oracle.py scan --tree version2-sp3
.pyenv/python.exe tools/planet_oracle.py compare --tree version2-sp3
```

`capture --units event pixmap decor` and `booltrace --units event pixmap decor`
produce the Boolean controls; `phase --units event` observes constants by
function name. For BlupiCreate: `capture --units decblupi`, then
`phase --units decblupi --function BlupiCreate@CDecor`. Capture requires the
existing C2 stub; build it with `.pyenv/python.exe tools/c2trace/c2trace.py build-stub`
if absent. For the version control,
use `capture --units event --source build/planet-forensics/version --label version-event`,
then `phase --units event --label version-event --native-tree version2-sp3`.
`crt` reads archive members. Reproduce the SB1 cross-version Display scan with:

```powershell
.pyenv/python.exe tools/fnprobe.py src/pixmap.cpp Display 0x22240 --tree sp3 --name planet-sb1-sp3
.pyenv/python.exe tools/fnprobe.py src/pixmap.cpp Display 0x22240 --tree rtm --name planet-sb1-rtm
.pyenv/python.exe tools/planet_oracle.py cross
```

[EVIDENCE] Key receipts: `inventory.json`, `archives.json`, `source.json`,
`matrix.json`, per-tree `scan.json`/`compare-planet.json`, `crt.json`,
`capture-*/booleans.json`, `audit.json`, and `phase.json`. There were 77 successful
source compilations including two SB1 Display controls; instrumentation replays
are separate. An initial full use-graph allocation trace was interrupted for
cost, and its discarded result was replaced by the bounded scalar trace.
No CL/C2 processes remained after cleanup. The earlier full SB1 gate/relink
remains documented in [BLOCKER_CHECKS.md](BLOCKER_CHECKS.md); source and acceptance
inputs have not changed since that PASS.
