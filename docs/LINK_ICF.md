# sol-icf: LINK 5.10 survivor selection (2026-10-18)

[EVIDENCE] **PARTIAL. The survivor rule is recovered and predicts every traced
sort entry. Historical CRT sorting fixes the unwind-filter owner in a diagnostic
link. No genuine source/input correction reproducing all oracle choices is
established, and no game-source patch is proposed.** The remaining getter must
survive in the sound block, not the pixmap block stated in the task.

## Rule and addresses

[EVIDENCE] Addresses below are VAs in the unmodified SP3 LINK.EXE, image base
`0x400000` (hash in `provenance.json`). Ghidra 12.1 project:
`build/sol-icf/ghidra/LinkICF.gpr`; exports: `decomp/`.

1. `0x4604e4` calls eligibility predicate `0x431f30`. Accepted code
   **contributions**, including anonymous EH code and library `.text`, are
   appended at `0x46058b` to a flat vector: pointer `0x494398`, count
   `0x49439c`, capacity `0x4943a0`. This is not a symbol hash-table walk.
   `0x462f10` obtains a contribution's supplemental record; its `+0x20`
   points back to the contribution. Initial order follows object/contribution
   loading, including library extraction. The traced vector has 793 entries.
2. ICF entry `0x431fb0` calls imported **MSVCRT qsort** at `0x431fed`
   through IAT slot `0x401140`, on four-byte vector elements. Comparator
   `0x432130` uses ascending **effective raw size**, `con[+8]-con[+0x14]`,
   then ascending relocation count, word `con[+0x24]` (zero for the special
   linker module at global `0x47f028`). Null records/contributions sort last.
   It returns zero for equal keys: **no name, checksum, address, or module
   tie-breaker**. Names/total symbol population are not the selection key.
3. `0x43202a` scans the sorted size/relocation groups. `0x432970` visits
   each not-yet-folded candidate in that order and compares later candidates.
   For an equivalent later body it sets `later.con[+4] |= 0x4000` and
   `later.con[+0x10] = first.con`. **The first equivalent eligible contribution
   in qsort's resulting order survives.** There is no later owner preference.
   `0x432a90`/`0x432530` establish content/relocation equivalence; hashes are
   comparison filters, not sort keys. Further ICF iterations retain this order.

[EVIDENCE] The imported qsort's unstable equal-key permutation matters.
The pinned linker actually loads `C:\Windows\SysWOW64\msvcrt.dll`, version
`7.0.26100.9711`, qsort RVA `0x5da40` (observed VA `0x75d0da40`).
`predict.py` models the historical `VC/CRT/SRC/QSORT.C`: middle pivot moved
to the low element, cutoff eight, selection-sort small partitions. The host
model calls Windows' x64 CRT qsort; its complete permutation equals the x86
debuggee's. `check_models.py` checks **all 793/790 record identities**, not
just the interesting functions. All eight controls pass (`model-checks.json`).

[EVIDENCE] Candidates later discarded by reference elimination still affect
the permutation. Removing three dead SB2 bodies changes historical survivors
despite preserving the live code inputs. Thus a count of final/live functions
or the map's public symbols cannot reconstruct the original ICF vector.

## Predictions versus observed selection

[EVIDENCE] Zero-based indices in the sorted 793-entry vector:

| Family | Modern CRT order | Historical CRT order | Observed survivor |
|---|---|---|---|
| Empty destructors / OutputDebug | misc 142; jauge 174; button 190 | misc 168; button 191; jauge 207 | misc with both |
| Four `mov eax,[ecx+4]; ret` getters | network 143; pixmap Back 175; Decor 176; sound 183 | network 141; pixmap Back 149; Decor 164; sound 166 | network with both |
| Unwind filters | frame 278; ehvecdtr 299 | ehvecdtr 275; frame 287 | frame modern; ehvecdtr historical |

