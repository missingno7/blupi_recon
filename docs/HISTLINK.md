# sol-icf round 2: historical LINK launcher (HEAD 7e69994)

[EVIDENCE] HEAD advanced during final review to `4a357bc`, a documentation-only
TreatEventBase correction. The measured object sets stayed byte-identical;
the delivered tools patch checks against the current HEAD as well.

[EVIDENCE] **Launcher implemented; two expected image conclusions do not hold
for the current input order.** All 111 LINK MSVCRT imports bind to the original
VC5 redistributable before LINK's entry instruction, the PE timestamp is
`0x36245013`, and independent historical links are byte-identical. Entry is
`0x24d10` (-0x30), but **frame, not ehvecdtr, survives**. Full historical CRT
binding also changes import-array ordering beyond ICF. Neither discrepancy
is concealed by changing inputs, selecting survivors, or patching output.

## Delivered implementation

[EVIDENCE] `tools/histlink.py` runs the original
`C:/tools/msvc5_sp3/VC/BIN/LINK.EXE`; its hash is pinned to
`04c892989f4cc8076ef45e033215eb7aefbdf1459cbee9b13a81f8ddb718b579`.
It copies `VC/REDIST/MSVCRT.DLL` unchanged to a private basename under the
report directory (hash
`e390b0529fb9ee1214bb68302a4658b578134d92ab76bc5eb48f3badd843d210`).
The system KnownDLL remains loaded; neither original file is modified.

[EVIDENCE] A WOW64 debug session stops before the executable entry at
`0x465280` (LINK has no TLS callbacks). The launcher loads the private CRT
using the process's LoadLibraryW, resolves **every one of LINK's 111 MSVCRT
imports** to that DLL's exports, rewrites and reads back the running IAT,
then restores the original entry byte, stack and full WOW64 register state.
This includes data imports, not just function imports. No LINK instruction
executes before rebinding. Other modules' IATs are not rebound.

[EVIDENCE] The launcher uses the existing repo WOW64 definitions in
`tools/c2trace/c2dbg.py`; it executes no compiler and reads/writes no output
image. A kill-on-close Job Object owns LINK and its possible CVTRES child.
Errors/timeouts terminate and drain the tool tree synchronously. Normal,
missing-input (1181), and forced-timeout paths report zero active processes.
TMP/TEMP are always `build/tmp`; original tool files are hash-checked before
and after. The CLI refuses tools other than the pinned SP3 LINK.

[EVIDENCE] `tools/relink.py --historical-host` invokes the launcher and embeds
the complete `historical-host.json` audit in build.json. It retains ordinary
regression gating and failure cleanup. Default modern linking is unchanged.
`histlink.patch` contains the new tool and the focused relink.py integration;
it is checked against a private copy of HEAD, without editing docs.

## Clock semantics and observed APIs

[EVIDENCE] Wall time is frozen at **1998-10-14 07:17:39 UTC**. Debugger API
boundaries emulate SYSTEMTIME for GetSystemTime/GetLocalTime, FILETIME for
GetSystemTimeAsFileTime/GetSystemTimePreciseAsFileTime, and UTC/no-DST
GetTimeZoneInformation. Only the tool process is affected; system time is
unchanged. `TZ=UTC0` and consistent local/UTC API results make VC5's own time
conversion independent of the current PC's time zone. No CRT time/qsort
implementation is substituted: original DLL code executes.

[EVIDENCE] In this link, LINK calls `time()` once through IAT `0x401120`,
at `0x443a2c` (return `0x443a32`). The **real historical time() returns
`0x36245013`**, verified at the return site. Its observed calls are
GetLocalTime (`0x780086fa`), GetSystemTime (`0x78008705`), and
GetTimeZoneInformation (`0x780087c6`). CRT initialization also calls
GetLocalTime (`0x78002a76`); modern CRT initialization calls
GetSystemTimeAsFileTime. The precise FILETIME API is supported but not hit.
LINK calls ctime once (return `0x44f08f`) to format the map's timestamp.

