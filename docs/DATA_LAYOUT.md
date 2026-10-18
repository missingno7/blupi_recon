# sol-relink: initialized data recovered; unreferenced BSS remains open

[EVIDENCE] Delivery is `scratch/sol-relink/data.patch`, checked against
**bc81fef**. `delivery.json` names the seven candidate files; only those files
belong to the patch. No canonical source, manifest, documentation or compiler
catalog was edited by this worker. Other workers advanced HEAD from f47a4e6
while this work ran (resources, removal of empty obstacle.cpp, default entry,
and `_ftime`/`_mkdir`). The candidates include those integrated changes.

## Result and limits

[EVIDENCE] The original **48,964-byte** virtual `.data` deficit is reduced to
**128 bytes** in a diagnostic reusing the provided, path-correct sound object.
The fresh sandbox build is **96 bytes short**: its longer sound source path
adds 16 string bytes and changes subsequent alignment. The smaller sandbox
number does not represent a better reconstruction. Sound's gate remains
INCOMPLETE; no drive was mapped and no source-path substitution was introduced.

[EVIDENCE] Final historical-path diagnostic:

| Item | Oracle | Candidate |
|---|---:|---:|
| `.data` RVA | 0x2e000 | 0x2e000 |
| Virtual size | 0x51e1c | 0x51d9c (-128) |
| Raw size | 0x50800 | 0x50800 |
| Virtual tail after raw bytes | 0x161c | 0x159c |
| Equal raw `.data` bytes at identical offsets | 329,728 | 329,711 |

[EVIDENCE] The **17 unequal bytes** occupy 16 pointer fields, not missing
numeric tables: fourteen fields point into `.text`; `_iob`'s two pointers at
RVAs 0x7e300/0x7e308 point at the shifted CRT `__bufin`. All locations and
unmodified values are in `build/sol-relink/final-accounting.json`. This is
**not** normalized/strict equality. The first raw difference remains
`.data+0xc` (RVA 0x2e00c), a CRT initializer pointer to shifted code.

[EVIDENCE] Every independently located initialized game-data symbol now has
its oracle placement in the historical-path diagnostic. Its nine ordinary
COFF data bands compare completely and directly equal, including compiler
padding (table below). All initialized CRT data anchors also have zero drift.
The first remaining **storage placement** drift is `g_hWnd`: oracle RVA
**0x7e618**, linked **0x7e610** (-8), at the start of the unresolved BSS area.

[EVIDENCE] Final verification preserves **all 409 active accepted entries**;
zero regressions, expected sound INCOMPLETE. All **460 surviving raw function
bodies** are unchanged from the input object set. Three unreferenced SB2
big-number functions are removed. `.text` remains 32 bytes short; `.rdata`
size remains exact. Current HEAD's independently reconstructed `.rsrc` is exact
in this link; that resource work is not part of this delivery.

## Object-by-object initialized layout

[EVIDENCE] Ranges below are RVAs, end exclusive. They describe the primary
contribution band, including its pooled-string first definitions and alignment.
Identical string COMDATs shared with earlier objects are exempt from single-TU
ownership. Public symbols come from `/MAP`; local-only arrays are placed by
linked caller relocations and independent oracle code bindings.

| Object | Oracle/final primary `.data` band | Ordinary numeric/record band bytes |
|---|---|---:|
| 10_blupi | 0x2e030..0x2e4b0 | 16 |
| 00_button | 0x2e4b0..0x2e4b8 | 8 |
| 01_ddutil | 0x2e4b8..0x2e6e8 | 4 |
| 14_decblock | 0x2e6e8..0x34418 | 23,792 |
| 15_decdesign | 0x34418..0x34440 | strings only |
| 12_decor | 0x34440..0x34480 | strings only |
| 13_event | 0x34480..0x77f38 | 276,612 |
| 03_menu | 0x77f38..0x77f80 | 70 + 2 alignment |
| 04_misc | 0x77f80..0x78aa8 | strings only |
| 05_movie | 0x78aa8..0x78ab4 | strings only |
| 06_network | 0x78ab4..0x78dd8 | strings only; GUIDs in `.rdata` |
| 18_dectables | 0x78dd8..0x7b260 | 9,352 |
| 07_pixmap | 0x7b260..0x7b430 | 32 |
| 08_sound | 0x7b430..0x7b530 | 84 |
| 09_text | 0x7b530..0x7bc40 | 1,808 |
| CRT | 0x7bc40..0x7e60c | initialized payload ends before BSS alignment |

[EVIDENCE] Jauge, decblupi, decnet and decmove have no ordinary initialized
data contribution. `.CRT$*` occupies 0x2e000..0x2e030 before game data.
The nine wholly equal ordinary game bands (excluding menu, which has no
fixed-code table binding) are independently audited in `final-accounting.json`.

