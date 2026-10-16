# Exception handling, destructors, and scope

### EH-01 `/GX` is needed when the target emits C++ unwind paths
- Symptom: cleanup calls, EH funclets, or exception-handler references are absent despite similar mainline instructions.
- Mechanism: VC5 `/GX` enabled the measured C++ exception frames and unwind code; function-body comparisons alone can omit generated cleanup sections.
- Conditions: measured on VC5 SP3 static-library code. The evidence confirms frames/funclets and array-unwind support, not every EH state-table encoding.
- Action: match `/GX` first, then recover real try/catch and destructor scope. Compare object funclet sections as well as linked code.
- Experiment: compile a minimal throwing call with a local destructor with `/GX` on/off; inspect frame handlers, unwind references, and funclet sections.
- Evidence: blood2_recon (private): `docs/seed_pass/rezmgr.md` - masked-exact seed counts `/O1` 4, `/O2` 31, `/GX /O2` 62; measured C++ EH frames/funclets required `/GX`. `docs/seed_pass/util.md` - `/GX` required for one EH frame and vector-constructor iterator.
- Confidence: supported
- Transfer: flag-specific.

### EH-02 Lexical fall-through and early return can generate different cleanup tests
- Symptom: target keeps a zeroing instruction and sends an early branch to a destructor test, while candidate returns directly to the epilogue.
- Mechanism: an `if` block that falls through to scope end joins cleanup paths; an explicit return can let VC5 fold a cleanup test. Example: with a destructor-bearing local in scope, `if (failed) { log_error(); }` and `if (failed) return;` have different cleanup joins.
- Conditions: one VC5 SP3 `/O2` Client function with a nontrivial reference wrapper and destructor. Trivial locals may not show it.
- Action: recover the original lexical block and return shape from source/debug evidence before changing cleanup order.
- Experiment: compare a scoped fall-through form with an early-return form at the same declaration position; verify the destructor branch and all function bytes.
- Evidence: blood2_recon (private): `docs/vc5_phase_model.md` - fall-through matched exactly; return form failed at three declaration placements and removed the test and zeroing instruction.
- Confidence: proven
- Transfer: flag-specific.