[EVIDENCE] LINK also calls GetTickCount through IAT `0x401060` for elapsed
timing, at return sites `0x444e8f`, `0x4444be`, `0x444867`, `0x4448db`,
`0x444eb6`. **This monotonic uptime counter is native**, not a Unix/date
clock; it does not determine the PE timestamp. LINK imports neither clock()
nor _ftime(). No original boot uptime is known or invented.

[HYPOTHESIS] The pinned VC5 SP3 redistributable is the appropriate CRT from
the selected compiler/linker installation. This restores its actual algorithms
and reproduces the oracle's observed UTC link instant; it does not prove the
original PC's exact OS, locale, time zone or installed CRT patch level.
Native file metadata, Win32 services, elapsed timing, and the unmodified
CVTRES child remain native. This is a scoped build-host emulation, not a
Windows 1998 virtual machine.

## Image verification (identical supplied build/verify/objects)

[EVIDENCE] Primary images: `build/sol-icf/round2-modern/` and
`build/sol-icf/round2-historical/`. Both reuse the same supplied objects and
pass the accepted-function gate at 116,160 / 141,488. Both are 535,552 bytes.
The historical EXE SHA-256 is
`9f5fc9a0717d3d1bd9226443293abc3cbd233cdf62f3b6a0e1a348093e0e484d`.
Independent standalone and integrated historical launches produce identical
**whole files**, including the timestamp; no normalized equality is claimed.
All 19 primary objects and their compile logs are archived unchanged in
`build/sol-icf/round2-inputs/`, with provenance. A link from that private
snapshot also produces exactly the same whole-file SHA-256.

| Item | Modern | Historical | Oracle |
|---|---|---|---|
| Empty ret owner / RVA | jauge / `0x1ede0` | button / `0x2a30` | misc / `0x1f010` |
| Four-byte getter owner / RVA | sound / `0x22db0` | sound / `0x22db0` | sound / `0x22dc0` |
| Unwind-filter owner / RVA | frame / `0x27380` | frame / `0x27380` | ehvecdtr / `0x243f0` |
| Entry RVA | `0x24d10` | `0x24d10` | `0x24d40` |
| `.text` same-offset equal bytes | 23,062 | 109,395 | 176,019 |
| `.rdata` same-offset equal bytes | 4,546 | 4,652 | 4,978 |
| `.data` same-offset equal bytes | 329,713 | 329,713 | 329,728 |

[EVIDENCE] ICF still has 793 candidates and unchanged size/relocation keys.
Moving import libraries before LIBC changes their **initial order**, starting
at index 463, relative to round 1. Both modern and historical complete ICF
permutations match the prior models. Thus the historical survivor rule is
working; the expected ehvecdtr survivor is not a consequence of these inputs.
No object/library order or source was changed to force the expected owner.

[EVIDENCE] `round2_report.py` performs read-only full-image comparisons:

- Clock-only control (`--system-crt`) changes **exactly offsets
  0xb8..0xbb**, the four-byte PE TimeDateStamp. No other byte changes.
- Historical versus modern changes 121,138 `.text`, 396 `.rdata`, 11 `.data`,
  zero `.rsrc` bytes, plus the timestamp. Section sizes/layout remain equal.
- Compared with the existing **ICF-only** diagnostic on the same inputs,
  full historical binding has **660 differing bytes**: four timestamp bytes,
  **260 `.text` bytes in 226 known IAT references**, and **396 bytes entirely
  inside the INT/IAT arrays**. Import names, hints and symbol sets are equal.
  Every additional byte is accounted for; output files are never rewritten.
- LINK also calls qsort twice at return `0x40f88c`: 137 import contributions,
  comparator `0x40f8d0` comparing only DLL object names. The historical sort
  changes 124 tied positions in each group, independently of ICF. These are
  the `.idata$4/$5` order changes. The fourth qsort (return `0x462428`) sorts
  map/public-symbol records. This disproves the requested “ICF and timestamp
  only” output assumption for **all-import** historical binding.

[EVIDENCE] Historical header versus oracle differs at only three byte offsets:

