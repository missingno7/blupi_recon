# MSVC 5.0 knowledge-base transfer validation

This document records the KB transfer checks against Speedy Blupi 1. **[EVIDENCE]** marks measured outcomes, **[HYPOTHESIS]** marks the question being tested or an interpretation, and **[OPEN]** marks unresolved coverage. Canonical `src/` files were not edited. Source variants and TU snapshots are under ignored `scratch/kb/merge/`; compiler objects, listings, JSON, and logs are under ignored `build/kb/`.

## Strictness and setup

**[EVIDENCE]** All PE comparisons below use `tools/cmpfunc.py` strict status: the full candidate function is compared at a forced or unique RVA, object relocation fields are checked, and unresolved targets remain `RELOC-UNVERIFIED`. The compiler/sweep checks also supplied the known full target function length. A normalized instruction diff is never counted as equality.

**[EVIDENCE]** Baseline command context: VC5 SP3 cl 11.00.7022, `/W3 /GX /O2 /ML /Gd`, defines `WIN32`, `NDEBUG`, `_WINDOWS`, `nullptr=0`, and `C:\tools\dx6sdk\INCLUDE`. A byte-preserving snapshot of all four TUs and their local headers was made before experiments. The four baseline object builds produced 34/34 strict accepted functions: sound 18/18, movie 12/12, misc 1/1, ddutil 3/3. Totals over all emitted functions were 18/27, 12/13, 1/12, and 3/9 strict respectively.

| Snapshot file | SHA-256 |
|---|---|
| `ddutil.cpp` | `6b8fbf357b64fdfe246e03faa4433fd2ddbf3da1b1801649c632867d320174d1` |
| `ddutil.h` | `6b0b625f2d23cedb51b88dc4344e9c98b36444654ad9937ede10c83974373a50` |
| `def.h` | `2c521aa760a93e3398099aaae4fd1f56041c8fbe5fb58f26ea957d05a9640b53` |
| `misc.cpp` | `3a884234ec7f4eee52ba02a4ede1c240c542d8d5279ff80f6efd1daedaf16ee5` |
| `misc.h` | `cc34d61ab605486fb666f72c3cb7055fc3dfe5205b988c96060fcc99b288d57f` |
| `movie.cpp` | `b4a113af16bd69522b9c6e7ceea36fe0a36dbb5f4ef11bbcd9aac60f58098e70` |
| `movie.h` | `9a0054f473b7260449d23f0919ddf25591164ababc11aa0241fb43d88388e195` |
| `resource.h` | `bbafbde619ec7a735c7beb54c32e9841b0f4a3dbafa4c1b973f33b6042c8655a` |
| `sound.cpp` | `7fbc9e7ebecda92879a70137e211814980ba9d3b7bd6e8b3b98da4d7b41830bf` |
| `sound.h` | `be68a7fc3af1fdb5ee70a407affdd1453d1ea86b2d6a135b68aac732e428dfb9` |

## Experiment 1: baseline

- **[HYPOTHESIS] Hypothesis:** the current reconstructed TUs reproduce every already accepted strict function before any probe or source-form variant is considered.
- **Rule IDs:** baseline gate for all rules.
- **[EVIDENCE] Baseline:** four copied TUs and local headers; compiler and flags listed above; 34 accepted functions in `recon.json`.
- **[EVIDENCE] Result:** 4 compiles; all 34 accepted functions were `STRICT`. The all-function counts are shown above. Results and compiler logs are in `build/kb/baseline/`.
- **[HYPOTHESIS] Conclusion:** usable strict baseline for this workspace snapshot. This does not claim a whole-image rebuild or full-project PCH/link equivalence.

## Experiment 2: TU counter placement and sensitivity

