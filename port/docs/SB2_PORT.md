# Speedy Blupi 2 / Speedy Eggbert 2 in the SDL3 port: plan

Decision (docs/SB2.md, "Structural-recovery experiment: decision", 2026-10-23): the SB2 reconstruction is
STRICT or certified EQUIVALENT for 99.9% (Eggbert 2) and 99.98% (Blupi 2) of game code; the port is now the
priority. This file is the plan; [ARCHITECTURE.md](ARCHITECTURE.md) section 9 already fixes the layout.

## Layout (ARCHITECTURE.md §9)

- `games/sb2/game/`: SB2's port code and its own executable (`blupi2`), linking the same `common/`
  libraries as SB1. No game registry: `common/platform/entry.cpp` calls the game's `BlupiMain`.
- Shared TUs compiled unchanged from `games/sb2/src/` (never edited for the port): the SB2 counterparts of
  SB1's `BLUPI_SHARED_TUS` (`button decblock decblupi decdesign decmove decnet decor dectables event jauge
  menu text`).
- Port-owned replacements of SB2's platform TUs, derived from SB1's port files and from SB2's reconstructed
  versions (`games/sb2/src/blupi.cpp, pixmap.cpp, sound.cpp, misc.cpp, movie.cpp, network.cpp`): SB2's
  `CPixmap` and `CSound` have different members (true-colour decor, benchmark, CD audio, `PlayMusic(HWND,int)`).
  Each file header names its historical origin; copied bodies stay verbatim where possible.
- Code moves into `common/` only when both games build with it. One program per game for all its editions
  and languages (Eggbert 2 = `_EGAMES`; Blupi 2 en/de/fr).
- Reference for behaviour and names: the 2013 C# port (external/mobile-eggbert-legacy, read-only), and the
  STRICT/EQUIVALENT-verified reconstruction itself.

## Milestones and gates

| Milestone | Done when |
|---|---|
| M1 build | `blupi2` builds with the mingw-x64 preset next to `blupi`; SB1's `ctest` suite and golden traces unchanged |
| M2 headless | `blupi2 --headless` reads an SB2 folder (read-only; writes to a `build/` user root), reaches the title, plays the attract demos deterministically; SB2 golden traces and screenshots added; SB1 unchanged |
| M3 playable | edition recognition (Eggbert 2 / Blupi 2 en/de/fr), SB2 language packs from the originals, bounded windowed run, Linux build; README ownership table updated |

Faithful mode first (640x480, original timing and colour depths); enhancements later and optional, as for SB1.

## M1/M2 implementation and evidence (2026-10-23)

| Milestone | Status |
|---|---|
| M1 build | Done: the mingw-x64 Ninja preset builds `blupi` and `blupi2` (supervisor run, 2026-10-23). |
| M2 headless | Done: title and all ten attract demos for Eggbert 2 and Blupi 2, in 8 and 16 bpp; four golden traces and the SB2 disk-layout test. `ctest`: 100/100 (95 SB1, 5 SB2). |
| M3 playable | Open: bounded windowed validation, Linux, packaging, and remaining edition-specific UI/error paths. Resource packs and data-based edition recognition are already implemented. |

[EVIDENCE] `games/sb2/CMakeLists.txt` builds `blupi2` using the twelve unchanged historical
TUs named above, with a separate historical context (no SB1 include paths), the six platform
adapters, SB2 tracing/languages, and the existing common libraries. No code moved out of SB1;
no `common/` behaviour, matching source, manifest, matching tool or acceptance status changed.
The README ownership table lists both games.

[EVIDENCE] SB2 `SetDecor`, `UpdateFrame`, `ReadConfig`'s ordinary settings, `DoInit`'s object
creation order, `BackgroundCache` and `CacheAll` come from `games/sb2/src`. Their platform
services use SB1's proven adapters: renderer commands, VFS, mixer, MIDI notifications,
zeroed object allocation and the posted-message order. Each platform file names its origin.
`CDecor::Build` runs once per logical tick, retaining its original state changes; there is
no SB1 scene builder or extracted `EndOfTick` in this executable.

[EVIDENCE] The SB2 atlas has 336 Blupi, 441 object, 289 element and 100 explosion entries.
The adapter uses those complete tables from the shared `dectables.cpp`. Explicit surrounding
values for historical negative-index reads and mouse-hotspot overreads were read statically
from both originals; only the six values before the Blupi atlas differ by edition, and the
adapter selects them from `PortEdition()`. No source relies on a modern linker's neighbouring data.

[EVIDENCE] `port_switches.h` compiles the Eggbert 2 superset (`_EGAMES=1`) for its layout and
phase table, overrides `REQUIRE_CD=0` after including the historical `def.h`, and leaves all
language code variants at 0 (SB2 German/French change resource strings only). `VERSION_TEXT`
is answered at runtime from the edition's title image: Eggbert 2 "Version 1.0", Blupi 2
"Version 2.2". Four English/German/French resource packs were extracted statically; only
original file sizes/hashes are consulted to choose the installed language. Originals are
never executed. Normal valid-data demo playback uses the same shared game logic for both editions.

### Reproduce and inspect

```powershell
$env:BLUPI_DATA = 'D:\Prog\blupi_recon\assets\Speedy Blupi'
$env:BLUPI2_DATA = 'D:\Prog\blupi_recon\assets\eggbert2'
cmake --preset mingw-x64 -S port -DBLUPI2_TEST_DATA_BLUPI=D:/Prog/blupi_recon/assets/blupi2
cmake --build build/port -j 4
ctest --test-dir build/port --output-on-failure

