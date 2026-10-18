# sol-relink Round 2

[EVIDENCE] **BSS CLOSED; .rdata PARTIAL.** Started at 699e45d, with final
patch based on HEAD b536b151f97e5614168bfb1b04afde8e517c2a63. Intervening integrated ICF documentation is
preserved. No canonical source, manifest or documentation file was edited.
The historical-path diagnostic image is static analysis only.

## BSS intervals, ownership and representation

[EVIDENCE] Initialized payload ends at VA 0x47e60c; the first BSS boundary is
0x47e610. All intervals below are zero in the oracle's file-backed data and
are bounded by independently resolved code operands. The complete reference
map is [STRICT_BSS_MAP.md](STRICT_BSS_MAP.md): 187 game references at 21
effective addresses plus 179 STRICT pinned-LIBC references at 64 addresses.
The machine-readable audit records every function RVA, relocation offset,
resolved symbol address, addend, current linked operand and target. Every one
of the **366 operands / 85 addresses** has zero linked BSS drift.

| Oracle interval / anchor | Bytes | Evidence and representation |
|---|---:|---|
| 0x47e610..617 | 8 | Leading blupi run before g_hWnd=0x47e618; opaque byte[8] in blupi.cpp |
| 0x47e61c..61f | 4 | Existing natural alignment after g_hWnd; no declaration added |
| 0x47e620..627 | 8 | Inside blupi band before g_CDPath=0x47e628; opaque byte[8] |
| g_CDPath=0x47e628..72b | 260 | Known array, exact size; operand 0x47e627 is g_CDPath-1, not an unknown global |
| g_updateTimer=0x47e72c | 4 | Known scalar |
| 0x47e730..733 | 4 | Before WindowProc hInstance=0x47e734 in same blupi band; opaque word |
| 0x47e738..767 | 48 | Known explicitly zero-initialized blupi globals; all exact |
| ddutil empty string=0x47e768 | 1 | Existing empty literal plus natural end alignment |
| misc g_hInstance/g_lastSprite=0x47e76c/770 | 8 | Exact anchors; misc ends 0x47e774 |
| 0x47e774..777 | 4 | Natural 8-byte alignment for the following byte[100] |
| 0x47e778..7db | 100 | Opaque byte[100] in existing data-only dectables.cpp; errno follows at 0x47e7dc |
| crt0dat errno / __pgmptr | 4 each | Exact 0x47e7dc / 0x47e814; current and oracle agree |
| wincrt0 end -> tzset start | 4 | Ordinary CRT alignment at 0x47e834..837; no declaration added |
| CRT communal run | existing | __bufin=0x47ece0, __crtheap=0x47fe08, __FPinit=0x47fe18; no missing communal array |

[EVIDENCE] New payload is **8+8+4+100=120 bytes**. The two newly required
natural four-byte boundaries add eight, recovering **128** net bytes.
The 104-byte late interval is 4 automatic + 100 represented bytes. An earlier
prediction paragraph called the combined 124 bytes "storage"; the corrected
accounting is recorded without changing the pre-compile prediction history.

[EVIDENCE] Ordinary TU bands force the two internal blupi holes into blupi's
contribution; the leading hole is represented in its first BSS band. The
later band can only be bounded to the post-misc/pre-CRT link interval. Large
arrays raise a VC5 BSS contribution's alignment to eight; putting byte[100]
inside misc would wrongly move g_hInstance (oracle VA is 4 mod 8).
[OPEN] No other BSS anchor distinguishes movie/network/dectables/pixmap/sound/
text as the original owner of the later band. dectables is the existing
data-only reconstruction owner within that band. Original filename, names,
types, and whether any opaque interval held multiple original objects remain
unknown. This representation is admitted by the owner's Round 2 policy, not
by a claim of ancestry. No known global is renamed to tune layout.

[EVIDENCE] Planet Blupi b0270ab blupi.cpp and misc.cpp supply no extra
name-backed definition explaining these intervals; SB2 ancestry does not
identify them either. The whole-oracle operand scan from Round 1 found no
inner references except the independently explained g_CDPath-1. Zero bytes
alone are not treated as an original variable name/type.

## Placement predicted before compiling

[EVIDENCE] [PREDICTIONS.md](PREDICTIONS.md) was written before the candidate
compile. Two bounded full-TU name probes retain the local-static suffix
S31272 and measure this relative order (VC5-DATA-01):

