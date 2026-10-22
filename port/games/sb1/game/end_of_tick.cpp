// EndOfTick: the state changes CDecor::Build made while drawing
// (port/docs/ROADMAP.md stage 4).
//
// In the original, Build (src/decblock.cpp) both drew the frame and changed
// game state. The port draws from the side-effect-free scene builder
// (game/scene.cpp) and applies those state changes here, at the moment Build
// would have run (once per tick, from SetDecor), in Build's order:
//
//   1. DecorNextAction: the camera-shake step (m_decorPhase++ or m_decorAction = 0)
//   2. m_blupiSec, set from Blupi's effect flags while Blupi is drawn behind the decor
//   3. DrawInfo: m_jauges[i].Draw() clears each visible jauge's redraw flag,
//      then the mission-10 tutorial clamp of m_blupiPos.x
//   4. m_time++ (wrapping at 32768)
//
// [EVIDENCE] These are all writes the historical Build, DrawInfo,
// DecorNextAction and VoyageDraw make (audit of src/decblock.cpp and
// src/decor.cpp). Each step reads only state the earlier steps do not change,
// as in the original. The native-versus-reference gates compare the full
// simulation state hash every frame.

#include "def.h"
#include "decor.h"
#include "jauge.h"
#include "dectables.h"

#include "game/port_game.h"
#include "game/member_access.h"

namespace
{
#define TICK_DECOR_MEMBERS(X) \
	X(m_decorAction) X(m_decorPhase) X(m_phase) X(m_blupiSec) X(m_blupiFront) \
	X(m_blupiShield) X(m_blupiPower) X(m_blupiCloud) X(m_blupiHide) \
	X(m_jauges) X(m_mission) X(m_bPrivate) X(m_nbTresor) X(m_blupiPos) \
	X(m_time)
BLUPI_MEMBER_ACCESS(DecorWrite, CDecor, TICK_DECOR_MEMBERS)

#define TICK_JAUGE_MEMBERS(X) X(m_bHide) X(m_bRedraw)
BLUPI_MEMBER_ACCESS(JaugeWrite, CJauge, TICK_JAUGE_MEMBERS)
}  // namespace

void EndOfTick(CDecor& decor)
{
	int&    m_decorAction = decor.*DecorWrite::m_decorAction;
	int&    m_decorPhase  = decor.*DecorWrite::m_decorPhase;
	const int& m_phase    = decor.*DecorWrite::m_phase;
	int&    m_blupiSec    = decor.*DecorWrite::m_blupiSec;
	const BOOL& m_blupiFront  = decor.*DecorWrite::m_blupiFront;
	const BOOL& m_blupiShield = decor.*DecorWrite::m_blupiShield;
	const BOOL& m_blupiPower  = decor.*DecorWrite::m_blupiPower;
	const BOOL& m_blupiCloud  = decor.*DecorWrite::m_blupiCloud;
	const BOOL& m_blupiHide   = decor.*DecorWrite::m_blupiHide;
	CJauge (&m_jauges)[2] = decor.*DecorWrite::m_jauges;
	const int&  m_mission  = decor.*DecorWrite::m_mission;
	const BOOL& m_bPrivate = decor.*DecorWrite::m_bPrivate;
	const int&  m_nbTresor = decor.*DecorWrite::m_nbTresor;
	POINT&  m_blupiPos     = decor.*DecorWrite::m_blupiPos;
	int&    m_time         = decor.*DecorWrite::m_time;
	int     num, i;

	// 1. CDecor::DecorNextAction (src/decblock.cpp), state changes only.
	if (m_decorAction != 0)
	{
		num = 0;
		while (table_decor_action[num] != 0)
		{
			if (m_decorAction == table_decor_action[num])
			{
				if (m_decorPhase < table_decor_action[num + 1])
				{
					m_decorPhase++;
					break;
				}
				m_decorAction = 0;
				break;
			}
			else
			{
				num += 2 + table_decor_action[num + 1] * 2;
			}
		}
	}

	// 2. CDecor::Build, Blupi drawn behind the decor.
	if (m_phase != WM_PHASE_BUILD)
	{
		m_blupiSec = 0;
		if (!m_blupiFront)
		{
			if (m_blupiShield)      m_blupiSec = SEC_SHIELD;
			else if (m_blupiPower)  m_blupiSec = SEC_POWER;
			else if (m_blupiCloud)  m_blupiSec = SEC_CLOUD;
			else if (m_blupiHide)   m_blupiSec = SEC_HIDE;
		}
	}

	// 3. CDecor::DrawInfo: CJauge::Draw clears m_bRedraw whether it draws or
	//    returns early, then the mission-10 clamp.
	if (m_phase == WM_PHASE_PLAY || m_phase == WM_PHASE_PLAYTEST)
	{
		for (i = 0; i < 2; i++)
		{
			if (!(m_jauges[i].*JaugeWrite::m_bHide)) m_jauges[i].*JaugeWrite::m_bRedraw = FALSE;
		}

		if (m_mission == 10 && m_phase == WM_PHASE_PLAY && !m_bPrivate)
		{
			if (m_nbTresor == 0 && m_blupiPos.x > 212) m_blupiPos.x = 212;
			if (m_nbTresor == 1 && m_blupiPos.x > 788) m_blupiPos.x = 788;
		}
	}

	// 4. End of CDecor::Build.
	m_time++;
	if (m_time >= 32768) m_time = 0;
}
