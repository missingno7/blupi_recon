# Speedy Blupi 1 translation-unit map

The per-function assignments are in [oracle/tu_map.csv](../oracle/tu_map.csv). This is a best-effort map of 421 function entries below RVA `0x24140`; the `kind` field in `oracle/functions.csv` retains its original inventory split. The map records direct source-object matches separately from source-order and behavior-based assignments.

## Evidence and interpretation

**[EVIDENCE]** I reran `tools/cmpfunc.py` against 109 existing objects: PB `*_o2_gy_gx.obj` and all SB2 `*o2*` variants. The fixed comparator result is in `build/tumap/cmpfunc_all.json`. I used only source functions with a single candidate location as direct anchors; repeated short bodies and locations that matched multiple source methods do not identify a source method by themselves. A static Capstone pass recorded literal, import, and direct-call references in `build/tumap/xref_report.tsv`.

**[EVIDENCE]** The CSV has exactly 421 unique rows. Its confidence totals are 162 `anchored`, 146 `bracketed`, and 113 `inferred`. Status totals are 204 `strict`, 10 `similar`, and 207 `none`; no row needed the `byte-equal`-only category. `similar` means the comparator found a fixed-byte match with relocation sites still unverified or a non-unique function identity. It is not a strict match. Status describes the matched COFF source body at that RVA; `size` remains the Ghidra body-byte count, and the evidence field records the COFF comparison size when available.

**[EVIDENCE]** Correction: RVA `0x1f1e0` is `misc.cpp`'s `InitHInstance`, not a trailing `menu.cpp` entry. In the PB source order it is misc's first function, immediately before anchored `LoadStringA` at `0x1f1f0`; the same-TU boundary-plus-successor placement proves the address. The previous map assigned the misc block start one function late.

**[EVIDENCE]** Correction: the network TU ends at `0x21030`; the 4-byte body there is followed by 16-byte-aligned pixmap functions at `0x21040` (`CPixmap` constructor), `0x210d0` (strict destructor), and `0x21170` (`SetDebug`). The constructor-to-destructor-to-`SetDebug` object order and alignment prove the pixmap boundary. The previous map extended `network.cpp` through the destructor.

**[EVIDENCE]** The earliest supported CRT member starts at `0x238b0`: the VC5 SP3 LIBC `strstr.obj` body has a unique byte hit there with four relocation bytes masked. `fclose.obj` and `fread.obj` similarly hit `0x23930` and `0x239b0` with 16 and 8 relocation bytes masked. These hits establish CRT ownership, not strict relocation equality; the map now marks `0x238b0`–`0x240c0` as CRT.

**[EVIDENCE]** `GetCharWidthA` now strictly anchors the text block at `0x23610`, immediately after the last matched sound function at `0x23600`; the `0x23610` row is reassigned from `sound.cpp` to `text.cpp`. New strict integrated anchors also update the pixmap, decor, and engine rows; see their per-function evidence in `oracle/tu_map.csv`.

**[EVIDENCE]** The clean combined verification strictly reproduces 22 functions in the pixmap block. Their rows are updated to anchored/strict with method identities in `oracle/tu_map.csv`; the unmatched constructor and Create-like body remain hypotheses. The integrated network object adds 13 STRICT inventory functions in its mapped block and two STRICT callbacks outside the function CSV; `JoinSession`, `CreateSession`, `Send`, and `Close` remain mismatches. The shared network header also makes `NetSendLobby`, `NetDraw`, and `ChatSend` STRICT; `DrawMap` shares NetDraw's 48-byte body and is excluded from duplicate acceptance.

**[EVIDENCE]** Seven additional strict source-function hits do not start at an entry in `oracle/functions.csv`: `WindowProc` at `0x1610`, `TimerStep` at `0x1c50`, the `CButton` constructor at `0x29f0`, the `CJauge` constructor at `0x1edd0`, the `CMovie` destructor at `0x20450`, and the network callbacks at `0x209c0` and `0x20b50`. They are verified object bodies but are not mapped function rows. The seven-digit RVA is always image-relative; `size` is the Ghidra body-byte count, not the span to the next entry.

**[HYPOTHESIS]** `tu` is the most likely original translation unit. The engine files are less certain than the utility/UI blocks: SB2 splits some CDecor code into `dec*.cpp` files, while the original may have grouped those methods differently. For that reason, `best_source` can point to an SB2 method in a different file than `tu`; file-only `best_source` entries leave the method identity open.

Confidence meanings: `anchored` means a unique source-object body starts at that oracle entry; `bracketed` means the entry falls inside a block located by source anchors and adjacent block boundaries; `inferred` means strings, imports, table references, call relationships, or source-list order favor the assignment. `unknown` is available but was not needed for the current best-effort partition; weak assignments are marked `inferred` and discussed below.

## Block table

The first and last addresses below are the first and last Ghidra function entries assigned to each block. “Anchored / strict” counts mapped entries, not orphan comparator targets.