[EVIDENCE] The real historical DLL produces **exactly** the modelled full
permutation. Separate original VC5 RTM, SP2, SP3 and VC4 CRT DLLs all produce
the historical result. Files were copied unchanged into ignored scratch
storage; their hashes and loaded addresses are recorded in the traces and
`build/sol-icf/*/summary.json`. SP3 qsort is `0x7802543d` when its unchanged
DLL is loaded at `0x78000000`.

[EVIDENCE] These historical-runtime runs are **debugger diagnostics**:
`link_trace.py --legacy-crt` loads an unchanged historical DLL under a distinct
basename and dispatches the ICF qsort call to its exported function with LINK's
actual vector/comparator. It changes only running-tool memory/stack/context;
it does not patch any tool, object, source, output, or original-game file.
Other linker qsort calls still use the host CRT. This is not a production
historical-host reproduction. A copied LINK beside MSVCRT.DLL, both with and
without `LINK.EXE.local`, still loaded the system CRT (negative controls).

## Oracle getter: genuine network function; sound physical owner

[EVIDENCE] `06_network.obj`'s four-byte getter is
`?GetNbProviders@CNetwork@@QAEHXZ`, not a GetEnable definition. Its full
16-byte section is `8b 41 04 c3` followed by twelve NOPs, identical to the
two pixmap getters and sound's GetEnable. Its map aliases can misleadingly
appear under `06_network.obj` with other class names.

[EVIDENCE] Oracle `CEvent::ChangePhase` calls `CNetwork::EnumProviders`
at RVA `0x1a36d -> 0x20a30`, reloads the same network receiver from
`[ebp+0x6d60]`, then calls **`0x1a378 -> 0x22dc0`** and stores the result
as the provider count at `[ebp+0x6d04]`. The reconstructed call sequence
independently names GetNbProviders. This is a genuine SB1 getter, folded in
the oracle, and cannot be removed as an SB2-only function. No verifier
acceptance/promotion is claimed for this currently unmatched caller.

[EVIDENCE] `oracle/tu_map.csv` and STRICT adjacent bodies put `0x22dc0`
inside **sound.cpp**: CSound::Create `0x22d60`, getter `0x22dc0`,
CSound::SetState `0x22dd0`. `docs/RELINK.md`'s round-2 getter discussion
also identifies CSound::GetEnable as the independent owner. Under the
oracle's ordinary TU order the desired physical survivor is sound's copy.
The task's description of this slot as the pixmap block is incorrect.

## Genuine input test and unmasked measurements

[EVIDENCE] Working HEAD was `45d770c`, rather than the supplied `bc81fef`.
The requested first reuse of `build/verify/objects` gave 29,726 same-offset
equal `.text` bytes and entry `0x24cf0`. Its data inputs differ from the fresh
canonical build. Subsequent controls all use the same private canonical
objects (`build/verify-sol-icf/objects`); their hashes are in `provenance.json`.
Do not attribute the old/rebuilt baseline difference to ICF.

[EVIDENCE] One source-supported population correction was tested:
remove only PlaceDecor8/9/10 definitions from decdesign.cpp, retaining headers.
They contain SB2-only key/door/hovercraft/goal options, have no incoming source
calls or assigned oracle bodies, and are documented as SB2-only in
`docs/SENIOR_SOL_DECDESIGN_2.md`. Candidate: `decdesign-no-sb2.cpp`.
The gate has zero accepted regressions; all 42 remaining decdesign bodies
are STRICT. Eligible-vector count changes 793 -> 790.

| Inputs / ICF sorting | ret survivor RVA / owner | getter survivor RVA / owner | unwind survivor RVA / owner | CRT entry | `.text` same-offset equal bytes |
|---|---|---|---|---|---|
| Oracle | `0x1f010` misc | `0x22dc0` sound block | `0x243f0` ehvecdtr | `0x24d40` | 176,019 |
| Canonical / host CRT | `0x1f1c0` misc | `0x20a60` network | `0x27360` frame | `0x24cf0` | 30,187 |
| Canonical / historical diagnostic | `0x1f1c0` misc | `0x20a60` network | `0x243c0` ehvecdtr | `0x24d10` | 30,262 |
| Remove SB2 stubs / host CRT | `0x1f1c0` misc | `0x20a60` network | `0x27360` frame | `0x24cf0` | 30,187 |
| Remove SB2 stubs / historical diagnostic | `0x1f1d0` misc | `0x21520` pixmap Decor | `0x27360` frame | `0x24cf0` | 28,273 |

