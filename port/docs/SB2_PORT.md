# Speedy Blupi 2 / Speedy Eggbert 2 in the SDL3 port: plan

Decision (docs/SB2.md, "Structural-recovery experiment: decision", 2026-10-23): the SB2 reconstruction is
STRICT or certified EQUIVALENT for 99.9% (Eggbert 2) and 99.98% (Blupi 2) of game code; the port is now the
priority. This file records the milestones; [ARCHITECTURE.md](ARCHITECTURE.md) section 9 fixes the layout.

## Layout (ARCHITECTURE.md §9)

- `games/sb2/game/`: SB2's port code and its own executable (`blupi2`), linking the same `common/`
  libraries as SB1. No game registry: `common/platform/entry.cpp` calls the game's `BlupiMain`.
- Eleven TUs compiled unchanged from `games/sb2/src/` (never edited for the port): `button decblock
  decblupi decdesign decmove decnet decor dectables jauge menu text`. M1/M2 also shared `event.cpp`;
  M3 derives it in `games/sb2/game/` for runtime edition selection, following SB1's edition principles.
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
| M4a scene split | SB2 const scene plus Build-ordered EndOfTick, historical-command and faithful-view gates, SB1/SB2 goldens unchanged |
| M4b presentation | extend SB1's optional widescreen/interpolation/display-rate/Display options to SB2, without simulation changes |

Faithful mode first (640x480, original timing and colour depths); enhancements later and optional, as for SB1.

## M1/M2 implementation and evidence (2026-10-23)

| Milestone | Status |
|---|---|
| M1 build | Done: the mingw-x64 Ninja preset builds `blupi` and `blupi2` (supervisor run, 2026-10-23). |
| M2 headless | Done: title and all ten attract demos for Eggbert 2 and Blupi 2, in 8 and 16 bpp; four golden traces and the SB2 disk-layout test. `ctest`: 100/100 (95 SB1, 5 SB2). |
| M3 playable | Windows playable: edition flows, mission/pause/quit, save/load, editor, bounded windowed runs and 116/116 tests pass. Linux gate remains for the supervisor because WSL is denied in the sandbox. Resource packs and data-based edition recognition were already present in M2. |