# Headless, finite, all writes in build/; original data is only read.
build/port/blupi2.exe --headless --data assets/eggbert2 --user build/sb2-user --frames 8000 --color 16 --trace build/sb2.trace --demo-log build/sb2-demos.txt --screenshot 10:build/sb2-title.bmp --screenshot 1500:build/sb2-demo.bmp
```

`BLUPI2_TEST_DATA` is an optional CMake cache path, initially read from `BLUPI2_DATA`.
`BLUPI2_TEST_DATA_BLUPI` gates the second edition's tests separately. Each golden test
compares 800 frame/phase/state/screen checkpoints, checks demo transitions 300 through 309
and the wrap to 300, and replays all 8,011 displays' rendering commands on a second device.
It also requires both benchmark diagnostic files under that run's user root, checking
the write boundary for the two application/pixmap benchmark paths.
`BLUPI2_UPDATE_GOLDEN` updates only SB2 files; SB1's update option was never enabled.
`--state-dump N:FILE` diagnoses individual fields, including SB2's trajectory maps and
additional actor state. No pointer values or raw class padding enter the hash.

[EVIDENCE] Local build workaround, isolated from the preset's output:

```powershell
cmake -S port -B build/port-make -G 'MinGW Makefiles' -DCMAKE_BUILD_TYPE=RelWithDebInfo -DCMAKE_C_COMPILER=C:/msys64/mingw64/bin/gcc.exe -DCMAKE_CXX_COMPILER=C:/msys64/mingw64/bin/g++.exe -DCMAKE_MAKE_PROGRAM=C:/msys64/mingw64/bin/mingw32-make.exe -DCMAKE_PREFIX_PATH=C:/tools/sdl3-3.4.16-mingw/SDL3-3.4.16/x86_64-w64-mingw32 '-DBLUPI_TEST_DATA=D:/Prog/blupi_recon/assets/Speedy Blupi' -DBLUPI_TEST_DATA_EGGBERT=D:/Prog/blupi_recon/assets/eggbert/Game -DBLUPI2_TEST_DATA=D:/Prog/blupi_recon/assets/eggbert2 -DBLUPI2_TEST_DATA_BLUPI=D:/Prog/blupi_recon/assets/blupi2
cmake --build build/port-make -j 4
ctest --test-dir build/port-make --output-on-failure
```

[EVIDENCE] Supervisor gate (outside the Codex sandbox): `cmake --preset mingw-x64 -S port` with Visual
Studio 2022's bundled CMake/Ninja, MSYS2 MinGW GCC and `SDL3_DIR` at the pinned SDL3 builds both games, and
`ctest --test-dir build/port` passes 100/100 with `BLUPI_DATA`, `BLUPI2_DATA` and `BLUPI2_TEST_DATA_BLUPI` set.
The worker's Ninja stall was its sandbox's child-process limit; the Makefile build above is only a fallback.

### Services and limits

- [EVIDENCE] Background/decor colour settings are separate, as in SB2. VFS sends all writes
  (including `time.blp`, `bench.blp`, info and saves) to the selected user root. The data
  folder is read-only from the game's perspective. A missing data root fails startup.
- [EVIDENCE] MIDI uses SB2's integer music IDs (`sound/music%03d.blp`, ID minus one), default
  MIDI volume 10, and the existing common synthesizer. Without a SoundFont it fails and
  reports once; regression runs intentionally load none. CD audio returns failure and logs
  once when requested. It does not silently replace a requested CD track with MIDI.
- [EVIDENCE] AVI playback reports disabled and logs once; the historical movie path continues
  without the film. DirectPlay enumerates zero providers, fails session operations, and logs
  once on enumeration. These are the original unavailable-service failure paths.
- [OPEN] Shared edition code uses Eggbert 2's quit/intro/error flow. Blupi 2's goodbye screen
  and damaged-folder/CD-retry differences are not yet separately represented. CD validation
  itself is deliberately omitted (ARCHITECTURE.md section 9).
- [OPEN] No SB2 historical cross-build pixel comparison, bounded windowed run, Linux/Android
  validation, save/load/editor tour or audible SB2 music validation was performed for M1/M2.
  SB1's historical pixel proofs do not establish SB2 historical pixel equality.
- [OPEN] SB2 does not expose SB1's widescreen, interpolation, Display options or settings
  persistence. Those require SB2-specific scene/coupling work after faithful playback.