| TU | First–last RVA | Functions | Body bytes | Anchored / strict | Ancestry classification | Value score |
|---|---:|---:|---:|---:|---|---:|
| `blupi.cpp` | `0x01000–0x01ef0` | 11 | 5,120 | 6 / 8 | SB-engine | 3,584 |
| `button.cpp` | `0x02a30–0x030a0` | 17 | 1,560 | 12 / 13 | SB-engine | 1,373 |
| `ddutil.cpp` | `0x03100–0x038d0` | 8 | 2,033 | 7 / 8 | PB-derived | 1,952 |
| `decblock.cpp` | `0x03910–0x05950` | 26 | 8,304 | 0 / 0 | Unknown / weak SB2 candidate | 2,906 |
| `decdesign.cpp` | `0x05a20–0x08e80` | 42 | 14,181 | 4 / 4 | SB-engine, data-backed | 7,091 |
| `decblupi.cpp` | `0x09280–0x10480` | 20 | 29,540 | 1 / 4 | SB-engine | 20,678 |
| `decnet.cpp` | `0x10670–0x109c0` | 6 | 860 | 4 / 5 | SB-engine | 671 |
| `decmove.cpp` | `0x10a00–0x13cb0` | 39 | 12,846 | 0 / 11 | SB-engine, data-backed | 8,478 |
| `decor.cpp` | `0x13d50–0x161d0` | 21 | 9,246 | 2 / 8 | SB-engine | 6,657 |
| `event.cpp` | `0x16220–0x1d230` | 86 | 34,724 | 28 / 47 | SB-engine | 31,252 |
| `jauge.cpp` | `0x1edf0–0x1efd0` | 8 | 436 | 7 / 7 | SB-engine | 392 |
| `menu.cpp` | `0x1eff0–0x1f1b0` | 8 | 407 | 7 / 8 | SB-engine | 366 |
| `misc.cpp` | `0x1f1e0–0x1ffa0` | 10 | 3,643 | 6 / 6 | PB-derived | 3,461 |
| `movie.cpp` | `0x20150–0x205b0` | 12 | 1,025 | 12 / 12 | PB-derived | 984 |
| `network.cpp` | `0x205d0–0x21030` | 17 | 2,287 | 13 / 13 | SB-engine, direct strict anchors | 2,264 |
| `pixmap.cpp` | `0x21040–0x22910` | 36 | 6,056 | 22 / 22 | PB-derived, partial | 5,026 |
| `sound.cpp` | `0x22940–0x23610` | 29 | 3,140 | 23 / 21 | PB-derived | 3,109 |
| `text.cpp` | `0x23660–0x239b0` | 9 | 1,091 | 4 / 4 | PB-derived, partial | 971 |
| `crt` | `0x23af0–0x240c0` | 16 | 1,487 | 4 / 3 | VC5 SP3 LIBC and `/GX` support | 1,472 |

`Value score` is a ranking aid, not a probability: summed Ghidra body bytes multiplied by an ancestry-strength weight from 0.28 to 0.99. The weights are strongest for direct PB/SB2 code anchors and weaker for data-only or boundary-only assignments.

**[EVIDENCE]** PB-derived code is directly represented in `ddutil.cpp`, `misc.cpp`, `movie.cpp`, and `sound.cpp`; the sound block also references the literal `F:\Profil\sound.cpp`. `pixmap.cpp` and `text.cpp` have fewer relocation-masked PB matches, so their whole-block ancestry is less certain. SB2 has strict or relocation-masked anchors in `blupi`, `button`, `decblupi`, `decdesign`, `decnet`, `decor`, `event`, `jauge`, and `menu`. `decdesign` has strict code anchors at `0x5ab0`, `0x5b40`, `0x8520`, and `0x8530`; its broader block assignment and `decmove` also use SB2 table/behavior evidence. `decblock` has no useful direct source-body anchor. The former `wave.cpp` range is CRT/runtime code per the boundary correction below, not evidence for a DirectSound sample TU.

## Link order

**[EVIDENCE]** Representative direct anchors appear in this order in `.text`: `blupi` (`0x1000`), `button` (`0x29f0`), `ddutil` (`0x3110`), `decdesign` (`0x5ab0`, `0x5b40`, `0x8520`, `0x8530`), `decblupi` (`0xf8c0`), `decnet` (`0x10670`), `decor` (`0x13d50`, `0x15de0`), `event` (`0x16360`), `jauge` (`0x1edd0`), `menu` (`0x1eff0`), `misc` (`0x1f1e0`), `movie` (`0x20150`), `pixmap` (`0x210d0`), `sound` (`0x22940`), `text` (`0x23660`), then VC5 CRT code (`0x23af0`). The strongest continuous blocks are `misc` (`0x1f1e0`–`0x20150`), `movie` (`0x20150`–`0x205d0`), `pixmap` (`0x21040`–`0x22940`), and `sound` (`0x22940`–`0x23660`); the sound source path is a literal in the oracle.