- **[HYPOTHESIS] Hypothesis:** a strict sound function or one of its peers might change when declarations are inserted at different TU positions, as observed in the VC5 engine corpus.
- **Rule IDs:** CTX-01, CTX-02, CTX-03, CTX-04.
- **[EVIDENCE] Baseline:** scratch snapshot of `sound.cpp`; SP3 `/O2 /GX /ML /Gd`; target `?CreateBufferFromWaveFile@CSound@@IAEHHPAD@Z`, RVA `0x22b40`, 240 bytes. The independent peer mappings for `CreateSoundBuffer` and `ReadData` were supplied so their call relocations could be checked.
- **[EVIDENCE] Experiment:** 21 compiles: counts `0, 1, 2, 4, 8, 16, 32` at top, end, and immediately before `CreateBufferFromWaveFile`. A stable scratch source pathname was reused to keep path-sensitive constructs fixed. `vc5probe` compared every function in the TU; four additional `/FAs` compiles captured counter labels.
- **[EVIDENCE] Result:** the target remained `STRICT` at all 21 points; all 27 emitted functions retained their baseline status and RVA, with 21 strict, 1 reloc-unverified, and 5 mismatches. Raw object bodies did move: every sampled nonzero top probe changed four bodies (`CreateSoundBuffer`, `StopAllSounds`, `PlaySoundDS`, `InitMidiVolume`); every nonzero before-target probe changed three later bodies (`StopAllSounds`, `PlaySoundDS`, `InitMidiVolume`); end probes changed no function body. None of these changes gained or lost strict functions.
- **[EVIDENCE] Counter listing:** the parser saw `$L` label-set changes in 19 functions after a top declaration, 18 after a before-target declaration, and 5 after an end declaration. This listing exposed no `$T` labels, so it does not establish which SB1 counter phase moved.
- **[HYPOTHESIS] Conclusion:** the Blood II count-sensitivity mechanism has a measurable SB1 effect on some non-strict object bodies, but this sampled sound target has no strict window transition through 32 declarations. The placement/count pattern is not evidence for original filler or a source edit. More TUs/functions are needed before concluding how often SB1 code responds.
- **Artifacts:** `build/kb/counter_stable/` and `build/kb/counter_listings/`.

## Experiment 3: compiler build and optimization sweep

- **[HYPOTHESIS] Hypothesis:** compiler package or optimization differences may explain SB1 function residues, but any rank should be target- and TU-specific.
- **Rule IDs:** FLG-01, FLG-02, FLG-03, FLG-04.
- **[EVIDENCE] Baseline:** full `sound.cpp` and `misc.cpp` TU snapshots. All runs used `/W3 /GX /ML /Gd`, `WIN32`, `NDEBUG`, `_WINDOWS`, `nullptr=0`, DX6 headers, and explicit VC5 SP3 headers searched before compiler-tree defaults. Flag variants were `/O1`, `/O2`, `/Ox`. The same oracle PE and linker output were held fixed; compilation stopped at COFF object output.
- **[EVIDENCE] Experiment:** 36 compiles (six compiler trees × three flag sets × two TUs): VC5 SP3, SP2, RTM, NT5 1592, NT5 1691, and VC6 RTM.
- **[EVIDENCE] Sound target:** `CreateBufferFromWaveFile`, RVA `0x22b40`, 240 bytes. Under `/O2`, SP2, SP3, and both NT5 trees were `STRICT` and the TU had 21/27 strict functions; RTM was `STRICT` for this target but had 20/27 strict functions. RTM's `CreateSoundBuffer` body differed from SP3 in 23 non-relocated bytes and `ReadData` differed in 232 non-relocated bytes. `/Ox` emitted the same selected function bytes and strict counts as `/O2`. Every VC5 `/O1` run produced a 153-byte mismatch and 5/27 strict functions. VC6 `/O2` and `/Ox` produced 240-byte mismatches and 13/27 strict functions; VC6 `/O1` produced a 146-byte mismatch and 5/27 strict functions.
- **[EVIDENCE] Misc target:** `ConvLongToPos`, RVA `0x1f330`, 32 bytes. `/O2` and `/Ox` were `STRICT` under all six compiler trees; `/O1` produced a 13-byte mismatch under all six. Target function bytes were identical across all six trees for each flag set. The full TU had 6/12 strict under `/O2` and 5/12 under `/Ox` for VC5; one peer changed status even though `ConvLongToPos` did not.
- **[HYPOTHESIS] Conclusion:** SB1 transfer is target-specific. The SP2/SP3/NT5 selected functions agree in these runs; this does not establish whole-TU identity. The RTM sound result is a new counterexample to treating all VC5 drops as globally interchangeable: two same-TU helpers changed with compiler tree while the selected target function remained strict. VC6 matches the small utility but not the larger sound function. `/O2` and `/Ox` were identical in the selected function bodies, but `/Ox` changed a misc peer's strict status.
- **Artifacts:** `build/kb/compiler_sweep_sp3headers/` and `build/kb/compiler_sweep_misc_fixedheaders/`.

