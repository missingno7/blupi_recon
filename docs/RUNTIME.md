# Rebuilt-game cnc-ddraw smoke test (2026-10-19)

[EVIDENCE] The user authorized continuing with cnc-ddraw for game launches. Only
the rebuilt executable was run. Original assets and executables were not modified;
the original executable and ISO programs were not executed.

## Reproduce

After producing a passing `build/standalone-validation` relink:

```powershell
.\.pyenv\python.exe tools/relink.py --out-dir build/standalone-validation --historical-host
powershell -NoProfile -ExecutionPolicy Bypass -File tools/runtime_smoke.ps1 -Seconds 6 -Mission
```

The script copies data and the existing cnc-ddraw wrapper to a new ignored
`build/runtime-<timestamp>` directory. It uses a 640x480 window, GDI renderer,
60 FPS cap and unlocked mouse. It records module paths, hashes, window response,
exit status and a screenshot, then closes the process (terminating it if needed).
It selects 32-bit PowerShell to enumerate the game's 32-bit DLLs correctly.
Use no switch for title-only, `-Play` for player selection, `-Mission` for level
loading. Script success checks process/wrapper/exit health; inspect the screenshot
to establish which game screen was reached. No full gameplay assertion is implied.

The harness now selects `TrueColor=16` explicitly; use `-ColorDepth 8` to test
8-bit mode. The original configuration's `Benchmark=310185` implicitly enables
16-bit color, while `TrueColor=0` does not override it. Disabling the benchmark
without setting TrueColor caused the initial smoke test to use 8-bit color.
This was a harness configuration difference, not a reconstruction defect.

For manual play, enter the generated runtime directory and run `./BLUPI.EXE -nocd`.
Its copied `DATA/config.def` accepts `TrueColor=8` or `TrueColor=16`; restart after
changing the setting. The smoke harness closes its own process, manual launches
remain open until the player quits.

## Observed result

[EVIDENCE] `build/runtime-20261019-230611/smoke.json` and `startup.png`:

- Rebuilt EXE SHA-256: `557ca92e6ec8c71cfaf5e9f2d55969d3ebcd614c771c9010e074c6ad82ba3533`.
- Loaded local cnc-ddraw 7.1.0.0 (`git~541b5de`), SHA-256
  `85e0f7d530dfda134793a57cb3e76b0287dcc96892ee57162dd68f47283b03a9`.
- Title screen rendered; play click reached Choose player; single-player click
  loaded the first level, showing Blupi, terrain, doors and lives.
- Normal window close, exit code 0, no forced termination or surviving game process.
- Canonical C/C++ sources and strict/equivalence acceptance were unchanged.

[EVIDENCE] After manual testing, the user reported that the game looks good and
requested wrapping up this milestone. This is qualitative user acceptance, not
a recorded exhaustive gameplay or original-versus-rebuilt comparison.

[EVIDENCE] An initial sandbox launch failed with Windows DLL relocation error
623; the bounded test ran outside that sandbox. An early mission attempt showed
the CD prompt because the copied configuration pointed at the wrong directory.
`AddCDPath` resolves shared `data/`, images and movies one directory above the CD
language folder. The harness now creates an `ENGLISH` child and points `CD-Rom`
there. It uses the game's existing `-nocd` switch and disables the copied benchmark
setting. This fixes the test layout without patching the game.

[OPEN] Movement, jumping, combat, level completion, save/load round trips, audio
output, multiplayer and long sessions are not validated by this smoke test.
The original was not run for comparison. Static EQUIVALENT certificates remain
conditional local proofs; this smoke test does not strengthen them into a claim
of full-game behavioral identity.
