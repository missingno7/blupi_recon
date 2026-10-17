// DecBlupi.cpp
//

#include <misc.h>
#include <decor.h>
#include <text.h> // debug
//#include "resource.h"
#include <obstacle.h>
#include <dectables.h>


// handle Blupi's movement and current state.
void CDecor::BlupiStep()
{
	char str[100];
	BOOL bNear;
	int icon;
	POINT tinyPoint;
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
		rect.left = m_blupiPos.x + 12;
		rect.right = m_blupiPos.x + DIMBLUPIX - 12;
		rect.top = m_blupiPos.y + 11;
		rect.bottom = m_blupiPos.y + DIMBLUPIY - 2;
		TestPath(rect, m_blupiPos, &end);
	}
	m_blupiVent = FALSE;
	if (m_blupiTransport == -1 && !m_blupiJeep && !m_blupiTank && !m_blupiSkate
		&& m_blupiFocus)
	{
		tinyPoint.y = (end.y + DIMBLUPIY / 2) / DIMOBJY;
		tinyPoint.x = (end.x + DIMBLUPIX / 2) / DIMOBJX;
		num = m_decor[tinyPoint.x][tinyPoint.y].icon;
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
		rect.left = end.x + 12; rect.right = end.x + DIMBLUPIX - 12;
		rect.top = end.y + DIMBLUPIY - 2;
		rect.bottom = end.y + DIMBLUPIY - 1;
		bAirStart = !DecorDetect(rect);
	}
	else
	{
		bAirStart = FALSE;
	}
	rect.left = end.x + 12; rect.right = end.x + DIMBLUPIX - 12;
	rect.top = end.y + 10;
	rect.bottom = end.y + 20;
	icon = DecorDetect(rect);
	if (!m_blupiAir &&
		!m_blupiHelico && !m_blupiJeep && !m_blupiTank &&
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
		if (m_blupiHelico && !m_blupiShield && !m_bSuperBlupi)
		{
			m_blupiHelico = FALSE;
			tinyPoint.x = end.x - 34;
			tinyPoint.y = end.y - 34;
			ObjectStart(tinyPoint, TYPE_EXPLO2, 0, TRUE);
			m_decorAction = 1;
			m_decorPhase = 0;
			StopVehicleSound();
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
			StopVehicleSound();
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
			StopVehicleSound();
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
			StopVehicleSound();
			PlaySound(SOUND_BOUM, m_blupiPos, FALSE);
		}
		if (m_blupiFocus && m_blupiAction != ACTION_CLEAR1)
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
		!m_blupiHelico && !m_blupiJeep && !m_blupiTank &&
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
			if (m_blupiVitesse.y <= -14 && m_blupiAction != ACTION_CLEAR1 && !m_blupiSkate)
			{
				m_blupiJumpAie = TRUE;
				PlaySound(SOUND_JUMPAIE, end, FALSE);
			}
			else
			{
			PlaySound(SOUND_JUMPTOC, end, FALSE);
			}
			m_blupiVitesse.y = 1;
		}
		end.y += m_blupiVitesse.y * 2;
		if (m_blupiVitesse.y < 20)
		{
			m_blupiVitesse.y += 2;
		}
		rect.left = end.x + 12; rect.right = end.x + DIMBLUPIX - 12;
		rect.top = end.y + DIMBLUPIY - 30;
		rect.bottom = end.y + DIMBLUPIY - 1;
		if (m_blupiVitesse.y >= 0 && DecorDetect(rect))
		{
			end.y = end.y / 32 * 32 + BLUPIOFFY;
			if (!IsRessort(end))
			{
				PlaySound(SOUND_JUMPEND, end, FALSE);
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
		StopBlupi(TRUE);
	}
	if (m_blupiAction == ACTION_CHARGE && m_blupiPhase == 64)
	{
		StopBlupi(TRUE);
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
		if (m_blupiPhase == 14) StopBlupi(TRUE);
	}
	if ((m_blupiAction == ACTION_OUF1a || m_blupiAction == ACTION_OUF1b) && m_blupiPhase == 29)
	{
		StopBlupi(TRUE);
	}
	if (m_blupiAction == ACTION_OUF2 && m_blupiPhase == 32) StopBlupi(TRUE);
	if (m_blupiAction == ACTION_OUF3 && m_blupiPhase == 34) StopBlupi(TRUE);
	if (m_blupiAction == ACTION_OUF4 && m_blupiPhase == 40)
	{
		StopBlupi(TRUE);
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
		if (m_blupiPhase == 44) StopBlupi(TRUE);
	}
	if (m_blupiAction == ACTION_MOCKERY)
	{
		if (m_blupiPhase == 1)
		{
			PlaySound(SOUND_MOCKERY, m_blupiPos, FALSE);
			m_blupiTimeMockery = 200;
		}
		if (m_blupiPhase == 92) StopBlupi(TRUE);
	}
	if (m_blupiAction == ACTION_MOCKERYi)
	{
		if (m_blupiPhase == 6)
		{
			PlaySound(SOUND_MOCKERY, m_blupiPos, FALSE);
			m_blupiTimeMockery = 200;
		}
		if (m_blupiPhase == 104) StopBlupi(TRUE);
	}
	if (m_blupiAction == ACTION_STOPMARCH && m_blupiPhase == 3) StopBlupi();
	if (m_blupiAction == ACTION_STOPJUMP && m_blupiPhase == 5) StopBlupi();
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
		StopBlupi(TRUE);
		m_blupiPower = TRUE;
		m_blupiTimeShield = 100;
		m_blupiPosMagic = m_blupiPos;
		m_jauges[JAUGE_POWER].SetHide(FALSE);
		PlaySound(SOUND_STARTPOWER, end, FALSE);
	}
	if (m_blupiAction == ACTION_DRINK && m_blupiPhase == 36)
	{
		StopBlupi(TRUE);
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
		!m_blupiAir && !m_blupiHelico && !m_blupiJeep && !m_blupiTank && !m_blupiSkate &&
		!m_blupiNage && !m_blupiSurf && !m_blupiSuspend &&
		m_blupiFocus)
	{
		m_blupiAction = ACTION_UP;
		m_blupiPhase = 0;
	}

	if (!(m_keyPress & KEY_UP) && m_lastKeyPress & KEY_UP &&
		!m_blupiAir && !m_blupiHelico && !m_blupiJeep && !m_blupiTank && !m_blupiSkate &&
		!m_blupiNage && !m_blupiSurf && !m_blupiSuspend && m_blupiFocus)
	{
		StopBlupi();
	}

	if (m_keyPress & KEY_DOWN &&
		!(m_lastKeyPress & KEY_DOWN) &&
		m_blupiAction != ACTION_TURN && m_blupiAction != ACTION_JUMP &&
		m_blupiAction != ACTION_AIR && m_blupiAction != ACTION_DOWN &&
		m_blupiAction != ACTION_STOPPOP && m_blupiAction != ACTION_VERTIGO &&
		m_blupiAction != ACTION_ADVANCE && m_blupiAction != ACTION_RECEDE &&
		!m_blupiAir && !m_blupiHelico && !m_blupiJeep && !m_blupiTank && !m_blupiSkate &&
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
		!m_blupiAir && !m_blupiHelico && !m_blupiJeep && !m_blupiTank && !m_blupiSkate &&
		!m_blupiNage && !m_blupiSurf && !m_blupiSuspend &&
		m_blupiFocus)
	{
		m_blupiAction = ACTION_DOWN;
		m_blupiPhase = 0;
	}
	if (!(m_keyPress & KEY_DOWN) && m_lastKeyPress & KEY_DOWN &&
		!m_blupiAir && !m_blupiHelico && !m_blupiJeep && !m_blupiTank && !m_blupiSkate &&
		!m_blupiNage && !m_blupiSurf && !m_blupiSuspend && m_blupiFocus)
	{
		StopBlupi();
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
	if (!m_blupiHelico && !m_blupiJeep && !m_blupiTank && !m_blupiSkate &&
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
			!m_blupiHelico && !m_blupiJeep && !m_blupiTank && !m_blupiSkate &&
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
			!m_blupiHelico && !m_blupiJeep && !m_blupiTank && !m_blupiSkate &&
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
		if (m_blupiAction == ACTION_TURN && m_blupiPhase == 7)
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

	if ((!(m_keyPress & ~KEY_FIRE) || ((m_keyPress & KEY_LEFT) && (m_keyPress & KEY_RIGHT))) &&
		!m_blupiHelico && !m_blupiJeep && !m_blupiTank && !m_blupiSkate &&
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
	if (!m_blupiJeep &&
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
			StopBlupi();
		}
	if (!(m_keyPress & ~KEY_FIRE) && m_blupiSurf &&
			m_blupiFocus &&
			m_blupiAction == ACTION_MARCH)
		{
			StopBlupi();
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
				StopBlupi();
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

		}
		else if ((m_keyPress & KEY_RIGHT) && !(m_keyPress & KEY_LEFT))
		{
			if (m_blupiVitesse.x < 16)
			{
				m_blupiVitesse.x += 1;
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
				StopBlupi();
				m_blupiPosHelico = m_blupiPos;
				m_blupiFocus = TRUE;
				StopSound(SOUND_HELICOHIGH);
				StopSound(SOUND_HELICOLOW);
				PlaySound(SOUND_HELICOSTOP, m_blupiPos, FALSE);
			}
		}
	}
	





















































































































	



































































	









































































	if (m_blupiJeep && m_blupiFocus)
	{
		if (m_blupiVitesse.x == 0 && m_blupiAction == 2)
		{
			StopBlupi();
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
			StopBlupi();
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
			StopBlupi();
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
			StopBlupi();
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
			StopBlupi();
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
		if ( m_keyPress & (KEY_UP | KEY_DOWN) && !bAirStart && m_blupiTransport == -1 && m_blupiVitesse.x < 8)
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
			ObjectStart(tinyPoint, TYPE_SKATE, 0, TRUE);
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
		tinyPoint.x = end.x;
		tinyPoint.y = end.y / 64 * 64 + BLUPIOFFY;
		rect.left = tinyPoint.x + 12; rect.right = tinyPoint.x + DIMBLUPIX - 12; rect.top = tinyPoint.y + 11; rect.bottom = tinyPoint.y + DIMBLUPIY - 2;
			if (!DecorDetect(rect, TRUE))
			{
				m_blupiSuspend = FALSE;
				StopBlupi();
		end = tinyPoint;
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
			m_blupiActionOuf = 65;
			m_blupiNoBarre = 5;
			m_blupiTimeOuf = 0;
		}
		if ((m_keyPress & KEY_JUMP || m_keyPress & KEY_UP) && m_blupiAction != ACTION_JUMP && m_blupiAction != ACTION_TURN)
		{
			m_blupiAction = 4;
			m_blupiPhase = 0;
		}
		if (!(m_keyPress & (KEY_JUMP | KEY_UP)) && m_blupiAction == 4)
		{
			StopBlupi();
		}
		if (m_blupiAction == 4 && m_blupiPhase == 10)
		{
			m_blupiSuspend = FALSE;
			m_blupiAir = TRUE;
			m_blupiAction = ACTION_AIR;
			end.y -= 2;
			m_blupiNoBarre = 5;
			m_blupiPhase = 0;
			m_blupiVitesse.y = -11;
			PlaySound(SOUND_SUSPENDJUMP, end, FALSE);
		}
	}
	if (m_keyPress & KEY_FIRE &&
		!m_blupiHelico &&
		!m_blupiTank &&
		!m_blupiJeep &&
		!m_blupiSkate &&
		!bAirStart &&
		m_blupiTransport == -1 &&
		m_blupiFocus)
	{
		if (m_blupiPerso == 0)
		{
			m_blupiAction = ACTION_OUF3;
			m_blupiPhase = 0;
			PlaySound(SOUND_FIREKO, m_blupiPos, FALSE);
		}
		else
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
	}
	if (!m_blupiNage && !m_blupiSurf)
	{
		rect.left = m_blupiPos.x + 12; rect.right = m_blupiPos.x + DIMBLUPIX - 12;
		rect.top = m_blupiPos.y + 11; rect.bottom = m_blupiPos.y + DIMBLUPIY - 2;
	}
	else
	{
		rect.left = m_blupiPos.x + 12; rect.right = m_blupiPos.x + DIMBLUPIX - 12;
		rect.top = m_blupiPos.y + 15; rect.bottom = m_blupiPos.y + DIMBLUPIY - 10;
	}
	POINT blupiPos = end;
	TestPath(rect, m_blupiPos, &end);
	if (bLastBlupiAir && m_blupiPos.x == end.x && m_blupiPos.x != blupiPos.x)
	{
		end.y = blupiPos.y;
		TestPath(rect, m_blupiPos, &end);
	}
	if (m_blupiVent && m_blupiPos.y == end.y && m_blupiPos.y != blupiPos.y)
	{
		end.x = blupiPos.x;
		TestPath(rect, m_blupiPos, &end);
	}
	if (m_blupiTransport != -1 && m_blupiPos.x == end.x && m_blupiPos.x != blupiPos.x)
	{
		end.y = blupiPos.y;
		TestPath(rect, m_blupiPos, &end);
	}
	if (m_blupiHelico || m_blupiJeep || m_blupiTank || m_blupiSkate || m_blupiNage)
	{
		if (m_blupiPos.x == end.x && m_blupiPos.x != blupiPos.x)
		{
			end.y = blupiPos.y;
			TestPath(rect, m_blupiPos, &end);
		}
		else if (m_blupiPos.y == end.y && m_blupiPos.y != blupiPos.y)
		{
			end.x = blupiPos.x;
			TestPath(rect, m_blupiPos, &end);
		}
	}
	blupiPos = m_blupiPos;
	m_blupiPos = end;
	if ((m_blupiAction == ACTION_STOP ||
		m_blupiAction == ACTION_STOPMARCH ||
		m_blupiAction == ACTION_UP ||
		m_blupiAction == ACTION_DOWN) &&
		!m_blupiAir &&
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
			if (m_blupiHelico)
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
			if (m_blupiHelico)
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
			(bIsOpen || bIsOpen2))
		{
			if (!m_blupiHelico)
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
			rect.left = end.x + 20;
			rect.right = end.x + 22;
			rect.top = end.y + DIMBLUPIY - 2;
			rect.bottom = end.y + DIMBLUPIY;
			bIsOpen = !DecorDetect(rect);
			rect.left = end.x + DIMBLUPIX - 22;
			rect.right = end.x + DIMBLUPIX - 20;
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
			rect.left = end.x + 20;
			rect.right = end.x + 22;
			rect.top = end.y + DIMBLUPIY - 2;
			rect.bottom = end.y + DIMBLUPIY;
			bIsOpen = !DecorDetect(rect);
			rect.left = end.x + DIMBLUPIX - 22;
			rect.right = end.x + DIMBLUPIX - 20;
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
	

























	
















	





















	if ((m_blupiPower || m_blupiShield) && abs(m_blupiPos.x - m_blupiPosMagic.x) + abs(m_blupiPos.y - m_blupiPosMagic.y) >= 40)
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
	



	if (m_blupiHelico && m_blupiPos.y > 2 && m_blupiFocus && !m_blupiShield && !m_bSuperBlupi)
	{
		rect.left = end.x + 12; rect.right = end.x + DIMBLUPIX - 12;
		rect.top = end.y + 4;
		rect.bottom = end.y + 20;
		if (DecorDetect(rect))
		{
			StopBlupi();
			m_blupiHelico = FALSE;
			tinyPoint.x = m_blupiPos.x - 34;
			tinyPoint.y = m_blupiPos.y - 34;
			ObjectStart(tinyPoint, TYPE_EXPLO2, 0, TRUE);
			m_decorAction = 1;
			m_decorPhase = 0;
			StopVehicleSound();
			PlaySound(SOUND_BOUM, m_blupiPos, FALSE);
		}
	}
	if (!m_blupiHelico && !m_blupiJeep && !m_blupiTank && !m_blupiSkate && m_blupiFocus)
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
			PlaySound(SOUND_SURF, m_blupiPos, FALSE);
			m_jauges[JAUGE_AIR].SetHide(TRUE);
		}
		tinyPoint.x = m_blupiPos.x;
		tinyPoint.y = m_blupiPos.y - DIMBLUPIY;
		if ((m_blupiSurf || m_blupiNage) && (m_blupiPos.y % DIMOBJY == DIMOBJY - BLUPISURF || m_blupiPos.y % DIMOBJY == DIMOBJY / 2) && IsOutWater(tinyPoint) && m_keyPress & KEY_JUMP)
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
				StopBlupi();
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
	if ((m_blupiHelico || m_blupiJeep || m_blupiTank || m_blupiSkate) && m_blupiFocus && (IsSurfWater(m_blupiPos) || IsDeepWater(m_blupiPos)))
	{

		StopBlupi();
		m_blupiHelico = FALSE;
		m_blupiJeep = FALSE;
		m_blupiTank = FALSE;
		m_blupiSkate = FALSE;
		tinyPoint.x = m_blupiPos.x - 34;
		tinyPoint.y = m_blupiPos.y - 34;
		ObjectStart(tinyPoint, TYPE_EXPLO2, 0, TRUE);
		m_decorAction = 1;
		m_decorPhase = 0;
		StopVehicleSound();
		PlaySound(SOUND_BOUM, m_blupiPos, FALSE);
	}
	if (m_blupiFocus && !m_blupiSuspend && !m_blupiHelico && !m_blupiJeep && !m_blupiTank && !m_blupiSkate && !m_blupiNage && !m_blupiSurf && m_blupiNoBarre == 0 && GetTypeBarre(m_blupiPos) == 1)
	{
		tinyPoint.x = m_blupiPos.x;
		tinyPoint.y = (m_blupiPos.y + 22) / DIMOBJY * DIMOBJY + BLUPISUSPEND;
		rect.left = tinyPoint.x + 12; rect.right = tinyPoint.x + DIMBLUPIX - 12; rect.top = tinyPoint.y + 11; rect.bottom = tinyPoint.y + DIMBLUPIY - 2;
		if (!DecorDetect(rect, TRUE))
		{
			m_blupiPos = tinyPoint;
			m_blupiSuspend = TRUE;
			m_blupiAir = FALSE;
			StopBlupi();
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
			m_blupiAction = ACTION_CLEAR1;
			m_blupiPhase = 0;
			m_blupiFocus = FALSE;
			m_blupiHelico = FALSE;
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
			m_blupiRestart = FALSE;
			m_blupiActionOuf = 0;
			m_jauges[JAUGE_AIR].SetHide(TRUE);
			m_jauges[JAUGE_POWER].SetHide(TRUE);
		}
		tinyPoint.x = m_blupiPos.x - 34;
		tinyPoint.y = m_blupiPos.y - 34;
		ObjectStart(tinyPoint, TYPE_EXPLO4, 0, TRUE);
		m_decorAction = 2;
		m_decorPhase = 0;
		StopVehicleSound();
		PlaySound(SOUND_BOUM, m_blupiPos, FALSE);
	}
	if (m_blupiAction != ACTION_BYE && m_blupiFocus)
	{
		num = IsWorld(m_blupiPos);
		if (num != -1)
		{
			StopVehicleSound();
			PlaySound(SOUND_BYE, m_blupiPos, FALSE);
			m_blupiAction = ACTION_BYE;
			m_blupiPhase = 0;
			m_blupiFocus = FALSE;
			m_blupiFront = TRUE;
		}
	}
	if (m_blupiAction != ACTION_CLEAR1)
	{
		if (IsLave(m_blupiPos) && !m_blupiShield && !m_bSuperBlupi)
		{
			m_blupiAction = ACTION_CLEAR1;
			m_blupiPhase = 0;
			m_blupiFocus = FALSE;
			m_blupiHelico = FALSE;
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
			m_blupiAir = FALSE;
			m_blupiPos.y = m_blupiPos.y / DIMOBJY * DIMOBJY + BLUPIOFFY;
			m_blupiRestart = TRUE;
			m_blupiActionOuf = 0;
			m_jauges[JAUGE_AIR].SetHide(TRUE);
			m_jauges[JAUGE_POWER].SetHide(TRUE);
			StopVehicleSound();
			PlaySound(SOUND_FALL, m_blupiPos, FALSE);
		}
	}
	POINT tinyPoint5;
	























































































if (!m_blupiHelico && !m_blupiSkate && !m_blupiJeep && !m_blupiTank && m_blupiFocus)
	{
		num = MockeryDetect(m_blupiPos);
		if (num != 0)
		{
			m_blupiActionOuf = num;
			m_blupiTimeOuf = 0;
		}
	}
	num = MoveObjectDetect(m_blupiPos, &bNear);
	if (num != -1 && !bNear && m_moveObject[num].type == TYPE_BOMBEDOWN && !m_blupiHelico && !m_blupiJeep && !m_blupiTank && !m_blupiSkate && !m_blupiNage && !m_blupiSurf && !m_blupiSuspend && !m_blupiShield && !m_bSuperBlupi && m_blupiFocus)
	{
		m_blupiActionOuf = 48;
		m_blupiTimeOuf = 0;
	}
	if (num != -1 && bNear)
	{
		if (m_moveObject[num].type == TYPE_HELICO &&
			!m_blupiHelico && !m_blupiJeep && !m_blupiTank && !m_blupiSkate &&
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
		





















		if (m_moveObject[num].type == TYPE_JEEP &&
			!m_blupiHelico && !m_blupiJeep && !m_blupiTank && !m_blupiSkate &&
			!m_blupiNage &&
			!m_blupiSurf &&
			!m_blupiSuspend &&
			m_blupiFocus && m_blupiPosHelico.x == -1)
		{
			ObjectDelete(m_moveObject[num].posCurrent, m_moveObject[num].type);
			m_scrollAdd.x = 0;
			m_scrollAdd.y = 0;
			StopBlupi();
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
			!m_blupiHelico && !m_blupiJeep && !m_blupiTank && !m_blupiSkate &&
			!m_blupiNage &&
			!m_blupiSurf &&
			!m_blupiSuspend &&
			m_blupiFocus && m_blupiPosHelico.x == -1)
		{

			ObjectDelete(m_moveObject[num].posCurrent, m_moveObject[num].type);
			m_scrollAdd.x = 0;
			m_scrollAdd.y = 0;
			StopBlupi();
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
		tinyPoint5.x = 450;
		tinyPoint5.y = 430;
		VoyageInit(tinyPoint, tinyPoint5, 177, 10);
			m_blupiBullet += 10;
			if (m_blupiBullet > 10)
			{
				m_blupiBullet = 10;
			}
		}
		if ( m_moveObject[num].type == TYPE_SKATE &&
			!m_blupiHelico && !m_blupiJeep && !m_blupiTank && !m_blupiSkate &&
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
		



















if ((m_moveObject[num].type == TYPE_BOMBEDOWN ||
			m_moveObject[num].type == TYPE_BOMBEUP ||
			m_moveObject[num].type == TYPE_BOMBEMOVE ||
			m_moveObject[num].type == TYPE_BULLDOZER ||
			m_moveObject[num].type == TYPE_POISSON ||
			m_moveObject[num].type == TYPE_OISEAU) &&
			!m_blupiShield &&
			!m_bSuperBlupi)
		{
			if (!m_blupiJeep &&
				!m_blupiTank &&
				!m_blupiSkate &&
				(m_blupiFocus || m_blupiAction == ACTION_AIR || m_blupiAction == ACTION_JUMPAIE))
			{
				if (m_blupiHelico)
				{
					m_blupiAir = TRUE;
				}
				m_blupiAction = ACTION_CLEAR1;
			m_blupiPhase = 0;
			m_blupiFocus = FALSE;
			m_blupiHelico = FALSE;
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
			m_blupiRestart = FALSE;
			m_blupiActionOuf = 0;
			m_jauges[JAUGE_AIR].SetHide(TRUE);
			m_jauges[JAUGE_POWER].SetHide(TRUE);
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
				StopVehicleSound();
			}
			PlaySound(SOUND_BOUM, m_moveObject[num].posCurrent, FALSE);
		}
		






































		











































		if (m_moveObject[num].type == TYPE_BALLE &&
			!m_blupiShield &&
			!m_bSuperBlupi &&
			m_blupiAction != ACTION_WIN &&
			m_blupiAction != ACTION_BYE &&
			m_blupiAction != ACTION_CLEAR1 &&
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
			if (!m_blupiHelico && !m_blupiJeep && !m_blupiTank && !m_blupiSkate)
			{
				PlaySound(SOUND_GLU, m_moveObject[num].posCurrent, FALSE);
			}
			else
			{
				m_blupiHelico = FALSE;
				m_blupiJeep = FALSE;
				m_blupiTank = FALSE;
				m_blupiSkate = FALSE;
				tinyPoint = m_moveObject[num].posCurrent;
				tinyPoint.x -= 34;
				tinyPoint.y -= 34;
				ObjectStart(tinyPoint, TYPE_EXPLO3, 0, TRUE);
				StopVehicleSound();
				PlaySound(SOUND_BOUM, m_moveObject[num].posCurrent, FALSE);
				m_decorAction = 1;
				m_decorPhase = 0;
			}
			m_blupiCloud = FALSE;
			m_blupiHide = FALSE;
			m_jauges[JAUGE_POWER].SetHide(TRUE);
		}
		if (m_moveObject[num].type == TYPE_TRESOR)
		{
			ObjectDelete(m_moveObject[num].posCurrent, m_moveObject[num].type);
			if (m_mission == 10) { // BUG: VoyageInit is skipped in custom levels with id 10. To fix, add `&& !m_bPrivate`
				m_nbTresor++;
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
			!m_blupiHelico && !m_blupiJeep && !m_blupiTank && !m_blupiSkate &&
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
		












		if ( m_moveObject[num].type == TYPE_DRINK &&
			!m_blupiShield && !m_blupiPower &&
			!m_blupiCloud && !m_blupiHide &&
			!m_blupiHelico && !m_blupiJeep && !m_blupiTank && !m_blupiSkate &&
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
			!m_blupiHelico && !m_blupiJeep && !m_blupiTank && !m_blupiSkate &&
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
			tinyPoint5.x = 520;
			tinyPoint5.y = 438;
			VoyageInit(tinyPoint, tinyPoint5, GetIconPerso(), CHBUTTON);
				}
			}
			else if (!m_blupiShield && !m_bSuperBlupi)
			{
				ObjectDelete(m_moveObject[num].posCurrent, m_moveObject[num].type);
				m_blupiAction = ACTION_CLEAR1;
			m_blupiPhase = 0;
			m_blupiFocus = FALSE;
			m_blupiHelico = FALSE;
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
			m_blupiRestart = FALSE;
			m_blupiActionOuf = 0;
			m_jauges[JAUGE_AIR].SetHide(TRUE);
			m_jauges[JAUGE_POWER].SetHide(TRUE);
				tinyPoint = m_moveObject[num].posCurrent;
				tinyPoint.x -= 34;
				tinyPoint.y -= 34;
				ObjectStart(tinyPoint, TYPE_EXPLO3, 0, TRUE);
				StopVehicleSound();
				PlaySound(SOUND_BOUM, m_moveObject[num].posCurrent, FALSE);
				m_decorAction = 1;
				m_decorPhase = 0;
			}
		}
		if (m_moveObject[num].type == 12 &&
			!m_blupiHelico && !m_blupiJeep && !m_blupiTank && !m_blupiSkate &&
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
					StopVehicleSound();
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
		if (m_blupiHelico || m_blupiJeep || m_blupiTank || m_blupiSkate)
		{
			if (m_blupiHelico)
			{
				tinyPoint = m_blupiPos;
				tinyPoint.y -= 2;
				ObjectStart(tinyPoint, TYPE_HELICO, 0, TRUE);
			}
			if (m_blupiTransport == -1)
			{
				if (m_blupiJeep) { POINT pos = {m_blupiPos.x, m_blupiPos.y}; ObjectStart(pos, TYPE_JEEP, 0, TRUE); }
				if (m_blupiTank) { POINT pos = {m_blupiPos.x, m_blupiPos.y}; ObjectStart(pos, TYPE_TANK, 0, TRUE); }
				if (m_blupiSkate)
				{
					tinyPoint = m_blupiPos;
					tinyPoint.y--;
					ObjectStart(tinyPoint, TYPE_SKATE, 0, TRUE);
				}
			}
			m_blupiHelico = FALSE;
			m_blupiJeep = FALSE;
			m_blupiTank = FALSE;
			m_blupiSkate = FALSE;
			StopVehicleSound();
		}
		POINT pos = m_blupiPos;
		PlaySound(SOUND_ELECTRO, pos, FALSE);
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
	if (!m_blupiHelico && !m_blupiJeep && !m_blupiTank && !m_blupiSkate &&
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
			m_blupiActionOuf == ACTION_MOCKERYi)
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
	//// winphone ?
	//if (m_keyPress & KEY_FIRE && m_blupiAction == ACTION_STOP)
	//{
	//	m_blupiAction = ACTION_NON;
	//	m_blupiPhase = 0;
	//	PlaySound(SOUND_ERROR, m_blupiPos, FALSE);
	//}
	if ((m_blupiAction == ACTION_CLEAR1 && m_blupiPhase == 70) ||
		(m_blupiAction == ACTION_DROWN && m_blupiPhase == 90) ||
		(m_blupiAction == ACTION_GLU && m_blupiPhase == 90) ||
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

			StopVehicleSound();
			PlaySound(14, m_moveObject[num].posCurrent, FALSE);
			m_blupiAction = ACTION_WIN;
			m_blupiPhase = 0;
			m_blupiFocus = FALSE;
			m_blupiFront = TRUE;
			m_blupiAir = FALSE;
			m_blupiHelico = FALSE;
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
			m_term = 99;
		}
		else if (m_mission == 99)
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
			else if (num == 99)
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
	if (m_blupiFocus &&
		!m_blupiAir &&
		!m_blupiHelico &&
		!m_blupiShield &&
		!bIsOpen &&
		!bIsOpen2 &&
		m_blupiTransport == -1)
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
	if (m_phase == WM_PHASE_PLAY || m_phase == WM_PHASE_PLAYTEST)
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
	if (m_blupiHelico || m_blupiNage || m_blupiSurf)
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
				if (icon >= 0 && icon < MAXQUART && (!m_blupiHelico || icon != 214))
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
	int offset = 0;
	int speed;
	if (m_blupiAction == ACTION_TURN) return;
	if (m_blupiHelico)
	{
		if (m_blupiVitesse.y < -5)
		{
			if (m_time % 20 != 0 && m_time % 20 != 2 && m_time % 20 != 5 &&
				m_time % 20 != 8 && m_time % 20 != 10 && m_time % 20 != 11 &&
				m_time % 20 != 16 && m_time % 20 != 18) return;
		}
		else if (m_blupiVitesse.x == 0)
		{
			if (m_time % 50 != 0 && m_time % 50 != 12 && m_time % 50 != 30) return;
		}
		else
		{
			if (m_time % 20 != 0 && m_time % 20 != 3 && m_time % 20 != 5 &&
				m_time % 20 != 11 && m_time % 20 != 15) return;
		}
		offset = 22;
	}
	if (m_blupiJeep)
	{
		if (m_blupiVitesse.x == 0)
		{
			if (m_blupiPhase % 50 != 0 && m_blupiPhase % 50 != 12 &&
				m_blupiPhase % 50 != 20 && m_blupiPhase % 50 != 35) return;
		}
		else
		{
			if (m_blupiPhase % 20 != 0 && m_blupiPhase % 20 != 3 &&
				m_blupiPhase % 20 != 5 && m_blupiPhase % 20 != 11 &&
				m_blupiPhase % 20 != 15) return;
		}
		offset = 32;
	}
	if (m_blupiTank)
	{
		if (m_blupiVitesse.x == 0)
		{
			if (m_blupiPhase % 50 != 0 && m_blupiPhase % 50 != 15 &&
				m_blupiPhase % 50 != 28) return;
		}
		else
		{
			if (m_blupiPhase % 20 != 0 && m_blupiPhase % 20 != 4 &&
				m_blupiPhase % 20 != 12) return;
		}
		offset = 35;
	}
	if (offset != 0)
	{
		if (m_blupiDir == DIR_RIGHT)
		{
			pos.x += 5 - offset;
			speed = -20;
		}
		else
		{
			pos.x += offset;
			speed = 20;
		}
		ObjectStart(pos, TYPE_POLLUTION, speed, TRUE);
	}
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
	if (icon >= 174 && icon <= 181) return icon - 173;
	if (icon == 184) return 99;
	return -1;
}

int CDecor::GetTypeBarre(POINT pos)
{
	POINT pos2 = pos;
	pos.x += 30;
	pos.y += 22;
	if (pos.y % 64 > 44) return 0;
	if (pos.x < 0 || pos.x >= 6400 || pos.y < 0 || pos.y >= 6400) return 0;
	int icon = m_decor[pos.x / DIMOBJX][pos.y / DIMOBJY].icon;
	if (icon != 138 && icon != 202) return 0;
	if (pos.y >= 6336) return 1;
	icon = m_decor[pos.x / DIMOBJX][pos.y / DIMOBJY + 1].icon;
	if (!IsPassIcon(icon)) return 2;
	RECT rect;
	rect.left = pos2.x + 12;
	rect.right = pos2.x + 48;
	rect.top = pos2.y + 58;
	rect.bottom = pos2.y + 59;
	if (DecorDetect(rect, TRUE)) return 2;
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
	int y = pos.y / DIMOBJY;
	int x = (pos.x + 30) / DIMOBJX;
	int icon = m_decor[x][y].icon;
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
	cel.x = (pos.x + DIMOBJX / 2) / DIMOBJX;
	cel.y = (pos.y + DIMOBJY / 2) / DIMOBJY;
	while (IsValidCel(cel) && !IsBlocIcon(m_decor[cel.x][cel.y].icon))
	{
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

