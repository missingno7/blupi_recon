# Linker behavior versus compiler output

### LNK-01 COMDAT folding happens after compiler inlining and object generation
- Symptom: identical functions share an RVA or the surviving body appears at a different address in two linked images.
- Mechanism: the linker selects/folds eligible COMDAT sections according to link inputs and selection rules; this does not mean the compiler inlined the function.
- Conditions: observed with VC5 `/Gy` COMDAT objects and the Blood II linker family. Folding applies only to eligible COMDAT classes.
- Action: compare object bodies first; then inspect symbols, relocations, and linked RVAs separately.
- Experiment: link the same verified objects in controlled orders and compare folded symbols and final locations.
- Evidence: blood2_recon (private): `docs/seed_pass/rezmgr.md` - 25 symbols shared 10 Server addresses; several folds selected different copies in Server and Client.
- Confidence: proven
- Transfer: project-specific.

### LNK-02 PE metadata, imports, thunks, and section layout are linker evidence
- Symptom: function bodies match while Rich data, import/IAT order, thunk RVAs, or section layout differs.
- Mechanism: these bytes are downstream linker outputs; linker build and object/library order can change them independently of c2 instruction selection.
- Conditions: Blood II exporting modules distinguished 5.10.7287 from local 5.10.7303 via Rich/order evidence. SB1's PE records linker 5.10; controlled links support SP3 5.10.7303 as the best available candidate but do not identify the exact 5.10 patch.
- Action: freeze object bytes and link inputs before changing compiler/source. Report function code and image-layout equality separately.
- Experiment: link identical objects with candidate linkers and orders; compare Rich entries, imports, thunks, sections, and function bytes as separate measurements.
- Evidence: blood2_recon (private): `docs/toolchain.md` and `docs/toolchain_acquisition_log.md` - original 5.10.7287 differed from local 7303 in Rich/import-thunk ordering; 7287 was unavailable while 7168, 7255, and 7303 were tested. blupi_recon: `docs/TOOLCHAIN.md` - SB1 has PE linker field 5.10; 5.10.7303 is best available, exact patch remains open.
- Confidence: proven
- Transfer: project-specific.
