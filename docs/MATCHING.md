# Historical compile and function comparison

## Reproduction

`tools/msvc.py` compiles one source file to an i386 COFF object. It builds an isolated environment for `PATH`, `INCLUDE`, `LIB`, `TMP`, and `TEMP`; temporary files go to `build/tmp`. The compiler tree is selected with `--tree` (`sp3` by default; also `sp2`, `rtm`, `nt5-1592`, `nt5-1691`, `sp3-deu`, `sp3-jpn`, and `vc6`) or with an explicit path. The standalone NT5 compiler aliases use the SP3 VC headers unless `--vc-root` is supplied.

Custom `--include` directories are passed as `/I` arguments ahead of the compiler `INCLUDE`, so the selected DirectX SDK headers win over older DirectX headers bundled with VC. The environment still includes the selected compiler's Windows headers and libraries. Each compile log under `build/match/logs/` records the exact banner, environment paths, command, compiler output, and exit code.

The SP3 banner recorded for the experiment was:

```text
Microsoft (R) 32-bit C/C++ Optimizing Compiler Version 11.00.7022 for 80x86
```

Extract the root-level sources from the read-only Planet Blupi tag with this byte-preserving archive step (run from the repository root):

```powershell
@'
import io
import pathlib
import subprocess
import tarfile

root = pathlib.Path.cwd().resolve()
out = root / "build" / "match" / "pb_src"
out.mkdir(parents=True, exist_ok=True)
archive = subprocess.run(
    ["git", "-c", "safe.directory=D:/Prog/blupi_recon/external/planetblupi",
     "-C", "external/planetblupi", "archive", "v1.8.0"],
    stdout=subprocess.PIPE, check=True,
).stdout
with tarfile.open(fileobj=io.BytesIO(archive), mode="r:") as tar:
    tar.extractall(out)
'@ | .\.pyenv\python.exe -
```

Example compile command for a Planet Blupi TU:

```powershell
.\.pyenv\python.exe tools\msvc.py build\match\pb_src\movie.cpp `
  --tree sp3 --out build\match\obj\pb\movie_o2_gy_gx.obj `
  --include build\match\pb_src --include C:\tools\dx6sdk\INCLUDE `
  --define nullptr=0 --flag /O2 --flag /Gy --flag /GX
```

`nullptr=0` is the only PB source compatibility shim used. Run the complete eight-way `/O1` or `/O2`, `/Gy` on or off, `/GX` on or off matrix with:

```powershell
.\.pyenv\python.exe scratch\match\run_matrix.py pb
.\.pyenv\python.exe scratch\match\run_matrix.py sb2
```

The PB run uses `C:\tools\dx6sdk\INCLUDE`. The SB2 run uses `external\sb2-decomp\bass\inc` and `C:\tools\dx6sdk\INCLUDE`; its `dxsdk3` directory is empty in this checkout. The matrix helper records each compile in `scratch/match/{pb,sb2}_compile_matrix.tsv` and retains failed compiler output in the matching `_compile_errors.txt` file.

Compare all successfully built PB objects and retain per-function details as JSON:

```powershell
$pbObjects = Get-ChildItem build\match\obj\pb -Filter '*.obj' -File |
  Select-Object -ExpandProperty FullName
& .\.pyenv\python.exe tools\cmpfunc.py @pbObjects --summary `
  --json-out build\match\results\pb_all.json
```

Use the same command with `build\match\obj\sb2` and an `sb2_all.json` output for SB2. Omit `--summary` to print each function's locations and Capstone disassembly pairs for mismatches. `--at '?DecoratedName@@...=0xRVA'` compares at a specified RVA; `--symbol-va SYMBOL=0xVA` supplies an independent mapping for an external symbol.

## Comparator status meanings

- **STRICT** — bytes outside COFF relocation fields are identical at one unique location. Supported imports resolve to the expected IAT slot; calls to uniquely located functions resolve to that function; repeated external symbol references agree on one absolute target; defined TU data symbols map consistently.
- **RELOC-UNVERIFIED** — the non-relocated bytes match, but the location is ambiguous or at least one relocation target lacks enough evidence. A wildcard hit alone is never strict.
- **MISMATCH** — no location matches all non-relocated bytes, or a forced RVA fails the byte comparison. The report includes a nearest alignment and Capstone disassembly when available.

`raw_exact_hit_count` is stricter than a wildcard hit: it counts a candidate where even the relocation fields match the object's original bytes. File-level `wildcard` counts functions with one or more candidate locations, including ambiguous ones. File-level `strict` counts functions with exactly one candidate and verified relocations. The COFF reader handles section-definition auxiliaries, function symbols, `/Gy` sections, and i386 relocations; the executable is read as data and never run.

## Planet Blupi v1.8.0 results

**[EVIDENCE]** The SP3 matrix compiled 168 of 216 source/flag combinations. Six files (`blupi.cpp`, `button.cpp`, `decor.cpp`, `event.cpp`, `jauge.cpp`, `menu.cpp`) fail under every tested flag combination with duplicate `Term`, `CSound`, and `CPixmap` declarations. In the imported tree, the source includes lowercase header spellings and `decor.h` includes uppercase spellings of the same files; VC5 processes the same declarations twice. No PB source or header was changed to work around this.