## Applied source fixes

### event.cpp: +49,588 payload bytes

[EVIDENCE] `Phase` is 7,084 bytes: UINT, char[32], two BOOLs, and 40 176-byte
Buttons. STRICT callers bind the table to VA 0x4345d0. The existing 32 records
occupy 226,688 bytes and match there exactly. They stop at RVA 0x6bb50, inside
the oracle table. The missing records are:

| Index | RVA | Phase | Background |
|---|---:|---:|---|
| 32 | 0x6bb50 | 0x600 | image%.2d\\movie.blp |
| 33 | 0x6d6fc | 0x601 | image%.2d\\movie.blp |
| 34 | 0x6f2a8 | 0x60c | image%.2d\\movie.blp |
| 35 | 0x70e54 | 0x60f | image%.2d\\movie.blp |
| 36 | 0x72a00 | 0x614 | image%.2d\\bye.blp |
| 37 | 0x745ac | 0x607 | image%.2d\\insert.blp |
| 38 | 0x76158 | 0 | zero sentinel |

[EVIDENCE] The insert record's only nonzero button has message 0x609, type 0,
menu {1,40}, position (16,424), tips {1,106}. Other button fields are zero.
`SearchPhase` scans until phase==0; the missing sentinel has behavioral
evidence as well as its visible zero record. The corrected 39-record payload
is **276,276 exact bytes** through 0x77d03; the `ab` string follows at 0x77d04.
These are typed Phase/Button initializers, with no copied byte blob or padding.

### decblock.cpp: -192 COFF bytes, corrected order

[EVIDENCE] The first baseline **initialized payload/order** drift starts at
RVA **0x2e7e0** (`.data+0x7e0`): ventillod was placed before lave, while the
oracle has lave/eau1/eau2/ventillog/ventillod/ventilloh/ventillob followed by
the directional wind arrays. Definitions are now sorted by independent
reference/payload locations; their numeric contents are unchanged.

[EVIDENCE] Eight unreferenced definitions had no separate oracle slots:
`table_369d0`, `table_369e8`, `table_36610`, `table_36630`,
`table_guepe_turn2r`, `table_explo6`, `table_explo8`, `table_invertstart`.
Their COFF spans including natural alignment are respectively
24+32+24+8+24+24+24+32 = **192 bytes**. The first three duplicate the located
36530/365a8/365d0 tables. Explo8 is already the suffix of the referenced
nine-element explo4; invertstart duplicates pollution. No active caller
references a removed name. Shared declarations are retained to preserve
compiler context. The complete corrected 23,792-byte numeric band is exact.

### pixtables.h / relink.py: original atlas order and data-only TU slot

[EVIDENCE] Oracle atlas order is blupi (0x78dd8), object (0x79b80), element
(0x7a710), explo (0x7afd8), immediately before MouseHotSpot (0x7b260).
The existing header ordered blupi/element/explo/object; the data-only object
was linked after text. The candidate orders complete definitions and places
dectables.obj immediately before pixmap.obj in `oracle_order`. All atlas
initializer values and extents, including explo's established 648-byte
boundary, remain unchanged. No explicit alignment/padding is added.

[EVIDENCE] The final data-only COFF band is 9,352 bytes (the old order emitted
9,346); the six-byte difference is compiler alignment, not new numeric data.
Every byte at its actual oracle offset is equal. Keeping the TU separate also
restores the CRT initialized-data pull order. A measured alternative putting
the header in pixmap.cpp preserved the gate but left CRT data ordered
differently; it is excluded from delivery. Current HEAD's 270-object Rich
count is preserved (including the independently integrated CVTRES entry).

[HYPOTHESIS] The original file's name was not necessarily dectables.cpp.
The evidence establishes this data-only band and its link position; the
reconstruction keeps its existing manifest filename.

### text.cpp / texttables.h: -560 ordinary COFF bytes, corrected order

[EVIDENCE] The original text band is table_char[1536], the 15-byte accents
table plus one alignment byte, and two local byte-width tables[128]. Moving
the data header before the functions restores that order. The unreferenced
global int table_width[128] has no oracle payload hit and cannot fit this
band; it is unrelated to GetCharWidth's proven local byte tables. Remove it
(-512 bytes). Three unreferenced SB2 big-number routines have no oracle code
slots in the completed text block; their local coordinate table contributes
44 bytes plus four COFF alignment bytes (-48). The resulting **1,808-byte**
ordinary section is wholly equal, with all seven accepted text functions
preserved. Deleting these routines does not remove any game dependency.

## Measured link deltas per fix

