# Oracle inventory and function discovery

## [EVIDENCE] Primary image and inventory

`assets/Speedy Blupi/BLUPI.EXE` is the primary oracle: SHA-256 `be6de5a206bc636346dde1b1a4dd11ddc3bda30c6efe64eebe96513f9c8290ac`, 535,552 bytes, PE32 i386, timestamp `1998-10-14 07:17:39 UTC`, linker 5.10, image base `0x400000`, entry RVA `0x24d40`. Its version resource identifies `EPSITEC Speedy Blupi`, file version `0.4.0.0`; the Rich pairs are `(0x00000, 270)` and `(0x60684, 1)`. The base-relocation and debug data directories are both empty.

| Section | RVA | Virtual size | Raw size |
|---|---:|---:|---:|
| `.text` | `0x1000` | `0x2af93` | `0x2b000` |
| `.rdata` | `0x2c000` | `0x1372` | `0x1400` |
| `.data` | `0x2e000` | `0x51e1c` | `0x50800` |
| `.rsrc` | `0x80000` | `0x5b88` | `0x5c00` |

The primary imports 128 functions from `KERNEL32.dll` (66), `USER32.dll` (35), `GDI32.dll` (11), `WINMM.dll` (12), `DSOUND.dll` (1), `DDRAW.dll` (1), and `DPLAYX.dll` (2). Its version-resource translation is French (Switzerland), code page 1200; most resource leaves have neutral language IDs.

`oracle/inventory.csv` has one compact row for every PE found directly under `assets/` and in the extracted ISO: 12 EXEs and 122 DLLs. `build/oracle/inventory.json` contains the full per-file records, including import names, sections, version strings, Rich pairs, resource language IDs, hashes, and `.text` comparisons. `ddraw.dll` and `cnc-ddraw config.exe` are present in the supplied game directory but are not the game oracle.

## [EVIDENCE] Blupi executable variants

All five `BLUPI.EXE` files have image base `0x400000`, entry RVA `0x24d40`, linker 5.10, and the same `.text` size (`0x2b000`) at RVA `0x1000`.

| Copy | Size | SHA-256 | `.text` result versus primary |
|---|---:|---|---|
| `assets/Speedy Blupi/BLUPI.EXE` | 535,552 | `be6de5a206bc636346dde1b1a4dd11ddc3bda30c6efe64eebe96513f9c8290ac` | Primary |
| ISO `ENGLISH/BLUPI.EXE` | 535,552 | same as primary | Whole file and `.text` identical |
| ISO `US/BLUPI.EXE` | 535,552 | same as primary | Whole file and `.text` identical |
| ISO `FRANCAIS/BLUPI.EXE` | 538,112 | `e2ae251daf61730243506fef1663dd49115f0e3cbabdbdb5af64022ded71479d` | `.text` identical; whole file differs |
| ISO `DEUTSCH/BLUPI.EXE` | 538,112 | `356e190d0b606f8826cb23b0706fe39be31b201e29081dd147d9f20bf55b19c4` | 4,540 aligned `.text` bytes differ; no fixed displacement detected |

The primary and ISO English/US copies share the full executable hash. French has the same `.text` hash as the primary (`5f7b838f8c2e2d32fdd99e57104351c8ac1bcc7eb211ed1fddfd30cd5f292cd6`). German retains the same `.text` RVA and raw size, but differs at aligned offsets; this does not establish how many changed bytes are instructions versus embedded constants.

## [HYPOTHESIS] Build and language relationship

The ISO directory names, common 1998 timestamp, matching entry/linker/layout, and shared version-resource product identify the German, French, English, and US files as localized Speedy Blupi 1 builds. The primary is the English package copy: its adjacent `README.TXT` is English, and it is byte-identical to the ISO English and US copies. The embedded version translation remains French (Switzerland), so the package label and resource translation are recorded separately.

## [EVIDENCE] Function map

Ghidra 12.1 headless discovered 579 function bodies in the primary `.text`; the function entries cover 166,631 body bytes. `oracle/functions.csv` contains `rva,size,end,name,kind`. `size` is the Ghidra body-byte count; `end` is one past the highest body address. For the 11 functions with split bodies, those values are not a contiguous span; `build/oracle/ghidra_ranges.csv` records each exact address range.

| Kind | Functions | Body bytes |
|---|---:|---:|
| Game | 421 | 137,986 |
| CRT/library | 151 | 28,605 |
| Thunk | 7 | 40 |

The seven thunk entries are at RVAs `0x20450`, `0x23890`–`0x238a2`, `0x2793e`, and `0x2bed0`. The DirectSound, DirectDraw, and DirectPlay entries in `0x23890`–`0x238a2`, plus `RtlUnwind` at `0x2bed0`, jump through the import table.

`WinMain` is at RVA `0x1e30`. The PE entry at `0x24d40` parses the command line, gets the module handle, pushes the four WinMain arguments, and calls `0x401e30`; the callee reads the four stack arguments and returns with `ret 0x10`. The entry is labeled `WinMainCRTStartup` in the CSV.

## [EVIDENCE] CRT boundary correction