## Experiment 4: source-form transfer on a nonmatching SB1 function

- **[HYPOTHESIS] Hypothesis:** the Blood II direct-member/reload observation might transfer to `PlaySoundDS` in SB1.
- **Rule IDs:** MEM-01, REG-03.
- **[EVIDENCE] Baseline:** `?PlaySoundDS@CSound@@IAEHKK@Z` was a 128-byte `MISMATCH` with no wildcard hit in the unforced strict comparison of the baseline sound object; the sound TU had 21/27 strict functions.
- **Source change:** only in `scratch/kb/merge/sourceform/sound_cached_member.cpp`, after the existing bounds check, read `m_lpDSB[dwSound]` once into a typed `LPDIRECTSOUNDBUFFER` local and use that local for the status and play calls. The canonical source was not edited.
- **[EVIDENCE] Experiment:** 1 candidate compile plus strict object comparison. A byte-level candidate-versus-baseline function diff was also classified by `vc5probe diff`.
- **[EVIDENCE] Result:** candidate body was 112 bytes and classified as size/structural versus the 128-byte baseline body. It remained `MISMATCH` with zero wildcard hits. Strict peer count stayed 21/27 and no peer status/RVA changed; there was no strict gain.
- **[HYPOTHESIS] Conclusion:** the source form changes VC5 code generation but does not validate as an SB1 reconstruction. The cached pointer crosses external COM calls, so its semantic equivalence also needs independent proof before any integration.
- **Artifacts:** source variant and preparation script in `scratch/kb/merge/sourceform_prepare.py`; outputs in `build/kb/sourceform/`.

## Experiment 5: inline-budget applicability

- **Hypothesis:** a visible inline/template candidate in the four SB1 TUs might exercise the `/O2` per-caller budget rule.
- **Rule IDs:** INL-01, INL-02, INL-04, TMP-01.
- **[EVIDENCE] Baseline:** the captured `sound.cpp`, `misc.cpp`, `movie.cpp`, `ddutil.cpp` files and their local headers.
- **[EVIDENCE] Result:** a source/header scan found no explicit `inline` or `template` definitions in these four TUs or local headers. No compiler run was made because there was no source-level candidate to measure. This does not prove that the optimizer never inlines a visible ordinary function.
- **[HYPOTHESIS] Conclusion:** Blood II inline-budget observations need validation in another SB1 TU or reconstructed header context; for the current four TUs the synthetic budget rule is not directly actionable.

## Retrieval on real SB1 residuals

**[EVIDENCE]** Ran the SB1 catalog tagger and hints on four existing strict-comparison JSON reports. Strict rules were unchanged: these tags are triage only. Unless a mapping is established independently, a forced RVA is advisory. Three cases had existing targeted source experiments; the CJauge investigation used five candidate compiler invocations in total (two value-12 builds, one failed duplicate-header attempt, and two value-5 builds including full-TU verification). No canonical-source edits.

| Function / comparison | Ranked hint and evidence followed | Candidate work and measured outcome |
|---|---|---|
| `CSound::PlaySoundDS`, best-candidate anchor `0x5880` (unmapped; 128 candidate bytes vs 68-byte anchor) | Rank 1: `VC5-SEM-01`; the instruction pair has immediate differences, but this low-score anchor is not a proven target mapping. | One earlier cached-member candidate compile changed 128 to 112 bytes and remained `MISMATCH` with zero wildcard hits; the TU stayed at 21/27 strict. No strict gain. Recorded as `BLUPI-NEG-01`; do not reuse the cache form without new evidence. |
| `ChangeSprite`, forced `0x1f210` (320 candidate bytes vs 285-byte body) | Rank 1: `VC5-SEM-01`; inspect actual cursor IDs/values before compiler-form hypotheses. | Two earlier source attempts changed cursor IDs/initial value; the second body was 288 bytes but still `MISMATCH`. No strict gain; register/branch residual remains. `BLUPI-DIA-01` records the decompiler limit. |
| `CPixmap::Create`, approximate `0x21190` association (832 vs 902 bytes) | Rank 1: SB1-validated `BLUPI-LAY-01`, then `VC5-SEM-01`; check the measured `this+8` slot and all later offsets before codegen edits. | Four recorded CPixmap object variants include the neutral `+8` member; the isolated candidate TU has 20 `STRICT`, 4 `RELOC-UNVERIFIED`, and 21 `MISMATCH` functions. `Create` itself remains mismatched: reduced TU residual, no strict gain for this target. |
| `CJauge::Draw`, forced `0x1ee50` (256 vs 255 bytes) | Rank 1: `VC5-SEM-01`; inspect the differing immediate against the source/header value. The oracle pushes 5 twice; the candidate header had `CHJAUGE=12`. | Five candidate compiler invocations: the first two value-12 builds remained `MISMATCH`; one copied-header attempt failed because duplicated include paths redefined `CHJAUGE` and `Term`; two value-5 builds then produced `RELOC-UNVERIFIED` at `0x1ee50`; all three `DrawPart@CPixmap` `REL32` relocations lack a unique verified target RVA. This reduces the residual but is not `STRICT`. Full `verify.py` exited 1 on accepted-function checks in button, menu, and text TUs; no accepted Jauge function regressed. A clean baseline run was not repeated, so those other-TU statuses are left unattributed. |