[EVIDENCE] Each source stage used `verify.py --build-dir sol-relink --tu ...`;
all accepted gates had zero regressions and expected sound INCOMPLETE. Saved
objects were re-linked serially with the common 1b9fe7c resource/default-entry
setup to avoid attributing those other workers' changes to these fixes:

| Stage | `.data` virtual size delta | First initialized-data placement issue |
|---|---:|---|
| Fresh canonical sandbox baseline | -48,928 | decblock at 0x2e7e0 |
| Restore seven Phase records | +672 | decblock at 0x2e7e0 |
| Correct decblock | +480 | atlas band at 0x78dd8 |
| Atlas-order/owner diagnostic | +480 | text; sound-path exception begins at 0x7b484 |
| Correct text | -96 | CRT order; sound-path exception |
| Final separate data TU/link position | -96 | sandbox path/BSS only |
| Final with provided historical-path sound object | **-128** | first BSS global at 0x7e618 |

[EVIDENCE] Per-step linker changes include natural alignment and are not
assumed equal to source payload changes. Semantic payload accounting is
49,588 - 192 - 560 = **48,836 recovered net bytes**; 48,964 - 48,836 = 128.
Final objects reflect bc81fef's additional integrated CRT spelling changes;
the virtual deficits above are unchanged. Artifact roots:
`build/sol-relink/{baseline,head-baseline,stages,final,final-historical}/`.

## Remaining 128 bytes: no admissible source fix established

[EVIDENCE] Initialized CRT payload ends at 0x47e60c. The natural next
eight-byte boundary is 0x47e610. Final historical-path BSS placements are:

| Symbol/range | Oracle VA | Linked VA | Difference/evidence |
|---|---:|---:|---|
| g_hWnd | 0x47e618 | 0x47e610 | -8; leading zero interval remains unexplained |
| g_CDPath | 0x47e628 | 0x47e618 | -16; another 8 beyond existing natural alignment |
| g_updateTimer | 0x47e72c | 0x47e71c | -16 |
| WindowProc hInstance | 0x47e734 | 0x47e720 | -20; another four-byte interval |
| misc g_lastSprite | 0x47e770 | 0x47e75c | -20 |
| unreferenced interval | 0x47e774..0x47e7dc | absent before crt0dat | **104 bytes** |
| crt0dat errno / __pgmptr | 0x47e7dc / 0x47e814 | 0x47e760 / 0x47e798 | -124 |
| frame and later CRT BSS / __FPinit | 0x47e8f4 / 0x47fe18 | 0x47e874 / 0x47fd98 | -128 |

[EVIDENCE] The final four-byte difference is ordinary CRT section alignment:
oracle wincrt0 ends at 0x47e834 and tzset begins at 0x47e838; linked wincrt0
ends at 0x47e7b8 and tzset begins immediately there. Thus 20 unresolved bytes
in/preceding blupi's run + 104 before crt0dat + four consequent alignment
bytes explain **128**. CRT communal `__bufin`, `___pioinfo`, `__crtheap`,
`__acmdln`, onexit pointers and `__FPinit` have the expected sizes/order in
the final link; no missing communal array explains the deficit.

[EVIDENCE] A full bytewise operand scan of oracle `.text`, plus pointer scans
of `.data`/`.rdata`, finds no reference into the leading eight bytes, the
four-byte timer gap or the 104-byte interval. The only apparent inner-gap
reference is 0x47e627, independently explained as g_CDPath-1. See
`bss-xrefs.json`. These bytes are zero in the original file-backed data.

[EVIDENCE] Two SP3 scalar probes reverse declarations but both emit
zeta/alpha/mid at +0/+4/+8, followed by explicit_zero=0 at +12. Thus the
uninitialized run is not generally in definition order. This compiler result
and its negative are recorded in candidate `catalog.json` (VC5-DATA-01).
An SB2-name ancestry probe does not identify the missing objects: a local
static's placement changes with its actual decorated context. No tested
ancestral names are inserted as fillers, and no VC6 hash formula is claimed.

[OPEN] The identities/types and original ownership of the 20+104 zero bytes
are not established. Assigning char[104], unused scalar flags, align pragmas
or any similar space-filling construct would merely force the size. No such
change is delivered. The 128-byte gap and 17 relocated pointer bytes remain
open; this is a partial closure, not a whole-image match.

## Verification and reproduction

[EVIDENCE] Final commands (static inspection only):

```powershell
.pyenv\python.exe tools/verify.py --build-dir sol-relink --tu src/event.cpp=scratch/sol-relink/data-src/event.cpp --tu src/decblock.cpp=scratch/sol-relink/data-src/decblock.cpp --tu src/dectables.cpp=scratch/sol-relink/data-src/dectables.cpp --tu src/text.cpp=scratch/sol-relink/data-src/text.cpp
.pyenv\python.exe scratch/sol-relink/run_candidate_relink.py --objects-dir build/sol-relink/final/objects --out-dir build/sol-relink/final --diagnostic
git apply --check scratch/sol-relink/data.patch
```

