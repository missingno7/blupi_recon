// DecBlupi.cpp
//

#include <misc.h>
#include <decor.h>
#include <text.h> // debug
//#include "resource.h"
#include <obstacle.h>
#include <dectables.h>


// SB2 animation and collision helpers, in binary definition order.
void CDecor::BlupiSearchIcon()

{
	int i = 0;
	int phase;
	int action = m_blupiAction;
	int icon = 2;

	if (m_blupiVent && !m_blupiHelico && !m_blupiOver)
	{
		if (action == ACTION_STOP) action = ACTION_VERTIGO;
		if (action == ACTION_MARCH) action = ACTION_PUSH;
	}

	if (m_blupiHelico)
	{
		if (action == ACTION_STOP) action = ACTION_STOPHELICO;
		if (action == ACTION_MARCH) action = ACTION_MARCHHELICO;
		if (action == ACTION_TURN) action = ACTION_TURNHELICO;
		if (action == ACTION_ADVANCE) action = ACTION_STOPHELICO;
		if (action == ACTION_RECEDE) action = ACTION_STOPHELICO;
	}

	if (m_blupiOver)
	{
		if (action == ACTION_STOP) action = ACTION_STOPOVER;
		if (action == ACTION_MARCH) action = ACTION_MARCHOVER;
		if (action == ACTION_TURN) action = ACTION_TURNOVER;	
		if (action == ACTION_ADVANCE) action = ACTION_STOPOVER;
		if (action == ACTION_RECEDE) action = ACTION_STOPOVER;
	}

	if (m_blupiJeep)
	{
		if (action == 1) action = 25;
		if (action == 2) action = 26;
		if (action == 10) action = 26;
		if (action == 3) action = 27;
	}

	if (m_blupiTank)
	{
		if (action == 1) action = 50;
		if (action == 2) action = 51;
		if (action == 10) action = 51;
		if (action == 3) action = 52;
	}

	if (m_blupiSkate)
	{
		if (action == 1) action = 37;
		if (action == 2) action = 38;
		if (action == 10) action = 38;
		if (action == 3) action = 39;
		if (action == ACTION_TURNAIR) action = ACTION_TURNSKATE;
		if (action == 4) action = 40;
		if (action == 5) action = 41;
	}

	if (m_blupiNage)
	{
		if (action == 1) action = 18;
		if (action == 2) action = 19;
		if (action == 3) action = 20;
		if (m_blupiVitesse.x == 0)
		{
			if (action == 18) action = 21;
			if (action == 19) action = 22;
		}
	}

	if (m_blupiSurf)
	{
		if (action == 1) action = 21;
		if (action == 2) action = 22;
		if (action == 3) action = 23;
	}

	if (m_blupiSuspend)
	{
		if (action == 1) action = 31;
		if (action == 2) action = 32;
		if (action == 3) action = 33;
		if (action == 4) action = 34;
	}

	if (m_blupiBalloon) action = ACTION_BALLOON;

	if (m_blupiEcrase)
	{
		if (action == ACTION_STOP) action = ACTION_STOPECRASE;
		if (action == ACTION_MARCH) action = ACTION_MARCHECRASE;
		if (action == ACTION_TURN) action = ACTION_MARCHECRASE;
	}

	if (action == ACTION_STOP)
	{
		phase = m_blupiPhase % 330;
		if (phase == 125 ||
			phase == 129 ||
			phase == 135 ||
			phase == 139 ||
			phase == 215 ||
			phase == 219 ||
			phase == 225 ||
			phase == 229 ||
			phase == 235 ||
			phase == 239 ||
			phase == 245 ||
			phase == 249 ||
			phase == 255 ||
			phase == 259 ||
			phase == 265 ||
			phase == 269)
		{
			PlaySound(SOUND_PATIENT, m_blupiPos, FALSE);
		}
	}
	if (action == ACTION_STOPSUSPEND && (((phase = m_blupiPhase % 328, phase == 0x76 || (phase == 0xe6)) || (phase == 0x116))))
	{
		PlaySound(SOUND_SINGE, m_blupiPos, FALSE);
	}
	if ((action == ACTION_STOPSURF || action == ACTION_MARCHSURF) && m_blupiPhase % 12 == 0 && m_blupiSurf)
	{
		MoveObjectTiplouf(m_blupiPos);
	}
	for (; table_blupi[i] != 0; i += table_blupi[i + 1] + 3)
	{
		if (action == table_blupi[i])
		{
			if (table_blupi[i + 2] == 0 || m_blupiPhase <= table_blupi[i + 2])
			{
				icon = m_blupiPhase % table_blupi[i + 1];
			}
			else icon = table_blupi[i + 2];
			icon = table_blupi[i + 3 + icon];
			break;
		}
	}
	if (action == ACTION_CLEAR1 || action == ACTION_CLEAR2 || action == ACTION_CLEAR3 || action == ACTION_GLU || (action == 57 && icon < 0x10a))
	{
		m_blupiChannel = CHELEMENT;
	}
	else
	{
		m_blupiChannel = 2;
	}
	int dir = m_blupiDir;
	if (m_blupiInvert)
	{
		if (m_blupiDir == DIR_RIGHT) dir = DIR_LEFT;
		if (m_blupiDir == DIR_LEFT) dir = DIR_RIGHT;
	}
	if (dir == DIR_LEFT && m_blupiChannel == 2)
	{
		if (action == ACTION_STOPSUSPEND)
		{
			if (icon == 0x90) icon = 0x9e;
			if (icon == 0x8f) icon = 0x91;
			if (icon == 0x97) icon = 0x92;
		}
		if (icon >= 0 && icon < 0x14f) icon = table_mirror[icon];
	}
	if (dir == DIR_LEFT && m_blupiChannel == CHELEMENT && icon >= 0xa8 && icon <= 0xab)
	{
		icon += 4;
	}
	m_blupiIcon = icon;
	m_blupiPhase++;
}

BOOL CDecor::BlupiIsGround()
{
	if (m_blupiTransport == -1)
	{
		RECT rect;
	rect = BlupiRect(m_blupiPos);
		rect.top = m_blupiPos.y + DIMBLUPIY - 2;
		rect.bottom = m_blupiPos.y + DIMBLUPIY - 1;
		return DecorDetect(rect, TRUE);
	}
	else {
		return FALSE;
	}
}

RECT CDecor::BlupiRect(POINT pos)
{
	RECT result;
	if (m_blupiNage || m_blupiSurf)
	{
		result.left = pos.x + 12;
		result.right = pos.x + DIMBLUPIX - 12;
		if (m_blupiAction == ACTION_STOP)
		{
			result.top = pos.y + 5;
			result.bottom = pos.y + DIMBLUPIY - 10;
		}
		else
		{
			result.top = pos.y + 15;
			result.bottom = pos.y + DIMBLUPIY - 10;
		}
	}
	else if (m_blupiJeep)
	{
		result.left = pos.x + 2;
		result.right = pos.x + DIMBLUPIX - 2;
		result.top = pos.y + 10;
		result.bottom = pos.y + DIMBLUPIY - 2;
	}
	else if (m_blupiTank)
	{
		result.left = pos.x + 2;
		result.right = pos.x + DIMBLUPIX - 2;
		result.top = pos.y + 10;
		result.bottom = pos.y + DIMBLUPIY - 2;
	}
	else if (m_blupiOver)
	{
		result.left = pos.x + 2;
		result.right = pos.x + DIMBLUPIX - 2;
		result.top = pos.y + 2;
		result.bottom = pos.y + DIMBLUPIY - 2;
	}
	else if (m_blupiBalloon)
	{
		result.left = pos.x + 10;
		result.right = pos.x + DIMBLUPIX - 10;
		result.top = pos.y + 5;
		result.bottom = pos.y + DIMBLUPIY - 2;
	}
	else if (m_blupiEcrase)
	{
		result.left = pos.x + 5;
		result.right = pos.x + DIMBLUPIX - 5;
		result.top = pos.y + 39;
		result.bottom = pos.y + DIMBLUPIY - 2;
	}
	else
	{
		result.left = pos.x + 12;
		result.right = pos.x + DIMBLUPIX - 12;
		result.top = pos.y + 11;
		result.bottom = pos.y + DIMBLUPIY - 2;
	}
	return result;
}

void CDecor::BlupiAdjust()
{
	int i;
	RECT rect;
	RECT tinyRect;
	tinyRect = BlupiRect(m_blupiPos);

	if (!DecorDetect(tinyRect))
	{
		return;
	}
	for (i = 0; i < 50; i++)
	{
		rect = tinyRect;
		rect.bottom = rect.top + 2;
		rect.left = m_blupiPos.x + 12;
		rect.right = m_blupiPos.x + 60 - 12;
		if (!DecorDetect(rect))
		{
			break;
		}
		tinyRect.top += 2;
		tinyRect.bottom += 2;
		m_blupiPos.y = m_blupiPos.y + 2;
	}
	for (i = 0; i < 50; i++)
	{
		rect = tinyRect;
		rect.right = rect.left + 2;
		rect.top = m_blupiPos.y + 11;
		rect.bottom = m_blupiPos.y + 60 - 2;
		if (!DecorDetect(rect))
		{
			break;
		}
		tinyRect.left += 2;
		tinyRect.right += 2;
		m_blupiPos.x = m_blupiPos.x + 2;
	}
	for (i = 0; i < 50; i++)
	{
		rect = tinyRect;
		rect.left = rect.right - 2;
		rect.top = m_blupiPos.y + 11;
		rect.bottom = m_blupiPos.y + 60 - 2;
		if (!DecorDetect(rect))
		{
			break;
		}
		tinyRect.left -= 2;
		tinyRect.right -= 2;
		m_blupiPos.x = m_blupiPos.x - 2;
	}
	for (i = 0; i < 50; i++)
	{
		rect = tinyRect;
		rect.right = rect.left + 2;
		if (!DecorDetect(rect))
		{
			break;
		}
		tinyRect.left += 2;
		tinyRect.right += 2;
		m_blupiPos.x = m_blupiPos.x + 2;
	}
	for (i = 0; i < 50; i++)
	{
		rect = tinyRect;
		rect.left = rect.right - 2;
		if (!DecorDetect(rect))
		{
			return;
		}
		tinyRect.left -= 2;
		tinyRect.right -= 2;
		m_blupiPos.x = m_blupiPos.x - 2;
	}
}

BOOL CDecor::BlupiBloque(POINT pos, int dir)
{
	RECT rect;
	rect = BlupiRect(pos);
	rect.top = rect.bottom - 20;
	rect.bottom -= 2;
	if (dir > 0)
	{
		rect.left = rect.right - 2;
	}
	if (dir < 0)
	{
		rect.right = rect.left + 2;
	}
	return DecorDetect(rect);
}

