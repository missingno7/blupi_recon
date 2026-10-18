# sol-icf round 3 — Rich census and CRT inventory

STATUS: PARTIAL. [EVIDENCE] The count rule and our complete loaded-member census
are recovered. [OPEN] The identities of the original link's two net additional
stamp-less modules are not established. No game-source or library-order fix is
proposed. `round3.patch` adds an optional, read-only `histlink.py --trace-rich`
census and catalog rule BLUPI-LNK-04 (including the alias negative); it does
not adjust a count or a linker decision.

[EVIDENCE] Baseline: HEAD `61b1515`, supplied `build/verify/objects`, SP3 LINK,
SP3 LIBC, DX5 imports before LIBC, historical host. The relink passes the gate
(116,160 bytes) and reproduces the previous historical image exactly:
SHA-256 `9f5fc9a0717d3d1bd9226443293abc3cbd233cdf62f3b6a0e1a348093e0e484d`.
The census-enabled launcher produces that same SHA-256. No output is patched.
The exact 19 baseline objects and logs are preserved under
`build/sol-icf/round3-base-inputs`, with hashes checked against the measured
link's build.json; shared `build/verify/objects` was rebuilt during this work.

## What LINK counts

[EVIDENCE] LINK VA `0x43e340` writes Rich. It walks the image's library list
at `this+0x1bc` with iterator `0x40faf0/0x40fb20` (next library `+0x3c`),
and each library's loaded-module list with `0x40fb60/0x40fb90`
(first module `library+0x38`, next module `module+0x6c`). The inner loop at
`0x43e3af` groups by DWORD `module+0x30`. At `0x43e3cb` it increments an
existing stamp's count; `0x43e3d0..0x43e3e6` creates a count of one. There is
no test here for retained sections, public symbols, code size, REF or ICF.

[EVIDENCE] `module+0x30` is initialized from the absolute static COFF
`@comp.id` symbol in `0x45f430`, specifically `0x45f4de..0x45f4f1`.
Zero means no nonzero stamp was supplied. Rich stores `(stamp,count)` pairs
at `0x43e477..0x43e484`, XORed with the header key. It stores no object name
or archive identity. A stamp of zero also contributes zero to the rotated-stamp
part of the key at `0x43e425..0x43e42f`.

[EVIDENCE] The breakpoint observes each increment, reads the module and library
from `[esp+0x1c]` and `[esp+0x2c]`, and resumes the original instruction.
`module+4` is its name, `module+8` its load index, `module+0` the archive
member's data offset (archive header offset +60). `library+0` points to its
path. `rich-inventory.json` decodes every selected archive member and symbol.

| Counted category | Stamp-less modules |
|---|---:|
| Game command-line objects, including the data-only TU | 19 |
| LIBC members | 104 |
| KERNEL32 import-library members | 71 |
| USER32 | 37 |
| GDI32 | 13 |
| WINMM | 14 |
| DSOUND | 3 |
| DDRAW | 3 |
| DPLAYX | 4 |
| OLDNAMES | 0 |
| Total | **268** |

[EVIDENCE] The converted `.res` contributes **one additional module**, stamp
`0x60684`, count 1. The RC input, CVTRES process and individual resources do
not each add another stamp-less module. The trace totals 269 modules and
reproduces both decoded Rich pairs exactly.

[EVIDENCE] Import accounting is **128 retained imported functions + 7 DLL
descriptors + 7 DLL null-thunk objects + 1 shared null descriptor + 2 discarded
function-import objects = 145**. The discarded ones are KERNEL32's
`_FindClose@4` and `_SetConsoleCtrlHandler@8`, pulled by unused portions of
LIBC `findfile.obj` and `winsig.obj`. Their members count despite REF removing
their thunks and IAT entries. Thus neither the final import count nor the map
alone recovers the Rich count.

