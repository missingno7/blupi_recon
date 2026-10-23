// Scene builder: port-side derivative of CDecor::Build, DecorNextAction,
// DrawInfo and VoyageDraw (see scene.h and port/docs/ROADMAP.md stage 3).
//
// Conventions:
//  * CDecor state is read through pointer-to-member accessors (DecorAccess),
//    bound to const references with the historical member names at the top of
//    each function, so the copied bodies stay textually close to src/ and any
//    write to game state is a compile error.
//  * Inside the copied bodies, m_pPixmap names a Recorder, not the CPixmap: its
//    methods have CPixmap's signatures plus a trailing Tag (layer, entity,
//    index) and append one Item with exactly the historical arguments. A call
//    that reached the real CPixmap would not compile.
//  * Historical state changes made while drawing are not made; where later
//    drawing in the same Build depends on them they are computed into locals
//    and the original statement is kept as a comment.
//  * Parts marked "not covered by the gate" are not reached by the --check-scene
//    runs (attract demos and the menu tour): editor (WM_PHASE_BUILD) and
//    network (m_bMulti) paths.

#include "game/scene.h"
#include "game/member_access.h"

#include <cmath>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <string>
#include <unordered_map>

#include "dectables.h"
#include "jauge.h"
#include "misc.h"
#include "network.h"
#include "text.h"

namespace blupi::scene
{
namespace
{
// Read access to the protected state of CDecor (and CJauge) without changing
// any header: pointers to members formed through a derived class.
#define SCENE_DECOR_MEMBERS(X) \
	X(m_pPixmap) \
	X(m_pNetwork) \
	X(m_decor) \
	X(m_bigDecor) \
	X(m_moveObject) \
	X(m_posDecor) \
	X(m_dimDecor) \
	X(m_posCelHili) \
	X(m_dimCelHili) \
	X(m_liftEnd) \
	X(m_phase) \
	X(m_time) \
	X(m_blupiPos) \
	X(m_blupiIcon) \
	X(m_blupiChannel) \
	X(m_blupiShield) \
	X(m_blupiPower) \
	X(m_blupiCloud) \
	X(m_blupiHide) \
	X(m_blupiFront) \
	X(m_blupiBullet) \
	X(m_blupiPerso) \
	X(m_blupiTimeShield) \
	X(m_blupiStartPos) \
	X(m_blupiStartDir) \
	X(m_bMulti) \
	X(m_team) \
	X(m_netBlupiPos) \
	X(m_netBlupiIcon) \
	X(m_netBlupiSec) \
	X(m_netBlupiTransport) \
	X(m_notifText) \
	X(m_jauges) \
	X(m_bPrivate) \
	X(m_bDrawSecret) \
	X(m_bJoystick) \
	X(m_mission) \
	X(m_nbVies) \
	X(m_nbTresor) \
	X(m_totalTresor) \
	X(m_voyageIcon) \
	X(m_voyageChannel) \
	X(m_voyagePhase) \
	X(m_voyageTotal) \
	X(m_voyageStart) \
	X(m_voyageEnd) \
	X(m_decorAction) \
	X(m_decorPhase)
BLUPI_MEMBER_ACCESS(DecorAccess, CDecor, SCENE_DECOR_MEMBERS)

#define SCENE_JAUGE_MEMBERS(X) X(m_bHide) X(m_pos) X(m_dim) X(m_type) X(m_level) X(m_bMinimizeRedraw) X(m_bRedraw)
BLUPI_MEMBER_ACCESS(JaugeAccess, CJauge, SCENE_JAUGE_MEMBERS)

// Annotation of one recorded draw call.
struct Tag
{
	Layer  layer;
	Entity entity;
	int    index;
	bool   simCamera;  // World item placed with m_posDecor instead of the shaken view
	Item::Cull cull = Item::Cull::None;
	POINT  ref = {0, 0};
	bool   extended = false;
};

Tag AtBack(int index) { return {Layer::Background, Entity::Parallax, index, false}; }
Tag AtWorld(Entity e, int index = 0) { return {Layer::World, e, index, false}; }
Tag AtWorldSim(Entity e, int index = 0) { return {Layer::World, e, index, true}; }
Tag AtScreen(Entity e, int index = 0) { return {Layer::Screen, e, index, false}; }
Tag AtTarget(Entity e, int index = 0) { return {Layer::Target, e, index, false}; }

// Cell of a pixel coordinate; views and coverage edges can be negative.
int FloorDiv(int a, int b)
{
	return a >= 0 ? a / b : -((-a + b - 1) / b);
}

// Build's visibility rules (the tile loop ranges and the move-object bounds
// tests of CDecor::Build), for a view of any position and size. At Build's
// own view and 640x480 they select exactly the items Build draws.
bool VisibleRule(Item::Cull cull, int index, POINT ref, POINT view, int width, int height)
{
	const int i = index / MAXCELY, j = index % MAXCELY;
	const int cx = FloorDiv(view.x, DIMOBJX), cy = FloorDiv(view.y, DIMOBJY);
	switch (cull)
	{
	case Item::Cull::None:
		return true;
	case Item::Cull::BigCell:
		return i >= cx - 1 && i < cx + width / DIMOBJX + 3 && j >= cy - 1 && j < cy + height / DIMOBJY + 2;
	case Item::Cull::Cell:
		return i >= cx && i < cx + width / DIMOBJX + 2 && j >= cy && j < cy + height / DIMOBJY + 2;
	case Item::Cull::Point:
		return ref.x >= view.x - DIMOBJX && ref.y >= view.y - DIMOBJY &&
			   ref.x <= view.x + width && ref.y <= view.y + height;
	}
	return true;
}

// A tile cell, extended unless Build draws it at its own view.
Tag AtCell(Entity e, Item::Cull rule, int i, int j, POINT view)
{
	Tag t = AtWorld(e, i * MAXCELY + j);
	t.cull = rule;
	t.extended = !VisibleRule(rule, t.index, t.ref, view, LXIMAGE, LYIMAGE);
	return t;
}

// An object tested on ref, extended unless Build draws it at its own view.
Tag AtPoint(Entity e, int index, POINT ref, POINT view)
{
	Tag t = AtWorld(e, index);
	t.cull = Item::Cull::Point;
	t.ref = ref;
	t.extended = !VisibleRule(t.cull, index, ref, view, LXIMAGE, LYIMAGE);
	return t;
}

// Whether Build draws an object tested on ref at some view within the
// coverage around view (with no coverage: Build's own bounds test).
bool Covered(POINT ref, POINT view, const Coverage& c)
{
	POINT from = {view.x - c.left, view.y - c.top};
	return VisibleRule(Item::Cull::Point, 0, ref, from, LXIMAGE + c.left + c.right, LYIMAGE + c.top + c.bottom);
}

// The parallax background of Build (the first loop of CDecor::Build), as
// DrawPart destinations and source rectangles, for the background image
// position pos at the view's left/top corner (Build: posDecor * 2 / 3). Build
// draws two columns; a wider view needs more.
int ParallaxParts(POINT pos, int width, POINT* dest, RECT* src, int max)
{
	int   i, j, n = 0;
	POINT tinyPoint;
	RECT  rect;

	int x = (pos.x % DIMDECORX + DIMDECORX) % DIMDECORX;   // Build: pos.x % DIMDECORX (pos >= 0)
	rect.left = x;
	int y = (pos.y % DIMDECORY + DIMDECORY) % DIMDECORY;
	tinyPoint.x = 0;
	for (i = 0; (i < 2 || tinyPoint.x < width) && n + 2 <= max; i++)
	{
		tinyPoint.y = 0;
		rect.top = y;
		rect.right = DIMDECORX;
		rect.bottom = DIMDECORY;
		for (j = 0; j < 2; j++)
		{
			dest[n] = tinyPoint;
			src[n] = rect;
			n++;
			rect.bottom -= rect.top;
			tinyPoint.y += rect.bottom;
			rect.top = 0;
			rect.bottom = DIMDECORY;
		}
		tinyPoint.x += DIMDECORX - rect.left;
		rect.left = 0;
	}
	return n;
}
constexpr int kMaxParallaxParts = 16;

// Stands in for m_pPixmap inside the copied bodies; records instead of drawing.
class Recorder
{
public:
	explicit Recorder(Scene& scene) : m_scene(scene) {}