void CDecor::BlupiStep()
{
	char str[100];
	BOOL bNear;
	int icon;
	POINT tinyPoint;
	BlupiAdjust();
	//m_blupiLastPos = m_blupiPos;
	POINT end = m_blupiPos;
	BOOL bAirStart = FALSE;
	BOOL bLastBlupiAir = m_blupiAir;
	int lastBlupiAction = m_blupiAction;
	BOOL bIsOpen = FALSE;
	BOOL bIsOpen2 = FALSE;
	int num;
	end.x += m_blupiVector.x;
	end.y += m_blupiVector.y;
	RECT rect;
	if (m_blupiVector.x != 0 || m_blupiVector.y != 0)
	{
		rect = BlupiRect(m_blupiPos);
		rect.top = m_blupiPos.y + 11;
		rect.bottom = m_blupiPos.y + DIMBLUPIY - 2;
		TestPath(rect, m_blupiPos, &end);
	}
	m_blupiVent = FALSE;
	if (m_blupiTransport == -1 && !m_blupiJeep && !m_blupiTank && !m_blupiSkate
		&& m_blupiFocus)
	{
		POINT windCell;
		windCell.x = (end.x + DIMBLUPIX / 2) / DIMOBJX;
		windCell.y = (end.y + DIMBLUPIY / 2) / DIMOBJY;
		num = m_decor[windCell.x][windCell.y].icon;
		if (num == 110)
		{
			end.x -= 9;
		}
		if (num == 114)
		{
			end.x += 9;
		}
		if (num == 118)
		{
			end.y -= 20;
		}
		if (num == 122)
		{
			end.y += 20;
		}
		if (num >= 110 && num <= 125)
		{
			m_blupiVent = TRUE;
			rect.left = m_blupiPos.x + 12;
			rect.right = m_blupiPos.x + DIMBLUPIX - 12;
			rect.top = m_blupiPos.y + 11;
			rect.bottom = m_blupiPos.y + DIMBLUPIY - 2;
			TestPath(rect, m_blupiPos, &end);
		}
	}
	if (m_blupiTransport == -1)
	{
		rect = BlupiRect(end);
		rect.top = end.y + DIMBLUPIY - 2;
		rect.bottom = end.y + DIMBLUPIY - 1;
		bAirStart = !DecorDetect(rect);
	}
	else
	{
		bAirStart = FALSE;
	}
	rect = BlupiRect(end);
	rect.top = end.y + 10;
	rect.bottom = end.y + 20;
	icon = DecorDetect(rect);
	int detectIcon = m_detectIcon;
	if (!m_blupiAir &&
		!m_blupiHelico && !m_blupiOver && !m_blupiBalloon && !m_blupiEcrase && !m_blupiJeep && !m_blupiTank &&
		!m_blupiNage &&
		!m_blupiSurf &&
		!m_blupiSuspend &&
		bAirStart &&
		m_blupiFocus)
	{
		if (m_blupiFocus)
		{
			m_blupiAction = ACTION_AIR;
			m_blupiPhase = 0;
		}
		m_blupiVitesse.y = 1;
		m_blupiAir = TRUE;
		bLastBlupiAir = TRUE;
	}
	if (!m_blupiNage &&
		!m_blupiSurf &&
		!m_blupiSuspend &&
		!m_blupiAir &&
		IsRessort(end))
	{
		if ((m_blupiHelico || m_blupiOver) && !m_blupiShield && !m_bSuperBlupi)
		{
			m_blupiHelico = FALSE;
			m_blupiOver = FALSE;
			tinyPoint.x = end.x - 34;
			tinyPoint.y = end.y - 34;
			ObjectStart(tinyPoint, TYPE_EXPLO2, 0, TRUE);
			m_decorAction = 1;
			m_decorPhase = 0;
			StopSound(SOUND_HELICOHIGH);
			StopSound(SOUND_HELICOLOW);
			StopSound(SOUND_JEEPHIGH);
			StopSound(SOUND_JEEPLOW);
			PlaySound(SOUND_BOUM, m_blupiPos, FALSE);
		}
		if (m_blupiJeep && !m_blupiShield && !m_bSuperBlupi)
		{
			m_blupiJeep = FALSE;
			tinyPoint.x = end.x - 34;
			tinyPoint.y = end.y - 34;
			ObjectStart(tinyPoint, TYPE_EXPLO2, 0, TRUE);
			m_decorAction = 1;
			m_decorPhase = 0;
			StopSound(SOUND_HELICOHIGH);
			StopSound(SOUND_HELICOLOW);
			StopSound(SOUND_JEEPHIGH);
			StopSound(SOUND_JEEPLOW);
			PlaySound(SOUND_BOUM, m_blupiPos, FALSE);
		}
		if (m_blupiTank && !m_blupiShield && !m_bSuperBlupi)
		{
			m_blupiTank = FALSE;
			tinyPoint.x = end.x - 34;
			tinyPoint.y = end.y - 34;
			ObjectStart(tinyPoint, TYPE_EXPLO2, 0, TRUE);
			m_decorAction = 1;
			m_decorPhase = 0;
			StopSound(SOUND_HELICOHIGH);
			StopSound(SOUND_HELICOLOW);
			StopSound(SOUND_JEEPHIGH);
			StopSound(SOUND_JEEPLOW);
			PlaySound(SOUND_BOUM, m_blupiPos, FALSE);
		}
		if (m_blupiSkate && !m_blupiShield && !m_bSuperBlupi)
		{
			m_blupiSkate = FALSE;
			tinyPoint.x = end.x - 34;
			tinyPoint.y = end.y - 34;
			ObjectStart(tinyPoint, TYPE_EXPLO2, 0, TRUE);
			m_decorAction = 1;
			m_decorPhase = 0;
			StopSound(SOUND_HELICOHIGH);
			StopSound(SOUND_HELICOLOW);
			StopSound(SOUND_JEEPHIGH);
			StopSound(SOUND_JEEPLOW);
			PlaySound(SOUND_BOUM, m_blupiPos, FALSE);
		}
		if (m_blupiFocus && m_blupiAction != ACTION_CLEAR1 &&
			m_blupiAction != ACTION_CLEAR2 &&
			m_blupiAction != ACTION_CLEAR3 &&
			m_blupiAction != ACTION_CLEAR4 &&
			m_blupiAction != ACTION_CLEAR5 &&
			m_blupiAction != ACTION_CLEAR6 &&
			m_blupiAction != ACTION_CLEAR7 &&
			m_blupiAction != ACTION_CLEAR8)
		{
			m_blupiAction = ACTION_AIR;
			m_blupiPhase = 0;
		}
		if (m_keyPress & KEY_JUMP && m_keyPress & KEY_UP && m_blupiFocus)
		{
			if (m_blupiPower) m_blupiVitesse.y = -29;
			else m_blupiVitesse.y = -25;
		}
		else if (m_keyPress & KEY_JUMP && m_keyPress & KEY_DOWN && m_blupiFocus)
		{
			if (m_blupiPower) m_blupiVitesse.y = -19;
			else m_blupiVitesse.y = -16;
		}
		else if (m_keyPress & KEY_JUMP && m_blupiFocus)
		{
			if (m_blupiPower) m_blupiVitesse.y = -25;
			else m_blupiVitesse.y = -19;
		}
		else
		{
			if (m_blupiPower) m_blupiVitesse.y = -16;
			else m_blupiVitesse.y = -10;
		}
		m_blupiAir = TRUE;
		bLastBlupiAir = TRUE;
		PlaySound(SOUND_RESSORT, end, FALSE);
	}
	if (m_keyPress & KEY_JUMP &&
		!m_blupiHelico && !m_blupiOver && !m_blupiBalloon && !m_blupiEcrase && !m_blupiJeep && !m_blupiTank &&
		!m_blupiNage &&
		!m_blupiSurf &&
		!m_blupiSuspend
		&& m_blupiFocus)
	{
		if (m_blupiAction != ACTION_JUMP &&
			m_blupiAction != ACTION_TURN &&
			!m_blupiAir)
		{
			m_blupiAction = ACTION_JUMP;
			m_blupiPhase = 0;
		}
		if (m_blupiAction == ACTION_JUMP && m_blupiPhase == 3)
		{
			m_blupiAction = ACTION_AIR;
			m_blupiPhase = 0;
			if (m_blupiSkate)
			{
				PlaySound(SOUND_JUMP1, end, FALSE);
				if (m_blupiPower) m_blupiVitesse.y = -17;
				else m_blupiVitesse.y = -13;
			}
			else
			{
				if (m_keyPress & KEY_UP)
				{
					PlaySound(SOUND_JUMP2, end, FALSE);
					if (m_blupiPower) m_blupiVitesse.y = -26;
				else m_blupiVitesse.y = -22;
				}
				else if (m_keyPress & KEY_DOWN)
				{
					PlaySound(SOUND_JUMP0, end, FALSE);
					if (m_blupiPower) m_blupiVitesse.y = -16;
				else m_blupiVitesse.y = -12;
				}
				else
				{
					PlaySound(SOUND_JUMP1, end, FALSE);
					if (m_blupiPower) m_blupiVitesse.y = -22;
				else m_blupiVitesse.y = -16;
				}
			}
			m_blupiAir = TRUE;
			bLastBlupiAir = TRUE;
		}
	}
	if (m_blupiAir)
	{
		if (icon && m_blupiVitesse.y < 0)
		{
			if (m_blupiVitesse.y < -14 && m_blupiAction != ACTION_CLEAR1 &&
			m_blupiAction != ACTION_CLEAR2 &&
			m_blupiAction != ACTION_CLEAR3 &&
			m_blupiAction != ACTION_CLEAR4 &&
			m_blupiAction != ACTION_CLEAR5 &&
			m_blupiAction != ACTION_CLEAR6 &&
			m_blupiAction != ACTION_CLEAR7 &&
			m_blupiAction != ACTION_CLEAR8 && !m_blupiSkate)
			{
				m_blupiJumpAie = TRUE;
				PlaySound(SOUND_JUMPAIE, end, FALSE);
			}
			else
			{
			PlaySound(SoundEnviron(SOUND_JUMPTOC, detectIcon), end, FALSE);
			}
			m_blupiVitesse.y = 1;
		}
		end.y += m_blupiVitesse.y * 2;
		if (m_blupiVitesse.y < 20)
		{
			m_blupiVitesse.y += 2;
		}
		rect = BlupiRect(end);
		rect.top = end.y + DIMBLUPIY - 30;
		rect.bottom = end.y + DIMBLUPIY - 1;
		if (m_blupiVitesse.y >= 0 && DecorDetect(rect))
		{
			end.y = end.y / 32 * 32 + BLUPIOFFY;
			if (!IsRessort(end))
			{
				PlaySound(SoundEnviron(SOUND_JUMPEND, m_detectIcon), end, FALSE);
			}
			if (m_blupiFocus)
			{
				if (m_blupiVitesse.y > 20)
				{
					m_blupiAction = ACTION_STOPJUMP; // ??
				}
				else
				{
					m_blupiAction = ACTION_STOPJUMP;
				}
				m_blupiPhase = 0;
			}
			m_blupiAir = FALSE;
			if (m_blupiJumpAie)
			{
				m_blupiJumpAie = FALSE;
				m_blupiAction = ACTION_JUMPAIE;
				m_blupiPhase = 0;
			}
		}
		rect.left = end.x + 20;
		rect.right = end.x + DIMBLUPIX - 20;
		rect.top = end.y + DIMBLUPIY - 35;
		rect.bottom = end.y + DIMBLUPIY - 1;
		num = AscenseurDetect(rect, m_blupiPos, end);
		if (m_blupiVitesse.y >= 0 && num != -1)
		{
			m_blupiTransport = num;
			bAirStart = FALSE;
			PlaySound(SOUND_JUMPEND, end, FALSE);
			end.y = m_moveObject[num].posCurrent.y - DIMOBJY + BLUPIOFFY;
			if (m_blupiFocus)
			{
				if (m_blupiVitesse.y > 20)
				{
					m_blupiAction = ACTION_STOPJUMP; // ???
				}
				else
				{
					m_blupiAction = ACTION_STOPJUMP;
				}
				m_blupiPhase = 0;
			}
			m_blupiAir = FALSE;
			if (m_blupiJumpAie)
			{
				m_blupiJumpAie = FALSE;
				m_blupiAction = ACTION_JUMPAIE;
				m_blupiPhase = 0;
			}
		}
	} // if (m_blupiAir)

	if (m_blupiAction == ACTION_JUMPAIE && m_blupiPhase == 30)
	{
		m_blupiAction = ACTION_STOP;
		m_blupiPhase = 0;
		m_blupiFocus = TRUE;
	}
	if (m_blupiAction == ACTION_CHARGE && m_blupiPhase == 64)
	{
		m_blupiAction = ACTION_STOP;
		m_blupiPhase = 0;
		m_blupiFocus = TRUE;
		m_blupiCloud = TRUE;
		m_blupiTimeShield = 100;
		m_jauges[JAUGE_POWER].SetHide(FALSE);
		PlaySound(ACTION_DRINK, end, FALSE);
	}
	if (m_blupiAction == ACTION_HELICOGLU)
	{
		if (m_blupiPhase == 8)
		{
			tinyPoint.x = m_blupiPos.x;
			tinyPoint.y = m_blupiPos.y + 40;
			if (m_blupiVitesse.y > 0)
			{
				tinyPoint.y += m_blupiVitesse.y * 4;
			}
			m_blupiVitesse.y -= 10;
			if (ObjectStart(tinyPoint, TYPE_BALLE, 55, TRUE))
			{
				PlaySound(SOUND_FIREOK, m_blupiPos, FALSE);
				m_blupiTimeFire = 10;
				m_blupiBullet--;
			}
		}
		if (m_blupiPhase == 14) 
		{
			m_blupiAction = ACTION_STOP;
			m_blupiPhase = 0;
			m_blupiFocus = TRUE;
		}
	}
	if ((m_blupiAction == ACTION_OUF1a || m_blupiAction == ACTION_OUF1b) && m_blupiPhase == 29)
	{
		m_blupiAction = ACTION_STOP;
		m_blupiPhase = 0;
		m_blupiFocus = TRUE;
	}
	if (m_blupiAction == ACTION_OUF2 && m_blupiPhase == 32) 
	{
		m_blupiAction = ACTION_STOP;
		m_blupiPhase = 0;
		m_blupiFocus = TRUE;
	}
	if (m_blupiAction == ACTION_OUF3 && m_blupiPhase == 34) 
	{
		m_blupiAction = ACTION_STOP;
		m_blupiPhase = 0;
		m_blupiFocus = TRUE;
	}
	if (m_blupiAction == ACTION_OUF4 && m_blupiPhase == 40)
	{
		m_blupiAction = ACTION_STOP;
		m_blupiPhase = 0;
		m_blupiFocus = TRUE;
		num = MoveObjectDetect(end, &bNear);
		if (num != -1 && !bNear && end.y - BLUPIFLOOR == m_moveObject[num].posCurrent.y)
		{
			if (m_blupiDir == DIR_RIGHT && end.x < m_moveObject[num].posCurrent.x)
			{
				tinyPoint.x = end.x - 16;
				tinyPoint.y = end.y;
				int num2 = MoveObjectDetect(tinyPoint, &bNear);
				if (num2 == -1)
				{
					m_blupiAction = ACTION_RECEDE;
					m_blupiPhase = 0;
				}
			}
			if (m_blupiDir == DIR_LEFT && end.x > m_moveObject[num].posCurrent.x)
			{
				tinyPoint.x = end.x + 16;
				tinyPoint.y = end.y;
				int num2 = MoveObjectDetect(tinyPoint, &bNear);
				if (num2 == -1)
				{
					m_blupiAction = ACTION_RECEDE;
					m_blupiPhase = 0;
				}
			}
		}
	}
	if (m_blupiAction == ACTION_OUF5)
	{
		if (m_blupiPhase == 4) PlaySound(SOUND_OUF2, m_blupiPos, FALSE);
		if (m_blupiPhase == 44) 
		{
			m_blupiAction = ACTION_STOP;
			m_blupiPhase = 0;
			m_blupiFocus = TRUE;
		}
	}
	if (m_blupiAction == ACTION_MOCKERY)
	{
		if (m_blupiPhase == 1)
		{
			PlaySound(SOUND_MOCKERY, m_blupiPos, FALSE);
			m_blupiTimeMockery = 200;
		}
		if (m_blupiPhase == 92) 
		{
			m_blupiAction = ACTION_STOP;
			m_blupiPhase = 0;
			m_blupiFocus = TRUE;
		}
	}
	if (m_blupiAction == ACTION_MOCKERYi)
	{
		if (m_blupiPhase == 6)
		{
			PlaySound(SOUND_MOCKERY, m_blupiPos, FALSE);
			m_blupiTimeMockery = 200;
		}
		if (m_blupiPhase == 104) 
		{
			m_blupiAction = ACTION_STOP;
			m_blupiPhase = 0;
			m_blupiFocus = TRUE;
		}
	}
	if (m_blupiAction == ACTION_MOCKERYp)
	{
		if (m_blupiPhase == 4) PlaySound(SOUND_OUF2, m_blupiPos, FALSE);
		if (m_blupiPhase == 60) 
		{
			m_blupiAction = ACTION_STOP;
			m_blupiPhase = 0;
			m_blupiFocus = TRUE;
		}
	}
	if (m_blupiAction == ACTION_STOPMARCH && m_blupiPhase == 3) 
	{
		m_blupiAction = ACTION_STOP;
		m_blupiPhase = 0;
	}
	if (m_blupiAction == ACTION_STOPJUMP && m_blupiPhase == 5) 
	{
		m_blupiAction = ACTION_STOP;
		m_blupiPhase = 0;
	}
	if (m_blupiAction == ACTION_STOPJUMPh && m_blupiPhase == 2)
	{
		m_blupiAction = ACTION_AIR;
		m_blupiPhase = 0;
		m_blupiVitesse.y = -12;
		m_blupiAir = TRUE;
		bLastBlupiAir = TRUE;
	}
	if (m_blupiAction == ACTION_SUCETTE && m_blupiPhase == 36)
	{
		m_blupiAction = ACTION_STOP;
		m_blupiPhase = 0;
		m_blupiFocus = TRUE;
		m_blupiPower = TRUE;
		m_blupiTimeShield = 100;
		m_blupiPosMagic = m_blupiPos;
		m_jauges[JAUGE_POWER].SetHide(FALSE);
		PlaySound(SOUND_STARTPOWER, end, FALSE);
	}
	if (m_blupiAction == ACTION_DRINK && m_blupiPhase == 36)
	{
		m_blupiAction = ACTION_STOP;
		m_blupiPhase = 0;
		m_blupiFocus = TRUE;
		m_blupiHide = TRUE;
		m_blupiTimeShield = 100;
		m_blupiPosMagic = m_blupiPos;
		m_jauges[JAUGE_POWER].SetHide(FALSE);
		PlaySound(SOUND_STARTHIDE, end, FALSE);
	}

	if (m_keyPress & KEY_UP &&
		!(m_lastKeyPress & KEY_UP) &&
		m_blupiAction != ACTION_TURN && m_blupiAction != ACTION_JUMP &&
		m_blupiAction != ACTION_AIR && m_blupiAction != ACTION_VERTIGO &&
		m_blupiAction != ACTION_ADVANCE && m_blupiAction != ACTION_RECEDE &&
		!m_blupiAir && !m_blupiHelico && !m_blupiOver && !m_blupiBalloon && !m_blupiEcrase && !m_blupiJeep && !m_blupiTank && !m_blupiSkate &&
		!m_blupiNage && !m_blupiSurf && !m_blupiSuspend &&
		m_blupiFocus)
	{
		m_blupiAction = ACTION_UP;
		m_blupiPhase = 0;
	}

	if (!(m_keyPress & KEY_UP) && m_lastKeyPress & KEY_UP &&
		!m_blupiAir && !m_blupiHelico && !m_blupiOver && !m_blupiBalloon && !m_blupiEcrase && !m_blupiJeep && !m_blupiTank && !m_blupiSkate &&
		!m_blupiNage && !m_blupiSurf && !m_blupiSuspend && m_blupiFocus)
	{
		m_blupiAction = ACTION_STOP;
		m_blupiPhase = 0;
	}

	if (m_keyPress & KEY_DOWN &&
		!(m_lastKeyPress & KEY_DOWN) &&
		m_blupiAction != ACTION_TURN && m_blupiAction != ACTION_JUMP &&
		m_blupiAction != ACTION_AIR && m_blupiAction != ACTION_DOWN &&
		m_blupiAction != ACTION_STOPPOP && m_blupiAction != ACTION_VERTIGO &&
		m_blupiAction != ACTION_ADVANCE && m_blupiAction != ACTION_RECEDE &&
		!m_blupiAir && !m_blupiHelico && !m_blupiOver && !m_blupiBalloon && !m_blupiEcrase && !m_blupiJeep && !m_blupiTank && !m_blupiSkate &&
		!m_blupiNage && !m_blupiSurf && !m_blupiSuspend &&
		m_blupiFocus)
	{
		m_blupiAction = ACTION_DOWN;
		m_blupiPhase = 0;
	}

	if (m_keyPress & KEY_DOWN &&
		!(m_keyPress & (KEY_JUMP | KEY_LEFT | KEY_RIGHT)) &&
		m_blupiAction != ACTION_TURN && m_blupiAction != ACTION_JUMP &&
		m_blupiAction != ACTION_AIR && m_blupiAction != ACTION_DOWN &&
		m_blupiAction != ACTION_STOPPOP && m_blupiAction != ACTION_VERTIGO &&
		m_blupiAction != ACTION_ADVANCE && m_blupiAction != ACTION_RECEDE &&
		!m_blupiAir && !m_blupiHelico && !m_blupiOver && !m_blupiBalloon && !m_blupiEcrase && !m_blupiJeep && !m_blupiTank && !m_blupiSkate &&
		!m_blupiNage && !m_blupiSurf && !m_blupiSuspend &&
		m_blupiFocus)
	{
		m_blupiAction = ACTION_DOWN;
		m_blupiPhase = 0;
	}
	if (!(m_keyPress & KEY_DOWN) && m_lastKeyPress & KEY_DOWN &&
		!m_blupiAir && !m_blupiHelico && !m_blupiOver && !m_blupiBalloon && !m_blupiEcrase && !m_blupiJeep && !m_blupiTank && !m_blupiSkate &&
		!m_blupiNage && !m_blupiSurf && !m_blupiSuspend && m_blupiFocus)
	{
		m_blupiAction = ACTION_STOP;
		m_blupiPhase = 0;
	}
	if (m_blupiAction == ACTION_UP && m_blupiPhase == 4)
	{
		m_scrollAdd.y = -150;
		PlaySound(SOUND_LOOKUP, end, FALSE);
	}
	if (m_blupiAction == ACTION_DOWN && m_blupiPhase == 4)
	{
		m_scrollAdd.y = 150;
		PlaySound(SOUND_DOWN, end, FALSE);
	}
	if (!m_blupiHelico && !m_blupiOver && !m_blupiBalloon && !m_blupiEcrase && !m_blupiJeep && !m_blupiTank && !m_blupiSkate &&
		!m_blupiNage &&
		!m_blupiSurf &&
		!m_blupiSuspend
		&& m_blupiFocus)
	{
		if (m_keyPress & KEY_DOWN &&
			!(m_keyPress & (KEY_JUMP | KEY_LEFT | KEY_RIGHT)) &&
			m_blupiAction != ACTION_STOPPOP &&
			m_blupiDir == DIR_LEFT &&
			(num = CaisseInFront()) != -1)
		{
			end.x = m_moveObject[num].posCurrent.x + 64 - 5;
			m_blupiAction = ACTION_STOPPOP;
			m_blupiPhase = 0;
			m_scrollAdd.y = 0;
			PlaySound(SOUND_POP, end, FALSE);
		}
		if (m_keyPress & KEY_DOWN && m_keyPress & KEY_RIGHT &&
			!(m_keyPress & (KEY_JUMP | KEY_LEFT)) &&
			m_blupiAction != ACTION_POP &&
			m_blupiDir == DIR_LEFT &&
			(num = CaisseInFront()) != -1)
		{
			m_blupiAction = ACTION_POP;
			m_blupiPhase = 0;
			m_scrollAdd.y = 0;
			PlaySound(SOUND_POP, end, FALSE);
		}
		if (m_keyPress & KEY_DOWN &&
			!(m_keyPress & (KEY_JUMP | KEY_LEFT | KEY_RIGHT)) &&
			m_blupiAction != ACTION_STOPPOP &&
			m_blupiDir == DIR_RIGHT &&
			(num = CaisseInFront()) != -1)
		{
			end.x = m_moveObject[num].posCurrent.x - DIMBLUPIX + 5;
			m_blupiAction = ACTION_STOPPOP;
			m_blupiPhase = 0;
			m_scrollAdd.y = 0;
			PlaySound(SOUND_POP, end, FALSE);
		}
		if (m_keyPress & KEY_DOWN && m_keyPress & KEY_LEFT &&
			!(m_keyPress & (KEY_JUMP | KEY_RIGHT)) &&
			m_blupiAction != ACTION_POP &&
			m_blupiDir == DIR_RIGHT &&
			(num = CaisseInFront()) != -1)
		{
			m_blupiAction = ACTION_POP;
			m_blupiPhase = 0;
			m_scrollAdd.y = 0;
			PlaySound(SOUND_POP, end, FALSE);
		}
		if (m_blupiAction == ACTION_POP && m_blupiActionOuf != ACTION_OUF3)
		{
			m_blupiActionOuf = ACTION_OUF3;
			m_blupiTimeOuf = 0;
		}
	}
	if (m_blupiAction != ACTION_STOPPOP &&
		m_blupiAction != ACTION_POP &&
		m_blupiFocus)
	{
		if (m_keyPress & KEY_LEFT && !(m_lastKeyPress & KEY_LEFT) && !m_blupiAir && m_keyPress & (KEY_UP | KEY_DOWN))
		{
			m_blupiAction = 1;
			m_blupiPhase = 0;
		}
		if (!(m_keyPress & KEY_LEFT) && m_lastKeyPress & KEY_LEFT && m_keyPress & (KEY_UP | KEY_DOWN))
		{
			m_blupiAction = 1;
			m_blupiPhase = 0;
		}
		if (m_keyPress & KEY_RIGHT && !(m_lastKeyPress & KEY_RIGHT) && !m_blupiAir && m_keyPress & (KEY_UP | KEY_DOWN))
		{
			m_blupiAction = 1;
			m_blupiPhase = 0;
		}
		if (!(m_keyPress & KEY_RIGHT) && m_lastKeyPress & KEY_RIGHT && m_keyPress & (KEY_UP | KEY_DOWN))
		{
			m_blupiAction = 1;
			m_blupiPhase = 0;
		}
	}
	int num3;
	int num4;
	if (m_keyPress & KEY_LEFT && !(m_keyPress & KEY_RIGHT) && m_blupiFocus)
	{
		if (m_blupiDir == DIR_RIGHT &&
			m_blupiAction != ACTION_TURN &&
			m_blupiAction != ACTION_TURNAIR &&
			m_blupiAction != ACTION_UP &&
			m_blupiAction != ACTION_DOWN &&
			m_blupiAction != ACTION_POP &&
			((!m_blupiJeep && !m_blupiTank && !m_blupiSkate) || abs(m_blupiVitesse.x) <= 8))
		{
			if (m_blupiAir)
			{
				PlaySound(SOUND_TURN, end, FALSE);
				m_blupiAction = ACTION_TURNAIR;
				m_blupiPhase = 0;
				m_blupiDir = DIR_LEFT;
			}
			else
			{
				PlaySound(SOUND_TURN, end, FALSE);
				m_blupiAction = ACTION_TURN;
				m_blupiPhase = 0;
			}
		}
		if (m_blupiDir == DIR_LEFT &&
			m_blupiAction != ACTION_MARCH &&
			m_blupiAction != ACTION_PUSH &&
			m_blupiAction != ACTION_JUMP &&
			m_blupiAction != ACTION_UP &&
			m_blupiAction != ACTION_DOWN &&
			m_blupiAction != ACTION_POP &&
			!m_blupiAir)
		{
			m_blupiAction = ACTION_MARCH;
			m_blupiPhase = 0;
		}
		if (m_blupiDir == DIR_LEFT &&
			m_blupiAction != ACTION_TURN &&
			m_blupiAction != ACTION_JUMP &&
			m_blupiAction != ACTION_UP &&
			m_blupiAction != ACTION_DOWN &&
			m_blupiAction != ACTION_POP &&
			!m_blupiHelico && !m_blupiOver && !m_blupiBalloon && !m_blupiEcrase && !m_blupiJeep && !m_blupiTank && !m_blupiSkate &&
			!m_blupiNage &&
			!m_blupiSurf &&
			!m_blupiSuspend)
		{
			if (m_blupiAction == ACTION_PUSH)
			{
				end.x -= CaisseGetMove(5);
			}
			else
			{
				num3 = m_blupiPhase;
				if (num3 > 3 || m_blupiAir)
				{
					num3 = 3;
				}
				num4 = table_vitesse_march[num3];
				if (m_blupiPower)
				{
					num4 *= 3;
					num4 /= 2;
				}
				end.x -= num4;
			}
		}
		if (m_blupiDir == DIR_RIGHT && m_blupiAction == ACTION_POP)
		{
			end.x -= CaisseGetMove(3);
		}
	} // if (m_keyPress & KEY_LEFT && !(m_keyPress & KEY_RIGHT) && m_blupiFocus)

	if (m_keyPress & KEY_RIGHT && !(m_keyPress & KEY_LEFT) && m_blupiFocus)
	{
		if (m_blupiDir == DIR_LEFT &&
			m_blupiAction != ACTION_TURN &&
			m_blupiAction != ACTION_TURNAIR &&
			m_blupiAction != ACTION_UP &&
			m_blupiAction != ACTION_DOWN &&
			m_blupiAction != ACTION_POP &&
			((!m_blupiJeep && !m_blupiTank && !m_blupiSkate) || abs(m_blupiVitesse.x) <= 8))
		{
			if (m_blupiAir)
			{
				PlaySound(SOUND_TURN, end, FALSE);
				m_blupiAction = ACTION_TURNAIR;
				m_blupiPhase = 0;
				m_blupiDir = DIR_RIGHT;
			}
			else
			{
				PlaySound(SOUND_TURN, end, FALSE);
				m_blupiAction = ACTION_TURN;
				m_blupiPhase = 0;
			}
		}
		if (m_blupiDir == DIR_RIGHT &&
			m_blupiAction != ACTION_MARCH &&
			m_blupiAction != ACTION_PUSH &&
			m_blupiAction != ACTION_JUMP &&
			m_blupiAction != ACTION_UP &&
			m_blupiAction != ACTION_DOWN &&
			m_blupiAction != ACTION_POP &&
			!m_blupiAir)
		{
			m_blupiAction = ACTION_MARCH;
			m_blupiPhase = 0;
		}
		if (m_blupiDir == DIR_RIGHT &&
			m_blupiAction != ACTION_TURN &&
			m_blupiAction != ACTION_JUMP &&
			m_blupiAction != ACTION_UP &&
			m_blupiAction != ACTION_DOWN &&
			m_blupiAction != ACTION_POP &&
			!m_blupiHelico && !m_blupiOver && !m_blupiBalloon && !m_blupiEcrase && !m_blupiJeep && !m_blupiTank && !m_blupiSkate &&
			!m_blupiNage &&
			!m_blupiSurf &&
			!m_blupiSuspend)
		{
			if (m_blupiAction == ACTION_PUSH)
			{
				end.x += CaisseGetMove(5);
			}
			else
			{
				num3 = m_blupiPhase;
				if (num3 > 3 || m_blupiAir)
				{
					num3 = 3;
				}
				num4 = table_vitesse_march[num3];
				if (m_blupiPower)
				{
					num4 *= 3;
					num4 /= 2;
				}
				end.x += num4;
			}
		}
		if (m_blupiDir == DIR_LEFT && m_blupiAction == ACTION_POP)
		{
			end.x += CaisseGetMove(3);
		}
	} // if (m_keyPress & KEY_RIGHT && !(m_keyPress & KEY_LEFT) && m_blupiFocus)

	if (m_blupiHelico)
	{
		if (m_blupiAction == ACTION_TURN && m_blupiPhase == 10)
		{
			m_blupiAction = ACTION_MARCH;
			if (m_blupiDir == DIR_LEFT) m_blupiDir = DIR_RIGHT;
			else m_blupiDir = DIR_LEFT;
		}
	}
	else if (m_blupiOver)
	{
		if (m_blupiAction == ACTION_TURN && m_blupiPhase == 7)
		{
			m_blupiAction = ACTION_MARCH;
			if (m_blupiDir == DIR_LEFT) m_blupiDir = DIR_RIGHT;
			else m_blupiDir = DIR_LEFT;
		}
	}
	else if (m_blupiJeep)
	{
		if (m_blupiAction == ACTION_TURN && m_blupiPhase == 7)
		{
			m_blupiAction = ACTION_STOP;
			m_blupiPhase = 0;
			if (m_blupiDir == DIR_LEFT) m_blupiDir = DIR_RIGHT;
			else m_blupiDir = DIR_LEFT;
		}
	}
	else if (m_blupiTank)
	{
		if (m_blupiAction == ACTION_TURN && m_blupiPhase == 12)
		{
			m_blupiAction = ACTION_STOP;
			m_blupiPhase = 0;
			if (m_blupiDir == DIR_LEFT) m_blupiDir = DIR_RIGHT;
			else m_blupiDir = DIR_LEFT;
		}
	}
	else if (m_blupiSkate)
	{
		if (m_blupiAction == ACTION_TURN && m_blupiPhase == 7)
		{
			m_blupiAction = ACTION_STOP;
			m_blupiPhase = 0;
			if (m_blupiDir == DIR_LEFT) m_blupiDir = DIR_RIGHT;
			else m_blupiDir = DIR_LEFT;
		}
	}
	else if (m_blupiNage || m_blupiSurf)
	{
		if (m_blupiAction == ACTION_TURN && m_blupiPhase == 10)
		{
			m_blupiAction = ACTION_MARCH;
			if (m_blupiDir == DIR_LEFT) m_blupiDir = DIR_RIGHT;
			else m_blupiDir = DIR_LEFT;
		}
	}
	else if (m_blupiSuspend)
	{
		if (m_blupiAction == ACTION_TURN && m_blupiPhase == 10)
		{
			m_blupiAction = ACTION_STOP;
			m_blupiPhase = 0;
			if (m_blupiDir == DIR_LEFT) m_blupiDir = DIR_RIGHT;
			else m_blupiDir = DIR_LEFT;
		}
	}
	else
	{
		if (m_blupiAction == ACTION_TURN && m_blupiPhase == 6)
		{
			m_blupiAction = ACTION_STOP;
			m_blupiPhase = 0;
			if (m_blupiDir == DIR_LEFT) m_blupiDir = DIR_RIGHT;
			else m_blupiDir = DIR_LEFT;
		}
		if (m_blupiAction == ACTION_TURNAIR && m_blupiPhase == 6)
		{
			m_blupiAction = ACTION_AIR;
			m_blupiPhase = 0;
		}
	}

	if (!m_blupiSuspend && m_blupiAction == ACTION_JUMP && m_blupiPhase == 3)
	{
		m_blupiAction = ACTION_STOP;
		m_blupiPhase = 0;
	}

	if ((!(m_keyPress & ~KEY_FIRE) || ((m_keyPress & KEY_LEFT) && (m_keyPress & KEY_RIGHT))) &&
		!m_blupiHelico && !m_blupiOver && !m_blupiBalloon && !m_blupiEcrase && !m_blupiJeep && !m_blupiTank && !m_blupiSkate &&
		!m_blupiNage &&
		!m_blupiSurf &&
		m_blupiFocus)
	{
		if (m_blupiAction == ACTION_PUSH || m_blupiAction == ACTION_UP)
		{
			m_blupiAction = ACTION_STOP;
			m_blupiPhase = 0;
		}
		if (m_blupiAction == ACTION_MARCH)
		{
			if (m_blupiSuspend || m_blupiPhase < 10)
			{
				m_blupiAction = ACTION_STOP;
				m_blupiPhase = 0;
			}
			else
			{
				m_blupiAction = ACTION_STOPMARCH;
				m_blupiPhase = 0;
			}
		}
		if (m_blupiAction == ACTION_DOWN)
		{
			m_blupiAction = ACTION_STOP;
			m_blupiPhase = 0;
			PlaySound(SOUND_UP, end, FALSE);
		}
		m_scrollAdd.y = 0;
		if (lastBlupiAction == ACTION_PUSH)
		{
			StopSound(SOUND_PUSH);
		}
		if (lastBlupiAction == ACTION_POP || lastBlupiAction == ACTION_STOPPOP)
		{
			StopSound(SOUND_POP);
		}
	}
	if (!m_blupiBalloon &&
		!m_blupiEcrase &&
		!m_blupiJeep &&
		!m_blupiTank &&
		!m_blupiSkate &&
		!m_blupiNage &&
		!m_blupiSurf &&
		m_blupiFocus)
	{
		if (m_blupiAction == ACTION_RECEDE && m_blupiDir == DIR_LEFT)
		{
			end.x += 4;
		}
		if (m_blupiAction == ACTION_RECEDE && m_blupiDir == DIR_RIGHT)
		{
			end.x -= 4;
		}
		if (m_blupiAction == ACTION_ADVANCE && m_blupiDir == DIR_LEFT)
		{
			end.x -= 4;
		}
		if (m_blupiAction == ACTION_ADVANCE && m_blupiDir == DIR_RIGHT)
		{
			end.x += 4;
		}
	}

	if (!(m_keyPress & ~KEY_FIRE))
	{
		if ((m_blupiJeep || m_blupiTank || m_blupiSkate) && m_blupiFocus)
		{
			if (m_blupiAction == ACTION_ADVANCE && m_blupiDir == DIR_LEFT)
			{
				end.x -= 5;
			}
			if (m_blupiAction == ACTION_ADVANCE && m_blupiDir == DIR_RIGHT)
			{
				end.x += 5;
			}
		}
	}
	if (!(m_keyPress & ~KEY_FIRE) && m_blupiNage &&
			m_blupiFocus &&
			m_blupiAction == ACTION_MARCH)
		{
			m_blupiAction = ACTION_STOP;
			m_blupiPhase = 0;
		}
	if (!(m_keyPress & ~KEY_FIRE) && m_blupiSurf &&
			m_blupiFocus &&
			m_blupiAction == ACTION_MARCH)
		{
			m_blupiAction = ACTION_STOP;
			m_blupiPhase = 0;
		}
	if (m_blupiHelico && (m_blupiFocus || m_blupiAction == ACTION_HELICOGLU))
	{
		if (m_keyPress & KEY_FIRE &&
			m_blupiTimeFire == 0 &&
			m_blupiAction != ACTION_TURN &&
			m_blupiAction != ACTION_HELICOGLU &&
			bAirStart)
		{
			if (m_blupiBullet == 0)
			{
				PlaySound(SOUND_FIREKO, m_blupiPos, FALSE);
			}
			else
			{
				m_blupiAction = ACTION_HELICOGLU;
				m_blupiPhase = 0;
				m_blupiFocus = FALSE;
			}
		}
		if (bAirStart && m_bHelicoStop)
		{
			StopSound(SOUND_HELICOLOW);
			PlaySound(SOUND_HELICOHIGH, m_blupiPos, TRUE);
		}
		if (!bAirStart && m_bHelicoMarch)
		{
			StopSound(SOUND_HELICOHIGH);
			PlaySound(SOUND_HELICOLOW, m_blupiPos, TRUE);
		}
		////m_blupiMotorHigh = bCanUseLift;
		if (m_blupiAction != ACTION_TURN && m_blupiAction != ACTION_HELICOGLU)
		{
			if (bAirStart)
			{
				m_blupiAction = ACTION_MARCH;
			}
			else
			{
				m_blupiAction = ACTION_STOP;
				m_blupiPhase = 0;
				m_blupiVitesse.y = 0;
			}
		}

		if (!(m_keyPress & (KEY_UP | KEY_JUMP)))
		{
			if (m_keyPress & KEY_DOWN)
{
			if (m_blupiVitesse.y < 12) m_blupiVitesse.y += 1;
		}
			else
{
			if (m_blupiVitesse.y > 1) m_blupiVitesse.y -= 1;
			if (m_blupiVitesse.y < 1) m_blupiVitesse.y += 1;
		}
		}
		else
{
			if (m_blupiVitesse.y > -10) m_blupiVitesse.y -= 1;
		}
		end.y += m_blupiVitesse.y;

		if ((m_keyPress & KEY_LEFT) && !(m_keyPress & KEY_RIGHT))
		{
			if (m_blupiVitesse.x > -16)
			{
				m_blupiVitesse.x -= 1;
			}
			tinyPoint.x = end.x + m_blupiVitesse.x;
			tinyPoint.y = end.y;
			if (BlupiBloque(tinyPoint, -1))
			{
				m_blupiVitesse.x = 0;
			}
		}
		else if ((m_keyPress & KEY_RIGHT) && !(m_keyPress & KEY_LEFT))
		{
			if (m_blupiVitesse.x < 16)
			{
				m_blupiVitesse.x += 1;
			}
			tinyPoint.x = end.x + m_blupiVitesse.x;
			tinyPoint.y = end.y;
			if (BlupiBloque(tinyPoint, 1))
			{
				m_blupiVitesse.x = 0;
			}
		}
		else
		{
			if (m_blupiVitesse.x > 0)
			{
				m_blupiVitesse.x -= 2;
				if (m_blupiVitesse.x < 0) m_blupiVitesse.x = 0;
			}
			if (m_blupiVitesse.x < 0)
			{
				m_blupiVitesse.x += 2;
				if (m_blupiVitesse.x > 0) m_blupiVitesse.x = 0;
			}
		}
		end.x += m_blupiVitesse.x;

		MoveObjectPollution();

		if ( m_keyPress & KEY_DOWN && !bAirStart && m_blupiTransport == -1)
		{

			rect.left = m_blupiPos.x + 20;
			rect.right = m_blupiPos.x + 22;
			rect.top = m_blupiPos.y + 60 - 2;
			rect.bottom = m_blupiPos.y + 60;
			bIsOpen = !DecorDetect(rect);
			rect.left = m_blupiPos.x + 60 - 22;
			rect.right = m_blupiPos.x + 60 - 20;
			rect.top = m_blupiPos.y + 60 - 2;
			rect.bottom = m_blupiPos.y + 60;
			bIsOpen2 = !DecorDetect(rect);
			if (!bIsOpen && !bIsOpen2)
			{
				tinyPoint.x = m_blupiPos.x;
				tinyPoint.y = m_blupiPos.y - BLUPIFLOOR;
				ObjectStart(tinyPoint, TYPE_HELICO, 0, TRUE);
				m_blupiHelico = FALSE;
				m_blupiAction = ACTION_STOP;
				m_blupiPhase = 0;
				m_blupiPosHelico = m_blupiPos;
				m_blupiFocus = TRUE;
				StopSound(SOUND_HELICOHIGH);
				StopSound(SOUND_HELICOLOW);
				PlaySound(SOUND_HELICOSTOP, m_blupiPos, FALSE);
			}
		}
	}
	if (m_blupiOver && (m_blupiFocus || m_blupiAction == ACTION_HELICOGLU))
	{
		if (m_keyPress & (KEY_UP | KEY_JUMP) && m_bHelicoStop)
		{
			StopSound(SOUND_HELICOLOW);
			PlaySound(SOUND_HELICOHIGH, m_blupiPos, TRUE);
		}
		if (!(m_keyPress & (KEY_UP | KEY_JUMP)) && m_bHelicoMarch)
		{
			StopSound(SOUND_HELICOHIGH);
			PlaySound(SOUND_HELICOLOW, m_blupiPos, TRUE);
		}
		////m_blupiMotorHigh = bCanUseLift;
		if (m_blupiAction != ACTION_TURN)
		{
			if (bAirStart)
			{
				m_blupiAction = ACTION_MARCH;
			}
			else
			{
				if (m_blupiAction != 1)
				{
					m_blupiAction = ACTION_STOP;
					m_blupiPhase = 0;
					PlaySound(SOUND_OVERSTOP, m_blupiPos, FALSE);
				}
				m_blupiVitesse.y = 0;
			}
		}
		rect = BlupiRect(end);
		rect.top = end.y + 60 - 2;
		rect.bottom = end.y + 60 + OVERHEIGHT - 1;
		BOOL bOverAir = !DecorDetect(rect);
		num = MoveAscenseurDetect(m_blupiPos, OVERHEIGHT);
		if (num != -1)
		{
			bOverAir = FALSE;
		}
		if (m_keyPress & (KEY_JUMP | KEY_UP) && !bOverAir)
		{
			if (m_blupiVitesse.y == 0 && num != -1)
			{
				m_blupiVitesse.y = -5;
			}
			else if (m_blupiVitesse.y > -5)
			{
				m_blupiVitesse.y -= 1;
			}
		}
		else if (m_keyPress & KEY_DOWN)
		{
			if (m_blupiVitesse.y < 12)
			{
				m_blupiVitesse.y += 5;
			}
		}
		else if (m_blupiVitesse.y < 12 && m_time % 2 == 0)
		{
			m_blupiVitesse.y += 1;
		}
		end.y += m_blupiVitesse.y;
		if (m_keyPress & KEY_LEFT && !(m_keyPress & KEY_RIGHT) && bAirStart)
		{
			if (m_blupiVitesse.x > -12)
			{
				m_blupiVitesse.x -= 1;
			}
			tinyPoint.x = end.x + m_blupiVitesse.x;
			tinyPoint.y = end.y;
			if (BlupiBloque(tinyPoint, -1))
			{
				m_blupiVitesse.x = 0;
			}
		}
		else if (m_keyPress & KEY_RIGHT && !(m_keyPress & KEY_LEFT) && bAirStart)
		{
			if (m_blupiVitesse.x < 12)
			{
				m_blupiVitesse.x += 1;
			}
			tinyPoint.x = end.x + m_blupiVitesse.x;
			tinyPoint.y = end.y;
			if (BlupiBloque(tinyPoint, 1))
			{
				m_blupiVitesse.x = 0;
			}
		}
		else
		{
			if (m_blupiVitesse.x > 0)
			{
				m_blupiVitesse.x -= 1;
			}
			if (m_blupiVitesse.x < 0)
			{
				m_blupiVitesse.x += 1;
			}
		}
		end.x += m_blupiVitesse.x;

		MoveObjectPollution();

		if (m_keyPress & KEY_DOWN && !bAirStart && m_blupiTransport == -1)
		{
			
			rect.left = m_blupiPos.x + 20;
			rect.right = m_blupiPos.x + 22;
			rect.top = m_blupiPos.y + 60 - 2;
			rect.bottom = m_blupiPos.y + 60;
			bIsOpen = !DecorDetect(rect);
			rect.left = m_blupiPos.x + 60 - 22;
			rect.right = m_blupiPos.x + 60 - 20;
			rect.top = m_blupiPos.y + 60 - 2;
			rect.bottom = m_blupiPos.y + 60;
			bIsOpen2 = !DecorDetect(rect);
			if (!bIsOpen && !bIsOpen2)
			{
				tinyPoint.x = m_blupiPos.x;
				tinyPoint.y = m_blupiPos.y - BLUPIFLOOR;
				ObjectStart(tinyPoint, TYPE_OVER, 0, TRUE);
				m_blupiOver = FALSE;
				m_blupiAction = ACTION_STOP;
				m_blupiPhase = 0;
				m_blupiPosHelico = m_blupiPos;
				m_blupiFocus = TRUE;
				StopSound(SOUND_HELICOHIGH);
				StopSound(SOUND_HELICOLOW);
				PlaySound(SOUND_HELICOSTOP, m_blupiPos, FALSE);
			}
		}
	}
	if (m_blupiBalloon && m_blupiFocus)
	{
		if (!(m_keyPress & (KEY_JUMP | KEY_UP)))
		{
			if (m_keyPress & KEY_DOWN)
			{
				if (m_blupiVitesse.y < 0 && m_time % 6 == 0) m_blupiVitesse.y += 1;
			}
			else if (m_blupiVitesse.y > -3 && m_time % 6 == 0)
			{
				m_blupiVitesse.y -= 1;
			}
		}
		else
		{
			if (m_blupiVitesse.y > -5 && m_time % 6 == 0) m_blupiVitesse.y -= 1;
		}
		end.y += (int)m_blupiVitesse.y;
		if (m_keyPress & KEY_LEFT && !(m_keyPress & KEY_RIGHT))
		{
			if (m_blupiVitesse.x > -10)
			{
				m_blupiVitesse.x -= 1;
			}
			tinyPoint.x = end.x + m_blupiVitesse.x;
			tinyPoint.y = end.y;
			if (BlupiBloque(tinyPoint, -1))
			{
				m_blupiVitesse.x = 0;
			}
		}
		else if (m_keyPress & KEY_RIGHT && !(m_keyPress & KEY_LEFT))
		{
			if (m_blupiVitesse.x < 10)
			{
				m_blupiVitesse.x += 1;
			}
			tinyPoint.x = end.x + m_blupiVitesse.x;
			tinyPoint.y = end.y;
			if (BlupiBloque(tinyPoint, 1))
			{
				m_blupiVitesse.x = 0;
			}
		}
		else
		{
			if (m_blupiVitesse.x > 0)
			{
				m_blupiVitesse.x -= 2;
				if (m_blupiVitesse.x < 0)
				{
					m_blupiVitesse.x = 0;
				}
			}
			if (m_blupiVitesse.x < 0)
			{
				m_blupiVitesse.x += 2;
				if (m_blupiVitesse.x > 0)
				{
					m_blupiVitesse.x = 0;
				}
			}
		}
		end.x += (int)m_blupiVitesse.x;
	}
	if (m_blupiEcrase && m_blupiFocus)
	{
		if (bAirStart)
		{
			if (m_blupiVitesse.y < 2)
			{
				m_blupiVitesse.y += 1;
			}
		}
		else
		{
			m_blupiVitesse.y = 0;
		}
		end.y += (int)m_blupiVitesse.y;
		if (bAirStart)
		{
			num3 = 7;
		}
		else
		{
			num3 = 4;
		}
		if (m_keyPress & KEY_LEFT && !(m_keyPress & KEY_RIGHT))
		{
			num3 = -num3;
			if (m_blupiVitesse.x > num3)
			{
				m_blupiVitesse.x -= 1;
			}
			tinyPoint.x = end.x + (int)m_blupiVitesse.x;
			tinyPoint.y = end.y;
			if (BlupiBloque(tinyPoint, -1))
			{
				m_blupiVitesse.x = 0;
			}
		}
		else if (m_keyPress & KEY_RIGHT && !(m_keyPress & KEY_LEFT))
		{
			if (m_blupiVitesse.x < num3)
			{
				m_blupiVitesse.x += 1;
			}
			tinyPoint.x = end.x + (int)m_blupiVitesse.x;
			tinyPoint.y = end.y;
			if (BlupiBloque(tinyPoint, 1))
			{
				m_blupiVitesse.x = 0;
			}
		}
		else
		{
			if (m_blupiVitesse.x > 0)
			{
				m_blupiVitesse.x -= 2;
				if (m_blupiVitesse.x < 0)
				{
					m_blupiVitesse.x = 0;
				}
			}
			if (m_blupiVitesse.x < 0)
			{
				m_blupiVitesse.x += 2;
				if (m_blupiVitesse.x > 0)
				{
					m_blupiVitesse.x = 0;
				}
			}
		}
		if (m_blupiVitesse.x == 0 && !bAirStart)
		{
			m_blupiAction = ACTION_STOP;
			m_blupiPhase = 0;
		}
		end.x += (int)m_blupiVitesse.x;
	}
	if (m_blupiJeep && m_blupiFocus)
	{
		if (m_blupiVitesse.x == 0 && m_blupiAction == 2)
		{
			m_blupiAction = ACTION_STOP;
			m_blupiPhase = 0;
		}
		if (m_blupiAction != ACTION_STOP && m_bJeepStop)
		{
			StopSound(SOUND_JEEPLOW);
			PlaySound(SOUND_JEEPHIGH, m_blupiPos, TRUE);
		}
		if (m_blupiAction == ACTION_STOP && m_bJeepMarch)
		{
			StopSound(SOUND_JEEPHIGH);
			PlaySound(SOUND_JEEPLOW, m_blupiPos, TRUE);
		}
		//m_blupiMotorHigh = (m_blupiAction != 1);
		if (bAirStart)
		{
			if (m_blupiVitesse.y < 50)
			{
				m_blupiVitesse.y += 5;
			}
		}
		else
		{
			if (m_blupiVitesse.y != 0)
			{
				PlaySound(SOUND_JUMPEND, m_blupiPos, FALSE);
			}
			m_blupiVitesse.y = 0;
		}
		end.y += (int)m_blupiVitesse.y;
		if (m_blupiTransport == -1)
		{
			rect.left = end.x + 20;
			rect.right = end.x + 60 - 20;
			rect.top = end.y + 60 - 35;
			rect.bottom = end.y + 60 - 1;
			num = AscenseurDetect(rect, m_blupiPos, end);
			if (m_blupiVitesse.y >= 0 && num != -1)
			{
				m_blupiTransport = num;
				bAirStart = FALSE;
				PlaySound(SOUND_JUMPEND, end, FALSE);
				end.y = m_moveObject[num].posCurrent.y - 64 + BLUPIOFFY;
			}
		}
		if ((m_keyPress & KEY_LEFT) && !(m_keyPress & KEY_RIGHT))
		{
			if (m_blupiVitesse.x > -20)
			{
				m_blupiVitesse.x -= 1;
			}
		}
		else if ((m_keyPress & KEY_RIGHT) && !(m_keyPress & KEY_LEFT))
		{
			if (m_blupiVitesse.x < 20)
			{
				m_blupiVitesse.x += 1;
			}
		}
		else
		{
			if (m_blupiVitesse.x > 0)
			{
				m_blupiVitesse.x -= 2;
				if (m_blupiVitesse.x < 0)
				{
					m_blupiVitesse.x = 0;
				}
			}
			if (m_blupiVitesse.x < 0)
			{
				m_blupiVitesse.x += 2;
				if (m_blupiVitesse.x > 0)
				{
					m_blupiVitesse.x = 0;
				}
			}
		}
		if (m_blupiAction == 3)
		{
			m_blupiVitesse.x = 0;
		}
		end.x += (int)m_blupiVitesse.x;
		MoveObjectPollution();
		if (m_keyPress & (KEY_UP | KEY_DOWN) && !bAirStart && m_blupiTransport == -1)
		{
			tinyPoint.x = m_blupiPos.x;
			tinyPoint.y = m_blupiPos.y - BLUPIFLOOR;
			ObjectStart(tinyPoint, TYPE_JEEP, 0, TRUE);
			m_blupiJeep = FALSE;
			m_blupiAction = ACTION_STOP;
			m_blupiPhase = 0;
			m_blupiPosHelico = m_blupiPos;
			StopSound(SOUND_JEEPHIGH);
			StopSound(SOUND_JEEPLOW);
			PlaySound(SOUND_JEEPSTOP, m_blupiPos, FALSE);
		}
	}
	if (m_blupiTank && m_blupiFocus)
	{
		if (m_blupiAction == 53 && m_blupiPhase == 6)
		{
			m_blupiAction = ACTION_STOP;
			m_blupiPhase = 0;
		}
		if (m_keyPress & KEY_FIRE && m_blupiTimeFire == 0 && m_blupiAction != ACTION_TURN)
		{
			if (m_blupiBullet == 0)
			{
				PlaySound(SOUND_FIREKO, m_blupiPos, FALSE);
			}
			else
			{
				if (m_blupiDir == DIR_LEFT)
				{
					tinyPoint.x = m_blupiPos.x - 35;
					tinyPoint.y = m_blupiPos.y;
					num4 = -5;
					m_blupiVitesse.x += 12;
				}
				else
				{
					tinyPoint.x = m_blupiPos.x + 35;
					tinyPoint.y = m_blupiPos.y;
					num4 = 5;
					m_blupiVitesse.x -= 12;
				}
				if (ObjectStart(tinyPoint, TYPE_BALLE, num4, TRUE))
				{
					m_blupiAction = 53;
					m_blupiPhase = 0;
					PlaySound(SOUND_FIREOK, m_blupiPos, FALSE);
					m_blupiTimeFire = 10;
					m_blupiBullet--;
				}
			}
		}
		if (m_blupiVitesse.x == 0 && m_blupiAction == 2)
		{
			m_blupiAction = ACTION_STOP;
			m_blupiPhase = 0;
		}
		if (m_blupiAction != ACTION_STOP && m_bJeepStop)
		{
			StopSound(SOUND_JEEPLOW);
			PlaySound(SOUND_JEEPHIGH, m_blupiPos, TRUE);
		}
		if (m_blupiAction == ACTION_STOP && m_bJeepMarch)
		{
			StopSound(SOUND_JEEPHIGH);
			PlaySound(SOUND_JEEPLOW, m_blupiPos, TRUE);
		}
		//m_blupiMotorHigh = (m_blupiAction != 1);
		if (bAirStart)
		{
			if (m_blupiVitesse.y < 50)
			{
				m_blupiVitesse.y += 5;
			}
		}
		else
		{
			if (m_blupiVitesse.y != 0)
			{
				PlaySound(SOUND_JUMPEND, m_blupiPos, FALSE);
			}
			m_blupiVitesse.y = 0;
		}
		end.y += (int)m_blupiVitesse.y;
		if (m_blupiTransport == -1)
		{
			rect.left = end.x + 20;
			rect.right = end.x + 60 - 20;
			rect.top = end.y + 60 - 35;
			rect.bottom = end.y + 60 - 1;
			num = AscenseurDetect(rect, m_blupiPos, end);
			if (m_blupiVitesse.y >= 0 && num != -1)
			{
				m_blupiTransport = num;
				bAirStart = FALSE;
				PlaySound(SOUND_JUMPEND, end, FALSE);
				end.y = m_moveObject[num].posCurrent.y - 64 + BLUPIOFFY;
			}
		}
		if ((m_keyPress & KEY_LEFT) && !(m_keyPress & KEY_RIGHT))
		{
			if (m_blupiVitesse.x > -12)
			{
				m_blupiVitesse.x -= 1;
			}
		}
		else if ((m_keyPress & KEY_RIGHT) && !(m_keyPress & KEY_LEFT))
		{
			if (m_blupiVitesse.x < 12)
			{
				m_blupiVitesse.x += 1;
			}
		}
		else
		{
			if (m_blupiVitesse.x > 0)
			{
				m_blupiVitesse.x -= 3;
				if (m_blupiVitesse.x < 0)
				{
					m_blupiVitesse.x = 0;
				}
			}
			if (m_blupiVitesse.x < 0)
			{
				m_blupiVitesse.x += 3;
				if (m_blupiVitesse.x > 0)
				{
					m_blupiVitesse.x = 0;
				}
			}
		}
		if (m_blupiAction == 3)
		{
			m_blupiVitesse.x = 0;
		}
		end.x += (int)m_blupiVitesse.x;
		MoveObjectPollution();
		if ( m_keyPress & (KEY_UP | KEY_DOWN) && !bAirStart && m_blupiTransport == -1)
		{

			tinyPoint.x = m_blupiPos.x;
			tinyPoint.y = m_blupiPos.y;
			ObjectStart(tinyPoint, TYPE_TANK, 0, TRUE);
			m_blupiTank = FALSE;
			m_blupiAction = ACTION_STOP;
			m_blupiPhase = 0;
			m_blupiPosHelico = m_blupiPos;
			StopSound(SOUND_JEEPHIGH);
			StopSound(SOUND_JEEPLOW);
			PlaySound(SOUND_JEEPSTOP, m_blupiPos, FALSE);
		}
	}
	if (m_blupiSkate && m_blupiFocus)
	{
		if (!(m_keyPress & ~KEY_FIRE) && m_blupiAction == ACTION_MARCH)
		{
			m_blupiAction = ACTION_STOP;
			m_blupiPhase = 130;
		}
		if ((m_keyPress & KEY_LEFT) && !(m_keyPress & KEY_RIGHT))
		{
			if (m_blupiVitesse.x > -15)
			{
				m_blupiVitesse.x -= 1;
			}
		}
		else if ((m_keyPress & KEY_RIGHT) && !(m_keyPress & KEY_LEFT))
		{
			if (m_blupiVitesse.x < 15)
			{
				m_blupiVitesse.x += 1;
			}
		}
		else
		{
			if (m_blupiVitesse.x > 0)
			{
				m_blupiVitesse.x -= 1;
				if (m_blupiVitesse.x < 0)
				{
					m_blupiVitesse.x = 0;
				}
			}
			if (m_blupiVitesse.x < 0)
			{
				m_blupiVitesse.x += 1;
				if (m_blupiVitesse.x > 0)
				{
					m_blupiVitesse.x = 0;
				}
			}
		}
		if (m_blupiAction == 3)
		{
			m_blupiVitesse.x = 0;
		}
		end.x += (int)m_blupiVitesse.x;
		if ( m_keyPress & (KEY_UP | KEY_DOWN) && !bAirStart && !m_blupiAir && m_blupiTransport == -1 && m_blupiVitesse.x < 8)
		{

			m_blupiSkate = FALSE;
			m_blupiAction = 43;
			m_blupiPhase = 0;
			m_blupiFocus = FALSE;
			m_blupiPosHelico = m_blupiPos;
		}
	}
	if (m_blupiAction == 42)
	{
		if (m_blupiPhase == 8)
		{
			num = MoveObjectDetect(m_blupiPos, &bNear);
			if (num != -1)
			{
				ObjectDelete(m_moveObject[num].posCurrent, m_moveObject[num].type);
			}
		}
		if (m_blupiPhase == 20)
		{
			m_blupiAction = ACTION_STOP;
			m_blupiPhase = 0;
			m_blupiFocus = TRUE;
		}
	}
	if (m_blupiAction == 43)
	{
		if (m_blupiPhase == 12)
		{
			tinyPoint.x = m_blupiPos.x;
			tinyPoint.y = m_blupiPos.y - BLUPIFLOOR + 1;
			ObjectStart(tinyPoint, TYPE_SKATE, 0, TRUE);
		}
		if (m_blupiPhase == 20)
		{
			m_blupiAction = ACTION_STOP;
			m_blupiPhase = 0;
			m_blupiFocus = TRUE;
		}
	}
	if (m_blupiNage && m_blupiFocus)
	{
		if (m_blupiTransport == -1)
		{
			if (!(m_keyPress & (KEY_JUMP | KEY_UP)))
			{
				if (m_keyPress & KEY_DOWN)
{
				if (m_blupiVitesse.y < 5)
				{
					m_blupiVitesse.y += 1;
				}
			}
				else
{
				if (m_blupiAction == 1)
				{
					num3 = -1;
				}
				else
				{
					num3 = 0;
				}
				if (m_blupiVitesse.y > num3)
				{
					m_blupiVitesse.y -= 1;
				}
				if (m_blupiVitesse.y < num3)
				{
					m_blupiVitesse.y += 1;
				}
			}
			}
			else
{
				if (m_blupiVitesse.y > -5)
				{
					m_blupiVitesse.y -= 1;
				}
			}
			end.y += (int)m_blupiVitesse.y;
		}
		if (m_keyPress & KEY_LEFT)
		{
			if (m_blupiVitesse.x > -8)
			{
				m_blupiVitesse.x -= 1;
			}
		}
		else if (m_keyPress & KEY_RIGHT)
		{
			if (m_blupiVitesse.x < 8)
			{
				m_blupiVitesse.x += 1;
			}
		}
		else
		{
			if (m_blupiVitesse.x > 0)
			{
				m_blupiVitesse.x -= 2;
				if (m_blupiVitesse.x < 0)
				{
					m_blupiVitesse.x = 0;
				}
			}
			if (m_blupiVitesse.x < 0)
			{
				m_blupiVitesse.x += 2;
				if (m_blupiVitesse.x > 0)
				{
					m_blupiVitesse.x = 0;
				}
			}
		}
		num = table_vitesse_nage[m_blupiPhase % 14 / 2];
		end.x += (int)(m_blupiVitesse.x * num / 7);
		if (m_time % 70 == 0 || m_time % 70 == 28)
		{
			MoveObjectBlup(end);
		}
		if (m_time % 5 == 0)
		{
			if (!m_blupiShield && !m_bSuperBlupi)
			{
				m_blupiLevel--;
			}
			if (m_blupiLevel == 25)
			{
				m_jauges[JAUGE_AIR].SetType(1);
			}
			m_jauges[JAUGE_AIR].SetLevel(m_blupiLevel);
			if (m_blupiLevel == 0)
			{
				m_blupiAction = ACTION_DROWN;
				m_blupiPhase = 0;
				m_blupiFocus = FALSE;
				m_blupiHelico = FALSE;
				m_blupiOver = FALSE;
				m_blupiJeep = FALSE;
				m_blupiTank = FALSE;
				m_blupiSkate = FALSE;
				m_blupiNage = FALSE;
				m_blupiSurf = FALSE;
				m_blupiSuspend = FALSE;
				m_blupiJumpAie = FALSE;
				m_blupiShield = FALSE;
				m_blupiPower = FALSE;
				m_blupiCloud = FALSE;
				m_blupiHide = FALSE;
				m_blupiInvert = FALSE;
				m_blupiBalloon = FALSE;
				m_blupiEcrase = FALSE;
				m_blupiAir = FALSE;
				m_blupiRestart = TRUE;
				m_blupiActionOuf = 0;
				m_jauges[JAUGE_AIR].SetHide(TRUE);
				m_jauges[JAUGE_POWER].SetHide(TRUE);
				PlaySound(SOUND_DROWN, end, FALSE);
			}
		}
	}
	if (m_blupiSurf && m_blupiFocus)
	{
		if (m_blupiTransport == -1)
		{
			if (!(m_keyPress & (KEY_JUMP | KEY_UP)))
			{
				if (m_keyPress & KEY_DOWN)
{
				if (m_blupiVitesse.y < 5)
				{
					m_blupiVitesse.y += 1;
				}
			}
				else
{
				if (m_blupiVitesse.y > -2)
				{
					m_blupiVitesse.y -= 1;
				}
				if (m_blupiVitesse.y < -2)
				{
					m_blupiVitesse.y += 1;
				}
			}
			}
			else
{
				if (m_blupiVitesse.y > -5)
				{
					m_blupiVitesse.y -= 1;
				}
			}
			end.y += (int)m_blupiVitesse.y;
			end.y += BLUPISURF;
			if (end.y % 64 > 30)
			{
				end.y += 64 - end.y % 64;
			}
			end.y -= BLUPISURF;
		}
		if (m_keyPress & KEY_LEFT)
		{
			if (m_blupiVitesse.x > -8)
			{
				m_blupiVitesse.x -= 1;
			}
		}
		else if (m_keyPress & KEY_RIGHT)
		{
			if (m_blupiVitesse.x < 8)
			{
				m_blupiVitesse.x += 1;
			}
		}
		else
		{
			if (m_blupiVitesse.x > 0)
			{
				m_blupiVitesse.x -= 2;
				if (m_blupiVitesse.x < 0)
				{
					m_blupiVitesse.x = 0;
				}
			}
			if (m_blupiVitesse.x < 0)
			{
				m_blupiVitesse.x += 2;
				if (m_blupiVitesse.x > 0)
				{
					m_blupiVitesse.x = 0;
				}
			}
		}
		num = table_vitesse_surf[m_blupiPhase % 12 / 2];
		end.x += (int)(m_blupiVitesse.x * num / 10);
	}
	POINT tinyPoint4;
	if (m_blupiSuspend && m_blupiFocus)
	{
		if (m_keyPress & KEY_LEFT && m_blupiAction == 2)
		{
			end.x -= 5;
		}
		if (m_keyPress & KEY_RIGHT && m_blupiAction == 2)
		{
			end.x += 5;
		}
		num = GetTypeBarre(end);
		if (num == 2)
		{
		tinyPoint4.x = end.x;
		tinyPoint4.y = end.y / 64 * 64 + BLUPIOFFY;
		rect = BlupiRect(tinyPoint4);
			if (!DecorDetect(rect, TRUE))
			{
				m_blupiSuspend = FALSE;
				m_blupiAction = ACTION_STOP;
				m_blupiPhase = 0;
		end = tinyPoint4;
				m_blupiPos = end;
			}
		}
		if ((m_keyPress & KEY_DOWN && m_blupiPhase > 5) || num == 0)
		{
			m_blupiSuspend = FALSE;
			m_blupiAir = TRUE;
			m_blupiAction = 5;
			end.y = end.y;
			m_blupiVitesse.y = 0;
			m_blupiNoBarre = 5;
			m_blupiActionOuf = 65;
			m_blupiTimeOuf = 0;
		}
		if ((m_keyPress & KEY_JUMP || m_keyPress & KEY_UP) && m_blupiAction != ACTION_JUMP && m_blupiAction != ACTION_TURN)
		{
			m_blupiAction = 4;
			m_blupiPhase = 0;
		}
		if (!(m_keyPress & (KEY_JUMP | KEY_UP)) && m_blupiAction == 4)
		{
			m_blupiAction = ACTION_STOP;
			m_blupiPhase = 0;
		}
		if (m_blupiAction == 4 && m_blupiPhase == 10)
		{
			m_blupiSuspend = FALSE;
			m_blupiAir = TRUE;
			m_blupiAction = ACTION_AIR;
			m_blupiPhase = 0;
			end.y -= 2;
			m_blupiVitesse.y = -11;
			m_blupiNoBarre = 5;
			PlaySound(SOUND_SUSPENDJUMP, end, FALSE);
		}
	}
	if (m_keyPress & KEY_FIRE &&
		!m_blupiHelico &&
		!m_blupiOver &&
		!m_blupiBalloon &&
		!m_blupiEcrase &&
		!m_blupiTank &&
		!m_blupiJeep &&
		!m_blupiSkate &&
		!bAirStart &&
		m_blupiTransport == -1 &&
		m_blupiFocus)
	{
		if (m_blupiDynamite > 0)
		{
			rect.left = end.x + 18;
			rect.right = end.x + 20;
			rect.top = end.y + DIMBLUPIY - 2;
			rect.bottom = end.y + DIMBLUPIY;
			bIsOpen = !DecorDetect(rect);
			rect.left = end.x + DIMBLUPIX - 20;
			rect.right = end.x + DIMBLUPIX - 18;
			rect.top = end.y + DIMBLUPIY - 2;
			rect.bottom = end.y + DIMBLUPIY;
			bIsOpen2 = !DecorDetect(rect);
			if (!bIsOpen && !bIsOpen2 && ObjectStart(end, TYPE_DYNAMITEf, 0, TRUE))
			{
				m_blupiAction = ACTION_OUF2;
				m_blupiPhase = 0;
				m_blupiFocus = FALSE;
				PlaySound(SOUND_PERSOPOSE, end, FALSE);
				m_blupiDynamite--;
			}
			if (m_blupiFocus)
			{
				m_blupiAction = ACTION_OUF3;
				m_blupiPhase = 0;
				PlaySound(SOUND_ERROR, end, FALSE);
			}
		}
		else if (m_blupiPerso > 0)
		{
			num = MoveObjectDetect(end, &bNear);
			if (num == -1 || m_moveObject[num].type != TYPE_BOMBEPERSO1 + m_team)
			{
				rect.left = end.x + 18;
				rect.right = end.x + 20;
				rect.top = end.y + DIMBLUPIY - 2;
				rect.bottom = end.y + DIMBLUPIY;
				bIsOpen = !DecorDetect(rect);
				rect.left = end.x + DIMBLUPIX - 20;
				rect.right = end.x + DIMBLUPIX - 18;
				rect.top = end.y + DIMBLUPIY - 2;
				rect.bottom = end.y + DIMBLUPIY;
				bIsOpen2 = !DecorDetect(rect);
				num = MoveChargeDetect(end);
				if (num == -1 && !bIsOpen && !bIsOpen2 && ObjectStart(end, TYPE_BOMBEPERSO1 + m_team, 0, TRUE))
				{
					m_blupiAction = ACTION_OUF2;
					m_blupiPhase = 0;
					m_blupiFocus = FALSE;
					PlaySound(SOUND_PERSOPOSE, end, FALSE);
					m_blupiPerso--;
				}
			}
			if (m_blupiFocus)
			{
				m_blupiAction = ACTION_OUF3;
				m_blupiPhase = 0;
				PlaySound(SOUND_ERROR, end, FALSE);
			}
		}
		else
		{
			m_blupiAction = ACTION_OUF3;
			m_blupiPhase = 0;
			PlaySound(SOUND_FIREKO, m_blupiPos, FALSE);
		}
	}
	rect = BlupiRect(m_blupiPos);
	tinyPoint4 = end;
	TestPath(rect, m_blupiPos, &end);
	if (bLastBlupiAir && m_blupiPos.x == end.x && m_blupiPos.x != tinyPoint4.x)
	{
		end.y = tinyPoint4.y;
		TestPath(rect, m_blupiPos, &end);
	}
	if (m_blupiVent && m_blupiPos.y == end.y && m_blupiPos.y != tinyPoint4.y)
	{
		end.x = tinyPoint4.x;
		TestPath(rect, m_blupiPos, &end);
	}
	if (m_blupiTransport != -1 && m_blupiPos.x == end.x && m_blupiPos.x != tinyPoint4.x)
	{
		end.y = tinyPoint4.y;
		TestPath(rect, m_blupiPos, &end);
	}
	if (m_blupiHelico || m_blupiOver || m_blupiBalloon || m_blupiEcrase || m_blupiJeep || m_blupiTank || m_blupiSkate || m_blupiNage)
	{
		if (m_blupiPos.x == end.x && m_blupiPos.x != tinyPoint4.x)
		{
			end.y = tinyPoint4.y;
			TestPath(rect, m_blupiPos, &end);
		}
		else if (m_blupiPos.y == end.y && m_blupiPos.y != tinyPoint4.y)
		{
			end.x = tinyPoint4.x;
			TestPath(rect, m_blupiPos, &end);
		}
	}
	POINT blupiPos = m_blupiPos;
	m_blupiPos = end;
	if ((m_blupiAction == ACTION_STOP ||
		m_blupiAction == ACTION_STOPMARCH ||
		m_blupiAction == ACTION_UP ||
		m_blupiAction == ACTION_DOWN) &&
		!m_blupiAir &&
		!m_blupiBalloon &&
		!m_blupiEcrase &&
		!m_blupiJeep &&
		!m_blupiTank &&
		!m_blupiSkate &&
		!m_blupiNage &&
		!m_blupiSurf &&
		!m_blupiSuspend &&
		m_blupiFocus)
	{
		if (m_blupiTransport != -1)
		{
			AscenseurVertigo(m_blupiTransport, &bIsOpen, &bIsOpen2);
		}
		else
		{
			rect.left = end.x + 24;
			rect.right = end.x + 26;
			rect.top = end.y + DIMBLUPIY - 2;
			rect.bottom = end.y + DIMBLUPIY;
			bIsOpen = !DecorDetect(rect);
			rect.left = end.x + DIMBLUPIX - 26;
			rect.right = end.x + DIMBLUPIX - 24;
			rect.top = end.y + DIMBLUPIY - 2;
			rect.bottom = end.y + DIMBLUPIY;
			bIsOpen2 = !DecorDetect(rect);
		}
		if (m_blupiDir == DIR_LEFT && bIsOpen && !bIsOpen2)
		{
			if (m_blupiHelico || m_blupiOver || AscenseurShift(m_blupiTransport))
			{
				m_blupiAction = ACTION_RECEDE;
				m_blupiPhase = 0;
			}
			else
			{
				PlaySound(SOUND_VERTIGO, end, FALSE);
				m_blupiAction = ACTION_VERTIGO;
				m_blupiPhase = 0;
			}
		}
		if (m_blupiDir == DIR_RIGHT && !bIsOpen && bIsOpen2)
		{
			if (m_blupiHelico || m_blupiOver || AscenseurShift(m_blupiTransport))
			{
				m_blupiAction = ACTION_RECEDE;
				m_blupiPhase = 0;
			}
			else
			{
				PlaySound(SOUND_VERTIGO, end, FALSE);
				m_blupiAction = ACTION_VERTIGO;
				m_blupiPhase = 0;
			}
		}
		if (m_blupiAction != ACTION_VERTIGO &&
			m_blupiAction != ACTION_ADVANCE &&
			m_blupiAction != ACTION_RECEDE &&
			(bIsOpen || bIsOpen2))
		{
			if (!m_blupiHelico && !m_blupiOver)
			{
				PlaySound(SOUND_VERTIGO, end, FALSE);
			}
			m_blupiAction = ACTION_ADVANCE;
			m_blupiPhase = 0;
		}
	}
	if (m_blupiAction == ACTION_STOP &&
		m_blupiJeep &&
		!m_blupiHelico &&
		!m_blupiOver &&
		!m_blupiBalloon &&
		!m_blupiEcrase &&
		!m_blupiNage &&
		!m_blupiSurf &&
		!m_blupiSuspend &&
		m_blupiFocus)
	{
		if (m_blupiTransport != -1)
		{
			AscenseurVertigo(m_blupiTransport, &bIsOpen, &bIsOpen2);
		}
		else
		{
			rect.left = end.x + 2;
			rect.right = end.x + 18;
			rect.top = end.y + DIMBLUPIY - 2;
			rect.bottom = end.y + DIMBLUPIY;
			bIsOpen = !DecorDetect(rect);
			rect.left = end.x + DIMBLUPIX - 18;
			rect.right = end.x + DIMBLUPIX - 2;
			rect.top = end.y + DIMBLUPIY - 2;
			rect.bottom = end.y + DIMBLUPIY;
			bIsOpen2 = !DecorDetect(rect);
		}
		if (bIsOpen && !bIsOpen2)
		{
			m_blupiAction = ACTION_ADVANCE;
			m_blupiPhase = 0;
		}
		if (bIsOpen2 && !bIsOpen)
		{
			m_blupiAction = ACTION_ADVANCE;
			m_blupiPhase = 0;
		}
	}
	if (m_blupiAction == ACTION_STOP &&
		m_blupiTank &&
		!m_blupiHelico &&
		!m_blupiOver &&
		!m_blupiBalloon &&
		!m_blupiEcrase &&
		!m_blupiNage &&
		!m_blupiSurf &&
		!m_blupiSuspend &&
		m_blupiFocus)
	{
		if (m_blupiTransport != -1)
		{
			AscenseurVertigo(m_blupiTransport, &bIsOpen, &bIsOpen2);
		}
		else
		{
			rect.left = end.x + 2;
			rect.right = end.x + 18;
			rect.top = end.y + DIMBLUPIY - 2;
			rect.bottom = end.y + DIMBLUPIY;
			bIsOpen = !DecorDetect(rect);
			rect.left = end.x + DIMBLUPIX - 18;
			rect.right = end.x + DIMBLUPIX - 2;
			rect.top = end.y + DIMBLUPIY - 2;
			rect.bottom = end.y + DIMBLUPIY;
			bIsOpen2 = !DecorDetect(rect);
		}
		if (bIsOpen && !bIsOpen2)
		{
			m_blupiAction = ACTION_ADVANCE;
			m_blupiPhase = 0;
		}
		if (bIsOpen2 && !bIsOpen)
		{
			m_blupiAction = ACTION_ADVANCE;
			m_blupiPhase = 0;
		}
	}
	if (m_blupiAction == ACTION_STOP &&
		m_blupiSkate &&
		!m_blupiHelico &&
		!m_blupiOver &&
		!m_blupiBalloon &&
		!m_blupiEcrase &&
		!m_blupiNage &&
		!m_blupiSurf &&
		!m_blupiSuspend &&
		m_blupiFocus)
	{
		if (m_blupiTransport != -1)
		{
			AscenseurVertigo(m_blupiTransport, &bIsOpen, &bIsOpen2);
		}
		else
		{
			rect.left = end.x + 12;
			rect.right = end.x + 19;
			rect.top = end.y + DIMBLUPIY - 2;
			rect.bottom = end.y + DIMBLUPIY;
			bIsOpen = !DecorDetect(rect);
			rect.left = end.x + DIMBLUPIX - 19;
			rect.right = end.x + DIMBLUPIX - 12;
			rect.top = end.y + DIMBLUPIY - 2;
			rect.bottom = end.y + DIMBLUPIY;
			bIsOpen2 = !DecorDetect(rect);
		}
		if (bIsOpen && !bIsOpen2)
		{
			m_blupiAction = ACTION_ADVANCE;
			m_blupiPhase = 0;
		}
		if (bIsOpen2 && !bIsOpen)
		{
			m_blupiAction = ACTION_ADVANCE;
			m_blupiPhase = 0;
		}
	}
	if (m_blupiFocus)
	{
		if (m_blupiAction == ACTION_VERTIGO && m_blupiPhase == 16)
		{
			m_blupiAction = ACTION_RECEDE;
			m_blupiPhase = 0;
		}
		if (m_blupiAction == ACTION_RECEDE && m_blupiPhase == 3)
		{
			m_blupiAction = ACTION_STOP;
			m_blupiPhase = 0;
			m_blupiActionOuf = 0;
		}
		num3 = 5;
		if (m_blupiJeep)
		{
			num3 = 10;
		}
		if (m_blupiTank)
		{
			num3 = 10;
		}
		if (m_blupiSkate)
		{
			num3 = 10;
		}
		if (m_blupiAction == ACTION_ADVANCE && m_blupiPhase == num3)
		{
			m_blupiAction = ACTION_STOP;
			m_blupiPhase = 0;
		}
	}
	BlupiSearchIcon();
	if (m_blupiShield)
	{
		if (m_blupiTimeShield == 10)
		{
			PlaySound(SOUND_STOPSHIELD, m_blupiPos, FALSE);
		}
		if (m_blupiTimeShield == 0)
		{
			m_blupiShield = FALSE;
			m_jauges[JAUGE_POWER].SetHide(TRUE);
		}
		else if (m_time % 5 == 0)
		{
			m_blupiTimeShield--;
			m_jauges[JAUGE_POWER].SetLevel(m_blupiTimeShield);
		}
	}
	if (m_blupiPower)
	{
		if (m_blupiTimeShield == 20)
		{
			PlaySound(SOUND_STOPPOWER, m_blupiPos, FALSE);
		}
		if (m_blupiTimeShield == 0)
		{
			m_blupiPower = FALSE;
			m_jauges[JAUGE_POWER].SetHide(TRUE);
		}
		else if (m_time % 3 == 0)
		{
			m_blupiTimeShield--;
			m_jauges[JAUGE_POWER].SetLevel(m_blupiTimeShield);
		}
	}
	if (m_blupiCloud)
	{
		if (m_blupiTimeShield == 25)
		{
			PlaySound(SOUND_STOPCLOUD, m_blupiPos, FALSE);
		}
		if (m_blupiTimeShield == 0)
		{
			m_blupiCloud = FALSE;
			m_jauges[JAUGE_POWER].SetHide(TRUE);
		}
		else if (m_time % 4 == 0)
		{
			m_blupiTimeShield--;
			m_jauges[JAUGE_POWER].SetLevel(m_blupiTimeShield);
		}
	}
	if (m_blupiHide)
	{
		if (m_blupiTimeShield == 20)
		{
			PlaySound(SOUND_STOPHIDE, m_blupiPos, FALSE);
		}
		if (m_blupiTimeShield == 0)
		{
			m_blupiHide = FALSE;
			m_jauges[JAUGE_POWER].SetHide(TRUE);
		}
		else if (m_time % 4 == 0)
		{
			m_blupiTimeShield--;
			m_jauges[JAUGE_POWER].SetLevel(m_blupiTimeShield);
		}
	}

	if (m_blupiInvert)
	{
		if (m_blupiTimeShield == 0)
		{
			m_blupiInvert = FALSE;
			m_jauges[JAUGE_POWER].SetHide(TRUE);
			tinyPoint.x = m_blupiPos.x;
			tinyPoint.y = m_blupiPos.y + 100;
			ObjectStart(tinyPoint, TYPE_INVERTSTOP, -60, FALSE);
			tinyPoint.x = m_blupiPos.x;
			tinyPoint.y = m_blupiPos.y - 100;
			ObjectStart(tinyPoint, TYPE_INVERTSTOP, 60, FALSE);
			tinyPoint.x = m_blupiPos.x - 100;
			tinyPoint.y = m_blupiPos.y;
			ObjectStart(tinyPoint, TYPE_INVERTSTOP, 10, FALSE);
			tinyPoint.x = m_blupiPos.x + 100;
			tinyPoint.y = m_blupiPos.y;
			ObjectStart(tinyPoint, TYPE_INVERTSTOP, -10, FALSE);
			PlaySound(SOUND_INVERTSTOP, end, FALSE);
		}
		else if (m_time % 3 == 0)
		{
			m_blupiTimeShield--;
			m_jauges[JAUGE_POWER].SetLevel(m_blupiTimeShield);
		}
	}
	if (m_blupiBalloon)
	{
		if (m_blupiTimeShield == 0)
		{
			m_blupiBalloon = FALSE;
			m_jauges[JAUGE_POWER].SetHide(TRUE);
			tinyPoint.x = m_blupiPos.x - 34;
			tinyPoint.y = m_blupiPos.y - 34;
			ObjectStart(tinyPoint, TYPE_EXPLO6, 0, TRUE);
			PlaySound(SOUND_RESSORT, m_blupiPos, FALSE);
		}
		else if (m_time % 2 == 0)
		{
			m_blupiTimeShield--;
			m_jauges[JAUGE_POWER].SetLevel(m_blupiTimeShield);
		}
	}
	if (m_blupiEcrase)
	{
		if (m_blupiTimeShield == 0)
		{
			m_blupiEcrase = FALSE;
			m_blupiAir = TRUE;
			m_jauges[JAUGE_POWER].SetHide(TRUE);
			ObjectStart(m_blupiPos, TYPE_INVERTSTART, -60, FALSE);
			ObjectStart(m_blupiPos, TYPE_INVERTSTART, 60, FALSE);
			ObjectStart(m_blupiPos, TYPE_INVERTSTART, 10, FALSE);
			ObjectStart(m_blupiPos, TYPE_INVERTSTART, -10, FALSE);
			tinyPoint.x = m_blupiPos.x - 34;
			tinyPoint.y = m_blupiPos.y - 34;
			ObjectStart(tinyPoint, TYPE_EXPLO5, 0, TRUE);
			PlaySound(SOUND_RESSORT, m_blupiPos, FALSE);
		}
		else if (m_time % 2 == 0)
		{
			m_blupiTimeShield--;
			m_jauges[JAUGE_POWER].SetLevel(m_blupiTimeShield);
		}
	}
	if (m_blupiPower && abs(m_blupiPos.x - m_blupiPosMagic.x) + abs(m_blupiPos.y - m_blupiPosMagic.y) >= 40)
	{
		num = MoveObjectFree();
		if (num != -1)
		{
			m_moveObject[num].type = TYPE_MAGICTRACK;
			m_moveObject[num].phase = 0;
			m_moveObject[num].posCurrent = m_blupiPos;
			m_moveObject[num].posStart = m_moveObject[num].posCurrent;
			m_moveObject[num].posEnd = m_moveObject[num].posCurrent;
			m_moveObject[num].step = STEP_STOPSTART;
			m_moveObject[num].time = 0;
			MoveObjectStepIcon(num);
			m_blupiPosMagic = m_blupiPos;
		}
	}
	if (m_blupiShield && abs(m_blupiPos.x - m_blupiPosMagic.x) + abs(m_blupiPos.y - m_blupiPosMagic.y) >= 40)
	{
		num = MoveObjectFree();
		if (num != -1)
		{
			m_moveObject[num].type = TYPE_SHIELDTRACK;
			m_moveObject[num].phase = 0;
			m_moveObject[num].posCurrent = m_blupiPos;
			m_moveObject[num].posStart = m_moveObject[num].posCurrent;
			m_moveObject[num].posEnd = m_moveObject[num].posCurrent;
			m_moveObject[num].step = STEP_STOPSTART;
			m_moveObject[num].time = 0;
			MoveObjectStepIcon(num);
			m_blupiPosMagic = m_blupiPos;
		}
	}
	if (m_blupiTimeNoAsc > 0) m_blupiTimeNoAsc--;
	if (m_blupiHelico && m_blupiPos.y > 2 && m_blupiFocus && !m_blupiShield && !m_bSuperBlupi)
	{
		rect = BlupiRect(end);
		rect.top = end.y + 4;
		rect.bottom = end.y + 20;
		if (DecorDetect(rect))
		{
			m_blupiAction = ACTION_STOP;
			m_blupiPhase = 0;
			m_blupiHelico = FALSE;
			tinyPoint.x = m_blupiPos.x - 34;
			tinyPoint.y = m_blupiPos.y - 34;
			ObjectStart(tinyPoint, TYPE_EXPLO2, 0, TRUE);
			m_decorAction = 1;
			m_decorPhase = 0;
			StopSound(SOUND_HELICOHIGH);
			StopSound(SOUND_HELICOLOW);
			StopSound(SOUND_JEEPHIGH);
			StopSound(SOUND_JEEPLOW);
			PlaySound(SOUND_BOUM, m_blupiPos, FALSE);
		}
	}
	if (!m_blupiHelico && !m_blupiOver && !m_blupiBalloon && !m_blupiEcrase && !m_blupiJeep && !m_blupiTank && !m_blupiSkate && m_blupiFocus)
	{
		if (!m_blupiNage && !m_blupiSurf && IsSurfWater(m_blupiPos))
		{
			m_scrollAdd.x = 0;
			m_scrollAdd.y = 0;
			m_blupiAir = FALSE;
			m_blupiNage = FALSE;
			m_blupiSurf = TRUE;
			m_blupiAction = ACTION_STOP;
			m_blupiPhase = 0;
			m_blupiVitesse.x = 0;
			m_blupiVitesse.y = 0;
			MoveObjectPlouf(m_blupiPos);
			if (m_blupiTransport != -1)
			{
				m_blupiPos.y = m_blupiPos.y - 10;
				m_blupiTransport = -1;
			}
			if (m_blupiCloud)
			{
				m_blupiCloud = FALSE;
				m_jauges[JAUGE_POWER].SetHide(TRUE);
			}
		}
		if (!m_blupiNage && !IsSurfWater(m_blupiPos) && IsDeepWater(m_blupiPos))
		{
			if (!m_blupiSurf)
			{
				m_blupiAction = ACTION_STOP;
				m_blupiPhase = 0;
				m_blupiVitesse.x = 0;
				m_blupiVitesse.y = 0;
				MoveObjectPlouf(m_blupiPos);
			}
			m_blupiAir = FALSE;
			m_blupiSurf = FALSE;
			m_blupiNage = TRUE;
			m_blupiLevel = 100;
			m_jauges[JAUGE_AIR].SetLevel(m_blupiLevel);
			m_jauges[JAUGE_AIR].SetType(2);
			m_jauges[JAUGE_AIR].SetHide(FALSE);
		}
		if (m_blupiNage && IsSurfWater(m_blupiPos))
		{
			m_blupiAir = FALSE;
			m_blupiNage = FALSE;
			m_blupiSurf = TRUE;
			PlaySound(SOUND_SURF, m_blupiPos, FALSE);
			m_jauges[JAUGE_AIR].SetHide(TRUE);
		}
		tinyPoint4.x = m_blupiPos.x;
		tinyPoint4.y = m_blupiPos.y - DIMBLUPIY;
		if ((m_blupiSurf || m_blupiNage) && (m_blupiPos.y % DIMOBJY == DIMOBJY - BLUPISURF || m_blupiPos.y % DIMOBJY == DIMOBJY / 2) && IsOutWater(tinyPoint4) && m_keyPress & KEY_JUMP)
		{
			m_blupiNage = FALSE;
			m_blupiSurf = FALSE;
			m_blupiAir = TRUE;
			m_blupiAction = ACTION_AIR;
			m_blupiPhase = 0;
			m_blupiVitesse.x = 0;
			if (m_blupiPower)
			{
				m_blupiVitesse.y = -16;
			}
			else
			{
				m_blupiVitesse.y = -12;
			}
			MoveObjectTiplouf(m_blupiPos);
			PlaySound(SOUND_JUMP0, m_blupiPos, FALSE);
			m_jauges[JAUGE_AIR].SetHide(TRUE);
		}
		if ((m_blupiSurf || m_blupiNage) && IsOutWater(m_blupiPos))
		{
			if (m_blupiVitesse.y < 0)
			{
				if (m_blupiTransport == -1)
				{
					m_blupiPos = blupiPos;
				}
				else
				{
					m_blupiTransport = -1;
					m_blupiNage = FALSE;
					m_blupiSurf = FALSE;
					m_blupiAir = TRUE;
					m_blupiAction = ACTION_AIR;
					m_blupiPhase = 0;
					m_blupiPos.y = m_blupiPos.y - 10;
					m_blupiVitesse.x = 0;
					m_blupiVitesse.y = -10;
					PlaySound(SOUND_JUMP0, m_blupiPos, FALSE);
					m_jauges[JAUGE_AIR].SetHide(TRUE);
				}
			}
			else if (m_blupiVitesse.y > 0)
			{
				m_blupiNage = FALSE;
				m_blupiSurf = FALSE;
				m_blupiAir = FALSE;
				m_blupiAction = ACTION_AIR;
				m_blupiPhase = 0;
				m_blupiPos.y = m_blupiPos.y + 30;
				m_blupiVitesse.x = 0;
				m_blupiVitesse.y = 0;
				PlaySound(SOUND_JUMP0, m_blupiPos, FALSE);
				m_jauges[JAUGE_AIR].SetHide(TRUE);
			}
			else
			{
				m_blupiNage = FALSE;
				m_blupiSurf = FALSE;
				m_blupiAir = FALSE;
				m_blupiAction = ACTION_STOP;
				m_blupiPhase = 0;
				m_blupiPos.y = m_blupiPos.y - 10;
				m_blupiVitesse.x = 0;
				m_blupiVitesse.y = 0;
				PlaySound(SOUND_JUMP0, m_blupiPos, FALSE);
				m_jauges[JAUGE_AIR].SetHide(TRUE);
			}
		}
		if ((m_blupiSurf || m_blupiNage) && m_blupiActionOuf != ACTION_OUF1a)
		{
			m_blupiActionOuf = ACTION_OUF1a;
			m_blupiTimeOuf = 0;
		}
	}
	if ((m_blupiHelico || m_blupiOver || m_blupiJeep || m_blupiTank || m_blupiSkate) && m_blupiFocus && (IsSurfWater(m_blupiPos) || IsDeepWater(m_blupiPos)))
	{

		m_blupiAction = ACTION_STOP;
		m_blupiPhase = 0;
		m_blupiHelico = FALSE;
		m_blupiOver = FALSE;
		m_blupiJeep = FALSE;
		m_blupiTank = FALSE;
		m_blupiSkate = FALSE;
		tinyPoint.x = m_blupiPos.x - 34;
		tinyPoint.y = m_blupiPos.y - 34;
		ObjectStart(tinyPoint, TYPE_EXPLO2, 0, TRUE);
		m_decorAction = 1;
		m_decorPhase = 0;
		StopSound(SOUND_HELICOHIGH);
		StopSound(SOUND_HELICOLOW);
		StopSound(SOUND_JEEPHIGH);
		StopSound(SOUND_JEEPLOW);
		PlaySound(SOUND_BOUM, m_blupiPos, FALSE);
	}
	if (m_blupiFocus && !m_blupiSuspend && !m_blupiHelico && !m_blupiOver && !m_blupiBalloon && !m_blupiEcrase && !m_blupiJeep && !m_blupiTank && !m_blupiSkate && !m_blupiNage && !m_blupiSurf && m_blupiNoBarre == 0 && GetTypeBarre(m_blupiPos) == 1)
	{
		tinyPoint4.x = m_blupiPos.x;
		tinyPoint4.y = (m_blupiPos.y + 22) / DIMOBJY * DIMOBJY + BLUPISUSPEND;
		rect = BlupiRect(tinyPoint4);
		if (!DecorDetect(rect, TRUE))
		{
			m_blupiPos = tinyPoint4;
			m_blupiSuspend = TRUE;
			m_blupiAir = FALSE;
			m_blupiAction = ACTION_STOP;
			m_blupiPhase = 0;
			m_blupiActionOuf = 0;
			PlaySound(SOUND_SUSPENDTOC, m_blupiPos, FALSE);
		}
	}
	if (m_blupiNoBarre > 0)
	{
		m_blupiNoBarre--;
	}
	if (IsVentillo(m_blupiPos))
	{
		if (m_blupiFocus && !m_blupiShield && !m_bSuperBlupi)
		{
			BlupiDead(ACTION_CLEAR1, ACTION_CLEAR2);
		}
		tinyPoint.x = m_blupiPos.x - 34;
		tinyPoint.y = m_blupiPos.y - 34;
		ObjectStart(tinyPoint, TYPE_EXPLO4, 0, TRUE);
		m_decorAction = 2;
		m_decorPhase = 0;
		StopSound(SOUND_HELICOHIGH);
		StopSound(SOUND_HELICOLOW);
		StopSound(SOUND_JEEPHIGH);
		StopSound(SOUND_JEEPLOW);
		PlaySound(SOUND_BOUM, m_blupiPos, FALSE);
	}
	if (m_blupiAction != ACTION_BYE && m_blupiFocus)
	{
		num = IsWorld(m_blupiPos);
		if (num != -1)
		{
			StopSound(SOUND_HELICOHIGH);
			StopSound(SOUND_HELICOLOW);
			StopSound(SOUND_JEEPHIGH);
			StopSound(SOUND_JEEPLOW);
			PlaySound(SOUND_BYE, m_blupiPos, FALSE);
			m_blupiAction = ACTION_BYE;
			m_blupiPhase = 0;
			m_blupiFocus = FALSE;
			m_blupiFront = TRUE;
		}
	}
	POINT tinyPoint2;
	POINT tinyPoint5;
	if (m_blupiAction != ACTION_CLEAR1 &&
		m_blupiAction != ACTION_CLEAR2 &&
		m_blupiAction != ACTION_CLEAR3 &&
		m_blupiAction != ACTION_CLEAR4 &&
		m_blupiAction != ACTION_CLEAR5 &&
		m_blupiAction != ACTION_CLEAR6 &&
		m_blupiAction != ACTION_CLEAR7 &&
		m_blupiAction != ACTION_CLEAR8)
	{
		if (IsLave(m_blupiPos) && !m_blupiShield && !m_bSuperBlupi)
		{
			BlupiDead(ACTION_CLEAR3, -1);
			m_blupiRestart = TRUE;
			m_blupiPos.y = m_blupiPos.y / DIMOBJY * DIMOBJY + BLUPIOFFY;
			PlaySound(SOUND_FALL, m_blupiPos, FALSE);
		}
		if (IsPiege(m_blupiPos) && !m_blupiOver && !m_blupiJeep && !m_blupiTank && !m_blupiShield && !m_bSuperBlupi && m_blupiFocus)
		{
			BlupiDead(ACTION_GLU, -1);
			m_blupiRestart = TRUE;
			m_blupiAir = TRUE;
			ObjectStart(m_blupiPos, TYPE_TENTACULE, 0, TRUE);
			PlaySound(SOUND_GLU, m_blupiPos, FALSE);
		}
		if (IsGoutte(m_blupiPos, FALSE) && !m_blupiOver && !m_blupiJeep && !m_blupiTank && !m_blupiShield && !m_bSuperBlupi && m_blupiFocus)
		{
			BlupiDead(ACTION_GLU, -1);
			m_blupiRestart = TRUE;
			m_blupiAir = TRUE;
			PlaySound(SOUND_GLU, m_blupiPos, FALSE);
		}
		if (IsScie(m_blupiPos) && !m_blupiOver && !m_blupiJeep && !m_blupiTank && !m_blupiShield && !m_bSuperBlupi && m_blupiFocus)
		{
			BlupiDead(ACTION_CLEAR4, -1);
			m_blupiFront = TRUE;
			m_blupiRestart = TRUE;
			m_blupiAir = TRUE;
		}
		if (IsSwitch(m_blupiPos, &tinyPoint) && !m_blupiOver && !m_blupiBalloon && !m_blupiJeep && !m_blupiTank && !m_blupiSkate && !m_blupiShield && !m_bSuperBlupi && m_blupiFocus && m_blupiPosHelico.x == -1)
		{
			
			ActiveSwitch(m_decor[tinyPoint.x][tinyPoint.y].icon == 385, tinyPoint);
			m_blupiAction = ACTION_SWITCH;
			m_blupiPhase = 0;
			m_blupiFocus = FALSE;
			m_blupiVitesse.x = 0;
			m_blupiVitesse.y = 0;
			m_blupiPosHelico = m_blupiPos;
		}
		if (IsBlitz(m_blupiPos, FALSE) && !m_blupiShield && !m_bSuperBlupi)
		{
			BlupiDead(ACTION_CLEAR1, -1);
			m_blupiRestart = TRUE;
			m_blupiAir = TRUE;
			m_blupiPos.y = m_blupiPos.y / DIMOBJY * DIMOBJY + BLUPIOFFY;
			PlaySound(SOUND_FALL, m_blupiPos, FALSE);
		}
		if (IsEcraseur(m_blupiPos) && !m_blupiEcrase && !m_blupiShield && !m_bSuperBlupi && m_blupiFocus)
		{
			m_blupiAction = ACTION_STOP;
			m_blupiPhase = 0;
			m_blupiVitesse.x = 0;
			m_blupiVitesse.y = 0;
			m_blupiEcrase = TRUE;
			m_blupiBalloon = FALSE;
			m_blupiAir = FALSE;
			m_blupiHelico = FALSE;
			m_blupiOver = FALSE;
			m_blupiJeep = FALSE;
			m_blupiTank = FALSE;
			m_blupiSkate = FALSE;
			m_blupiNage = FALSE;
			m_blupiSurf = FALSE;
			m_blupiSuspend = FALSE;
			m_blupiJumpAie = FALSE;
			m_blupiShield = FALSE;
			m_blupiPower = FALSE;
			m_blupiCloud = FALSE;
			m_blupiHide = FALSE;
			m_blupiTimeShield = 100;
			m_blupiPosMagic = m_blupiPos;
			m_jauges[JAUGE_POWER].SetHide(FALSE);
			if (!m_blupiJeep && !m_blupiTank)
			{
				StopSound(SOUND_HELICOHIGH);
				StopSound(SOUND_HELICOLOW);
				StopSound(SOUND_JEEPHIGH);
				StopSound(SOUND_JEEPLOW);
			}
			PlaySound(SOUND_ECRASE, m_blupiPos, FALSE);
			ObjectStart(m_blupiPos, TYPE_INVERTSTART, -60, FALSE);
			ObjectStart(m_blupiPos, TYPE_INVERTSTART, 60, FALSE);
			ObjectStart(m_blupiPos, TYPE_INVERTSTART, 10, FALSE);
			ObjectStart(m_blupiPos, TYPE_INVERTSTART, -10, FALSE);
			tinyPoint.x = m_blupiPos.x - 34;
			tinyPoint.y = m_blupiPos.y - 34;
			ObjectStart(tinyPoint, TYPE_EXPLO5, 0, TRUE);
			m_decorAction = 2;
			m_decorPhase = 0;
		}
		if (IsTeleporte(m_blupiPos) != -1 && !m_blupiHelico && !m_blupiOver && !m_blupiBalloon && !m_blupiEcrase && !m_blupiJeep && !m_blupiTank && !m_blupiSkate && !m_blupiAir && m_blupiFocus && m_blupiPosHelico.x == -1)
		{
			m_blupiAction = ACTION_TELEPORTE;
			m_blupiPhase = 0;
			m_blupiVitesse.x = 0;
			m_blupiVitesse.y = 0;
			m_blupiFocus = FALSE;
			m_blupiPos.x = m_blupiPos.x / DIMOBJX * DIMOBJX;
			PlaySound(71, m_blupiPos, FALSE); // SB2 teleporter sound at 0x10303.
			tinyPoint.x = m_blupiPos.x;
			tinyPoint.y = m_blupiPos.y - 5;
			ObjectStart(tinyPoint, TYPE_EXPLO7, 0, TRUE);
		}
		if (IsBridge(m_blupiPos, &tinyPoint2) && m_blupiFocus)
		{
			tinyPoint2.x *= DIMOBJX;
			tinyPoint2.y *= DIMOBJY;
			ObjectStart(tinyPoint2, TYPE_BRIDGE, 0, TRUE);
		}
		int num2 = IsDoor(m_blupiPos, &tinyPoint2);
		if (num2 != -1 && (m_blupiCle & (1 << (num2 - 334))) != 0)
		{
			OpenDoor(tinyPoint2);
			m_blupiCle &= ~((1 << (num2 - 334)));
			tinyPoint.x = 520;
			tinyPoint.y = 418;
			tinyPoint5.x = tinyPoint2.x * 64 - m_posDecor.x;
			tinyPoint5.y = tinyPoint2.y * 64 - m_posDecor.y;
			VoyageInit(tinyPoint, tinyPoint5, 214 + (num2 - 334) * 7, 10);
		}
	}
	if (!m_blupiHelico && !m_blupiSuspend && !m_blupiOver && !m_blupiBalloon && !m_blupiEcrase && !m_blupiSkate && !m_blupiJeep && !m_blupiTank && m_blupiFocus)
	{
		num = MockeryDetect(m_blupiPos);
		if (num != 0)
		{
			m_blupiActionOuf = num;
			m_blupiTimeOuf = 0;
		}
	}
	MoveObjectFollow(m_blupiPos);
	num = MoveObjectDetect(m_blupiPos, &bNear);
	if (num != -1 && !bNear && m_moveObject[num].type == TYPE_BOMBEDOWN && !m_blupiHelico && !m_blupiOver && !m_blupiBalloon && !m_blupiEcrase && !m_blupiJeep && !m_blupiTank && !m_blupiSkate && !m_blupiNage && !m_blupiSurf && !m_blupiSuspend && !m_blupiShield && !m_bSuperBlupi && m_blupiFocus)
	{
		m_blupiActionOuf = 48;
		m_blupiTimeOuf = 0;
	}
	if (num != -1 && bNear)
	{
		if (m_moveObject[num].type == TYPE_HELICO &&
			!m_blupiHelico && !m_blupiOver && !m_blupiBalloon && !m_blupiEcrase && !m_blupiJeep && !m_blupiTank && !m_blupiSkate &&
			!m_blupiNage &&
			!m_blupiSurf &&
			!m_blupiSuspend &&
			m_blupiFocus && m_blupiPosHelico.x == -1)
		{
			ObjectDelete(m_moveObject[num].posCurrent, m_moveObject[num].type);
			m_scrollAdd.x = 0;
			m_scrollAdd.y = 0;
			m_blupiAir = FALSE;
			m_blupiHelico = TRUE;
			m_blupiVitesse.x = 0;
			PlaySound(SOUND_HELICOSTART, m_blupiPos, FALSE);
			PlaySound(SOUND_HELICOLOW, m_blupiPos, TRUE);
			if (m_blupiCloud || m_blupiHide)
			{
				m_blupiCloud = FALSE;
				m_blupiHide = FALSE;
				m_jauges[JAUGE_POWER].SetHide(TRUE);
			}
		}

		if (m_moveObject[num].type == TYPE_OVER &&
			!m_blupiHelico && !m_blupiOver && !m_blupiBalloon && !m_blupiEcrase && !m_blupiJeep && !m_blupiTank && !m_blupiSkate &&
			!m_blupiNage &&
			!m_blupiSurf &&
			!m_blupiSuspend &&
			m_blupiFocus && m_blupiPosHelico.x == -1)
		{
			ObjectDelete(m_moveObject[num].posCurrent, m_moveObject[num].type);
			m_scrollAdd.x = 0;
			m_scrollAdd.y = 0;
			m_blupiAir = FALSE;
			m_blupiOver = TRUE;
			m_blupiVitesse.x = 0;
			PlaySound(SOUND_HELICOSTART, m_blupiPos, FALSE);
			PlaySound(SOUND_HELICOLOW, m_blupiPos, TRUE);
			if (m_blupiCloud || m_blupiHide)
			{
				m_blupiCloud = FALSE;
				m_blupiHide = FALSE;
				m_jauges[JAUGE_POWER].SetHide(TRUE);
			}
		}

		if (m_moveObject[num].type == TYPE_JEEP &&
			!m_blupiHelico && !m_blupiOver && !m_blupiBalloon && !m_blupiEcrase && !m_blupiJeep && !m_blupiTank && !m_blupiSkate &&
			!m_blupiNage &&
			!m_blupiSurf &&
			!m_blupiSuspend &&
			m_blupiFocus && m_blupiPosHelico.x == -1)
		{
			ObjectDelete(m_moveObject[num].posCurrent, m_moveObject[num].type);
			m_scrollAdd.x = 0;
			m_scrollAdd.y = 0;
			m_blupiAction = ACTION_STOP;
			m_blupiPhase = 0;
			m_blupiAir = FALSE;
			m_blupiJeep = TRUE;
			m_blupiVitesse.x = 0;
			PlaySound(SOUND_JEEPLOW, m_blupiPos, TRUE);
			if (m_blupiCloud || m_blupiHide)
			{
				m_blupiCloud = FALSE;
				m_blupiHide = FALSE;
				m_jauges[JAUGE_POWER].SetHide(TRUE);
			}
		}
		if ( m_moveObject[num].type == TYPE_TANK &&
			!m_blupiHelico && !m_blupiOver && !m_blupiBalloon && !m_blupiEcrase && !m_blupiJeep && !m_blupiTank && !m_blupiSkate &&
			!m_blupiNage &&
			!m_blupiSurf &&
			!m_blupiSuspend &&
			m_blupiFocus && m_blupiPosHelico.x == -1)
		{

			ObjectDelete(m_moveObject[num].posCurrent, m_moveObject[num].type);
			m_scrollAdd.x = 0;
			m_scrollAdd.y = 0;
			m_blupiAction = ACTION_STOP;
			m_blupiPhase = 0;
			m_blupiAir = FALSE;
			m_blupiTank = TRUE;
			m_blupiVitesse.x = 0;
			PlaySound(SOUND_JEEPLOW, m_blupiPos, TRUE);
			if (m_blupiCloud || m_blupiHide)
			{
				m_blupiCloud = FALSE;
				m_blupiHide = FALSE;
				m_jauges[JAUGE_POWER].SetHide(TRUE);
			}
		}
		if (m_moveObject[num].type == TYPE_BULLET && m_blupiFocus && m_blupiBullet < 10)
		{
			ObjectDelete(m_moveObject[num].posCurrent, m_moveObject[num].type);
			tinyPoint.x = m_moveObject[num].posCurrent.x - m_posDecor.x;
			tinyPoint.y = m_moveObject[num].posCurrent.y - m_posDecor.y;
		tinyPoint5.x = 428;
		tinyPoint5.y = 430;
		VoyageInit(tinyPoint, tinyPoint5, 177, 10);
			m_blupiBullet += 10;
			if (m_blupiBullet > 10)
			{
				m_blupiBullet = 10;
			}
		}
		if ( m_moveObject[num].type == TYPE_SKATE &&
			!m_blupiHelico && !m_blupiOver && !m_blupiBalloon && !m_blupiEcrase && !m_blupiJeep && !m_blupiTank && !m_blupiSkate &&
			!m_blupiNage &&
			!m_blupiSurf &&
			!m_blupiSuspend &&
			m_blupiFocus && m_blupiPosHelico.x == -1)
		{
			m_scrollAdd.x = 0;
			m_scrollAdd.y = 0;
			m_blupiAction = ACTION_TAKESKATE;
			m_blupiPhase = 0;
			m_blupiPos.y = m_moveObject[num].posCurrent.y / DIMOBJY * DIMOBJY + BLUPIOFFY;
			m_blupiFocus = FALSE;
			m_blupiAir = FALSE;
			m_blupiSkate = TRUE;
			m_blupiVitesse.x = 0;
			if (m_blupiCloud || m_blupiHide)
			{
				m_blupiCloud = FALSE;
				m_blupiHide = FALSE;
				m_jauges[JAUGE_POWER].SetHide(TRUE);
			}
		}

		if ((m_moveObject[num].type == TYPE_BOMBEUP ||
			m_moveObject[num].type == TYPE_BOMBEMOVE ||
			m_moveObject[num].type == TYPE_BOMBEFOLLOW1 ||
			m_moveObject[num].type == TYPE_BOMBEFOLLOW2) &&
			m_blupiBalloon &&
			m_blupiPosHelico.x == -1)
		{
			m_blupiBalloon = FALSE;
			m_blupiAir = TRUE;
			m_blupiTimeShield = 0;
			m_jauges[JAUGE_POWER].SetHide(TRUE);
			m_decorAction = 0;
			tinyPoint.x = m_blupiPos.x - 34;
			tinyPoint.y = m_blupiPos.y - 34;
			ObjectStart(tinyPoint, TYPE_EXPLO6, 0, TRUE);
			PlaySound(SOUND_RESSORT, m_blupiPos, FALSE);
			m_blupiPos.y = m_blupiPos.y + 4;
			m_blupiVitesse.y = 0;
			m_blupiPosHelico = m_blupiPos;
		}
		else if ((m_moveObject[num].type == TYPE_BOMBEDOWN ||
			m_moveObject[num].type == TYPE_BOMBEUP ||
			m_moveObject[num].type == TYPE_BOMBEFOLLOW1 ||
			m_moveObject[num].type == TYPE_BOMBEFOLLOW2 ||
			m_moveObject[num].type == TYPE_BOMBEMOVE ||
			m_moveObject[num].type == TYPE_BULLDOZER ||
			m_moveObject[num].type == TYPE_POISSON ||
			m_moveObject[num].type == TYPE_OISEAU) &&
			!m_blupiShield &&
			!m_bSuperBlupi &&
			m_blupiPosHelico.x == -1)
		{
			if (!m_blupiJeep &&
				!m_blupiTank &&
				!m_blupiSkate &&
				(m_blupiFocus || m_blupiAction == ACTION_AIR || m_blupiAction == ACTION_JUMPAIE))
			{
				if (m_blupiHelico || m_blupiOver || m_blupiBalloon || m_blupiEcrase)
				{
					m_blupiAir = TRUE;
				}
				BlupiDead(ACTION_CLEAR1, ACTION_CLEAR2);
			}
			if (m_moveObject[num].type == TYPE_POISSON || m_moveObject[num].type == TYPE_OISEAU)
			{
				tinyPoint = m_moveObject[num].posCurrent;
				ObjectDelete(tinyPoint, m_moveObject[num].type);
				tinyPoint.x -= 34;
				tinyPoint.y -= 34;
				ObjectStart(tinyPoint, TYPE_EXPLO3, 0, TRUE);
				m_decorAction = 2;
				m_decorPhase = 0;
			}
			else
			{
				tinyPoint = m_moveObject[num].posCurrent;
				ObjectDelete(tinyPoint, m_moveObject[num].type);
				tinyPoint.x -= 34;
				tinyPoint.y -= 34;
				ObjectStart(tinyPoint, TYPE_EXPLO1, 0, TRUE);
				m_decorAction = 1;
				m_decorPhase = 0;
			}
			if (!m_blupiJeep && !m_blupiTank)
			{
				StopSound(SOUND_HELICOHIGH);
				StopSound(SOUND_HELICOLOW);
				StopSound(SOUND_JEEPHIGH);
				StopSound(SOUND_JEEPLOW);
			}
			PlaySound(SOUND_BOUM, m_moveObject[num].posCurrent, FALSE);
		}
		if (m_moveObject[num].type == TYPE_GUEPE &&
			m_blupiFocus &&
			!m_blupiBalloon &&
			!m_blupiShield &&
			!m_bSuperBlupi)
		{
			m_blupiAction = ACTION_STOP;
			m_blupiPhase = 0;
			m_blupiVitesse.x = 0;
			m_blupiVitesse.y = 0;
			m_blupiBalloon = TRUE;
			m_blupiEcrase = FALSE;
			m_blupiAir = FALSE;
			m_blupiHelico = FALSE;
			m_blupiOver = FALSE;
			m_blupiJeep = FALSE;
			m_blupiTank = FALSE;
			m_blupiSkate = FALSE;
			m_blupiNage = FALSE;
			m_blupiSurf = FALSE;
			m_blupiSuspend = FALSE;
			m_blupiJumpAie = FALSE;
			m_blupiShield = FALSE;
			m_blupiPower = FALSE;
			m_blupiCloud = FALSE;
			m_blupiHide = FALSE;
			m_blupiTimeShield = 100;
			m_blupiPosMagic = m_blupiPos;
			m_jauges[JAUGE_POWER].SetHide(FALSE);
			if (!m_blupiJeep && !m_blupiTank)
			{
				StopSound(SOUND_HELICOHIGH);
				StopSound(SOUND_HELICOLOW);
				StopSound(SOUND_JEEPHIGH);
				StopSound(SOUND_JEEPLOW);
			}
			PlaySound(SOUND_JUMPAIE, m_moveObject[num].posCurrent, FALSE);
			tinyPoint.x = m_blupiPos.x - 34;
			tinyPoint.y = m_blupiPos.y - 34;
			ObjectStart(tinyPoint, TYPE_EXPLO5, 0, TRUE);
			m_decorAction = 5;
			m_decorPhase = 0;
		}
		if (m_moveObject[num].type == TYPE_CREATURE &&
			m_moveObject[num].step != STEP_ADVANCE &&
			m_moveObject[num].step != STEP_RECEDE &&
			m_blupiFocus &&
			!m_blupiBalloon &&
			!m_blupiShield &&
			!m_bSuperBlupi)
		{
			
			tinyPoint.x = m_blupiPos.x;
			tinyPoint.y = (m_blupiPos.y + DIMOBJY - 10) / DIMOBJY * DIMOBJY + 4;
			ObjectStart(tinyPoint, TYPE_TENTACULE, 0, TRUE);
			m_blupiAction = ACTION_GLU;
			m_blupiPhase = 0;
			m_blupiSuspend = FALSE;
			m_blupiJumpAie = FALSE;
			m_blupiFocus = FALSE;
			m_blupiRestart = TRUE;
			if (bAirStart)
			{
				m_blupiAir = TRUE;
			}
			if (!m_blupiHelico && !m_blupiOver && !m_blupiBalloon && !m_blupiEcrase && !m_blupiJeep && !m_blupiTank && !m_blupiSkate)
			{
				PlaySound(SOUND_GLU, m_moveObject[num].posCurrent, FALSE);
			}
			else
			{
				m_blupiHelico = FALSE;
				m_blupiOver = FALSE;
				m_blupiBalloon = FALSE;
				m_blupiEcrase = FALSE;
				m_blupiJeep = FALSE;
				m_blupiTank = FALSE;
				m_blupiSkate = FALSE;
				tinyPoint = m_moveObject[num].posCurrent;
				tinyPoint.x -= 34;
				tinyPoint.y -= 34;
				ObjectStart(tinyPoint, TYPE_EXPLO3, 0, TRUE);
				StopSound(SOUND_HELICOHIGH);
				StopSound(SOUND_HELICOLOW);
				StopSound(SOUND_JEEPHIGH);
				StopSound(SOUND_JEEPLOW);
				PlaySound(SOUND_BOUM, m_moveObject[num].posCurrent, FALSE);
				m_decorAction = 1;
				m_decorPhase = 0;
			}
			m_blupiCloud = FALSE;
			m_blupiHide = FALSE;
			m_jauges[JAUGE_POWER].SetHide(TRUE);
		}
		if (m_moveObject[num].type == TYPE_BALLE &&
			!m_blupiShield &&
			!m_bSuperBlupi &&
			m_blupiAction != ACTION_WIN &&
			m_blupiAction != ACTION_BYE &&
			m_blupiAction != ACTION_CLEAR1 &&
			m_blupiAction != ACTION_CLEAR2 &&
			m_blupiAction != ACTION_CLEAR3 &&
			m_blupiAction != ACTION_CLEAR4 &&
			m_blupiAction != ACTION_CLEAR5 &&
			m_blupiAction != ACTION_CLEAR6 &&
			m_blupiAction != ACTION_CLEAR7 &&
			m_blupiAction != ACTION_CLEAR8 &&
			m_blupiAction != ACTION_GLU &&
			m_blupiAction != 57 &&
			m_blupiAction != ACTION_HIDE)
		{
			
			tinyPoint = m_moveObject[num].posCurrent;
			ObjectDelete(tinyPoint, m_moveObject[num].type);
			m_blupiAction = ACTION_GLU;
			m_blupiPhase = 0;
			m_blupiSuspend = FALSE;
			m_blupiJumpAie = FALSE;
			m_blupiFocus = FALSE;
			m_blupiRestart = TRUE;
			if (bAirStart)
			{
				m_blupiAir = TRUE;
			}
			if (m_blupiHelico || m_blupiOver || m_blupiBalloon || m_blupiEcrase || m_blupiJeep || m_blupiTank || m_blupiSkate)
			{
				m_blupiHelico = FALSE;
				m_blupiOver = FALSE;
				m_blupiBalloon = FALSE;
				m_blupiEcrase = FALSE;
				m_blupiJeep = FALSE;
				m_blupiTank = FALSE;
				m_blupiSkate = FALSE;
			}
			StartSploutchGlu(m_moveObject[num].posCurrent);
			m_blupiCloud = FALSE;
			m_blupiHide = FALSE;
			m_jauges[JAUGE_POWER].SetHide(TRUE);
		}
		if (m_moveObject[num].type == TYPE_TRESOR)
		{
			ObjectDelete(m_moveObject[num].posCurrent, m_moveObject[num].type);
			if (m_mission == 10) { // BUG: VoyageInit is skipped in custom levels with id 10. To fix, add `&& !m_bPrivate`
				m_nbTresor++;
				OpenDoorsTresor();
				PlaySound(SOUND_TRESOR, m_moveObject[num].posCurrent, FALSE);
			}
			else {
				tinyPoint.x = m_moveObject[num].posCurrent.x - m_posDecor.x;
				tinyPoint.y = m_moveObject[num].posCurrent.y - m_posDecor.y;
				POINT end;
				end.x = 572;
				end.y = 436;
				VoyageInit(tinyPoint, end, 6, CHELEMENT);
			}
			ObjectStart(m_moveObject[num].posCurrent, TYPE_TRESORTRACK, -60, FALSE);
			ObjectStart(m_moveObject[num].posCurrent, TYPE_TRESORTRACK, 60, FALSE);
			ObjectStart(m_moveObject[num].posCurrent, TYPE_TRESORTRACK, 10, FALSE);
			ObjectStart(m_moveObject[num].posCurrent, TYPE_TRESORTRACK, -10, FALSE);
		}
		if (m_moveObject[num].type == TYPE_CLE1 &&
			(m_voyageIcon != 215 || m_voyageChannel != CHELEMENT) &&
			!(m_blupiCle & CLE_RED))
		{
			ObjectDelete(m_moveObject[num].posCurrent, m_moveObject[num].type);
			tinyPoint.x = m_moveObject[num].posCurrent.x - m_posDecor.x;
			tinyPoint.y = m_moveObject[num].posCurrent.y - m_posDecor.y;
			POINT end2;
			end2.x = 520;
			end2.y = 418;
			VoyageInit(tinyPoint, end2, 215, CHELEMENT);
			ObjectStart(m_moveObject[num].posCurrent, TYPE_TRESORTRACK, -60, FALSE);
			ObjectStart(m_moveObject[num].posCurrent, TYPE_TRESORTRACK, 60, FALSE);
			ObjectStart(m_moveObject[num].posCurrent, TYPE_TRESORTRACK, 10, FALSE);
			ObjectStart(m_moveObject[num].posCurrent, TYPE_TRESORTRACK, -10, FALSE);
		}
		if (m_moveObject[num].type == TYPE_CLE2 &&
			(m_voyageIcon != 222 || m_voyageChannel != CHELEMENT) &&
			!(m_blupiCle & CLE_GREEN))
		{
			ObjectDelete(m_moveObject[num].posCurrent, m_moveObject[num].type);
			tinyPoint.x = m_moveObject[num].posCurrent.x - m_posDecor.x;
			tinyPoint.y = m_moveObject[num].posCurrent.y - m_posDecor.y;
			POINT end3;
			end3.x = 530;
			end3.y = 418;
			VoyageInit(tinyPoint, end3, 222, CHELEMENT);
			ObjectStart(m_moveObject[num].posCurrent, TYPE_TRESORTRACK, -60, FALSE);
			ObjectStart(m_moveObject[num].posCurrent, TYPE_TRESORTRACK, 60, FALSE);
			ObjectStart(m_moveObject[num].posCurrent, TYPE_TRESORTRACK, 10, FALSE);
			ObjectStart(m_moveObject[num].posCurrent, TYPE_TRESORTRACK, -10, FALSE);
		}
		if (m_moveObject[num].type == TYPE_CLE3 &&
			(m_voyageIcon != 229 || m_voyageChannel != CHELEMENT) &&
			!(m_blupiCle & CLE_BLUE))
		{
			ObjectDelete(m_moveObject[num].posCurrent, m_moveObject[num].type);
			tinyPoint.x = m_moveObject[num].posCurrent.x - m_posDecor.x;
			tinyPoint.y = m_moveObject[num].posCurrent.y - m_posDecor.y;
			POINT end4;
			end4.x = 540;
			end4.y = 418;
			VoyageInit(tinyPoint, end4, 229, CHELEMENT);
			ObjectStart(m_moveObject[num].posCurrent, TYPE_TRESORTRACK, -60, FALSE);
			ObjectStart(m_moveObject[num].posCurrent, TYPE_TRESORTRACK, 60, FALSE);
			ObjectStart(m_moveObject[num].posCurrent, TYPE_TRESORTRACK, 10, FALSE);
			ObjectStart(m_moveObject[num].posCurrent, TYPE_TRESORTRACK, -10, FALSE);
		}
		if (m_moveObject[num].type == TYPE_EGG && m_nbVies < 10 && m_blupiFocus)
		{
			ObjectDelete(m_moveObject[num].posCurrent, m_moveObject[num].type);
			tinyPoint.x = m_moveObject[num].posCurrent.x - m_posDecor.x;
			tinyPoint.y = m_moveObject[num].posCurrent.y - m_posDecor.y;
			VoyageInit(tinyPoint, VoyageGetPosVie(m_nbVies + 1), 21, CHELEMENT);
		}
		if (m_moveObject[num].type == TYPE_SHIELD && !m_blupiShield && !m_blupiPower && m_blupiFocus)
		{
			PlaySound(SOUND_STARTSHIELD, m_moveObject[num].posCurrent, FALSE);
			ObjectDelete(m_moveObject[num].posCurrent, m_moveObject[num].type);
			m_blupiShield = TRUE;
			m_blupiPower = FALSE;
			m_blupiCloud = FALSE;
			m_blupiHide = FALSE;
			m_blupiTimeShield = 100;
			m_blupiPosMagic = m_blupiPos;
			m_jauges[JAUGE_POWER].SetHide(FALSE);
		}
		if ( m_moveObject[num].type == TYPE_POWER &&
			!m_blupiShield && !m_blupiPower &&
			!m_blupiHelico && !m_blupiOver && !m_blupiBalloon && !m_blupiEcrase && !m_blupiJeep && !m_blupiTank && !m_blupiSkate &&
			m_blupiFocus)
		{
			ObjectDelete(m_moveObject[num].posCurrent, m_moveObject[num].type);
			m_blupiAction = ACTION_SUCETTE;
			m_blupiPhase = 0;
			m_blupiCloud = FALSE;
			m_blupiHide = FALSE;
			m_blupiFocus = FALSE;
			PlaySound(SOUND_SUCETTE, end, FALSE);
		}

		if (m_moveObject[num].type == TYPE_INVERT && m_blupiFocus)
		{
			ObjectDelete(m_moveObject[num].posCurrent, m_moveObject[num].type);
			m_blupiInvert = TRUE;
			m_blupiTimeShield = 100;
			m_blupiPosMagic = m_blupiPos;
			m_jauges[JAUGE_POWER].SetHide(FALSE);
			PlaySound(SOUND_INVERTSTART, end, FALSE);
			ObjectStart(m_blupiPos, TYPE_INVERTSPIN, -56, FALSE);
			ObjectStart(m_blupiPos, TYPE_INVERTSTART, -60, FALSE);
			ObjectStart(m_blupiPos, TYPE_INVERTSTART, 60, FALSE);
			ObjectStart(m_blupiPos, TYPE_INVERTSTART, 10, FALSE);
			ObjectStart(m_blupiPos, TYPE_INVERTSTART, -10, FALSE);
			int keyPress = m_keyPress;
			if (keyPress & KEY_LEFT)
			{
				m_keyPress = (m_keyPress & ~KEY_LEFT) | KEY_RIGHT;
			}
			if (keyPress & KEY_RIGHT)
			{
				m_keyPress = (m_keyPress & ~KEY_RIGHT) | KEY_LEFT;
			}
		}
		if ( m_moveObject[num].type == TYPE_DRINK &&
			!m_blupiShield && !m_blupiPower &&
			!m_blupiCloud && !m_blupiHide &&
			!m_blupiHelico && !m_blupiOver && !m_blupiBalloon && !m_blupiEcrase && !m_blupiJeep && !m_blupiTank && !m_blupiSkate &&
			m_blupiFocus)
		{
			ObjectDelete(m_moveObject[num].posCurrent, m_moveObject[num].type);
			m_blupiAction = ACTION_DRINK;
			m_blupiPhase = 0;
			m_blupiShield = FALSE;
			m_blupiPower = FALSE;
			m_blupiJumpAie = FALSE;
			m_blupiFocus = FALSE;
			PlaySound(SOUND_DRINK, end, FALSE);
		}
		if (m_moveObject[num].type == TYPE_CHARGE &&
			!m_blupiShield &&
			!m_blupiPower &&
			!m_blupiCloud &&
			!m_blupiHelico && !m_blupiOver && !m_blupiBalloon && !m_blupiEcrase && !m_blupiJeep && !m_blupiTank && !m_blupiSkate &&
			m_blupiFocus)
		{
			m_blupiAction = ACTION_CHARGE;
			m_blupiPhase = 0;
			m_blupiShield = FALSE;
			m_blupiPower = FALSE;
			m_blupiJumpAie = FALSE;
			m_blupiFocus = FALSE;
			m_blupiCloud = TRUE;
			m_blupiTimeShield = 100;
			PlaySound(SOUND_CHARGE, end, FALSE);
			if (m_blupiHide)
			{
				m_blupiHide = FALSE;
				m_jauges[JAUGE_POWER].SetHide(TRUE);
			}
		}
		if (m_moveObject[num].type >= TYPE_BOMBEPERSO1 &&
			m_moveObject[num].type <= TYPE_BOMBEPERSO4 &&
			m_blupiFocus)
		{
			if (m_moveObject[num].type - TYPE_BOMBEPERSO1 == m_team)
			{
				if (m_blupiPerso < 5 && m_blupiAction == ACTION_DOWN)
				{

					ObjectDelete(m_moveObject[num].posCurrent, m_moveObject[num].type);
					tinyPoint.x = m_moveObject[num].posCurrent.x - m_posDecor.x;
					tinyPoint.y = m_moveObject[num].posCurrent.y - m_posDecor.y;
			tinyPoint5.x = 465;
			tinyPoint5.y = 438;
			VoyageInit(tinyPoint, tinyPoint5, GetIconPerso(), CHBUTTON);
				}
			}
			else if (!m_blupiShield && !m_bSuperBlupi)
			{
				ObjectDelete(m_moveObject[num].posCurrent, m_moveObject[num].type);
				BlupiDead(ACTION_CLEAR1, ACTION_CLEAR2);
				tinyPoint = m_moveObject[num].posCurrent;
				tinyPoint.x -= 34;
				tinyPoint.y -= 34;
				ObjectStart(tinyPoint, TYPE_EXPLO3, 0, TRUE);
				PlaySound(SOUND_BOUM, m_moveObject[num].posCurrent, FALSE);
				m_decorAction = 1;
				m_decorPhase = 0;
			}
		}
		if (m_moveObject[num].type == TYPE_DYNAMITE &&
			m_blupiFocus &&
			m_blupiDynamite == 0 &&
			(m_voyageIcon != 252 || m_voyageChannel != CHELEMENT) &&
			m_blupiAction == ACTION_DOWN)
		{
			
			ObjectDelete(m_moveObject[num].posCurrent, m_moveObject[num].type);
			tinyPoint.x = m_moveObject[num].posCurrent.x - m_posDecor.x;
			tinyPoint.y = m_moveObject[num].posCurrent.y - m_posDecor.y;
			tinyPoint5.x = 505;
			tinyPoint5.y = 414;
			VoyageInit(tinyPoint, tinyPoint5, 252, CHELEMENT);
		}
		if (m_moveObject[num].type == 12 &&
			!m_blupiHelico && !m_blupiOver && !m_blupiBalloon && !m_blupiEcrase && !m_blupiJeep && !m_blupiTank && !m_blupiSkate &&
			!m_blupiNage &&
			!m_blupiSurf &&
			!m_blupiSuspend &&
			m_blupiFocus &&
			m_blupiAction == ACTION_MARCH)
		{
			end = m_moveObject[num].posCurrent;
			if (m_blupiDir == DIR_LEFT && m_blupiPos.x > end.x)
			{
				end.x = m_blupiPos.x - 59;
				PlaySound(SOUND_PUSH, end, FALSE);
				m_blupiActionOuf = ACTION_OUF1b;
				m_blupiTimeOuf = 0;
				m_blupiAction = ACTION_PUSH;
				m_blupiPhase = 0;
			}
			if (m_blupiDir == DIR_RIGHT && m_blupiPos.x < end.x)
			{
				end.x = m_blupiPos.x + 55;
				PlaySound(SOUND_PUSH, end, FALSE);
				m_blupiActionOuf = ACTION_OUF1b;
				m_blupiTimeOuf = 0;
				m_blupiAction = ACTION_PUSH;
				m_blupiPhase = 0;
			}
			if (!TestPushCaisse(num, end, FALSE))
			{
				m_blupiPos.x = blupiPos.x;
			}
		}
		if ((m_moveObject[num].type == TYPE_GOAL || m_moveObject[num].type == TYPE_CLE) && m_blupiFocus)
		{
			if (m_goalPhase == 0)
			{
				if (m_nbTresor >= m_totalTresor)
				{
					if (m_moveObject[num].type == TYPE_CLE)
					{
						m_bFoundCle = TRUE;
					}
					StopSound(SOUND_HELICOHIGH);
					StopSound(SOUND_HELICOLOW);
					StopSound(SOUND_JEEPHIGH);
					StopSound(SOUND_JEEPLOW);
					PlaySound(SOUND_ENDOK, m_moveObject[num].posCurrent, FALSE);
					m_blupiAction = ACTION_WIN;
					m_blupiPhase = 0;
					m_blupiFocus = FALSE;
					m_blupiFront = TRUE;
					m_blupiPos.y = m_moveObject[num].posCurrent.y;
				}
				else
				{
					PlaySound(SOUND_ENDKO, m_moveObject[num].posCurrent, FALSE);
				}
				m_goalPhase = 50;
			}
			else
			{
				m_goalPhase--;
			}
		}
		else
		{
			m_goalPhase = 0;
		}
	}
	else
	{
		m_goalPhase = 0;
	}
	NetPlayerCollide(m_blupiPos, &num);
	if (num != -1 && m_netBlupiSec[num] == 3 &&
		!m_blupiNage && !m_blupiSurf && !m_blupiShield &&
		!m_blupiCloud && !m_blupiHide && !m_bSuperBlupi && m_blupiFocus)
	{
		NetStopCloud(num);
		m_blupiAction = 57;
		m_blupiPhase = 0;
		m_blupiSuspend = FALSE;
		m_blupiJumpAie = FALSE;
		m_blupiFocus = FALSE;
		m_blupiRestart = FALSE;
		if (bAirStart) m_blupiAir = TRUE;
		if (m_blupiHelico || m_blupiOver || m_blupiBalloon || m_blupiEcrase || m_blupiJeep || m_blupiTank || m_blupiSkate)
		{
			if (m_blupiHelico)
			{
				tinyPoint.x = m_blupiPos.x;
				tinyPoint.y = m_blupiPos.y;
				tinyPoint.y -= 2;
				ObjectStart(tinyPoint, TYPE_HELICO, 0, TRUE);
			}
			if (m_blupiOver)
			{
				tinyPoint.x = m_blupiPos.x;
				tinyPoint.y = m_blupiPos.y;
				tinyPoint.y -= 2;
				ObjectStart(tinyPoint, TYPE_OVER, 0, TRUE);
			}
			if (m_blupiTransport == -1)
			{
				if (m_blupiJeep) { tinyPoint.x = m_blupiPos.x;
				tinyPoint.y = m_blupiPos.y; ObjectStart(tinyPoint, TYPE_JEEP, 0, TRUE); }
				if (m_blupiTank) { tinyPoint.x = m_blupiPos.x;
				tinyPoint.y = m_blupiPos.y; ObjectStart(tinyPoint, TYPE_TANK, 0, TRUE); }
				if (m_blupiSkate)
				{
					tinyPoint.x = m_blupiPos.x;
				tinyPoint.y = m_blupiPos.y;
					tinyPoint.y--;
					ObjectStart(tinyPoint, TYPE_SKATE, 0, TRUE);
				}
			}
			m_blupiHelico = FALSE;
			m_blupiOver = FALSE;
			m_blupiBalloon = FALSE;
			m_blupiEcrase = FALSE;
			m_blupiJeep = FALSE;
			m_blupiTank = FALSE;
			m_blupiSkate = FALSE;
			StopSound(SOUND_HELICOHIGH);
			StopSound(SOUND_HELICOLOW);
			StopSound(SOUND_JEEPHIGH);
			StopSound(SOUND_JEEPLOW);
		}
		PlaySound(SOUND_ELECTRO, m_blupiPos, FALSE);
	}

	if (m_blupiAction == ACTION_PUSH && m_blupiFocus)
	{
		num = CaisseInFront();
		if (num != -1)
		{
			end = m_moveObject[num].posCurrent;
			if (m_blupiDir == DIR_LEFT)
			{
				end.x = m_blupiPos.x - 59;
			}
			else
			{
				end.x = m_blupiPos.x + 55;
			}
			if (!TestPushCaisse(num, end, FALSE))
			{
				m_blupiPos.x = blupiPos.x;
			}
		}
		else
		{
			m_blupiAction = ACTION_STOP;
			m_blupiPhase = 0;
		}
	}
	if (m_blupiAction == ACTION_POP && m_blupiFocus)
	{
		num = CaisseInFront();
		if (num != -1)
		{
			end = m_moveObject[num].posCurrent;
			if (m_blupiDir == DIR_LEFT)
			{
				end.x = m_blupiPos.x - 59;
			}
			else
			{
				end.x = m_blupiPos.x + 55;
			}
			if (!TestPushCaisse(num, end, TRUE))
			{
				m_blupiAction = ACTION_STOP;
				m_blupiPhase = 0;
			}
		}
		else
		{
			m_blupiAction = ACTION_STOP;
			m_blupiPhase = 0;
		}
	}
	if (!m_blupiHelico && !m_blupiOver && !m_blupiBalloon && !m_blupiEcrase && !m_blupiJeep && !m_blupiTank && !m_blupiSkate &&
		!m_blupiNage &&
		!m_blupiSurf &&
		m_blupiFocus)
	{
		if (m_blupiActionOuf == ACTION_OUF1a && m_blupiAction == ACTION_STOP)
		{
			if (m_blupiTimeOuf > 50)
			{
				m_blupiAction = m_blupiActionOuf;
				m_blupiPhase = 0;
				PlaySound(SOUND_OUF1, m_blupiPos, FALSE);
			}
			m_blupiActionOuf = 0;
		}
		if (m_blupiActionOuf == ACTION_OUF1b && m_blupiAction == ACTION_STOP)
		{
			if (m_blupiTimeOuf > 50)
			{
				m_blupiAction = m_blupiActionOuf;
				m_blupiPhase = 0;
			}
			m_blupiActionOuf = 0;
		}
		if (m_blupiAction == ACTION_OUF1b && m_blupiPhase == ACTION_JUMP)
		{
			PlaySound(SOUND_OUF1, m_blupiPos, FALSE);
		}
		if (m_blupiActionOuf == ACTION_OUF5 && m_blupiAction == ACTION_STOP)
		{
			if (m_blupiTimeOuf > 10 && m_blupiTimeOuf < 50)
			{
				m_blupiAction = m_blupiActionOuf;
				m_blupiPhase = 0;
			}
			m_blupiActionOuf = 0;
		}
		if (m_blupiActionOuf == ACTION_OUF3 && m_blupiAction == ACTION_STOP)
		{
			if (m_blupiTimeOuf > 60)
			{
				m_blupiAction = m_blupiActionOuf;
				m_blupiPhase = 0;
				PlaySound(SOUND_OUF3, m_blupiPos, FALSE);
			}
			m_blupiActionOuf = 0;
		}
		if (m_blupiActionOuf == ACTION_OUF4 && m_blupiAction == ACTION_STOP)
		{
			if (m_blupiTimeOuf < 10)
			{
				m_blupiAction = m_blupiActionOuf;
				m_blupiPhase = 0;
				PlaySound(SOUND_OUF4, m_blupiPos, FALSE);
			}
			m_blupiActionOuf = 0;
		}
		if ((m_blupiActionOuf == ACTION_MOCKERY ||
			m_blupiActionOuf == ACTION_MOCKERYi ||
			m_blupiActionOuf == ACTION_MOCKERYp)
			&& m_blupiAction == ACTION_STOP)
		{
			if (m_blupiTimeOuf < 20)
			{
				m_blupiAction = m_blupiActionOuf;
				m_blupiPhase = 0;
			}
			m_blupiActionOuf = 0;
		}
	}
	if (m_blupiAction == ACTION_MARCH &&
		m_blupiActionOuf != ACTION_MOCKERY &&
		m_blupiActionOuf != ACTION_MOCKERYi &&
		m_blupiActionOuf != ACTION_MOCKERYp &&
		!m_blupiSurf &&
		!m_blupiNage)
	{
		m_blupiActionOuf = 0;
	}
	if (m_blupiActionOuf != 0)
	{
		m_blupiTimeOuf++;
	}
	if (m_blupiTimeMockery > 0)
	{
		m_blupiTimeMockery--;
	}
	if (m_blupiPos.x - 30 > m_blupiPosHelico.x || m_blupiPos.x + 30 < m_blupiPosHelico.x || m_blupiPos.y - 30 > m_blupiPosHelico.y || m_blupiPos.y + 30 < m_blupiPosHelico.y)
	{
		m_blupiPosHelico.x = -1;
	}
	if (m_blupiTimeFire > 0)
	{
		m_blupiTimeFire--;
	}
	if (m_blupiAction == ACTION_TELEPORTE && m_blupiPhase == 128)
	{
		POINT blupiPos2;
		if (SearchTeleporte(m_blupiPos, &blupiPos2))
		{
			m_blupiPos = blupiPos2;
			ObjectStart(m_blupiPos, TYPE_MAGICTRACK, 20, FALSE);
			ObjectStart(m_blupiPos, TYPE_MAGICTRACK, -20, FALSE);
		}
		m_blupiFocus = TRUE;
		m_blupiPosHelico = m_blupiPos;
	}
	if (m_blupiAction == ACTION_SWITCH && m_blupiPhase == 10)
	{
		m_blupiAction = ACTION_STOP;
		m_blupiPhase = 0;
		m_blupiFocus = TRUE;
	}
	if ((m_blupiAction == ACTION_CLEAR1 && m_blupiPhase == 70) ||
		(m_blupiAction == ACTION_CLEAR2 && m_blupiPhase == 100) ||
		(m_blupiAction == ACTION_CLEAR3 && m_blupiPhase == 70) ||
		(m_blupiAction == ACTION_CLEAR4 && m_blupiPhase == 110) ||
		(m_blupiAction == ACTION_CLEAR5 && m_blupiPhase == 90) ||
		(m_blupiAction == ACTION_CLEAR6 && m_blupiPhase == 90) ||
		(m_blupiAction == ACTION_CLEAR7 && m_blupiPhase == 90) ||
		(m_blupiAction == ACTION_CLEAR8 && m_blupiPhase == 90) ||
		(m_blupiAction == ACTION_DROWN && m_blupiPhase == 90) ||
		(m_blupiAction == ACTION_GLU && m_blupiPhase == 100) ||
		(m_blupiAction == 57 && m_blupiPhase == 90))
	{
		if (m_nbVies > 0)
		{

			m_blupiAction = ACTION_HIDE;
			m_blupiIcon = -1;
			m_blupiPhase = 0;
			if (m_blupiRestart || m_bMulti)
			{
				m_blupiPos = m_blupiValidPos;
			}
		blupiPos = GetPosDecor(m_blupiPos);
		tinyPoint.x = m_blupiPos.x - blupiPos.x;
		tinyPoint.y = m_blupiPos.y - blupiPos.y;
			VoyageInit(VoyageGetPosVie(m_nbVies), tinyPoint, 48, GetBlupiChannelStandard());
			NetSendBarePacket(PK_DIE, m_nbVies);
		}
		else
		{
			m_nbVies = -1;
			m_term = -1;
			DoorsLost();
			NetSendBarePacket(PK_LOST, 0);
		}
			m_blupiFront = FALSE;
	}
	if (m_bMulti && m_blupiAction != ACTION_WIN)
	{
		int count = 0;
		for (num = 0; num < MAXNETPLAYER; num++)
		{
			if (m_pNetwork->m_players[num].bIsPresent) count++;
		}
		if (count < 2)
		{

			StopSound(SOUND_HELICOHIGH);

			StopSound(SOUND_HELICOLOW);

			StopSound(SOUND_JEEPHIGH);

			StopSound(SOUND_JEEPLOW);
			PlaySound(14, m_moveObject[num].posCurrent, FALSE);
			m_blupiAction = ACTION_WIN;
			m_blupiPhase = 0;
			m_blupiFocus = FALSE;
			m_blupiFront = TRUE;
			m_blupiAir = FALSE;
			m_blupiHelico = FALSE;
			m_blupiOver = FALSE;
			m_blupiJeep = FALSE;
			m_blupiTank = FALSE;
			m_blupiSkate = FALSE;
			m_blupiNage = FALSE;
			m_blupiSurf = FALSE;
			m_blupiVent = FALSE;
			m_blupiSuspend = FALSE;
			m_blupiShield = FALSE;
			m_blupiPower = FALSE;
			m_blupiCloud = FALSE;
			m_blupiHide = FALSE;
			m_blupiInvert = FALSE;
			m_blupiBalloon = FALSE;
			m_blupiEcrase = FALSE;
			LoadString(287, str, 100);
			NotifPush(str);
		}
	}
	if (m_dimDecor.y == 0)
	{
		num3 = LYIMAGE;
	}
	else
	{
		num3 = DIMOBJY * MAXCELY;
	}
	if (m_blupiPos.y >= num3 + 1 && m_blupiPos.y <= num3 + 40)
	{
		PlaySound(SOUND_FALL, m_blupiPos, FALSE);
	}
	if (m_blupiPos.y > num3 + 1000)
	{
		m_term = -1;
		DoorsLost();
		NetSendBarePacket(PK_LOST, 0);
	}
	if (m_blupiAction == ACTION_WIN && m_blupiPhase == 70)
	{
		if (m_bPrivate)
		{
			m_term = 1;
		}
		else if (m_mission == 1)
		{
			m_term = 199;
		}
		else if (m_mission == 199)
		{
			m_term = -2;
		}
		else if (m_bFoundCle)
		{
			OpenGoldsWin();
			m_term = 1;
		}
		else
		{
			OpenDoorsWin();
			m_term = m_mission / 10 * 10;
		}
	}
	if (m_blupiAction == ACTION_BYE && m_blupiPhase == 50)
	{
		num = IsWorld(m_blupiPos);
		if (num != -1)
		{
			if (m_mission == 1)
			{
				m_term = num * 10;
			}
			else if (num == 199)
			{
				m_term = 1;
			}
			else
			{
				m_term = m_mission / 10 * 10 + num;
			}
		}
	}
	if (lastBlupiAction == ACTION_MOCKERY && m_blupiAction != ACTION_MOCKERY)
	{
		StopSound(SOUND_MOCKERY);
		NetStopSound(SOUND_MOCKERY);
	}
	if (lastBlupiAction == ACTION_MOCKERYi && m_blupiAction != ACTION_MOCKERYi)
	{
		StopSound(SOUND_MOCKERY);
		NetStopSound(SOUND_MOCKERY);
	}
	if (lastBlupiAction == ACTION_MOCKERYp && m_blupiAction != ACTION_MOCKERYp)
	{
		StopSound(SOUND_OUF2);
		NetStopSound(SOUND_OUF2);
	}
	if (m_blupiFocus &&
		!m_blupiAir &&
		(!m_blupiHelico || BlupiIsGround()) &&
		(!m_blupiOver || BlupiIsGround()) &&
		!m_blupiBalloon &&
		!m_blupiEcrase &&
		!m_blupiShield &&
		!bIsOpen &&
		!bIsOpen2 &&
		m_blupiTransport == -1 &&
		!IsLave(m_blupiPos) &&
		!IsPiege(m_blupiPos) &&
		!IsGoutte(m_blupiPos, TRUE) &&
		!IsScie(m_blupiPos) &&
		!IsSwitch(m_blupiPos, &tinyPoint) &&
		!IsBridge(m_blupiPos, &tinyPoint) &&
		IsTeleporte(m_blupiPos) == -1 &&
		!IsBlitz(m_blupiPos, TRUE) &&
		!IsTemp(m_blupiPos) &&
		!IsBalleTraj(m_blupiPos) &&
		!IsMoveTraj(m_blupiPos))
	{
		if (m_blupiFifoNb > 0)
		{
			m_blupiValidPos = m_blupiFifoPos[0];
		}
		BlupiAddFifo(m_blupiPos);
	}
	end.x = m_blupiPos.x + m_scrollAdd.x;
	end.y = m_blupiPos.y + m_scrollAdd.y;
	int num28 = abs(m_scrollPoint.x - end.x);
	int num29 = abs(m_scrollPoint.y - end.y);
	num4 = SCROLL_SPEED;
	if (num28 > SCROLL_MARGX * 2)
	{
		num4 += (num28 - SCROLL_MARGX * 2) / 4;
	}
	if (num29 > SCROLL_MARGY * 2)
	{
		num4 += (num29 - SCROLL_MARGY * 2) / 4;
	}
	if (m_scrollPoint.x < end.x)
	{
		m_scrollPoint.x = m_scrollPoint.x + num4;
		if (m_scrollPoint.x >= end.x)
		{
			m_scrollPoint.x = end.x;
		}
	}
	if (m_scrollPoint.x > end.x)
	{
		m_scrollPoint.x = m_scrollPoint.x - num4;
		if (m_scrollPoint.x <= end.x)
		{
			m_scrollPoint.x = end.x;
		}
	}
	if (m_scrollPoint.y < end.y)
	{
		m_scrollPoint.y = m_scrollPoint.y + num4;
		if (m_scrollPoint.y >= end.y)
		{
			m_scrollPoint.y = end.y;
		}
	}
	if (m_scrollPoint.y > end.y)
	{
		m_scrollPoint.y = m_scrollPoint.y - num4;
		if (m_scrollPoint.y <= end.y)
		{
			m_scrollPoint.y = end.y;
		}
	}
	if ((m_phase == WM_PHASE_PLAY || m_phase == WM_PHASE_PLAYTEST) &&
		m_blupiAction != ACTION_CLEAR2 && m_blupiAction != ACTION_CLEAR3)
	{
		m_posDecor = GetPosDecor(m_scrollPoint);
	}
	if (m_time % 4 == 0)
	{
		AdaptMotorVehicleSound(m_blupiPos);
	}
	VoyageStep();

	m_lastKeyPress = m_keyPress;

}