The table gives the best single flag variant per PB file, selected by strict count, wildcard count, then raw-exact count. “Raw exact” is the subset of candidate functions whose full bytes, including relocation fields, are identical.

| PB source | Functions | Wildcard functions | Raw exact | Strict | Reloc-unverified |
|---|---:|---:|---:|---:|---:|
| action.cpp | 4 | 0 | 0 | 0 | 0 |
| arrange.cpp | 14 | 0 | 0 | 0 | 0 |
| chemin.cpp | 7 | 0 | 0 | 0 | 0 |
| ddutil.cpp | 9 | 8 | 0 | 2 | 6 |
| decblupi.cpp | 57 | 0 | 0 | 0 | 0 |
| decgoal.cpp | 1 | 0 | 0 | 0 | 0 |
| decio.cpp | 4 | 0 | 0 | 0 | 0 |
| decmap.cpp | 6 | 0 | 0 | 0 | 0 |
| decmove.cpp | 17 | 0 | 0 | 0 | 0 |
| decstat.cpp | 11 | 0 | 0 | 0 | 0 |
| fifo.cpp | 4 | 0 | 0 | 0 | 0 |
| fog.cpp | 3 | 0 | 0 | 0 | 0 |
| misc.cpp | 12 | 9 | 2 | 2 | 7 |
| movie.cpp | 13 | 13 | 2 | 7 | 6 |
| obstacle.cpp | 45 | 0 | 0 | 0 | 0 |
| pixmap.cpp | 45 | 4 | 3 | 1 | 3 |
| sound.cpp | 27 | 23 | 10 | 12 | 11 |
| tablefloor.cpp | 0 | 0 | 0 | 0 | 0 |
| tableobj.cpp | 0 | 0 | 0 | 0 | 0 |
| text.cpp | 11 | 2 | 0 | 0 | 2 |
| wave.cpp | 3 | 0 | 0 | 0 | 0 |

The strict per-function locations found in the best variants are:

| Function | Source file | Oracle RVA | Size | Status | Flags |
|---|---|---:|---:|---|---|
| _DDCopyBitmap | ddutil.cpp | 0x03340 | 512 | STRICT | /O2 /Gy /GX |
| _DDColorMatch | ddutil.cpp | 0x037a0 | 256 | STRICT | /O2 /Gy /GX |
| ?LoadStringA@@YAXIPADH@Z | misc.cpp | 0x1f1f0 | 32 | STRICT | /O2 /Gy /GX |
| ?ConvLongToPos@@YA?AUtagPOINT@@J@Z | misc.cpp | 0x1f330 | 32 | STRICT | /O2 /Gy /GX |
| ?initAVI@CMovie@@IAEHXZ | movie.cpp | 0x20150 | 80 | STRICT | /O2 /Gy /GX |
| ?termAVI@CMovie@@IAEXXZ | movie.cpp | 0x201a0 | 33 | STRICT | /O1 /Gy /GX |
| ?positionMovie@CMovie@@IAEXPAXUtagRECT@@@Z | movie.cpp | 0x201d0 | 48 | STRICT | /O2 /Gy /GX |
| ?fileCloseMovie@CMovie@@IAEXPAX@Z | movie.cpp | 0x20200 | 80 | STRICT | /O2 /Gy /GX |
| ?playMovie@CMovie@@IAEXPAXH@Z | movie.cpp | 0x203c0 | 128 | STRICT | /O2 /Gy /GX |
| ??0CMovie@@QAE@XZ | movie.cpp | 0x20440 | 16 | STRICT | /O1 /Gy /GX |
| ?GetEnable@CMovie@@QAEHXZ | movie.cpp | 0x20490 | 3 | STRICT | /O1 /Gy /GX |
| ?IsPalette@CPixmap@@QAEHXZ | pixmap.cpp | 0x1efc0 | 4 | STRICT | /O1 /Gy /GX |
| ?InitMidiVolume@@YAXH@Z | sound.cpp | 0x22c30 | 128 | STRICT | /O2 /Gy /GX |
| ??0CSound@@QAE@XZ | sound.cpp | 0x22cb0 | 96 | STRICT | /O2 /Gy /GX |
| ?GetEnable@CSound@@QAEHXZ | sound.cpp | 0x22dc0 | 4 | STRICT | /O1 /Gy /GX |
| ?SetState@CSound@@QAEXH@Z | sound.cpp | 0x22dd0 | 10 | STRICT | /O1 /Gy /GX |
| ?SetAudioVolume@CSound@@QAEXH@Z | sound.cpp | 0x22de0 | 13 | STRICT | /O1 /Gy /GX |
| ?GetAudioVolume@CSound@@QAEHXZ | sound.cpp | 0x22df0 | 32 | STRICT | /O2 /Gy /GX |
| ?SetMidiVolume@CSound@@QAEXH@Z | sound.cpp | 0x22e10 | 13 | STRICT | /O1 /Gy /GX |
| ?GetMidiVolume@CSound@@QAEHXZ | sound.cpp | 0x22e20 | 32 | STRICT | /O2 /Gy /GX |
| ?Flush@CSound@@QAEXH@Z | sound.cpp | 0x22ec0 | 64 | STRICT | /O2 /Gy /GX |
| ?SuspendMusic@CSound@@QAEXXZ | sound.cpp | 0x23540 | 80 | STRICT | /O2 /Gy /GX |
| ?IsPlayingMusic@CSound@@QAEHXZ | sound.cpp | 0x235b0 | 16 | STRICT | /O2 /Gy /GX |
| ?SetSuspendSkip@CSound@@QAEXH@Z | sound.cpp | 0x23600 | 13 | STRICT | /O1 /Gy /GX |

