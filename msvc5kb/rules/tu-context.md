# Translation-unit context and counters

### CTX-01 TU symbol count can select equal-size code
- Symptom: a function has the right length and instruction structure, but differs in register choice, commutative operand order, SIB roles, or a small schedule window.
- Mechanism: measured VC5 output choices varied with TU-wide symbol/counter context. The exact c2 tie-break algorithm is unknown.
- Conditions: MSVC 5.0 SP3 engine builds at `/O1` and `/O2`; not every function is sensitive, and the measured windows are not an ABI.
- Action: verify the authentic TU/header context first; use a bounded count scan only to test sensitivity. Never retain synthetic filler in canonical source.
- Experiment: compile an unchanged scratch TU across a declared count range at top, end, and before the target; compare strict target windows and every peer function.
- Evidence: blood2_recon (private): `docs/object_residue.md` - 20/28 residual Object TUs shared a shift across builds; 63–64% of residual bytes aligned after a per-TU shift. `docs/vc5_phase_model.md` - periodic/bounded output windows measured.
- Confidence: proven
- Transfer: version-specific; SB1 sound sweep found no strict flips through 32 declarations in one TU/function.

### CTX-02 Front-end and back-end numbering spans more than one function
- Symptom: a later function or inline copy changes after an earlier local/scope change, although its own source is untouched.
- Mechanism: VC5 front-end symbols are assigned across the TU; generated inline copies, labels, and temporaries continue numbering into back-end work. Earlier functions can also change later numbering through expansion.
- Conditions: measured with VC5 SP3 listings and objects; internal data structures and exact counter formulas remain unknown.
- Action: capture per-function `$L`/`$T` ranges before and after one authentic source change, then compare the entire TU.
- Experiment: compile one scratch source snapshot with and without one used local or real cleanup scope; inspect listing counters and strict status for all functions.
- Evidence: blood2_recon (private): `docs/vc5_phase_model.md` - one added local shifted 15 front-end and 40 back-end numbers, including 13 generated destructor copies.
- Confidence: proven
- Transfer: version-specific.

### CTX-03 Probe placement distinguishes local source context from delayed work
- Symptom: an end-of-file probe has no effect, while a declaration before a function changes that function or later code.
- Mechanism: top and before-target declarations alter source-order numbering; end declarations can affect delayed template/back-end work without preceding a function definition.
- Conditions: placement effects depend on anchor, function order, template instances, and the current TU.
- Action: test top, end, and before-target separately. Treat a matching count as a constraint, never as evidence that filler declarations existed in the original.
- Experiment: use one unchanged TU, one bounded count list, and identical flags for each insertion point; retain per-function strict vectors.
- Evidence: blood2_recon (private): `docs/vc5_phase_model.md` - controlled placement scans separated a template/back-end-sensitive case from a front-end-sensitive case; end probes did not move the latter.
- Confidence: proven
- Transfer: version-specific.

### CTX-04 Declaration and opened-file costs are measurable but approximate
- Symptom: adding a real declaration or distinct header shifts a listing counter although runtime behavior is unchanged.
- Mechanism: measured VC5 front-end numbering consumes values for declarations and distinct opened files. Names, comments, whitespace, and macros alone did not consume the calibrated values.
- Conditions: calibration examples were approximately: extern/typedef/global 1; prototype 1 plus named parameters; struct 7; enum 1 plus enumerators; function shell 4–5; local or source `if` label 1; float constant about 2. These are probe-specific estimates, not a stable formula.
- Action: reconstruct actual declarations and include graph from source/build evidence; do not translate estimates into padding.
- Experiment: add one construct at a time to a minimal scratch TU and compare listing deltas under the exact compiler build.
- Evidence: blood2_recon (private): `docs/vc5_phase_model.md` - calibrated construct costs; six distinct declaration names produced identical output; an empty distinct include cost one while a guarded repeat cost zero.
- Confidence: proven
- Transfer: version-specific.

### CTX-05 Automatic `/YX` PCH mode is part of code-generation context
- Symptom: many functions move after a build drops or changes automatic PCH handling although their bodies are unchanged.
- Mechanism: the measured MSDev `/YX` path changed the effective TU numbering/context; PCH recreation was TU-local in the captured rebuild.
- Conditions: observed in the Blood II IDE-style project flow, not established for every compiler driver or PCH layout.
- Action: reproduce the target project's actual PCH mode and include order before tuning a function. Do not toggle `/YX` as a source-level repair.
- Experiment: compile the same source, headers, flags, and PCH procedure with authentic `/YX` and without it; compare all functions and objects.
- Evidence: blood2_recon (private): `docs/object_residue.md` - Object strict bytes rose 87.46% to 90.53% and near-match bytes 84.38% to 86.85%, with regressions too; all 134 TUs recreated the PCH in that rebuild. `docs/toolchain.md` - `/YX` is a measured non-neutral build choice.
- Confidence: proven
- Transfer: flag-specific.

### CTX-06 Isolate compiler passes before blaming a VC5 service pack
- Symptom: one compiler package appears to fix a register or operand residue while another does not.
- Mechanism: front-end numbering changes can alter back-end decisions even when the c2 binary is held constant; package changes may affect more than one pass.
- Conditions: an RTM c1xx + SP3 c2 hybrid was tested; this does not prove every service-pack difference is a front-end-only effect.
- Action: hold source, flags, headers, linker, and one compiler pass fixed at a time. Compare listing/counter maps before interpreting a package rank.
- Experiment: use a small front-end/back-end isolation matrix on more than one TU, then repeat under the target project’s PCH context.
- Evidence: blood2_recon (private): `docs/object_residue.md` - RTM front end plus SP3 back end reproduced SP3 output plus a declaration-count shift in four TUs; 40/58 apparent other-backend-only functions were count-reproducible. With `/YX` held, no tested alternate c2 reproduced an SP3-missed residual.
- Confidence: supported
- Transfer: version-specific.