void CDecor::BlupiDead(int action1, int action2)
{
	if (action2 == -1)
	{
		m_blupiAction = action1;
	}
	else
	{
		m_blupiAction = ((rand() % 2 == 0) ? action1 : action2);
	}
	m_blupiPhase = 0;
	m_blupiFocus = FALSE;
	m_blupiHelico = FALSE;
	m_blupiOver = FALSE;
	m_blupiJeep = FALSE;
	m_blupiTank = FALSE;
	m_blupiSkate = FALSE;
	m_blupiNage = FALSE;
	m_blupiSurf = FALSE;
	m_blupiSuspend = FALSE;
	m_blupiJumpAie = FALSE;
	m_blupiShield = FALSE;
	m_blupiPower = FALSE;
	m_blupiCloud = FALSE;
	m_blupiHide = FALSE;
	m_blupiInvert = FALSE;
	m_blupiBalloon = FALSE;
	m_blupiEcrase = FALSE;
	m_blupiRestart = FALSE;
	m_blupiActionOuf = 0;
	m_jauges[JAUGE_AIR].SetHide(TRUE);
	m_jauges[JAUGE_POWER].SetHide(TRUE);
	StopSound(SOUND_HELICOHIGH);
	StopSound(SOUND_HELICOLOW);
	StopSound(SOUND_JEEPHIGH);
	StopSound(SOUND_JEEPLOW);

	POINT pos;
	POINT pos2;
	if (m_blupiAction == ACTION_CLEAR2)
	{
		pos.x = m_blupiPos.x - m_posDecor.x;
		pos.y = m_blupiPos.y - m_posDecor.y;
		pos2.x = m_blupiPos.x - m_posDecor.x;
		pos2.y = m_blupiPos.y - m_posDecor.y - 300;
		VoyageInit(pos, pos2, 230, CHELEMENT);
		PlaySound(SOUND_ANGEL, m_blupiPos, FALSE);
	}
	if (m_blupiAction == ACTION_CLEAR3)
	{
		pos.x = m_blupiPos.x - m_posDecor.x;
		pos.y = m_blupiPos.y - m_posDecor.y;
		pos2.x = m_blupiPos.x - m_posDecor.x;
		pos2.y = m_blupiPos.y - m_posDecor.y - 2000;
		VoyageInit(pos, pos2, 40, CHELEMENT);
		PlaySound(SOUND_ANGEL, m_blupiPos, FALSE);
	}
	if (m_blupiAction == ACTION_CLEAR4)
	{
		ObjectStart(m_blupiPos, TYPE_INVERTSTART, -70, FALSE);
		ObjectStart(m_blupiPos, TYPE_INVERTSTART, 20, FALSE);
		ObjectStart(m_blupiPos, TYPE_INVERTSTART, -20, FALSE);
		PlaySound(75, m_blupiPos, FALSE);
	}
}