The earlier `0x24140` split was too late. A VC5 SP3 compile of `VC\CRT\SRC\FOPEN.C` strictly matches `_fsopen` at `0x23af0` and `fopen` at `0x23b20`, with their helper relocations verified; `SPRINTF.C` strictly matches `sprintf` at `0x23b40`. A scan of the VC5 SP3 LIBC archive places `_strstr` at `0x238b0`, `_fclose` at `0x23930`, and `_fread` at `0x239b0`; each has a unique archive-member byte hit with relocation bytes masked. This moves the first supported CRT entry and game coverage end to `0x238b0` (141,488 bytes from RVA `0x1000`); archive hits establish CRT ownership but are not strict relocation proofs. `oracle/tu_map.csv` reclassifies `0x238b0`–`0x240c0` as CRT. The existing `kind` labels in `oracle/functions.csv` retain their original inventory split.

## [EVIDENCE] Vtables and strings

No RTTI name markers (`.?AV`, `.?AU`, `.?AT`, `.?AW`) were found, and no distinct-target function-pointer run qualified as a vtable. The two exact `.data` pointer runs repeat one function each: RVA `0x7bd30` has two pointers to `FUN_004278d0`; RVA `0x7e580` has six pointers to `FUN_0042b2d0`. They are listed in `build/oracle/pointer_runs.json`, while `oracle/vtables.csv` has no confirmed entries.

The PE resource tree has 31 `RT_STRING` blocks with 329 nonempty neutral-language strings. Block IDs are `7–19`, `26`, `32–33`, `38–39`, `63–67`, `126–128`, `130`, `188`, `194–195`, and `201`; their data spans RVA `0x826e8`–`0x85b82`. Each string's ID, RVA, and text is in `build/oracle/resource_strings.csv`. Printable string extraction found 146 strings in `.rdata` (RVA `0x2c349`–`0x2d366`) and 313 in `.data` (RVA `0x2e040`–`0x7b510`), including game asset paths and CRT/import text; see `build/oracle/strings.csv`.

## [EVIDENCE] eGames "Speedy Eggbert" builds (second oracle family, 2026-10-18)

Downloaded with the owner's approval, analysed statically only, stored outside the repo in `C:\tools\download\eggbert\`
(archive.org `speedy-eggbert-egames`: zip md5 eea7da256778be7a9e62885f8eb6017f, ISO md5 972f6d09b6bf9fb045f6644df74176e7,
both matching the published hashes). Scripts: scratch/research/egfind.py, egcmp.py.

- `egames.exe` (EN, 534,528 B, SHA-256 c759447edb409846..., link timestamp 1999-06-24 15:13:35): **linker 5.00**, no Rich
  header, separate `.idata` (IAT at 0x7f2bc), `.text` 0x2ac93, `.data` at 0x2d000. A different link setup from the
  oracle's linker 5.10. The ISO's Deu/Esp/Fra (537,088 B) and Ita/Por (536,576 B) `EGAMES.EXE` give the same results.
- **Same compiler and source for almost everything:** 405 of the 408 accepted functions occur byte-identically
  (relocations masked) in every eGames executable. Changed: WindowProc, ReadConfig, CheckWorld1 (eGames edits; our
  `_EGAMES` guards, inherited from SB2/Planet Blupi, do not reproduce them).
- **Open functions:** DrawButtons, Build, DoInit and TreatEventBase are instruction-identical to the oracle (normalized
  for absolute addresses), so they add no constraint. DoInit allocates `new CDecor` with the **same 0x258d4**: the
  97,532-byte tail is stable across builds eight months apart. ChangePhase differs: it compares WM_PHASE_BYE (0x614) where
  the oracle compares WM_PHASE_DOQUIT (0x634), and one `m_tryInsertCount = 40; m_tryPhase = m_phase;` CD-insert block
  sits earlier. 1948/2029 oracle instructions align; the early zero/WM_PHASE_PLAY register roles are the same as in the
  oracle. It is a second compiled body of ChangePhase with a known small source delta.
- No debug files (pdb/map/dbg) on the disc.

## [OPEN] Limits

The vtable scan only reports consecutive aligned pointers to distinct Ghidra function entries. No such arrays were found, but this does not exclude dynamically initialized or indirect tables outside that pattern. The function kinds beyond the explicitly recognized thunks and CRT names use the inferred linker-order split above.

## Regeneration and verification

Run from the repository root with the supplied `.pyenv` Python, Ghidra 12.1, and the configured JDK:

```powershell
.\.pyenv\python.exe tools\oracle.py all --fresh
```

This clears only `build/oracle/`, extracts the ISO with the configured 7-Zip, writes the full generated reports there, and regenerates the compact CSVs under `oracle/`. Ghidra function bodies and ranges define the boundaries; Capstone then disassembles a deterministic sample of 10 single-range functions and checks their first and last instruction boundaries. Results are saved in `build/oracle/boundary_spotcheck.json`; focused startup and CRT disassembly is in `build/oracle/startup_disassembly.txt`. `.\.pyenv\python.exe tools\oracle.py verify` reruns the 10-boundary check against the existing outputs.
