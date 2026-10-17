// DecBlupi.cpp
//

#include <misc.h>
#include <decor.h>
#include <text.h> // debug
//#include "resource.h"
#include <obstacle.h>
#include <dectables.h>


// get Blupi's current graphics channel
// (changes for certain actions such as burned or glued)
int CDecor::GetBlupiChannelActual()
{
	if (m_bMulti && m_blupiChannel == CHBLUPI && m_team > 0) {
		return m_team + CHBLUPI1 - 1;
	}
	else return m_blupiChannel;
}

// get the graphics channel corresponding to Blupi's team
// (for Personal bomb, etc)
int CDecor::GetBlupiChannelStandard()
{
	if (m_bMulti && m_team > 0) {
		return m_team + CHBLUPI1 - 1;
	}
	else return CHBLUPI;
}

// get various flags relating to Blupi's current state
void CDecor::GetBlupiInfo(BOOL* bHelico, BOOL* bJeep, BOOL* bSkate, BOOL* bNage)
{
	*bHelico = m_blupiHelico;
	*bJeep = m_blupiJeep | m_blupiTank;
	*bSkate = m_blupiSkate;
	*bNage = m_blupiNage | m_blupiSurf;
}

// set blupi's current graphics icon according to his state
// from ghidra
void CDecor::BlupiSearchIcon()

{
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
		if (action == ACTION_STOP) action = ACTION_STOPJEEP;
		if (action == ACTION_MARCH) action = ACTION_MARCHJEEP;
		if (action == ACTION_ADVANCE) action = ACTION_MARCHJEEP;
		if (action == ACTION_TURN) action = ACTION_TURNJEEP;
	}

	if (m_blupiTank)
	{
		if (action == ACTION_STOP) action = ACTION_STOPTANK;
		if (action == ACTION_MARCH) action = ACTION_MARCHTANK;
		if (action == ACTION_ADVANCE) action = ACTION_MARCHTANK;
		if (action == ACTION_TURN) action = ACTION_TURNTANK;
	}

	if (m_blupiSkate)
	{
		if (action == ACTION_STOP) action = ACTION_STOPSKATE;
		if (action == ACTION_MARCH) action = ACTION_MARCHSKATE;
		if (action == ACTION_ADVANCE) action = ACTION_MARCHSKATE;
		if (action == ACTION_TURN) action = ACTION_TURNSKATE;
		if (action == ACTION_TURNAIR) action = ACTION_TURNSKATE;
		if (action == ACTION_JUMP) action = ACTION_JUMPSKATE;
		if (action == ACTION_AIR) action = ACTION_AIRSKATE;
	}

	if (m_blupiNage)
	{
		if (action == ACTION_STOP) action = ACTION_STOPNAGE;
		if (action == ACTION_MARCH) action = ACTION_MARCHNAGE;
		if (action == ACTION_TURN) action = ACTION_TURNNAGE;
		if (m_blupiVitesse.x == 0)
		{
			if (action == ACTION_STOPNAGE) action = ACTION_STOPSURF;
			if (action == ACTION_MARCHNAGE) action = ACTION_MARCHSURF;
		}
	}

	if (m_blupiSurf)
	{
		if (action == ACTION_STOP) action = ACTION_STOPSURF;
		if (action == ACTION_MARCH) action = ACTION_MARCHSURF;
		if (action == ACTION_TURN) action = ACTION_TURNSURF;
	}

	if (m_blupiSuspend)
	{
		if (action == ACTION_STOP) action = ACTION_STOPSUSPEND;
		if (action == ACTION_MARCH) action = ACTION_MARCHSUSPEND;
		if (action == ACTION_TURN) action = ACTION_TURNSUSPEND;
		if (action == ACTION_JUMP) action = ACTION_JUMPSUSPEND;
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
	for (int i = 0; table_blupi[i] != 0; i += table_blupi[i + 1] + 3)
	{
		if (action == table_blupi[i])
		{
			phase = table_blupi[i + 2];
			if (phase == 0 || m_blupiPhase <= phase)
			{
				phase = m_blupiPhase % table_blupi[i + 1];
			}
			icon = table_blupi[i + 3 + phase];
			break;
		}
	}
	if ((action == ACTION_CLEAR1 ||
		action == ACTION_CLEAR2 ||
		action == ACTION_CLEAR3 ||
		action == ACTION_GLU ||
		action == ACTION_ELECTRO) && icon < 0x10a)
	{
		m_blupiChannel = CHELEMENT;
	}
	else
	{
		m_blupiChannel = CHBLUPI;
	}
	if (m_blupiDir == DIR_LEFT || (m_blupiDir == DIR_RIGHT && m_blupiInvert))
	{
		if (m_blupiChannel == CHBLUPI)
		{
			if (action == ACTION_STOPSUSPEND)
			{
				if (icon == 0x90) icon = 0x9e;
				if (icon == 0x8f) icon = 0x91;
				if (icon == 0x97) icon = 0x92;
			}
			if (icon > -1 && icon < 0x14f)
			{
				icon = table_mirror[icon];
			}
		}
		if (true && m_blupiChannel == CHELEMENT && icon > 0xa7 && icon < 0xac)
		{
			icon = (icon + 4);
		}
	}
	m_blupiIcon = icon;
	m_blupiPhase++;
}

BOOL CDecor::BlupiIsGround()
{
	if (m_blupiTransport == -1)
	{
		RECT rect = BlupiRect(m_blupiPos);
		rect.top = m_blupiPos.y + DIMBLUPIY - 2;
		rect.bottom = m_blupiPos.y + DIMBLUPIY - 1;
		return DecorDetect(rect, TRUE);
	}
	else {
		return FALSE;
	}
}

// get Blupi's hitbox according to his state
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