[EVIDENCE] The wrapper runs the exact candidate tools/relink.py source with
the canonical module location; compiler/linker subprocesses remain owned by
the sanctioned tool. `patch-audit.json` proves the patch applies to private
HEAD copies and reproduces all seven candidates byte-for-byte. Final gate
transcript: `gate-final.log`. No executable or ISO program was executed, no
original/reference/toolchain file was modified, and no commit or background
process was created by this worker.

[EVIDENCE] `datacmp.py` coverage is **317,810 / 334,706 initialized bytes**
(466 EXACT, ten ambiguous) with the provided historical sound object:
317,634 `.data` + 176 `.rdata`. The sandbox has one additional expected sound
path MISMATCH and 317,790 covered bytes. Ambiguous duplicate wind arrays are
not silently counted; the independent complete decblock-band comparison is
reported separately. The 17 raw pointer differences are never masked or
counted as strict matching bytes.

## Round 2: BSS closure and non-text audit (2026-10-18)

[EVIDENCE] This update supersedes Round 1's policy-dependent "no admissible
source fix" conclusion. The owner explicitly permits opaque representations
of independently proven unreferenced storage. Three blupi-band intervals are
represented by 8/8/4 bytes; the later 104-byte interval is represented by a
100-byte array plus four bytes of natural alignment. A further four bytes of
natural CRT alignment give **120 storage + 8 alignment = 128 bytes**.
No explicit padding, align directive, raw blob or renamed known global is used.

[EVIDENCE] The blupi intervals are 0x47e610..617, 0x47e620..627 and
0x47e730..733. Neighbours are g_hWnd=0x47e618, g_CDPath=0x47e628,
g_updateTimer=0x47e72c and WindowProc's hInstance=0x47e734. The later band
is after misc's last word (0x47e770) and before CRT errno (0x47e7dc):
natural alignment at 0x47e774..777, opaque bytes 0x47e778..7db. The existing
data-only dectables TU owns that representation within the proven link band.
[OPEN] Original names/types are unknown; the original TU of the later band is
not distinguishable among movie/network/dectables/pixmap/sound/text. This is
reconstruction ownership, not a claim that the original file was dectables.cpp.

[EVIDENCE] Name-order probes preserved hInstance's S31272 suffix. The recorded
VC5-DATA-01 ordering predicted all placements before the candidate compile;
every prediction was observed. **All 366 BSS operands from STRICT bodies
(187 game, 179 pinned LIBC) at 85 distinct effective addresses now equal the
oracle targets.** Historical-path .data virtual size is exactly 0x51e1c,
including the oracle 0x161c uninitialized file tail. First BSS drift: none.

[EVIDENCE] Explicit Win32/DX import libraries must precede LIBC/OLDNAMES to
recover the oracle hint/name layout: game KERNEL32 imports first, then the
other DLLs, then CRT KERNEL32 imports on rescan. This changes .rdata raw
same-offset equality **2470 -> 4546 / 4978**, with every name/hint and all
160 descriptor bytes exact. GUIDs (176 bytes) and non-pointer constants/strings
are exact. Remaining .rdata bytes: ILT 200, IAT 200, code pointers 32.

[EVIDENCE] Final .data equality is **329713 / 329728**: all 15 differing bytes
are in 14 pointers into shifted .text. All 44 non-import differing words
(30 .rdata, 14 .data) have identified code targets. Resources and every file
alignment tail are exact. Seven remaining header bytes are timestamp (4),
entry point (1), .text virtual size (1), and Rich object count (1: 268 vs 270).
[OPEN] The two original stamp-less inputs and exact oracle import-sort input
population/order remain unidentified. A model of 137 genuine imported
contributions reproduces all 128 current slots with the host CRT; historical
qsort on those same inputs does not reproduce the oracle. Five pinned LINK
5.x variants give the same .rdata. No import permutation or header is patched.

[EVIDENCE] Zero accepted regressions with the two candidate TUs. Sandbox
sound.cpp remains INCOMPLETE as required; the separately labelled historical
diagnostic reuses the existing build/verify/objects/08_sound.obj unchanged.
All 460 COFF function body byte strings and three anonymous EH code sections
are unchanged (not a claim of linked-image equality). The library-order
change alters ICF survivors and linked .text equality drops 30185 -> 23062;
its virtual deficit changes -32 -> -16. Full evidence, addresses, negatives
and reproduction: scratch/sol-relink/r2/FINDINGS.md, STRICT_BSS_MAP.md,
IMPORT_SLOT_DIFFS.md, data2.patch. No canonical files were edited by the worker.