**[EVIDENCE] Verdict:** retrieval consistently puts literal/offset investigation before source-form experiments when those operands differ. The CPixmap layout rule is directly supported by SB1 offsets. The CJauge constant correction is validated as a source-value fix but remains relocation-unverified; general `VC5-SEM-01` stays `transferred-unverified` because these cases do not establish that every such residue is a source defect. No new strict match was obtained; the CJauge correction, prior local-cache change, and CPixmap layout work reduced residuals without making the selected functions strict. PlaySoundDS and CPixmap target addresses are advisory, not confirmed identities.

## Experiment 6: `blupi.cpp` and `decnet.cpp` source forms

- **[EVIDENCE] Setup:** VC5 SP3 11.00.7022, `/W3 /GX /O2 /ML /Gd /Gy`, defines `WIN32 NDEBUG _WINDOWS nullptr=0`, private headers under `scratch/tu/blupi_include/`, both candidate objects compared together. The oracle was only inspected statically.
- **[EVIDENCE] `BLUPI-BENCH-01` (Blupi-specific):** the inherited SB2 benchmark used an ascending loop and memory text; target pseudocode instead counts down ten calls, handles millisecond wrap, divides elapsed time by ten, writes `CheckTime = %d\r\n` to `data\\time.blp` if open, and returns the average. The source repair produced `STRICT` at `0x1d60` (208-byte candidate span; 204-byte Ghidra body), with all relocations proven.
- **[EVIDENCE] `BLUPI-SETDECOR-ARGS-01` (Blupi-specific):** the initial `Build(RECT)` call was a MISMATCH. Static target disassembly pushes the `RECT` and a `POINT`; declaring the private call as `Build(RECT, POINT)` and passing `posMouse` made the 208-byte span at `0x14f0` `STRICT`. The call maps to oracle `0x404350`; that callee ends with `ret 0x18`, confirming six stack words. This is a call/header correction, not a general compiler rule.
- **[EVIDENCE] `VC5-SEM-01` / `ReadConfig` (unverified source form):** the target loop advances its source pointer. Switching the candidate from `pText[i]` reads to `*pText` plus `pText++` reduced the candidate from 688 to 672 bytes; the target body is 673 bytes and remains MISMATCH. Final `kb.py hints` still tags condition, immediate, displacement, and load-count differences.
- **[EVIDENCE] `VC5-SEM-01` / `UpdateFrame` (unverified source reconstruction):** removed the SB2 intro-phase branch absent from target pseudocode. The candidate shrank from 544 to 512 bytes against 528 target bytes; still MISMATCH.
- **[EVIDENCE] `VC5-LOC-03` / `ObjectStart` (unverified scope hypothesis):** moving `NetMessage` to function scope matches the target `sub esp,0x14` frame, but the function remains 480/489 bytes and MISMATCH. An inline `POINT` probe produced a `0xc` frame and was reverted.
- **[EVIDENCE] `BLUPI-DECNET-DIV-01` / `VC5-SEM-01` (negative):** `(pos+63)/64` did not match. Direct two-dimensional cell indexing and the explicit flattened `/DIMOBJX`, `* MAXCELY`, `/DIMOBJY` source forms produced byte-identical code; both remain a 112-byte MISMATCH against 100 target bytes.
- **[EVIDENCE] `VC5-SEM-01` / `DoInit` (unverified):** inline `POINT` arguments and the three target `OutputDebug` call sites (`Image: init`, `SavePalette`, `InitSysPalette`) produced a 2,800-byte candidate against 2,801 target bytes, still MISMATCH. Target allocation sizes are recorded in `docs/BLUPI_TU.md`; the remaining immediate/displacement differences are unresolved.
- **[EVIDENCE] `WindowProc` triage (unverified):** final hints are `native_fewer_loads` and `native_more_stack`; top transferred hints are `VC5-INL-03`, `VC5-LOC-02`, `VC5-LOC-03`, `VC5-LOC-04`. No source change was made from these hints alone.
- **[EVIDENCE] Aggregate:** the final two-object comparison has 9 STRICT blupi bodies and 4 STRICT decnet bodies, zero relocation-unverified bodies. `ObjectDelete` at `0x108a0` is STRICT and owned by `decnet.cpp`, resolving the engine worker's fold suspicion; it is not a folded duplicate.
- **Artifacts:** `build/blupi/results/blupi_decnet_final_worker.json`; per-function statuses and private header deltas are in `docs/BLUPI_TU.md`.

