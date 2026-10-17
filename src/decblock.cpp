#include <def.h>
#include "decor.h"
#include <misc.h>
#include <obstacle.h>
#include <dectables.h>
void CDecor::InitGamer()
{
	m_nbVies = 3;
	for (int i = 0; i < 100; i++) m_doors[i] = 1;
}

void CDecor::SetTime(int time)
{
	m_time = time;
}

int CDecor::GetTime()
{
	return m_time;
}

void CDecor::PlayPrepare(BOOL bTest)
{
	if (bTest) m_nbVies = 3;

	if (m_bMulti)
	{
		m_nbVies = 10;
		m_blupiPos = m_blupiStartPos[m_team];
		m_blupiDir = m_blupiStartDir[m_team];
	}
	else
	{
		m_blupiPos = m_blupiStartPos[0];
		m_blupiDir = m_blupiStartDir[0];
	}

	if (m_blupiDir == DIR_LEFT)
	{
		m_blupiIcon = 4;
	}
	else
	{
		m_blupiIcon = 0;
	}

	m_blupiAction = ACTION_STOP;
	m_blupiPhase = 0;
	m_blupiFocus = TRUE;
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
	m_blupiInvert = FALSE;
	m_blupiBalloon = FALSE;
	m_blupiEcrase = FALSE;
	m_blupiActionOuf = 0;
	m_blupiTimeNoAsc = 0;
	m_blupiTimeMockery = 0;
	m_blupiValidPos = m_blupiPos;
	m_blupiEnergyUnused = 100;
	m_blupiTimeFire = 0;
	m_blupiTimeMockery = 0;
	m_blupiTimeOuf = 0;
	m_blupiActionOuf = 0;
	m_nbTresor = 0;
	m_totalTresor = 0;
	for (int i = MAXMOVEOBJECT; i != 0; i--)
	{
		if (m_moveObject[i].type == TYPE_TRESOR)
		{
			m_totalTresor++;
		}
		m_moveObject[i].posCurrent = m_moveObject[i].posStart;
		m_moveObject[i].step = STEP_STOPSTART;
		m_moveObject[i].phase = 0;
		m_moveObject[i].time = 0;

		if (m_moveObject[i].type == TYPE_TRESOR ||
			m_moveObject[i].type == TYPE_EGG ||
			m_moveObject[i].type == TYPE_SHIELD ||
			m_moveObject[i].type == TYPE_POWER ||
			m_moveObject[i].type == TYPE_INVERT ||
			m_moveObject[i].type == TYPE_BOMBEDOWN ||
			m_moveObject[i].type == TYPE_BOMBEUP ||
			m_moveObject[i].type == TYPE_BOMBEFOLLOW1 ||
			m_moveObject[i].type == TYPE_BOMBEFOLLOW2)
		{
			m_moveObject[i].phase = rand() % 23;
		}

		if (m_moveObject[i].type == TYPE_BALLE)
		{
			m_moveObject[i].type = 0;
		}

		if (m_bMulti &&
			(m_moveObject[i].type == TYPE_CAISSE ||
			m_moveObject[i].type == TYPE_GOAL ||
			m_moveObject[i].type == TYPE_CLE ||
			m_moveObject[i].type == TYPE_BLUPIHELICO ||
			m_moveObject[i].type == TYPE_BLUPITANK))
		{
			m_moveObject[i].type = 0;
		}
	}
	m_goalPhase = 0;
	MoveObjectSort();
	UpdateCaisse();
	m_scrollPoint = m_blupiPos;
	m_scrollAdd = POINT( 0, 0 );
	m_blupiPosHelico.x = -1;
	m_nbLinkCaisse = 0;
	m_bHelicoMarch = FALSE;
	m_bHelicoStop = FALSE;
	m_bJeepMarch = FALSE;
	m_bJeepStop = FALSE;
	m_blupiTimeShield = FALSE;
	m_blupiNoBarre = 0;
	m_blupiValidPos = m_blupiPos;
	m_blupiFifoNb = 0;
	m_blupiTimeFire = 0;
	NetDataFlush();
	NotifFlush();
	m_voyageIcon = -1;
	m_jauges[JAUGE_AIR].SetHide(TRUE);
	m_jauges[JAUGE_POWER].SetHide(TRUE);
	m_bFoundCle = FALSE;
	m_term = 0;
	m_time = 0;
	m_bPause = FALSE;
}

