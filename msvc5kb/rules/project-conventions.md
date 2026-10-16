# Project source conventions

These rules describe source/header facts from one project. They are not MSVC language rules and should not be transferred without independent evidence.

### SRC-01 Wrapper call sites can explain an inlining residue
- Symptom: the target keeps early typed helper calls out of line but expands later sites, while the candidate calls a generic template directly.
- Mechanism: VC5's caller-local inline budget responds to nested sites; wrapper names and call sites are project conventions.
- Conditions: Blood II packet/container wrappers were inferred from debug or sibling call lists. Broad conversion regressed strict peers in one history.
- Action: establish wrapper use from source/debug/sibling evidence before changing call sites; do not transfer wrapper names or call patterns to another project.
- Experiment: compare native/candidate call lists, substitute only independently evidenced wrapper calls, and rebuild all peer targets.
- Evidence: blood2_recon (private): `docs/vc5_phase_model.md` - 16 Client rows (18,352 bytes) had the native-call/candidate-inline signature; one wrapper-supported case became exact while broad conversion lost strict peers. `docs/seed_pass/managers.md` - six helper families were out-of-line COMDATs under `/O1` and inline under `/O2`.
- Confidence: supported
- Transfer: project-specific.

### SRC-02 Expand a vector macro only when the component statement form is evidenced
- Symptom: a project vector macro produces a different load/store or x87 sequence from the target.
- Mechanism: measured output followed the expanded statement/temporary sequence, not the macro's name. A macro such as `SCALE(v,s)` may expand to three component statements. `VEC_*` is a project convention, not an MSVC feature.
- Conditions: 24 candidate rows across eight histories included one strict Client output using explicit scaled-vector statements; a second explicit expansion remained 122 bytes off in Server and 2,249 bytes off in Client. Four dot-term permutations retained a 10-byte Server residue.
- Action: inspect the actual macro and test one semantically equivalent expansion at a time. Do not assume scalar expansion is generally better.
- Experiment: compile the macro and one independently written equivalent component form under the target flags; compare both image configurations and float semantics.
- Evidence: blood2_recon (private): `experiments/functions/si_CastRay__b670c21e17.jsonl` - component statements reached exact Client output; `si_GetAttachedModelNodeTransform__820cedde85.jsonl` - another expansion stayed far off; `experiments/functions/AddAndMoveBack__4e64b68a2c.jsonl` - four dot-term orders preserved the same 10-byte mismatch.
- Confidence: supported
- Transfer: project-specific.

### SRC-03 Verify the real helper signature before changing caller expressions
- Symptom: a caller is near-match around return-value handling or type-dependent instructions despite similar apparent values.
- Mechanism: a verified helper return type changed the caller output; this is declaration/type context, not a VC5 idiom.
- Conditions: one Blood II Client `/O2` packet-handler history; the verified signature came from that project's sound-manager interface.
- Action: establish the actual declaration and return type from project evidence before trying expression spellings. Do not transfer API names or types.
- Experiment: compile the same caller with the evidenced signature and the prior declaration; compare call/result instructions and full function bytes.
- Evidence: blood2_recon (private): `experiments/functions/OnPreloadListPacket__3a1df7c98e.jsonl` - two return-type and four declaration-context variants reached exact Client output.
- Confidence: supported
- Transfer: project-specific.
