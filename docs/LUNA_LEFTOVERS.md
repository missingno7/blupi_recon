# Luna leftovers sweep

## Scope and verification

[EVIDENCE] The worker owns candidate copies in `scratch/luna-leftovers/` for
`button.cpp`, `network.cpp`, `text.cpp`, `sound.cpp`, `misc.cpp`, `decnet.cpp`,
`pixmap.cpp`, and `blupi.cpp`. Canonical `src/` files and headers were not
edited. Candidate runs used the current shared headers and VC5 SP3
`/W3 /GX /O2 /ML /Gd /Gy` settings through
`.pyenv\python.exe tools\verify.py --tu src/X.cpp=scratch\luna-leftovers\X.cpp --build-dir lunaleft`.
Each of the eight candidate TUs completed verification without an accepted
function regression. Seven runs stayed at the no-substitution baseline of
100,560 reported game bytes; the button candidate reported 100,656 because
its strict `MouseDown` span includes 12 bytes of matched alignment after the
84-byte oracle body. Count the new body as 84 unique original bytes.

## Target ledger

Sizes below are oracle body bytes; candidate spans are from the isolated
`lunaleft` runs and include alignment where the verifier reports it.

| TU | RVA | Function | Candidate | Result |
|---|---:|---|---:|---|
| button.cpp | 0x2a30 | `CButton::Create` | 176 | MISMATCH / 161 |
| button.cpp | 0x2b40 | `CButton::Draw` | 336 | MISMATCH / 325 |
| button.cpp | 0x2e10 | `CButton::GetToolTips` | 192 | MISMATCH / 237 |
| button.cpp | 0x2f70 | `CButton::MouseDown` | 96 | **STRICT / 84** |
| network.cpp | 0x20cb0 | `CNetwork::JoinSession` | 224 | MISMATCH / 237 |
| network.cpp | 0x20dd0 | `CNetwork::CreateSession` | 224 | MISMATCH / 234 |
| network.cpp | 0x20ec0 | `CNetwork::Send` | 80 | MISMATCH / 77 |
| network.cpp | 0x20fe0 | `CNetwork::Close` | 16 | MISMATCH / 18 |
| text.cpp | 0x236b0 | `DrawChar` | 144 | MISMATCH / 144 |
| sound.cpp | 0x22940 | `CSound::CreateSoundBuffer` | 208 | RELOC-UNVERIFIED / 206 |
| misc.cpp | 0x1f210 | `ChangeSprite` | 288 | MISMATCH / 285 |
| misc.cpp | 0x1f410 | `AddCDPath` | 336 | MISMATCH / 324 |
| decnet.cpp | 0x106b0 | `ObjectStart` | 480 | MISMATCH / 489 |
| pixmap.cpp | 0x21190 | `CPixmap::Create` | 912 | MISMATCH / 902 |
| pixmap.cpp | 0x21b20 | clipped icon blit (`QuickIcon` hypothesis) | 800 | MISMATCH / 770 |
| blupi.cpp | 0x1030 | `ReadConfig` | 688 | MISMATCH / 673 |
| blupi.cpp | 0x1ef0 | `DoInit` | 2816 | MISMATCH / 2801 |

[EVIDENCE] `CButton::MouseDown` at `0x2f70` is STRICT for 84 unique original
bytes. The verifier proves the call relocations to `Detect` at `0x2f00` and
the four-argument `CSound::PlayImage` at `0x230c0`. Its 96-byte candidate
section includes the 12-byte aligned tail before `MouseMove`.

[EVIDENCE] `CSound::CreateSoundBuffer` remains `RELOC-UNVERIFIED`: its code
bytes match, but the embedded `__FILE__` path differs (`F:\Profil\sound.cpp`
in the oracle). No path directive or other source-path override was added.

## Compiler findings

- **[EVIDENCE; Blupi-specific; VC5-CTL-01, VC5-SEM-01] `MouseDown`:** the
  original positive `if (Detect) { ... } else return FALSE` and two-argument
  `PlayImage` call emitted a 96-byte mismatch. Rewriting the guard as an early
  false return and calling `PlayImage(SOUND_CLICK, pos, -1, 0)` produced
  STRICT equality with both call targets proven. Flags and TU context are the
  SP3 settings above, with the shared headers and no PCH.
- **[EVIDENCE; negative; VC5-ORD-01] `CButton::Create`:** target stores place
  the icon width before enable/demo/hide and then store position Y and height.
  A full field-store reorder and a focused width/flag/position reorder both
  remained MISMATCH at 176/161 bytes. The scratch body is restored to its
  starting form.
- **[EVIDENCE; negative/incomplete; VC5-CTL-01, VC5-LOC-02] `CButton::Draw`:**
  positive nested guards matching the target control-flow description and a
  pointer walk for submenu icons remained MISMATCH at 336/325 bytes. Those
  target-backed source forms remain in the scratch candidate for review.
- **[EVIDENCE; negative; VC5-CTL-01, VC5-CTL-03] `CNetwork::Close`:** direct
  boolean return, guard-return, ternary return, and a local boolean assignment
  were tested. The ordinary return forms kept VC5's `neg/sbb/inc` sequence;
  the local conditional added a branch and frame. None matched the target's
  `xor edx; test eax; sete dl; mov eax,edx` tail (16/18 bytes). The network
  candidate is restored to its starting form.
- **[EVIDENCE; negative; VC5-SEM-01] `CNetwork::Send`:** splitting the
  assignment out of the HRESULT condition left the candidate at 80/77 bytes.
- **[EVIDENCE; negative/incomplete; VC5-LOC-02, VC5-CTX-01] `JoinSession` and
  `CreateSession`:** `ZeroMemory`, aggregate-zero initialization, and placing
  `DPNAME` in the post-`Open` success path were tested against the target's
  larger stack setup. Candidate spans ranged from 224 to 256 bytes and did not
  match the 237/234-byte targets. The network candidate is restored.
- **[EVIDENCE; semantic correction, still unverified; VC5-SEM-01] `AddCDPath`:**
  the oracle pseudocode checks `data\\`, `image\\`, `image08\\`, `image16\\`,
  and `movie\\`, then uses the `..\\` literal when that prefix branch is
  taken. The scratch body now follows those checks instead of the inherited
  `bDaniel` plus two-prefix logic; it remains MISMATCH at 336/324 bytes.

## Header deltas and next work

[EVIDENCE] Header deltas: none. Candidates use the current canonical headers.
No canonical source or manifest changes were made.

[OPEN] Next, compare target and candidate instruction streams for `AddCDPath`
and the two pixmap targets, then pursue `ObjectStart` only if new source or
layout evidence is found. Existing notes already document the `ReadConfig`
SIB residue, `DoInit` class-size difference, and prior negative `ObjectStart`
probes; do not repeat those probes without new evidence. `MouseDown` is ready
for supervisor integration as an isolated strict body.