POINT CDecor::GetPosDecor(POINT pos)
{
	POINT result;

	if (this->m_dimDecor.x == 0)
	{
		result.x = 0;
	}
	else
	{
		result.x = pos.x - LXIMAGE / 2;
		if (result.x < 0) result.x = 0;
		if (result.x > MAXCELX * DIMOBJX - LXIMAGE) result.x = MAXCELX * DIMOBJX - LXIMAGE;
	}

	if (this->m_dimDecor.y == 0)
	{
		result.y = 0;
		return result;
	}
	result.y = pos.y - LYIMAGE / 2;
	if (result.y < 0) result.y = 0;
	if (result.y > MAXCELY * DIMOBJY - LYIMAGE) result.y = MAXCELY * DIMOBJY - LYIMAGE;

	return result;
}

void CDecor::BlupiAddFifo(POINT pos)
{
	if (m_blupiFifoNb < 10)
	{
		if (m_blupiFifoNb > 0 && pos.x == m_blupiFifoPos[m_blupiFifoNb - 1].x && pos.y == m_blupiFifoPos[m_blupiFifoNb - 1].y)
		{
			return;
		}
		m_blupiFifoPos[m_blupiFifoNb] = pos;
		m_blupiFifoNb++;
		return;
	}
	else
	{
		if (pos.x == m_blupiFifoPos[9].x && pos.y == m_blupiFifoPos[9].y)
		{
			return;
		}
		for (int i = 0; i < 9; i++)
		{
			m_blupiFifoPos[i] = m_blupiFifoPos[i + 1];
		}
		m_blupiFifoPos[9] = pos;
		return;
	}
}

