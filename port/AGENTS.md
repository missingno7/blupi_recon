Scope: everything under `port/` — the native SDL3 port. The repository-wide rules in
`../AGENTS.md` still apply (immutable `assets/` and `external/`, no git commit, host safety,
`build/`/`scratch/` for generated output). This file adds what is specific to the port and
overrides the root file's "no SDL, no modernization" rule *only inside `port/`*.

Boundary with the reconstruction:
- `src/` is the historical reconstruction. Never edit it for the port: not for a modern
  compiler, not for SDL, not for portability. If a shared TU cannot be used as is, add a
  port-side replacement under `port/game/`, name its historical origin in the file header, and
  list it in `port/README.md` (ownership table). Keep copied bodies verbatim where possible so
  they can be diffed against `src/`.
- The port must never change what `tools/verify.py`, `tools/build.py` or `tools/relink.py`
  produce. It has its own build (`port/CMakeLists.txt`, output in `build/port*`).
- Shared TUs (compiled unchanged from `src/`) are listed in `BLUPI_SHARED_TUS` in
  `port/CMakeLists.txt`. Moving a TU from shared to port-owned is an architectural change:
  document why in `port/docs/ARCHITECTURE.md`.

Layering inside `port/`:
- `compat/include` holds declarations only (types, SDK constant values, prototypes) for names the
  shared TUs or the historical-derived bodies in `game/` use (e.g. `MM_MCINOTIFY` for the copied
  window procedure). Do not add a Win32/DirectX name that neither needs. Do not add
  behaviour that silently pretends to work: either implement the semantics the game relies on,
  or fail the way the original did without the service, log it once, and document it.
- Presentation never changes game state. Every state change `Build` made while drawing lives in
  `game/end_of_tick.cpp`, in `Build`'s order. Interpolation and the wide view draw a tick's
  scene into the back buffer or the wide target and never write the game's screen (traces must
  stay golden in every mode: `ctest -R golden`).
- `DrawView` at alpha 1 and 640 wide must issue exactly `FlattenScene`'s commands
  (`--check-view`). Items beyond `Build`'s own set are flagged `extended` and carry `Build`'s
  visibility rule; never draw them in the faithful frame.
- Test-only state injection goes through `--probe` (`game/probe.cpp`), never through gameplay
  code paths.
- Port UI (the Display options screen, `game/options_screen.cpp`) is a presentation layer:
  drawn over the tick's frame when presenting, never into the game's frame or screen. It uses
  its own posted messages, handled before the window procedure. Port settings live in
  `blupi-sdl3.json` (`game/settings.h`) and must never change the simulation.
- Input the original rejects (e.g. editor clicks over the side areas of a wide view) may be
  handled only by a documented port derivative of the historical handler
  (`game/editor_wide.cpp`); input inside the original frame always goes to the game unchanged.
- `game/scene.cpp` is a derivative of `CDecor::Build` and its helpers and must stay
  equivalent (`ctest -R scene`). It reads state only through const accessors; never make it
  write game state.
- `CPixmap` reaches pixels only through `render::Device` commands; validity and colour keys
  are decided in `CPixmap`'s registry, never in a backend (`ctest -R commands` is the gate).
- `render/` and `platform/` must not include compat headers or use Win32 names (key codes in
  `platform/host.cpp` are plain integers documented as VK numbering). Game logic must not
  include SDL headers.
- Faithful mode is the default and the reference: 640x480, original timing (one logical frame
  per `Timer` interval, one `Build` per frame), original colour depths. Enhancements
  (widescreen, interpolation, high refresh rates, new renderers) must be optional and must not
  change simulation results; see the coupling table in `docs/ARCHITECTURE.md`.

Saves and data:
- Keep on-disk struct sizes (static asserts in `tests/test_layout.cpp`). Never write into the
  game data root; all writes go through `platform/vfs` to the user root.

Testing:
- Build: `cmake --preset mingw-x64 -S port`, `cmake --build build/port -j 4` (keep `-j` ≤ 4).
  Linux (WSL): `sh port/tools/linux_check.sh` builds and runs the suite from a case-sensitive
  copy; both platforms must give the same golden traces.
- Android (`port/android`, see its README): build output under `build/port-android`, native
  compiles capped by CMake job pools, one Gradle task at a time. Never bundle game data in the
  APK: players import their own copy. Emulator runs are bounded; stop the emulator afterwards.
- Code shared with Android must build with Clang as well as GCC (`-fsigned-char` and
  `-fms-extensions` stand in for MSVC semantics in the historical context).
- Use `game/member_access.h` to reach protected members of game classes (GCC 15 rejects the
  older same-name pointer idiom).
- `ctest --test-dir build/port` with `SDL3.dll` on PATH. Configure with `BLUPI_DATA` set to the
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
