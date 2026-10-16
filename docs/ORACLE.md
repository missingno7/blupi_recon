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

## [HYPOTHESIS] CRT boundary

The game/CRT split is inferred at RVA `0x24140`, where Ghidra first recognizes `__global_unwind2`; following functions include `__local_unwind2`, `__abnormal_termination`, `__NLG_Notify1`, `_strncpy`, `__exit`, `__amsg_exit`, `__aulldiv`, `__aullrem`, and `_abort`. Kinds are assigned by function entry relative to this split, except Ghidra-marked thunks. The split is a useful linker-order boundary, not proof that every unnamed routine has a CRT source identity.

## [EVIDENCE] Vtables and strings

No RTTI name markers (`.?AV`, `.?AU`, `.?AT`, `.?AW`) were found, and no distinct-target function-pointer run qualified as a vtable. The two exact `.data` pointer runs repeat one function each: RVA `0x7bd30` has two pointers to `FUN_004278d0`; RVA `0x7e580` has six pointers to `FUN_0042b2d0`. They are listed in `build/oracle/pointer_runs.json`, while `oracle/vtables.csv` has no confirmed entries.

The PE resource tree has 31 `RT_STRING` blocks with 329 nonempty neutral-language strings. Block IDs are `7–19`, `26`, `32–33`, `38–39`, `63–67`, `126–128`, `130`, `188`, `194–195`, and `201`; their data spans RVA `0x826e8`–`0x85b82`. Each string's ID, RVA, and text is in `build/oracle/resource_strings.csv`. Printable string extraction found 146 strings in `.rdata` (RVA `0x2c349`–`0x2d366`) and 313 in `.data` (RVA `0x2e040`–`0x7b510`), including game asset paths and CRT/import text; see `build/oracle/strings.csv`.

## [OPEN] Limits

The vtable scan only reports consecutive aligned pointers to distinct Ghidra function entries. No such arrays were found, but this does not exclude dynamically initialized or indirect tables outside that pattern. The function kinds beyond the explicitly recognized thunks and CRT names use the inferred linker-order split above.

## Regeneration and verification

Run from the repository root with the supplied `.pyenv` Python, Ghidra 12.1, and the configured JDK:

```powershell
.\.pyenv\python.exe tools\oracle.py all --fresh
```

This clears only `build/oracle/`, extracts the ISO with the configured 7-Zip, writes the full generated reports there, and regenerates the compact CSVs under `oracle/`. Ghidra function bodies and ranges define the boundaries; Capstone then disassembles a deterministic sample of 10 single-range functions and checks their first and last instruction boundaries. Results are saved in `build/oracle/boundary_spotcheck.json`; focused startup and CRT disassembly is in `build/oracle/startup_disassembly.txt`. `.\.pyenv\python.exe tools\oracle.py verify` reruns the 10-boundary check against the existing outputs.
