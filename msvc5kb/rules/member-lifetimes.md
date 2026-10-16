# Member access, locals, and value lifetimes

### MEM-01 Repeated member reads can be the required source form
- Symptom: target code reloads a member through its object at each use while candidate code caches it in a local or keeps it live in a register.
- Mechanism: a source-level common-subexpression changes value lifetime, register pressure, frame decisions, and scheduling. Example: `if (p->ready) consume(p->ready);` exposes repeated member reads.
- Conditions: direct member forms were strict in seven measured Blood II functions; one separate case required retaining a computed local. This is not a rule that reloads are generally better.
- Action: try direct member access against a cached local only when the target shows repeated reloads and source evidence does not establish the cache.
- Experiment: change one real repeated read in a scratch TU, compile the owning TU, then compare the target and every peer function strictly.
- Evidence: blood2_recon (private): `docs/vc5_codegen_map.md` - direct member forms were strict in seven functions across Server and Client. SB1 check: a scratch `PlaySoundDS` cached-pointer variant changed the body from 128 to 112 bytes but remained a mismatch; its TU stayed at 21/27 strict functions. See `docs/MSVC5KB_VALIDATION.md`.
- Confidence: proven
- Transfer: project-specific; the exact source preference came from measured functions, and SB1 did not validate the cached form.

### MEM-02 Keep floating-point value lifetime and representation separate
- Symptom: float copies or helper arguments differ in x87 loads/stores, stack temporaries, or the return sequence.
- Mechanism: typed staging and bit-preserving transfers each reached strict Client bodies. The proposed mechanism is VC5 x87 temporary lifetime and register-stack scheduling; the histories do not prove every float mismatch has that cause.
- Conditions: three Client `/O2` histories reached exact output through float lifetime or bit-transfer forms. One bit-transfer match disturbed a previously strict Server result.
- Action: use bit-preserving transfer only for a true bit-pattern operation; for numeric values, try a narrowly scoped typed local at the original value boundary. Check `/O1` and `/O2` independently.
- Experiment: compile minimal numeric-float and bit-copy examples under both flags; inspect x87 sequence and compare full functions.
- Evidence: blood2_recon (private): `experiments/functions/si_GetPropReal__f78842644c.jsonl` - two bit-transfer and two local-pointer variants reached exact Client output while Server residues remained; `si_GetPropRotation__747480222d.jsonl` - staged float arguments reached exact Client output; `si_GetSoundDuration__3f1f5ead19.jsonl` - one of two lifetime forms reached exact Client output.
- Confidence: supported
- Transfer: flag-specific; numeric and bitwise semantics must remain separate.