[EVIDENCE] At M1/M2, `games/sb2/CMakeLists.txt` built `blupi2` using twelve unchanged historical
TUs (SB1's counterparts, including `event.cpp`), with a separate historical context (no SB1 include paths), the six platform
adapters, SB2 tracing/languages, and the existing common libraries. No code moved out of SB1;
no `common/` behaviour, matching source, manifest, matching tool or acceptance status changed.
The README ownership table lists both games.

[EVIDENCE] SB2 `SetDecor`, `UpdateFrame`, `ReadConfig`'s ordinary settings, `DoInit`'s object
creation order, `BackgroundCache` and `CacheAll` come from `games/sb2/src`. Their platform
services use SB1's proven adapters: renderer commands, VFS, mixer, MIDI notifications,
zeroed object allocation and the posted-message order. Each platform file names its origin.
At M1-M3, `CDecor::Build` ran once per logical tick, retaining its original state changes.
M4a below replaces that call with SB2's scene and extracted `EndOfTick`.

[EVIDENCE] The SB2 atlas has 336 Blupi, 441 object, 289 element and 100 explosion entries.
The adapter uses those complete tables from the shared `dectables.cpp`. Explicit surrounding
values for historical negative-index reads and mouse-hotspot overreads were read statically
from both originals; only the six values before the Blupi atlas differ by edition, and the
adapter selects them from `PortEdition()`. No source relies on a modern linker's neighbouring data.

[EVIDENCE] `port_switches.h` compiles the Eggbert 2 superset (`_EGAMES=1`) for its header layout,
overrides `REQUIRE_CD=0` after including the historical `def.h`, and leaves all
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
- [EVIDENCE] M3 replaces the shared `event.cpp` with the port derivative described below;
  both editions now use their own phase table and quit/intro/error behavior. CD validation
  itself remains deliberately omitted (ARCHITECTURE.md section 9).
- [OPEN] SB2 historical cross-build pixel comparison, Linux/Android validation, packaging
  and audible SB2 music validation remain outside the headless gates. SB1's historical
  pixel proofs do not establish SB2 historical pixel equality.
- [OPEN] SB2 does not expose SB1's widescreen, interpolation, Display options or settings
  persistence. M4a supplies the scene/coupling split; presentation enhancements are M4b.

## M3 edition flows and playability (2026-10-23)

[EVIDENCE] `game/event.cpp` names its historical origin, `games/sb2/src/event.cpp`.
Its ordinary bodies and both original phase tables remain directly comparable to that file.
The recognized data edition selects the 38-phase Blupi or 40-phase Eggbert table (plus
sentinel); a build-time assertion guards each size. This is the reason `CEvent` became
port-owned; the other eleven shared TUs retain the same headers and compile context.
The title version and optional gamer-screen credit are edition data in `language.cpp`.
Blupi has no `by www.epsitec.com` credit. Behavior the data cannot carry uses documented
runtime edition branches, following SB1's quit branch rather than introducing a game registry.

| Flow | Speedy Blupi 2 | Speedy Eggbert 2 |
|---|---|---|
| Quit from title | BYE is drawn; a key/click dismisses it; DOQUIT closes | BYE closes directly |
| Publisher intro | Both phases absent; timer never runs | Optional 61-tick INTRO1 → INTRO2 → INIT transitions retained |
| Normal startup | Title (`_INTRO=0`) | Title (`_INTRO=0`, as in the reconstruction) |
| Background-cache / official-mission read error | INSERT, retry state and countdown retained | Return to title |
| CD policy | No CD checks; INSERT retries the supplied folder every 41 ticks; Back returns to player selection | No CD checks |

The missing-CD gate is therefore unreachable in normal installed-folder play. Blupi's
retry path is still reachable for damaged/missing mission or background data; it retries
that same folder, without drive detection, until the files are repaired or the player uses
Back. TESTCD is not a startup requirement. Its two historical stores set the same retry
state, so the port uses one order for both editions. Matching sources retain all original
CD checks and edition switches unchanged.

[EVIDENCE] Eight new menu/editor golden traces cover both editions at 8/16 bpp, with
independent rendering-command replay at every display. The tour starts official world 1,
pauses/resumes, returns through the player and title menus, and exercises each quit flow.
The editor tour opens and scrolls a private mission, saves it under the user root, enters
playtest and returns to the editor. Pause checks moving objects and the pause flag:
the historical `Build` still advances its rendering counter `m_time` while paused.

[EVIDENCE] Two save tests use F3/F4 and an actual 57,008-byte `data/s001-000.blp` in the
user root. A short manual walk changes the position; loading restores it and the checked
actor, world, trajectory, moving-object, life and treasure fields. Four failure tests use
empty user-root overrides (world 1 or the help image), checking both recovery paths and
Blupi's timer reset/Back button. Two intro tests use `--probe` (test-only phase injection,
as in SB1): Eggbert advances the phases, Blupi rejects them. Since the shipped folder has
no intro images, those tests copy title art into the test user root as explicit fixtures.
They make no claim about the publisher screens' historical pixels.

[EVIDENCE] The two Blupi attract traces change only in state hashes because its table's
`CEvent::m_index` is two lower. Frame, phase and screen hashes stay equal at all 800
checkpoints in each depth. Eggbert attract and every SB1 golden file remain unchanged.
The audit and reason are in [ORACLE.md](ORACLE.md), with local evidence under
`build/w-portm3/` and the worker report in `scratch/w-portm3/REPORT.md`.

### M3 platform gates

- [EVIDENCE] The mingw-x64 preset configures successfully. Ninja stalls after its glob
  recheck in this sandbox; it was interrupted and left no running build processes.
  The documented `build/port-make` fallback builds both games with `-j 4`.
- [EVIDENCE] `ctest --test-dir build/port-make --output-on-failure` passes **116/116**
  in 211 seconds (95 SB1, 21 SB2), both golden-update options OFF, with all three requested
  data roots configured. Log: `build/w-portm3/ctest.log`.
- [EVIDENCE] Both editions complete 100-frame windowed runs at 16 bpp, navigate the title
  and player menus into world 1, replay their rendering commands without mismatch, and
  exit with code 0. Each process has a separate 20-second outer timeout and was closed.
  Screenshots/logs: `build/w-portm3/windowed-{eggbert,blupi}/`. Blupi `--language de`
  and `--language fr` also reach world 1 in separate 90-frame headless checks.
- [EVIDENCE] WSL enumeration is rejected with
  `Wsl/EnumerateDistros/Service/E_ACCESSDENIED`. Linux execution is unavailable here;
  directly launching the check also returns `Wsl/Service/CreateInstance/E_ACCESSDENIED`.
  The supervisor must run `sh port/tools/linux_check.sh`. The script now copies SB2's
  historical source and both optional SB2 data folders to the case-sensitive workspace,
  and enables their tests (also Eggbert 1 when its data is present).
- [OPEN] After the Linux gate: packaging/Android, audible music and a historical
  SB2 pixel-oracle comparison. Scene extraction and end-of-tick separation are implemented
  in M4a below. Widescreen, interpolation, display-rate presentation and Display options
  remain M4b, with their own equivalence gates. Faithful mode stays the default.


