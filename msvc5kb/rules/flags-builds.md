# Flags and compiler builds

### FLG-01 Select flags from strict target evidence, not from option names
- Symptom: `/Ox` or a preferred optimization level is proposed to explain a small instruction-order mismatch.
- Mechanism: optimization options select different code paths, but a flag's name does not predict whether its output matches a target.
- Conditions: SB1 control: two strict-target functions were tested in full TUs across RTM, SP2, SP3, NT5 1592/1691, VC6, and `/O1`, `/O2`, `/Ox`. `CreateBufferFromWaveFile` matched at 240 bytes only for VC5 `/O2` and `/Ox`; `ConvLongToPos` matched at 32 bytes under `/O2` and `/Ox` for all six compiler trees, including VC6. On these two functions `/O2` and `/Ox` object bytes were identical, while the misc TU lost one strict peer under `/Ox`. Blood II `/Ox` evidence is from a different project.
- Action: compile the owning TU with one flag changed at a time; record target function bytes and every peer function. Do not infer a global winner.
- Experiment: hold source, include order, defines, runtime, EH, and compiler tree fixed; compare `/O1`, `/O2`, and `/Ox` by strict target status, length, and function bytes.
- Evidence: blupi_recon: `docs/MSVC5KB_VALIDATION.md` - 36 full-TU compiles; sound `/O1` sizes 153 (VC5) and 146 (VC6), all mismatch; utility `/O1` size 13, all mismatch; selected `/O2`/`/Ox` body hashes match, but misc TU strict count is 6/12 under `/O2` and 5/12 under `/Ox`. blood2_recon (private): `docs/toolchain.md` - one SP3 Object project saw no byte change from `/G5 /O2` to `/Ox`; `/Ob2` reduced reported match rate to about 68%.
- Confidence: proven
- Transfer: flag-specific.

### FLG-02 VC5 service-pack rankings can differ by TU despite one matching function
- Symptom: SP2/SP3 appear interchangeable on a utility function, but another TU has a different set of strict peers under RTM.
- Mechanism: source, compiler package, and TU context interact. A single strict function cannot establish whole-TU equality.
- Conditions: SB1 `/O2 /GX /ML /Gd`, with VC5 SP3 headers explicitly searched first, produced identical selected-function bytes in SP2, SP3, RTM, NT5 1592, and NT5 1691 for two TUs. In `sound.cpp`, RTM differed from SP3 in `CreateSoundBuffer` (23 fixed bytes) and `ReadData` (232 fixed bytes), and the TU had 20 rather than 21 strict functions. In `misc.cpp`, `ConvLongToPos` was byte-identical across the same builds. This is a controlled SB1 counterexample to universal package interchangeability.
- Action: compare the owning TU and its strict peers under each candidate package. Keep header roots and flags fixed; isolate compiler components before naming the responsible pass.
- Experiment: compile the same scratch TU with each tree, fixed VC5 headers, and identical flags; compare per-function bytes and relocation-aware strict statuses.
- Evidence: blupi_recon: `docs/MSVC5KB_VALIDATION.md` - sound/misc sweeps; `build/kb/compiler_sweep_sp3headers/peer_build_byte_diffs.json` - raw body differences and fixed-byte counts.
- Confidence: proven
- Transfer: version-specific; observed functions and flags only.

### FLG-03 Rank compiler packages for the target and keep the linker fixed
- Symptom: one compiler is called globally best because it wins one module or one TU.
- Mechanism: source, target, flags, and TU context interact. Per-function wins can coexist with different whole-module rankings.
- Conditions: Blood II compiler ranking held the SP3 linker constant and ranked strict function counts rather than byte coverage.
- Action: sweep packages only for the owning source and same target; report strict functions and bytes, then isolate front end, back end, and TU context.
- Experiment: build every candidate with identical source, flags, headers, and one linker; compare strict results before ranking.
- Evidence: blood2_recon (private): `analysis/tcrank.json` - Object 2.1: SP3 2,324/2,587 strict functions, NT5 2,323, SP2 2,315, RTM 1,595, VC6 974; union 2,382 and 205 unmatched by all. CShell: SP2/SP3/NT5 each 1,835; Server: each 1,656. `docs/object_residue.md` - 40/58 apparent other-backend-only functions were count-reproducible.
- Confidence: proven
- Transfer: project-specific.

### FLG-04 `/Ox` is not automatically a distinct tuning lever
- Symptom: changing `/O2` to `/Ox` is proposed to explain one small scheduling residue.
- Mechanism: in one measured SP3 Object project, `/Ox` selected no function bytes different from `/G5 /O2`.
- Conditions: one project/configuration only; other options and source can change the result.
- Action: do not spend a source search on `/Ox` until a one-TU check proves it changes the target output.
- Experiment: compile the same TU with all settings fixed except `/O2` versus `/Ox`; compare objects, not banners.
- Evidence: blood2_recon (private): `docs/toolchain.md` - `/Ox`, `/G6`, and `/GB` matched `/G5 /O2` in the measured Object build; `/Ob2` reduced its reported match rate to about 68%.
- Confidence: negative
- Transfer: flag-specific.