	void SetClipping(RECT clip, Tag t)
	{
		Item& it = Add(Item::Op::SetClip, t);
		it.rect = clip;
	}
	void DrawPart(int chDst, int channel, POINT dest, RECT rect, int mode, BOOL bMask, Tag t)
	{
		Item& it = Add(Item::Op::DrawPart, t);
		it.chDst = chDst;
		it.channel = channel;
		it.rect = rect;
		it.mode = mode;
		it.mask = bMask;
		Place(it, dest, t);
	}
	void QuickIcon(int channel, int rank, POINT pos, Tag t)
	{
		Item& it = Add(Item::Op::QuickIcon, t);
		it.channel = channel;
		it.rank = rank;
		Place(it, pos, t);
	}
	void DrawIcon(int chDst, int channel, int rank, POINT pos, int mode, BOOL bMask, Tag t)
	{
		Item& it = Add(Item::Op::DrawIcon, t);
		it.chDst = chDst;
		it.channel = channel;
		it.rank = rank;
		it.mode = mode;
		it.mask = bMask;
		Place(it, pos, t);
	}
	void Text(Item::Op op, POINT pos, const char* pText, int font, Tag t)
	{
		Item& it = Add(op, t);
		it.text = pText;
		it.font = font;
		Place(it, pos, t);
	}
	// m_jauges[i].Draw()
	// m_jauges[i].Draw(), as a snapshot of the jauge (CJauge::Draw, src/jauge.cpp).
	// Its state change (m_bRedraw = FALSE) is made by EndOfTick.
	void JaugeDraw(const CJauge& jauge, int i, Tag t)
	{
		if (jauge.*JaugeAccess::m_bMinimizeRedraw && !(jauge.*JaugeAccess::m_bRedraw)) return;
		const POINT& pos = jauge.*JaugeAccess::m_pos;
		const POINT& dim = jauge.*JaugeAccess::m_dim;
		Item& it = Add(Item::Op::JaugeDraw, t);
		it.index = i;
		it.mask = jauge.*JaugeAccess::m_bHide;
		it.mode = jauge.*JaugeAccess::m_type;
		it.rank = jauge.*JaugeAccess::m_level;
		it.pos = pos;
		it.rect = RECT{pos.x, pos.y, pos.x + dim.x, pos.y + dim.y};
	}

	Item& Last() { return m_scene.items.back(); }

private:
	Item& Add(Item::Op op, Tag t)
	{
		m_scene.items.emplace_back();
		Item& it = m_scene.items.back();
		it.op = op;
		it.layer = t.layer;
		it.entity = t.entity;
		it.index = t.index;
		it.cull = t.cull;
		it.ref = t.ref;
		it.simCamera = t.simCamera;
		it.extended = t.extended;
		return it;
	}
	void Place(Item& it, POINT pos, Tag t)
	{
		it.pos = pos;
		if (t.layer == Layer::World)
		{
			POINT origin = t.simCamera ? m_scene.camera : m_scene.view;
			it.world.x = pos.x + origin.x;
			it.world.y = pos.y + origin.y;
		}
	}

