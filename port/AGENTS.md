Scope: everything under `port/` — the native SDL3 port. The repository-wide rules in
`../AGENTS.md` still apply (immutable `assets/` and `external/`, no git commit, host safety,
`build/`/`scratch/` for generated output). This file adds what is specific to the port and
overrides the root file's "no SDL, no modernization" rule *only inside `port/`*.

Boundary with the reconstruction:
- `src/` is the historical reconstruction. Never edit it for the port: not for a modern
  compiler, not for SDL, not for portability. If a shared TU cannot be used as is, add a
  port-side replacement under `games/sb1/game/`, name its historical origin in the file header, and
  list it in `port/README.md` (ownership table). Keep copied bodies verbatim where possible so
  they can be diffed against `src/`. The one kind of `src/` change that serves the port is the form of
  a target switch (root AGENTS.md, Targets), and only when every target's verifier report and the
  relinked image stay identical.
- The port must never change what `tools/verify.py`, `tools/build.py` or `tools/relink.py`
  produce. It has its own build (`port/CMakeLists.txt`, output in `build/port*`).
- Shared TUs (compiled unchanged from `src/`) are listed in `BLUPI_SHARED_TUS` in
  `port/CMakeLists.txt`. Moving a TU from shared to port-owned is an architectural change:
  document why in `port/docs/ARCHITECTURE.md`.

Layering inside `port/`:
- `common/compat/include` holds declarations only (types, SDK constant values, prototypes) for names the
  shared TUs or the historical-derived bodies in `games/sb1/game/` use (e.g. `MM_MCINOTIFY` for the copied
  window procedure). Do not add a Win32/DirectX name that neither needs. Do not add
  behaviour that silently pretends to work: either implement the semantics the game relies on,
  or fail the way the original did without the service, log it once, and document it.
- Presentation never changes game state. Every state change `Build` made while drawing lives in
  `games/sb1/game/end_of_tick.cpp`, in `Build`'s order. Interpolation and the wide view draw a tick's
  scene into the back buffer or the wide target and never write the game's screen (traces must
  stay golden in every mode: `ctest -R golden`).
- `DrawView` at alpha 1 and 640 wide must issue exactly `FlattenScene`'s commands
  (`--check-view`). Items beyond `Build`'s own set are flagged `extended` and carry `Build`'s
  visibility rule; never draw them in the faithful frame.
- Test-only state injection goes through `--probe` (`games/sb1/game/probe.cpp`), never through gameplay
  code paths.
- Port UI (the Display options screen, `games/sb1/game/options_screen.cpp`) is a presentation layer:
  drawn over the tick's frame when presenting, never into the game's frame or screen. It uses
  its own posted messages, handled before the window procedure. Port settings live in
  `blupi-sdl3.json` (`games/sb1/game/settings.h`) and must never change the simulation.
- Input the original cannot map (the wide editor's world, which no longer lies under its
  toolbar) may be handled only by a documented port derivative of the historical handler
  (`games/sb1/game/editor_wide.cpp`, routed by `PortEditorMouse`). Everywhere else, input in UI
  coordinates goes to the game unchanged.
- `games/sb1/game/scene.cpp` is a derivative of `CDecor::Build` and its helpers and must stay
  equivalent (`ctest -R scene`). It reads state only through const accessors; never make it
  write game state.
- `CPixmap` reaches pixels only through `render::Device` commands; validity and colour keys
  are decided in `CPixmap`'s registry, never in a backend (`ctest -R commands` is the gate).
- `common/render/` and `common/platform/` must not include compat headers or use Win32 names (key codes in
  `common/platform/host.cpp` are plain integers documented as VK numbering). Game logic must not
  include SDL headers.
- Faithful mode is the default and the reference: 640x480, original timing (one logical frame
  per `Timer` interval, one `Build` per frame), original colour depths. Enhancements
  (widescreen, interpolation, high refresh rates, new renderers) must be optional and must not
  change simulation results; see the coupling table in `docs/ARCHITECTURE.md`.

Games and languages (docs/MULTI_GAME.md, docs/LANGUAGES.md):
- `common/` holds what does not depend on a game: `compat/`, `platform/`, `render/`. `games/sb1/game/` holds
  Speedy Blupi 1's (and Speedy Eggbert 1's) port code. Both are include roots, so includes stay
  `"platform/host.h"`, `"game/port_game.h"`. Move code into `common/` only when a second game builds with it.