[EVIDENCE] OLDNAMES' machine-zero weak-alias objects also count individually
when loaded, even though their `.text` is empty. The alias trial below loads
`oldnames/ftime.obj` (archive data offset `0xaa8c`) and `oldnames/mkdir.obj`
(`0xe4c0`), each stamp zero, and increases exactly 268 -> 270.

## CRT and imports compared with the oracle

[EVIDENCE] `libscan.py` scans **all 624 LIBC members**, not just map members:
585 ordinary COFF objects and 39 machine-zero alias objects. It searches all
1,131 typed function bodies, 1,080 code sections (including assembly without
function types), and 722 initialized non-code sections in the oracle and our
image. COFF relocation fields are excluded from the signature; trailing
alignment NOP/INT3 bytes are omitted. These are **diagnostic signature
locations, not strict matches**. The complete results are
`round3-libscan.json`.

[EVIDENCE] No function or initialized-section signature occurs only in the
oracle. 101 data-section signatures occur in both; 621 in neither, including
short/zero/low-information sections for which this search supplies no useful
identity. This is not a proof of absence for discarded members or a complete
symbol-level initialized-data acceptance claim.

[EVIDENCE] `crtvalidate.py` additionally constrains code/IAT relocation
targets to their independently located CRT callees or actual PE imports,
removing 4,644 oracle and 4,617 candidate-image false candidate locations.
It converges after two elimination rounds. Again there is no oracle-only
function. Shared helper signatures remain possible in unlinked members
(e.g. `mbsdup`/`strdup`, `memcpy`/`memmove`, and narrow/wide output helpers),
so a matching signature does not prove that its archive member was selected.

[EVIDENCE] All **183 LIBC public/static code map rows** have oracle locations
in `round3-report.json` / `round3-crt-map.csv`. The only initially ambiguous
retained function, `_srand`, is resolved by its shared `holdrand` relocation
with independently unique `_rand`. Folded `ArrayUnwindFilter` is reported by
LINK under the surviving frame module; its body belongs to ehvecdtr too.
128 rows have delta -0x10, 53 delta -0x30, and the two names for the shared
filter both name our `0x27380` versus oracle `0x243f0`. There is no missing
retained CRT body needed to explain the pre-entry extent.

| Anchor | Oracle RVA | Ours RVA | Delta |
|---|---:|---:|---:|
| First CRT function, `strstr` | 0x238b0 | 0x238a0 | -0x10 |
| `_ftime` | 0x23d10 | 0x23d00 | -0x10 |
| `ArrayUnwindFilter` / `FrameUnwindFilter` | 0x243f0 | 0x27380 | moved |
| `srand` | 0x24410 | 0x243e0 | -0x30 |
| `_mkdir` | 0x24b30 | 0x24b00 | -0x30 |
| `WinMainCRTStartup` | 0x24d40 | 0x24d10 | -0x30 |
| `DestructExceptionObject`, after the frame filter | 0x277d0 | 0x277c0 | -0x10 |

[EVIDENCE] **Entry minus game-code deficit is -0x20, not zero.** Game extent
is short 0x10, established at `strstr`. Keeping the 32-byte filter in frame
rather than ehvecdtr removes it before startup and restores those bytes later.
This fully explains the additional -0x20 before `wincrt0`; it is not evidence
for another missing live CRT function. Historical current-input ICF keeps
button's empty return, sound's getter, and frame's filter, as in round 2.

[EVIDENCE] Oracle/candidate have the same 128 imported names/ordinals and
hints, and byte-identical import descriptor directories. The same seven
`FF 25 <IAT>` sites are found in each image (four standalone DirectX thunks,
two embedded CRT tail jumps to GetOEMCP/GetACP, and the RtlUnwind thunk), all
at delta -0x10. There is no oracle-only retained thunk. The 326 differing
`.rdata` bytes include INT/IAT permutations and code-dependent metadata;
the audit explicitly lists the 33 differing bytes outside INT/IAT arrays.

