# Register, operand, and address order

### REG-01 Commutative x87 operand order can be a TU-context discriminator
- Symptom: an x87 add or multiply loads the same operands in reverse order while result, size, and control flow agree.
- Mechanism: a context-dependent VC5 tie-break can select either legal stack operand order; the exact heuristic is unknown.
- Conditions: measured in VC5 engine code and harness TUs. Do not generalize to noncommutative operations or assume that changing expression order is safe.
- Action: test real TU declarations and inline expansion context before rewriting the expression.
- Experiment: scan top/end/before positions over a bounded range and compare the exact x87 sequence and all peer functions.
- Evidence: blood2_recon (private): `docs/seed_pass/util.md` - two math functions with 23 and 11 differing bytes became byte-identical at some measured count contexts; `docs/object_residue.md` - same-size x87 operand order recurred as a TU-context residue.
- Confidence: proven
- Transfer: version-specific.

### REG-02 SIB base/index order can change without changing the address sum
- Symptom: one memory operand encodes the same two registers in reversed base/index roles and changes a few instruction bytes.
- Mechanism: a measured VC5 back-end temporary-number window selected a SIB order; another temporary created a second window.
- Conditions: observed in one `/O2` packet-building case with several inlined insertions. Not every LEA or memory operand follows this pattern.
- Action: first test authentic TU context; then compare the source pointer/index expression only if context scans do not explain it.
- Experiment: sweep a bounded count at top and end, recording exact function bytes and all SIB forms.
- Evidence: blood2_recon (private): `docs/vc5_phase_model.md` - three inlined sites shared one back-end window and a second temporary added another; `docs/object_residue.md` - operand-order windows differed by TU.
- Confidence: proven
- Transfer: version-specific.

### REG-03 Pointer spelling can change address formation and alias reloads
- Symptom: an indexed load uses a different base/index register or the target reloads a pointer where the candidate keeps it live.
- Mechanism: equivalent typed pointer expressions changed VC5 address formation and observed alias/live-range choices; no universal allocation rule was isolated.
- Conditions: strict Client candidates occurred in three Blood II histories; one array form that matched Client left Server one byte off.
- Action: compare direct typed base-plus-index, a real pointer local, and an evidence-backed reload boundary one at a time. Preserve observable aliasing.
- Experiment: compile two semantically equivalent access forms under `/O1` and `/O2`, inspect the memory operands, and compare strict bytes in both targets.
- Evidence: blood2_recon (private): `experiments/functions/con_PrintString__6366803e5a.jsonl` - pointer reload plus live count local reached exact Client output; `OnLoadWorldPacket__e32e00afb4.jsonl` - typed response access/lifetime reached exact Client output; `PopSmallest__a8d6b09930.jsonl` - direct array arithmetic matched Client while Server became one byte non-strict.
- Confidence: supported
- Transfer: general-VC5; evidence is sparse and flag-sensitive.

### REG-04 A real loop-invariant list head can change register allocation
- Symptom: the target carries a sentinel/head pointer across a scan while the candidate reloads or recomputes it at each loop test.
- Mechanism: the hypothesis is that an explicit local changes VC5's live range and register assignment around the loop test. Example: `Node *end = head; for (Node *p = first; p != end; p = p->next) { ... }` makes the sentinel lifetime explicit.
- Conditions: two list-search bodies matched in Client `/O2`; their Server `/O1` forms were already strict.
- Action: use an explicit typed head local only when source/debug evidence supports that lifetime; compare both flag configurations.
- Experiment: build otherwise equivalent loop forms with and without the real loop-invariant local, then compare complete functions and peers.
- Evidence: blood2_recon (private): `experiments/functions/sm_FindClient__55920410ca.jsonl` - 6 ledger rows including replays; `sm_FindClientByVirtualID__7416fad9f1.jsonl` - second body reached strict Client output. Two bodies matched across seven recorded rows.
- Confidence: supported
- Transfer: general-VC5; only two loop shapes are measured.
