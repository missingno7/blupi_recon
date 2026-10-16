# Negative results

### NEG-01 Swapping local declaration order did not predict stack-slot order
- Symptom: a frame-slot mismatch invites broad reordering of existing local declarations.
- Mechanism: measured VC5 slot assignment was not a simple source declaration-order walk. The evidence does not establish a universal lifetime rule.
- Conditions: one engine receive function tested three declaration orders with identical slot assignment; a broader separate search had one strict Server output among many candidate orders, while the paired Client result regressed.
- Action: stop trying name/order permutations. Investigate real value lifetime, aliasing, initialization, and register pressure instead.
- Experiment: compare a few source-evidenced orders of existing used locals under every target flag set; close the family if slots and strict status do not improve.
- Evidence: blood2_recon (private): `docs/vc5_codegen_map.md` - three declaration orders left the tested slot assignment unchanged. `experiments/functions/FitIntoPushawayPlanes__da44e980af.jsonl` - 35 initial orders and 32 refinements produced one distinct strict Server result but broke an exact Client form by five bytes. `StairStepCollide2__38066f35cd.jsonl` - six orders and eight refinements left the recorded residues.
- Confidence: negative
- Transfer: project-specific.

### NEG-02 Cleanup and string-copy spelling searches did not close one initializer residue
- Symptom: a constructor mismatch suggests changing cleanup registration, object lifetime, or equivalent string-copy spelling.
- Mechanism: the tested candidate forms changed output but none reached strict equality; no general VC5 rule for destructor registration or literal pooling follows.
- Conditions: one Client `/O2` initializer history; literal pooling remains open.
- Action: stop repeating the same forms unless new source or call-context evidence appears. Do not infer string pooling from array-copy substitutions.
- Experiment: reopen only with a falsifiable instruction-level hypothesis and change one lifetime/copy form per build.
- Evidence: blood2_recon (private): `experiments/functions/cm_Init__8fca2d76bd.jsonl` - 15 cleanup/lifetime/string-copy candidates and zero strict target outputs.
- Confidence: negative
- Transfer: version-specific evidence for that function.

### NEG-03 Declaration padding is not an acceptable code-generation repair
- Symptom: an operand/address mismatch tempts a dummy declaration or local to move a counter or alignment.
- Mechanism: one measured inlined-container residue was attributed to the actual base/index order inside the inline operation; the history ruled out padding as a route for that case. The ledger did not run a broad padding matrix.
- Conditions: evidence is limited to the cited inlined-array-container residue; the no-padding rule is also a matching-project constraint.
- Action: recover real caller/type/expression context. Keep artificial declarations, unused locals, and padding out of canonical source; use only scratch diagnostic probes.
- Experiment: do not compile padding variants. Test only source-supported caller/type forms and require strict full-function equality.
- Evidence: blood2_recon (private): `recon/engine/units/managers/clmessagehelper.cpp@8d9aa60e` - declaration padding was ruled out for one base/index residue; `docs/calibration_audit.md@9de6faf5` - calibration excluded filler as byte forcing.
- Confidence: negative
- Transfer: project-specific evidence; repository hard rule is broader than this experiment.
