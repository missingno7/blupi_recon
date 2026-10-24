# Speedy Blupi 1: historical-closure ledger (2026-10-24)

Goal (user direction, 2026-10-24): reproduce the original `BLUPI.EXE` (US, SHA-256 `be6de5a2…90ac`) from meaningful
reconstructed source with the original toolchain, byte for byte, and explain every remaining difference. This file
tracks each remaining uncertainty, what is proven about it, and the next evidence that could close it. It
distinguishes (1) reproducing the linker output, (2) recovering the linker inputs, (3) recovering the source, and
(4) finding an alternative input set with the same output.

## Where it stands

| Item | Status | Evidence |
|---|---|---|
| Game code | 127,456 / 141,488 bytes STRICT; every function STRICT or FOLDED except ChangePhase (32 bytes) and TreatEventBase (8 bytes), both certified EQUIVALENT | docs/STATUS.md |
| `.data`, `.rsrc`, file/section sizes | byte-exact in the canonical relink except one CRT pointer byte | docs/BLOCKER_CHECKS.md |
| Whole image, canonical relink | 121,401 differing bytes (`557ca92e…`), mostly function placement | docs/BLOCKER_CHECKS.md |
| Whole image, experimental branch `sb1-histlink` | 596 differing bytes: 412/412 functions at original RVA, original entry; differences = import slot order (555), ChangePhase/TreatEventBase (40), Rich count (1) | evidence/sb1-histlink on that branch |

## Mechanisms now proven (reproducing the linker)

- [EVIDENCE] **Function emission order** (msvc5kb BLUPI-LAY-04): C2 emits file-static and inline functions only once a
  compiled reference needs them. The original places each DirectPlay callback before its caller, so the original
  callbacks had external linkage (`scratch/w-emitorder`).
- [EVIDENCE] **ICF survivors** (LINK 5.10 `0x431fb0`; RTM 5.00 `0x41e88d`): MSVCRT's unstable qsort over executable
  COMDAT contributions in load order, key (effective size, relocation count); the first eligible equivalent survives.
  REF-dead contributions take part. An exact replay predicts every sorted position and fold in 51+ real links
  (`scratch/w-sb1icf`, `scratch/w-histcause`, `scratch/w-deadpop`). RTM differs only in record width and in not
  admitting `.text$x` funclets.
- [EVIDENCE] **Import slot order** (LINK `0x40f88c`, comparator `0x40f8d0`): the same unstable qsort over `.idata$4/$5`
  contributions, keyed only on the import archive member name; reproduces 137/137 positions (`scratch/w-histimport`).
- [EVIDENCE] **Rich**: LINK counts every loaded module, including REF-dead and empty members; it stores no identities.
- [EVIDENCE] **Owner of the shared getter in Speedy Eggbert 1** (`0x20be0`): network.cpp (canonical since `a8eea8f`).

## Linker inputs: what the evidence determines

- [EVIDENCE] The original link contained **REF-dead executable code** that the canonical source lacks: with the
  evidenced external callbacks, no layout reaches the original survivors without additional dead contributions, and
  adding them reproduces 412/412 placement and the original entry.
- [NEGATIVE] **Which dead functions is not identified.** The ICF regions are broad (`scratch/w-histcause` joint.json):
  20 minimal subsets of the eight-helper roster place all game functions; names, bodies beyond their (size,
  relocation count) key, and class-layout views are irrelevant to the image.
- [NEGATIVE] **The eight-helper candidate on `sb1-histlink` is a US-only fit.** Under Speedy Eggbert 1's own RTM LINK it
  places only 261/408 functions; its two `Is*Traj` bodies (160 bytes) come from sb2-decomp's decompilation (double
  division), not from SB2's proven 112-byte form or the C# port (`scratch/w-deadpop`).
