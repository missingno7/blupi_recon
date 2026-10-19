# Remaining-blocker checks (2026-10-19)

Cleanup note: the historical paths below were consolidated into the local research
archive; restore scripts as described in [RECOVERY_MILESTONE.md](RECOVERY_MILESTONE.md).
Large intermediates must be regenerated. The findings below remain the canonical record.

[EVIDENCE] Fresh checks of commit `78617ac` produced no new STRICT function. Canonical source and
`recon.json` are unchanged. The two open functions remain TreatEventBase (8 differing instructions)
and ChangePhase (8). All experiments below used VC5 SP3 and the manifest flags through `tools/msvc.py`,
serially, with scratch inputs and build outputs.

## Declaration controls

[HYPOTHESIS] An ignored scalar return value could affect allocation before disappearing from the
emitted code. The `register` storage class could affect the rank-zero toggle's old-value range.

[EVIDENCE] `scratch/blockers-20261019/declarations.py` made 17 full-event-TU compiles:

- Canonical and oracle-ordered ChangePhase controls: 32 and 4,668 differing non-relocation bytes.
- Nine separate return-declaration changes, and their combination: `void` to `BOOL` for
  CDecor::SetDemoPlay/ChangePause/SetMulti, CSound::StopMusic, CPixmap::MouseInvalidate and
  CNetwork::FreeProviderList/FreeSessionList; `BOOL` to `void` for CNetwork::Send/Close.
  Every function payload equals the oracle-ordered control. Changed decorated callee names are
  unproved relocations; these diagnostic headers are not candidates for promotion.
- `register` on TreatEventBase's `i`, `c` or `bEnable`: every function payload equals canonical.
- A named old flag used by both the toggle and feedback, with and without `register`: both give
  4,619 differing bytes, versus canonical's 8. The storage class does not improve this form.

[EVIDENCE] No result improved either target. These observations concern emitted payloads; allocation
costs were not traced in this round. Sources, objects and results are under
`scratch/blockers-20261019/` and `build/blockers-declarations/`.

## Compiler front-end runtime

[HYPOTHESIS] LINK's dependence on its host CRT suggested checking the compiler's host too. Static
imports show that CL and C1XX import `qsort`; C2 does not. C1XX has two qsort call sites, with numeric
comparators at RVAs `0x28835` and `0x834f4`.

[EVIDENCE] A bounded adaptation of `tools/histlink.py` observed one canonical event.cpp compile:
1,151 qsort calls, all from C1XX return VA `0x10428823`, using comparator `0x10428835`. There were
11,647 array entries in total, at most 301 per call; 67 calls had equal keys. That justified an actual
historical-runtime control instead of assuming sorting was irrelevant.

[EVIDENCE] Two successful controls followed:

1. Route all 1,151 calls to qsort in the unchanged VC5 SP3 redistributable
   (`MSVCRT.DLL`, SHA-256 `e390b0529fb9ee1214bb68302a4658b578134d92ab76bc5eb48f3badd843d210`).
2. Bind all 66 CL and 101 C1XX CRT imports, including data, to that DLL before their CRT startup.
   Every binding was read back. C1XX's import-page protection was temporarily changed and restored.

Both controls reproduce all 94 function payloads, symbolic relocations and COFF symbol order. Each
360,347-byte object differs from the observed system-CRT control only at COFF timestamp byte 4.
TreatEventBase stays at 8 differing bytes and ChangePhase at 32. No bytes were copied from the oracle.

[EVIDENCE] The initial full-front-end attempt stopped at a read-only C1XX import page before compiling;
after correcting temporary page protection, the rerun completed. Every debugger report, including the
failed attempt's printed cleanup result, records process wait 0, zero active job processes and unchanged
tool files. Scope: CL/C1XX in event.cpp; C2 and MSPDB retained their system CRT bindings. Other TUs,
runtime versions and a complete historical OS environment were not tested.

Reproduce the successful controls (one command at a time):

```powershell
.\.pyenv\python.exe scratch/blockers-20261019/compiler_qsort.py
.\.pyenv\python.exe scratch/blockers-20261019/compiler_qsort.py --historical-qsort
.\.pyenv\python.exe scratch/blockers-20261019/compiler_qsort.py --historical-front-end
```

The driver goes through `msvc.main` and its compile lock. Reports are in
`build/blockers-compiler-qsort*/host.json` and `build/blockers-compiler-front-end-historical/host.json`;
comparisons are in `scratch/blockers-20261019/{qsort,frontend}-comparison.json`.

## Fresh acceptance and image measurement

[EVIDENCE] `tools/verify.py --build-dir blockers-baseline`: exit 0, zero accepted regressions,
127,456 / 141,488 bytes (90.1%). `sound.cpp` compiled through the existing historical-path helper and
its full relocation gate passed. The restricted sandbox initially could not start CL (Windows error 623);
the authorized elevated run completed. Verifier totals: 411 STRICT and 6 FOLDED emitted function records;
the remaining records include unused/ambiguous bodies, so these totals are not a count of unique game functions.

[EVIDENCE] A union of the verified coverage intervals plus the two open sections (6,928 and 7,072 bytes)
leaves exactly RVA `0x23890..0x238b0` uncovered. That 32-byte interval is four six-byte DirectX import
thunks and eight INT3 alignment bytes, identified by static disassembly and `oracle/functions.csv`.
It is included in the existing coverage denominator but is linker output rather than another source function.
The coverage metric and acceptance rules are unchanged.

[EVIDENCE] `tools/relink.py --objects-dir build/verify-blockers-baseline/objects --out-dir
build/blockers-relink --historical-host`: gate PASS, zero regressions, no degraded path. Output is
535,552 bytes, SHA-256 `557ca92e6ec8c71cfaf5e9f2d55969d3ebcd614c771c9010e074c6ad82ba3533`.
Static, same-offset comparison with the original:

| Area | Differing bytes |
|---|---:|
| Headers | 2 |
| `.text` | 121,100 |
| `.rdata` | 298 |
| `.data` | 1 |
| `.rsrc` | 0 |
| Whole file | 121,401 |

[EVIDENCE] Header differences are file offsets `0x94` (Rich count) and `0xd8` (entry RVA).
The sole `.data` difference is in the pointer at RVA `0x7bc60`: original `0x424bb0`, reconstructed
`0x424b90` (the latter maps to `__exit`). Section sizes, RVAs, raw offsets and timestamp agree.
The historical-host report confirms zero surviving processes and unchanged tool files. No linked image
or original game executable was run.

[OPEN] The source IL distinctions identified in `RESIDUAL_CAUSES.md`, the original link input population
and order, and the exact linker build remain unresolved. These controls narrow the tested alternatives;
they do not establish that 100% matching is impossible or justify steering constructs in canonical source.