## [EVIDENCE] Transfer summary

| Finding | SB1 status |
|---|---|
| Fixed compiler, flags, include paths, TU, and strict relocation checks before editing source | Directly applicable workflow; baseline and toolchain docs support it. |
| `/O1` versus `/O2` and `/Ox` effects | Confirmed for two SB1 functions only; not a global rank. |
| VC5 RTM/SP2/SP3/NT5 interchangeability | Not universal: selected functions agree in part, but RTM changes two sound helpers under fixed SP3 headers. Broader TU coverage remains open. |
| TU counter sensitivity | Some SB1 sound object bodies changed, but no strict status flipped in the bounded scan; phase attribution and other TUs remain open. |
| Direct member reload versus cached local | Tested on `PlaySoundDS`; code changed but no strict gain. Needs new source/target evidence. |
| Blood II inline budgets, nested wrapper sites, EH lowering, templates, duplicate COMDAT handling | Mechanisms remain supported only under the cited Blood II conditions; the current four SB1 TUs did not provide a visible inline/template candidate. |
| `VEC_*`, packet wrappers, helper signatures | Project conventions only; irrelevant to these SB1 TUs unless independently present in SB1 source. |
| COMDAT folding, Rich data, imports, and thunk order | Linker-only; retain separate from compiler-function equality. SB1 exact 5.10 patch remains open. |

## [OPEN] Open checks

- Test counter and source-form rules on a second nontrivial SB1 TU with a proven target boundary and independently mapped relocations.
- Capture `/FAs` `$T` output or an alternate counter view if the VC5 SB1 compiler emits it; current `sound.cpp` listings exposed `$L` labels only.
- Isolate RTM versus SP3 `c1xx` and `c2` components under the fixed-header setup before attributing the two sound-helper differences to one compiler pass.
- Find an SB1 header/TU with a visible inline/template body before importing the Blood II inline-budget threshold.

## Experiment 7: StopSound declaration context and the MouseMove conflict

