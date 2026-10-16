# Seeded Ghidra pseudocode pilot

## Exporter

**[EVIDENCE]** `tools/decomp.py` imports the primary oracle into a cached Ghidra 12.1 project at `build/decomp/project`, seeds all 579 starts from `oracle/functions.csv`, then writes 20 compact TU files under `build/decomp/tu/`. The function headers carry RVA, Ghidra body-byte count, TU, `best_source`, map status, any accepted strict symbol, and the calling-convention seed. `HYP_` names and source-map assignments remain visibly tentative; accepted source symbols use `STRICT_` names. The executable's analyzed import names are retained. No guessed structure definitions were applied.

Run a full seed/export pass with:

```powershell
.\.pyenv\python.exe tools\decomp.py --force
```

After export, `tools/decomp.py 0x5e80` or `tools/decomp.py CButton::TreatEvent` prints one cached function. The Python and Java files total 213 lines. The completed headless log reports `seeded=579 exported=579 TU files=20`.

**[EVIDENCE: prior Hercules findings supplied for this pilot]** Ghidra 12.1 previously gave useful bodies, callees, and references, but unreliable prototypes and calling conventions (including omitted `this` and cdecl mislabeled stdcall). It missed data-only roots until seeded and its raw output was too large to use directly. Pseudocode helped recover state and structure; source candidates were re-derived from instructions and checked with VC5 rather than copied from pseudocode.

## Measured pilot

All compile probes use VC5 SP3 `/W3 /GX /O2 /ML /Gd`, with `TMP` and `TEMP` in `build/tmp`; SB2 probes also use the SB2 source, BASS, and DX6 include directories. Objects, JSON results, and logs are under `build/decomp/`; candidate source is under `scratch/decomp/candidate/`.

**[EVIDENCE]** The requested map contains zero literal `best_source=none` entries. For the last two cases I used two `wave.cpp` entries whose map has only a file-level `pb:wave.cpp` assignment and no function name. That tests a missing function-level ancestor without presenting the map as stronger than it is.

| Function | Source-only candidate | With seeded pseudocode / measured residual | Compiles |
|---|---|---|---:|
| PB `ChangeSprite`, `0x1f210` | Copied Planet `misc.cpp`: **MISMATCH**, 320-byte object body. At the forced entry, its first instructions are `mov ecx,[mouseType]; xor eax,eax; cmp ecx,1`; oracle starts `mov eax,[mouseType]; push edi; cmp eax,1`. | Shifted the cursor IDs and initial handle value to follow the decompilation; still **MISMATCH**, 288-byte object body. Its first instruction still uses `ECX` instead of oracle `EAX`; the branch/register sequence also differs. The decompiler clarified that IDs 1–12 select the cursor strings; PB's `SPRITE_MAP`/`SPRITE_FILL` cases do not appear in that routine. It called the input `HCURSOR`, which is not proven as a source type. | 2 |
| SB2 `CButton::TreatEvent`, `0x2d60` | Copied SB2 source: fixed bytes match, initially **RELOC-UNVERIFIED** (144 object bytes; oracle Ghidra body count 111). | **STRICT** at a unique location after resolving direct-call and local switch-label relocations. No source edit or recompile was needed. Pseudocode shows `this` plus three arguments and the Win32 message switch; it renders `POINT` return as `ulonglong` split into two integers and shows raw fields at `this+0x1c` and `this+0x14`. | 1 |
| SB2 `CMenu::TreatEvent`, `0x1f040` | Copied SB2 source: fixed bytes match, initially **RELOC-UNVERIFIED** (144 object bytes; oracle Ghidra body count 117). | **STRICT** at a unique location after resolving direct-call and local switch-label relocations. No source edit or recompile was needed. Pseudocode gives `this` plus three arguments, the message switch, and the `m_nbButtons` gate at `this+0x20`. | 1 |
| SB2 `CEvent::FlushInput`, `0x166f0` | **STRICT** (32-byte object window; oracle body count 24), with its call relocation verified. | Pseudocode gets the two writes/call and offsets (`+0x9494`, `+0x6d58`). The first export showed an extra `int param_1`; after rerunning the cached project the signature no longer had it. The oracle body ends in plain `ret`, and the final signature agrees with the no-argument source. | 1 |
| File-only `pb:wave.cpp`, wrapper `0x23b20` | The copied ancestor TU compiled three functions and had **0/3 wildcard hits**; no source method could be assigned to this entry. | Translated the pseudocode wrapper (`path`, output pointer, capacity `0x40`): **STRICT** at a unique location (32-byte object window; oracle body count 21), with the callee mapped to `0x423af0`. Exact candidate: [orphan_23b20_pseudo.cpp](../scratch/decomp/candidate/orphan_23b20_pseudo.cpp). | 1 pseudo candidate, plus shared ancestor-TU compile |
| File-only `pb:wave.cpp`, wrapper `0x23bb0` | Same ancestor-TU result: no function-level source candidate. | Translated the one-argument helper call: **STRICT** at a unique location (16-byte object window; oracle body count 14), with the callee mapped to `0x424480`. Exact candidate: [orphan_23bb0_pseudo.cpp](../scratch/decomp/candidate/orphan_23bb0_pseudo.cpp). | 1 pseudo candidate, plus shared ancestor-TU compile |

