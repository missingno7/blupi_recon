// Editor input in the side areas of a wide view (port/docs/ROADMAP.md
// stage 8). Derived from CEvent::BuildDown and CEvent::BuildMove
// (src/event.cpp) and CDecor::ScreenPosToCelPos (src/decdesign.cpp).
//
// The world frame's place in the wide view is scene::PlaceView's: centred, and
// clamped at a world edge like the play view. PortEditorMouse (main.cpp) gives a
// press on one of the game's buttons to the game unchanged and passes world input
// here in frame coordinates, including positions below 0 or from LXIMAGE on,
// which the original rejected as outside its window. Cells outside the world are
// treated like positions outside the original window: nothing happens and no
// cell is highlighted.

#include "def.h"
#include "decor.h"
#include "event.h"

#include "game/port_game.h"
#include "game/member_access.h"

namespace
{
#define EDITOR_DECOR_MEMBERS(X) X(m_posDecor) X(m_dimCelHili) X(m_posCelHili)
BLUPI_MEMBER_ACCESS(DecorEdit, CDecor, EDITOR_DECOR_MEMBERS)

int FloorDiv(int a, int b)
{
	return a >= 0 ? a / b : -((-a + b - 1) / b);
}

// CDecor::ScreenPosToCelPos without the frame bounds test.
POINT CelPos(const CDecor& decor, POINT pos)
{
	const POINT& m_posDecor = decor.*DecorEdit::m_posDecor;
	const POINT& m_dimCelHili = decor.*DecorEdit::m_dimCelHili;
	POINT result;
	result.x = FloorDiv(m_posDecor.x - m_dimCelHili.x * 32 + 32 + pos.x, DIMOBJX);
	result.y = FloorDiv(m_posDecor.y - m_dimCelHili.y * 32 + 32 + pos.y, DIMOBJY);
	if (result.x < 0 || result.x >= MAXCELX || result.y < 0 || result.y >= MAXCELY)
	{
		result.x = -1;
		result.y = -1;
	}
	return result;
}

// CEvent::BuildDown for a cell computed above.
void BuildDown(CEvent& event, CDecor& decor, POINT cel, int fwKeys, BOOL bMix)
{
	if (cel.x == -1) return;
	if (fwKeys & MK_RBUTTON)
	{
		decor.DeleteCel(cel);
		return;
	}
	if (event.GetState(WM_DECOR1) == 1) decor.DeleteCel(cel);
	if (event.GetState(WM_DECOR2) == 1) decor.PlaceDecor1(cel, event.GetMenu(WM_DECOR2), fwKeys, bMix);
	if (event.GetState(WM_DECOR3) == 1) decor.PlaceDecor2(cel, event.GetMenu(WM_DECOR3), fwKeys, bMix);
	if (event.GetState(WM_DECOR4) == 1) decor.PlaceDecor3(cel, event.GetMenu(WM_DECOR4), fwKeys, bMix);
	if (event.GetState(WM_DECOR5) == 1) decor.PlaceDecor4(cel, event.GetMenu(WM_DECOR5), fwKeys, bMix);
	if (event.GetState(WM_DECOR6) == 1) decor.PlaceDecor5(cel, event.GetMenu(WM_DECOR6), fwKeys, bMix);
	if (event.GetState(WM_DECOR7) == 1) decor.PlaceDecor6(cel, event.GetMenu(WM_DECOR7), fwKeys, bMix);
	if (event.GetState(WM_DECOR8) == 1) decor.PlaceDecor7(cel, event.GetMenu(WM_DECOR8), fwKeys, bMix);
}
}  // namespace

void PortEditorSideMouse(CEvent* event, CDecor* decor, UINT message, int fwKeys, POINT pos)
{
	if (event == NULL || decor == NULL) return;
	const POINT cel = CelPos(*decor, pos);
	switch (message)
	{
	case WM_LBUTTONDOWN:
	case WM_RBUTTONDOWN:
		BuildDown(*event, *decor, cel, fwKeys, TRUE);
		break;
	case WM_MOUSEMOVE:
		// CEvent::BuildMove: draw while a button is held, then the highlight.
		if (fwKeys & (MK_LBUTTON | MK_RBUTTON)) BuildDown(*event, *decor, cel, fwKeys, FALSE);
		decor->*DecorEdit::m_posCelHili = cel;
		break;
	}
}