BOOL CDecor::DecorDetect(RECT rect, BOOL bCaisse)
{
	m_detectIcon = -1;
	if (rect.left < 0 || rect.top < 0)
	{
		return TRUE;
	}
	int num;
	if (m_dimDecor.x == 0)
	{
		num = 640;
	}
	else
	{
		num = 6400;
	}
	if (rect.right > num)
	{
		return TRUE;
	}
	if (m_blupiHelico ||
		m_blupiOver ||
		m_blupiBalloon ||
		m_blupiEcrase ||
		m_blupiNage ||
		m_blupiSurf)
	{
		if (m_dimDecor.y == 0)
		{
			num = 480;
		}
		else
		{
			num = 6400;
		}
		if (rect.bottom > num)
		{
			return TRUE;
		}
	}
	int num2 = rect.left / 16;
	int num3 = (rect.right + 16 - 1) / 16;
	int num4 = rect.top / 16;
	int num5 = (rect.bottom + 16 - 1) / 16;
	RECT src;
	for (int i = num4; i <= num5; i++)
	{
		for (int j = num2; j <= num3; j++)
		{
			int num6 = j / 4;
			int num7 = i / 4;
			if (num6 >= 0 && num6 < 100 && num7 >= 0 && num7 < 100)
			{
				int icon = m_decor[num6][num7].icon;
				if (icon >= 0 && icon < MAXQUART && (!m_blupiHelico || icon != 214) && (!m_blupiOver || icon != 214) && (icon != 324 || m_time / 4 % 20 < 18))
				{
					num6 = j % 4;
					num7 = i % 4;
					if (table_decor_quart[icon * 16 + num7 * 4 + num6] != 0)
					{
						src.left = j * 16;
						src.right = src.left + 16;
						src.top = i * 16;
						src.bottom = src.top + 16;
						RECT tinyRect;
						if (IntersectRect(&tinyRect, &src, &rect))
						{
							m_detectIcon = icon;
							return TRUE;
						}
					}

				}
			}
		}
	}
	if (!bCaisse)
	{
		return FALSE;
	}
	for (int k = 0; k < m_nbRankCaisse; k++)
	{
		int num8 = m_rankCaisse[k];
		src.left = m_moveObject[num8].posCurrent.x;
		src.right = m_moveObject[num8].posCurrent.x + 64;
		src.top = m_moveObject[num8].posCurrent.y;
		src.bottom = m_moveObject[num8].posCurrent.y + 64;
		RECT tinyRect;
		if (IntersectRect(&tinyRect, &src, &rect))
		{
			m_detectIcon = m_moveObject[num8].icon;
			return TRUE;
		}
	}
	return FALSE;
}