	Scene& m_scene;
};

// Text functions of src/text.h, recorded (same arguments plus a Tag).
void DrawText(Recorder* pPixmap, POINT pos, const char* pText, int font, Tag t)
{
	pPixmap->Text(Item::Op::DrawText, pos, pText, font, t);
}

void DrawTextCenter(Recorder* pPixmap, POINT pos, const char* pText, int font, Tag t)
{
	pPixmap->Text(Item::Op::DrawTextCenter, pos, pText, font, t);
}

// Copy of CJauge::GetHide (src/jauge.cpp), const.
BOOL GetHide(const CJauge& jauge)
{
	return jauge.*JaugeAccess::m_bHide;
}

// Copy of CDecor::GetBlupiChannelActual (src/decblock.cpp), const.
int GetBlupiChannelActual(const CDecor& decor)
{
	const BOOL& m_bMulti = decor.*DecorAccess::m_bMulti;
	const int& m_blupiChannel = decor.*DecorAccess::m_blupiChannel;
	const int& m_team = decor.*DecorAccess::m_team;

	if (m_bMulti && m_blupiChannel == 2 && m_team > 0) {
		return m_team + CHBLUPI1 - 1;
	}
	else return m_blupiChannel;
}

// Copy of CDecor::GetBlupiChannelStandard (src/decblock.cpp), const.
int GetBlupiChannelStandard(const CDecor& decor)
{
	const BOOL& m_bMulti = decor.*DecorAccess::m_bMulti;
	const int& m_team = decor.*DecorAccess::m_team;

	if (m_bMulti && m_team > 0) {
		return m_team + CHBLUPI1 - 1;
	}
	else return 2;
}

// Copy of CDecor::GetIconPerso (src/decblock.cpp), const.
int GetIconPerso(const CDecor& decor)
{
	const BOOL& m_bMulti = decor.*DecorAccess::m_bMulti;
	const int& m_team = decor.*DecorAccess::m_team;

	if (m_bMulti) return m_team + 0x6c;
	return 0x6c;
}

// Derived from CDecor::DecorNextAction (src/decblock.cpp): returns the camera
// shaken by the current decor action. The two state changes of the original
// (m_decorPhase++, m_decorAction = 0) are not made; nothing later in Build
// reads them.
POINT DecorNextAction(const CDecor& decor)
{
	const int& m_decorAction = decor.*DecorAccess::m_decorAction;
	const int& m_decorPhase = decor.*DecorAccess::m_decorPhase;
	const POINT& m_posDecor = decor.*DecorAccess::m_posDecor;
	const POINT& m_dimDecor = decor.*DecorAccess::m_dimDecor;

	int num, num2;
	POINT posDecor;
	num = 0;
	if (m_decorAction == 0)
	{
		return m_posDecor;
	}
	posDecor = m_posDecor;
	while (table_decor_action[num] != 0)
	{
		if (m_decorAction == table_decor_action[num])
		{
			if (m_decorPhase < table_decor_action[num + 1])
			{
				posDecor.x += 3 * table_decor_action[num + 2 + m_decorPhase * 2];
				posDecor.y += 3 * table_decor_action[num + 2 + m_decorPhase * 2 + 1];
				if (m_dimDecor.x == 0)
				{
					num2 = 0;
				}
				else
				{
					num2 = DIMOBJX * MAXCELX - LXIMAGE;
				}
				if (posDecor.x < 0)
				{
					posDecor.x = 0;
				}
				if (posDecor.x > num2)
				{
					posDecor.x = num2;
				}
				if (m_dimDecor.y == 0)
				{
					num2 = 0;
				}
				else
				{
					num2 = DIMOBJY * MAXCELY - LYIMAGE;
				}
				if (posDecor.y < 0)
				{
					posDecor.y = 0;
				}
				if (posDecor.y > num2)
				{
					posDecor.y = num2;
				}
				// m_decorPhase++;  (state change, ROADMAP.md stage 4)
				break;
			}
			// m_decorAction = 0;  (state change, ROADMAP.md stage 4)
			break;
		}
		else
		{
			num += 2 + table_decor_action[num + 1] * 2;
		}
	}
	return posDecor;
}

// Derived from CDecor::DrawInfo (src/decblock.cpp): the HUD. The mission-10
// clamp of m_blupiPos.x applies to a local copy, i.e. only to the tutorial
// lookup that follows it here (Blupi was drawn before with the unclamped
// position, as in the original).
void DrawInfo(const CDecor& decor, Recorder* m_pPixmap)
{
	const int& m_phase = decor.*DecorAccess::m_phase;
	const char (&m_notifText)[MAXNOTIF][100] = decor.*DecorAccess::m_notifText;
	const int& m_nbVies = decor.*DecorAccess::m_nbVies;
	const int& m_blupiBullet = decor.*DecorAccess::m_blupiBullet;
	const int& m_blupiPerso = decor.*DecorAccess::m_blupiPerso;
	const int& m_mission = decor.*DecorAccess::m_mission;
	const BOOL& m_bPrivate = decor.*DecorAccess::m_bPrivate;
	const int& m_nbTresor = decor.*DecorAccess::m_nbTresor;
	const int& m_totalTresor = decor.*DecorAccess::m_totalTresor;
	const CJauge (&m_jauges)[2] = decor.*DecorAccess::m_jauges;
	POINT m_blupiPos = decor.*DecorAccess::m_blupiPos;  // local copy, see above
	const BOOL& m_bJoystick = decor.*DecorAccess::m_bJoystick;
	const POINT& m_posCelHili = decor.*DecorAccess::m_posCelHili;
	const POINT& m_dimCelHili = decor.*DecorAccess::m_dimCelHili;
	const POINT& m_posDecor = decor.*DecorAccess::m_posDecor;
	const int& m_liftEnd = decor.*DecorAccess::m_liftEnd;

	POINT pos;
	char text[100];
	int i, x, y, icon;

	if (m_phase == WM_PHASE_PLAY || m_phase == WM_PHASE_PLAYTEST)
	{
		pos.x = 10;
		pos.y = 10;
		for (i = 0; i < MAXNOTIF; i++)
		{
			if (m_notifText[i][0] != '\0')
			{
				DrawText(m_pPixmap, pos, m_notifText[i], FONTWHITE, AtScreen(Entity::Text, i));
			}
			pos.y += DIMTEXTY;
		}

		pos.x = -5;
		pos.y = 417;
		for (i = 0; i < m_nbVies; i++)
		{
			m_pPixmap->QuickIcon(GetBlupiChannelStandard(decor), 48, pos, AtScreen(Entity::Hud));
			pos.x += 16;
		}

		pos.x = 420;
		pos.y = 442;
		for (i = 0; i < m_blupiBullet; i++)
		{
			m_pPixmap->QuickIcon(CHELEMENT, 176, pos, AtScreen(Entity::Hud));
			pos.x += 6;
		}

		if (m_blupiPerso > 0)
		{
			pos.x = 520;
			pos.y = 438;
			m_pPixmap->QuickIcon(CHBLUPI, GetIconPerso(decor), pos, AtScreen(Entity::Hud));
			sprintf(text, "= %d", m_blupiPerso);
			pos.x = 552;
			pos.y = 452;
			DrawText(m_pPixmap, pos, text, FONTWHITE, AtScreen(Entity::Hud));
		}

		if ((m_mission != 1 && m_mission % 10 != 0) || m_bPrivate)
		{
			sprintf(text, "%d/%d", m_nbTresor, m_totalTresor);
			pos.x = 590;
			pos.y = 452;
			DrawText(m_pPixmap, pos, text, FONTWHITE, AtScreen(Entity::Hud));
		}

		for (i = 0; i < 2; i++)
		{
			if (!GetHide(m_jauges[i])) m_pPixmap->JaugeDraw(m_jauges[i], i, AtScreen(Entity::Jauge, i));
		}

		if (m_mission == 10 && m_phase == WM_PHASE_PLAY && !m_bPrivate)
		{
			// State change in the original (gameplay inside drawing, ROADMAP.md
			// stage 4); here it changes the local copy only.
			if (m_nbTresor == 0 && m_blupiPos.x > 212) m_blupiPos.x = 212;
			if (m_nbTresor == 1 && m_blupiPos.x > 788) m_blupiPos.x = 788;

			POINT cel = { (m_blupiPos.x + DIMBLUPIX / 2) / DIMOBJX,
				(m_blupiPos.y + DIMBLUPIY / 2) / DIMOBJY };
			for (i = 0; table_tutorial[i] != -1; i += 6)
			{
				if (cel.x >= table_tutorial[i] &&
					cel.x <= table_tutorial[i + 1] &&
					cel.y >= table_tutorial[i + 2] &&
					cel.y <= table_tutorial[i + 3] &&
					(table_tutorial[i + 4] == -1 || m_nbTresor == table_tutorial[i + 4]))
				{
					if (m_bJoystick)
						LoadString(table_tutorial[i + 5] + 100, text, 100);
					else
						LoadString(table_tutorial[i + 5], text, 100);
					pos.x = 360;
					pos.y = 460;
					DrawTextCenter(m_pPixmap, pos, text, 0 /* text.h default */, AtScreen(Entity::Text));
					break;
				}
			}
		}
	}

	// Editor; not covered by the gate.
	if (m_phase == WM_PHASE_BUILD)
	{
		if (m_posCelHili.x != -1)
		{
			// Placed with m_posDecor, not with the shaken camera.
			pos.x = m_posCelHili.x * DIMOBJX - m_posDecor.x;
			pos.y = m_posCelHili.y * DIMOBJY - m_posDecor.y;
			icon = 0;
			if (m_liftEnd != -1) icon = 31;

			for (x = 0; x < m_dimCelHili.x; x++)
			{
				for (y = 0; y < m_dimCelHili.y; y++)
				{
					{
						POINT iconPos = { pos.x + x * DIMOBJX, pos.y + y * DIMOBJY };
						m_pPixmap->QuickIcon(1, icon,
						iconPos, AtWorldSim(Entity::CellHili));
					}
				}
			}
		}
		if (m_phase == WM_PHASE_BUILD)
		{
			LoadString(0x66, text, 100);
			pos.x = 200;
			pos.y = 460;
			DrawText(m_pPixmap, pos, text, FONTGOLD, AtScreen(Entity::Text));
		}
	}

	if (m_phase == WM_PHASE_PLAYTEST)
	{
		LoadString(0x67, text, 100);
		pos.x = 200;
		pos.y = 460;
		DrawText(m_pPixmap, pos, text, FONTGOLD, AtScreen(Entity::Text));
	}
}

// Derived from CDecor::VoyageDraw (src/decor.cpp): the flying pickup icon.
void VoyageDraw(const CDecor& decor, Recorder* m_pPixmap)
{
	const int& m_voyageIcon = decor.*DecorAccess::m_voyageIcon;
	const int& m_voyageChannel = decor.*DecorAccess::m_voyageChannel;
	const int& m_voyagePhase = decor.*DecorAccess::m_voyagePhase;
	const int& m_voyageTotal = decor.*DecorAccess::m_voyageTotal;
	const POINT& m_voyageStart = decor.*DecorAccess::m_voyageStart;
	const POINT& m_voyageEnd = decor.*DecorAccess::m_voyageEnd;

	if (m_voyageIcon != -1)
	{
		POINT pos;
		pos.x = m_voyageStart.x + (m_voyageEnd.x - m_voyageStart.x) * m_voyagePhase / m_voyageTotal;
		pos.y = m_voyageStart.y + (m_voyageEnd.y - m_voyageStart.y) * m_voyagePhase / m_voyageTotal;
		m_pPixmap->QuickIcon(m_voyageChannel, m_voyageIcon, pos, AtScreen(Entity::Voyage));
		Item& voyage = m_pPixmap->Last();   // for wider views (DrawView)
		voyage.rect = RECT{m_voyageStart.x, m_voyageStart.y, m_voyageEnd.x, m_voyageEnd.y};
		voyage.mode = m_voyagePhase;
		voyage.font = m_voyageTotal;
	}
}
}  // namespace

// Derived from CDecor::Build (src/decblock.cpp). Uses the value m_time had
// before Build; the state changes of the original (m_blupiSec, m_time++ and
// those inside DecorNextAction/DrawInfo) are not made.
void BuildScene(const CDecor& decor, RECT rect, POINT mousePos, Scene& out, const Coverage& coverage)
{
	(void)mousePos;  // unused by Build too
	const int& m_phase = decor.*DecorAccess::m_phase;
	const int& m_time = decor.*DecorAccess::m_time;
	const Cellule (&m_decor)[MAXCELX][MAXCELY] = decor.*DecorAccess::m_decor;
	const Cellule (&m_bigDecor)[MAXCELX][MAXCELY] = decor.*DecorAccess::m_bigDecor;
	const MoveObject (&m_moveObject)[MAXMOVEOBJECT] = decor.*DecorAccess::m_moveObject;
	const POINT (&m_blupiStartPos)[4] = decor.*DecorAccess::m_blupiStartPos;
	const int (&m_blupiStartDir)[4] = decor.*DecorAccess::m_blupiStartDir;
	const BOOL& m_bMulti = decor.*DecorAccess::m_bMulti;
	CNetwork* const& m_pNetwork = decor.*DecorAccess::m_pNetwork;
	const POINT (&m_netBlupiPos)[MAXNETPLAYER] = decor.*DecorAccess::m_netBlupiPos;
	const int (&m_netBlupiIcon)[MAXNETPLAYER] = decor.*DecorAccess::m_netBlupiIcon;
	const int (&m_netBlupiSec)[MAXNETPLAYER] = decor.*DecorAccess::m_netBlupiSec;
	const int (&m_netBlupiTransport)[MAXNETPLAYER] = decor.*DecorAccess::m_netBlupiTransport;
	const POINT& m_blupiPos = decor.*DecorAccess::m_blupiPos;
	const int& m_blupiIcon = decor.*DecorAccess::m_blupiIcon;
	const BOOL& m_blupiShield = decor.*DecorAccess::m_blupiShield;
	const BOOL& m_blupiPower = decor.*DecorAccess::m_blupiPower;
	const BOOL& m_blupiCloud = decor.*DecorAccess::m_blupiCloud;
	const BOOL& m_blupiHide = decor.*DecorAccess::m_blupiHide;
	const BOOL& m_blupiFront = decor.*DecorAccess::m_blupiFront;
	const int& m_blupiTimeShield = decor.*DecorAccess::m_blupiTimeShield;
	const BOOL& m_bDrawSecret = decor.*DecorAccess::m_bDrawSecret;

	out.items.clear();
	Recorder recorder(out);
	Recorder* const m_pPixmap = &recorder;

	int i, j, num, channel;
	POINT tinyPoint, pos, posDecor;
	RECT lastClip;

	// CPixmap::GetClipping only returns the clip rectangle.
	lastClip = (decor.*DecorAccess::m_pPixmap)->GetClipping();
	m_pPixmap->SetClipping(rect, AtScreen(Entity::None));

	posDecor = DecorNextAction(decor);
	out.camera = decor.*DecorAccess::m_posDecor;
	out.view = posDecor;
	out.phase = m_phase;
	{
		// The bounds DecorNextAction clamps the shaken view to.
		const POINT& m_dimDecor = decor.*DecorAccess::m_dimDecor;
		out.maxView.x = m_dimDecor.x == 0 ? 0 : DIMOBJX * MAXCELX - LXIMAGE;
		out.maxView.y = m_dimDecor.y == 0 ? 0 : DIMOBJY * MAXCELY - LYIMAGE;
	}

	// Parallax background.
	{
		POINT dest[kMaxParallaxParts];
		RECT  src[kMaxParallaxParts];
		POINT image = {posDecor.x * 2 / 3, posDecor.y * 2 / 3};
		int   parts = ParallaxParts(image, LXIMAGE, dest, src, kMaxParallaxParts);
		for (i = 0; i < parts; i++)
		{
			m_pPixmap->DrawPart(-1, CHDECOR, dest[i], src[i], 1, FALSE, AtBack(i));
		}
	}

	// Big decor tiles. Build's loop, i from posDecor.x / DIMOBJX - 1 to
	// posDecor.x / DIMOBJX + LXIMAGE / DIMOBJX + 3 (exclusive) and j from
	// posDecor.y / DIMOBJY - 1 to posDecor.y / DIMOBJY + LYIMAGE / DIMOBJY + 2,
	// at x = i * DIMOBJX - posDecor.x and y = j * DIMOBJY - posDecor.y + 2;
	// widened by the coverage (cells outside Build's range are extended).
	{
		const int i0 = FloorDiv(posDecor.x - coverage.left, DIMOBJX) - 1;
		const int i1 = FloorDiv(posDecor.x + coverage.right, DIMOBJX) + LXIMAGE / DIMOBJX + 3;
		const int j0 = FloorDiv(posDecor.y - coverage.top, DIMOBJY) - 1;
		const int j1 = FloorDiv(posDecor.y + coverage.bottom, DIMOBJY) + LYIMAGE / DIMOBJY + 2;
		for (i = i0; i < i1; i++) {
			tinyPoint.x = i * DIMOBJX - posDecor.x;
			for (j = j0; j < j1; j++)
			{
				tinyPoint.y = j * DIMOBJY - posDecor.y + 2;
				if (i >= 0 && i < MAXCELX && j >= 0 && j < MAXCELY)
				{
					num = m_bigDecor[i][j].icon;
					channel = 9;
					if (num != -1)
					{
						if (num == 203)
						{
							num = table_marine[m_time / 3 % 11];
							channel = 1;
						}
						m_pPixmap->QuickIcon(channel, num, tinyPoint, AtCell(Entity::BigDecor, Item::Cull::BigCell, i, j, posDecor));
					}
				}
			}
		}
	}

	// Editor start markers; not covered by the gate. i = 4 reads one element
	// past m_blupiStartPos/m_blupiStartDir, exactly like the original.
	if (m_phase == WM_PHASE_BUILD)
	{
		for (i = 4; i >= 0; i--)
		{
			channel = 2;
			if (i > 0) channel = CHBLUPI1 - 1 + i;
			num = 4;
			if (m_blupiStartDir[i] == DIR_RIGHT) num = 0;
			tinyPoint.x = m_blupiStartPos[i].x - posDecor.x;
			tinyPoint.y = m_blupiStartPos[i].y - posDecor.y;
			m_pPixmap->QuickIcon(channel, num, tinyPoint, AtWorld(Entity::StartMarker, i));
			tinyPoint.x += 10;
			tinyPoint.x += i * 8 - 12;
			tinyPoint.y -= 20;
			m_pPixmap->QuickIcon(CHBUTTON, i + 120, tinyPoint, AtWorld(Entity::StartMarker, i));
		}
	}

	// Network Blupis; not covered by the gate.
	if (m_bMulti && m_phase != WM_PHASE_BUILD)
	{
		for (i = 0; i < 4; i++)
		{
			if (m_netBlupiIcon[i] != -1 && (m_netBlupiIcon[i] < 276 || m_netBlupiIcon[i] > 284) && m_netBlupiSec[i] != SEC_HIDE)
			{
				tinyPoint.x = m_netBlupiPos[i].x - posDecor.x;
				tinyPoint.y = m_netBlupiPos[i].y - posDecor.y;
				if (m_netBlupiSec[i] == SEC_SHIELD)
				{
					pos.x = tinyPoint.x;
					pos.y = tinyPoint.y + 2;
					m_pPixmap->QuickIcon(CHELEMENT, table_shield_blupi[m_time / 2 % 4], pos, AtWorld(Entity::NetBlupi, i));
				}
				if (m_netBlupiSec[i] == SEC_POWER)
				{
					m_pPixmap->QuickIcon(CHELEMENT, table_magicloop[m_time / 2 % 5], tinyPoint, AtWorld(Entity::NetBlupi, i));
				}
				if (m_netBlupiSec[i] == SEC_CLOUD)
				{
					for (j = 0; j < 3; j++)
					{
						pos.x = tinyPoint.x - 34;
						pos.y = tinyPoint.y - 34;
						m_pPixmap->QuickIcon(CHEXPLO, (i + j + m_time) % 6 + 48, pos, AtWorld(Entity::NetBlupi, i));
					}
				}
				int blupiChannel = m_netBlupiTransport[i];
				int team = m_pNetwork->m_players[i].team;
				if (blupiChannel == 2 && team > 0) blupiChannel = CHBLUPI1 - 1 + team;
				m_pPixmap->QuickIcon(blupiChannel, m_netBlupiIcon[i], tinyPoint, AtWorld(Entity::NetBlupi, i));
				pos.x = tinyPoint.x + 10;
				pos.y = tinyPoint.y - 20;
				m_pPixmap->QuickIcon(CHBUTTON, team + 120, pos, AtWorld(Entity::NetBlupi, i));
			}
		}
	}

	// Blupi behind the decor, with shield/power/cloud/hide effects.
	if (m_phase != WM_PHASE_BUILD)
	{
		// m_blupiSec = 0;  (state change, ROADMAP.md stage 4; the SEC_* values
		// set below are not read by the rest of Build)
		if (!m_blupiFront)
		{
			tinyPoint.x = m_blupiPos.x - posDecor.x;
			long screenY = m_blupiPos.y - posDecor.y;
			tinyPoint.y = screenY;
			pos.y = screenY - 34;
			if (m_blupiShield)
			{
				// m_blupiSec = SEC_SHIELD;
				if (m_blupiTimeShield > 25 || m_time % 4 < 2)
				{
					num = table_shield_blupi[m_time / 2 % 4];
					tinyPoint.y += 2;
					m_pPixmap->QuickIcon(CHELEMENT, num, tinyPoint, AtWorld(Entity::BlupiEffect));
					tinyPoint.y -= 2;
				}
				blupi_normal:
				m_pPixmap->QuickIcon(GetBlupiChannelActual(decor), m_blupiIcon, tinyPoint, AtWorld(Entity::Blupi));
			}
			else if (m_blupiPower)
			{
				// m_blupiSec = SEC_POWER;
				if (m_blupiTimeShield > 25 || m_time % 4 < 2)
				{
					num = table_magicloop[m_time / 2 % 5];
					m_pPixmap->QuickIcon(CHELEMENT, num, tinyPoint, AtWorld(Entity::BlupiEffect));
				}
				m_pPixmap->QuickIcon(GetBlupiChannelActual(decor), m_blupiIcon, tinyPoint, AtWorld(Entity::Blupi));
			}
			else if (m_blupiCloud)
			{
				// m_blupiSec = SEC_CLOUD;
				if (m_blupiTimeShield > 25 || m_time % 4 < 2)
				{
					for (j = 0; j < 3; j++)
					{
						pos.x = tinyPoint.x - 34;
						m_pPixmap->QuickIcon(CHEXPLO, (j + m_time) % 6 + 48, pos, AtWorld(Entity::BlupiEffect, j));
					}
				}
				m_pPixmap->QuickIcon(GetBlupiChannelActual(decor), m_blupiIcon, tinyPoint, AtWorld(Entity::Blupi));
			}
			else if (m_blupiHide)
			{
				// m_blupiSec = SEC_HIDE;
				if (m_blupiTimeShield > 25 || m_time % 4 < 2)
				{
					// Composed in the 64x64 CHTEMP surface with mask keys.
					POINT maskOrigin = { 0, 0 };
					m_pPixmap->DrawIcon(CHTEMP, 1, 0xF5, maskOrigin, 0, TRUE, AtTarget(Entity::BlupiEffect));
					m_pPixmap->DrawIcon(CHTEMP, GetBlupiChannelActual(decor), m_blupiIcon, maskOrigin, 1, FALSE, AtTarget(Entity::Blupi));
					m_pPixmap->DrawIcon(CHTEMP, 1, 0xED, maskOrigin, 0, TRUE, AtTarget(Entity::BlupiEffect));
					m_pPixmap->QuickIcon(CHTEMP, 0, tinyPoint, AtWorld(Entity::Blupi));
				}
				else
				{
					m_pPixmap->QuickIcon(GetBlupiChannelActual(decor), m_blupiIcon, tinyPoint, AtWorld(Entity::Blupi));
				}
			}
			else
			{
				goto blupi_normal;
			}
		}
	}

	// Move objects (except types 1 and 8-11), last slot first.
	const MoveObject *pMoveObject = &m_moveObject[MAXMOVEOBJECT - 1];
	for (i = MAXMOVEOBJECT; i != 0; pMoveObject--, i--)
	{
		// Build: type == 0 || posCurrent outside [posDecor - 64, posDecor + LXIMAGE/LYIMAGE]
		if (pMoveObject->type == 0 || !Covered(pMoveObject->posCurrent, posDecor, coverage)) continue;
		if (pMoveObject->type >= 8 && pMoveObject->type <= 11) continue;
		if (pMoveObject->type == 1) continue;
		const int slot = (int)(pMoveObject - m_moveObject);
		const Tag object = AtPoint(Entity::MoveObject, slot, pMoveObject->posCurrent, posDecor);
		tinyPoint.x = pMoveObject->posCurrent.x - posDecor.x;
		tinyPoint.y = pMoveObject->posCurrent.y - posDecor.y;
		if (pMoveObject->type == TYPE_BULLDOZER || pMoveObject->type == TYPE_BLUPIHELICO || pMoveObject->type == TYPE_BLUPITANK)
		{
			tinyPoint.x += 2;
			tinyPoint.y += BLUPIOFFY;
		}
		m_pPixmap->QuickIcon(pMoveObject->channel, pMoveObject->icon, tinyPoint, object);
		if ((m_bDrawSecret || m_phase != WM_PHASE_PLAY) && pMoveObject->type == TYPE_CAISSE && pMoveObject->icon != 32 && pMoveObject->icon != 33 && pMoveObject->icon != 34)
		{
			m_pPixmap->QuickIcon(1, 214, tinyPoint, object);
		}
	}

	// Decor tiles. Build's loop, i from posDecor.x / 64 to posDecor.x / 64 +
	// LXIMAGE / 64 + 2 (exclusive), j likewise, at x = i * 64 - posDecor.x and
	// y = j * 64 - posDecor.y; widened by the coverage.
	const int cellI0 = FloorDiv(posDecor.x - coverage.left, DIMOBJX);
	const int cellI1 = FloorDiv(posDecor.x + coverage.right, DIMOBJX) + LXIMAGE / DIMOBJX + 2;
	const int cellJ0 = FloorDiv(posDecor.y - coverage.top, DIMOBJY);
	const int cellJ1 = FloorDiv(posDecor.y + coverage.bottom, DIMOBJY) + LYIMAGE / DIMOBJY + 2;
	for (i = cellI0; i < cellI1; i++)
	{
		tinyPoint.x = i * DIMOBJX - posDecor.x;
		for (j = cellJ0; j < cellJ1; j++)
		{
			tinyPoint.y = j * DIMOBJY - posDecor.y;
			if (i >= 0 && i < 100 && j >= 0 && j < 100 && m_decor[i][j].icon != -1)
			{
				num = m_decor[i][j].icon;
				pos.x = tinyPoint.x;
				pos.y = tinyPoint.y;
				if ((num >= 107 && num <= 109) || num == 157)
				{
					pos.y -= 7;
				}
				if (num == 211)
				{
					num = table_ressort[(m_time / 2 + i * 7) % 8];
				}
				if (num == 214 && !m_bDrawSecret && m_phase == WM_PHASE_PLAY)
				{
					num = -1;
				}
				if (num != 68 && num != 91 && num != 92 &&
					(num < 110 || num > 137))
				{
					m_pPixmap->QuickIcon(1, num, pos, AtCell(Entity::Decor, Item::Cull::Cell, i, j, posDecor));
				}
			}
		}
	}

	// Lifts; in the editor also the mover start/end marks (not covered by the gate).
	pMoveObject = m_moveObject;
	for (i = MAXMOVEOBJECT; i != 0; i--)
	{
		const int slot = (int)(pMoveObject - m_moveObject);
		// Build: posCurrent within [posDecor - 64, posDecor + LXIMAGE/LYIMAGE]
		if (pMoveObject->type == TYPE_ASCENSEUR &&
			Covered(pMoveObject->posCurrent, posDecor, coverage))
		{
			tinyPoint.x = pMoveObject->posCurrent.x - posDecor.x;
			tinyPoint.y = pMoveObject->posCurrent.y - posDecor.y;
			m_pPixmap->QuickIcon(pMoveObject->channel, pMoveObject->icon, tinyPoint, AtPoint(Entity::Lift, slot, pMoveObject->posCurrent, posDecor));
		}
		if (m_phase == WM_PHASE_BUILD &&
			(pMoveObject->type == TYPE_ASCENSEUR || pMoveObject->type == TYPE_BOMBEMOVE ||
			 pMoveObject->type == TYPE_BULLDOZER || pMoveObject->type == TYPE_POISSON ||
			 pMoveObject->type == TYPE_OISEAU || pMoveObject->type == TYPE_BLUPIHELICO ||
			 pMoveObject->type == TYPE_BLUPITANK))
		{
			if (Covered(pMoveObject->posEnd, posDecor, coverage))
			{
				tinyPoint.x = pMoveObject->posEnd.x - posDecor.x;
				tinyPoint.y = pMoveObject->posEnd.y - posDecor.y;
				m_pPixmap->QuickIcon(1, 31, tinyPoint, AtPoint(Entity::PathMarker, slot, pMoveObject->posEnd, posDecor));
			}
			if (Covered(pMoveObject->posStart, posDecor, coverage))
			{
				tinyPoint.x = pMoveObject->posStart.x - posDecor.x;
				tinyPoint.y = pMoveObject->posStart.y - posDecor.y;
				m_pPixmap->QuickIcon(1, 30, tinyPoint, AtPoint(Entity::PathMarker, slot, pMoveObject->posStart, posDecor));
			}
		}
		pMoveObject++;
	}

	// Animated overlay tiles (lava, water, wind, fans); same loop as the decor
	// tiles.
	for (i = cellI0; i < cellI1; i++)
	{
		tinyPoint.x = i * DIMOBJX - posDecor.x;
		for (j = cellJ0; j < cellJ1; j++)
		{
			tinyPoint.y = j * DIMOBJY - posDecor.y;
			if (i >= 0 && i < MAXCELX && j >= 0 && j < MAXCELY && m_decor[i][j].icon != -1)
			{
				const Tag cell = AtCell(Entity::Overlay, Item::Cull::Cell, i, j, posDecor);
				num = m_decor[i][j].icon;
				pos = tinyPoint;
				if (num == 68)
				{
					num = table_decor_lave[(i * 13 + j * 7 + m_time / 3) % 5];
					m_pPixmap->QuickIcon(1, num, pos, cell);
				}
				if (num == 92)
				{
					num = table_decor_eau1[(i * 13 + j * 7 + m_time / 3) % 4];
					m_pPixmap->QuickIcon(1, num, pos, cell);
				}
				if (num == 91)
				{
					num = table_decor_eau2[(i * 13 + j * 7 + m_time / 3) % 4];
					m_pPixmap->QuickIcon(1, num, pos, cell);
				}
				if (num == 110)
				{
					num = table_decor_ventg[m_time / 1 % 4];
					m_pPixmap->QuickIcon(1, num, pos, cell);
				}
				if (num == 114)
				{
					num = table_decor_ventd[m_time / 1 % 4];
					m_pPixmap->QuickIcon(1, num, pos, cell);
				}
				if (num == 118)
				{
					num = table_decor_venth[m_time / 1 % 4];
					m_pPixmap->QuickIcon(1, num, pos, cell);
				}
				if (num == 122)
				{
					num = table_decor_ventb[m_time / 1 % 4];
					m_pPixmap->QuickIcon(1, num, pos, cell);
				}
				if (num == 126)
				{
					num = table_decor_ventillog[m_time / 2 % 3];
					m_pPixmap->QuickIcon(1, num, pos, cell);
				}
				if (num == 129)
				{
					num = table_decor_ventillod[m_time / 2 % 3];
					m_pPixmap->QuickIcon(1, num, pos, cell);
				}
				if (num == 132)
				{
					num = table_decor_ventilloh[m_time / 2 % 3];
					m_pPixmap->QuickIcon(1, num, pos, cell);
				}
				if (num == 135)
				{
					num = table_decor_ventillob[m_time / 2 % 3];
					m_pPixmap->QuickIcon(1, num, pos, cell);
				}
			}
		}
	}

	// Move objects of types 8-11, in front of the decor.
	// Build: posCurrent within [posDecor - DIMOBJX/DIMOBJY, posDecor + LXIMAGE/LYIMAGE]
	for (i = 0; i < MAXMOVEOBJECT; i++)
	{
		if (m_moveObject[i].type != 0 &&
			Covered(m_moveObject[i].posCurrent, posDecor, coverage) &&
			m_moveObject[i].type >= 8 && m_moveObject[i].type <= 11)
		{
			tinyPoint.x = m_moveObject[i].posCurrent.x - posDecor.x;
			tinyPoint.y = m_moveObject[i].posCurrent.y - posDecor.y;
			m_pPixmap->QuickIcon(m_moveObject[i].channel, m_moveObject[i].icon, tinyPoint, AtPoint(Entity::MoveObject, i, m_moveObject[i].posCurrent, posDecor));
		}

	}

	// Blupi in front of the decor.
	if (m_blupiFront && m_phase != WM_PHASE_BUILD)
	{
		tinyPoint.x = m_blupiPos.x - posDecor.x;
		tinyPoint.y = m_blupiPos.y - posDecor.y;
		m_pPixmap->QuickIcon(GetBlupiChannelActual(decor), m_blupiIcon, tinyPoint, AtWorld(Entity::Blupi));
	}
	DrawInfo(decor, m_pPixmap);
	VoyageDraw(decor, m_pPixmap);
	m_pPixmap->SetClipping(lastClip, AtScreen(Entity::None));
	// m_time++; if (m_time >= 32768) m_time = 0;  (state change, ROADMAP.md stage 4)
}

namespace
{
// Issues one item's CPixmap call with the item drawn at pos.
void Issue(const Item& item, POINT pos, CPixmap* pixmap, std::string& text)
{
	commands::Issue(item, pos, pixmap, text, CHBACK, CHJAUGE, DIMJAUGEX, DIMJAUGEY,
		[](CPixmap* p, POINT at, char* str, int font) { ::DrawText(p, at, str, font); },
		[](CPixmap* p, POINT at, char* str, int font) { ::DrawTextCenter(p, at, str, font); });
}

// Entities whose position changes between ticks (the rest is static or
// fixed on screen).
bool Moves(const Item& item)
{
	switch (item.entity)
	{
	case Entity::NetBlupi:
	case Entity::Blupi:
	case Entity::BlupiEffect:
	case Entity::MoveObject:
	case Entity::Lift:
		return item.layer == Layer::World;
	case Entity::Voyage:
		return item.layer == Layer::Screen;
	default:
		return false;
	}
}

POINT Where(const Item& item)
{
	return item.layer == Layer::World ? item.world : item.pos;
}

long Mix(long from, long to, double alpha)
{
	return from + lround((to - from) * alpha);
}
}  // namespace

void FlattenScene(const Scene& scene, CPixmap* pixmap)
{
	std::string text;
	commands::Flatten(scene.items, [&](const Item& item) { Issue(item, item.pos, pixmap, text); });
}

bool Visible(const Item& item, POINT ref, POINT view, int width, int height)
{
	return VisibleRule(item.cull, item.index, ref, view, width, height);
}

void PairScenes(const Scene& prev, const Scene& curr, Motion& out)
{
	out.valid = false;
	out.cameraFrom = curr.camera;
	out.from.resize(curr.items.size());
	for (size_t n = 0; n < curr.items.size(); n++) out.from[n] = Where(curr.items[n]);

	if (prev.items.empty() || prev.phase != curr.phase) return;
	if (labs(curr.camera.x - prev.camera.x) > kSnapDistance ||
		labs(curr.camera.y - prev.camera.y) > kSnapDistance) return;
	out.valid = true;
	out.cameraFrom = prev.camera;

	// Identity: entity, index and the occurrence of that pair in drawing order.
	auto base = [](const Item& it) { return ((uint64_t)it.entity << 32) | (uint32_t)it.index; };
	std::unordered_map<uint64_t, int> count;
	std::unordered_map<uint64_t, POINT> previous;
	for (const Item& it : prev.items)
	{
		if (!Moves(it)) continue;
		uint64_t key = base(it);
		int occurrence = count[key]++;
		previous[key ^ ((uint64_t)occurrence << 48)] = Where(it);
	}
	count.clear();
	for (size_t n = 0; n < curr.items.size(); n++)
	{
		const Item& it = curr.items[n];
		if (!Moves(it)) continue;
		uint64_t key = base(it);
		int occurrence = count[key]++;
		auto found = previous.find(key ^ ((uint64_t)occurrence << 48));
		if (found == previous.end()) continue;   // appeared: snap
		POINT was = found->second, now = Where(it);
		if (labs(now.x - was.x) > kSnapDistance || labs(now.y - was.y) > kSnapDistance) continue;  // jumped: snap
		out.from[n] = was;
	}
}

ViewPlacement PlaceView(const Scene& scene, POINT view, int width)
{
	ViewPlacement place;
	const int worldWidth = scene.maxView.x + LXIMAGE;
	const int frameX = FrameOffset(width, false);
	place.camera.y = view.y;
	if (worldWidth >= width)
	{
		// Centred on the frame, clamped to the world.
		place.camera.x = view.x - frameX;
		if (place.camera.x > worldWidth - width) place.camera.x = worldWidth - width;
		if (place.camera.x < 0) place.camera.x = 0;
	}
	else
	{
		// The whole world, centred.
		place.camera.x = (worldWidth - width) / 2;
	}
	place.frameX = view.x - place.camera.x;
	place.left = place.camera.x < 0 ? -place.camera.x : 0;
	place.right = worldWidth - place.camera.x < width ? worldWidth - place.camera.x : width;
	return place;
}

void DrawView(const Scene& scene, const Motion* motion, double alpha, CPixmap* pixmap, const ViewSpec& spec)
{
	const bool lerp = motion != NULL && motion->valid && motion->from.size() == scene.items.size() && alpha < 1.0;
	const bool wide = spec.width != LXIMAGE;
	if (alpha < 0.0) alpha = 0.0;

	// The camera, interpolated; the current tick's shake on top of it,
	// clamped like DecorNextAction clamps it.
	POINT camera = scene.camera, view = scene.view;
	if (lerp)
	{
		camera.x = Mix(motion->cameraFrom.x, scene.camera.x, alpha);
		camera.y = Mix(motion->cameraFrom.y, scene.camera.y, alpha);
		view.x = camera.x + scene.view.x - scene.camera.x;
		view.y = camera.y + scene.view.y - scene.camera.y;
		if (view.x > scene.maxView.x) view.x = scene.maxView.x;
		if (view.x < 0) view.x = 0;
		if (view.y > scene.maxView.y) view.y = scene.maxView.y;
		if (view.y < 0) view.y = 0;
	}

	// The view around the frame (at the faithful size: the frame itself).
	const ViewPlacement place = wide ? PlaceView(scene, view, spec.width) : ViewPlacement{view, 0, 0, LXIMAGE};
	// The screen layer: the centred frame, or the view's left edge (the
	// editor's toolbar). The parallax background is placed by the view camera,
	// relative to where the centred frame is.
	const int anchorX = FrameOffset(spec.width, false);
	const int hudX = spec.uiAtFrame ? place.frameX : FrameOffset(spec.width, spec.uiLeft);
	const POINT cullView = spec.cullAtFrame ? view : place.camera;
	const int cullWidth = spec.cullAtFrame ? LXIMAGE : spec.width;

	std::string text;
	bool parallax = false, clip = false;
	for (size_t n = 0; n < scene.items.size(); n++)
	{
		const Item& item = scene.items[n];
		POINT pos = item.pos;
		if (item.op == Item::Op::SetClip && wide)
		{
			// Build's clip is the whole frame: here the part of the view that
			// shows the world. The caller restores the clip afterwards.
			if (!clip) pixmap->SetClipping(RECT{place.left, 0, place.right, LYIMAGE});
			clip = true;
			continue;
		}
		if (item.layer == Layer::Background)
		{
			// Regenerated for the drawn view (Build's algorithm), at the same
			// place relative to the frame.
			if (parallax) continue;
			parallax = true;
			POINT dest[kMaxParallaxParts];
			RECT  src[kMaxParallaxParts];
			// The view camera decides it, as Build's camera does: the image as
			// Build would place it for a frame centred in this view, extended
			// sideways. Where the view is centred on the frame this is Build's
			// own position; where the view is clamped at a world edge, the
			// background stays still with the world.
			POINT image = {(place.camera.x + anchorX) * 2 / 3 - anchorX, view.y * 2 / 3};
			int   parts = ParallaxParts(image, spec.width, dest, src, kMaxParallaxParts);
			for (int i = 0; i < parts; i++) pixmap->DrawPart(item.chDst, item.channel, dest[i], src[i], item.mode, item.mask);
			continue;
		}
		if (item.layer == Layer::World)
		{
			POINT world = item.world;
			if (lerp)
			{
				world.x = Mix(motion->from[n].x, item.world.x, alpha);
				world.y = Mix(motion->from[n].y, item.world.y, alpha);
			}
			if (item.cull != Item::Cull::None)
			{
				POINT ref = {item.ref.x + world.x - item.world.x, item.ref.y + world.y - item.world.y};
				if (!Visible(item, ref, cullView, cullWidth, LYIMAGE)) continue;
			}
			// Items placed with the unshaken camera keep their offset from the
			// shaken ones (the frame moves with the shake).
			pos.x = world.x - (item.simCamera ? camera.x : view.x) + place.frameX;
			pos.y = world.y - (item.simCamera ? camera.y : view.y);
		}
		else if (item.extended)
		{
			continue;
		}
		else if (item.layer == Layer::Screen)
		{
			if (lerp && item.entity == Entity::Voyage)
			{
				pos.x = Mix(motion->from[n].x, item.pos.x, alpha);
				pos.y = Mix(motion->from[n].y, item.pos.y, alpha);
			}
			if (item.entity == Entity::Voyage && item.font > 0)
			{
				// From the object (frame placement) to the HUD (centred frame).
				pos.x += place.frameX + (hudX - place.frameX) * item.mode / item.font;
			}
			else
			{
				pos.x += hudX;
			}
		}
		Issue(item, pos, pixmap, text);
	}
}
}  // namespace blupi::scene