The wrapper names below are descriptive hypotheses; the code translates the observed arguments and direct calls. The two short source candidates are:

```cpp
extern "C" void __cdecl FUN_00423af0(const char *path, char *out, unsigned int capacity);
extern "C" void __cdecl WavePathHelper_DefaultCapacity(const char *path, char *out)
{
    FUN_00423af0(path, out, 0x40);
}

extern "C" void __cdecl FUN_00424480(void *value);
extern "C" void __cdecl WaveBuffer_Release(void *value)
{
    FUN_00424480(value);
}
```

The exact strict SB2 sources are `CButton::TreatEvent` in [button.cpp:241](../external/sb2-decomp/src/button.cpp#L241), `CMenu::TreatEvent` in [menu.cpp:50](../external/sb2-decomp/src/menu.cpp#L50), and `CEvent::FlushInput` in [event.cpp:1985](../external/sb2-decomp/src/event.cpp#L1985). The last method's body is `m_keyPress = 0; m_pDecor->KeyChange(0);`.

**[EVIDENCE]** The CButton and CMenu source objects first reported unverified local-label and direct-call relocations. Ghidra call targets plus each uniquely placed object's intra-TU label offsets supplied explicit `--symbol-va` mappings; `cmpfunc.py` then reported all relocations strict and a unique byte placement. This resolved relocation evidence; no bytes were masked or normalized. The wrapper candidates also required an explicit callee VA mapping. Full per-function records are in `build/decomp/results/*_seeded_unique.json` and `*_unique.json`.

## Verdict and translation notes

**[EVIDENCE]** In this selected set, source-only candidates yielded one strict match, two relocation-unverified matches, one mismatch, and no function-level candidate for the two wrappers. After pseudocode-guided translation and relocation mapping, five of six were strict; `ChangeSprite` remained a mismatch. The gain combines two wrapper translations and relocation proof for two already byte-matching UI methods, so it is not a pure code-generation rate.

**[HYPOTHESIS]** This small, selected pilot does not establish a general reduction in compiler iterations. It does show value in three places: decompilation quickly narrows unknown wrapper behavior, field offsets and call targets; it reveals when a source method's fixed bytes are already right but relocation evidence is missing; and it highlights prototype errors before source is promoted. Here, four ancestor-backed methods needed one compile each; only `ChangeSprite` needed a second source attempt. The two wrapper candidates each needed one compile after translation.

**[EVIDENCE]** Common VC5-sensitive translation pitfalls observed:

- Ghidra's `__thiscall` was seeded from the class-method source map, not independently discovered. `CEvent::FlushInput`'s inferred parameter list changed between the initial and cached-project export; its first pass showed an extra parameter that disappeared on rerun. The oracle body has a plain `ret`.
- A Win32 `POINT` return appeared as a 64-bit temporary and high/low-word extraction. Preserve the source `POINT` expression and test its compile; do not copy `ulonglong` scaffolding mechanically.
- Switches and adjacent conditions were merged or inverted. CButton's source early-return condition became one combined field predicate; the branch body and case mapping were useful, but source order remains a compiler fingerprint.
- Raw `this+offset` reads recover access locations, not member names or proven structure layouts. The `CDecor` offsets in `docs/ANCESTRY_SB2.md` remain hypotheses and were not injected as types.
- Pseudocode exposed the PB `ChangeSprite` cursor-ID difference, but did not reproduce register allocation, statement ordering, or the oracle's first `EAX` load. Semantic agreement alone did not make a byte match.

**[OPEN]** The selected set is not representative of large stateful methods, loops, or data-heavy functions. The source map has no literal `best_source=none` cases to test; the file-only wave assignments are the closest available substitutes. The exact linked layout and symbol identity for other unverified local tables must be independently established before calling them strict.

**Recommendation:** consult `build/decomp` when a target needs a body/callee/xref map, field offsets, a control-flow sketch, or an explanation for an unverified call/table relocation. Start from the instruction bytes and mapped source; use pseudocode to narrow candidate edits, then let VC5 output and strict `cmpfunc.py` results decide. Treat signatures, types, and C structure declarations as hypotheses unless independently proven.