BOOL CDecor::TestPath(RECT rect, POINT start, POINT *end)
{
	int num = abs(end->x - start.x);
	int num2 = abs(end->y - start.y);

	POINT tinyPoint = start;
	if (num > num2)
	{
		RECT rect2;
		if (end->x > start.x)
		{
			for (int i = 0; i <= num; i++)
			{
				int j = i * (end->y - start.y) / num;
				rect2.left = rect.left + i;
				rect2.right = rect.right + i;
				rect2.top = rect.top + j;
				rect2.bottom = rect.bottom + j;
				if (DecorDetect(rect2))
				{
					*end = tinyPoint;
					return FALSE;
				}
				tinyPoint.x = start.x + i;
				tinyPoint.y = start.y + j;
			}
		}
		if (end->x < start.x)
		{
			for (int i = 0; i >= -num; i--)
			{
				int j = i * (start.y - end->y) / num;
				rect2.left = rect.left + i;
				rect2.right = rect.right + i;
				rect2.top = rect.top + j;
				rect2.bottom = rect.bottom + j;
				if (DecorDetect(rect2))
				{
					*end = tinyPoint;
					return FALSE;
				}
				tinyPoint.x = start.x + i;
				tinyPoint.y = start.y + j;
			}
		}
	}
	else
	{
		RECT rect2;
		if (end->y > start.y)
		{
			for (int j = 0; j <= num2; j++)
			{
				int i = j * (end->x - start.x) / num2;
				rect2.left = rect.left + i;
				rect2.right = rect.right + i;
				rect2.top = rect.top + j;
				rect2.bottom = rect.bottom + j;
				if (DecorDetect(rect2))
				{
					*end = tinyPoint;
					return FALSE;
				}
				tinyPoint.x = start.x + i;
				tinyPoint.y = start.y + j;
			}
		}
		if (end->y < start.y)
		{
			for (int j = 0; j >= -num2; j--)
			{
				int i = j * (start.x - end->x) / num2;
				rect2.left = rect.left + i;
				rect2.right = rect.right + i;
				rect2.top = rect.top + j;
				rect2.bottom = rect.bottom + j;
				if (DecorDetect(rect2))
				{
					*end = tinyPoint;
					return FALSE;
				}
				tinyPoint.x = start.x + i;
				tinyPoint.y = start.y + j;
			}
		}
	}
	return TRUE;
}