## M4a scene and end-of-tick split (2026-10-23)

[EVIDENCE] This extends SB1's `scene.cpp` / `end_of_tick.cpp` and main-loop seam.
SB2 `game/scene.cpp` keeps SB2's `CDecor::Build` draw order and helper bodies from
`games/sb2/src/{decblock,decor}.cpp`, with const member-access bindings and a recorder.
It includes SB2's rear tiles 384/385, big-decor offsets, shield loop, extra object passes,
trap/drop/crusher/saw/timer/lightning animations, keys/dynamite HUD and delayed voyage 40.
The scene stores world/entity/culling metadata in the same form as SB1.

`EndOfTick` applies the following effects once, where Build ran, in this order:

1. Camera shake phase/finish (stopped by SB2 pause), retaining the pre-increment shaken view.
2. Blupi's secondary effect flags when behind the decor.
3. Visible overlay cells, x major / y minor: drop icon stores and lightning sound/network effects.
4. Visible gauge redraw flags, then the mission-10 tutorial position clamp.
5. Voyage 40's two RNG draws and particle `ObjectStart`, including network effects.
6. `m_time` increment and wrap at 32768, including while paused.

The normal path builds a scene, applies EndOfTick and flattens the stored scene, then
executes the historical buttons/cursor once per tick. Drawing never consumes RNG or
spawns objects. `--reference` uses the unchanged shared Build; `--check-scene` builds the
const scene before it and compares every rendering-device command, also checking state
purity. That check uses historical state updates, so `tests/sb2_scene.cmake` separately
compares native scene+EndOfTick with the reference every frame (phase, full state hash,
screen hash). `--check-view` compares faithful DrawView commands with FlattenScene, both
with and without motion metadata at alpha 1, and checks presentation purity.

[EVIDENCE] `common/scene/commands.h` is now built by both games. Only command storage,
layer/coverage values, primitive replay and faithful list traversal moved there, with
point/rectangle/flag types, channel/gauge constants and text callbacks passed by each
adapter. Entity identities, visibility, scene builders, views and mutations stay game-owned.
SB1's algorithms and historical include context are unchanged; no historical TU moved.

The sixteen `scene_sb2_*` tests cover both editions and depths: all ten attract demos,
menu/pause/quit tour, editor/playtest, and `tests/sb2_scene.probe`. The explicit test fixture
covers all overlay animation families, rear tiles, big-decor offsets, mover passes and
cull boundaries, network actors, effects/masking, HUD keys/dynamite, gauge redraw modes,
shake/pause/finish, clock wrap, tutorial clamps and both voyage-40 intervals. It uses
only `--probe` state injection, following SB1; no gameplay paths inject test state.
Goldens are compared as checked in, with both update options OFF.

[EVIDENCE] M4a Windows gates: the mingw-x64 preset configures; Ninja stalls at the
sandbox glob recheck and is stopped. The documented `build/port-make` fallback builds
both games with `-j 4`. Serial `ctest --test-dir build/port-make --output-on-failure`
passes **132/132** in **395.31 seconds** (95 SB1, 37 SB2), with SB1, Eggbert 1, Eggbert 2
and Blupi 2 data roots configured and both golden-update options OFF. All sixteen new
scene/state/view/command comparisons pass, alongside all existing goldens. Each attract
reference checks 7,009 Build calls and 8,011 device displays; each effects reference checks
90 Build calls and 171 displays, with zero mismatches. Log: `build/w-portm4a/ctest.log`;
per-scenario logs/traces: `build/port-make/tests/scene_sb2_*/`.

[EVIDENCE] `git diff --check` passes; matching sources (`src/`, `games/sb2/src/`), tools,
manifests and `port/tests/golden/` are unchanged. No original executable was run, no data
was written, no commit was made, and no game/build process remained after the gates.
Worker report: `scratch/w-portm4a/REPORT.md`.

[OPEN] M4b must extend coverage/culling to the wider view (only Build's own set may be
faithful), pair scenes and interpolate visual positions, capture the tick's UI and
present stored frames at display rate, then add SB1's Display options/settings flow.
M4a accepts only zero Coverage and DrawView at alpha 1, 640 wide; other requests fail
explicitly. No M4b option is exposed. Keep simulation camera, sound pan, activation,
pause semantics, RNG/particle spawns and all EndOfTick effects at the original tick rate.
Add SB2's wide/interpolation/presentation gates and retain every current golden trace.
Linux and Ninja supervisor reruns remain required in this sandbox; historical SB2 pixel
comparison and Android/packaging/audible music validation remain separate work.
