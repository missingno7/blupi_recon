# Reconstructed source provenance

**[EVIDENCE]** Planet Blupi references are from https://github.com/blupi-games/planetblupi, tag v1.8.0, commit 203ccc8, licensed GPLv3+. SB2 reconstruction material is from external/sb2-decomp and is GPLv3. Reference repositories are read-only. Provenance stays here so source comments do not shift observable __LINE__ values.

## PB-derived core files

- sound.cpp: Planet Blupi v1.8.0-derived Windows DirectSound source, retaining the matched source line numbers and TRY_DS sites. The core candidate's CSound::StopSound body is strict at 0x23050, but is not in canonical source: adding its declaration to shared sound.h changes seven bytes in accepted CButton::MouseMove, so the prior header context is retained.
- movie.cpp and ddutil.cpp: Planet Blupi v1.8.0-derived Windows DirectDraw/AVI source; the core candidates were identical to the already accepted files.
- misc.cpp: Planet Blupi v1.8.0-derived utility source. Its active AddUserPath folder literal is changed to c:\Speedy Blupi\ to match the SB1 path data.
- sound.h, movie.h, misc.h, ddutil.h, and resource.h: Planet Blupi-derived declarations and support headers. sound.h remains without the candidate-only StopSound declaration to preserve the accepted UI TU context.

## Pixmap core integration

- pixmap.cpp: Planet Blupi-derived DirectDraw implementation, integrated from the core candidate. It yields 22 STRICT functions in the full multi-TU run, including CPixmap::~CPixmap.
- pixmap.h: the PB-derived class layout includes one BOOL m_bTrueColorBack after m_mouseType, at this+8. The offset is required by oracle accesses and shifts later members by four bytes. The name is a hypothesis taken from SB2; SB2's surrounding layout does not match SB1. This field also allows the destructor and other pixmap methods to match.

## UI cluster

- button.cpp: PB-derived routines with SB1 behavior/layout updates. SetIconMenu and SetToolTips use the SB2 reconstruction methods (GPLv3).
- button.h: PB-derived UI declaration set adjusted to the SB1 member layout established in docs/UI_CLUSTER.md. Keep the UI Create signature and m_bDemo field; the oracle's call cleanup and +0x18 access require them.
- jauge.cpp: PB-derived routines with the SB1 field layout; GetLevel and GetType use SB2 reconstruction methods (GPLv3).
- jauge.h: UI declaration set with the CDecor* member omitted to match observed SB1 offsets; retain GetLevel and GetType declarations.
- menu.cpp and menu.h: PB-derived menu routines and declarations, adjusted to the observed SB1 member offsets and initialization behavior.
- text.cpp: PB-derived text routines plus SB2 DrawCharSingle, DrawTextLeft, and DrawChar code (GPLv3). The SB2-style texttables.h glyph table is GPLv3; its equality with the SB1 table remains open.
- text.h: UI free-function declarations used by the candidate text TU.
- decor.h: three include names were lower-cased (DEF.H, SOUND.H, PIXMAP.H) to avoid VC5 reopening the same headers under different case spellings. This is a compile-context fix; declarations do not change.
- def.h: existing layout and line numbers retained; only SB1 channel values changed to CHLITTLE=7 and CHTEXT=6. The candidate-only blank first line was dropped.

**[EVIDENCE]** def.h does not undefine DrawText or GetCharWidth. Windows expands those names to DrawTextA and GetCharWidthA; object symbol names differ, while function body bytes match the version built with the candidate header's #undef lines. No source-level compile shim remains.

**[EVIDENCE]** The coherent UI/core header set preserves every previously accepted UI function in the clean combined build. sound.h is the exception to the core candidate header set: its StopSound declaration was held back because adding it regressed CButton::MouseMove by seven bytes.

**[OPEN]** DrawChar and the glyph table do not yet match the oracle; table identity is unproven. Per-function UI evidence and remaining differences are in docs/UI_CLUSTER.md.