void CDecor::MoveObjectPollution()
{
	POINT pos = m_blupiPos;
	BOOL bPollution = FALSE;
	POINT offset = { 0, 0 };
	int speed = 20;

	if (m_blupiAction == ACTION_TURN) return;

	if (m_blupiHelico)
	{
		if (m_blupiVitesse.y < -5)
		{
			if (m_time % 20 != 0 &&
				m_time % 20 != 2 &&
				m_time % 20 != 5 &&
				m_time % 20 != 8 &&
				m_time % 20 != 10 &&
				m_time % 20 != 11 &&
				m_time % 20 != 16 &&
				m_time % 20 != 18)
			{
				return;
			}
		}
		else if (m_blupiVitesse.x == 0)
		{
			if (m_time % 50 != 0 &&
				m_time % 50 != 12 &&
				m_time % 50 != 30)
			{
				return;
			}
		}
		else if (m_time % 20 != 0 &&
			m_time % 20 != 3 &&
			m_time % 20 != 5 &&
			m_time % 20 != 11 &&
			m_time % 20 != 15)
		{
			return;
		}
		offset.x = 22;
		bPollution = TRUE;
	}
	if (m_blupiOver)
	{
		if (!(m_keyPress & (KEY_JUMP | KEY_UP)))
		{
			if (m_time % 50 != 0 && m_time % 50 != 12 && m_time % 50 != 30) return;
			speed = 20;
			offset.x = 30;
		}
		else
		{
			if (m_time % 20 != 0 && m_time % 20 != 2 && m_time % 20 != 5 &&
			    m_time % 20 != 8 && m_time % 20 != 11 && m_time % 20 != 13 &&
			    m_time % 20 != 14 && m_time % 20 != 18) return;
			speed = 58;
			offset.x = Random(-10, 10);
			offset.y = 22;
		}
		bPollution = TRUE;
	}
	if (m_blupiJeep)
	{
		if (m_blupiVitesse.x == 0)
		{
			if (m_blupiPhase % 50 != 0 &&
				m_blupiPhase % 50 != 12 &&
				m_blupiPhase % 50 != 20 &&
				m_blupiPhase % 50 != 35)
			{
				return;
			}
		}
		else if (m_blupiPhase % 20 != 0 &&
			m_blupiPhase % 20 != 3 &&
			m_blupiPhase % 20 != 5 &&
			m_blupiPhase % 20 != 11 &&
			m_blupiPhase % 20 != 15)
		{
			return;
		}
		offset.x = 32;
		bPollution = TRUE;
	}
	if (m_blupiTank)
	{
		if (m_blupiVitesse.x == 0)
		{
			if (m_blupiPhase % 50 != 0 &&
				m_blupiPhase % 50 != 15 &&
				m_blupiPhase % 50 != 28)
			{
				return;
			}
		}
		else if (m_blupiPhase % 20 != 0 &&
			m_blupiPhase % 20 != 4 &&
			m_blupiPhase % 20 != 12)
		{
			return;
		}
		offset.x = 35;
		bPollution = TRUE;
	}

	if (!bPollution) return;

	if (m_blupiDir == DIR_RIGHT)
	{
		pos.x -= offset.x - 5;
		if (speed < 50) speed = -speed;
	}
	else
	{
		pos.x += offset.x;
	}
	pos.y += offset.y;
	ObjectStart(pos, TYPE_POLLUTION, speed, TRUE);
}

