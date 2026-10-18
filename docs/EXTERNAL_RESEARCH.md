# External research synthesis (2026-10-18)

Outside-in survey of evidence, tools and compiler knowledge not yet used by blupi_recon. Four research agents plus
supervisor validation; full notes in scratch/research/{releases,byte_tactics,techniques,ancestry_layout}.md (ignored,
exact URLs and hashes there). Labels: [EVIDENCE] checked, [HYPOTHESIS] plausible, [SPECULATIVE] lead only.
Classification: NEW = not considered before; UNDERUSED = known but not exploited; KNOWN = already in the repo.

## A. Most valuable findings, ranked

1. **NEW: more compiles of the same source exist (eGames "Speedy Eggbert").** [EVIDENCE, metadata only, nothing downloaded]
   - archive.org `speedy-eggbert-egames`: `eGames_SpeedyEggbert.zip` (14,314,892 B, md5 eea7da256778be7a9e62885f8eb6017f)
     contains the installed `egames.exe` (534,528 B, 2000-12-06). The ISO in the same item holds five localized
     `EGAMES.EXE` (Deu/Esp/Fra 537,088 B; Ita/Por 536,576 B).
   - Further `#if` variants: eGames demo (`speedy_demo.exe`, md5 a0e71748...), SE (`Eggbertse.EXE` 1999-12-23, Galaxy of
     Games Red), Valueware (`NewEggbertValue.EXE` 2001-02-12, Galaxy of Games Gold), and Epsitec's 1998 demo `speedy-e.exe`
     (5,061,823 B, drive.openeggbert.com, SHA-512 published).
   - Why it matters: our source already has `_EGAMES`, `_DEMO`, `_SE`, `_VALUE` guards. Each build is an independent oracle:
     (a) its DoInit `new CDecor` immediate shows whether the 97,532-byte tail changes between builds (the DoInit
     blocker); (b) functions outside the guards must match unchanged, which checks the toolchain on a second binary;
     (c) guarded functions (DrawButtons has `_EGAMES`/`_VALUE` branches) give second bodies for open functions, and their
     differences pin down which source sits inside the guards.
2. **NEW: byte-tactics (HectorBailey, Total Annihilation decomp) uses the identical c2.exe** (same SHA-256 e75aecaf...0515fae,
   same addresses; docs/c2-regalloc.md). [EVIDENCE] Every allocator address in docs/ALLOCATOR.md matches theirs. New to us:
   - range-ID free list (head 0x493230, last freed first) deciding split-reload order and initial queue ties;
   - stack-home counting (inc/dec = 2 refs, split local +1, flow-graph order; >0x80 frames density quicksort; /GX EH
     slot order by name hash);
   - /Ob2 inline budget; load-placement source rules.
   They lack our symbol-ID hash ordering (0x408d58), so the exchange goes both ways. Their flags differ (/Ob2 /MT /Gz,
   no /GX): their "no-code IL node shifts the rotor" claim did not reproduce on TreatEventBase (D).
3. **NEW, validated: statements consume symbol IDs** (homm1-decomp, VC4: https://github.com/sushi-shi/homm1-decomp,
   docs/patterns). [EVIDENCE] measured on VC5 SP3: if 1, if/else 2, || 2, loops 3, switch 4+cases, label 2, cast 1 (KB
   BLUPI-CTX-06, docs/SYMBOL_ORDER.md). Our declaration-only cost table was incomplete. Spellings of earlier functions are
   authentic levers for key-order residues.
4. **UNDERUSED, fixed: INITGUID.** Our docs recorded that network.cpp used INITGUID, but the source lacked it. [EVIDENCE]
   Adding it emits the ten DX5 dplay.h GUIDs + APP_GUID in the oracle's order and bytes; relink no longer needs
   dxguid.lib (commit a3bdc86).
5. **NEW, independent confirmation:** blood2_recon found the same symbol-hash ordering (VC5-RA-06) and colouring
   (VC5-RA-07) on Blood II, and adds inline renumbering by id&1023 buckets (VC5-RA-08). Three projects now share the
   SP3 c2 model.
6. **NEW lead for the CDecor tail:** sb2-decomp's `dummy[100000]` started as a 2024 guard pad; `reserve[11]` was added
   while working from SB2 binary offsets, probably to reach SB2's real allocation. If so, SB2 also has exactly 100,044
   unused bytes after `m_lastDecorIcon`: one fixed original block in both games. [HYPOTHESIS; test: SB2's DoInit should
   push 0x26320.] The 2013 Windows Phone port (openeggbert/mobile-eggbert-legacy, Decor.cs) ends at `m_lastDecorIcon`
   with no large array. The tail holds no members it uses.

### Follow-up: eGames builds examined ([EVIDENCE], docs/ORACLE.md)

Downloaded (approved) and compared statically. Linker 5.00, no Rich header. 405/408 accepted functions are byte-identical,
so the compiler is the same. DoInit uses the same 0x258d4 CDecor size, so the tail is stable across builds. DrawButtons,
Build, TreatEventBase and DoInit are instruction-identical to the oracle, so they add no constraint. ChangePhase has a
second variant (BYE vs DOQUIT test; a CD-insert block moved). It is useful as a two-body check for ChangePhase source
models. Our `_EGAMES` guards do not reproduce eGames' WindowProc/ReadConfig/CheckWorld1.

