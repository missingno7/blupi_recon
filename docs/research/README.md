# Research index

This page indexes the preserved research: compiler studies, translation-unit reconstructions, sweeps,
worker reports, linker and whole-image investigations, and edition notes. Every document stays at its
original path, and nothing here was moved or deleted.

- **The numbers in these documents are snapshots** from the date of each investigation. Several
  record totals, policies or plans that were later superseded. Current figures are only in
  [../STATUS.md](../STATUS.md).
- **Binding rules** are in [AGENTS.md](../../AGENTS.md).
- **Working procedures** are in [BUILDING.md](../BUILDING.md) and [VERIFYING.md](../VERIFYING.md).

The labels in the tables are used as follows.

| Label | Meaning |
|---|---|
| reference | still describes current tools or facts |
| ledger | a living record of open and closed questions |
| snapshot | dated results, not current status |
| superseded | its policy or plan was replaced; it is kept for evidence |

## Whole image, linker and data layout

| Document | Content | Label |
|---|---|---|
| [SB1_CLOSURE.md](../SB1_CLOSURE.md) | SB1 whole-image closure: mechanisms, dead imports, BSS, TU partition (underdetermined) | ledger |
| [HISTLINK.md](../HISTLINK.md) | Historical LINK host: MSVCRT qsort, clock, launcher | reference; its SP3-only CLI text predates the RTM LINK/CVTRES modes |
| [LINK_ICF.md](../LINK_ICF.md), [LINK_CONTEXT.md](../LINK_CONTEXT.md), [LAYOUT_ORDER.md](../LAYOUT_ORDER.md) | ICF/COMDAT model, Rich count, link blockers, function order | snapshot |
| [RELINK.md](../RELINK.md), [DATA_LAYOUT.md](../DATA_LAYOUT.md), [DATA_LAYOUT_R2.md](../DATA_LAYOUT_R2.md) | Early SB1 full-link and data/BSS investigations | snapshot, later superseded figures |
| [experiments/FIFO_LIBORDER.md](../experiments/FIFO_LIBORDER.md) | An experimental library-order fit that is not adopted | snapshot (negative witness) |
| Branch `sb1-histlink`, `evidence/sb1-histlink/` | A 412/412 experimental linker-input candidate | evidence only. **Not** recovered source; not integrated (SB1_CLOSURE.md) |

## Compiler (VC5) behaviour

| Document | Content | Label |
|---|---|---|
| [COMPILER_STUDY.md](../COMPILER_STUDY.md) | Residue causes, source forms, c2 tracing rounds | snapshot |
| [ALLOCATOR.md](../ALLOCATOR.md), [SCHEDULER.md](../SCHEDULER.md), [SYMBOL_ORDER.md](../SYMBOL_ORDER.md), [C1_IL.md](../C1_IL.md) | Models of the SP3 allocator, scheduler, symbol-ID operand order and front-end IL | reference models |
| [LOWERING.md](../LOWERING.md), [SOLSIB.md](../SOLSIB.md) | POINT/constant lowering; event SIB allocation | snapshot |
| [RESIDUAL_CAUSES.md](../RESIDUAL_CAUSES.md) | SB1 remaining functions: tested forms and closure tests | snapshot |
| [CONTEXT_REVISIT.md](../CONTEXT_REVISIT.md), [BLOCKER_CHECKS.md](../BLOCKER_CHECKS.md) | Wider-context negatives; compiler-host checks; 2026-10-19 relink receipt | snapshot |
| [STEERING.md](../STEERING.md), `steering/` | A compiler-steering demonstration | snapshot. Never canonical source |
| [MSVC5KB_VALIDATION.md](../MSVC5KB_VALIDATION.md), [msvc5kb/README.md](../../msvc5kb/README.md) | Rule validations; the live knowledge base | KB = reference |
| [BLOOD2_TRANSFER.md](../BLOOD2_TRANSFER.md) | Findings handed to the Blood II project | snapshot |
| [PLANET_ORACLE.md](../PLANET_ORACLE.md) | Planet Blupi as an independent compiler oracle (RTM/SP3 fingerprint) | snapshot |
| [EXTERNAL_RESEARCH.md](../EXTERNAL_RESEARCH.md) | Survey of external compiler and ancestry sources | snapshot |

## Translation-unit reconstructions (SB1)

| Document | Content | Label |
|---|---|---|
| [BLUPI_TU.md](../BLUPI_TU.md), [DECBLOCK_TU.md](../DECBLOCK_TU.md), [DECOR_TU.md](../DECOR_TU.md), [EVENT_TU.md](../EVENT_TU.md), [NETWAVE_TU.md](../NETWAVE_TU.md) | Per-TU candidates and residues | snapshot |
| [CORE_CLUSTER.md](../CORE_CLUSTER.md), [UI_CLUSTER.md](../UI_CLUSTER.md), [ENGINE_PROBE.md](../ENGINE_PROBE.md), [BIGBLUPI.md](../BIGBLUPI.md) | Planet-Blupi-derived clusters, UI TUs, engine probes, BlupiStep | snapshot |
| [HEADERS.md](../HEADERS.md), [LAYOUT_DEBT.md](../LAYOUT_DEBT.md) | Header context experiments; the resolved `decor.h` layout | snapshot |
| [TU_MAP.md](../TU_MAP.md) | Function-to-TU attribution (the CSV under `oracle/` is canonical) | reference; counts are a snapshot |

