# Inlining and budget

### INL-01 VC5 `/O2` makes caller-local inline decisions
- Symptom: a caller expands early helper calls but leaves later calls out of line; another caller makes different choices.
- Mechanism: measured VC5 spends an inline budget per caller; expansion size and nested bodies consume that budget.
- Conditions: VC5 SP3 `/O2` micro-TU evidence; the observed budget fit about six small template-helper expansions, not a universal call count.
- Action: list call sites in source order and inspect the native inline/call pattern before changing helper definitions.
- Experiment: compile one caller with repeated calls to a small helper; vary call count, callee size, and nesting, and compare which sites remain calls.
- Evidence: blood2_recon (private): `docs/vc5_phase_model.md` - a micro-TU reproduced caller-local choices; ten preceding callers and 75 counter-phase shifts did not change them.
- Confidence: proven
- Transfer: version-specific.

### INL-02 Nested sites inside an expanded wrapper can be decided later and in reverse order
- Symptom: target expands a helper at later wrapper sites but keeps earlier wrapper sites as calls, opposite the candidate's direct-call pattern.
- Mechanism: direct sites were considered in source order; sites inside an already expanded wrapper were considered later and in reverse textual order. Example: `push_value(i)` can add a nested `insert` call site that direct `insert` calls do not have.
- Conditions: measured for VC5 SP3 `/O2` in one inline-wrapper micro-TU.
- Action: restore an evidence-backed wrapper layer at the original call sites; do not add inline/noinline decorations to imitate a call list.
- Experiment: compare eight direct sites with eight wrapper sites; add one nested expansion at a time and record the remaining calls.
- Evidence: blood2_recon (private): `docs/vc5_phase_model.md` - direct and wrapper sequences showed the reversed nested decision order and a per-caller limit.
- Confidence: proven
- Transfer: version-specific.

### INL-04 `/O1` and `/O2` change inline presence and may change scheduling
- Symptom: one image has an out-of-line helper COMDAT and another has the body expanded; independent stores can also move.
- Mechanism: measured VC5 `/O1` and `/O2` selected different optimization and inlining behavior.
- Conditions: Blood II Server used `/O1`, Client `/O2`; separate libraries could use their own flags.
- Action: confirm per-TU flags. Build the identical source under each real flag set before adjusting helper or statement form.
- Experiment: compare helper calls, stores, and strict peer functions under `/O1` and `/O2` with all other inputs fixed.
- Evidence: blood2_recon (private): `docs/vc5_codegen_map.md` - measured `/O1` versus `/O2` store order; `docs/seed_pass/rezmgr.md` - a separately flagged VC5 library matched into both images.
- Confidence: proven
- Transfer: flag-specific.