- **[EVIDENCE] Baseline:** The copied canonical source tree compiled with VC5 SP3 cl 11.00.7022, `/W3 /GX /O2 /ML /Gd /Gy`, defines `WIN32`, `NDEBUG`, `_WINDOWS`, `nullptr=0`, and the DX6 include directory. The full accepted-function comparison covered all 9 manifest TUs: 103/104 accepted functions were `STRICT`, with no baseline regression.
- **[EVIDENCE] Candidate:** In a scratch copy only, add `BOOL StopSound(int channel);` after `Play` in `sound.h` and add the already reconstructed `CSound::StopSound(int)` body to `sound.cpp`. The body at RVA `0x23050` is `STRICT`: the oracle body is 99 bytes; the object function is 112 bytes with 13 trailing NOPs and no relocations.
- **[EVIDENCE] Cross-TU result:** The StopSound-only variant has 100/104 accepted functions `STRICT`. Three accepted functions regress from `STRICT` to `MISMATCH`: `CButton::TreatEvent` (`0x2d60`), `CButton::MouseMove` (`0x2fd0`), and `CMenu::TreatEvent` (`0x1f040`). `kb.py tags` on MouseMove reports `reorder_regalloc`, `block_reorder`, `load_reorder`, and `sib_order`; this is a code-shape change, not a source-semantic fix.
- **[EVIDENCE] Counter window:** With the StopSound declaration present, `vc5probe` tested counts 0–32 at the top of `button.cpp`, immediately before `MouseMove`, and at end of file. Top and before-function probes restore `MouseMove` to `STRICT` for counts 7–14 and 23–30. End probes never restore it in the tested range. This localizes the measured effect to context preceding `MouseMove`; attribution to a front-end numbering choice is an inference from placement, not a captured internal counter label. Probe declarations are diagnostic only.
- **[EVIDENCE] Header candidates:** Planet Blupi v1.8.0 has no `StopSound(int)` declaration; its later `StopSound(bool,Sint32)` and other sound APIs do not match the SB1 target method. SB2 places `StopSound(int)` after `Play` and also declares `ErrorSomething`, `SetCDAudio`, and `PlayCDAudio`. A scratch header using those three additional SB2 declarations still leaves MouseMove and CButton::TreatEvent mismatched, and makes CMenu::TreatEvent `RELOC-UNVERIFIED`. The SB1 sound block does not establish those SB2 methods as SB1 declarations.
- **[EVIDENCE] Other included headers:** `button.cpp` includes `def.h`, `pixmap.h`, `sound.h`, `decor.h`, `button.h`, and `misc.h`; PB and SB2 use different include lists and orders. Importing their `button.h`-first order fails against the current SB1 header dependencies because `button.h` needs `CPixmap`, `CDecor`, and `CSound` declared first. No upstream include-order candidate was therefore treated as an original context.
- **[OPEN] Missing sound context:** The target helpers at `0x22f00` and `0x22fc0` remain unnamed. Their `ret 0x10` and `ret 0x0c` cleanups and decompilation show playback-helper behavior, but do not prove their C++ names or prototypes. Current `StopAllSounds()` and `PlaySoundDS(DWORD,DWORD)` declarations do not identify them. Guessed overloads would be filler, so no header delta is proposed yet.
- **[OPEN] Conclusion:** The strict StopSound body and accepted MouseMove cannot currently coexist under a source-supported scratch header. Reconstruct the original SB1 helper declarations/order from stronger source or symbol evidence, then rerun the full accepted-function comparison before proposing a shared header change.
- **Artifacts:** Count probes and copied header variants are under `scratch/ctx/`; outputs are under `build/ctx/probes/` and `build/ctx/verify4_*`.

## Experiment 8: Authentic aggregate headers

- **[EVIDENCE, Blupi-specific]** Against HEAD `a998026`, private complete trees remove def.h's Object enum, 14 POINT
  operators, MAKE_RECT/MAKE_POINT and constructor macros; all 48 TU uses become local aggregates or existing-local
  field assignments. Existing CButton::Create static symbol suffixes measure reductions 443, 62 and 12 (517 total).
- **[EVIDENCE]** Four full-tree `verify.py --build-dir sol-macros` states use pinned VC5 SP3 /O2 /GX /Gy, manifest
  settings and private headers. Base / +tables+DecorNextAction / -44 externs / both have 3/3/5/4 accepted regressions.
  DecorNextAction is STRICT (272 bytes); the last state restores GetBlupiInfo. Every state regresses MouseMove, whose
  TU has no aggregate replacement. Sound's sandbox build-path limitation is expected.