- [HYPOTHESIS, weak] A six-method population of SB2-proven bodies (SoundEnviron, SetBuildOfficialMissions, BlupiDead,
  DynamiteStart, AscenseurShift + MoveAscenseurDetect | IsFromage | IsGrotte) reproduces placement and entry in **all
  nine** SB1 images (412/412 Blupi, 408/408 Eggbert). But it is 3 of 4,053 US fits, about what chance predicts for an
  extra filter of that strength, it changes two existing payloads through header context (MouseMove 7 bytes,
  IsVentillo 371) and needs six unobserved tail fields. Not adopted.
- [EVIDENCE] **Imports**: exactly two extra import members cannot produce the original slot order (2.3 M cases, any
  keys). Six REF-dead imports (one KERNEL32 late in the game objects, two USER32 between movie and pixmap, three USER32
  after `wsprintfA`) reproduce all 128 slots and the INT/IAT bytes in a real link; with them Rich becomes 274, so the
  original must also have had four fewer loaded modules (e.g. a coarser TU partition: five CDecor TUs as one gives
  270) (`scratch/w-importsolve`). [OPEN] The APIs (only SetTimer, KillTimer, OutputDebugStringA are attested as
  unimported calls in SB1/SB2/Planet Blupi sources) and the original TU partition.
- [NEGATIVE] Library versions: VC5 RTM/SP2/SP3 system import libraries are identical; DX5 import libraries are
  confirmed by the original hints.

## June 1998 demo: tenth target and independent constraint

[EVIDENCE] `sb1/blupi/demo` is registered and verifiable, with its own audited inventory and RTM
compiler/library evidence: [SB1_DEMO.md](SB1_DEMO.md). The existing nine reports are identical under
like sandbox conditions, and the primary relink remains `557ca92e6ec8c71cfaf5e9f2d55969d3ebcd614c771c9010e074c6ad82ba3533`.
The demo's original physical owners are button destructor / network session getter / sound enable
getter / **FrameUnwindFilter** / **aw_cmp _strncnt**; entry `0x24a60`. Its CRT choices differ from
the successful retail layout and were derived independently from complete member-body placements.

[EVIDENCE / conditional model] The binary fixup decoder reproduces all sites in 419 US and 418
Eggbert bodies. The June executable-COMDAT model uses real COFF keys and 23 binary-derived live
keys, while retaining explicit assumptions for unobserved dead definitions and original CRT closure.
RTM replay agrees at all 792 positions in a forecast frozen before a real six-method diagnostic link.

[NEGATIVE] In the measured 42-method family through cardinality six, the sequence is **4,053 US
fits -> 3 US+Eggbert fits -> 0 with the demo**. All three lose June game-owner constraints even
without checking the CRT owners. The six-method source witness also changes MouseMove (7 bytes)
and IsVentillo (371) on all three images. The fixed eight-helper subset family already has zero
US+Eggbert fits; June does not narrow that empty family further. No tested common family preserves
every existing payload and satisfies all three links. These bounds do not exhaust larger or
revision-specific dead populations.

[EVIDENCE] The **same +6 import encounter pattern survives all three links**, giving 128/128
slots in each; the RTM demo witness was linked for real. Its baseline gives 15/128. The pattern
still does not recover API identities. June has no Rich header, so it cannot decide the **-4
loaded modules** or original TU partition. Full byte counts, source-gate failures, model boundaries
and process cleanup receipts are in `scratch/w-demo10/REPORT.md` and its JSON evidence.

## Dead-import source search (2026-10-24, `scratch/w-deadimports/REPORT.md`)

[EVIDENCE] Encounter windows pinned by COFF symbol index in all three links: the KERNEL32 reference after misc.obj's
`CreateDirectoryA` (symbol 128, `AddUserPath`) up to the KERNEL32 null; the USER32 pair after movie.obj's
`InvalidateRect` (symbol 30) and before pixmap.obj's `GetClientRect` (symbol 251 retail / 241 June); the last three
USER32 after sound.obj's `wsprintfA` (symbol 21, `ReadData`). 608 KERNEL32 and 524 USER32 archive APIs are absent
from every SB1 image; neither SB2 executable imports any USER32 function SB1 lacks.