## SB2 reconstruction results

**[EVIDENCE]** With the explicit `bass\inc` and DX6 include paths, 176 of 184 source/flag combinations compiled. `decio.cpp` fails in all variants with C2086 (`i` redefinition at source lines 429 and 493). The `dxsdk3` directory is empty, so DX6 headers were used. The table reports the best variant per successfully built source file, selected by strict count, wildcard count, then raw-exact count; SB2's `obstacle.cpp` and `dectables.cpp` objects contain no function symbols.

| SB2 source | Functions | Wildcard functions | Strict |
|---|---:|---:|---:|
| blupi.cpp | 27 | 7 | 2 |
| button.cpp | 34 | 14 | 10 |
| ddutil.cpp | 8 | 3 | 0 |
| decblock.cpp | 44 | 0 | 0 |
| decblupi.cpp | 33 | 1 | 0 |
| decdesign.cpp | 31 | 0 | 0 |
| decmove.cpp | 59 | 0 | 0 |
| decnet.cpp | 31 | 3 | 0 |
| decor.cpp | 79 | 2 | 2 |
| dectables.cpp | 0 | 0 | 0 |
| event.cpp | 109 | 36 | 15 |
| jauge.cpp | 25 | 9 | 8 |
| menu.cpp | 23 | 8 | 4 |
| misc.cpp | 26 | 9 | 2 |
| movie.cpp | 27 | 13 | 7 |
| network.cpp | 35 | 0 | 0 |
| obstacle.cpp | 0 | 0 | 0 |
| pixmap.cpp | 66 | 7 | 4 |
| sound_bass.cpp | 44 | 6 | 3 |
| sound_dx.cpp | 14 | 0 | 0 |
| text.cpp | 23 | 4 | 1 |
| wave.cpp | 3 | 0 | 0 |

SB2 finds strict matches in additional code that PB could not compile from its mixed-case headers, including `?GetNum@@YAHPAD@Z` at RVA `0x1000` (48 bytes) and several `CButton`, `CEvent`, `CJauge`, and `CMenu` functions. Some byte-identical short functions from different source names map to the same RVA; these are address-level code matches and should not be counted as separate unique code bodies.

## Mismatch analysis

**[EVIDENCE]** `/O2` produced more hits than `/O1` in the larger matched files: for example, `movie.cpp` goes from 4 to 13 wildcard functions (3 to 7 strict), and `sound.cpp` from 5 to 23 wildcard functions (5 to 12 strict). Accessors can favor `/O1`; several 3–13 byte getters/setters are strict only under `/O1`. In the tested matrix, toggling `/Gy` or `/GX` did not change per-file match counts.

**[EVIDENCE]** On the same `misc.cpp` TU and `/O2 /Gy /GX` flags, SP2, RTM, SP3, and NT5-1592 produced identical function bytes for all 12 functions. Their banners were 11.00.7022 (SP2/RTM/SP3) and 11.00.7071 (NT5-1592). VC6 reported 12.00.8168 and changed 6 of the 11 common function byte sequences; the file fell from 9 wildcard functions to 5. This demonstrates toolchain sensitivity while keeping the tested VC5 service-pack choice from explaining the `misc.cpp` near misses.

**[EVIDENCE]** `ChangeSprite` (320 bytes, 33 relocation fields) and `Random` (32 bytes) have no wildcard hit under the SP3 `/O1` or `/O2` variants, or under the SP2, RTM, NT5-1592, and VC6 checks. A forced comparison of `ChangeSprite` at RVA `0x1f210`, immediately after the matched `LoadStringA` helper, shows different first instructions and branch/register use in Capstone. The RVA is an adjacent-function inference, not a proven function boundary.

**[HYPOTHESIS]** The matched utility, movie, and sound functions are shared code from the Planet Blupi lineage. The persistent `ChangeSprite` and `Random` differences are more likely to come from Speedy Blupi source/TU differences than from the tested VC5 update level. Compiler flags still matter for other functions, so a source difference is not established solely by a failed byte search.

**[OPEN]** The experiment compiles each TU independently and has not reconstructed the original full project build context or validated all TU data targets against their final linked payload bytes. The six PB case-variant header failures and the SB2 `decio.cpp` C2086 failure also leave those functions unmeasured. The next useful step is to identify the original Speedy Blupi source or reconstruct the affected source/header context, then test matched functions inside that complete context.