- **[EVIDENCE]** 859 serial `ctxsweep_tu.py` diagnostic compiles check all accepted peers with private headers beside
  its generated TU. [LINK_CONTEXT.md](LINK_CONTEXT.md#authentic-aggregate-experiment-sol-macros-2026-10-18) records
  every regression and bounded window, including new IsVentillo thresholds 106/150. Masked byte equality is diagnostic
  only; full verifier outcomes provide acceptance evidence. No probe declarations are retained in candidate source.
- **[EVIDENCE, negative]** The six changed TUs preserve helper-control non-relocation function bytes except
  GetBlupiInfo/MoveObjectSort (context) and unmatched DrawMap. Separate zero POINT aggregates preserve Build's body;
  existing-pos reuse changes 8 bytes (rejected). DrawMap local RECT, common-block, member-assignment and clip-reuse
  forms give frame 0x38 and 159 non-relocation differences against the oracle; all-four-before-Build gives frame 0x28
  but still 146 differences. No tested authentic replacement preserves it.
- **[OPEN]** No zero-regression authentic state or integration patch is established. Artifacts are in
  `scratch/sol-macros/`; canonical source and acceptance records are unchanged. Catalog BLUPI-CTX-01 and BLUPI-REG-03
  retain the findings and negatives.

## Experiment 9: DX5 decmove include placement

[EVIDENCE, Blupi-specific] At `70d630c` plus `dx5-authentic.patch`, moving
the existing `decor.h` include below `misc.h` and `dectables.h` in decmove.cpp
makes MoveObjectSort STRICT at `0x13b20` (240 bytes, two verified calls).
The model predicts 31 + 105 + 1 = 137 declarations/parameter numbers before
decor.h. Both orders of the two moved headers pass every active accepted
entry in whole-tree verification; DecorNextAction stays STRICT (272 bytes).

[EVIDENCE, negative] Moving only misc.h (31), or only dectables.h with
obstacle.h (106), leaves MoveObjectSort as the sole accepted regression.
No declaration or body was added. Seven serial full-tree runs and one
baseline fnprobe used pinned VC5 SP3 /O2 /GX /Gy and DX5; sound retained the
expected INCOMPLETE historical-path gate and passed accepted code-byte checks.

[HYPOTHESIS] This is a plausible SB1 include arrangement using existing,
needed declarations; exact historical ordering is not established by PB.
PB b0270ab's decmove.h has only two one-argument prototypes (cost four),
and its movement/icon tables are not SB1's moving-object animation tables.
Supervisor gate outside the sandbox: `tools/verify.py --accept` 113,344 / 141,488, zero regressions; integrated
(docs/LINK_CONTEXT.md "DX5 state integrated").

## LINK 5.10 ICF survivor order (sol-icf, 2026-10-18)

[EVIDENCE; BLUPI-LNK-01/02] The ICF pass sorts a contribution vector with
the imported CRT qsort, comparing only effective raw size and relocation count;
the first equivalent contribution survives. Eight complete vector permutations
match predictions, including two genuine dead-SB2-stub removal controls.
The host Windows CRT and historical VC5 CRT have different unstable tie orders.
Using the original VC5 qsort in a debugger session keeps ehvecdtr's unwind filter
and moves entry 0x24cf0 -> 0x24d10; oracle entry is 0x24d40. No tool files were patched.

[EVIDENCE, negative] Removing PlaceDecor8..10 passes every accepted-function
check, but fails to reproduce all oracle survivors with either CRT algorithm.
GetNbProviders is genuine: oracle call 0x1a378 targets the shared getter at
0x22dc0, which physically lies in sound.cpp's block. No game-source patch is proposed.
[OPEN] Recover the original eligible contribution population/order and a plain
historical-runtime launch environment. Details, diagnostic limitations, exact
addresses, commands and unmasked linked-byte metrics: scratch/sol-icf/FINDINGS.md.

## Declaration-type residue family (sol-proto, 2026-10-18)

[EVIDENCE; BLUPI-TYPE-01] 1,211 serial private-header/TU variants (1,107
singleton/additional controls, 103 mover pairs, one 16-return joint control)
give no qualifying reduction of TreatEventBase/DrawButtons/ChangePhase/Build
(8/63/32/2328 differing bytes). Normal setters returning their assigned value,
wrappers returning their final call, PlayImage returning void, const values,
coordinate-pair locals and selected enums have inert survivors. The narrower
draw-secret field gives Build 2324 bytes but worse alignment and accepted regressions.

[EVIDENCE] The final ledger has 5,522 completed TU compiles and 86 compiler
failures through tools/msvc.py; all surviving headers are checked in every
transitive including TU. Full private baseline verify exits 0 with zero accepted
regressions and the expected sound.cpp INCOMPLETE historical-path gate.
Relocation-masked fnprobe/COFF screens are diagnostic, never claimed STRICT.
Artifacts and bounded-search limits: scratch/sol-proto/FINDINGS.md, INVENTORY.md,
ledger.jsonl, variants.json, combination-plan.json, gate.log. No game-source patch.

## Cross-project PCH and flag controls (sol-xfer, 2026-10-18)

[EVIDENCE; VC5-CTX-06] A serial 19-TU rebuild in recon.json order with one
private /YX /Fp PCH and /FD /Fd recreates the PCH for every TU. No TU reuses
an earlier TU's PCH. Versus the no-PCH control, all 460 function payloads and
relocation signatures and all emitted $S suffixes are identical; event remains
cheat_code$S33910. Both exact object sets were compared by verify.py's existing
run_comparator: 408 STRICT, 4 FOLDED, 41 MISMATCH, 7 RELOC-UNVERIFIED; zero
accepted regressions, 115,952 game bytes. The sound historical-path gate is
INCOMPLETE (ordinary path used, no drive mapping attempted). No new match.

[EVIDENCE] Identical-header micro TUs demonstrate the missing positive case:
create in A gives $S178; use in B gives $S179; B without PCH and same-TU A
reuse give $S178. This holds under Blupi, Blood II Object, and CShell flags.
Thus /YX is context-specific, not universally neutral or universally a shift.
Declaration and statement costs also agree across those three flag sets.
The RTM c1xx/SP3 c2 header micro has N=0, not evidence for a universal hybrid
equivalence. Saved-register/stack/key-call sample payloads agree under both
/O2 profiles but differ under CShell's /Ot /Oi /Oy /Ob1 profile.

[EVIDENCE] Provenance: blood2_recon@4e3f16bcc523bf981e9a521b4b2f3efa75b4acf1,
docs/object_residue.md, docs/compiler_knowledge.md, and canonical
recon/knowledge/compiler/vc5.json (VC5-CTX-06/10/11, VC5-RA-08).
Its pinned c1xx/c2 hashes equal ours. Complete transfer table, limits,
commands, per-TU PCH lifecycle and suffixes: scratch/sol-xfer/FINDINGS.md,
provenance.json, part-a/analysis.json and micros/results.json.
[OPEN] The rebuild tests current headers and the specified manifest order;
it does not recover the original .dsp, original header prefixes or LINK build.

## Private proposal provenance merge (2026-10-19)

[EVIDENCE] Reviewed all six private KB-CANDIDATES.json files against their
FINDINGS.md [EVIDENCE] passages, including sol-posbuild/r2/FINDINGS.md.
Catalog refs cite each worker directory and FINDINGS section:

- `scratch/sol-posbuild`: Controls A/B/C narrow SRC-03 and extend ALLOC-04;
  round 2 sections 1-4 supply COPY-01, ALLOC-13 and aggregate/tail negatives.
- `scratch/sol-poscpdb`: E1/D1 supply ALLOC-14 connected-range guard costs
  and REG-03 outgoing-copy direction/real-call versus intrinsic limits.
- `scratch/sol-posteb`: toggle controls, KB-gap review and Trace integrity
  qualify REG-10, SCHED-01 and ALLOC-01. Reprioritization evidence is limited
  to zero returns/unchanged priorities; its success formula stays unvalidated.
- `scratch/sol-ccp`: sections 1-3/5-6 supply ALLOC-01 cost/component limits
  and REG-09 integer-interner identity/one-byte-memset negatives.
- `scratch/sol-cdb`: sections 1-4 extend SCHED-02/REG-03 and supply C2-01
  early-forwarding bounds. Proposed C1-02 is absent from this catalog;
  its evidence is incorporated into C2-01 instead.
- `scratch/sol-cbuild`: Minimality, Earliest measured stages and Discriminating
  micros supply C2-01 reconstruction/LICM bounds and ALLOC-02 interference.

[EVIDENCE] Added BLUPI-ALLOC-13/14, BLUPI-COPY-01 and BLUPI-C2-01; extended
existing rules otherwise. New-rule compile counts cover named control pairs
or micro TUs, not worker totals. JSON loading, unique IDs, existing field
shapes/types, known tags and retrieval of all added/changed rules pass.
kb.py has no validate/lint command. Only the catalog and this document changed;
this review used no compiles or game execution.

[OPEN] Historical source, a missing Build header cut, unique oracle allocation
history, the exact LICM descendant and reprioritization success remain
unpromoted. Masked/aligned diagnostics and interventions are never STRICT;
all source negatives retain their measured scope.
