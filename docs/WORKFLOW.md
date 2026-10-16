# Matching source workflow

`recon.json` is the canonical input list and acceptance record. It pins the VC5
SP3 compiler tree, `/O2 /GX` release flags, DirectX 6 headers, and the
`nullptr=0` source compatibility define. `src/` keeps original filenames and
source line numbers; see [src/PROVENANCE.md](../src/PROVENANCE.md) for upstream
and license details.

From the repository root, build and verify every listed TU:

```powershell
.\.pyenv\python.exe tools\verify.py
```

The verifier removes and recreates only `build/verify/`, sends `TMP` and `TEMP`
to the repository's `build/tmp/`, compiles each TU with the manifest settings,
and compares all objects in one run. It writes objects, logs, and JSON details
under `build/verify/`, prints compiler component SHA-256 hashes once, and
reports per-TU counts and strict game `.text` coverage. A nonzero exit means a
previously accepted function failed its status, RVA, or code-size check.

To check a candidate without changing `src/`, substitute one TU for that run:

```powershell
.\.pyenv\python.exe tools\verify.py --tu src/movie.cpp=scratch\tu\movie.cpp
```

The canonical source remains the manifest's default for all other TUs. Use
`--accept` after a clean build to add newly STRICT functions to `recon.json`:

```powershell
.\.pyenv\python.exe tools\verify.py --accept
```

Promotion is refused if any existing accepted function regresses, and a
substituted candidate cannot be promoted.

## Comparator statuses

- **STRICT** means one unique code location matches every non-relocation byte;
  each relocation has one run-wide symbol binding; imports resolve to their
  IAT slots; calls to reconstructed functions land at their unique matched
  RVA; other calls land at an exact start in `oracle/functions.csv`; and
  referenced data lands in the expected PE section with identical initialized
  bytes.
- **RELOC-UNVERIFIED** means code bytes match, but a target could not be proven
  or a documented source-path string differs. The report includes the reason.
- **MISMATCH** means code bytes or a proven relocation target/data payload
  differs, or a forced accepted RVA no longer matches.

The report gives function code size separately from trailing `/O2` alignment
padding. Padding is not counted as function body coverage.

The current known data exception is `__FILE__`: compiling
`src/sound.cpp` produces `D:\Prog\blupi_recon\src\sound.cpp`, while the oracle
contains `F:\Profil\sound.cpp` at RVA `0x7b484`. `cmpfunc.py` reports this as a
`build-path data difference` and leaves affected functions unverified; it does
not mask the literal bytes. Other string literals and initialized data must
match byte-for-byte.