void CDecor::MoveObjectPlouf(POINT pos)
{
	for (int i = 0; i < MAXMOVEOBJECT; i++)
	{
		if (m_moveObject[i].type == TYPE_PLOUF) return;
	}
	pos.y -= 45;
	PlaySound(SOUND_PLOUF, pos, FALSE);
	ObjectStart(pos, TYPE_PLOUF, 0, TRUE);
}

void CDecor::MoveObjectTiplouf(POINT pos)
{
	for (int i = 0; i < MAXMOVEOBJECT; i++)
	{
		if (m_moveObject[i].type == TYPE_TIPLOUF) return;
	}
	if (m_blupiDir == DIR_RIGHT)
	{
		pos.x += 5;
	}
	else
	{
		pos.x -= 5;
	}
	pos.y -= 45;
	PlaySound(SOUND_TIPLOUF, pos, FALSE);
	ObjectStart(pos, TYPE_TIPLOUF, 0, TRUE);
}

void CDecor::MoveObjectBlup(POINT pos)
{
	PlaySound(SOUND_BLUP, pos, FALSE);
	pos.y -= 20;
	int num = 0;
	POINT tinyPoint = pos;
	while (tinyPoint.y > 0)
	{
		int icon = m_decor[(tinyPoint.x + 16) / DIMOBJX][tinyPoint.y / DIMOBJY].icon;
		if (icon != 91 && icon != 92)
		{
			break;
		}
		num++;
		tinyPoint.y -= DIMOBJY;
	}
	num--;
	if (num <= 0)
	{
		return;
	}
	int num2 = MoveObjectFree();
	if (num2 == -1)
	{
		return;
	}
	m_moveObject[num2].type = TYPE_BLUP;
	m_moveObject[num2].phase = 0;
	m_moveObject[num2].posCurrent.x = pos.x;
	m_moveObject[num2].posCurrent.y = pos.y;
	m_moveObject[num2].posStart = m_moveObject[num2].posCurrent;
	m_moveObject[num2].posEnd.x = pos.x;
	m_moveObject[num2].posEnd.y = pos.y - num * DIMOBJY;
	m_moveObject[num2].timeStopStart = 0;
	m_moveObject[num2].stepAdvance = num * 10;
	m_moveObject[num2].step = STEP_ADVANCE;
	m_moveObject[num2].time = 0;
	MoveObjectStepIcon(num2);
}
int CDecor::IsWorld(POINT pos)
{
	pos.x += 30;
	pos.y += 30;
	if (pos.x < 0 || pos.x >= 6400 || pos.y < 0 || pos.y >= 6400) return -1;
	int icon = m_decor[pos.x / DIMOBJX][pos.y / DIMOBJY].icon;
	if (icon >= 158 && icon <= 165) return icon - 157;
	if (icon >= 166 && icon <= 173) return icon - 165;
	if (icon == 309 || icon == 310) return 9;
	if (icon >= 411 && icon <= 415) return icon - 401;
	if (icon >= 416 && icon <= 420) return icon - 406;
	if (icon >= 174 && icon <= 181) return icon - 173;
	if (icon == 184) return 199;
	return -1;
}

void CDecor::ActiveSwitch(BOOL bState, POINT cel)
{
	POINT pos;
	pos.x = cel.x * DIMOBJX;
	pos.y = cel.y * DIMOBJY;
	ModifDecor(pos, bState ? 384 : 385);
	PlaySound(bState ? 77 : 76, pos, FALSE);
	cel.x -= 20;
	for (int i = 0; i < 41; i++)
	{
		if (cel.x >= 0 && cel.x < 100 && m_decor[cel.x][cel.y].icon == (bState ? 379 : 378))
		{
			pos.x = cel.x * DIMOBJX;
			pos.y = cel.y * DIMOBJY;
			ModifDecor(pos, bState ? 378 : 379);
		}
		cel.x++;
	}
}

int CDecor::GetTypeBarre(POINT pos)
{
	POINT pos2 = pos;
	pos.x += 30;
	pos.y += 22;
	if (pos.y % 64 > 44)
	{
		return 0;
	}
	if (pos.x < 0 || pos.x >= 6400 || pos.y < 0 || pos.y >= 6400)
	{
		return 0;
	}
	int icon = m_decor[pos.x / DIMOBJX][pos.y / DIMOBJY].icon;
	if (icon != 138 && icon != 202)
	{
		return 0;
	}
	if (pos.y >= 6336)
	{
		return 1;
	}
	icon = m_decor[pos.x / DIMOBJX][pos.y / DIMOBJY + 1].icon;
	if (!IsPassIcon(icon))
	{
		return 2;
	}
	RECT rect;
	rect = BlupiRect(pos2);
	rect.top = pos2.y + 60 - 2;
	rect.bottom = pos2.y + 60 - 1;
	if (DecorDetect(rect, TRUE))
	{
		return 2;
	}
	return 1;
}

BOOL CDecor::IsLave(POINT pos)
{
	pos.x += DIMBLUPIX / 2;
	if (pos.x < 0 || pos.x >= 6400 || pos.y < 0 || pos.y >= 6400)
	{
		return false;
	}
	return m_decor[pos.x / DIMOBJX][pos.y / DIMOBJY].icon == 68;
}

BOOL CDecor::IsPiege(POINT pos)
{
	pos.x += DIMBLUPIX / 2;
	pos.y += DIMBLUPIY;
	if (pos.x % 64 < 15 || pos.x % 64 > 49 || pos.x < 0 || pos.x >= 6400 || pos.y < 0 || pos.y >= 6400) return FALSE;
	return m_decor[pos.x / DIMOBJX][pos.y / DIMOBJY].icon == 373;
}

BOOL CDecor::IsGoutte(POINT pos, BOOL bAlways)
{
	pos.x += DIMBLUPIX / 2;
	if (pos.x % 64 < 15 || pos.x % 64 > 49)
	{
		return FALSE;
	}
	if (pos.x < 0 || pos.x >= 6400 || pos.y < 0 || pos.y >= 6400)
	{
		return FALSE;
	}
	int icon = m_decor[pos.x / DIMOBJX][pos.y / DIMOBJY].icon;
	if (bAlways)
	{
		return icon == 404 || icon == 410;
	}
	return icon == 404;
}

BOOL CDecor::IsScie(POINT pos)
{
	pos.x += DIMBLUPIX / 2;
	if (pos.x % 64 < 4 || pos.x % 64 > 60 || pos.x < 0 || pos.x >= 6400 || pos.y < 0 || pos.y >= 6400) return FALSE;
	return m_decor[pos.x / DIMOBJX][pos.y / DIMOBJY].icon == 378;
}

BOOL CDecor::IsSwitch(POINT pos, POINT *outCelSwitch)
{
	if (pos.x % DIMOBJX > 6)
	{
		return FALSE;
	}
	pos.x += DIMBLUPIX / 2;
	if (pos.x < 0 || pos.x >= DIMOBJX * MAXCELX || pos.y < 0 || pos.y >= DIMOBJY * MAXCELY)
	{
		return FALSE;
	}
	outCelSwitch->x = pos.x / DIMOBJX;
	outCelSwitch->y = pos.y / DIMOBJY;
	return m_decor[pos.x / DIMOBJX][pos.y / DIMOBJY].icon == 384 || m_decor[pos.x / DIMOBJX][pos.y / DIMOBJY].icon == 385;
}

BOOL CDecor::IsEcraseur(POINT pos)
{
	if (m_time / 3 % 10 > 2)
	{
		return FALSE;
	}
	pos.x += 30;
	if (pos.x < 0 || pos.x >= 6400 || pos.y < 0 || pos.y >= 6400) return FALSE;
	return m_decor[pos.x / DIMOBJX][pos.y / DIMOBJY].icon == 317;
}

BOOL CDecor::IsBlitz(POINT pos, BOOL bAlways)
{
	pos.x += 30;
	if (pos.x < 0 || pos.x >= 6400 || pos.y < 0 || pos.y >= 6400)
	{
		return FALSE;
	}
	POINT tinyPoint;
	tinyPoint.x = pos.x / DIMOBJX;
	tinyPoint.y = pos.y / DIMOBJY;
	return m_decor[tinyPoint.x][tinyPoint.y].icon == 305 && (bAlways || BlitzActif(tinyPoint));
}

BOOL CDecor::IsRessort(POINT pos)
{
	pos.x += 30;
	pos.y += 60;
	if (pos.x < 0 || pos.x >= 6400 || pos.y < 0 || pos.y >= 6400)
	{
		return FALSE;
	}
	return m_decor[pos.x / DIMOBJX][pos.y / DIMOBJY].icon == 211;
}

BOOL CDecor::IsTemp(POINT pos)
{
	pos.x += 30;
	pos.y += 60;
	if (pos.x < 0 || pos.x >= 6400 || pos.y < 0 || pos.y >= 6400)
	{
		return FALSE;
	}
	return m_decor[pos.x / DIMOBJX][pos.y / DIMOBJY].icon == 324;
}

BOOL CDecor::IsBridge(POINT pos, POINT *outCelBridge)
{
	pos.x += 30;
	pos.y += 60;
	if (pos.x >= 0 && pos.x < 6400 && pos.y >= 0 && pos.y < 6400 && m_decor[pos.x / DIMOBJX][pos.y / DIMOBJY].icon == 364)
	{
		outCelBridge->x = pos.x / DIMOBJX;
		outCelBridge->y = pos.y / DIMOBJY;
		return TRUE;
	}
	pos.y -= 60;
	if (pos.x >= 0 && pos.x < 6400 && pos.y >= 0 && pos.y < 6400 && m_decor[pos.x / DIMOBJX][pos.y / DIMOBJY].icon == 364)
	{
		outCelBridge->x = pos.x / DIMOBJX;
		outCelBridge->y = pos.y / DIMOBJY;
		return TRUE;
	}
	return FALSE;

}

int CDecor::IsDoor(POINT pos, POINT *outCelPorte)
{
	int num;
	if (m_blupiDir == 1)
	{
		num = -60;
	}
	else
	{
		num = 60;
	}
	pos.x += 30;
	for (int i = 0; i < 2; i++)
	{
		if (pos.x >= 0 && pos.x < DIMOBJX * MAXCELX && pos.y >= 0 && pos.y < DIMOBJY * MAXCELY && m_decor[pos.x / DIMOBJX][pos.y / DIMOBJY].icon >= 334 && m_decor[pos.x / DIMOBJX][pos.y / DIMOBJY].icon <= 336)
		{
			outCelPorte->x = pos.x / DIMOBJX;
			outCelPorte->y = pos.y / DIMOBJY;
			return m_decor[pos.x / DIMOBJX][pos.y / DIMOBJY].icon;
		}
		pos.x += num;
	}
	return -1;
}

int CDecor::IsTeleporte(POINT pos)
{
	if (pos.x % DIMOBJX > 6)
	{
		return -1;
	}
	pos.x += 30;
	pos.y -= 60;
	if (pos.x < 0 || pos.x >= DIMOBJX * MAXCELX || pos.y < 0 || pos.y >= DIMOBJY * MAXCELY)
	{
		return -1;
	}
	if (m_decor[pos.x / DIMOBJX][pos.y / DIMOBJY].icon >= 330 && m_decor[pos.x / DIMOBJX][pos.y / DIMOBJY].icon <= 333)
	{
		return m_decor[pos.x / DIMOBJX][pos.y / DIMOBJY].icon;
	}
	return -1;
}

BOOL CDecor::SearchTeleporte(POINT pos, POINT *newpos)
{
	int num = IsTeleporte(pos);
	if (num == -1)
	{
		return FALSE;
	}
	for (int i = 0; i < 100; i++)
	{
		for (int j = 0; j < 100; j++)
		{
			if (num == m_decor[i][j].icon)
			{
				newpos->x = i * 64;
				newpos->y = j * 64 + 60;
				if (newpos->x < pos.x - 40 || newpos->x > pos.x + 40 || newpos->y < pos.y - 40 || newpos->y > pos.y + 40)
				{
					return TRUE;
				}
			}
		}
	}
	return FALSE;
}

BOOL CDecor::IsSurfWater(POINT pos)
{
	if (pos.y % DIMOBJY >= DIMOBJY - BLUPISURF)
	{
		int x = (pos.x + DIMBLUPIX / 2) / DIMOBJX;
		int icon2 = m_decor[x][(pos.y + BLUPISURF) / DIMOBJY].icon;
		if (m_decor[x][pos.y / DIMOBJY].icon != 92 && icon2 == 92)
		{
			return TRUE;
		}
	}
	return FALSE;
}

BOOL CDecor::IsDeepWater(POINT pos)
{
	POINT cel;
	cel.x = (pos.x + 30) / DIMOBJX;
	cel.y = pos.y / DIMOBJY;
	int icon = m_decor[cel.x][cel.y].icon;
	return icon == 91 || icon == 92;
}

BOOL CDecor::IsOutWater(POINT pos)
{
	int icon = m_decor[(pos.x + DIMBLUPIX / 2) / DIMOBJX][(pos.y + DIMBLUPIY / 2) / DIMOBJY].icon;
	if (icon == 91 || icon == 92) return FALSE;
	return IsPassIcon(icon);
}

BOOL CDecor::IsPassIcon(int icon)
{
	if (icon == 324 && m_time / 4 % 20 >= 18)
	{
		return TRUE;
	}
	if (icon >= 0 && icon < MAXQUART)
	{
		for (int i = 0; i < 16; i++)
		{
			if (table_decor_quart[icon * 16 + i] != 0)
			{
				return FALSE;
			}
		}
	}
	return TRUE;
}

BOOL CDecor::IsBlocIcon(int icon)
{
	if (icon < 0 || icon >= MAXQUART)
	{
		return FALSE;
	}
	if (icon == 324 && m_time / 4 % 20 < 18)
	{
		return TRUE;
	}
	for (int i = 0; i < 16; i++)
	{
		if (table_decor_quart[icon * 16 + i] == 0)
		{
			return FALSE;
		}
	}
	return TRUE;
}

void CDecor::FlushBalleTraj()
{
	for (int i = 0; i < sizeof(m_balleTraj); i++) m_balleTraj[i] = 0;
}

void CDecor::SetBalleTraj(POINT cel)
{
	if (!IsValidCel(cel)) return;
	int num = cel.x / 8 + cel.y * (TRAJBYTEX);
	m_balleTraj[num] |= 1 << (cel.x & 7);
}

BOOL CDecor::IsBalleTraj(POINT pos)
{
	POINT cel;
	cel.x = (pos.x + DIMOBJX / 2) / DIMOBJX;
	cel.y = (pos.y + DIMOBJY / 2) / DIMOBJY;
	if (IsValidCel(cel))
	{
		return m_balleTraj[cel.y * (TRAJBYTEX) + cel.x / 8] & (1 << (cel.x & 7));
	}
	return FALSE;
}

void CDecor::FlushMoveTraj()
{
	for (int i = 0; i < sizeof(m_moveTraj); i++) m_moveTraj[i] = 0;
}

void CDecor::SetMoveTraj(POINT cel)
{
	if (!IsValidCel(cel)) return;
	int num = cel.y * (TRAJBYTEX);
	num += cel.x / 8;
	int num2 = cel.x & 7;
	m_moveTraj[num] |= 1 << num2;
}

BOOL CDecor::IsMoveTraj(POINT pos)
{
	POINT cel;
	cel.x = (pos.x + DIMOBJX / 2) / DIMOBJX;
	cel.y = (pos.y + DIMOBJY / 2) / DIMOBJY;
	if (IsValidCel(cel))
	{
		int num = cel.y * (TRAJBYTEX);
		num += cel.x / 8;
		int num2 = cel.x & 7;
		return m_moveTraj[num] & (1 << num2);
	}
	return FALSE;
}

int CDecor::SearchDistRight(POINT pos, POINT dir, int type)
{
	POINT cel;
	int num = 0;
	if (type == TYPE_POLLUTION ||
		type == TYPE_TRESORTRACK ||
		type == TYPE_INVERTSTART ||
		type == TYPE_INVERTSTOP ||
		type == TYPE_INVERTSPIN ||
		type == TYPE_EXPLO8)
	{
		return 500;
	}
	cel.x = (pos.x + DIMOBJX / 2) / DIMOBJX;
	cel.y = (pos.y + DIMOBJY / 2) / DIMOBJY;
	while (IsValidCel(cel) && !IsBlocIcon(m_decor[cel.x][cel.y].icon))
	{
		if (type == TYPE_BALLE)
		{
			SetBalleTraj(cel);
		}
		num += DIMOBJX;
		cel.x += dir.x;
		cel.y += dir.y;
	}
	if ((type == TYPE_GLU || type == TYPE_ELECTRO) && num >= DIMOBJX)
	{
		num -= DIMOBJX;
	}
	if (type == TYPE_BALLE && num >= 10)
	{
		num -= 10;
	}
	return num;
}

BOOL CDecor::IsVentillo(POINT pos)
{
	int num;
	BOOL flag = FALSE;
	POINT tinyPoint;
	pos.x += 30;
	pos.y += 30;
	if (pos.x < 0 || pos.x >= 6400 || pos.y < 0 || pos.y >= 6400)
	{
		return FALSE;
	}
	int icon = m_decor[pos.x / DIMOBJX][pos.y / DIMOBJY].icon;
	if (icon < 126 || icon > 137)
	{
		return FALSE;
	}
	if (icon == 126)
	{
		if (pos.x % 64 <= 16)
		{
			flag = TRUE;
		}
		tinyPoint.x = -64;
		tinyPoint.y = 0;
		num = 110;
	}
	if (icon == 129)
	{
		if (pos.x % 64 >= 48)
		{
			flag = TRUE;
		}
		tinyPoint.x = 64;
		tinyPoint.y = 0;
		num = 114;
	}
	if (icon == 132)
	{
		if (pos.y % 64 <= 32)
		{
			flag = TRUE;;
		}
		tinyPoint.x = 0;
		tinyPoint.y = -64;
		num = 118;
	}
	if (icon == 135)
	{
		if (pos.y % 64 >= 48)
		{
			flag = TRUE;
		}
		tinyPoint.x = 0;
		tinyPoint.y = 64;
		num = 122;
	}
	if (!flag)
	{
		return FALSE;
	}
	ModifDecor(pos, -1);
	while (TRUE)
	{
		pos.x += tinyPoint.x;
		pos.y += tinyPoint.y;
		if (num != m_decor[pos.x / DIMOBJX][pos.y / DIMOBJY].icon)
		{
			break;
		}
		ModifDecor(pos, -1);
		if (pos.x < 0 || pos.x >= 6400 || pos.y < 0 || pos.y >= 6400) break;
	}
	return TRUE;
}