[NEGATIVE] No attested source fills all six. Later Epsitec code can fill the first two windows (Colobot
`DisplayError`: OutputDebugStringA; DX7 `FlipToGDISurface`: RedrawWindow/DrawMenuBar; Blupimania2 capture wrappers:
SetCapture/ReleaseCapture), but each breaks the ICF survivors of another image; nothing attested supplies the last
three USER32 references. 18 real links, every existing US/Eggbert payload unchanged.

## `.data` virtual size of the SB1 secondaries (2026-10-24, `scratch/w-bss32/REPORT.md`)

[EVIDENCE] The 32-byte `.data` virtual-size excess of every SB1 secondary relink (both Blupi languages, all six
Eggbert 1 languages) is a sandbox artefact: the sandbox `sound.cpp` path makes the `__FILE__` literal longer, shifting
initialized data by 16 bytes, which the CRT's `__bufin` 16-byte alignment rounds to 32. With the historical-path
`sound.obj` all nine retail targets have the exact virtual size and BSS placement. No source or CRT change is needed;
builds must use the historical build path (docs/BUILD_PATH.md).

## Translation-unit partition (2026-10-25, `scratch/w-tupart/REPORT.md`)

[EVIDENCE] `.data` cannot fix the original object boundaries: PE data has no boundary marker and input alignments of 4
or 8 bytes allow a zero gap inside or between objects. Four of our objects contribute no data at all (decblupi, decnet,
decmove, jauge). Six whole-file concatenations are rejected in all three images (US, Eggbert 1, June demo) because they
would place a later table before an earlier literal (blupi+button, ddutil+decblock, decor+event, network+dectables,
pixmap+sound, sound+text), and event/jauge/menu must contain a split. Two 15-object groupings (A: decblock..decmove as
one TU; B: decdesign..decor as one TU) reproduce the engine data bytes of all three images exactly as today's 19 do,
and either gives Rich 270 with the six-dead-import witness.

[NEGATIVE] A real compile of each grouping keeps most current STRICT functions but not all: A 130/133 (GetBlupiInfo,
MockeryDetect, MoveObjectSort break), B 125/128 (IsVentillo, ModifDecor, MoveObjectSort break), identically in US and
Eggbert 1. Sparse breakage neither confirms nor excludes merged originals. [OPEN] The original TU count, filenames and
the four modules the import witness removes; an authenticated project/object list would decide it.

## Open items and the next evidence for each

| Open item | Next evidence that could decide it |
|---|---|
| Dead-function population (ICF) | The June demo now rejects the bounded common six-method fits; next: original project/object lists, authenticated revision-specific dead bodies, larger dependency-closed families and original CRT/member closure |
| Import order + Rich (+6 dead imports, -4 modules) | The same joint model; a source revision with dead USER32 calls in the movie→pixmap and post-text regions; the original TU partition (docs/TU_MAP.md) |
| ChangePhase (32 bytes), TreatEventBase (8 bytes) | Independent register-allocation track (docs/STATUS.md). [NEGATIVE] The linker work gives no new evidence about their source context. |
| `.data` CRT pointer byte | follows from the CRT filter survivor; resolved in the 412/412 layout (`.data` byte-exact there) |

## Policy refinement (proposal, not adopted)

AGENTS.md removes SB2/Planet Blupi code that no SB1 executable shows. Linker layout is indirect evidence, so a
four-level classification is proposed for SB1 source:

1. **Present**: the function's body is in the executable (STRICT/FOLDED/EQUIVALENT).
2. **Source-supported**: historical source attests it in SB1's lineage, but the executable has no body.
3. **Linker-inferred**: a dead-stripped function required by reproducible linker constraints that are consistent
   across **all** independent SB1 images and change no existing payload, with its body proven elsewhere (e.g. SB2).
4. **Speculative**: anything that merely improves linker output.

Only levels 1-3 would enter canonical source, each labelled; level 3 additionally needs a documented cross-build
witness. Level 4 stays on experimental branches. Today no candidate reaches level 3: the cross-build fit is weak
and not payload-preserving.
