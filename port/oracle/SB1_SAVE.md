# SB1 saved-game interchange capture

`../tools/oracle_save_sb1.ps1` is separate from `oracle_capture.ps1`; existing
capture defaults are unchanged. Only the supervisor invokes historical runs.
Original executable hashes (the complete target catalog and installed files)
are refused, including renamed originals. Both executable paths must be under
the repository's `build/`; the historical executable needs sibling `gate.json`
with `status=PASS` and successful `build.json` for the selected target. Legacy
build reports without `target` are accepted only for the primary US target.
These receipts are recorded and hashed; they are not recomputed matching proofs.

From the repository root, preparation only (neither game launches):

```powershell
powershell -ExecutionPolicy Bypass -File port\tools\oracle_save_sb1.ps1 -BuiltExe build\tmp\relink-us\BLUPI.EXE -PortExe build\port-w-save\port\blupi-sdl3.exe -PrepareOnly
```

Use a current validated source rebuild instead of the example `-BuiltExe` when
available. The worker's private port build is reproducible through
`scratch/w-save/CMakeLists.txt`; after integration use `build\port\blupi-sdl3.exe`.
An already compiled current 32-bit `build/oracle/ddraw.dll` is required. Build
instructions are in `port/docs/ORACLE.md`; preparation refuses a missing/stale
proxy rather than launching another capture to create one.

Supervisor runtime commands (visible historical window; hands off input):

```powershell
powershell -ExecutionPolicy Bypass -File port\tools\oracle_save_sb1.ps1 -BuiltExe build\tmp\relink-us\BLUPI.EXE -PortExe build\port-w-save\port\blupi-sdl3.exe -ColorDepth 16
powershell -ExecutionPolicy Bypass -File port\tools\oracle_save_sb1.ps1 -BuiltExe build\tmp\relink-us\BLUPI.EXE -PortExe build\port-w-save\port\blupi-sdl3.exe -ColorDepth 8
```

Run commands one at a time. There are six child processes per command, all
sequential and bounded to 60 seconds each. Historical capture closes with the
existing proxy's `WM_CLOSE` at the requested display count. Timeout kills/reaps
the child and gives `INCOMPLETE`. No original game, installer or launcher is
copied into the capture. All writes and copied data live under a new
`build/oracle/save-sb1-<edition>-<timestamp>/` directory. Do not reuse the data
folder for normal gameplay.

The same scripts support `-Edition eggbert` with a **matching Eggbert 1 English**
`-BuiltExe` and its PASS/target build reports. No historical Eggbert capture has
been performed by this worker. The wrapper does not test exchanging files
between different editions; it tests each edition's rebuild against its port.

`-PortOnly` rehearses the complete capture/transfer/compare path using six port
processes. Success is labelled `PORT_ONLY_MATCH`, never `MATCH`; it is not a
historical interchange result. `-PrepareOnly -PortOnly` also launches nothing.

## Capture and acceptance

1. `sb1_save.input`: enter player 1's world-1 hub, walk right for ten ticks,
   F3, select slot 0 and press Return. Close after 130 ticks / 141 displays.
   Both builds produce `DATA/s001-000.blp`; header must be `(54416,1,2)`.
2. Compare **all** 54,416 bytes of the independently produced files. Record
   SHA-256, differing byte count and first difference offset; no normalization.
3. Copy each producer's file to two fresh consumers (rebuild and port). Thus
   both directions compare consumers of the **same** source file. Four
   transfer destinations and their verified hashes are in `compare.json`.
4. `sb1_load.input`: start the hub, F4, select slot 0, Return at input frame
   100, then a short right walk, Ctrl jump, and pause. Run 160 ticks / 171
   displays. Compare every display **111..170 inclusive** (60 frames), including
   the load result, through the existing `oracle_diff.py` raw-frame decoder.
   Logs are compared at identical indices; raw equality additionally covers
   8-bit palettes. No timing shifts or ignored mismatches are allowed.

`compare.json` is `MATCH` only with all six successful complete captures,
byte-identical producer saves and both 60/60 restore comparisons. Each pair's
`oracle_diff.txt`, raw frames, display logs and port traces remain reviewable.
For example, rerun diagnostics with:

```powershell
.\.pyenv\python.exe port\tools\oracle_diff.py build\oracle\save-sb1-blupi-<timestamp>\compare-from-hist
```

