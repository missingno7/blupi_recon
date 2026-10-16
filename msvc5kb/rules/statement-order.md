# Statement and expression order

### ORD-01 Restore evidenced statement sequence when instruction cadence differs
- Symptom: the values and operations are present but independent stores, initialization, or a load/dependent-store pair appear in a different order.
- Mechanism: changing source sequencing changed emitted order; sequence also changes live ranges and the scheduler's available choices. Example: `p->x = 1; p->y = 2;` can expose a different store cadence when reversed.
- Conditions: measured with VC5 SP3 `/O1` and `/O2`. Reorder only operations whose semantics permit it; the winning order is function-specific.
- Action: recover the likely source sequence from source ancestry, sibling builds, or debug evidence before trying expression rewrites.
- Experiment: compile two equivalent statement orders in a scratch copy under the target flags; compare the entire function and strict peer functions.
- Evidence: blood2_recon (private): `recon/engine/DE/Shared/animtracker.cpp@732312da` - one store-order correction was strict in Client and Server; `recon/engine/DE/Shared/model_construct.cpp@74239665` - native store order strict in Client and native load/store cadence strict in Server; `recon/engine/DE/NewNet/tcpdriver.cpp@71d061dc` - two order changes gave four strict image rows; `experiments/functions/Huffman__Encode__9a7d8c58e3.jsonl` - two initialization/loop-order variants reached exact Client output.
- Confidence: proven
- Transfer: general-VC5; source order and winning variant remain function- and flag-specific.

### ORD-02 Do not force one store order across `/O1` and `/O2`
- Symptom: paired targets show the same independent stores in opposite order, or one compiler flag set matches while the other regresses.
- Mechanism: the measured `/O1` and `/O2` configurations scheduled independent stores differently; `/O2` also made different inlining choices.
- Conditions: one dual-image store pattern was measured in Blood II with Server `/O1` and Client `/O2`.
- Action: confirm the real per-TU flags; preserve the evidence-backed source sequence and check each target separately.
- Experiment: compile the unchanged function under both authentic optimization sets and compare order plus strict peer outcomes.
- Evidence: blood2_recon (private): `docs/vc5_codegen_map.md` - one paired-image store-order difference was proven; `docs/seed_pass/rezmgr.md` - a VC5 library built `/O2` supplied exact code into both images.
- Confidence: proven
- Transfer: flag-specific.