void CDecor::BuildPrepare()
{
	for (int i = 0; i < MAXMOVEOBJECT; i++)
	{
		m_moveObject[i].posCurrent = m_moveObject[i].posStart;
		m_moveObject[i].step = STEP_STOPSTART;
		m_moveObject[i].time = 0;
		m_moveObject[i].phase = 0;
		if (m_moveObject[i].type == TYPE_BALLE)
		{
			m_moveObject[i].type = 0;
		}
	}
	m_voyageIcon = -1;
	m_posCelHili.x = -1;
	m_liftEnd = -1;
	m_time = 0;
	m_bPause = FALSE;
	NetDataFlush();
}

int CDecor::IsTerminated()
{
	return m_term;
}

int CDecor::GetBlupiChannelActual()
{
	if (m_bMulti && m_blupiChannel == 2 && m_team > 0) {
		return m_team + CHBLUPI1 - 1;
	}
	else return m_blupiChannel;
}

int CDecor::GetBlupiChannelStandard()
{
	if (m_bMulti && m_team > 0) {
		return m_team + CHBLUPI1 - 1;
	}
	else return 2;
}

int CDecor::GetIconPerso()
{
	if (m_bMulti) return m_team + 0x6c;
	return 0x6c;
}

void CDecor::KeyChange(int keyPress)
{
	m_keyPress = keyPress;
}

void CDecor::SetJoystickEnable(BOOL bJoystick)
{
	m_bJoystick = bJoystick;
}

void CDecor::SetDemoPlay(BOOL param)
{
	m_bDemoPlay = param;
}

void CDecor::GetBlupiInfo(BOOL* bHelico, BOOL* bJeep, BOOL* bSkate, BOOL* bNage)
{
	*bHelico = m_blupiHelico;
	*bJeep = m_blupiJeep | m_blupiTank;
	*bSkate = m_blupiSkate;
	*bNage = m_blupiNage | m_blupiSurf;
}

void CDecor::PlaySound(int sound, POINT pos, BOOL bLocal)
{
	if (!bLocal) NetPlaySound(sound, pos);

	pos.x -= m_posDecor.x;
	pos.y -= m_posDecor.y;
	m_pSound->PlayImage(sound, pos, -1, bLocal);

	if (sound == SOUND_HELICOHIGH) m_bHelicoMarch = TRUE;
	if (sound == SOUND_HELICOLOW) m_bHelicoStop = TRUE;
	if (sound == SOUND_JEEPHIGH) m_bJeepMarch = TRUE;
	if (sound == SOUND_JEEPLOW) m_bJeepStop = TRUE;
}

void CDecor::AdaptMotorVehicleSound(POINT pos)
{
	POINT blupiPos = POINT(pos.x - m_posDecor.x, pos.y - m_posDecor.y);

	if (m_bHelicoMarch) m_pSound->Play(SOUND_HELICOHIGH, blupiPos.x, blupiPos.y);
	if (m_bHelicoStop) m_pSound->Play(SOUND_HELICOLOW, blupiPos.x, blupiPos.y);
	if (m_bJeepMarch)  m_pSound->Play(SOUND_JEEPHIGH, blupiPos.x, blupiPos.y);
	if (m_bJeepStop) m_pSound->Play(SOUND_JEEPLOW, blupiPos.x, blupiPos.y);
}

