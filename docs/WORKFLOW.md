# Matching source workflow

`recon.json` is the canonical input list and acceptance record. It pins the VC5
SP3 compiler tree, `/O2 /GX /Gy` flags, DirectX 6 headers, and the `nullptr=0`
source compatibility define. `src/` keeps original filenames and source line
numbers; see [src/PROVENANCE.md](../src/PROVENANCE.md) for file provenance.

`/Gy` packages functions as COMDATs; the original link also folded identical
COMDATs (linker ICF, commonly `/OPT:ICF`). Comparing the eight earlier TUs with
and without `/Gy` found identical function body bytes (128 functions). A
separate comparison of the new pixmap TU found all 45 function bodies
byte-identical with and without `/Gy`. The oracle's shared `OutputDebug` body
demonstrates link-time folding.

From the repository root, build and verify every listed TU:

```powershell
.\.pyenv\python.exe tools\verify.py
```

The verifier removes and recreates only `build/verify/`, sends `TMP` and `TEMP`
to the repository's `build/tmp/`, compiles each TU with the manifest settings,
and compares all objects in one run. It writes objects, logs, and JSON details
under `build/verify/`, prints compiler component SHA-256 hashes once, and
reports per-TU counts, location proofs, and strict/folded game `.text` coverage. A
nonzero exit means a previously accepted function failed its status, RVA, or
code-size check.

To check a candidate without changing `src/`, substitute one TU for that run:

```powershell
.\.pyenv\python.exe tools\verify.py --tu src/movie.cpp=scratch\tu\movie.cpp
```

The canonical source remains the manifest's default for all other TUs. Use
`--accept` after a clean build to add newly STRICT or FOLDED functions to
`recon.json`:

```powershell
.\.pyenv\python.exe tools\verify.py --accept
```

Promotion is refused if any existing accepted function regresses, and a
substituted candidate cannot be promoted. Newly verified folded bodies are
recorded with `status: "folded"` and an `into` owner; ordinary accepted bodies
remain `STRICT`.

## Comparator statuses

- **STRICT** means one proved code location matches every non-relocation byte;
  the location can be unique by bytes, placed by TU order or verified calls, or
  anchored by an accepted `--at` assertion;
  each relocation has one run-wide symbol binding; imports resolve to their
  IAT slots; calls to reconstructed functions land at their unique matched
  RVA; other calls land at an exact start in `oracle/functions.csv`; and
  referenced data lands in the expected PE section with identical initialized
  bytes.
- **FOLDED** means the candidate body matches exactly, an object from another
  TU independently emits that same body, and verified call relocations from at
  least two placed caller objects land on the shared oracle body. A map entry or
  raw oracle reference alone cannot establish folding. Calls bind to the owner
  address. The manifest records the owner symbol/RVA; folded code is reported
  separately, excluded from the originating TU's owned-byte count, and counted
  once under its owner.
- **RELOC-UNVERIFIED** means code bytes match, but a target could not be proven
  or a documented source-path string differs. The report includes the reason.
- **MISMATCH** means code bytes or a proven relocation target/data payload
  differs, or a forced accepted RVA no longer matches.

The comparator reports each location as `unique`, `placed by TU order`,
`placed by verified call targets`, `forced by --at`, or unresolved. For
identical-byte hits it uses the nearest unique locations in the same object's
function order. Same-TU object order takes precedence over a stale TU-map
boundary. A TU boundary and one unique neighbour can prove a placement
when the function extent and 16-byte `/O2` alignment fit the observed gap,
allowing intervening unlocated function sections to have been dead-stripped;
otherwise the function stays ambiguous. A helper can also be placed when
multiple uniquely located callers target the same candidate, which is an
oracle function start.
Only selected locations contribute relocation bindings. A conflict between
selected bindings is a mismatch. The JSON and console report adjacent object
functions whose oracle gap is not just 16-byte alignment padding.

The report gives function code size separately from trailing `/O2` alignment
padding. Padding is not counted as function body coverage. Folded functions do
not generate TU-order layout warnings.

The UI cluster's compiler-generated switch labels are listed as explicit
`symbol_mappings` in `recon.json`; their addresses were checked against static
oracle disassembly and are described in [docs/UI_CLUSTER.md](UI_CLUSTER.md).

The current known data exception is `__FILE__`: `msvc.py` passes an absolute
source path, producing `D:\Prog\blupi_recon\src\sound.cpp`, while the oracle
contains `F:\Profil\sound.cpp` at RVA `0x7b484`. A VC5 `/E` probe with the
relative command-line path `src/sound.cpp` produced the literal
`src/sound.cpp`; it does not recreate the original drive path. Reproducing
`F:\Profil\sound.cpp` would require that original source path or an equivalent
path directive. `cmpfunc.py` reports the difference and leaves affected
functions unverified; it does not mask the literal. Other string literals and
initialized data must match byte-for-byte.

VC5 `.bss` globals are checked against the oracle's writable `.data` zero-fill
tail. The final file-alignment unit is accepted only when its bytes are all
zero, since PE file padding can precede that zero-fill range.