`blupi_unknown_47e610 < g_hWnd < bss47e620Unknown < g_CDPath <
g_updateTimer < unknownBss47e730 < WindowProc::hInstance`.

[EVIDENCE] Removing the other probe names, retaining the three ordinary
uninitialized definitions after all functions, and adding byte[100] in the
data-only TU predicted every VA in the table. The actual COFF/map agrees.
Probes: build/sol-relink/r2/name-order.json, name-order-leading.json,
storage-probe.obj. The exact VC5 identifier hash remains unproved. These
opaque identifiers express address evidence, not original identifiers.

## Before/after link deltas

| Inputs / sanctioned relinker | .data virtual delta | .rdata equal / 4978 | .data raw equal / 329728 | First BSS drift |
|---|---:|---:|---:|---|
| before-historical, original library order | -128 | 2470 | 329711 | g_hWnd at 0x47e610 instead of 0x47e618 |
| historical, opaque storage only | 0 | 2470 | 329713 | none |
| final-historical, import libraries before LIBC | **0** | **4546** | **329713** | **none** |
| final-sandbox (degraded sound path) | +32 | 4546 | 327656 | expected sound .data path shift |

[EVIDENCE] Final .data VA 0x42e000, virtual size **0x51e1c**, raw size
**0x50800**, implicit uninitialized tail **0x161c**, and .rsrc VA/bytes all
equal the oracle. No initialized game-data drift remains. First raw .data
difference is +0xc (RVA 0x2e00c), a CRT code pointer, not a layout gap.

## .rdata: recovered cause and remaining sort population

[EVIDENCE] Putting LIBC before the import libraries extracts all KERNEL32
hint/name records in the first pass. The oracle has ten game KERNEL32 records,
then the other DLLs, then later CRT KERNEL32 records. The source/link correction
puts explicit Win32/DX5 import libraries first, LIBC/OLDNAMES last. This was
predicted before the relink. All **2198 hint/name bytes** and **160 descriptor
bytes** become exact; all 128 imported names/ordinals and hints were already
the same. DLL IAT/ILT group boundaries and terminators are exact.

[EVIDENCE] The network's ten DX5 GUIDs and APP_GUID (176 bytes) are already
exact in declaration order. Every non-pointer CRT constant and string is exact;
there is no GUID, missing literal, string-pooling or hint-content correction.
The 1367-byte ordinary .rdata band has only 21 differing code-pointer bytes;
the 168-byte .xdata$x has 11. An initial region label ended at 0x2c773; it was
corrected to the actual 0x2c777 end of user32.dll. The one alignment byte at
0x2c777 and four at 0x2c21c are exact.

[EVIDENCE] LINK static analysis identifies the import contribution sort:
0x440630 recognizes .idata$2/$4/$5, calls 0x40f830 at 0x440876; qsort call
0x40f886 compares module names at 0x40f8d0, returning zero on equal names.
It relinks the resulting contribution list. Thus the within-DLL permutation
is an unstable MSVCRT sort, not alphabetical ordering of imported functions.
Read-only archive extraction plus /VERBOSE recovers **137 .idata$5
contributions: 130 import records + seven null-thunk records**. FindClose and
SetConsoleCtrlHandler are loaded but discarded; their sort participation
matters. The host CRT model reproduces **all 128/128 current live slots**.
Artifact: build/sol-relink/r2/import-model/{contributions,models}.json.

[EVIDENCE, negative] Applying the VC5 QSORT.C algorithm to that exact current
population gives only 15/128 oracle slots; omitting the discarded records,
omitting nulls, reversing load order, or guessing from live hint/name layout
also fails. These are model controls, not source/input mutations. SP3, 7255,
1592, 1691 and 1197 genuine linkers all produce identical .rdata. The 1592
control changes resource conversion output; it is not the delivery toolchain.
The attempted hardware-only LINK qsort trace captured zero calls and is
inconclusive; it is not used to claim a runtime permutation. No debugger
memory, source, object, tool binary or import-output byte was patched.

[HYPOTHESIS] The oracle's dead import population/null-record ordering differs,
or its exact linker/runtime environment differs. The historical-runtime ICF
result in docs/LINK_ICF.md is relevant but is not proof of this import input.
[OPEN] Recover the original import-sort input population/order. Hint/name
payload and final PE omit dead contributions, so they do not uniquely reveal
it. Do not add dummy imports, rename globals, remove real code, or permute raw
ILT/IAT bytes to tune the result. No unsupported runtime/source fix is proposed.

