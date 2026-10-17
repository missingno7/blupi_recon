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

`src/sound.cpp` carries `"build_path": "F:\\Profil\\sound.cpp"` in `recon.json`: its `__FILE__` literal is
the original developer's path. `msvc.py --build-path` maps `F:` temporarily for that one compile and always
removes the mapping again. It never touches an existing `F:`. Where the mapping is impossible (Codex sandbox, or
`F:` in use), the verifier prints `NOTE: regression gate INCOMPLETE`, checks that TU's accepted functions for
identical code bytes only, and refuses `--accept`. See [BUILD_PATH.md](BUILD_PATH.md).

## Difference-first matching loop

Work from where the bytes differ, not from blind runs ([COMPILER_STUDY.md](COMPILER_STUDY.md)):

```powershell
.\.pyenv\python.exe tools\residues.py                    # all unmatched functions, classified differing regions
.\.pyenv\python.exe tools\residues.py --detail 0x159f0    # every region of one function, candidate vs oracle
.\.pyenv\python.exe tools\fnprobe.py src/decor.cpp=scratch\me\decor.cpp FileExist 0x159f0 --region 0x50:0xd0
```

`residues.py` reads the last canonical `verify.py` run. `fnprobe.py` compiles one TU in about a second and diffs
one function (relocations masked, so `differing bytes=0` is a lead, not a strict match). Name the cause of each
region (register / order / constant / missing code), form one hypothesis, test it with `fnprobe`, then confirm with
`verify.py --tu`.

## Comparator statuses

- **STRICT** means one proved code location matches every non-relocation byte;
  the location can be unique by bytes, placed by TU order or verified references, or
  anchored by an accepted `--at` assertion;
  each relocation has one run-wide symbol binding; `__imp_` imports resolve to
  their IAT slots, while plain import calls may resolve through a `.text`
  linker thunk only when its bytes are exactly `ff 25` plus that same import's
  IAT VA; calls to reconstructed functions land at their unique matched
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

When a data symbol has one definition in the run but no reference has pinned
its address yet, `cmpfunc.py` can bind it by payload only if its initialized
bytes occur exactly once at natural alignment in the corresponding oracle
section. The complete payload is still checked at that address. Count-prefixed
numeric `table_` arrays use their declared element count to exclude only COFF
alignment padding from the data extent. This lets ordinary initialized source
tables prove relocations without hardcoded VA mappings; the result JSON records
payload-based bindings.

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

The event header uses `Phase::backName[32]`, removes the three SB2-only `CEvent` fields before `m_oldMousePos`, and forward-declares `CNetwork` so the original header order can be retained. Evidence and per-file deltas are recorded in [src/PROVENANCE.md](../src/PROVENANCE.md).

### Build-context debt — resolved

**[EVIDENCE]** The shared header set from `scratch/hdr/src/` is integrated in
`src/`: the six formerly gated declarations are unconditional, all per-TU
`SB1_GAME_API`/`SB1_EVENT_API` defines are removed from `recon.json`, and no
`#ifdef SB1_*` block remains in a source header. The four candidate headers
differed from the prior canonical headers only by removing those guards, so
the existing member declaration order is retained. All 20 translation units
now compile with the same shared class declarations; their ordinary include
sets can still differ.

**[EVIDENCE]** The clean no-switch build has 317 STRICT and 3 FOLDED functions,
covering 42,016 unique game `.text` bytes. `CButton::MouseMove` at `0x2fd0`
remains STRICT. The only prior accepted function that does not match is
`CDecor::GetBlupiInfo` at `0x57b0` in `decblock.cpp`; it remains in
`recon.json` with a `withdrawn` reason, evidence link, and date rather than
being silently deleted.

**[EVIDENCE]** The top-of-file and before-definition count probes for
`GetBlupiInfo` restored STRICT output only with 24–27 or 32 scratch `extern`
declarations. End-of-file counts did not restore it. These probes point to
missing front-end context before the function; the original declarations or
preceding incomplete `decblock.cpp` bodies are not yet identified. Do not
turn the count probes into source declarations. See
[the header experiment](HEADERS.md#whole-tree-and-isolation-experiments).

The verifier retains withdrawn entries in `accepted_functions`, reports their
current candidate status separately, and excludes them from the failure gate.
If a later clean build makes a withdrawn candidate STRICT or FOLDED,
`verify.py --accept` reactivates its record and preserves the withdrawal
metadata in `withdrawal_history`.

`forced_locations` supplies evidence-backed `--at` comparisons for identical bodies whose TU placement is established; all code bytes and relocations still pass the comparator. `symbol_mappings` names compiler-generated local switch destinations only where their target addresses are statically established. `promotion_exclusions` keeps ambiguous event aliases and duplicate `ObjectDelete` output from becoming extra accepted functions. The latter body is owned by `decnet.cpp` at `0x108a0`; it is counted once even though `decmove.cpp` also emits an identical candidate.

## Register residues: tracing c2.exe (tools/c2trace)

When a function differs only in scratch registers (eax/ecx/edx), trace the compiler's own temp-register assignment
instead of guessing. `tools/c2trace/c2trace.py build-stub` once, then `capture NAME src/X.cpp[=cand.cpp]`,
`assigns NAME SYMBOL` (each temp's path: preference / rotor / first free) and `shift NAME SYMBOL RVA`: it moves c2's
rotor pointer by one or two slots at each assignment and reports the differing bytes. A `MATCH` at assignment K means
the original source had one rotor step more or less just before K; `map NAME SYMBOL K` shows the instruction. The
debugger only reads and pokes c2.exe's memory in its own run, never files; rotor shifts are diagnostics, never
source. Model and addresses: docs/COMPILER_STUDY.md, round 4.