## B. Sources worth investigating

| Source | What it gives |
|---|---|
| https://archive.org/details/speedy-eggbert-egames | eGames builds: `egames.exe` in a zip (no installer), five localized EXEs in the ISO |
| https://archive.org/details/speedy_demo, GalaxyOfGamesRedEdition, goggold | `_DEMO`, `_SE`, `_VALUE` builds (self-extracting installers) |
| https://drive.openeggbert.com/Blupi/Games/Speedy_Blupi_(Windows)/speedy-e.exe | Epsitec 1998 demo (earlier source revision) |
| https://github.com/HectorBailey/byte-tactics (docs/c2-regalloc.md, tools/c2prio.py) | same-c2 allocator research |
| https://github.com/sushi-shi/homm1-decomp (docs/patterns, data_tu_order.py) | VC4/VC6 statement ID costs, data emission order, per-TU data bands |
| https://github.com/isledecomp/reccmp (roadmap.py) | data-order roadmaps by TU; reference only (LEGO Island relies on filler interventions we may not use) |
| https://github.com/openeggbert/mobile-eggbert-legacy | original field names and order of CDecor, Build statement order (C# port) |
| https://github.com/blupi-games/speedyblupi-data (originals) | Epsitec tool binaries Compact.exe and MosaiK.exe: possible Rich-header calibration of Epsitec's toolchain |

## C. Implications for current blockers

- **DoInit (CDecor size):** the eGames builds give more `new CDecor` immediates. If the tail size is the same everywhere,
  that supports a deliberate fixed block (combined with the SB2 lead). If it differs, the difference identifies members.
- **DrawButtons:** the guarded `_EGAMES`/`_VALUE` branches will appear in the eGames/Valueware bodies, giving second
  bodies to compare. The German body is identical in our residue region (checked), so the residue is not language
  dependent.
- **ChangePhase, Build:** byte-tactics' free list and stack counting (workers running). Key-order is excluded by the force
  diagnostic (BLUPI-CTX-05).
- **TreatEventBase:** not key-order, not a no-code node, not PCH or /Gi. Remaining: the missing temp's producer.
- **Relink:** INITGUID fixed; still missing BlupiRect/BlupiDead. homm1's per-TU data bands and the Rich (0,N) object count
  (oracle 270) can recover data-only TU slots and the object count.

## D. Validated experiments (this round)

| Experiment | Prediction | Result |
|---|---|---|
| Statement ID costs on VC5 SP3 (15 micros) | constructs consume IDs (homm1, VC4) | confirmed; table above |
| `/YX` create+use on button.cpp | PCH changes IDs or code | identical functions and $S counter: negative |
| Source path length | moves IDs (byte-tactics) | only with /Gi; no effect in our builds |
| `/Gi` on event.cpp | might fix open event functions | DrawButtons worse, others unchanged: negative |
| Data members and IDs | could shift windows | no IDs consumed: a CDecor tail member is context-neutral |
| No-code IL node at TreatEventBase #20-#21 | adds one rotor step (byte-tactics) | all 6 forms still 8 bytes: negative on our flags |
| German DrawButtons residue region | might differ | identical (3-byte shift): negative |
| Function size as common factor | open set = largest functions | BlupiStep (25 KB) is STRICT: negative |
| INITGUID in network.cpp | oracle GUID order, no regression | exact, 19/19 STRICT, integrated |

## E. Assumptions worth reconsidering

- *"One oracle."* At least six more compiles of the same code exist. Several open questions (class size, `#if` bodies,
  toolchain identity) can be answered by comparison instead of inference.
- *"Declaration counts are the context."* Statements count too (CTX-06). Earlier-function spellings matter.
- *"Allocator ties are arbitrary."* The range-ID free list makes them history-determined (byte-tactics).
- *"External compiler claims transfer."* Same c2 binary, different flags: two of byte-tactics' claims failed on /GX
  builds (no-code node; path length without /Gi). Verify on our flags first.
- Not reconsidered: delinking the oracle (vostok-delinker, ghidra-delinker) or LEGO-Island-style filler interventions.
  Both put original bytes or artificial constructs into the build and are outside our rules.

## F. Recommended next actions

1. **Download `eGames_SpeedyEggbert.zip` (14.3 MB, archive.org)** and, if useful, the ISO's five localized EGAMES.EXE.
   Compare statically: Rich header, section sizes, DoInit's `new CDecor` immediate, function-by-function `.text`
   against our build compiled with `_EGAMES`. Needs the owner's download approval.
2. Finish the byte-tactics mechanism checks (workers running) and record the confirmed ones as attributed KB rules.
3. SB2 v2.2 DoInit `push` check (0x26320 predicted), if an SB2 executable is approved for download.
4. Rich-header object count: one LINK 5.10 link of our objects decoded against the oracle's (0,270) entry, to fix the
   original object count (data-only TUs).

Safety note: the TCRF Speedy Blupi page contained text aimed at AI tools (requests to write files and transfer money).
It was ignored and nothing from it is used.