## Sweeps and worker reports

| Document | Label |
|---|---|
| [SWEEP_MIDA.md](../SWEEP_MIDA.md), [SWEEP_RESWEEP.md](../SWEEP_RESWEEP.md), [SWEEP_SWEEPA.md](../SWEEP_SWEEPA.md), [SWEEP_SWEEPB.md](../SWEEP_SWEEPB.md) | snapshot |
| [SENIOR_SOL_BUILD.md](../SENIOR_SOL_BUILD.md), [_DECBLOCK](../SENIOR_SOL_DECBLOCK.md), [_DECBLUPI](../SENIOR_SOL_DECBLUPI.md), [_DECDESIGN](../SENIOR_SOL_DECDESIGN.md), [_DECDESIGN_2](../SENIOR_SOL_DECDESIGN_2.md), [_DECMOVE](../SENIOR_SOL_DECMOVE.md), [_DECOR](../SENIOR_SOL_DECOR.md), [_DECOR3](../SENIOR_SOL_DECOR3.md), [_EVENT](../SENIOR_SOL_EVENT.md), [_LEFTOVERS](../SENIOR_SOL_LEFTOVERS.md), [_SMALL](../SENIOR_SOL_SMALL.md), [_TREAT](../SENIOR_SOL_TREAT.md) | snapshot (one-time worker reports) |
| [LUNA_LEFTOVERS.md](../LUNA_LEFTOVERS.md) | snapshot |

## Targets, editions and ancestry

| Document | Content | Label |
|---|---|---|
| [MULTI_GAME.md](../MULTI_GAME.md) | Target model, edition/language switches, source rules | reference; its status table and SB2 bring-up plan are superseded |
| [EGGBERT.md](../EGGBERT.md), [GERMAN_BUILD.md](../GERMAN_BUILD.md) | Eggbert 1 edition and language deltas; the English/German comparison | reference; its byte table is a snapshot |
| [SB1_DEMO.md](../SB1_DEMO.md), [oracle/sb1-blupi-demo/README.md](../../oracle/sb1-blupi-demo/README.md) | June 1998 demo: policy, inventory, `_DEMO` arms | reference (an evidence-only target) |
| [SB2.md](../SB2.md), [games/sb2/src/PROVENANCE.md](../../games/sb2/src/PROVENANCE.md) | SB2 matching, 20 research rounds, edition overlay | ledger. Rounds before 18, the 2026-10-23 edition policy and the post-overlay table are superseded |
| [ORACLE.md](../ORACLE.md), [DECOMP_PILOT.md](../DECOMP_PILOT.md) | Primary oracle inventory; seeded Ghidra workflow | reference |
| [ANCESTRY_PLANETBLUPI.md](../ANCESTRY_PLANETBLUPI.md), [ANCESTRY_SB2.md](../ANCESTRY_SB2.md), [src/PROVENANCE.md](../../src/PROVENANCE.md) | Lineage of the reconstructed sources | reference |

## Tools, build and project history

| Document | Content | Label |
|---|---|---|
| [WORKFLOW.md](../WORKFLOW.md), [MATCHING.md](../MATCHING.md), [PERMUTER.md](../PERMUTER.md) | Comparator internals, invocation, permuter | reference |
| [TOOLCHAIN.md](../TOOLCHAIN.md), [BUILD_PATH.md](../BUILD_PATH.md), [RESOURCES.md](../RESOURCES.md) | Toolchain fingerprints, `__FILE__` path, resource reconstruction | reference |
| [STANDALONE.md](../STANDALONE.md), [RUNTIME.md](../RUNTIME.md) | The 2026-10-19 source-build baseline and certificates; the cnc-ddraw smoke test | reference; build commands moved to BUILDING.md |
| [RECOVERY_MILESTONE.md](../RECOVERY_MILESTONE.md) | 2026-10-19 cleanup and the `build/recovery/research.zip` archive | snapshot. The archive is ignored and is the only copy: back it up |
| [SB1_STATUS_2026-10-19.md](SB1_STATUS_2026-10-19.md) | The former `docs/STATUS.md` (SB1 only), unchanged apart from links | snapshot |
| [CONSOLIDATION_PLAN.md](../CONSOLIDATION_PLAN.md) | The 2026-10-25 consolidation plan and its stage status | ledger |

## Port research

| Document | Content | Label |
|---|---|---|
| [port/docs/ORACLE.md](../../port/docs/ORACLE.md), [port/oracle/SB1_SAVE.md](../../port/oracle/SB1_SAVE.md) | Cross-build display oracle; save interchange receipts | reference |
| [port/docs/FIDELITY.md](../../port/docs/FIDELITY.md), `port/provenance.json` | Historical-to-port provenance map and its gate | reference |
| [port/docs/ROADMAP.md](../../port/docs/ROADMAP.md), [port/docs/SB2_PORT.md](../../port/docs/SB2_PORT.md) | Port milestones; the SB2 port log | ledger; early milestone totals are snapshots |
