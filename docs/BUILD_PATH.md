# Historical build path (`F:\Profil\sound.cpp`)

## [EVIDENCE] The original build path is in the binary

- `BLUPI.EXE` contains exactly one absolute source path: `F:\Profil\sound.cpp`, at file offset `0x79c84`
  (scan of the whole file for `X:\...` strings). It is the `__FILE__` argument of `TRY_DS` in
  `src/sound.cpp` (`TraceErrorDS(rval, __FILE__, __LINE__)`), referenced by `CSound::CreateSoundBuffer`
  (`0x22940`, 206 B).
- MSVC 5.0 SP3 (cl 11.00.7022) expands `__FILE__` to the source path exactly as given on the cl command line.
  Compiled as `F:\Profil\sound.cpp`, the object holds `F:\Profil\sound.cpp\0` with the same neighbouring
  rdata bytes (`...\xff\xff\xff\xffF:\Profil\sound.cpp\0Data1 : %d`) as the executable.
- Control, same source and manifest (2026-10-17):
  - compiled from `D:\Prog\blupi_recon\src\sound.cpp`: sound.cpp 27 STRICT + 1 RELOC-UNVERIFIED
    (`CreateSoundBuffer`, code bytes equal, `__FILE__` payload differs);
  - compiled as `F:\Profil\sound.cpp`: 28 STRICT, `CreateSoundBuffer` STRICT with both relocations proved.
    Accepted; coverage 107,008 -> 107,216 / 141,488 B. `src/sound.cpp` is unchanged.
- Rejected alternative: `#line 1 "F:\\Profil\\sound.cpp"` in the source (sol-leftovers round 3) also matches,
  but the original source did not contain it; it would be a construct whose only purpose is to force bytes.

## Mechanism (tools/buildpath.py, used by tools/msvc.py `--build-path`)

`recon.json` marks the unit: `{"source": "src/sound.cpp", "build_path": "F:\\Profil\\sound.cpp"}`.
`verify.py` and `permute.py` pass it to `msvc.py --build-path`. For that one compile:

1. take the machine-wide lock `build/locks/drive-F.lock` (before a compile slot, so waiting never holds a slot);
2. refuse if `F:` is defined (QueryDosDevice), is a logical drive, or exists as a path - an existing mapping is
   never replaced or removed. The one exception is a stale definition pointing exactly at our own staging
   directory (left by a killed run), which is removed first;
3. copy the source byte-for-byte to `build/histpath/F/Profil/sound.cpp`;
4. `DefineDosDeviceW(DDD_RAW_TARGET_PATH | DDD_NO_BROADCAST_SYSTEM, "F:", "\??\<repo>\build\histpath\F")` -
   logon-session only, not persistent, no broadcast to Explorer; confirm the mapping and the staged bytes;
5. compile `F:\Profil\sound.cpp` (headers still resolve through the usual `/I` list);
6. in `finally` (also on compile errors or Ctrl+C): remove the definition with `DDD_EXACT_MATCH_ON_REMOVE` and our
   exact target, so only our definition can ever be deleted; confirm it is gone; delete the staged copy.

No manual mapping, `subst`, registry or permanent machine configuration is involved.
`.pyenv\python.exe tools\buildpath.py status F:` shows the drive state;
`.pyenv\python.exe tools\buildpath.py selftest F:\Profil\sound.cpp` exercises success, an exception inside the
block, a foreign mapping (refused and left intact) and a stale own mapping (cleaned). All four pass (2026-10-17).

## Fallback: sandboxed workers and an occupied drive

- [EVIDENCE] The Codex restricted-token sandbox may not define DOS devices (`DefineDosDeviceW` ->
  ERROR_ACCESS_DENIED). Nothing is changed in that case.
- If the mapping is denied or `F:` is in use, `msvc.py` prints `buildpath: UNAVAILABLE ...` and compiles from the
  real path (identical code, only `__FILE__` differs). `verify.py` then:
  - prints `NOTE: regression gate INCOMPLETE` naming the unit;
  - checks that unit's accepted functions for identical code bytes only (RELOC-UNVERIFIED tolerated);
  - refuses `--accept`.
- Tested both ways: a simulated foreign `F:` mapping (unsandboxed; the mapping stayed intact) and a real Codex
  sandbox run. Both exit 0 with the NOTE.
- The authoritative gate and every `--accept` run outside the sandbox with `F:` free (the supervisor's runs).