## Evidence-backed negative trials

[EVIDENCE] Reverting `_ftime`/`struct _timeb` to `ftime`/`struct timeb`, and
`_mkdir` to `mkdir`, restores the two OLDNAMES members and Rich 270. The full
required verifier run exits 0 with **zero accepted regressions**, 115,952
available bytes and the expected sound-path INCOMPLETE exception:

```
.pyenv\python.exe tools/verify.py --tu src/blupi.cpp=scratch/sol-icf/round3-src/blupi.cpp --tu src/event.cpp=scratch/sol-icf/round3-src/event.cpp --build-dir sol-icf-r3
```

[EVIDENCE] Primary layout trials reuse the attested sound object and every
other supplied object, replacing only the two verifier-built candidate TUs
(`build/sol-icf/round3-inputs`). Their relink gate is PASS, 116,160 bytes.

| Historical-host input | Rich N | Entry | Raw `.text` differing bytes |
|---|---:|---:|---:|
| Baseline underscored calls | 268 | 0x24d10 | 66,622 |
| Both OLDNAMES aliases, usual libraries | 270 | 0x24c40 | 67,540 |
| Both aliases, OLDNAMES before LIBC | 270 | 0x24c40 | 66,785 |
| Both aliases, IDE default libraries | 270 | 0x24c40 | 67,540 |
| RTM LIBC, unchanged game objects | 268 | 0x24d10 | 145,532 |
| SP2 LIBC, unchanged game objects | 268 | 0x24d30 | 66,521 |

[EVIDENCE] Physically putting explicit libraries before objects reproduces
the corresponding alias rows, without repairing placement. With ordinary
order, `_ftime` moves to `0x2b840`, `_mkdir` to `0x2b900`; with OLDNAMES
first they land at `0x24e40` / `0x24f00`, still after startup. Both contradict
the oracle's early member positions. These trials identify the two objects
removed by reconstruction commit `bc81fef`, **not the original's two objects**.
Restoring them solely to fit the Rich count is rejected. All RTM/SP2/SP3
OLDNAMES and Win32 import libraries checked are byte-identical across service
packs; the tested LIBC archives each load the same 104 member names.

[EVIDENCE] The removed text big-number routines call `sprintf`, whose member
is already pulled by blupi. The removed trajectory/decmove helpers use
memset/memory intrinsics, existing game methods, rand and existing Win32
imports; no new CRT-member requirement is established by those bodies.
Restoring them has no evidence-based explanation for two additional modules.

[EVIDENCE] `iat_inverse.py` first validates its entire 137-entry permutation
against the live LINK trace, then inserts hypothetical missing import records
in a **mathematical model only**. All seven existing DLLs and all insertion
positions are tested: 966 one-record cases and 469,959 two-record cases.
None reproduces the oracle's 128-slot order (best 51/128 and 79/128).
Therefore the difference is not explained by simply adding one/two discarded
import members while preserving the present initial contribution order.
This does not rule out removed *and* added inputs, or a different initial order.

## Limits, delivery and remaining work

[EVIDENCE] The other worker advanced HEAD to `accd2a1`, including a new Build
body in `cf2b637`. A separate final relink of its rebuilt current objects is
under `build/sol-icf/round3-head-accd2a1`: gate PASS, 116,160 bytes; Rich is
still 268+1 and the census still 269 modules. `.text` is now **32 bytes larger**
than the oracle, with 29,721 same-offset equal initialized bytes. `strstr`
starts at `0x238d0` (+0x20), frame still owns the filter at `0x273b0`, and
entry is **0x24d40**, equal by cancellation: +0x20 game extent minus the
same 0x20 filter displacement before startup. Entry minus game drift remains
**-0x20**. This is not a corrected CRT layout. The baseline tables above
remain pinned to the task's original objects and have not been overwritten
with these newer measurements.

