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

## Round 2: tail functions (residue map driven)

### Results ([EVIDENCE])

| Function | Cause found | Result |
|---|---|---|
| `CPixmap::QuickIcon` 0x21b20 | generic path and clip/blit tail are Planet Blupi `DrawIcon` + `BltFast` code (nbx/nby, `limit`, `HRESULT ddrval`); the reconstruction's reused `num` produced the 0x10 vs 0x14 frame | STRICT |
| `CPixmap::Create` 0x21190 | `SetDisplayMode(..., bTrueColor ? 16 : 8)` uses the parameter (oracle reads its register), `m_dim = dim`; this also removed the global zero-register promotion | 597 -> 4 bytes (one scheduling swap) |
| `AddCDPath` 0x1f410 | SB1 prefix list from the oracle strings, no `daniel` case | 3 regions (copy-tail layout) |
| `CEvent::ChangePhase` 0x19310 | if-chain (not the SB2 `switch`), `WM_PHASE_DOQUIT`, `WM_PHASE_SETUPp`, demo worlds 200-219, `MESS_LEAVE`; Sol WIP integrated | 1,879/2,029 instructions; rest is the ebx/edi assignment of promoted 0 / `WM_PHASE_PLAY` |
| `CDecor::Build` 0x4350 | `type >= 8 && type <= 11`; cloud loop like the net-blupi loop | 1,155/1,237 instructions; per-branch blupi QuickIcon tails still open |

### Emission order (corrected)

[EVIDENCE] A free scratch-register choice follows the allocations emitted before it in layout order (BLUPI-REG-04).
Micro TU: an extra load in `case 0x102`/`case 0x150` (bodies emitted before the `case 0x100` body) flips the 0x100
case's choices regardless of source position; in `case 0xA0`/`case 0x50` (emitted after) it never does. Real
`TreatEventBase`: the same diagnostic load flips both residues in `WM_NCMOUSEMOVE` (emitted before `WM_KEYDOWN`) and
in no case emitted after it. An earlier draft of this section said "descending case value"; that was wrong.
Consequence: the original differs from our source in something emitted before the residue sites (function start,
network receive block, dispatch, `WM_NCMOUSEMOVE`), although all of that code is byte-identical, so the difference
must change free/forced allocation status without changing bytes.

### Zero promotion

[EVIDENCE] `CPixmap::Create`: removing any one of four early zero uses (diagnostic only) stops VC5 from keeping 0 in
a callee-saved register; the original had a different early register demand (the `bTrueColor` parameter kept live),
not fewer zeros. Use `fnprobe` on single-substitution diagnostics to find such thresholds, then look for a real
source difference nearby.

### Negative / open

- [EVIDENCE] Merged duplicate tails (`goto key_recorded` inlined) and all VC5 front-end/back-end mixes (42 pairs) leave
  `TreatEventBase`, `DrawMap`, `Send`, `DrawButtons` unchanged.
- [OPEN] `TreatEventBase` (8 bytes), `DrawButtons` (split-title scheduling), `DrawMap` (per-field registers for the
  RECT/POINT temporaries), `CNetwork::Send` (parameters cached in esi/edi), `ChangePhase` (constant register swap),
  `Build` (tail sharing in the blupi block), `CPixmap::Create` (vtable load order), `DoInit` (CDecor size, layout debt).

### DecorNextAction / GetBlupiInfo ([EVIDENCE], still open)

- The DecorNextAction body (scratch/sol-build) is byte-identical to the oracle; inserting it before GetBlupiInfo swaps
  both commutative OR operand pairs there (`m_blupiJeep|m_blupiTank`, `m_blupiSurf|m_blupiNage`).
- Isolation: the action table alone is inert; a trivial `return m_posDecor;` body, or any free/static/member function
  defined before GetBlupiInfo, causes the same swap; the same function after GetBlupiInfo is inert. Removing the 14
  POINT operator helpers from def.h is inert. Operand spelling (natural, swapped, local-variable forms) is inert.
- Extern-declaration probes before GetBlupiInfo (with DecorNextAction) fix only the jeep pair (counts 3-6, 11-16);
  extra static functions give the same 2-byte state; no tested context reaches 0.
- Conclusion: the original TU differs in the number/kind of definitions before GetBlupiInfo in a way not yet
  identified. The non-inline POINT operators in def.h are a reconstruction convenience (they would also clash at
  link time) and are recorded for cleanup, but they are not the cause here.
- [EVIDENCE] Removing the 14 POINT operator helpers from def.h compiles every TU (none uses them) but regresses four
  accepted functions (IsDeepWater, MockeryDetect, MoveObjectSort, QuickIcon): their context currently depends on
  those definitions, which therefore stand in for some genuine original context. Kept until that is identified.

## Round 3: allocator measurements (build/tmp/ra lab)

### Rotor ([EVIDENCE], BLUPI-REG-05)

Serialized loads (each followed by a call) take eax, ecx, edx, eax, ... - a per-function rotor that calls do not
reset. Free expression temporaries take the next register and advance it; forced registers (call results, cdq/idiv,
`cl` shift counts) do not; `a + b` advances it twice (ecx, edx), which looks like a reset. A plain-copy local
(`t = h[k]; V1(t);`) takes the first free register (eax) and leaves the rotor alone; a computed local (`t = h[k] + 1`)
behaves as a temporary; a local used twice goes to a callee-saved register.

### Counter domains ([EVIDENCE])

Using the Blood II phase model (one TU-wide symbol counter; front-end and back-end numbered decisions):
- TreatEventBase: no change for 0-300 `extern` probes at the end of event.cpp (back-end domain), 0-256 at the top,
  0-256 directly before the function. Its two residues are not symbol-number decisions.
- DrawButtons: no change for 0-128 probes directly before it or 0-64 at the top; a few large top counts make it worse
  (78). Not a small counter offset either.

### Negative ([EVIDENCE])

- Local/temporary conversions of single-use loads emitted before TreatEventBase's toggle (network block, prologue,
  text-input guard, WM_NCMOUSEMOVE) are inert in the full function.
- DrawButtons: selection/font locals at the split-title site are worse.
- The simple rotor model mispredicts one choice in TreatEventBase's cheat code (`lea ecx,[edi+1]`), so the real
  free-choice rule has more inputs than the micro corpus shows. [OPEN] Reading the selection routine in c2.exe
  (Ghidra) is the remaining route for these two functions.