A MATCH proves this bounded saved-state interchange and visible continuation
against the selected rebuilt executable/configuration. It does not establish
arbitrary mission/state/corrupt-save compatibility, other languages/editions,
RNG/audio/network/UI persistence, original-executable execution, or a historical
STRICT/equivalence result. RNG, pause, UI and notification state are absent from
SB1's save schema. A pixel difference can therefore require investigation even
when bytes round-trip; click/timer races must be reported, not aligned away.

The separate `fidelity_save_sb1_{blupi,eggbert}_{8,16}` tests observe actual
F3/F4 calls at the write/read boundary. They compare every persisted value (85
CDecor members and six gauge properties) individually, as well as their state
hashes, require displacement before load, and check the real user-root save's
size/revision. Preview slot 999 traffic from historical `CEvent::ReadAll` is
observed but not mistaken for the committed save/load. This is local port
evidence, not an independently captured historical saved-game fixture.

## Supervisor results (2026-10-24)

[EVIDENCE] These are historical **rebuild**/port captures, superseding the worker's
preparation-only and port-only rehearsal status for this scenario. All three
receipts record target `sb1/blupi/us`, source HEAD
`874d84cb1d2f919ad9852d13966a835ba1cba65f`, the world-1 hub ten-tick walk,
slot-0 F3/F4 scripts, and both 60-frame restores at display indices 111..170.
The six captures per receipt completed successfully. No frames or save bytes
were normalized, shifted or ignored.

| Directory under `build/oracle/` | Depth | Recorded status | Producer saves | Restores from historical / port save |
|---|---:|---|---|---|
| `save-sb1-blupi-20261024-234212-717815` | 8 | MATCH | 54,416 bytes each; identical | 60/60 / 60/60, raw frames and palettes equal |
| `save-sb1-blupi-20261024-234255-144811` | 16 | MATCH | 54,416 bytes each; identical | 60/60 / 60/60, raw frames and palettes equal |
| `save-sb1-blupi-20261024-234109-984867` | 16 | DIFFERENT | 54,416 bytes each; five differing bytes | 60/60 / 60/60, raw frames and palettes equal |

[EVIDENCE] Executable SHA-256 values recorded by all three receipts:

- Canonical historical relink `build/relink-save/BLUPI.EXE`:
  `557ca92e6ec8c71cfaf5e9f2d55969d3ebcd614c771c9010e074c6ad82ba3533`.
- Port `build/port-w-save/port/blupi-sdl3.exe`:
  `fac927fdeddbd533f64b70d1e2c9de43b9800bf18973ff158ded3c561113b5ca`.
- Both producer saves in each MATCH, and the port save in DIFFERENT:
  `8535489f30e6c40e426fca2c0c1126ef3a0da8927b7a166fa5278b2c3cb8d2f0`.
- Historical producer save in DIFFERENT:
  `8c281eeda8130d997f878238db3b0c22d5da535811fd17a981d5178683ac9756`.

Raw `compare.json` SHA-256 values, in table order:

1. `a56dd8c8169764908a3e43359f412adcb5344fec780028a2c4ddeedcadca13d3`
2. `ee958d531a7538070de829b698ea6aa8ed419970782e6e81bdf530a202f25522`
3. `abf51528141a750c749e51ea4735e74bf1d3ad1334b6d3eab0cd93ec45dcbcd7`

[EVIDENCE] The earlier 16-bpp run remains DIFFERENT. Raw zero-based save offsets
and historical-to-port byte values are: 40252: 29→30; 40292: 167→164;
40298: 8→9; 40300: 29→30; 50044: 29→30. Four are +1. The supervisor attributes
this to the documented historical asynchronous timer/click race shifting the
save by one tick ([ORACLE.md, “Timing race on click frames”](../docs/ORACLE.md)).
Both consumers nevertheless restored each supplied save identically; the fresh
16-bpp rerun matched the independently produced save bytes too. The mismatch
was retained, not converted into a pass by alignment or masking.

[EVIDENCE] F472 is VERIFIED_DERIVATIVE only for SB1 US, this scenario, both
depths, and these executable hashes. The separate four local fixtures also
verify 85 persisted CDecor members and six gauge properties for Blupi US and
Eggbert English, but no historical Eggbert/language interchange is claimed.
The identical US saves support equal serialized values for the seventeen
read-before-write storage fields identified by the initialization audit in this
scenario. They do not prove their pre-construction heap contents or zero defaults
for every allocation; F393/F422 remain UNVERIFIED. Other states, editions,
languages, malformed saves and non-persisted RNG/audio/network/UI state retain
the limits stated above. These are runtime receipts, not matching acceptance.
