# Templates and COMDATs

### TMP-01 Template instances can be numbered after ordinary source declarations
- Symptom: a template member or inline copy changes after a declaration near the end of its owning TU.
- Mechanism: measured VC5 instantiated template members after parsing ordinary source, so instance numbering followed the TU's source declarations; instance decisions had broad periodic windows.
- Conditions: measured on VC5 SP3 template members/inline copies; other compiler drops are not confirmed.
- Action: compile the full owning TU and header set. A one-function harness cannot establish delayed-template context.
- Experiment: compare full-TU listings before/after one authentic declaration and record the instantiated member's counters and call shape.
- Evidence: blood2_recon (private): `docs/vc5_phase_model.md` - template instances followed ordinary source numbering and exhibited 256-wide decision windows.
- Confidence: proven
- Transfer: version-specific.

### TMP-02 Duplicate COMDAT bodies retain their own TU context until link selection
- Symptom: one linked template copy is strict while another object's duplicate body differs, or changing a duplicate's TU moves its peer functions.
- Mechanism: each COMDAT copy is compiled in its owning TU; temporary names and counter phase can differ before the linker discards or folds a copy.
- Conditions: measured with duplicate header/template COMDATs and file-local floating temporaries. A linked survivor does not reveal every discarded object body.
- Action: inspect each object copy and the linked survivor separately; do not alter a canonical function merely to make a discarded duplicate match.
- Experiment: disassemble each object copy with relocations, compare the linked copy, then vary only authentic owning-TU context.
- Evidence: blood2_recon (private): `docs/vc5_phase_model.md` - duplicate nonlinked copies had their own phase; a floating-temporary change changed protected peer status.
- Confidence: proven
- Transfer: version-specific.