## Every remaining non-.text whole-image difference

[EVIDENCE] Both files are **535552 bytes**. A full file-offset audit excludes
only the .text raw payload, includes every header, section tail, inter-section
gap and overlay, and accounts for **454 differing bytes**:

| Cause / location | Differing bytes | Evidence |
|---|---:|---|
| IAT, RVA 0x2c000..2c21b | 200 | Per-DLL slot permutation; all slot pairs listed in IMPORT_SLOT_DIFFS.md |
| ILT, RVA 0x2c8c0..2cadb | 200 | Identical paired permutation; same list gives both RVAs |
| CRT .rdata scope/handler pointers | 21 | 21 words point to identified shifted CRT function/funclet targets |
| .xdata$x pointers | 11 | Nine words point to blupi/event .text$x labels, all -0x10 |
| Initialized .data pointers | 15 | Fourteen words point to identified shifted CRT routines |
| COFF TimeDateStamp, file 0xb8..bb | 4 | Build-time metadata; deliberately not forced |
| AddressOfEntryPoint, file 0xd8 | 1 | 0x24d10 vs oracle 0x24d40; _WinMainCRTStartup code placement |
| .text VirtualSize, file 0x1b0 | 1 | 0x2af83 vs oracle 0x2af93; remaining code/layout deficit 16 |
| Rich count, file 0x94 | 1 | 268 vs oracle 270 stamp-less inputs; current 249 loaded library members + 19 game objects |

[OPEN] Two original stamp-less inputs are unidentified. The 268 count exists
before this Round 2 patch as well as after it; library order does not change
it. Do not restore empty objects or OLDNAMES aliases just to force the count.

[EVIDENCE] Everything else outside .text is byte-exact: DOS stub and other
Rich fields, all remaining headers/data directories, GUIDs, constants, strings,
import names/hints/descriptors, all other initialized data, all resource raw
bytes, every file-alignment tail and gap. Virtual BSS has the exact extent and
every STRICT target; no executable was run to inspect it.

[EVIDENCE] All 44 differing non-import words follow. These identify relocation
causes; CRT entries with unresolved relocations remain marked as such in the
JSON and are not promoted to STRICT by address adjustment. Anonymous EH code
uses COFF .xdata$x -> .text$x label evidence, not invented function symbols.

