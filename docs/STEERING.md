# Source-level steering (experimental, branch `claude/steering`, 2026-10-19)

Question: can a code-neutral C++ construct change one decision inside the unmodified VC5 SP3 compiler and then
vanish before emission? Two outcome classes are kept apart:

- **Historical STRICT**: canonical `src/`, no steering constructs. Unchanged: 127,456 / 141,488 game-code bytes.
- **SOURCE_STEERED_EXACT**: the same compiler and flags reproduce the oracle with a documented steering construct.
  Steering sources live only under `steering/` as patches; nothing here is historical source.

Tools: `tools/c2trace/lifetime.py` (tuples, blocks and range-id high-water mark per function at C1 read, first
allocation cost pass and preference pass; temp assignments; byte identity with a base function). Micro sources:
`msvc5kb/micros/steer/`. Candidate scripts: `scratch/claude/steer/` (ignored).

## Lifetime of code-neutral constructs ([EVIDENCE], micros lifetime_m2/m6/m7, store-first micro m3)

| Construct | In C1 IL | At first cost pass | Residue | Final code |
|---|---|---|---|---|
| `if (0) {...}` | yes | no | none | identical |
| `int k = 0; if (k) {...}`, `j = 0; x = j ? E : x;` | yes | no (C2 constant propagation) | +3..+4 range ids; the dead arm's expressions are interned | identical |
| `x != x`, `i - i`, `(i - i) * E`, `0 * E` guards | yes | no | none | identical |
| unused calculation, unused load, unused local, local `extern` | yes | no | none | identical |
| member/global/local self-assignment (single) | yes | no | none | identical |
| dead call, dead call to a `dllimport` | yes | no | none; no object symbol | identical |
| duplicated tests (`&&`, `\|\|`, nested, `else if`, `x == x`) | yes | no | 0..1 id | identical |
| identical arms, same-value phi, constant ternary, one-pass loop, `goto` | yes | no | +2..+8 ids | identical |
| inline getter/toggle (event.cpp) | yes | no (forwarded) | +3 ids | identical |
| switch, conditional redundant store, nested branch, member self-copy x3 | yes | yes | | changed |
| 32-bit named select destination (int/unsigned/long/enum) | yes | yes | merges with the destination local | changed |
| 8/16-bit named select destination | yes | yes; setcc gets a preference | | adds movsx |

Rule: everything code-neutral is removed by early C2 before the first allocation cost pass. What survives is
identity only (expression-descriptor and range numbering, and front-end IDs for declarations and `if`).
Constructs that survive into allocation changed the emitted code in every tested case.

## Positive control: Build from its pre-fix source ([EVIDENCE])

`lifetime.py` on the historical fix: C1 tuples 1,713, cost-stage tuples 1,702, blocks 205, temps 124 and size 3,872
are identical before and after; only range ids move. The fix is an identity change, the class that survives.

Steering construct (`steering/decblock_build.patch`, against canonical): keep the pre-fix single expression and add
`j = 0; tinyPoint.y = j ? 0 - posDecor.y % DIMOBJY : tinyPoint.y;` (j is overwritten by its loop right after).
C2 interns NEG(y % 64) from the dead arm before constant propagation deletes it, which reproduces the fix's
interning order. `tools/verify.py --tu src/decblock.cpp=<patched>`: exit 0, decblock 26/26 STRICT with relocations,
127,456 bytes, zero regressions. Neighbours: `{ int k_ = 0; if (k_) ...= E; }` also gives 0 bytes, but its
declaration and `if` cost two front-end IDs and change GetBlupiInfo. The ternary form costs none. Placing it before
the statement, `if (0)`, `x != x`/`i - i` guards, products and expression statements stay at 8 bytes.

## Targets ([EVIDENCE])

- **ChangePhase** (oracle-order source, `scratch/claude/steer/cpident.py`). The zero/PLAY priorities stay -65/-78
  in all 36 code-neutral variants:
  - Dead branches and dead-arm interning of `phase == PLAY`, `m_phase == PLAY`, `!m_bBuildOfficialMissions`,
    `!m_bDemoPlay` and `m_bHili` at four positions (top, guard, index, oldPhase).
  - One to eight ID-consuming blocks.

  The `k_` declaration's ID cost changes tie and key decisions elsewhere (1995 -> 1879 aligned), confirming identity
  is live, but the decision is quantity-driven (BLUPI-ALLOC-16/17). The only priority movers are repeated member
  self-copies (-62/-79, -65/-79); they add instructions and move the gap the wrong way.
- **TreatEventBase** (`scratch/claude/t7.py`, `t8.py`). 21 upstream and in-block identity and dead-arm variants all
  stay at 8 bytes (one shifts an unrelated byte). The rotor counts tuples, not ids. Only a preference-coalesced copy
  survives to temp assignment code-free (the BltFast mechanism, BLUPI-REG-11). Every variable that could be the
  setcc destination either merges with bEnable (32-bit) or needs a movsx (8/16-bit).

## Result

SOURCE_STEERED_EXACT is achievable for identity-class residues: shown on Build, the class of every solved allocation
residue so far. It is not achievable for the two open functions with any tested mechanism. Both decisions depend on
IL that exists at the decision point: cost quantities for ChangePhase, temp count and preference for TreatEventBase.
No code-neutral construct survives to either point. 100% source-steered game code is therefore not reached. Steering
could close them only with a construct that survives to the cost pass or temp assignment and is deleted after it; no
such deletion exists in this compiler beyond the preference-coalesced copy.
