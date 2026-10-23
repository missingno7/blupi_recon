// SB2 EndOfTick: derivative of games/sb2/src/{decblock,decor}.cpp,
// extending SB1's game/end_of_tick.cpp. Called once where Build ran.
// Order: shake; Blupi secondary effect; visible drops/lightning (including
// sound/network); gauge flags/tutorial clamp; voyage RNG/particle; clock.
// BuildScene never performs these effects. Tests compare the complete state
// and rendering commands with the unchanged historical Build.
#include "def.h"
#include "decor.h"
#include "jauge.h"
#include "dectables.h"
#include "misc.h"

extern int table_decor_goutte[];

#include "game/port_game.h"
#include "game/member_access.h"

namespace
{
#define TICK_DECOR_MEMBERS(X) \
	X(m_decorAction) X(m_decorPhase) X(m_phase) X(m_blupiSec) X(m_blupiFront) \
	X(m_blupiShield) X(m_blupiPower) X(m_blupiCloud) X(m_blupiHide) \
	X(m_jauges) X(m_mission) X(m_bPrivate) X(m_nbTresor) X(m_blupiPos) \
	X(m_time) X(m_posDecor) X(m_dimDecor) X(m_bPause) X(m_decor) \
	X(m_voyageIcon) X(m_voyageChannel) \
	X(m_voyagePhase) X(m_voyageTotal) X(m_voyageStart) X(m_voyageEnd)
BLUPI_MEMBER_ACCESS(DecorWrite, CDecor, TICK_DECOR_MEMBERS)

#define TICK_JAUGE_MEMBERS(X) X(m_bHide) X(m_bRedraw)
BLUPI_MEMBER_ACCESS(JaugeWrite, CJauge, TICK_JAUGE_MEMBERS)

POINT TickDecorAction(CDecor& decor)
{
	int& m_decorAction = decor.*DecorWrite::m_decorAction;
	int& m_decorPhase = decor.*DecorWrite::m_decorPhase;
	const POINT& m_posDecor = decor.*DecorWrite::m_posDecor;
	const POINT& m_dimDecor = decor.*DecorWrite::m_dimDecor;
	const BOOL& m_bPause = decor.*DecorWrite::m_bPause;
	int num, num2;
	POINT posDecor;
	num = 0;
	if (m_decorAction == 0 || m_bPause)
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
	return posDecor;
}

} // namespace

void EndOfTick(CDecor& decor)
{
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

	// 1. DecorNextAction, retaining its pre-increment shaken view for culling.
	POINT posDecor = TickDecorAction(decor);

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

	// 3. Build's overlay tile loop, x major / y minor, at its shaken view.
	// Other overlay animations write only the local num; none produces 305,
	// 404 or 410. Drops write the stored icon; BlitzActif emits sound/network
	// messages, so it must run once here even when paused, exactly as Build did.
	Cellule (&cells)[MAXCELX][MAXCELY] = decor.*DecorWrite::m_decor;
	for (int x = posDecor.x / DIMOBJX; x < posDecor.x / DIMOBJX + LXIMAGE / DIMOBJX + 2; ++x)
	{
		for (int y = posDecor.y / DIMOBJY; y < posDecor.y / DIMOBJY + LYIMAGE / DIMOBJY + 2; ++y)
		{
			if (x < 0 || x >= MAXCELX || y < 0 || y >= MAXCELY) continue;
			int icon = cells[x][y].icon;
			if (icon == 404 || icon == 410)
			{
				num = table_decor_goutte[(x * 13 + y * 7 + m_time / 2) % 48];
				cells[x][y].icon = num >= 404 && num <= 407 ? 404 : 410;
			}
			if (icon == 305) decor.BlitzActif(POINT{x, y});
		}
	}

	// 4. CDecor::DrawInfo: CJauge::Draw clears m_bRedraw whether it draws or
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

	// 5. VoyageDraw's particle spawn, after DrawInfo's tutorial clamp.
	if (decor.*DecorWrite::m_voyageIcon == 40 && decor.*DecorWrite::m_voyageChannel == CHELEMENT)
	{
		static int speed[] = { -8, -6, -4, 0, 4, 6, 8 };
		int phase = decor.*DecorWrite::m_voyagePhase - 30;
		if (phase < 0) phase = 0;
		const POINT& start = decor.*DecorWrite::m_voyageStart;
		const POINT& end = decor.*DecorWrite::m_voyageEnd;
		int total = decor.*DecorWrite::m_voyageTotal;
		POINT pos = {start.x + (end.x - start.x) * phase / total,
					 start.y + (end.y - start.y) * phase / total};
		const POINT& camera = decor.*DecorWrite::m_posDecor;
		pos.x += camera.x - 34;
		pos.y += camera.y;
		int vx = speed[Random(0, 6)];
		int vy = Random(-10, 10);
		if (phase == 0) { vx /= 2; vy *= 4; }
		pos.y += vy;
		decor.ObjectStart(pos, TYPE_EXPLO8, vx, TRUE);
	}

	// 6. End of CDecor::Build.
	m_time++;
	if (m_time >= 32768) m_time = 0;
}