// prevent blupi from getting stuck in blocks
void CDecor::BlupiAdjust()
{
	int i;
	RECT tinyRect = BlupiRect(m_blupiPos);

	if (!DecorDetect(tinyRect))
	{
		return;
	}
	for (i = 0; i < 50; i++)
	{
		RECT rect = tinyRect;
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
		RECT rect = tinyRect;
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
		RECT rect = tinyRect;
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
		RECT rect = tinyRect;
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
		RECT rect = tinyRect;
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
	RECT rect = BlupiRect(pos);
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

// handle Blupi's movement and current state.
void CDecor::BlupiStep()
{
	BOOL bNear;
	int icon;
	POINT tinyPoint;
	POINT tinyPoint2 = m_blupiPos; // = m_blupiPos ?
	BlupiAdjust();
	//m_blupiLastPos = m_blupiPos;
	POINT end = m_blupiPos;
	int lastBlupiAction = m_blupiAction;
	BOOL bLastBlupiAir = m_blupiAir;
	BOOL bAirStart = FALSE;
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
		num = m_decor[(end.x + DIMBLUPIX / 2) / DIMOBJX][(end.y + DIMBLUPIY / 2) / DIMOBJY].icon;
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
	m_blupiVent = FALSE;
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
	int detectIcon = m_detectIcon;
	if (!m_blupiAir &&
		!IsBlupiMotorVehicle() &&
		!m_blupiBalloon &&
		!m_blupiEcrase &&
		!m_blupiNage &&
		!m_blupiSurf &&
		!m_blupiSuspend &&
		m_blupiFocus &&
		bAirStart)
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
		if (IsBlupiVehicle() &&
			!m_blupiShield &&
			!m_blupiHide &&
			!m_bSuperBlupi)
		{
			m_blupiHelico = FALSE;
			m_blupiOver = FALSE;
			m_blupiJeep = FALSE;
			m_blupiTank = FALSE;
			m_blupiSkate = FALSE;
			tinyPoint.x = end.x - 34;
			tinyPoint.y = end.y - 34;
			ObjectStart(tinyPoint, TYPE_EXPLO2, 0);
			m_decorAction = 1;
			m_decorPhase = 0;
			StopVehicleSound();
			PlaySound(SOUND_BOUM, m_blupiPos);
		}
		if (m_blupiFocus && !IsDeadAction(m_blupiAction))
		{
			m_blupiAction = ACTION_AIR;
			m_blupiPhase = 0;
		}
		if (m_keyPress & KEY_JUMP && m_blupiFocus)
		{
			if (m_keyPress & KEY_UP)
			{
				m_blupiVitesse.y = (m_blupiPower ? -29 : -25);
			}
			else if (m_keyPress & KEY_DOWN)
			{
				m_blupiVitesse.y = (m_blupiPower ? -19 : -16);
			}
			else
			{
				m_blupiVitesse.y = (m_blupiPower ? -25 : -19);
			}
		}
		else
		{
			m_blupiVitesse.y = (m_blupiPower ? -16 : -10);
		}
		m_blupiAir = TRUE;
		bLastBlupiAir = TRUE;
		PlaySound(SOUND_RESSORT, end);
	}
	if (m_keyPress & KEY_JUMP &&
		!IsBlupiMotorVehicle() &&
		!m_blupiBalloon &&
		!m_blupiEcrase &&
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
				PlaySound(SOUND_JUMP1, end);
				m_blupiVitesse.y = (m_blupiPower ? -22 : -16);
			}
			else
			{	
				if (m_keyPress & KEY_UP)
				{
					PlaySound(SOUND_JUMP2, end);
					m_blupiVitesse.y = (m_blupiPower ? -26 : -22);
				}
				else if (m_keyPress & KEY_DOWN)
				{
					PlaySound(SOUND_JUMP0, end);
					m_blupiVitesse.y = (m_blupiPower ? -16 : -12);
				}
				else
				{
					PlaySound(SOUND_JUMP1, end);
					m_blupiVitesse.y = (m_blupiPower ? -22 : -16);
				}
			}
			m_blupiAir = TRUE;
			bLastBlupiAir = TRUE;
		}
	}
	if (m_blupiAir)
	{
		if (DecorDetect(rect) && m_blupiVitesse.y < 0)
		{
			if (m_blupiVitesse.y < -14 && !IsDeadAction(m_blupiAction) && !m_blupiSkate)
			{
				m_blupiJumpAie = TRUE;
				PlaySound(SOUND_JUMPAIE, end);
			}
			else
			{
				PlaySound(SoundEnviron(SOUND_JUMPTOC, detectIcon), end);
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
				PlaySound(SoundEnviron(SOUND_JUMPEND, m_detectIcon), end);
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
		rect.top = end.y + DIMBLUPIY - 33;
		rect.bottom = end.y + DIMBLUPIY - 1;
		icon = AscenseurDetect(rect, m_blupiPos, end);
		if (m_blupiVitesse.y >= 0 && icon != -1)
		{
			m_blupiTransport = icon;
			bAirStart = FALSE;
			PlaySound(SOUND_JUMPEND, end, FALSE);
			end.y = m_moveObject[icon].posCurrent.y - DIMOBJY + BLUPIOFFY;
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

	switch (m_blupiAction)
	{
	case ACTION_JUMPAIE:
		if (m_blupiPhase == 30) StopBlupi(TRUE);
		break;
	case ACTION_CHARGE:
		if (m_blupiPhase == 64)
		{
			StopBlupi(TRUE);
			m_blupiCloud = TRUE;
			m_blupiTimeShield = 100;
			m_jauges[JAUGE_POWER].SetHide(FALSE);
			PlaySound(ACTION_DRINK, end);
		}
		break;
	case ACTION_HELICOGLU:
		if (m_blupiPhase == 8)
		{
			tinyPoint.x = m_blupiPos.x;
			tinyPoint.y = m_blupiPos.y + 40;
			if (m_blupiVitesse.y > 0)
			{
				tinyPoint.y += m_blupiVitesse.y * 4;
			}
			m_blupiVitesse.y -= 10;
			if (ObjectStart(tinyPoint, TYPE_BALLE, 55) != -1)
			{
				PlaySound(SOUND_FIREOK, m_blupiPos);
				m_blupiTimeFire = 10;
				m_blupiBullet--;
			}
		}
		if (m_blupiPhase == 14) StopBlupi(TRUE);
		break;
	case ACTION_OUF1a:
	case ACTION_OUF1b:
		if (m_blupiPhase == 29) StopBlupi(TRUE);
		break;
	case ACTION_OUF2:
		if (m_blupiPhase == 32) StopBlupi(TRUE);
		break;
	case ACTION_OUF3:
		if (m_blupiPhase == 34) StopBlupi(TRUE);
		break;
	case ACTION_OUF4:
		if (m_blupiPhase == 40)
		{
			StopBlupi(TRUE);
			icon = MoveObjectDetect(end, &bNear);
			if (icon != -1 && !bNear && end.y - BLUPIFLOOR == m_moveObject[num].posCurrent.y)
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
		break;
	case ACTION_OUF5:
		if (m_blupiPhase == 4) PlaySound(SOUND_OUF2, m_blupiPos);
		if (m_blupiPhase == 44) StopBlupi(TRUE);
		break;
	case ACTION_MOCKERY:
		if (m_blupiPhase == 1)
		{
			PlaySound(SOUND_MOCKERY, m_blupiPos);
			m_blupiTimeMockery = 300;
		}
		if (m_blupiPhase == 92) StopBlupi(TRUE);
		break;
	case ACTION_MOCKERYi:
		if (m_blupiPhase == 6)
		{
			PlaySound(SOUND_MOCKERY, m_blupiPos);
			m_blupiTimeMockery = 300;
		}
		if (m_blupiPhase == 104) StopBlupi(TRUE);
		break;
	case ACTION_MOCKERYp:
		if (m_blupiPhase == 4) PlaySound(SOUND_OUF2, m_blupiPos);
		if (m_blupiPhase == 60) StopBlupi(TRUE);
		break;
	case ACTION_NON:
		if (m_blupiPhase == 18) StopBlupi(TRUE);
		break;
	case ACTION_STOPMARCH:
		if (m_blupiPhase == 3) StopBlupi();
		break;
	case ACTION_STOPJUMP:
		if (m_blupiPhase == 5) StopBlupi();
		break;
	case ACTION_STOPJUMPh:
		if (m_blupiPhase == 2)
		{
			m_blupiAction = ACTION_AIR;
			m_blupiPhase = 0;
			m_blupiVitesse.y = -12;
			m_blupiAir = TRUE;
			bLastBlupiAir = TRUE;
		}
	case ACTION_SUCETTE:
		if (m_blupiPhase == 32)
		{
			//ObjectStart(m_sucettePos, m_sucetteType, 0); // respawn item (winphone behavior)
			StopBlupi(TRUE);
			m_blupiPower = TRUE;
			m_blupiTimeShield = 100;
			m_blupiPosMagic = m_blupiPos;
			m_jauges[JAUGE_POWER].SetHide(FALSE);
			PlaySound(SOUND_STARTPOWER, end);
		}
	case ACTION_DRINK:
		if (m_blupiPhase == 36)
		{
			//ObjectStart(m_sucettePos, m_sucetteType, 0); // respawn item (winphone behavior)
			StopBlupi(TRUE);
			m_blupiHide = TRUE;
			m_blupiTimeShield = 100;
			m_blupiPosMagic = m_blupiPos;
			m_jauges[JAUGE_POWER].SetHide(FALSE);
			PlaySound(SOUND_STARTHIDE, end);
		}
	}

	if (m_keyPress & KEY_UP &&
		!(m_lastKeyPress & KEY_UP) &&
		IsBlupiStanding() &&
		m_blupiAction != ACTION_UP &&
		m_blupiFocus)
	{
		m_blupiAction = ACTION_UP;
		m_blupiPhase = 0;
	}

	if (!(m_keyPress & (KEY_UP | KEY_DOWN)) &&
		m_lastKeyPress & KEY_UP &&
		IsBlupiStanding() &&
		m_blupiFocus)
	{
		StopBlupi();
	}

	if (m_keyPress & KEY_DOWN &&
		!(m_lastKeyPress & KEY_DOWN) &&
		IsBlupiStanding() &&
		m_blupiAction != ACTION_DOWN &&
		m_blupiAction != ACTION_STOPPOP &&
		m_blupiFocus)
	{
		m_blupiAction = ACTION_DOWN;
		m_blupiPhase = 0;
	}

	if (m_keyPress & KEY_DOWN && 
		!(m_keyPress & (KEY_JUMP | KEY_LEFT | KEY_RIGHT)) &&
		IsBlupiStanding() &&
		m_blupiAction != ACTION_DOWN &&
		m_blupiAction != ACTION_STOPPOP &&
		m_blupiFocus)
	{
		m_blupiAction = ACTION_DOWN;
		m_blupiPhase = 0;
	}
	if (!(m_keyPress & KEY_DOWN) &&
		m_lastKeyPress & KEY_DOWN &&
		IsBlupiStanding() &&
		m_blupiFocus)
	{
		StopBlupi();
	}
	if (m_blupiAction == ACTION_UP && m_blupiPhase == 4)
	{
		m_scrollAdd.y = -150;
		PlaySound(SOUND_LOOKUP, end);
	}
	if (m_blupiAction == ACTION_DOWN && m_blupiPhase == 4)
	{
		m_scrollAdd.y = 150;
		PlaySound(SOUND_DOWN, end);
	}
	if (!IsBlupiVehicle() &&
		!m_blupiBalloon &&
		!m_blupiEcrase &&
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
			PlaySound(SOUND_POP, end);
		}
		if (m_keyPress & (KEY_DOWN | KEY_RIGHT) && 
			!(m_keyPress & KEY_JUMP) &&
			m_blupiAction != ACTION_POP &&
			m_blupiDir == DIR_LEFT &&
			CaisseInFront() != -1)
		{
			m_blupiAction = ACTION_POP;
			m_blupiPhase = 0;
			m_scrollAdd.y = 0;
			PlaySound(SOUND_POP, end);
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
			PlaySound(SOUND_POP, end);
		}
		if (m_keyPress & (KEY_DOWN | KEY_LEFT) && 
			!(m_keyPress & KEY_JUMP) &&
			m_blupiAction != ACTION_POP &&
			m_blupiDir == DIR_RIGHT &&
			CaisseInFront() != -1)
		{
			m_blupiAction = ACTION_POP;
			m_blupiPhase = 0;
			m_scrollAdd.y = 0;
			PlaySound(SOUND_POP, end);
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
		if (m_keyPress & KEY_LEFT && !(m_lastKeyPress & (KEY_LEFT | KEY_RIGHT)) && !m_blupiAir && m_keyPress & (KEY_UP | KEY_DOWN))
		{
			m_blupiAction = 1;
			m_blupiPhase = 0;
		}
		if (!(m_keyPress & (KEY_LEFT | KEY_RIGHT)) && m_lastKeyPress & KEY_LEFT && m_keyPress & (KEY_UP | KEY_DOWN))
		{
			m_blupiAction = 1;
			m_blupiPhase = 0;
		}
		if (m_keyPress & KEY_RIGHT && !(m_lastKeyPress & (KEY_LEFT | KEY_RIGHT)) && !m_blupiAir && m_keyPress & (KEY_UP | KEY_DOWN))
		{
			m_blupiAction = 1;
			m_blupiPhase = 0;
		}
		if (!(m_keyPress & (KEY_LEFT | KEY_RIGHT)) && m_lastKeyPress & KEY_RIGHT && m_keyPress & (KEY_UP | KEY_DOWN))
		{
			m_blupiAction = 1;
			m_blupiPhase = 0;
		}
	}
	int num3;
	int num4;
	if (m_keyPress & KEY_LEFT && m_blupiFocus)
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
				PlaySound(SOUND_TURN, end);
				m_blupiAction = ACTION_TURNAIR;
				m_blupiPhase = 0;
				m_blupiDir = DIR_LEFT;
			}
			else
			{
				PlaySound(SOUND_TURN, end);
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
			!IsBlupiVehicle() &&
			!m_blupiBalloon &&
			!m_blupiEcrase &&
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
	} // if (m_keyPress & KEY_LEFT && m_blupiFocus)

	if (m_keyPress & KEY_RIGHT && m_blupiFocus)
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
				PlaySound(SOUND_TURN, end);
				m_blupiAction = ACTION_TURNAIR;
				m_blupiPhase = 0;
				m_blupiDir = DIR_RIGHT;
			}
			else
			{
				PlaySound(SOUND_TURN, end);
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
			!IsBlupiVehicle() &&
			!m_blupiBalloon &&
			!m_blupiEcrase &&
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
	} // if (m_keyPress & KEY_RIGHT && m_blupiFocus)

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
			StopBlupi();
			if (m_blupiDir == DIR_LEFT) m_blupiDir = DIR_RIGHT;
			else m_blupiDir = DIR_LEFT;
		}
	}
	else if (m_blupiTank)
	{
		if (m_blupiAction == ACTION_TURN && m_blupiPhase == 12)
		{
			StopBlupi();
			if (m_blupiDir == DIR_LEFT) m_blupiDir = DIR_RIGHT;
			else m_blupiDir = DIR_LEFT;
		}
	}
	else if (m_blupiSkate)
	{
		if (m_blupiAction == ACTION_TURN && m_blupiPhase == 14)
		{
			StopBlupi();
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
			StopBlupi();
			if (m_blupiDir == DIR_LEFT) m_blupiDir = DIR_RIGHT;
			else m_blupiDir = DIR_LEFT;
		}
	}
	else
	{
		if (m_blupiAction == ACTION_TURN && m_blupiPhase == 6)
		{
			StopBlupi();
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
		StopBlupi();
	}

	if (!(m_keyPress & (KEY_LEFT | KEY_RIGHT | KEY_UP | KEY_DOWN)) &&
		!IsBlupiVehicle() &&
		!m_blupiBalloon &&
		!m_blupiEcrase &&
		!m_blupiNage &&
		!m_blupiSurf &&
		m_blupiFocus)
	{
		if (m_blupiAction == ACTION_PUSH || m_blupiAction == ACTION_UP)
		{
			StopBlupi();
		}
		if (m_blupiAction == ACTION_MARCH)
		{
			if (m_blupiSuspend || m_blupiPhase < 10)
			{
				StopBlupi();
			}
			else
			{
				m_blupiAction = ACTION_STOPMARCH;
				m_blupiPhase = 0;
			}
		}
		if (m_blupiAction == ACTION_DOWN)
		{
			StopBlupi();
			PlaySound(ACTION_UP, end);
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

	if (!(m_keyPress & (KEY_LEFT | KEY_RIGHT | KEY_UP | KEY_DOWN | KEY_JUMP)))
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
		if ((m_blupiNage || m_blupiSurf) &&
			m_blupiFocus &&
			m_blupiAction == ACTION_MARCH)
		{
			StopBlupi();
		}
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
				PlaySound(SOUND_FIREKO, m_blupiPos);
			}
			else
			{
				m_blupiAction = ACTION_HELICOGLU;
				m_blupiPhase = 0;
				m_blupiFocus = FALSE;
			}
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
				StopBlupi();
				m_blupiVitesse.y = 0;
			}
		}
		
		if (m_keyPress & (KEY_UP | KEY_JUMP))
		{
			if (m_blupiVitesse.y > -10) m_blupiVitesse.y -= 2;
		}
		else if (m_keyPress & KEY_DOWN)
		{
			if (m_blupiVitesse.y < 12) m_blupiVitesse.y += 2;
		}
		else
		{
			if (m_blupiVitesse.y > 1) m_blupiVitesse.y -= 1;
			if (m_blupiVitesse.y < 1) m_blupiVitesse.y += 1;
		}
		end.y += m_blupiVitesse.y;

		if (m_keyPress & KEY_LEFT)
		{
			if (m_blupiVitesse.x > -16)
			{
				m_blupiVitesse.x -= 2;
			}
			if (BlupiBloque(POINT( end.x + m_blupiVitesse.x, end.y ), -1))
			{
				m_blupiVitesse.x = 0;
			}
		}
		else if (m_keyPress & KEY_RIGHT)
		{
			if (m_blupiVitesse.x < 16)
			{
				m_blupiVitesse.x += 2;
			}
			if (BlupiBloque(POINT( end.x + m_blupiVitesse.x, end.y ), -1))
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
				ObjectStart(tinyPoint, TYPE_HELICO, 0);
				m_blupiHelico = FALSE;
				StopBlupi(TRUE);
				m_blupiPosHelico = m_blupiPos;
				StopSound(SOUND_HELICOHIGH);
				StopSound(SOUND_HELICOLOW);
				PlaySound(SOUND_HELICOSTOP, m_blupiPos);
			}
		}
	}
	if (m_blupiOver && (m_blupiFocus || m_blupiAction == ACTION_HELICOGLU))
	{
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
					StopBlupi();
				}
				m_blupiVitesse.y = 0;
			}
		}
		rect = BlupiRect(end);
		rect.top = end.y + 60 - 2;
		rect.bottom = end.y + 60 + OVERHEIGHT - 1;
		bIsOpen = !DecorDetect(rect);
		num = MoveAscenseurDetect(m_blupiPos, OVERHEIGHT);
		if (num != -1)
		{
			bIsOpen = FALSE;
		}
		if (m_keyPress & (KEY_JUMP | KEY_UP) && !bIsOpen)
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
		if (m_keyPress & KEY_LEFT && bAirStart)
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
		else if (m_keyPress & KEY_RIGHT && bAirStart)
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

		if ( !bAirStart && m_blupiTransport == -1)
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
				ObjectStart(tinyPoint, TYPE_OVER, 0);
				m_blupiOver = FALSE;
				StopBlupi(TRUE);
				m_blupiPosHelico = m_blupiPos;
				StopSound(SOUND_HELICOHIGH);
				StopSound(SOUND_HELICOLOW);
				PlaySound(SOUND_HELICOSTOP, m_blupiPos);
			}
		}
	}
	if (m_blupiBalloon && m_blupiFocus)
	{
		if (m_keyPress & (KEY_JUMP | KEY_UP))
		{
			if (m_blupiVitesse.y > -5 && m_time % 6 == 0)
			{
				m_blupiVitesse.y -= 1;
			}
		}
		else if (m_keyPress & KEY_DOWN)
		{
			if (m_blupiVitesse.y < 0 && m_time % 6 == 0)
			{
				m_blupiVitesse.y += 1;
			}
		}
		else if (m_blupiVitesse.y > -3 && m_time % 6 == 0)
		{
			m_blupiVitesse.y -= 1;
		}
		end.y += (int)m_blupiVitesse.y;
		if (m_keyPress & KEY_LEFT)
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
		else if (m_keyPress & KEY_RIGHT)
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
		if (m_keyPress & KEY_LEFT) num3 = -num3;
		if (m_keyPress & KEY_LEFT)
		{
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
		else if (m_keyPress & KEY_RIGHT)
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
			StopBlupi();
		}
		end.x += (int)m_blupiVitesse.x;
	}
	if (m_blupiJeep && m_blupiFocus)
	{
		if (m_blupiVitesse.x == 0 && m_blupiAction == 2)
		{
			StopBlupi();
		}
		//m_blupiMotorHigh = (m_blupiAction != 1);
		rect = BlupiRect(end);
		rect.right -= 40;
		rect.top = end.y + 60 - 2;
		rect.bottom = end.y + 60 - 1;
		bIsOpen = !DecorDetect(rect);
		rect.left += 40;
		rect.right += 40;
		bIsOpen2 = !DecorDetect(rect);
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
				PlaySound(SOUND_JUMPEND, m_blupiPos);
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
				PlaySound(SOUND_JUMPEND, end);
				end.y = m_moveObject[num].posCurrent.y - 64 + BLUPIOFFY;
			}
		}
		if (bIsOpen && !bIsOpen2)
		{
			int num13 = -20;
			if (m_blupiVitesse.x > num13)
			{
				m_blupiVitesse.x -= 1;
			}
		}
		else if (!bIsOpen && bIsOpen2)
		{
			int num14 = 20;
			if (m_blupiVitesse.x < num14)
			{
				m_blupiVitesse.x += 1;
			}
		}
		else if (m_keyPress & KEY_LEFT)
		{
			if (m_blupiVitesse.x > -20)
			{
				m_blupiVitesse.x -= 1;
			}
		}
		else if (m_keyPress & KEY_RIGHT)
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
		if ( !bLastBlupiAir && m_blupiTransport == -1)
		{
			tinyPoint.x = m_blupiPos.x;
			tinyPoint.y = m_blupiPos.y - BLUPIFLOOR;
			ObjectStart(tinyPoint, TYPE_JEEP, 0);
			m_blupiJeep = FALSE;
			StopBlupi();
			m_blupiPosHelico = m_blupiPos;
			StopSound(SOUND_JEEPHIGH);
			StopSound(SOUND_JEEPLOW);
			PlaySound(SOUND_JEEPSTOP, m_blupiPos);
		}
	}
	if (m_blupiTank && m_blupiFocus)
	{
		if (m_blupiAction == 53 && m_blupiPhase == 6)
		{
			StopBlupi();
		}
		if (m_keyPress & KEY_FIRE && m_blupiTimeFire == 0 && m_blupiAction != ACTION_TURN)
		{
			if (m_blupiBullet == 0)
			{
				PlaySound(SOUND_FIREKO, m_blupiPos);
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
				if (ObjectStart(tinyPoint, TYPE_BALLE, num4) != -1)
				{
					m_blupiAction = 53;
					m_blupiPhase = 0;
					PlaySound(SOUND_FIREOK, m_blupiPos);
					m_blupiTimeFire = 10;
					m_blupiBullet--;
				}
			}
		}
		if (m_blupiVitesse.x == 0 && m_blupiAction == 2)
		{
			StopBlupi();
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
				PlaySound(SOUND_JUMPEND, m_blupiPos);
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
				PlaySound(SOUND_JUMPEND, end);
				end.y = m_moveObject[num].posCurrent.y - 64 + BLUPIOFFY;
			}
		}
		if (m_keyPress & KEY_LEFT)
		{
			if (m_blupiVitesse.x > -12)
			{
				m_blupiVitesse.x -= 1;
			}
		}
		else if (m_keyPress & KEY_RIGHT)
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
		if ( m_keyPress & KEY_DOWN && !bAirStart && m_blupiTransport == -1)
		{
			
			tinyPoint.x = m_blupiPos.x;
			tinyPoint.y = m_blupiPos.y;
			ObjectStart(tinyPoint, TYPE_TANK, 0);
			m_blupiTank = FALSE;
			StopBlupi();
			m_blupiPosHelico = m_blupiPos;
			StopSound(SOUND_JEEPHIGH);
			StopSound(SOUND_JEEPLOW);
			PlaySound(SOUND_JEEPSTOP, m_blupiPos);
		}
	}
	if (m_blupiSkate && m_blupiFocus)
	{
		if (m_blupiVitesse.x == 0 && m_blupiAction == 2)
		{
			StopBlupi();
		}
		if (m_keyPress & KEY_LEFT)
		{
			if (m_blupiVitesse.x > -15)
			{
				m_blupiVitesse.x -= 1;
			}
		}
		else if (m_keyPress & KEY_RIGHT)
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
		if ( m_keyPress & KEY_DOWN && !bAirStart && !m_blupiAir && m_blupiTransport == -1 && m_blupiVitesse.x < 8)
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
			StopBlupi(TRUE);
		}
	}
	if (m_blupiAction == 43)
	{
		if (m_blupiPhase == 12)
		{
			tinyPoint.x = m_blupiPos.x;
			tinyPoint.y = m_blupiPos.y - BLUPIFLOOR + 1;
			ObjectStart(tinyPoint, TYPE_SKATE, 0);
		}
		if (m_blupiPhase == 20)
		{
			StopBlupi(TRUE);
		}
	}
	if (m_blupiNage && m_blupiFocus)
	{
		if (m_blupiTransport == -1)
		{
			if (m_keyPress & (KEY_JUMP | KEY_UP))
			{
				if (m_blupiVitesse.y > -5)
				{
					m_blupiVitesse.y -= 1;
				}
			}
			else if (m_keyPress & KEY_DOWN)
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
			if (!m_blupiShield && !m_blupiHide && !m_bSuperBlupi)
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
				PlaySound(SOUND_DROWN, end);
			}
		}
	}
	if (m_blupiSurf && m_blupiFocus)
	{
		if (m_blupiTransport == -1)
		{
			if (m_keyPress & (KEY_JUMP | KEY_UP))
			{
				if (m_blupiVitesse.y > -5)
				{
					m_blupiVitesse.y -= 1;
				}
			}
			else if (m_keyPress & KEY_DOWN)
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
				StopBlupi();
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
		if (!(m_keyPress & KEY_JUMP) && !(m_keyPress & (KEY_UP | KEY_DOWN)) && m_blupiAction == 4)
		{
			StopBlupi();
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
			PlaySound(SOUND_SUSPENDJUMP, end);
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
			rect.top = end.y + 60 - 2;
			rect.bottom = end.y + 60;
			bIsOpen = !DecorDetect(rect);
			rect.left = end.x + 60 - 20;
			rect.right = end.x + 60 - 18;
			rect.top = end.y + 60 - 2;
			rect.bottom = end.y + 60;
			bIsOpen2 = !DecorDetect(rect);
			if (!bIsOpen && !bIsOpen2 && ObjectStart(end, TYPE_DYNAMITEf, 0) != -1)
			{
				m_blupiAction = ACTION_PUTDYNAMITE;
				m_blupiPhase = 0;
				m_blupiFocus = FALSE;
				PlaySound(SOUND_PERSOPOSE, end);
				m_blupiDynamite--;
			}
		}
		else if (m_blupiPerso > 0)
		{
			num = MoveObjectDetect(end, &bNear);
			if (num == -1 || m_moveObject[num].type != TYPE_BOMBEPERSO1)
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
				if (num == -1 && !bIsOpen && !bIsOpen2 && ObjectStart(end, TYPE_BOMBEPERSO1, 0) != -1)
				{
					m_blupiAction = ACTION_OUF2;
					m_blupiPhase = 0;
					m_blupiFocus = FALSE;
					PlaySound(SOUND_PERSOPOSE, end);
					m_blupiPerso--;
				}
			}
			if (m_blupiFocus)
			{
				m_blupiAction = ACTION_OUF3;
				m_blupiPhase = 0;
				PlaySound(SOUND_ERROR, end);
			}
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
				PlaySound(SOUND_VERTIGO, end);
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
				PlaySound(SOUND_VERTIGO, end);
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
				PlaySound(SOUND_VERTIGO, end);
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
		if (!bIsOpen && bIsOpen2)
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
		if (!bIsOpen && bIsOpen2)
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
		if (!bIsOpen && bIsOpen2)
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
			StopBlupi();
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
			StopBlupi();
		}
	}
	BlupiSearchIcon();
	if (m_blupiShield)
	{
		if (m_blupiTimeShield == 10)
		{
			PlaySound(SOUND_STOPSHIELD, m_blupiPos);
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
			PlaySound(SOUND_STOPPOWER, m_blupiPos);
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
			PlaySound(SOUND_STOPCLOUD, m_blupiPos);
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
			PlaySound(SOUND_STOPHIDE, m_blupiPos);
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
			ObjectStart(tinyPoint, TYPE_INVERTSTOP, -60);
			tinyPoint.x = m_blupiPos.x;
			tinyPoint.y = m_blupiPos.y - 100;
			ObjectStart(tinyPoint, TYPE_INVERTSTOP, 60);
			tinyPoint.x = m_blupiPos.x - 100;
			tinyPoint.y = m_blupiPos.y;
			ObjectStart(tinyPoint, TYPE_INVERTSTOP, 10);
			tinyPoint.x = m_blupiPos.x + 100;
			tinyPoint.y = m_blupiPos.y;
			ObjectStart(tinyPoint, TYPE_INVERTSTOP, -10);
			PlaySound(SOUND_INVERTSTOP, end);
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
			ObjectStart(tinyPoint, TYPE_EXPLO6, 0);
			PlaySound(SOUND_RESSORT, m_blupiPos);
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
			ObjectStart(m_blupiPos, TYPE_INVERTSTART, -60);
			ObjectStart(m_blupiPos, TYPE_INVERTSTART, 60);
			ObjectStart(m_blupiPos, TYPE_INVERTSTART, 10);
			ObjectStart(m_blupiPos, TYPE_INVERTSTART, -10);
			tinyPoint.x = m_blupiPos.x - 34;
			tinyPoint.y = m_blupiPos.y - 34;
			ObjectStart(tinyPoint, TYPE_EXPLO5, 0);
			PlaySound(SOUND_RESSORT, m_blupiPos);
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
	/* // winphone
	if (m_blupiHide && abs(m_blupiPos.x - m_blupiPosMagic.x) + abs(m_blupiPos.y - m_blupiPosMagic.y) >= 10)
	{
		num = MoveObjectFree();
		if (num != -1)
		{
			m_moveObject[num].type = TYPE_HIDETRACK;
			m_moveObject[num].icon = m_blupiIcon;
			m_moveObject[num].channel = CHBLUPI;
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
	*/
	if (m_blupiTimeNoAsc > 0)
	{
		m_blupiTimeNoAsc--;
	}
	if (m_blupiHelico && m_blupiPos.y > 2 && m_blupiFocus && !m_blupiShield && !m_blupiHide && !m_bSuperBlupi)
	{
		rect = BlupiRect(end);
		rect.top = end.y + 4;
		rect.bottom = end.y + 20;
		if (DecorDetect(rect))
		{
			StopBlupi();
			m_blupiHelico = FALSE;
			tinyPoint.x = m_blupiPos.x - 34;
			tinyPoint.y = m_blupiPos.y - 34;
			ObjectStart(tinyPoint, TYPE_EXPLO2, 0);
			m_decorAction = 1;
			m_decorPhase = 0;
			StopVehicleSound();
			PlaySound(SOUND_BOUM, m_blupiPos);
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
			StopBlupi();
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
				StopBlupi();
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
			PlaySound(SOUND_DROWN, m_blupiPos);
			m_jauges[JAUGE_AIR].SetHide(TRUE);
		}
		tinyPoint4.x = m_blupiPos.x;
		tinyPoint4.y = m_blupiPos.y - DIMOBJY;
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
			PlaySound(SOUND_JUMP0, m_blupiPos);
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
					PlaySound(SOUND_JUMP0, m_blupiPos);
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
				PlaySound(SOUND_JUMP0, m_blupiPos);
				m_jauges[JAUGE_AIR].SetHide(TRUE);
			}
			else
			{
				m_blupiNage = FALSE;
				m_blupiSurf = FALSE;
				m_blupiAir = FALSE;
				StopBlupi();
				m_blupiPos.y = m_blupiPos.y - 10;
				m_blupiVitesse.x = 0;
				m_blupiVitesse.y = 0;
				PlaySound(SOUND_JUMP0, m_blupiPos);
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
		
		StopBlupi();
		m_blupiHelico = FALSE;
		m_blupiOver = FALSE;
		m_blupiJeep = FALSE;
		m_blupiTank = FALSE;
		m_blupiSkate = FALSE;
		tinyPoint.x = m_blupiPos.x - 34;
		tinyPoint.y = m_blupiPos.y - 34;
		ObjectStart(tinyPoint, TYPE_EXPLO2, 0);
		m_decorAction = 1;
		m_decorPhase = 0;
		StopVehicleSound();
		PlaySound(SOUND_BOUM, m_blupiPos);
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
			StopBlupi();
			m_blupiActionOuf = 0;
			PlaySound(SOUND_SUSPENDTOC, m_blupiPos);
		}
	}
	if (m_blupiNoBarre > 0)
	{
		m_blupiNoBarre--;
	}
	if (IsVentillo(m_blupiPos))
	{
		if (m_blupiFocus && !m_blupiShield && !m_blupiHide && !m_bSuperBlupi)
		{
			BlupiDead(ACTION_CLEAR1, ACTION_CLEAR2);
		}
		tinyPoint.x = m_blupiPos.x - 34;
		tinyPoint.y = m_blupiPos.y - 34;
		ObjectStart(tinyPoint, TYPE_EXPLO4, 0);
		m_decorAction = 2;
		m_decorPhase = 0;
		StopVehicleSound();
		PlaySound(SOUND_BOUM, m_blupiPos);
	}
	if (m_blupiAction != ACTION_STOPTANK && m_blupiFocus)
	{
		num = IsWorld(m_blupiPos);
		if (num != -1)
		{
			StopVehicleSound();
			PlaySound(SOUND_BYE, m_blupiPos);
			m_blupiAction = ACTION_BYE;
			m_blupiPhase = 0;
			m_blupiFocus = FALSE;
			m_blupiFront = TRUE;
		}
	}
	int num27 = MoveObjectDetect(m_blupiPos, &bNear);
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
		if (IsLave(m_blupiPos) && !m_blupiShield && !m_blupiHide && !m_bSuperBlupi)
		{
			BlupiDead(ACTION_CLEAR3, -1);
			m_blupiRestart = TRUE;
			m_blupiPos.y = m_blupiPos.y / DIMOBJY * DIMOBJY + BLUPIOFFY;
			PlaySound(SOUND_FALL, m_blupiPos);
		}
		if (IsPiege(m_blupiPos) && !m_blupiOver && !m_blupiJeep && !m_blupiTank && !m_blupiShield && !m_blupiHide && !m_bSuperBlupi && m_blupiFocus)
		{
			BlupiDead(ACTION_GLU, -1);
			m_blupiRestart = TRUE;
			m_blupiAir = TRUE;
			ObjectStart(m_blupiPos, TYPE_TENTACULE, 0);
			PlaySound(SOUND_GLU, m_blupiPos);
		}
		if (IsGoutte(m_blupiPos, FALSE) && !m_blupiOver && !m_blupiJeep && !m_blupiTank && !m_blupiShield && !m_blupiHide && !m_bSuperBlupi && m_blupiFocus)
		{
			BlupiDead(ACTION_GLU, -1);
			m_blupiRestart = TRUE;
			m_blupiAir = TRUE;
			PlaySound(SOUND_GLU, m_blupiPos);
		}
		if (IsScie(m_blupiPos) && !m_blupiOver && !m_blupiJeep && !m_blupiTank && !m_blupiShield && !m_blupiHide && !m_bSuperBlupi && m_blupiFocus)
		{
			BlupiDead(ACTION_CLEAR4, -1);
			m_blupiFront = TRUE;
			m_blupiRestart = TRUE;
			m_blupiAir = TRUE;
		}
		if ( (num27 == -1 || !bNear) && IsSwitch(m_blupiPos, &tinyPoint) && !m_blupiOver && !m_blupiBalloon && !m_blupiJeep && !m_blupiTank && !m_blupiSkate && !m_blupiShield && !m_blupiHide && !m_bSuperBlupi && m_blupiFocus)
		{
			
			ActiveSwitch(m_decor[tinyPoint.x][tinyPoint.y].icon == 385, tinyPoint);
			m_blupiAction = ACTION_SWITCH;
			m_blupiPhase = 0;
			m_blupiFocus = FALSE;
			m_blupiVitesse.x = 0;
			m_blupiVitesse.y = 0;
		}
		if (IsBlitz(m_blupiPos, FALSE) && !m_blupiShield && !m_blupiHide && !m_bSuperBlupi)
		{
			BlupiDead(ACTION_CLEAR1, -1);
			m_blupiRestart = TRUE;
			m_blupiAir = TRUE;
			m_blupiPos.y = m_blupiPos.y / DIMOBJY * DIMOBJY + BLUPIOFFY;
			PlaySound(SOUND_FALL, m_blupiPos);
		}
		if (IsEcraseur(m_blupiPos) && !m_blupiEcrase && !m_blupiShield && !m_blupiHide && !m_bSuperBlupi && m_blupiFocus)
		{
			StopBlupi();
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
				StopVehicleSound();
			}
			PlaySound(SOUND_ECRASE, m_blupiPos);
			ObjectStart(m_blupiPos, TYPE_INVERTSTART, -60);
			ObjectStart(m_blupiPos, TYPE_INVERTSTART, 60);
			ObjectStart(m_blupiPos, TYPE_INVERTSTART, 10);
			ObjectStart(m_blupiPos, TYPE_INVERTSTART, -10);
			tinyPoint.x = m_blupiPos.x - 34;
			tinyPoint.y = m_blupiPos.y - 34;
			ObjectStart(tinyPoint, TYPE_EXPLO5, 0);
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
			PlaySound(SOUND_TELEPORTE, m_blupiPos);
			tinyPoint.x = m_blupiPos.x;
			tinyPoint.y = m_blupiPos.y - 5;
			ObjectStart(tinyPoint, TYPE_EXPLO7, 0);
		}
		if (IsBridge(m_blupiPos, &tinyPoint2) && m_blupiFocus)
		{
			tinyPoint2.x *= DIMOBJX;
			tinyPoint2.y *= DIMOBJY;
			ObjectStart(tinyPoint2, TYPE_BRIDGE, 0);
		}
		int num2 = IsDoor(m_blupiPos, &tinyPoint2);
		if (num2 != -1 && (m_blupiCle & (1 << num2) - 334) != 0)
		{
			OpenDoor(tinyPoint2);
			m_blupiCle &= ~((1 << num2) - 334);
			tinyPoint.x = 520;
			tinyPoint.y = 418;
			tinyPoint5.x = tinyPoint2.x * 64 - m_posDecor.x;
			tinyPoint5.y = tinyPoint2.y * 64 - m_posDecor.y;
			VoyageInit(tinyPoint, tinyPoint5, 214 + (num2 - 334) * 7, 10);
		}
	}
	if (!m_blupiHelico && !m_blupiSuspend && !m_blupiOver && !m_blupiBalloon && !m_blupiEcrase && !m_blupiSkate && !m_blupiJeep && !m_blupiTank && !m_blupiJeep && m_blupiFocus)
	{
		num = MockeryDetect(m_blupiPos);
		if (num != 0)
		{
			m_blupiActionOuf = num;
			m_blupiTimeOuf = 0;
		}
	}
	MoveObjectFollow(m_blupiPos);
	num = num27;
	if (num != -1 && !bNear && m_moveObject[num].type == TYPE_BOMBEDOWN && !m_blupiHelico && !m_blupiOver && !m_blupiBalloon && !m_blupiEcrase && !m_blupiJeep && !m_blupiTank && !m_blupiSkate && !m_blupiNage && !m_blupiSurf && !m_blupiSuspend && !m_blupiShield && !m_bSuperBlupi && m_blupiFocus)
	{
		m_blupiActionOuf = 48;
		m_blupiTimeOuf = 0;
	}
	if (num != -1 && bNear)
	{
		if (m_moveObject[num].type == TYPE_HELICO &&
			!IsBlupiVehicle() &&
			!m_blupiBalloon &&
			!m_blupiEcrase &&
			!m_blupiNage &&
			!m_blupiSurf &&
			!m_blupiSuspend &&
			m_blupiFocus)
		{
			ObjectDelete(m_moveObject[num].posCurrent, m_moveObject[num].type);
			m_scrollAdd.x = 0;
			m_scrollAdd.y = 0;
			m_blupiAir = FALSE;
			m_blupiHelico = TRUE;
			m_blupiVitesse.x = 0;
			if (m_blupiCloud || m_blupiHide)
			{
				m_blupiCloud = FALSE;
				m_blupiHide = FALSE;
				m_jauges[JAUGE_POWER].SetHide(TRUE);
			}
		}
		if (m_moveObject[num].type == TYPE_OVER &&
			!IsBlupiVehicle() &&
			!m_blupiBalloon &&
			!m_blupiEcrase &&
			!m_blupiNage &&
			!m_blupiSurf &&
			!m_blupiSuspend &&
			m_blupiFocus)
		{
			ObjectDelete(m_moveObject[num].posCurrent, m_moveObject[num].type);
			m_scrollAdd.x = 0;
			m_scrollAdd.y = 0;
			m_blupiAir = FALSE;
			m_blupiOver = TRUE;
			m_blupiVitesse.x = 0;
			if (m_blupiCloud || m_blupiHide)
			{
				m_blupiCloud = FALSE;
				m_blupiHide = FALSE;
				m_jauges[JAUGE_POWER].SetHide(TRUE);
			}
		}
		if (m_moveObject[num].type == TYPE_JEEP &&
			!IsBlupiVehicle() &&
			!m_blupiBalloon &&
			!m_blupiEcrase &&
			!m_blupiNage &&
			!m_blupiSurf &&
			!m_blupiSuspend &&
			m_blupiFocus)
		{
			ObjectDelete(m_moveObject[num].posCurrent, m_moveObject[num].type);
			m_scrollAdd.x = 0;
			m_scrollAdd.y = 0;
			StopBlupi();
			m_blupiAir = FALSE;
			m_blupiJeep = TRUE;
			m_blupiVitesse.x = 0;
			if (m_blupiCloud || m_blupiHide)
			{
				m_blupiCloud = FALSE;
				m_blupiHide = FALSE;
				m_jauges[JAUGE_POWER].SetHide(TRUE);
			}
		}
		if ( m_moveObject[num].type == TYPE_TANK &&
			!IsBlupiVehicle() &&
			!m_blupiBalloon &&
			!m_blupiEcrase &&
			!m_blupiNage &&
			!m_blupiSurf &&
			!m_blupiSuspend &&
			m_blupiFocus)
		{
			
			ObjectDelete(m_moveObject[num].posCurrent, m_moveObject[num].type);
			m_scrollAdd.x = 0;
			m_scrollAdd.y = 0;
			StopBlupi();
			m_blupiAir = FALSE;
			m_blupiTank = TRUE;
			m_blupiVitesse.x = 0;
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
			tinyPoint5.x = 570;
			tinyPoint5.y = 430;
			VoyageInit(tinyPoint, tinyPoint5, 177, 10);
			m_blupiBullet += 10;
			if (m_blupiBullet > 10)
			{
				m_blupiBullet = 10;
			}
		}
		if ( m_moveObject[num].type == TYPE_SKATE &&
			!IsBlupiVehicle() &&
			!m_blupiBalloon &&
			!m_blupiEcrase &&
			!m_blupiNage &&
			!m_blupiSurf &&
			!m_blupiSuspend &&
			m_blupiFocus)
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
			ObjectStart(tinyPoint, TYPE_EXPLO6, 0);
			PlaySound(SOUND_RESSORT, m_blupiPos);
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
				ObjectStart(tinyPoint, TYPE_EXPLO3, 0);
				m_decorAction = 2;
				m_decorPhase = 0;
			}
			else
			{
				tinyPoint = m_moveObject[num].posCurrent;
				ObjectDelete(tinyPoint, m_moveObject[num].type);
				tinyPoint.x -= 34;
				tinyPoint.y -= 34;
				ObjectStart(tinyPoint, TYPE_EXPLO1, 0);
				m_decorAction = 1;
				m_decorPhase = 0;
			}
			if (!m_blupiJeep && !m_blupiTank)
			{
				StopVehicleSound();
			}
			PlaySound(SOUND_BOUM, m_moveObject[num].posCurrent);
		}
		if (m_moveObject[num].type == TYPE_GUEPE &&
			m_blupiFocus &&
			!m_blupiBalloon &&
			!m_blupiShield &&
			!m_bSuperBlupi)
		{
			StopBlupi();
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
				StopVehicleSound();
			}
			PlaySound(SOUND_JUMPAIE, m_moveObject[num].posCurrent);
			tinyPoint.x = m_blupiPos.x - 34;
			tinyPoint.y = m_blupiPos.y - 34;
			ObjectStart(tinyPoint, TYPE_EXPLO5, 0);
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
			ObjectStart(tinyPoint, TYPE_TENTACULE, 0);
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
			if (IsBlupiVehicle() ||
				m_blupiBalloon ||
				m_blupiEcrase)
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
				ObjectStart(tinyPoint, TYPE_EXPLO3, 0);
				StopVehicleSound();
				PlaySound(SOUND_BOUM, m_moveObject[num].posCurrent);
				m_decorAction = 1;
				m_decorPhase = 0;
			}
			else
			{
				PlaySound(SOUND_GLU, m_moveObject[num].posCurrent);
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
			!IsDeadAction(m_blupiAction) &&
			m_blupiAction != ACTION_GLU &&
			m_blupiAction != ACTION_ELECTRO &&
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
			if (IsBlupiVehicle() ||
				m_blupiBalloon ||
				m_blupiEcrase)
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
			tinyPoint.x = m_moveObject[num].posCurrent.x - m_posDecor.x;
			tinyPoint.y = m_moveObject[num].posCurrent.y - m_posDecor.y;
			POINT end;
			end.x = 430;
			end.y = 430;
			if (m_mission == 10) { // BUG: VoyageInit is skipped in custom levels with id 10. To fix, add `&& !m_bPrivate`
				m_nbTresor++;
				OpenDoorsTresor();
				PlaySound(SOUND_TRESOR, m_moveObject[num].posCurrent, FALSE);
			}
			else {
				VoyageInit(tinyPoint, end, 6, CHELEMENT);
			}
			ObjectStart(m_moveObject[num].posCurrent, TYPE_TRESORTRACK, -60);
			ObjectStart(m_moveObject[num].posCurrent, TYPE_TRESORTRACK, 60);
			ObjectStart(m_moveObject[num].posCurrent, TYPE_TRESORTRACK, 10);
			ObjectStart(m_moveObject[num].posCurrent, TYPE_TRESORTRACK, -10);
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
			ObjectStart(m_moveObject[num].posCurrent, TYPE_TRESORTRACK, -60);
			ObjectStart(m_moveObject[num].posCurrent, TYPE_TRESORTRACK, 60);
			ObjectStart(m_moveObject[num].posCurrent, TYPE_TRESORTRACK, 10);
			ObjectStart(m_moveObject[num].posCurrent, TYPE_TRESORTRACK, -10);
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
			ObjectStart(m_moveObject[num].posCurrent, TYPE_TRESORTRACK, -60);
			ObjectStart(m_moveObject[num].posCurrent, TYPE_TRESORTRACK, 60);
			ObjectStart(m_moveObject[num].posCurrent, TYPE_TRESORTRACK, 10);
			ObjectStart(m_moveObject[num].posCurrent, TYPE_TRESORTRACK, -10);
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
			ObjectStart(m_moveObject[num].posCurrent, TYPE_TRESORTRACK, -60);
			ObjectStart(m_moveObject[num].posCurrent, TYPE_TRESORTRACK, 60);
			ObjectStart(m_moveObject[num].posCurrent, TYPE_TRESORTRACK, 10);
			ObjectStart(m_moveObject[num].posCurrent, TYPE_TRESORTRACK, -10);
		}
		if (m_moveObject[num].type == TYPE_EGG && m_nbVies < 10 && m_blupiFocus)
		{
			ObjectDelete(m_moveObject[num].posCurrent, m_moveObject[num].type);
			tinyPoint.x = m_moveObject[num].posCurrent.x - m_posDecor.x;
			tinyPoint.y = m_moveObject[num].posCurrent.y - m_posDecor.y;
			VoyageInit(tinyPoint, VoyageGetPosVie(m_nbVies + 1), 21, CHELEMENT);
		}
		if (m_moveObject[num].type == TYPE_SHIELD && !m_blupiShield && !m_blupiHide && !m_blupiPower && m_blupiFocus)
		{
			PlaySound(SOUND_STARTSHIELD, m_moveObject[num].posCurrent);
			m_blupiShield = TRUE;
			m_blupiPower = FALSE;
			m_blupiCloud = FALSE;
			m_blupiHide = FALSE;
			m_blupiTimeShield = 100;
			m_blupiPosMagic = m_blupiPos;
			m_jauges[JAUGE_POWER].SetHide(FALSE);
		}
		if ( m_moveObject[num].type == TYPE_POWER &&
			!m_blupiShield &&
			!IsBlupiVehicle() &&
			!m_blupiBalloon &&
			!m_blupiEcrase &&
			m_blupiFocus)
		{
			ObjectDelete(m_moveObject[num].posCurrent, m_moveObject[num].type);
			m_blupiAction = ACTION_SUCETTE;
			m_blupiPhase = 0;
			m_blupiCloud = FALSE;
			m_blupiHide = FALSE;
			m_blupiFocus = FALSE;
			PlaySound(SOUND_SUCETTE, end);
		}
		if (m_moveObject[num].type == TYPE_INVERT && !m_blupiHide && m_blupiFocus)
		{
			ObjectDelete(m_moveObject[num].posCurrent, m_moveObject[num].type);
			m_blupiInvert = TRUE;
			m_blupiTimeShield = 100;
			m_blupiPosMagic = m_blupiPos;
			m_jauges[JAUGE_POWER].SetHide(FALSE);
			PlaySound(SOUND_INVERTSTART, end);
			ObjectStart(m_blupiPos, TYPE_INVERTSTART, -60);
			ObjectStart(m_blupiPos, TYPE_INVERTSTART, 60);
			ObjectStart(m_blupiPos, TYPE_INVERTSTART, 10);
			ObjectStart(m_blupiPos, TYPE_INVERTSTART, -10);
		}
		if ( m_moveObject[num].type == TYPE_DRINK &&
			!m_blupiShield &&
			!m_blupiCloud &&
			!IsBlupiVehicle() &&
			!m_blupiBalloon &&
			!m_blupiEcrase &&
			m_blupiFocus)
		{
			ObjectDelete(m_moveObject[num].posCurrent, m_moveObject[num].type);
			m_blupiAction = ACTION_DRINK;
			m_blupiPhase = 0;
			m_blupiShield = FALSE;
			m_blupiPower = FALSE;
			m_blupiJumpAie = FALSE;
			m_blupiFocus = FALSE;
			PlaySound(SOUND_DRINK, end);
		}
		if (m_moveObject[num].type == TYPE_CHARGE &&
			!m_blupiShield &&
			!m_blupiHide &&
			!m_blupiPower &&
			!m_blupiCloud &&
			!IsBlupiVehicle() &&
			!m_blupiBalloon &&
			!m_blupiEcrase &&
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
			PlaySound(SOUND_CHARGE, end);
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
			if (m_moveObject[num].type == TYPE_BOMBEPERSO1)
			{
				if (m_blupiPerso < 5 )
				{
					
					ObjectDelete(m_moveObject[num].posCurrent, m_moveObject[num].type);
					tinyPoint.x = m_moveObject[num].posCurrent.x - m_posDecor.x;
					tinyPoint.y = m_moveObject[num].posCurrent.y - m_posDecor.y;
					tinyPoint5.x = 0;
					tinyPoint5.y = 438;
					VoyageInit(tinyPoint, tinyPoint5, 108, CHBUTTON);
				}
			}
			else if (!m_blupiShield && !m_bSuperBlupi)
			{
				ObjectDelete(m_moveObject[num].posCurrent, m_moveObject[num].type);
				BlupiDead(ACTION_CLEAR1, ACTION_CLEAR2);
				tinyPoint = m_moveObject[num].posCurrent;
				tinyPoint.x -= 34;
				tinyPoint.y -= 34;
				ObjectStart(tinyPoint, TYPE_EXPLO3, 0);
				PlaySound(SOUND_BOUM, m_moveObject[num].posCurrent);
				m_decorAction = 1;
				m_decorPhase = 0;
			}
		}
		if (m_moveObject[num].type == TYPE_DYNAMITE &&
			m_blupiFocus &&
			m_blupiDynamite == 0 &&
			(m_voyageIcon != 252 || m_voyageChannel != CHELEMENT) )
		{
			
			ObjectDelete(m_moveObject[num].posCurrent, m_moveObject[num].type);
			tinyPoint.x = m_moveObject[num].posCurrent.x - m_posDecor.x;
			tinyPoint.y = m_moveObject[num].posCurrent.y - m_posDecor.y;
			tinyPoint5.x = 505;
			tinyPoint5.y = 414;
			VoyageInit(tinyPoint, tinyPoint5, 252, CHELEMENT);
			m_blupiAction = ACTION_TAKEDYNAMITE;
			m_blupiPhase = 0;
			m_blupiFocus = FALSE;
		}
		if (m_moveObject[num].type == 12 &&
			!IsBlupiVehicle() &&
			!m_blupiBalloon &&
			!m_blupiEcrase &&
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
				PlaySound(SOUND_PUSH, end);
				m_blupiActionOuf = ACTION_OUF1b;
				m_blupiTimeOuf = 0;
				m_blupiAction = ACTION_PUSH;
				m_blupiPhase = 0;
			}
			if (m_blupiDir == DIR_RIGHT && m_blupiPos.x < end.x)
			{
				end.x = m_blupiPos.x + 55;
				PlaySound(SOUND_PUSH, end);
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
					StopVehicleSound();
					PlaySound(SOUND_ENDOK, m_moveObject[num].posCurrent);
					m_blupiAction = ACTION_WIN;
					m_blupiPhase = 0;
					m_blupiFocus = FALSE;
					m_blupiFront = TRUE;
					m_blupiPos.y = m_moveObject[num].posCurrent.y;
				}
				else
				{
					PlaySound(SOUND_ENDKO, m_moveObject[num].posCurrent);
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
			StopBlupi();
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
				StopBlupi();
			}
		}
		else
		{
			StopBlupi();
		}
	}
	if (!IsBlupiVehicle() &&
		!m_blupiBalloon &&
		!m_blupiEcrase &&
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
				PlaySound(SOUND_OUF1, m_blupiPos);
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
			PlaySound(SOUND_OUF1, m_blupiPos);
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
				PlaySound(SOUND_OUF3, m_blupiPos);
			}
			m_blupiActionOuf = 0;
		}
		if (m_blupiActionOuf == ACTION_OUF4 && m_blupiAction == ACTION_STOP)
		{
			if (m_blupiTimeOuf < 10)
			{
				m_blupiAction = m_blupiActionOuf;
				m_blupiPhase = 0;
				PlaySound(SOUND_OUF4, m_blupiPos);
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
	if (m_blupiAction == ACTION_TAKEDYNAMITE && m_blupiPhase == 18)
	{
		StopBlupi(TRUE);
	}
	if (m_blupiAction == ACTION_PUTDYNAMITE && m_blupiPhase == 26)
	{
		StopBlupi(TRUE);
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
			ObjectStart(m_blupiPos, TYPE_MAGICTRACK, 20);
			ObjectStart(m_blupiPos, TYPE_MAGICTRACK, -20);
		}
		m_blupiFocus = TRUE;
		m_blupiPosHelico = m_blupiPos;
	}
	if (m_blupiAction == ACTION_SWITCH && m_blupiPhase == 10)
	{
		StopBlupi(TRUE);
	}
	//// winphone ?
	//if (m_keyPress & KEY_FIRE && m_blupiAction == ACTION_STOP)
	//{
	//	m_blupiAction = ACTION_NON;
	//	m_blupiPhase = 0;
	//	PlaySound(SOUND_ERROR, m_blupiPos);
	//}
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
		(m_blupiAction == ACTION_ELECTRO && m_blupiPhase == 90))
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
			POINT posDecor = GetPosDecor(m_blupiPos);
			tinyPoint.x = m_blupiPos.x - posDecor.x - 30;
			tinyPoint.y = m_blupiPos.y - posDecor.y;
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
		int num = 0;
		for (int i = 0; i < MAXNETPLAYER; i++)
		{
			if (m_pNetwork->m_players[i].bIsPresent) num++;
		}
		if (num < 2)
		{
			char str[100];
			StopVehicleSound();
			PlaySound(14, m_blupiPos, FALSE);
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
		PlaySound(SOUND_FALL, m_blupiPos);
	}
	if (m_blupiPos.y > num3 + 1000)
	{
		m_term = -1;
		DoorsLost();
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
		!m_blupiHide &&
		!bIsOpen &&
		!bIsOpen2 &&
		m_blupiTransport == -1 &&
		!IsLave(m_blupiPos) &&
		!IsPiege(m_blupiPos) &&
		!IsGoutte(m_blupiPos, TRUE) &&
		!IsScie(m_blupiPos) &&
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
	end.x = m_blupiPos.x + DIMBLUPIX / 2 + m_scrollAdd.x;
	end.y = m_blupiPos.y + DIMBLUPIY / 2 + m_scrollAdd.y;
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
		m_blupiAction != ACTION_CLEAR2 &&
		m_blupiAction != ACTION_CLEAR3)
	{
		m_posDecor = GetPosDecor(m_scrollPoint);
	}
	if (m_time % 4 == 0)
	{
		AdaptMotorVehicleSound(m_blupiPos);
	}
	VoyageStep();

	m_lastKeyPress = m_keyPress;

	// DEBUG v
	sprintf(m_notifText[0], "time %d", m_time);
	sprintf(m_notifText[1], "act %d", m_blupiAction);
	m_notifTime = 200;
	// DEBUG ^
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
	m_blupiVent = FALSE;
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
		PlaySound(SOUND_ANGEL, m_blupiPos);
	}
	if (m_blupiAction == ACTION_CLEAR3)
	{
		pos.x = m_blupiPos.x - m_posDecor.x;
		pos.y = m_blupiPos.y - m_posDecor.y;
		pos2.x = m_blupiPos.x - m_posDecor.x;
		pos2.y = m_blupiPos.y - m_posDecor.y - 2000;
		VoyageInit(pos, pos2, 40, CHELEMENT);
		PlaySound(SOUND_ANGEL, m_blupiPos);
	}
	if (m_blupiAction == ACTION_CLEAR4)
	{
		ObjectStart(m_blupiPos, TYPE_INVERTSTART, -70);
		ObjectStart(m_blupiPos, TYPE_INVERTSTART, 20);
		ObjectStart(m_blupiPos, TYPE_INVERTSTART, -20);
		PlaySound(SOUND_SCIE, m_blupiPos);
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
				if (icon >= 0 && icon < MAXQUART && (!m_blupiHelico || icon != 214) && (m_blupiOver || icon != 214) && (icon != 324 || m_time / 4 % 20 < 18))
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

int CDecor::IsWorld(POINT pos)
{
	pos.x += 30;
	pos.y += 30;
	if (pos.x < 0 || pos.x >= DIMOBJX * MAXCELX || pos.y < 0 || pos.y >= DIMOBJY * MAXCELY)
	{
		return -1;
	}
	int icon = m_decor[pos.x / DIMOBJX][pos.y / DIMOBJY].icon;
	if (icon >= Object::World_1 && icon <= Object::World_8)
	{
		return icon - Object::World_1 + 1;
	}
	if (icon >= Object::WorldDone_1 && icon <= Object::WorldDone_8)
	{
		return icon - Object::WorldDone_1 + 1;
	}
	if (icon == Object::World_9 || icon == Object::WorldDone_9)
	{
		return 9;
	}
	if (icon >= Object::World_10 && icon <= Object::World_14)
	{
		return icon - Object::World_10 + 10;
	}
	if (icon >= Object::WorldDone_10 && icon <= Object::WorldDone_14)
	{
		return icon - Object::WorldDone_10 + 10;
	}
	if (icon >= Object::Level_1 && icon <= Object::Level_8)
	{
		return icon - Object::Level_1 + 1;
	}
	if (icon == Object::WorldBack)
	{
		return 199;
	}
	return -1;
}

char CDecor::GetTypeBarre(POINT pos)
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
	if (IsPassIcon(icon))
	{
		return 2;
	}
	RECT rect;
	rect.left = pos2.x + 12;
	rect.right = pos2.x + 48;
	rect.top = pos2.y + 58;
	rect.bottom = pos2.y + 59;
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

BOOL CDecor::IsSurfWater(POINT pos)
{
	if (pos.y % 64 < 64 - BLUPISURF)
	{
		return FALSE;
	}
	int icon = m_decor[(pos.x + 30) / DIMOBJX][pos.y / DIMOBJY].icon;
	int icon2 = m_decor[(pos.x + 30) / DIMOBJX][(pos.y + BLUPISURF) / DIMOBJY].icon;
	return icon != 92 && icon2 == 92;
}

BOOL CDecor::IsDeepWater(POINT pos)
{
	int y = pos.y / DIMOBJY;
	int x = (pos.x + 30) / DIMOBJX;
	int icon = m_decor[x][y].icon;
	return icon == 91 || icon == 92;
}

BOOL CDecor::IsOutWater(POINT pos)
{
	int icon = m_decor[(pos.x + DIMBLUPIX / 2) / DIMOBJX][(pos.y + DIMBLUPIY / 2) / DIMOBJY].icon;
	return icon != 91 && icon != 92 && IsPassIcon(icon);
}

BOOL CDecor::IsPassIcon(int icon)
{
	if (icon >= 0 && icon < MAXQUART)
	{
		for (int i = 0; i < 16; i++)
		{
			if (table_decor_quart[icon * 16 + i] != 0) return FALSE;
		}
	}
	return TRUE;
}

BOOL CDecor::IsBlocIcon(int icon)
{
	if (icon < 0 || icon >= MAXQUART) return FALSE;
	for (int i = 0; i < 16; i++)
	{
		if (table_decor_quart[icon * 16 + i] == 0) return FALSE;
	}
	return TRUE;
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
		cel += dir;
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
	int num = 0;
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
	do
	{
		pos.x += tinyPoint.x;
		pos.y += tinyPoint.y;
		if (num != m_decor[pos.x / DIMOBJX][pos.y / DIMOBJY].icon)
		{
			break;
		}
		ModifDecor(pos, -1);
	} while (pos.x >= 0 && pos.x < 6400 && pos.y >= 0 && pos.y < 6400);
	return TRUE;
}