| Data RVA | Oracle pointer | Linked pointer | Target / object and offset |
|---:|---:|---:|---|
| 0x0002c2d8 | 0x0042429f | 0x0042428f | `??_L@YGXPAXIHP6EX0@Z1@Z+0x6f (LIBC:ehvecctr.obj)` |
| 0x0002c2e8 | 0x00424343 | 0x00424333 | `??_M@YGXPAXIHP6EX0@Z@Z+0x73 (LIBC:ehvecdtr.obj)` |
| 0x0002c2f4 | 0x004243c0 | 0x004243b0 | `?__ArrayUnwind@@YGXPAXIHP6EX0@Z@Z+0x40 (LIBC:ehvecdtr.obj)` |
| 0x0002c2f8 | 0x004243cd | 0x004243bd | `?__ArrayUnwind@@YGXPAXIHP6EX0@Z@Z+0x4d (LIBC:ehvecdtr.obj)` |
| 0x0002c304 | 0x00424e8a | 0x00424e5a | `_WinMainCRTStartup+0x14a (LIBC:wincrt0.obj)` |
| 0x0002c308 | 0x00424e9f | 0x00424e6f | `_WinMainCRTStartup+0x15f (LIBC:wincrt0.obj)` |
| 0x0002c3d4 | 0x00427366 | 0x00427336 | `___FrameUnwindToState+0x76 (LIBC:frame.obj)` |
| 0x0002c3d8 | 0x00427373 | 0x00427343 | `___FrameUnwindToState+0x83 (LIBC:frame.obj)` |
| 0x0002c3e8 | 0x00427521 | 0x00427511 | `?CallCatchBlock@@YAPAXPAUEHExceptionRecord@@PAUEHRegistrationNode@@PAU_CONTEXT@@PBU_s_FuncInfo@@PAXHK@Z+0xe1 (LIBC:frame.obj)` |
| 0x0002c3f0 | 0x004274e9 | 0x004274d9 | `?CallCatchBlock@@YAPAXPAUEHExceptionRecord@@PAUEHRegistrationNode@@PAU_CONTEXT@@PBU_s_FuncInfo@@PAXHK@Z+0xa9 (LIBC:frame.obj)` |
| 0x0002c3f4 | 0x004274f6 | 0x004274e6 | `?CallCatchBlock@@YAPAXPAUEHExceptionRecord@@PAUEHRegistrationNode@@PAU_CONTEXT@@PBU_s_FuncInfo@@PAXHK@Z+0xb6 (LIBC:frame.obj)` |
| 0x0002c3fc | 0x0042779b | 0x0042778b | `?BuildCatchObject@@YAXPAUEHExceptionRecord@@PAUEHRegistrationNode@@PBU_s_HandlerType@@PBU_s_CatchableType@@@Z+0x1db (LIBC:frame.obj)` |
| 0x0002c400 | 0x004277a1 | 0x00427791 | `?BuildCatchObject@@YAXPAUEHExceptionRecord@@PAUEHRegistrationNode@@PBU_s_HandlerType@@PBU_s_CatchableType@@@Z+0x1e1 (LIBC:frame.obj)` |
| 0x0002c40c | 0x0042781a | 0x0042780a | `?_DestructExceptionObject@@YAXPAUEHExceptionRecord@@E@Z+0x4a (LIBC:frame.obj)` |
| 0x0002c410 | 0x00427825 | 0x00427815 | `?_DestructExceptionObject@@YAXPAUEHExceptionRecord@@E@Z+0x55 (LIBC:frame.obj)` |
| 0x0002c420 | 0x0042793e | 0x0042792e | `?terminate@@YAXXZ+0x6e (LIBC:hooks.obj)` |
| 0x0002c428 | 0x00427911 | 0x00427901 | `?terminate@@YAXXZ+0x41 (LIBC:hooks.obj)` |
| 0x0002c42c | 0x00427917 | 0x00427907 | `?terminate@@YAXXZ+0x47 (LIBC:hooks.obj)` |
| 0x0002c438 | 0x004279ce | 0x004279be | `?_inconsistency@@YAXXZ+0x6e (LIBC:hooks.obj)` |
| 0x0002c440 | 0x004279a1 | 0x00427991 | `?_inconsistency@@YAXXZ+0x41 (LIBC:hooks.obj)` |
| 0x0002c444 | 0x004279a7 | 0x00427997 | `?_inconsistency@@YAXXZ+0x47 (LIBC:hooks.obj)` |
| 0x0002c79c | 0x0042bee0 | 0x0042bed0 | `$L31824+0x0 (10_blupi.obj)` |
| 0x0002c7a4 | 0x0042beeb | 0x0042bedb | `$L31825+0x0 (10_blupi.obj)` |
| 0x0002c7ac | 0x0042bef6 | 0x0042bee6 | `$L31826+0x0 (10_blupi.obj)` |
| 0x0002c7b4 | 0x0042bf01 | 0x0042bef1 | `$L31827+0x0 (10_blupi.obj)` |
| 0x0002c7bc | 0x0042bf0c | 0x0042befc | `$L31828+0x0 (10_blupi.obj)` |
| 0x0002c7c4 | 0x0042bf17 | 0x0042bf07 | `$L31829+0x0 (10_blupi.obj)` |
| 0x0002c7ec | 0x0042bf30 | 0x0042bf20 | `$L35580+0x0 (13_event.obj)` |
| 0x0002c814 | 0x0042bf60 | 0x0042bf50 | `$L35594+0x0 (13_event.obj)` |
| 0x0002c81c | 0x0042bf7b | 0x0042bf6b | `$L35595+0x0 (13_event.obj)` |
| 0x0002e00c | 0x00429cb0 | 0x00429c70 | `___initstdio+0x0 (LIBC:_file.obj)` |
| 0x0002e010 | 0x0042aa60 | 0x0042aa20 | `?__CxxSetUnhandledExceptionFilter@@YAXXZ+0x0 (LIBC:unhandld.obj)` |
| 0x0002e01c | 0x00429d70 | 0x00429d30 | `___endstdio+0x0 (LIBC:_file.obj)` |
| 0x0002e028 | 0x0042aa80 | 0x0042aa40 | `?__CxxRestoreUnhandledExceptionFilter@@YAXXZ+0x0 (LIBC:unhandld.obj)` |
| 0x0007bc60 | 0x00424bb0 | 0x00424b80 | `__exit+0x0 (LIBC:crt0dat.obj)` |
| 0x0007bd24 | 0x0042aa00 | 0x0042a9c0 | `?__CxxUnhandledExceptionFilter@@YGJPAU_EXCEPTION_POINTERS@@@Z+0x0 (LIBC:unhandld.obj)` |
| 0x0007bd30 | 0x004278d0 | 0x004278c0 | `?terminate@@YAXXZ+0x0 (LIBC:hooks.obj)` |
| 0x0007bd34 | 0x004278d0 | 0x004278c0 | `?terminate@@YAXXZ+0x0 (LIBC:hooks.obj)` |
| 0x0007e580 | 0x0042b2d0 | 0x0042b290 | `__fptrap+0x0 (LIBC:crt0fp.obj)` |
| 0x0007e584 | 0x0042b2d0 | 0x0042b290 | `__fptrap+0x0 (LIBC:crt0fp.obj)` |
| 0x0007e588 | 0x0042b2d0 | 0x0042b290 | `__fptrap+0x0 (LIBC:crt0fp.obj)` |
| 0x0007e58c | 0x0042b2d0 | 0x0042b290 | `__fptrap+0x0 (LIBC:crt0fp.obj)` |
| 0x0007e590 | 0x0042b2d0 | 0x0042b290 | `__fptrap+0x0 (LIBC:crt0fp.obj)` |
| 0x0007e594 | 0x0042b2d0 | 0x0042b290 | `__fptrap+0x0 (LIBC:crt0fp.obj)` |