[OPEN] Rich supplies a **net count**, not a named set difference. Extra
data-only/dead game objects, weak aliases, discarded library code/imports,
and substitutions among such inputs can leave no independent final-image
identity. The original module list, project, map, or a source-supported
reference is needed to name the two confidently. The present evidence does
not justify adding two empty TUs, restoring aliases, or retaining dead SB2
bodies to fit the count. The original TU partition is itself not fully proven
(docs/TU_MAP.md and LINK_CONTEXT.md); current data-only ownership is partly
a reconstruction convenience.

[EVIDENCE] Remaining baseline header differences are exactly file offsets
`0x94` (Rich 268/270), `0xd8` (entry 0x24d10/0x24d40), and `0x1b0`
(`.text` VirtualSize 0x2af83/0x2af93). TimeDateStamp is 0x36245013;
file size 535,552 equals the oracle. Total unequal file bytes 66,966:
three header, 66,622 `.text`, 326 `.rdata`, 15 `.data`; `.rsrc` exact.

[EVIDENCE] Delivery: `round3.patch` (optional Rich observation and catalog),
`round3-tools/histlink.py` (candidate), the census reports under
`build/sol-icf/round3-*`, `richtrace.py`/`richreport.py`, `libscan.py`,
`crtvalidate.py`, `iat_inverse.py`, `round3_report.py`, JSON reports and the
CRT/member CSVs. `git apply --check scratch/sol-icf/round3.patch` passes.
No canonical source/docs, assets, external reference or toolchain file was
modified; no commit or game/output-image execution occurred. The successful
census reports have process wait 0, active job processes 0 and unchanged tool
hashes. One exploratory driver attempted two launch() calls in one Python
process; its second call failed before LINK entry with WinError 6. The
corresponding single-process CLI trial was rerun successfully; the optional
census delivery is verified through that normal CLI path.

## Text for docs/RELINK.md (not applied)

[EVIDENCE] Round 3 recovers LINK's Rich writer at 0x43e340. It counts every
loaded module once by its `@comp.id` stamp, including weak aliases and members
later discarded by REF/ICF. Current (0,268) = 19 game + 104 LIBC + 145 import
members; the converted resource is separately (0x60684,1). The oracle's 270
does not identify two missing live CRT members. All 183 current CRT code map
rows have oracle signature locations and no oracle-only CRT function/data
signature was found in the full SP3 LIBC inventory. The first CRT RVA differs
by -0x10, whereas entry differs by -0x30: the remaining -0x20 before startup
is the unwind filter kept later in frame.obj. Reinstating ftime/mkdir aliases
restores Rich 270 but moves their CRT bodies past startup, contrary to the
oracle; this is rejected. Original additional-module identities remain OPEN.

## Import order: forward model and inverse search ([EVIDENCE], worker sol-rich rounds 1-2)

- LINK 5.10 /OPT:REF drops IAT slots of imports referenced only by discarded code but still counts their modules in Rich
  (controlled test). Our 137 qsort inputs include two such dead KERNEL32 imports (FindClose, SetConsoleCtrlHandler) and
  seven null thunks; they participate in the sort.
- Library extraction rule recovered: game objects in command-line order, undefined externals queued in COFF symbol-table
  order on first encounter, libraries scanned over the growing queue, repeated sweeps. All 249 members replay exactly.
- The ported VC5 SP3 MSVCRT qsort reproduces both live 137-entry permutations and our 128 IAT slots exactly.
- The oracle's physical hint/name order agrees with our load order for all 126 named imports, yet ~440,000 bounded
  hidden-entry configurations (k<=4 extra entries, null/dead placement) give no inverse; all fail USER32's 35 names.
- Demo/eGames/oracle share one IAT name order (demo: GDI32 GetDeviceCaps swap, explained by pixmap Create's earlier call).
  RTM LINK 5.00 with the same CRT gives the same 15/128. [OPEN] Rich +2 and the original sort input/implementation.
