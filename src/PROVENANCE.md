# Reconstructed source provenance

**[EVIDENCE]** Planet Blupi references are from `https://github.com/blupi-games/planetblupi`, tag `v1.8.0`, commit `203ccc8`, licensed GPLv3+. SB2 reconstruction material is from `external/sb2-decomp` and is GPLv3. Reference repositories are read-only. Provenance stays here so source comments do not shift observable `__LINE__` values.

## Existing matched translation units

- `sound.cpp`, `movie.cpp`, `misc.cpp`, and `ddutil.cpp`: byte-for-byte Planet Blupi v1.8.0 source copies with original CRLF and line numbers.
- `ddutil.h`, `misc.h`, `movie.h`, `resource.h`, and `sound.h`: the copied headers used by those TUs.

## UI cluster

- `button.cpp`: PB-derived routines with SB1 behavior/layout updates. `SetIconMenu` and `SetToolTips` use the SB2 reconstruction methods (GPLv3).
- `button.h`: PB-derived UI declaration set adjusted to the SB1 member layout established in `docs/UI_CLUSTER.md`.
- `jauge.cpp`: PB-derived routines with the SB1 field layout; `GetLevel` and `GetType` use SB2 reconstruction methods (GPLv3).
- `jauge.h`: UI declaration set with the `CDecor*` member omitted to match the observed SB1 offsets.
- `menu.cpp` and `menu.h`: PB-derived menu routines and declarations, adjusted to the observed SB1 member offsets and initialization behavior.
- `text.cpp`: PB-derived text routines plus SB2 `DrawCharSingle`, `DrawTextLeft`, and `DrawChar` code (GPLv3). The SB2-style `texttables.h` glyph table is GPLv3; its equality with the SB1 table remains open.
- `text.h`, `pixmap.h`, and `decor.h`: UI headers from the candidate set. In `decor.h`, three include names were lower-cased to avoid VC5 reopening the same headers under different case spellings.
- `def.h`: retained the existing header layout and line numbers; only SB1 channel values changed to `CHLITTLE=7` and `CHTEXT=6`. The candidate-only blank first line was dropped.

**[EVIDENCE]** `def.h` does not undefine `DrawText` or `GetCharWidth`. Windows expands those names to `DrawTextA` and `GetCharWidthA`; the resulting object symbols differ, while the function body bytes match the version built with the candidate header's `#undef` lines. No source-level compile shim remains.

**[OPEN]** `DrawChar` and the glyph table do not yet match the oracle; table identity is unproven. Per-function UI evidence and remaining differences are in `docs/UI_CLUSTER.md`.