- Languages are packs (`languages/*.txt`), generated from the original executables by
  `tools/language_pack.py` and embedded at build time. The US pack must equal `src/res/blupi.rc`
  (`ctest -R language`); the built-in packs keep the original's bytes.
- The game's build switches (`src/def.h`) get their port values in `games/sb1/game/port_switches.h`, a
  second forced include after the game-independent `common/compat/include/port_forced.h`. `LANGUAGE_DE`
  (the language's code variant) and `VERSION_TEXT` (the edition's title label) are run-time calls; the
  matching source uses them only in expressions (root AGENTS.md, Targets). Express an edition or language
  difference there, as data from the original, or as a language-pack field; add a port-side edition
  branch only for behaviour the data cannot carry (today one: Eggbert's quit, `main.cpp`).
- The edition is recognised from the data, the installed language from the installed executable's size
  and hash; the port uses nothing else of an original executable and never runs one.

Saves and data:
- Keep on-disk struct sizes (static asserts in `tests/test_layout.cpp`). Never write into the
  game data root; all writes go through `common/platform/vfs` to the user root.

Testing:
- Build: `cmake --preset mingw-x64 -S port`, `cmake --build build/port -j 4` (keep `-j` ≤ 4).
  Linux (WSL): `sh port/tools/linux_check.sh` builds and runs the suite from a case-sensitive
  copy; both platforms must give the same golden traces.
- Android (`port/android`, see its README): build output under `build/port-android`, native
  compiles capped by CMake job pools, one Gradle task at a time. Never bundle game data in the
  APK: players import their own copy. Emulator runs are bounded; stop the emulator afterwards.
- Code shared with Android must build with Clang as well as GCC (`-fsigned-char` and
  `-fms-extensions` stand in for MSVC semantics in the historical context).
- Use `games/sb1/game/member_access.h` to reach protected members of game classes (GCC 15 rejects the
  older same-name pointer idiom).
- `ctest --test-dir build/port` (on Windows the build copies `SDL3.dll` next to the tests). Configure with `BLUPI_DATA` set to the
  game folder to enable the demo-replay determinism and music tests (the music tests use the
  GeneralUser GS submodule in `third_party/generaluser-gs/` by default; initialise it with
  `git submodule update --init port/third_party/generaluser-gs`).
- `third_party/` holds unmodified upstream material, with provenance in
  `third_party/README.md`: source URL and commit, licence, SHA-256. Small headers are vendored.
  Large assets come in as shallow, pinned git submodules, never as committed binaries. Never edit
  inside them; move the pin instead and update the README.
- Gameplay regression uses the game's own demo system (`DEMO2xx.BLP`, attract mode) and
  `--headless --trace/--screenshot`. Do not build bots, route planners or AI players.
  `--input` scripts are for menu navigation and small smoke checks only.
- Golden traces (`tests/golden/*.trace`, `ctest -R golden`) are the regression gate for every
  change in faithful mode.
  - Regenerate them (`-DBLUPI_UPDATE_GOLDEN=ON`) only for an intended, explained change, and
    record the reason (`docs/ORACLE.md` lists the ones made so far).
  - Diagnose a state difference with `--state-dump N:file` on both builds.
- Renderer or behaviour changes that could alter pixels are checked against the historical
  rebuild with `port/tools/oracle_capture.ps1` (a visible window opens; follow the root runtime
  rules). Record new results in `docs/ORACLE.md`.
- Windowed runs of the port follow the root runtime rule: bounded (`--frames N`), process closed,
  data read from the original folder (read-only), writes to a `build/` user directory.

Releases (`docs/RELEASING.md`):
- The version lives only in `VERSION.txt` (not `VERSION`: on a case-insensitive drive that
  name shadows the C++ `<version>` header through the `port/` include path).
- Every package is built and fully tested before it is packed: Windows with the `mingw-x64`
  preset, Linux with `tools/linux_release.sh` in the WSL `Ubuntu-22.04` distro (glibc 2.35;
  the default distro's newer glibc would raise the minimum), Android with `assembleRelease`.
  Packages go to `build/release/` and are never committed.
- The Android release key and its properties file stay in `%USERPROFILE%\.android\`, outside
  the repository; never print, copy or commit their contents.
- `res/icon.png` and `res/icon.ico` change only through `tools/make_icon.py`.
- Packages carry `release/COPYING.txt` (GPLv3, inherited from the reconstructed source) and
  the third-party notices; game data is never packaged.
