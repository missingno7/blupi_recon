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
`--build-dir NAME` to clean and write a private `build/verify-NAME/` for
parallel candidate runs; promotion always uses the canonical `build/verify/`.
Use `--accept` after a clean build to add newly STRICT or FOLDED functions to
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
  the location can be unique by bytes, placed by TU order or verified references, or
  anchored by an accepted `--at` assertion;
  each relocation has one run-wide symbol binding; imports resolve to their
  IAT slots; calls to reconstructed functions land at their unique matched
  RVA; other calls land at an exact start in `oracle/functions.csv`; and
  reconstructed data lands in its expected PE section with identical
  initialized bytes. An explicitly mapped external data symbol must land in
  one oracle data section; its table contents are not claimed as reconstructed.
- **FOLDED** means the candidate body matches exactly, an object from another
  TU independently emits that same body, and verified function references from
  one or more placed caller objects land on the shared oracle body. A map entry
  or raw oracle reference alone cannot establish folding. Calls bind to the owner
  address. The manifest records the owner symbol/RVA; folded code is reported
  separately, excluded from the originating TU's owned-byte count, and counted
  once under its owner.
- **RELOC-UNVERIFIED** means code bytes match, but a target could not be proven
  or a documented source-path string differs. The report includes the reason.
- **MISMATCH** means code bytes or a proven relocation target/data payload
  differs, or a forced accepted RVA no longer matches.

The comparator reports each location as `unique`, `placed by TU order`,
`placed by verified references`, `forced by --at`, or unresolved. For
identical-byte hits it uses the nearest unique locations in the same object's
function order. Same-TU object order takes precedence over a stale TU-map
boundary. A TU boundary and one unique neighbour can prove a placement
when the function extent and 16-byte `/O2` alignment fit the observed gap,
allowing intervening unlocated function sections to have been dead-stripped;
otherwise the function stays ambiguous. A uniquely located caller can also
place an identical-byte candidate when its code bytes and every other
relocation are STRICT, and its direct call or function-address relocation lands
on that candidate at an oracle function start. Only selected locations contribute
relocation bindings. A conflict between selected bindings is a mismatch. The
JSON and console report adjacent object functions whose oracle gap is not just
16-byte alignment padding.

The report gives function code size separately from trailing `/O2` alignment
padding. Padding is not counted as function body coverage. Folded functions do
not generate TU-order layout warnings.

Compiler-generated UI switch labels that cannot be derived from their function
section remain explicit `symbol_mappings` in `recon.json`. Ordinary function
local labels resolve from section-relative offsets. For VC5 exception handlers,
the verifier also compares companion `.text$x` funclet bytes and checks their
relocations plus linked `.xdata$x` bytes and relocations. The absolute
`__except_list` symbol is checked as `FS:0`. These checks preserve relocation
fields for separate target validation; they do not treat masked bytes as a
match.

The manifest's `__pgmptr` proof clean-compiles `STDARGV.C` and `CRT0DAT.C` from
the pinned VC5 tree as evidence-only objects. It binds the symbol only when the
uniquely located CRT reference and the four-byte zero-filled definition resolve
to one writable oracle address; these objects do not count as reconstructed
TUs. Explicit mappings for external data are also constrained to one oracle
`.rdata`/`.data` section. A one-past table relocation is strict only when it
lands exactly at the end of the named initialized object and the object's bytes
match in full.

Call placement needs an actual direct relocation in a STRICT caller. If no
built object references an ambiguous method, the verifier leaves it unresolved
even when the body bytes are identical; for example, `CEvent::SetDemo` remains
excluded because the current object set has no direct incoming relocation.

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

## Integrated engine and event TUs

The manifest compiles the SB2-derived game candidates with per-TU `SB1_GAME_API`; `event.cpp` also uses `SB1_EVENT_API` for its measured event declarations, phase/resource values, and overloads. These defines keep the event/game declarations out of unrelated accepted TUs. The event header uses `Phase::backName[32]`, removes the three SB2-only `CEvent` fields before `m_oldMousePos`, and forward-declares `CNetwork` so the original header order can be retained. Evidence and per-file deltas are recorded in [src/PROVENANCE.md](../src/PROVENANCE.md).

`forced_locations` supplies evidence-backed `--at` comparisons for identical bodies whose TU placement is established; all code bytes and relocations still pass the comparator. `symbol_mappings` names compiler-generated local switch destinations only where their target addresses are statically established. `promotion_exclusions` keeps ambiguous event aliases and duplicate `ObjectDelete` output from becoming extra accepted functions. The latter body is owned by `decnet.cpp` at `0x108a0`; it is counted once even though `decmove.cpp` also emits an identical candidate.
