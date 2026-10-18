# Resources (.rsrc)

`src/res/blupi.rc` plus the cursor/icon files in `src/res/` rebuild the oracle's `.rsrc` section. `tools/relink.py`
compiles the script with the VC5 resource compiler (`C:\tools\msvc5_sp3\SHAREDIDE\BIN\RC.EXE`, include path VC + MFC);
LINK converts the `.res` with CVTRES 5.00.1668. `tools/make_rc.py` regenerates the script.

## Evidence

- [EVIDENCE] The oracle's Rich header has one object stamped `0x60684` (product 6 = CVTRES 5.00, build 1668). The installed
  `C:\tools\msvc5_sp3\VC\BIN\CVTRES.EXE` is file version 5.00.1668.1, so the original link used VC5's own resource
  converter. Our relink now carries the same Rich entry.
- [EVIDENCE] All 14 RT_CURSOR payloads (DIB data and hotspots) and the RT_ICON payload are byte-identical to Planet Blupi's
  original 1997 files (`external/planetblupi` commit b0270ab: cursor1.cur, map.cur, arrow*.cur, wait.cur, empty.cur,
  fill.cur, arrow.cur, blupi.ico). Planet Blupi's cursor statement order reproduces the oracle's cursor IDs 2-15 and the
  named groups (IDC_POINTER ... IDC_ARROW, IDR_MAINFRAME). These files are original Epsitec assets (GPLv3 in that repo).
- [EVIDENCE] Languages: every resource is LANG_NEUTRAL except dialog 111, which is French (Switzerland) (0x100c). The script
  therefore has a neutral section and a French-Swiss section for the dialog, unlike Planet Blupi's all-French script.
- [EVIDENCE] String table: 329 strings in 31 blocks, reconstructed as script text from the oracle's RT_STRING data. RC emits
  string blocks in order of first appearance; the oracle's data order (7-10, 188, 194, 195, 201, 11-14, 63, 64, 126, 128,
  130, 127, 65, 66, 26, 15, 32, 33, 16, 17, 67, 38, 39, 18, 19) records the original script's block order, and the
  generator follows it.
- [EVIDENCE] Version block: Planet Blupi's script structure with the oracle's values (0.4.0.0, "EPSITEC Speedy Blupi",
  "Copyright (C) 1997 Epsitec SA", translation 0x100c/1200).
- [HYPOTHESIS] Symbol names: dialog 111 and the string IDs are written numerically; the original symbolic names are unknown
  (several are in src/resource.h). Equivalent spellings give identical output.

## Result

Relink with resources (2026-10-18): `.rsrc` virtual size 0x5b88 equals the oracle's. After normalizing the resource
data-entry RVAs (the section currently sits 0xC000 lower because `.data` is still 48,964 bytes short), all 23,432 bytes
are identical.
