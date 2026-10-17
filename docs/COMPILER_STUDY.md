# Compiler study: residues, reduced examples and the residue map (2026-10-17)

Method: start from where the bytes differ, never from blind runs.
1. `tools/residues.py` aligns every unmatched function with its oracle body and classifies each differing region
   (register / order / constant / insert / delete / mixed). Relocated operands, addend-0 spellings and switch
   tables are normalized, so the regions are the real differences.
2. For one region, `tools/fnprobe.py` compiles one TU (canonical or candidate) with the manifest settings in about
   a second and diffs one function against the oracle (`--region` prints both sequences). Diagnostic only;
   acceptance is always `tools/verify.py`.
3. When the cause is unclear, rebuild the function's shape in a micro TU and vary one property at a time.

In Git Bash, set `MSYS_NO_PATHCONV=1` before passing `/flags` (`fnprobe --extra-flag=/G5`): MSYS otherwise rewrites
`/G5` into a path and the compiler silently ignores it. An earlier flag sweep in this study was invalid for this
reason and was redone.

## Results ([EVIDENCE])

| Function | Residue class (map) | Cause | Fix |
|---|---|---|---|
| `CDecor::FileExist` 0x159f0 | cleanup block at end vs inline | reconstruction `goto success` arrangement | Planet Blupi decio form (BLUPI-CTL-02) |
| `CDecor::CurrentRead` 0x153d0 | same | same | same |
| `CButton::Create` 0x2a30 | store order | member-wise POINT copies; flag order | `m_pos = pos; m_dim = iconDim;`, `m_bDemo` before `m_bHide` |
| `CDecor::AdaptBorder` 0x144d0 | register-only (rotation) | POINT temporaries as call arguments | `AdaptMidBorder(int x, int y)` (BLUPI-REG-03) |
| `ChangeSprite` 0x1f210 | constant off by one + reload | SB2 `SPRITE_MAP` shifted SB1 numbering; default assignment | SB1 numbering; `hCursor` left uninitialized (BLUPI-SEM-02) |

Coverage 107,440 -> 110,144 / 141,488 bytes. All accepted functions stay strict.

### Struct temporaries

[EVIDENCE] Micro TU (`AdaptBorder` shape, 30 lines): five calls with `MAKE_POINT(...)` arguments use eax for every
temporary; the same calls with `(int x, int y)` parameters rotate eax/ecx/edx/eax, and the following index
calculation follows the rotation. Inline-helper form (initializer, member stores, derived-class constructor) and a
single extra struct-temporary call elsewhere are neutral. On SB1 the scalar signature made `AdaptBorder` STRICT while
`AdaptMidBorder`'s own body stayed byte-identical (its accepted record was renamed to `?AdaptMidBorder@CDecor@@QAEXHH@Z`).

### Error-block placement

[EVIDENCE] `if (bad) goto error; ... return TRUE; error: ...` (error label at the end of the source) is laid out
with the cleanup right after the last check, and earlier NULL checks jump into its middle. The reconstruction's
`if (ok) goto success; error: ...; success:` made VC5 move the cleanup to the end.

### Uninitialized local

[EVIDENCE] `ChangeSprite` without a default `hCursor` assignment reads the dead `sprite` parameter slot
(`mov eax,[esp+0xc]`), exactly as the oracle does.

## TreatEventBase (still open)

[EVIDENCE] Exactly 8 bytes differ, at two coupled free-choice sites (+0x41a toggle, +0x6f3 PostMessage hwnd); all
other code and the 62 jump-table entries match. Reduced examples show the choice is function-global: a load added
anywhere (even laid out after the sites) can flip it, with period 3 in the number of inserted loads.
[EVIDENCE] Inert with identical code elsewhere: declaration order, case-label order, case source order, copies,
dead stores, local types, renames, blank lines, declaration-count probes 0-32, stubbing all 96 preceding
functions, scalar signatures for its POINT callees, all VC5 trees, 25 flags (BLUPI-NEG-03).
[OPEN] The original differs in something not yet tested; the BLUPI-REG-03 class (argument/expression forms that
leave bytes identical but change temporaries) is the best remaining direction.

## Other map entries ([OPEN])

- `DrawMap` 0x14550: tail register phase differs around the `Build(RECT, POINT)` / `DrawMap(8, RECT, RECT)` struct
  temporaries; local RECT variables enlarge the frame (rejected); temporary forms were neutral in the micro TU.
- `CNetwork::Send` 0x20ec0: oracle holds dwFlags/dwDataSize in esi/edi and evaluates `dwFlags != 0` first; natural
  assignment/ternary forms are inert. The comma expression is an SB2-reconstruction artifact kept until a form matches.
- `DrawButtons` 0x16f70: split-title draw calls; the oracle loads `m_pPixmap` right before its push (strict
  right-to-left argument order), the candidate hoists it; ternary/pointer-store/POINT-temporary forms tested.
- Larger: `ChangePhase`, `Build`, `DoInit` (CDecor size), pixmap `Create`/`QuickIcon` (frame), `AddCDPath`.