| File offset | Field | Historical | Oracle |
|---|---|---|---|
| `0x94` | Rich stamp-less object count | 268 (`0x84` encoded byte) | 270 (`0x86`) |
| `0xd8` | AddressOfEntryPoint | `0x24d10` | `0x24d40` |
| `0x1b0` | `.text` VirtualSize | `0x2af83` | `0x2af93` |

[EVIDENCE] The Rich-count discrepancy is present in the modern control too;
it is not introduced by histlink. The initial “Rich identical” assumption does
not hold for this object/library order. Timestamp, remaining PE header bytes,
file size, section RVAs/raw sizes, `.data` size/BSS extent and `.rsrc` agree.

[EVIDENCE] A private fresh HEAD build also passes every accepted function,
with expected sound historical-path INCOMPLETE (`round2-gate.log`, exit 0).
Its longer sound path shifts data, so fresh sandbox images are recorded
separately under `round2-fresh-*` and are not substituted into the primary
oracle image comparison. They give the same ICF-owner conclusions.

## Reproduce and artifacts

```powershell
.pyenv\python.exe tools/relink.py --objects-dir build/sol-icf/round2-inputs/objects --out-dir build/sol-icf/round2-historical --historical-host --diagnostic
.pyenv\python.exe scratch/sol-icf/round2_report.py
.pyenv\python.exe scratch/sol-icf/smoke_histlink.py
```

[EVIDENCE] Reports include input/tool hashes, all 111 IAT addresses/readbacks,
loaded modules, actual clock callers/returns, complete relevant qsort vectors,
and zero-active-process cleanup checks. See `round2-report.json`, output
`historical-host.json`, `build.json`, `gate.json`, `layout.json` and maps.
`histlink.patch` includes tools only; no docs, game source, assets, external
reference, acceptance record or toolchain file is edited. No game/linked
image was executed, no commit made, and no background process remains.

## Text to append to docs/RELINK.md (not applied)

### Historical LINK host (sol-icf round 2, 2026-10-18)

[EVIDENCE] `tools/relink.py --historical-host` runs the unchanged SP3 LINK
under `tools/histlink.py`. Before LINK's entry instruction, all 111 MSVCRT
imports (including data) bind to a byte-identical private copy of VC5 SP3's
redistributable, bypassing the modern KnownDLL without modifying either tool
file. An audit records each binding/readback. Repo-local TMP/TEMP, a bounded
synchronous debugger and a kill-on-close tool-tree job preserve host safety.

[EVIDENCE] The process-local wall clock is 1998-10-14 07:17:39 UTC, obtained
from the oracle's `0x36245013` TimeDateStamp. GetLocalTime/GetSystemTime,
FILETIME wall-clock APIs and time-zone information report that instant in UTC;
`TZ=UTC0` gives deterministic CRT conversion. The original CRT time() returns
that value at LINK return site `0x443a32`; ctime formats the map. GetTickCount
remains native elapsed timing. Original OS/locale/boot uptime are not inferred.
No output is patched. Independent historical links are whole-file identical.

[EVIDENCE] With HEAD 7e69994's supplied objects/import-libraries-before-LIBC
order, historical ICF keeps button's empty ret, sound's getter and **frame's**
filter. Both complete 793-entry permutations match the recovered rules;
ehvecdtr is not selected with these inputs. Entry is `0x24d10`, oracle
`0x24d40`. Full historical qsort also permutes import INT/IAT contributions:
relative to the ICF-only control, 260 code-reference bytes and 396 import-array
bytes change, in addition to the four timestamp bytes. All extra differences
are accounted for. Clock-only control changes just the timestamp.

[EVIDENCE] Historical header has three differing bytes: `0x94` (Rich count
268 vs 270, already differing in modern link), `0xd8` (entry), `0x1b0`
(`.text` VirtualSize 0x2af83 vs 0x2af93). File size remains 535,552;
`.text` has 109,395 same-offset equal bytes. Accepted-function gate is clean.
[OPEN] Recover the remaining original input population/order and open code
bodies; this launcher does not establish a byte-identical oracle relink.