[EVIDENCE] All candidate images remain `.text` virtual-size **32 bytes short**.
The historical canonical diagnostic places both ehvecdtr's filter and the
entry at the oracle's constant **-0x30 game-code displacement**. It removes
the extra -0x20 entry displacement caused by keeping frame's filter.
These are raw, unmasked linked-byte counts, not strict image matches.

[EVIDENCE, negative] SB2-stub cleanup is not the ICF solution and is not
proposed for integration here. Model-only restoration of Build's proven
3872-byte oracle span, and model-only restoration of the two oracle callback
positions, leave the three canonical historical predictions unchanged.
Coalescing the two pixmap accessors into one also fails the historical
prediction; no original single-accessor declaration was established, and
no source variant was compiled or linked for that hypothesis.
No arbitrary filler, renamed symbol, guessed deletion, or object-order
permutation was linked to tune survivors.

## Verification, reproduction, remaining work

[EVIDENCE] Four serial whole-tree verification runs used the manifest SP3
flags, canonical headers, no PCH, default jobs/slots, tools/msvc.py, and
repo-local TMP/TEMP: canonical, negative candidate, candidate in the explicitly
requested `sol-icf` build directory, then canonical restoration. All preserve
every active accepted function; sound has the expected historical-path
**INCOMPLETE**, coverage 115,952 / 141,488 in the sandbox. Logs:
`gate-current.log`, `gate-no-sb2.log`, `gate-no-sb2-required-dir.log`,
`gate-final.log`. No canonical source/header/manifest was modified.

[EVIDENCE] A final relink after canonical restoration reproduces **every raw
section payload byte** of the measured canonical baseline (`.text`, `.rdata`,
`.data`, `.rsrc`), including the same 30,187 oracle-equal `.text` bytes.
Recompilation changes COFF timestamps and whole-object hashes; both measured
and final hashes are retained in `provenance.json`. The unchanged-source
control objects differ only in their COFF timestamp fields. Section equality
between these two candidate links is not a claim of oracle image equality.

```powershell
.pyenv\python.exe tools/verify.py --build-dir sol-icf
.pyenv\python.exe tools/relink.py --objects-dir build/verify-sol-icf/objects --out-dir build/sol-icf/current --diagnostic
.pyenv\python.exe scratch/sol-icf/measure.py host-current
.pyenv\python.exe scratch/sol-icf/measure.py legacy-sp3 --crt C:/tools/msvc5_sp3/VC/REDIST/MSVCRT.DLL
.pyenv\python.exe scratch/sol-icf/check_models.py
git apply --check scratch/sol-icf/catalog.patch
```

[EVIDENCE] `catalog.patch` is a reviewable **documentation/catalog-only**
candidate: correct BLUPI-LNK-01's obsolete menu ownership and add the proven
BLUPI-LNK-02 rule plus the negative control. Plain `git apply --check` passes.
It is not applied. All work is in ignored scratch/build; no commit, original
game/linked-image execution, toolchain-file modification, or background
process was performed/left running.

[OPEN] Exact oracle eligible contribution population/order is not recovered.
For the four identical getters the required condition is that the historical
qsort permutation put sound's record before network and both pixmap records;
the measured 793-entry input does not satisfy it. Final PE bytes cannot reveal
all dead input contributions. Future source-backed corrections must predict
the entire changed vector, rather than search arbitrary counts. A plain launch
environment using the historical CRT is also not established. Consequently
there is no claimed byte-identical relink or complete oracle-survivor fix.