BOOL CDecor::BlitzActif(POINT cel)
{
	if (m_phase == WM_PHASE_BUILD) return !(m_time & 1);

	POINT pos = { cel.x * DIMOBJX, cel.y * DIMOBJY };

	int num = m_time % 100;

	if (m_blupiPos.x >= pos.x - 80 && m_blupiPos.x <= pos.x + 80 &&
		m_blupiPos.y >= pos.y - 500 && m_blupiPos.y <= pos.y + 500)
	{
		if (m_time % 100 < 70 && cel.y > 0 && m_decor[cel.x][cel.y - 1].icon == 0x130)
		{
			PlaySound(SOUND_BLITZ, pos, 0);
		}

		for (int i = 0; table_blitz[i] != -1; i++)
		{
			if (m_time % 100 == i) return TRUE;
		}
	}

	return FALSE;
}

int CDecor::SoundEnviron(int sound, int obstacle)
{
	if ((obstacle >= 32 && obstacle <= 34) || (obstacle >= 41 && obstacle <= 47) || (obstacle >= 139 && obstacle <= 143))
	{
		if (sound == SOUND_JUMPTOC)
		{
			return 79;
		}
		if (sound == SOUND_JUMPEND)
		{
			return 78;
		}
	}
	if ((obstacle >= 1 && obstacle <= 28) || (obstacle >= 78 && obstacle <= 90) || (obstacle >= 250 && obstacle <= 260) || (obstacle >= 311 && obstacle <= 316) || (obstacle >= 324 && obstacle <= 329))
	{
		if (sound == SOUND_JUMPTOC)
		{
			return 81;
		}
		if (sound == SOUND_JUMPEND)
		{
			return 80;
		}
	}
	if ((obstacle >= 284 && obstacle <= 303) || obstacle == 338)
	{
		if (sound == SOUND_JUMPTOC)
		{
			return 83;
		}
		if (sound == SOUND_JUMPEND)
		{
			return 82;
		}
	}
	if (obstacle >= 341 && obstacle <= 363)
	{
		if (sound == SOUND_JUMPTOC)
		{
			return 85;
		}
		if (sound == SOUND_JUMPEND)
		{
			return 84;
		}
	}
	if (obstacle >= 215 && obstacle <= 234)
	{
		if (sound == SOUND_JUMPTOC)
		{
			return 87;
		}
		if (sound == SOUND_JUMPEND)
		{
			return 86;
		}
	}
	if (obstacle >= 246 && obstacle <= 249)
	{
		if (sound == SOUND_JUMPTOC)
		{
			return 89;
		}
		if (sound == SOUND_JUMPEND)
		{
			return 88;
		}
	}
	if (obstacle >= 107 && obstacle <= 109)
	{
		if (sound == SOUND_JUMPTOC)
		{
			return 91;
		}
		if (sound == SOUND_JUMPEND)
		{
			return 90;
		}
	}
	return sound;
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

void CDecor::ActiveSwitch(BOOL bState, POINT cel)
{
	POINT pos;
	pos.x = cel.x * DIMOBJX;
	pos.y = cel.y * DIMOBJY;
	ModifDecor(pos, bState ? 384 : 385);
	PlaySound(bState ? 77 : 76, pos);
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
	RECT rect = BlupiRect(pos2);
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
	return pos.x >= 0 && pos.x < 6400 && pos.y >= 0 && pos.y < 6400 && m_decor[pos.x / DIMOBJX][pos.y / DIMOBJY].icon == 68;
}

BOOL CDecor::IsPiege(POINT pos)
{
	pos.x += DIMBLUPIX / 2;
	pos.y += DIMBLUPIY;
	return pos.x % 64 >= 15 && pos.x % 64 <= 49 && pos.x >= 0 && pos.x < 6400 && pos.y >= 0 && pos.y < 6400 && m_decor[pos.x / DIMOBJX][pos.y / DIMOBJY].icon == 373;
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
	return pos.x % 64 >= 4 && pos.x % 64 <= 60 && pos.x >= 0 && pos.x < 6400 && pos.y >= 0 && pos.y < 6400 && m_decor[pos.x / DIMOBJX][pos.y / DIMOBJY].icon == 378;
}

BOOL CDecor::IsSwitch(POINT pos, POINT *outCelSwitch)
{
	pos.x += DIMBLUPIX / 2;
	if (pos.x % DIMOBJX < 4 || pos.x % DIMOBJX > DIMOBJX - 4)
	{
		return FALSE;
	}
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
	return pos.x >= 0 && pos.x < 6400 && pos.y >= 0 && pos.y < 6400 && m_decor[pos.x / DIMOBJX][pos.y / DIMOBJY].icon == 317;
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
	return pos.x >= 0 && pos.x < 6400 && pos.y >= 0 && pos.y < 6400 && m_decor[pos.x / DIMOBJX][pos.y / DIMOBJY].icon == 211;
}

BOOL CDecor::IsTemp(POINT pos)
{
	pos.x += 30;
	pos.y += 60;
	return pos.x >= 0 && pos.x < 6400 && pos.y >= 0 && pos.y < 6400 && m_decor[pos.x / DIMOBJX][pos.y / DIMOBJY].icon == 324;
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
	POINT cel;
	cel.x = (pos.x + 30) / DIMOBJX;
	cel.y = pos.y / DIMOBJY;
	if (!IsValidCel(cel)) return FALSE;
	int icon = m_decor[cel.x][cel.y].icon;
	return icon == 91 || icon == 92;
}

BOOL CDecor::IsOutWater(POINT pos)
{
	int icon = m_decor[(pos.x + DIMBLUPIX / 2) / DIMOBJX][(pos.y + DIMBLUPIY / 2) / DIMOBJY].icon;
	return icon != 91 && icon != 92 && IsPassIcon(icon);
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
		return FALSE;
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

void CDecor::ModifDecor(POINT pos, int icon, BOOL bMulti)
{
	NetMessage message;
	m_decor[pos.x / DIMOBJX][pos.y / DIMOBJY].icon = icon;
	if (bMulti)
	{
		message.type = PK_MODIFDECOR;
		message.posX = pos.x;
		message.posY = pos.y;
		message.decorIcon = icon;
		NetMessagePush(&message);
	}
}

BOOL CDecor::IsRightBorder(int x, int y, int dx, int dy)
{
	int num = (m_dimDecor.x != 0) ? 100 : 10;
	int num2 = (m_dimDecor.y != 0) ? 100 : 8;
	if (x < 0 || x >= num || y < 0 || y >= num2)
	{
		return TRUE;
	}
	int icon = m_decor[x + dx][y + dy].icon;
	if (icon < 386 || icon > 397)
	{
		switch (icon)
		{
		case 245:
		case 400:
			break;
		case 250:
		case 251:
		case 252:
		case 253:
		case 254:
		case 255:
		case 256:
		case 257:
		case 258:
		case 259:
		case 260:
			icon = m_decor[x][y].icon;
			if (icon >= 250)
			{
				return icon <= 260;
			}
			return FALSE;
		default:
			icon = m_decor[x][y].icon;
			switch (icon)
			{
			case -1:
				return FALSE;
			case 32:
			case 33:
			case 34:
				return FALSE;
			case 68:
			case 92:
				return (dy != -1);
			case 317:
				return (dy != 1);
			case 19:
			case 21:
			case 25:
			case 26:
			case 28:
				return (dy != 1);
			case 27:
				return (dx == 0);
			case 45:
			case 46:
			case 47:
				return (dy != 1);
			default:
				switch (icon)
				{
				case 15:
				case 16:
				case 75:
				case 89:
				case 90:
				case 155:
					return (dx != -1 && dy != 1);
				case 17:
				case 18:
				case 74:
				case 87:
				case 88:
				case 154:
					return (dx != 1 && dy != 1);
				case 76:
				case 77:
				case 199:
				case 200:
					return (dx == 0);
				case 110:
				case 111:
				case 112:
				case 113:
				case 114:
				case 115:
				case 116:
				case 117:
				case 118:
				case 119:
				case 120:
				case 121:
				case 122:
				case 123:
				case 124:
				case 125:
					return FALSE;
				default:
					switch (icon)
					{
					case 126:
						return (dx == 1);
					case 129:
						return (dx == -1);
					case 132:
						return (dy == 1);
					case 135:
						return (dy == -1);
					case 139:
					case 140:
					case 141:
					case 142:
					case 143:
						return FALSE;
					default:
						switch (icon)
						{
						case 138:
							return (dy == -1);
						case 202:
							return FALSE;
						case 107:
						case 108:
						case 109:
						case 157:
							return (dx != 0);
						default:
							switch (icon)
							{
							case 309:
							case 310:
							case 410:
							case 411:
							case 412:
							case 413:
							case 414:
							case 415:
							case 416:
							case 417:
							case 418:
							case 419:
							case 420:
								break;
							default:
								switch (icon)
								{
								case 182:
								case 183:
									return (dy != 0);
								case 334:
								case 335:
								case 336:
									return (dy != 0);
								default:
									if (icon >= 250 && icon <= 260)
									{
										return FALSE;
									}
									if (icon >= 264 && icon <= 282)
									{
										return FALSE;
									}
									switch (icon)
									{
									case 378:
										return FALSE;
									case 404:
									case 410:
										return FALSE;
									case 421:
									case 422:
									case 423:
									case 424:
									case 425:
									case 426:
									case 427:
									case 428:
									case 429:
									case 430:
									case 431:
									case 432:
									case 433:
									case 434:
									case 435:
									case 436:
									case 437:
									case 438:
									case 439:
									case 440:
										return (dy != 0);
									default:
										return TRUE;
									}
								}
							}
							break;
						case 158:
						case 159:
						case 160:
						case 161:
						case 162:
						case 163:
						case 164:
						case 165:
						case 166:
						case 167:
						case 168:
						case 169:
						case 170:
						case 171:
						case 172:
						case 173:
						case 174:
						case 175:
						case 176:
						case 177:
						case 178:
						case 179:
						case 180:
						case 181:
							break;
						}
						return (dy == 1);
					}
				}
			}
		}
	}
	icon = m_decor[x][y].icon;
	if ((icon < 386 || icon > 397) && icon != 400)
	{
		return icon == 245;
	}
	return TRUE;
}


BOOL CDecor::IsFromage(POINT cel)
{
	if (!IsValidCel(cel)) return FALSE;

	int icon = m_decor[cel.x][cel.y].icon;
	return (icon >= 246 && icon <= 249) || icon == 339;
}

BOOL CDecor::IsGrotte(POINT cel)
{
	if (!IsValidCel(cel)) return FALSE;

	int icon = m_decor[cel.x][cel.y].icon;
	return icon == 284 || icon == 301 || icon == 337;
}

void CDecor::AdaptMidBorder(POINT cel)
{
	if (!IsValidCel(cel)) return;

	int num = 15;
	if (!IsRightBorder(cel.x, cel.y + 1, 0, -1 ))
	{
		num &= ~1;
	}
	if (!IsRightBorder(cel.x, cel.y + 1, 0, 1 ))
	{
		num &= ~2;
	}
	if (!IsRightBorder(cel.x + 1, cel.y, -1, 0 ))
	{
		num &= ~4;
	}
	if (!IsRightBorder(cel.x - 1, cel.y, 1, 0 ))
	{
		num &= ~8;
	}
	int num2 = m_decor[cel.x][cel.y].icon;
	if (num2 == 156)
	{
		num2 = 35;
	}
	if (num2 == 252 || num2 == 253)
	{
		num2 = 251;
	}
	if (num2 == 255)
	{
		num2 = 254;
	}
	if (num2 == 362)
	{
		num2 = 347;
	}
	if (num2 == 363)
	{
		num2 = 348;
	}
	if (num2 >= 341 && num2 <= 346)
	{
		num2 = 341;
	}
	for (int i = 0; i < 144; i++)
	{
		if (num2 == table_adapt_decor[i])
		{
			num2 = table_adapt_decor[i / 16 * 16 + num];
			if (num2 == 35 && rand() % 2 == 0)
			{
				num2 = 156;
			}
			if (num2 == 251)
			{
				num2 = Random(251, 253);
			}
			if (num2 == 254 && rand() % 2 == 0)
			{
				num2 = 255;
			}
			if (num2 == 347 && rand() % 2 == 0)
			{
				num2 = 362;
			}
			if (num2 == 348 && rand() % 2 == 0)
			{
				num2 = 363;
			}
			if (num2 == 341)
			{
				num2 = Random(341, 346);
			}
			m_decor[cel.x][cel.y].icon = num2;
			return;
		}
	}
	num2 = m_decor[cel.x][cel.y].icon;
	if (num2 == -1 || (num2 >= 264 && num2 <= 282))
	{
		num = 15;
		if (!IsFromage(POINT(cel.x, cel.y + 1)))
		{
			num &= ~1;
		}
		if (!IsFromage(POINT(cel.x, cel.y - 1)))
		{
			num &= ~2;
		}
		if (!IsFromage(POINT(cel.x + 1, cel.y)))
		{
			num &= ~4;
		}
		if (!IsFromage(POINT(cel.x - 1, cel.y)))
		{
			num &= ~8;
		}
		num2 = table_adapt_fromage[num];
		if (num2 == 268 && rand() % 2 == 0)
		{
			num2 = 279;
		}
		if (num2 == 269 && rand() % 2 == 0)
		{
			num2 = 280;
		}
		if (num2 == 264 && rand() % 2 == 0)
		{
			num2 = 281;
		}
		if (num2 == 265 && rand() % 2 == 0)
		{
			num2 = 282;
		}
		m_decor[cel.x][cel.y].icon = num2;
	}
	num2 = m_decor[cel.x][cel.y].icon;
	if (num2 == -1 || (num2 >= 285 && num2 <= 303 && num2 != 301))
	{
		num = 15;
		if (!IsGrotte(POINT(cel.x, cel.y + 1)))
		{
			num &= ~1;
		}
		if (!IsGrotte(POINT(cel.x, cel.y - 1)))
		{
			num &= ~2;
		}
		if (!IsGrotte(POINT(cel.x + 1, cel.y)))
		{
			num &= ~4;
		}
		if (!IsGrotte(POINT(cel.x - 1, cel.y)))
		{
			num &= ~8;
		}
		num2 = table_adapt_fromage[num + 16];
		if (num2 == 289 && rand() % 2 == 0)
		{
			num2 = 300;
		}
		if (num2 == 285 && rand() % 2 == 0)
		{
			num2 = 302;
		}
		if (num2 == 286 && rand() % 2 == 0)
		{
			num2 = 303;
		}
		m_decor[cel.x][cel.y].icon = num2;
	}
}

void CDecor::AdaptBorder(POINT cel)
{
	AdaptMidBorder(cel);
	AdaptMidBorder(POINT(cel.x + 1, cel.y));
	AdaptMidBorder(POINT(cel.x - 1, cel.y));
	AdaptMidBorder(POINT(cel.x, cel.y + 1));
	AdaptMidBorder(POINT(cel.x, cel.y - 1));
	int icon = m_decor[cel.x][cel.y].icon;
	if (icon != -1 && !IsPassIcon(icon))
	{
		MoveObjectDelete(cel);
	}
	icon = m_decor[cel.x][cel.y].icon;
	if (icon == 304)
	{
		for (int i = 0; i < 4; i++)
		{
			cel.y++;
			if (cel.y >= 100)
			{
				break;
			}
			icon = m_decor[cel.x][cel.y].icon;
			if (icon != -1)
			{
				break;
			}
			m_decor[cel.x][cel.y].icon = 305;
		}
	}
	if (icon == -1)
	{
		for (int i = 0; i < 4; i++)
		{
			cel.y++;
			if (cel.y >= 100)
			{
				return;
			}
			icon = m_decor[cel.x][cel.y].icon;
			if (icon != 305)
			{
				return;
			}
			m_decor[cel.x][cel.y].icon = -1;
		}
	}
}
