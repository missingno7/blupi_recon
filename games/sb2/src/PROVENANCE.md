# Speedy Blupi 2 reconstructed source: provenance

This tree is verified against `assets/eggbert2/Eggbert2.exe` (`sb2/eggbert/en`, `games/sb2/recon.json`,
[docs/SB2.md](../../../docs/SB2.md)). It started as a copy of the SB1 reconstruction (`src/`, whose
provenance is in [src/PROVENANCE.md](../../../src/PROVENANCE.md): Planet Blupi v1.8.0 and sb2-decomp
material, GPLv3) and is edited only from SB2 binary evidence. sb2-decomp (GPLv3) and Free Eggbert
(GPLv3) are read-only references; where their code was the starting point, it is named below, and every
such body was corrected until STRICT against the SB2 executable. Data tables marked "from the binary"
hold values read from Eggbert2.exe's `.data`, written in the source's usual form. Provenance stays in
this file so source comments do not shift observable `__LINE__` values.

Unchanged copies of SB1 files: `ddutil.cpp`, `jauge.cpp`, `menu.cpp`, `movie.cpp`, `network.cpp`,
`text.cpp`, and the headers other than those listed below.

## Engine and application

- pixmap.h, pixmap.cpp: SB2 members `m_bBenchmarkSuccess`, `m_bTrueColorDecor` (names from sb2-decomp,
  offsets and initial values from the binary); `~CPixmap` → `ReleaseAll` (split from the binary; name a
  hypothesis); `Create` (six arguments), `CacheAll`, `Benchmark`, `BackgroundCache`, true-colour and
  benchmark setters (written from the binary with Free Eggbert's `pixmap.cpp` as a hint); arrow hot
  spot from the binary; the empty inherited `Fill` stub removed (no callers, no SB2 body).
- sound.h, sound.cpp: `m_bCDAudio`, `int m_music` (as sb2-decomp); `PlayMusic(HWND, int)`,
  `PlayCDAudio` (written from the binary, sb2-decomp as the starting point), `SuspendMusic`'s `MCI_STOP`,
  constructor defaults, `SetCDAudio` (sb2-decomp). Lines 1-63 are SB1's; the `TRY_DS` line is [OPEN].
- button.cpp: `Draw`'s enabled/demo condition; SB1's `Redraw` and `GetEnable` definitions removed (no
  callers, no SB2 body; removing them puts `MouseMove` in its context window). misc.cpp: `GetCurrentDir` (`\release\`), `AddCDPath`.
- blupi.cpp: `ReadConfig`, `UpdateFrame`, `WindowProc`, `Benchmark`, `DoInit` from the binary.

## Game logic

- event.h, event.cpp: CEvent layout (`m_menuDecor[20]`, `m_9984`); the phase table `table[]` generated
  from the binary (40 phases); constructor, `CreateButtons`, `ReadInfo`, `MousePosToSprite`,
  `IntroStep`, `LoadImageFromDisc` (sb2-decomp, corrected), `BuildDown`, `ChangeButtons` (settings
  screen, region and dimension handlers), `DrawButtons`, `ChangePhase`, `TreatEventBase` (worker
  reports `scratch/w-event`), mission/demo numbering.
- def.h: `WM_DIMS1..4` values.
- decor.h: CDecor members per sb2-decomp with the corrections in docs/SB2.md section 4; `DescGameFile`
  SB2 fields; declarations of SB2 methods whose bodies exist in the binary and are called by
  reconstructed code (no filler; see docs/SB2.md section 8).
- decblock.cpp, decmove.cpp, decnet.cpp, dectables.cpp, decor.cpp, decdesign.cpp: SB1 bodies with the
  SB2 layout, plus SB2 functions written from the binary with sb2-decomp candidates (reports:
  `scratch/w-decmove`, `scratch/w-decor`, `scratch/w-decdesign`), including SB2 data tables from the
  binary (`table_blitz`, the decor placement tables).

## Speedy Blupi 2 edition

- `#if _EGAMES` blocks (event.h, event.cpp, blupi.cpp, misc.cpp, def.h) from `assets/blupi2/BLUPI.EXE`
  (`sb2/blupi/en`): Blupi 2's phase table (generated from its `.data`), no timed intro, its CD retry
  paths, `ReadConfig` and SB1's `AddCDPath` (reports `scratch/w-b2app`, `scratch/w-b2event`).