## Verification, limitations and reproduction

[EVIDENCE] The required sol-relink gate has zero accepted regressions; sound's
historical F: mapping is unavailable in the sandbox and remains INCOMPLETE.
No drive was mapped, no #line was added. The separately labelled historical
diagnostic copies existing build/verify/objects/08_sound.obj unchanged; its
SHA-256 and all inputs are recorded. It is not a new authoritative sound build.
The sandbox result (+32 .data) is reported independently above.

[EVIDENCE] body-audit.json compares all **460 COFF function body byte strings**
and three anonymous EH code sections: unchanged. Three WindowProc local-label
IDs renumber with the new declarations, retaining their section offsets;
sound's __FILE__ relocation name differs as expected in the sandbox. These
metadata differences and linked relocated bytes are not called strict matches.
The actual accepted-function verifier is the gate, not this raw-byte control.

[EVIDENCE] Import-library extraction changes ICF survivor choices: the current
getter survivor becomes the sound copy; empty return aliases move from misc
to jauge. It does **not** solve all original fold choices. .text same-offset
equality drops from 30185 to 23062 and virtual delta changes -32 to -16. The
oracle hint/name and BSS evidence supports this input correction independently
of incidental ICF choices. docs/LINK_ICF.md retains the separate survivor debt.

```powershell
.pyenv\python.exe tools/verify.py --build-dir sol-relink --tu src/blupi.cpp=scratch/sol-relink/r2/src/blupi.cpp --tu src/dectables.cpp=scratch/sol-relink/r2/src/dectables.cpp
.pyenv\python.exe scratch/sol-relink/r2/prepare_final_artifacts.py
.pyenv\python.exe scratch/sol-relink/r2/run_relink.py --objects-dir build/sol-relink/r2/final-historical/objects --out-dir build/sol-relink/r2/final-historical --diagnostic
.pyenv\python.exe scratch/sol-relink/r2/inspect_nontext.py build/sol-relink/r2/final-historical
.pyenv\python.exe scratch/sol-relink/r2/audit_round2.py build/sol-relink/r2/final-historical
git apply --check scratch/sol-relink/data2.patch
```

[EVIDENCE] Candidate files are under r2/src, r2/relink.py, r2/catalog.json,
r2/DATA_LAYOUT.md and r2/PROVENANCE.md. data2.patch is HEAD-relative and
patch-audit.json records private-copy application and byte-for-byte candidate
reproduction. Focused scripts and complete raw evidence stay in scratch/build.
No canonical source/docs/manifest, original material, reference repo, toolchain
file or linked-image byte was edited; no commit, game execution or background
process was made/left running by this worker. All shell operations were serial.