**[EVIDENCE]** The full partition in the CSV is `blupi ? button ? ddutil ? decblock ? decdesign ? decblupi ? decnet ? decmove ? decor ? event ? jauge ? menu ? misc ? movie ? network ? pixmap ? sound ? text ? crt`. The corrected `decdesign` block starts at `0x5a20`; its `SetCelHiliFromScreenPos` (`0x5ab0`) and `DeleteCel` (`0x5b40`) bodies compare STRICT with relocations verified, and object order plus 16-byte alignment brackets the neighboring entries. `decblupi` remains supported by movement-speed arrays and the `TestPath` match; `decmove` by the moving-object animation tables; `network` by DirectPlay error strings and imports. The broader engine assignments remain hypotheses.


**[EVIDENCE] CRT boundary correction:** `0x23af0` is the first identified VC5 SP3 LIBC entry, not a final `text.cpp` function. Compiling `VC\CRT\SRC\FOPEN.C` with `/D_CRTBLD /W3 /GX /O2 /ML /Gd /Gy` reproduces `_fsopen` strictly at `0x23af0` and `fopen` at `0x23b20`; the former calls `__getstream` and `__openfile`, and the latter passes `_SH_DENYNO` (`0x40`) to `_fsopen`. The `SPRINTF.C` probe independently matches `sprintf` at `0x23b40`. `GAME_TEXT_END` is therefore `0x23af0` (142,064 bytes from RVA `0x1000`), replacing the too-late `0x24140` split. Rows from `0x23af0` through `0x240c0` now use `tu=crt`; the remaining unnamed helpers are attributed from their position in the CRT block and their CRT or `/GX` code structure.

**[HYPOTHESIS]** The engine order is not a simple copy of the SB2 `.dsp` source list. The SB2 project lists `decblupi` before `decdesign`, and `decmove` before `decnet`; the table-backed helper and unique code anchors instead support the order shown above. The save/load candidates at `0x14880`, `0x14da0`, `0x153d0`, and `0x159f0` fall between two `decor.cpp` anchors and have SB2 ancestors in `decio.cpp`; the CSV groups them into `decor.cpp` because an original single CDecor TU is plausible, but that grouping remains open.

**[EVIDENCE]** The 201 mapped game bodies accepted STRICT in canonical objects have status `strict`. Three more `strict` rows in the CRT block identify `_fsopen`, `fopen`, and `sprintf` from VC5 SP3 source probes. Seven additional accepted game-function bodies are omitted from the function map because they have no corresponding start in `oracle/functions.csv`; confidence labels remain conservative where placement still comes from bracketing or TU evidence.
**[EVIDENCE] Boundary correction:** `decdesign.cpp` begins at `0x5a20`, not `0x5c20`. In the integrated `/Gy` object, forced comparisons of `SetCelHiliFromScreenPos` at `0x5ab0` and `DeleteCel` at `0x5b40` are code- and relocation-strict; their object order, the preceding/following function extents, and 16-byte alignment place the intervening rows in the same block. `oracle/tu_map.csv` now assigns `0x5a20` through `0x5ba0` to `decdesign.cpp`; the unrecognized neighboring bodies remain status `none`. `ObjectDelete` at `0x108a0` remains owned by `decnet.cpp`; the identical strict candidate in `decmove.cpp` is counted once under that owner.

## Matching priority

Ranked by body bytes × ancestry-strength weight, from highest expected matching value:

1. `event.cpp` — 34,724 bytes, score 31,252.
2. `decblupi.cpp` — 29,540 bytes, score 20,678.
3. `decmove.cpp` — 12,846 bytes, score 8,478.
4. `decdesign.cpp` — 14,181 bytes, score 7,091.
5. `decor.cpp` — 9,246 bytes, score 6,657.
6. `pixmap.cpp` — 6,056 bytes, score 5,026.
7. `blupi.cpp` — 5,120 bytes, score 3,584.
8. `misc.cpp` — 3,633 bytes, score 3,451.
9. `sound.cpp` — 3,140 bytes, score 3,109.
10. `decblock.cpp` — 8,304 bytes, score 2,906.
11. `network.cpp` — 2,568 bytes, score 2,264.
12. `ddutil.cpp` — 2,033 bytes, score 1,952.
13. `button.cpp` — 1,560 bytes, score 1,373.
14. `text.cpp` — 1,091 bytes, score 971.
15. `movie.cpp` — 1,025 bytes, score 984.
16. `decnet.cpp` — 860 bytes, score 671.
17. `jauge.cpp` — 436 bytes, score 392.
18. `menu.cpp` — 417 bytes, score 375.

Regeneration used the fixed comparator on the existing objects; summary and static xref artifacts are under ignored `build/tumap/`. The per-row map is a canonical CSV; no generated disassembly or object files are included in it.
