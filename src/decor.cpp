#include <ddraw.h>
#include <def.h>
#include <decor.h>
#include <misc.h>
#include <text.h>
#include <event.h>
#include <dectables.h>
#include <jauge.h>
#include <pixmap.h>
#include <sound.h>
#include <network.h>

int table_adapt_decor[80] = {
	153, 147, 148, 146, 40, 151, 150, 144, 39, 152, 149, 145, 38, 36, 37, 35,
	153, 147, 148, 146, 40, 151, 150, 144, 39, 152, 149, 145, 38, 36, 37, 156,
	75, 75, 155, 155, 75, 75, 155, 155, 74, 74, 154, 154, 75, 75, 155, 155,
	157, 157, 157, 157, 109, 109, 109, 109, 108, 108, 108, 108, 107, 107, 107, 107,
	202, 202, 138, 138, 202, 202, 138, 138, 202, 202, 138, 138, 202, 202, 138, 138
};

POINT CDecor::VoyageGetPosVie(int nbVies)
{
	return POINT( nbVies * 20 - 5, 417 );
}

void CDecor::VoyageInit(POINT start, POINT end, int icon, int channel)
{
	if (m_voyageIcon != -1)
	{
		m_voyagePhase = m_voyageTotal;
		VoyageStep();
	}
	m_voyageStart = start;
	m_voyageEnd = end;
	m_voyageIcon = icon;
	m_voyageChannel = channel;
	int num = abs(end.x - start.x);
	int num2 = abs(end.y - start.y);
	m_voyagePhase = 0;
	m_voyageTotal = (num + num2) / 10;
	if (icon == 48 && m_voyageChannel == GetBlupiChannelStandard())
	{
		m_voyageTotal = 60;
		m_nbVies--;
		m_pSound->PlayImage(SOUND_NEW, end, -1, 0);
	}
	if (icon == 21 && m_voyageChannel == CHELEMENT)
	{
		m_pSound->PlayImage(SOUND_EGG, start, -1, 0);
	}
	if (icon == 6 && m_voyageChannel == CHELEMENT)
	{
		if (m_nbTresor == m_totalTresor - 1)
		{
			m_pSound->PlayImage(SOUND_LASTTRESOR, start, -1, 0);
		}
		else
		{
			m_pSound->PlayImage(SOUND_TRESOR, start, -1, 0);
		}
	}
	if (icon == GetIconPerso() && m_voyageChannel == CHBUTTON)
	{
		m_pSound->PlayImage(SOUND_PERSOTAKE, start, -1, 0);
	}
	if (icon == 177 && m_voyageChannel == CHELEMENT)
	{
		m_pSound->PlayImage(SOUND_TAKEGLU, start, -1, 0);
	}
}

void CDecor::VoyageStep()
{
	if (m_voyageIcon != -1)
	{
		if (m_voyagePhase >= m_voyageTotal)
		{
			if (m_voyageIcon == 48 && m_voyageChannel == GetBlupiChannelStandard())
			{
				m_blupiAction = ACTION_STOP;
				m_blupiPhase = 0;
				m_blupiFocus = TRUE;
				m_blupiEnergyUnused = 100;
			}
			if (m_voyageIcon == 21 && m_voyageChannel == CHELEMENT)
			{
				m_nbVies++;
				m_pSound->PlayImage(SOUND_JUMPEND, m_voyageEnd, -1, 0);
			}
			if (m_voyageIcon == 6 && m_voyageChannel == CHELEMENT)
			{
				m_nbTresor++;
				m_pSound->PlayImage(SOUND_JUMPEND, m_voyageEnd, -1, 0);
			}
			if (m_voyageIcon == GetIconPerso() && m_voyageChannel == CHBUTTON)
			{
				m_blupiPerso++;
				m_pSound->PlayImage(SOUND_JUMPEND, m_voyageEnd, -1, 0);
			}
			if (m_voyageIcon == 177 && m_voyageChannel == CHELEMENT)
			{
				m_pSound->PlayImage(SOUND_JUMPEND, m_voyageEnd, -1, 0);
			}
			m_voyageIcon = -1;
		}
		m_voyagePhase++;
	}
}

void CDecor::VoyageDraw()
{
	if (m_voyageIcon != -1)
	{
		POINT pos;
		pos.x = m_voyageStart.x + (m_voyageEnd.x - m_voyageStart.x) * m_voyagePhase / m_voyageTotal;
		pos.y = m_voyageStart.y + (m_voyageEnd.y - m_voyageStart.y) * m_voyagePhase / m_voyageTotal;
		m_pPixmap->QuickIcon(m_voyageChannel, m_voyageIcon, pos);
	}
}

BOOL CDecor::IsRightBorder(int x, int y, int dx, int dy)
{
	int num = (m_dimDecor.x != 0) ? 100 : 10;
	int num2 = (m_dimDecor.y != 0) ? 100 : 8;
	if (x < 0) return TRUE;
	if (x >= num) return TRUE;
	if (y < 0) return TRUE;
	if (y >= num2) return TRUE;

	int icon = m_decor[x][y].icon;
	if (icon == -1) return FALSE;
	if (icon == 0x20 || icon == 0x21 || icon == 0x22) return FALSE;
	if (icon == 0x5c || icon == 0x44) return dy != -1;
	if (icon == 0x13 || icon == 0x15 || icon == 0x19 || icon == 0x1a || icon == 0x1c) return dy != 1;
	if (icon == 0x1b) return dx == 0;
	if (icon >= 0x2d && icon <= 0x2f) return dy != 1;
	if (icon == 0x0f || icon == 0x10 || icon == 0x4b || icon == 0x59 || icon == 0x5a || icon == 0x9b)
	{
		if (dx != -1)
		{
			if (dy != 1) return TRUE;
			return FALSE;
		}
		return FALSE;
	}
	if (icon == 0x11 || icon == 0x12 || icon == 0x4a || icon == 0x57 || icon == 0x58 || icon == 0x9a)
	{
		if (dx == 1) return FALSE;
		if (dy == 1) return FALSE;
	}
	else
	{
		if (icon == 0x4c || icon == 0x4d || icon == 199 || icon == 200) return dx == 0;
		if (icon >= 0x6e && icon <= 0x7d) return FALSE;
		if (icon == 0x7e) return dx == 1;
		if (icon == 0x81) return dx == -1;
		if (icon == 0x84) return dy == 1;
		if (icon == 0x87) return dy == -1;
		if (icon >= 0x8b && icon <= 0x8f) return FALSE;
		if (icon == 0x8a) return dy == -1;
		if (icon == 0xca) return FALSE;
		if (icon == 0x6b || icon == 0x6c || icon == 0x6d || icon == 0x9d) return dx != 0;
		if (icon >= 0x9e && icon <= 0xb5) return dy == 1;
		if (icon == 0xb6 || icon == 0xb7) return FALSE;
	}
	return TRUE;
}

void CDecor::AdaptMidBorder(POINT cel)
{
	int num = 15;
	if (cel.x < 0 || cel.x >= 100 || cel.y < 0 || cel.y >= 100) return;
	if (!IsRightBorder(cel.x, cel.y + 1, 0, -1)) num = 14;
	if (!IsRightBorder(cel.x, cel.y - 1, 0, 1)) num &= ~2;
	if (!IsRightBorder(cel.x + 1, cel.y, -1, 0)) num &= ~4;
	if (!IsRightBorder(cel.x - 1, cel.y, 1, 0)) num &= ~8;

	int icon = m_decor[cel.x][cel.y].icon;
	for (int i = 0; i < 80; i++)
	{
		if (icon == table_adapt_decor[i])
		{
			icon = table_adapt_decor[i / 16 * 16 + num];
			if (icon == 0x23 && rand() % 2 == 0) icon = 0x9c;
			m_decor[cel.x][cel.y].icon = icon;
			return;
		}
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
	if (icon != -1 && !IsPassIcon(icon)) MoveObjectDelete(cel);
}

BOOL CDecor::DrawMap(BOOL bPlay, int team)
{
	POINT oldPosDecor = m_posDecor;
	UINT oldPhase = m_phase;
	POINT oldScrollPoint = m_scrollPoint;
	BOOL bOldMulti = m_bMulti;
	int oldBlupiChannel = m_blupiChannel;
	int oldTeam = m_team;
	m_blupiChannel = 2;
	m_team = 0;
	m_phase = WM_PHASE_PLAY;
	LoadImages();
	if (bPlay) PlayPrepare(TRUE);
	m_bMulti = FALSE;
	if (team >= 0 && team < MAXTEAM)
	{
		for (int i = 0; i < MAXTEAM; i++)
		{
			if (m_pNetwork->m_players[i].bIsPresent)
			{
				int playerTeam = m_pNetwork->m_players[i].team;
				int icon = (m_blupiStartDir[playerTeam] == DIR_LEFT) ? 4 : 0;
				int channel = 2;
				if (playerTeam >= 1 && playerTeam < MAXTEAM)
				{
					channel = CHBLUPI1 + playerTeam - 1;
				}
				if (team == i)
				{
					m_blupiPos = m_blupiStartPos[playerTeam];
					m_blupiDir = m_blupiStartDir[playerTeam];
					m_blupiIcon = icon;
					m_blupiChannel = channel;
					m_team = playerTeam;
				}
				else
				{
					m_netBlupiPos[i] = m_blupiStartPos[playerTeam];
					m_netBlupiIcon[i] = icon;
					m_netBlupiSec[i] = 0;
					m_netBlupiTransport[i] = channel;
				}
			}
			else
			{
				m_netBlupiIcon[i] = -1;
			}
		}
		m_scrollPoint = m_blupiPos;
		m_scrollAdd.x = 0;
		m_scrollAdd.y = 0;
		m_bMulti = TRUE;
	}
	m_posDecor.x = m_scrollPoint.x - LXIMAGE / 2;
	if (m_posDecor.x < 0) m_posDecor.x = 0;
	int limit = (m_dimDecor.x != 0) ? MAXCELX * DIMOBJX - LXIMAGE : 118;
	if (m_posDecor.x > limit) m_posDecor.x = limit;

	m_posDecor.y = m_scrollPoint.y - 200;
	if (m_posDecor.y < 0) m_posDecor.y = 0;
	limit = (m_dimDecor.y != 0) ? MAXCELY * DIMOBJY - LYIMAGE : 80;
	if (m_posDecor.y > limit) m_posDecor.y = limit;

	m_scrollPoint = m_posDecor;
	Build(RECT(0, 0, LXIMAGE, LYIMAGE));
	m_pPixmap->DrawMap(CHMAP, RECT(0, 0, 522, 400), RECT(0, 0, 206, 158));
	m_posDecor = oldPosDecor;
	m_scrollPoint = oldScrollPoint;
	m_phase = oldPhase;
	m_blupiChannel = oldBlupiChannel;
	m_team = oldTeam;
	m_bMulti = bOldMulti;
	return TRUE;
}
void CDecor::GetMissionPath(char* str, int user, int mission, BOOL bUser)
{
	if (bUser)
	{
		sprintf(str, "data\\u%.3d-%.3d.blp", user, mission);
		AddUserPath(str);
	}
	else
	{
		sprintf(str, "data\\world%.3d.blp", mission);
		AddUserPath(str);
	}
}

BOOL CDecor::Write(int gamer, int mission, BOOL bUser)
{
	char filename[FILENAME_MAX];
	DescFile* pBuffer = NULL;
	FILE* file;
	int* blupiDir;
	POINT* blupiPos;
	int nb;
	int i;

	GetMissionPath(filename, gamer, mission, bUser);
	
	file = fopen(filename, "wb");
	if (file == NULL) goto error;
	
	pBuffer = (DescFile*)malloc(sizeof(DescFile));
	if (pBuffer == NULL) goto error;
	memset(pBuffer, 0, sizeof(DescFile));

	pBuffer->majRev = 1;
	pBuffer->minRev = 3;
	pBuffer->reserve1[0] = 0;
	pBuffer->posDecor = m_posDecor;
	pBuffer->dimDecor = m_dimDecor;
	pBuffer->music = m_music;
	pBuffer->region = m_region;
	strcpy(pBuffer->name, m_missionTitle);

	for (i = 0; i < MAXTEAM; i++)
	{
		pBuffer->blupiPos[i] = m_blupiStartPos[i];
		pBuffer->blupiDir[i] = m_blupiStartDir[i];
	}

	nb = fwrite(pBuffer, sizeof(DescFile), 1, file);
	if (nb < 1) goto error;

	nb = fwrite(m_decor, sizeof(Cellule), MAXCELX * MAXCELY, file);
	if (nb < MAXCELX * MAXCELY) goto error;

	nb = fwrite(m_bigDecor, sizeof(Cellule), MAXCELX * MAXCELY, file);
	if (nb < MAXCELX * MAXCELY) goto error;

	nb = fwrite(m_moveObject, sizeof(MoveObject), MAXMOVEOBJECT, file);
	if (nb < MAXMOVEOBJECT) goto error;

	free(pBuffer);
	fclose(file);
	return TRUE;

error: 
	if (pBuffer != NULL) free(pBuffer);
	if (file != NULL) fclose(file);
	return FALSE;
}

BOOL CDecor::Read(int gamer, int mission, BOOL bUser)
{
	char filename[FILENAME_MAX];
	FILE* file;
	DescFile* pBuffer;
	int majRev, minRev;
	int nb;
	int i = 4;


	pBuffer = 0;
	InitDecor();
	GetMissionPath(filename, gamer, mission, bUser);
	
	file = fopen(filename, "rb");
	if (file == NULL) goto error;
	
	pBuffer = (DescFile*)malloc(sizeof(DescFile));
	if (pBuffer == NULL) goto error;

	nb = fread(pBuffer, sizeof(DescFile), 1, file);
	if (nb < 1) goto error;
		
	majRev = pBuffer->majRev;
	minRev = pBuffer->minRev;
	m_posDecor = pBuffer->posDecor;
	m_dimDecor = pBuffer->dimDecor;
	m_music = pBuffer->music;
	m_region = pBuffer->region;
	if (majRev >= 1 && minRev >= 3)
	{
		strcpy(m_missionTitle, pBuffer->name);
	}

	for (i = 0; i < MAXTEAM; i++)
	{
		m_blupiStartPos[i] = pBuffer->blupiPos[i];
		m_blupiStartDir[i] = pBuffer->blupiDir[i];
	}

	nb = fread(m_decor, sizeof(Cellule), MAXCELX * MAXCELY, file);
	if (nb < MAXCELX * MAXCELY) goto error;
	

	if (majRev == 1 && minRev >= 1)
	{
		nb = fread(m_bigDecor, sizeof(Cellule), MAXCELX * MAXCELY, file);
		if (nb < MAXCELX * MAXCELY) goto error;
	}

	memset(m_moveObject, 0, sizeof(MoveObject) * MAXMOVEOBJECT);
	if (majRev == 1 && minRev >= 2)
	{
		nb = fread(m_moveObject, sizeof(MoveObject), MAXMOVEOBJECT, file);
		if (nb < MAXMOVEOBJECT) goto error;
	}
	else
	{
		nb = fread(m_moveObject, sizeof(MoveObject), MAXMOVEOBJECT / 2, file);
		if (nb < MAXMOVEOBJECT / 2) goto error;
	}

	free(pBuffer);
	fclose(file);
	return TRUE;
error:
	if (pBuffer != NULL) free(pBuffer);
	if (file != NULL) fclose(file);
	InitDecor();
	return FALSE;
}

BOOL CDecor::DeleteMission(int user, int mission, BOOL bUser)
{
	char filename[FILENAME_MAX];

	GetMissionPath(filename, user, mission, bUser);
	remove(filename);
	return TRUE;
}

BOOL CDecor::CurrentWrite(int gamer, int mission)
{
	char filename[MAX_PATH];
	FILE* file = NULL;
	DescGameFile* pBuffer = NULL;
	int nb, i, x, y;
	int majRev, minRev;

	sprintf(filename, "data\\s%.3d-%.3d.blp", gamer, mission);
	AddUserPath(filename);

	file = fopen(filename, "wb");
	if (file == NULL) goto error;

	pBuffer = (DescGameFile*)malloc(sizeof(DescGameFile));
	if (pBuffer == NULL) goto error;

	memset(pBuffer, 0, sizeof(DescGameFile));
	pBuffer->lg = sizeof(DescGameFile);
	pBuffer->majRev = 1;
	pBuffer->minRev = 2;
	pBuffer->reseve1[0] = 0;
	memcpy(pBuffer->decor, m_decor, sizeof(pBuffer->decor));
	memcpy(pBuffer->bigDecor, m_bigDecor, sizeof(pBuffer->bigDecor));
	memcpy(pBuffer->moveObject, m_moveObject, sizeof(pBuffer->moveObject));
	pBuffer->posDecor = m_posDecor;
	pBuffer->dimDecor = m_dimDecor;
	pBuffer->phase = m_phase;
	pBuffer->term = m_term;
	pBuffer->music = m_music;
	pBuffer->region = m_region;
	pBuffer->time = m_time;
	memcpy(pBuffer->missionTitle, m_missionTitle, sizeof(pBuffer->missionTitle));
	pBuffer->nbRankCaisse = m_nbRankCaisse;
	memcpy(pBuffer->rankCaisse, m_rankCaisse, sizeof(pBuffer->rankCaisse));
	pBuffer->nbLinkCaisse = m_nbLinkCaisse;
	memcpy(pBuffer->linkCaisse, m_linkCaisse, sizeof(pBuffer->linkCaisse));
	pBuffer->blupiPos = m_blupiPos;
	pBuffer->blupiValidPos = m_blupiValidPos;
	pBuffer->blupiAction = m_blupiAction;
	pBuffer->blupiDir = m_blupiDir;
	pBuffer->blupiPhase = m_blupiPhase;
	pBuffer->blupiVitesse = m_blupiVitesse;
	pBuffer->blupiIcon = m_blupiIcon;
	pBuffer->blupiSec = m_blupiSec;
	pBuffer->blupiChannel = m_blupiChannel;
	pBuffer->blupiVector = m_blupiVector;
	pBuffer->blupiTransport = m_blupiTransport;
	pBuffer->blupiFocus = m_blupiFocus;
	pBuffer->blupiAir = m_blupiAir;
	pBuffer->blupiHelico = m_blupiHelico;
	pBuffer->blupiJeep = m_blupiJeep;
	pBuffer->blupiTank = m_blupiTank;
	pBuffer->blupiSkate = m_blupiSkate;
	pBuffer->blupiNage = m_blupiNage;
	pBuffer->blupiSurf = m_blupiSurf;
	pBuffer->blupiVent = m_blupiVent;
	pBuffer->blupiSuspend = m_blupiSuspend;
	pBuffer->blupiJumpAie = m_blupiJumpAie;
	pBuffer->blupiShield = m_blupiShield;
	pBuffer->blupiPower = m_blupiPower;
	pBuffer->blupiCloud = m_blupiCloud;
	pBuffer->blupiHide = m_blupiHide;
	pBuffer->blupiPosHelico.x = m_blupiPosHelico.x;
	pBuffer->blupiPosHelico.y = m_blupiPosHelico.y;
	pBuffer->blupiPosMagic.x = m_blupiPosMagic.x;
	pBuffer->blupiPosMagic.y = m_blupiPosMagic.y;
	pBuffer->blupiRestart = m_blupiRestart;
	pBuffer->blupiFront = m_blupiFront;
	pBuffer->blupiBullet = m_blupiBullet;
	pBuffer->blupiPerso = m_blupiPerso;
	pBuffer->blupiNoBarre = m_blupiNoBarre;
	pBuffer->blupiTimeShield = m_blupiTimeShield;
	pBuffer->blupiTimeFire = m_blupiTimeFire;
	pBuffer->blupiTimeOuf = m_blupiTimeOuf;
	pBuffer->blupiActionOuf = m_blupiActionOuf;
	pBuffer->blupiFifoNb = m_blupiFifoNb;
	memcpy(pBuffer->blupiFifoPos, m_blupiFifoPos, sizeof(pBuffer->blupiFifoPos));
	memcpy(pBuffer->blupiStartPos, m_blupiStartPos, sizeof(pBuffer->blupiStartPos));
	memcpy(pBuffer->blupiStartDir, m_blupiStartDir, sizeof(pBuffer->blupiStartDir));
	for (i = 0; i < 2; i++) {
		pBuffer->jaugeHide[i] = m_jauges[i].GetHide();
		pBuffer->jaugeType[i] = m_jauges[i].GetType();
		pBuffer->jaugeLevel[i] = m_jauges[i].GetLevel();
	}
	pBuffer->blupiLevel = m_blupiLevel;
	pBuffer->energyUnused = m_blupiEnergyUnused;
	pBuffer->playHelicoHigh = m_bHelicoMarch;
	pBuffer->playHelicoLow = m_bHelicoStop;
	pBuffer->playJeepHigh = m_bJeepMarch;
	pBuffer->playJeepLow = m_bJeepStop;
	pBuffer->bFoundCle = m_bFoundCle;
	pBuffer->bPrivate = m_bPrivate;
	pBuffer->bCheatDoors = m_bCheatDoors;
	pBuffer->bSuperBlupi = m_bSuperBlupi;
	pBuffer->bDrawSecret = m_bDrawSecret;
	pBuffer->bJoystick = m_bJoystick;
	pBuffer->mission = m_mission;
	memcpy(pBuffer->doors, m_doors, sizeof(pBuffer->doors));
	pBuffer->nbVies = m_nbVies;
	pBuffer->nbTresor = m_nbTresor;
	pBuffer->totalTresor = m_totalTresor;
	pBuffer->goalPhase = m_goalPhase;
	pBuffer->scrollFollow = m_scrollFollow;
	pBuffer->scrollPoint = m_scrollPoint;
	pBuffer->scrollAdd = m_scrollAdd;
	pBuffer->voyageIcon = m_voyageIcon;
	pBuffer->voyageChannel = m_voyageChannel;
	pBuffer->voyagePhase = m_voyagePhase;
	pBuffer->voyageTotal = m_voyageTotal;
	pBuffer->voyageStart = m_voyageStart;
	pBuffer->voyageEnd = m_voyageEnd;
	pBuffer->decorAction = m_decorAction;
	pBuffer->decorPhase = m_decorPhase;

	nb = fwrite(pBuffer, sizeof(DescGameFile), 1, file);
	if (nb < 1) goto error;
	free(pBuffer);
	fclose(file);
	return TRUE;

error:
	if (pBuffer != NULL) free(pBuffer);
	if (file != NULL) fclose(file);
	return FALSE;

}

BOOL CDecor::CurrentRead(int gamer, int mission, int *pMission, BOOL *pbPrivate)
{
	char filename[MAX_PATH];
	FILE* file = NULL;
	DescGameFile* pBuffer = NULL;
	int majRev, minRev;
	int nb, i, x, y;

	sprintf(filename, "data\\s%.3d-%.3d.blp", gamer, mission);
	AddUserPath(filename);

	file = fopen(filename, "rb");
	if (file == NULL) goto error;

	pBuffer = (DescGameFile*)malloc(sizeof(DescGameFile));
	if (pBuffer == NULL) goto error;

	nb = fread(pBuffer, sizeof(DescGameFile), 1, file);
	if (nb < 1) goto error;
	if (pBuffer->lg == sizeof(DescGameFile)) goto success;
error:
	if (pBuffer != NULL) free(pBuffer);
	if (file != NULL) fclose(file);
	return FALSE;
success:

	majRev = pBuffer->majRev;
	minRev = pBuffer->minRev;
	memcpy(m_decor, pBuffer->decor, sizeof(m_decor));
	memcpy(m_bigDecor, pBuffer->bigDecor, sizeof(m_bigDecor));

	memcpy(m_moveObject, pBuffer->moveObject, sizeof(m_moveObject));
	m_posDecor = pBuffer->posDecor;
	m_dimDecor = pBuffer->dimDecor;
	m_phase = pBuffer->phase;
	m_term = pBuffer->term;
	m_music = pBuffer->music;
	m_region = pBuffer->region;
	m_time = pBuffer->time;
	memcpy(m_missionTitle, pBuffer->missionTitle, sizeof(m_missionTitle));
	m_nbRankCaisse = pBuffer->nbRankCaisse;
	memcpy(m_rankCaisse, pBuffer->rankCaisse, sizeof(m_rankCaisse));
	m_nbLinkCaisse = pBuffer->nbLinkCaisse;
	memcpy(m_linkCaisse, pBuffer->linkCaisse, sizeof(m_linkCaisse));
	m_blupiPos = pBuffer->blupiPos;
	m_blupiValidPos = pBuffer->blupiValidPos;
	m_blupiAction = pBuffer->blupiAction;
	m_blupiDir = pBuffer->blupiDir;
	m_blupiPhase = pBuffer->blupiPhase;
	m_blupiVitesse = pBuffer->blupiVitesse;
	m_blupiIcon = pBuffer->blupiIcon;
	m_blupiSec = pBuffer->blupiSec;
	m_blupiChannel = pBuffer->blupiChannel;
	m_blupiVector = pBuffer->blupiVector;
	m_blupiTransport = pBuffer->blupiTransport;
	m_blupiFocus = pBuffer->blupiFocus;
	m_blupiAir = pBuffer->blupiAir;
	m_blupiHelico = pBuffer->blupiHelico;
	m_blupiJeep = pBuffer->blupiJeep;
	m_blupiTank = pBuffer->blupiTank;
	m_blupiSkate = pBuffer->blupiSkate;
	m_blupiNage = pBuffer->blupiNage;
	m_blupiSurf = pBuffer->blupiSurf;
	m_blupiVent = pBuffer->blupiVent;
	m_blupiSuspend = pBuffer->blupiSuspend;
	m_blupiJumpAie = pBuffer->blupiJumpAie;
	m_blupiShield = pBuffer->blupiShield;
	m_blupiPower = pBuffer->blupiPower;
	m_blupiCloud = pBuffer->blupiCloud;
	m_blupiHide = pBuffer->blupiHide;
	m_blupiPosHelico.x = pBuffer->blupiPosHelico.x;
	m_blupiPosHelico.y = pBuffer->blupiPosHelico.y;
	m_blupiPosMagic.x = pBuffer->blupiPosMagic.x;
	m_blupiPosMagic.y = pBuffer->blupiPosMagic.y;
	m_blupiRestart = pBuffer->blupiRestart;
	m_blupiFront = pBuffer->blupiFront;
	m_blupiBullet = pBuffer->blupiBullet;
	m_blupiPerso = pBuffer->blupiPerso;
	m_blupiNoBarre = pBuffer->blupiNoBarre;
	m_blupiTimeShield = pBuffer->blupiTimeShield;
	m_blupiTimeFire = pBuffer->blupiTimeFire;
	m_blupiTimeOuf = pBuffer->blupiTimeOuf;
	m_blupiActionOuf = pBuffer->blupiActionOuf;
	m_blupiFifoNb = pBuffer->blupiFifoNb;
	memcpy(m_blupiFifoPos, pBuffer->blupiFifoPos, sizeof(m_blupiFifoPos));
	memcpy(m_blupiStartPos, pBuffer->blupiStartPos, sizeof(m_blupiStartPos));
	memcpy(m_blupiStartDir, pBuffer->blupiStartDir, sizeof(m_blupiStartDir));
	for (i = 0; i < 2; i++) {
		m_jauges[i].SetHide(pBuffer->jaugeHide[i]);
		m_jauges[i].SetType(pBuffer->jaugeType[i]);
		m_jauges[i].SetLevel(pBuffer->jaugeLevel[i]);
	}
	m_blupiLevel = pBuffer->blupiLevel;
	m_blupiEnergyUnused = pBuffer->energyUnused;
	m_bHelicoMarch = pBuffer->playHelicoHigh;
	m_bHelicoStop = pBuffer->playHelicoLow;
	m_bJeepMarch = pBuffer->playJeepHigh;
	m_bJeepStop = pBuffer->playJeepLow;
	m_bFoundCle = pBuffer->bFoundCle;
	m_bPrivate = pBuffer->bPrivate;
	m_bCheatDoors = pBuffer->bCheatDoors;
	m_bSuperBlupi = pBuffer->bSuperBlupi;
	m_bDrawSecret = pBuffer->bDrawSecret;
	m_bJoystick = pBuffer->bJoystick;
	m_mission = pBuffer->mission;
	memcpy(m_doors, pBuffer->doors, sizeof(m_doors));
	m_nbVies = pBuffer->nbVies;
	m_nbTresor = pBuffer->nbTresor;
	m_totalTresor = pBuffer->totalTresor;
	m_goalPhase = pBuffer->goalPhase;
	m_scrollFollow = pBuffer->scrollFollow;
	m_scrollPoint = pBuffer->scrollPoint;
	m_scrollAdd = pBuffer->scrollAdd;
	m_voyageIcon = pBuffer->voyageIcon;
	m_voyageChannel = pBuffer->voyageChannel;
	m_voyagePhase = pBuffer->voyagePhase;
	m_voyageTotal = pBuffer->voyageTotal;
	m_voyageStart = pBuffer->voyageStart;
	m_voyageEnd = pBuffer->voyageEnd;
	m_decorAction = pBuffer->decorAction;
	m_decorPhase = pBuffer->decorPhase;
	*pbPrivate = m_bPrivate;
	*pMission = m_mission;

	free(pBuffer);
	fclose(file);
	return TRUE;
}

BOOL CDecor::FileExist(int gamer, int rank, char* name)
{
	char filename[MAX_PATH];
	char temp[100];
	FILE* file = NULL;
	DescGameFile* pBuffer = NULL;
	int nb, i, open;

	sprintf(filename, "data\\s%.3d-%.3d.blp", gamer, rank);
	AddUserPath(filename);
	file = fopen(filename, "rb");
	if (file == NULL) goto error;

	pBuffer = (DescGameFile*)malloc(sizeof(DescGameFile));
	if (pBuffer == NULL) goto error;

	nb = fread(pBuffer, sizeof(DescGameFile), 1, file);
	if (nb < 1) goto error;
	if (pBuffer->lg == sizeof(DescGameFile)) goto success;
error:
	if (pBuffer != NULL) free(pBuffer);
	if (file != NULL) fclose(file);
	return FALSE;

success:
		open = 0;
		for (i = 0; i < sizeof(pBuffer->doors) / sizeof(pBuffer->doors[0]); i++)
		{
			if (pBuffer->doors[i] == 0) open++;
		}

		if (pBuffer->bPrivate)
		{
			LoadString(0x111, temp, 100);
			sprintf(name, temp, pBuffer->mission, pBuffer->nbTresor, pBuffer->totalTresor);
		}
		else
		{
			if (pBuffer->mission == 99)
			{
				LoadString(0x11c, temp, 100);
				sprintf(name, temp, pBuffer->nbVies, pBuffer->nbTresor, pBuffer->totalTresor);
			}
			else if (pBuffer->mission < 99)
			{
				LoadString(pBuffer->mission / 10 + 274, temp, 100);
				sprintf(name, temp, pBuffer->mission % 10, open, pBuffer->nbVies, pBuffer->nbTresor, pBuffer->totalTresor);
			}
			else
			{
				LoadString(0x11d, temp, 100);
				sprintf(name, temp, pBuffer->mission, open, pBuffer->nbVies, pBuffer->nbTresor, pBuffer->totalTresor);
			}
		}

	free(pBuffer);
	fclose(file);
	return TRUE;
}

BOOL CDecor::SearchWorld(int world, POINT *cel, POINT *newBlupiPos)
{
	for (int x = 0; x < MAXCELX; x++)
	{
		for (int y = 0; y < MAXCELY; y++)
		{
			int icon = m_decor[x][y].icon;
			if (icon >= Object::Level_1 && icon <= Object::Level_8)
			{
				if (icon - Object::Level_1 + 1 == world)
				{
					if (x > 0 && m_decor[x - 1][y].icon == Object::DoorLevel)
					{
						cel->x = x - 1;
						cel->y = y;
						newBlupiPos->x = (x - 2) * DIMOBJX + 2;
						newBlupiPos->y = y * DIMOBJY + 6;
						return TRUE;
					}
					if (x > 1 && m_decor[x - 2][y].icon == Object::DoorLevel)
					{
						cel->x = x - 2;
						cel->y = y;
						newBlupiPos->x = (x - 3) * DIMOBJX + 2;
						newBlupiPos->y = y * DIMOBJY + 6;
						return TRUE;
					}
					if (x < MAXCELX - 1 && m_decor[x + 1][y].icon == Object::DoorLevel)
					{
						cel->x = x + 1;
						cel->y = y;
						newBlupiPos->x = (x + 2) * DIMOBJX + 2;
						newBlupiPos->y = y * DIMOBJY + 6;
						return TRUE;
					}
					if (x < MAXCELX - 2 && m_decor[x + 2][y].icon == Object::DoorLevel)
					{
						cel->x = x + 2;
						cel->y = y;
						newBlupiPos->x = (x + 3) * DIMOBJX + 2;
						newBlupiPos->y = y * DIMOBJY + 6;
						return TRUE;
					}
				}
			}
		}
	}
	return FALSE;
}

BOOL CDecor::SearchDoor(int n, POINT *cel)
{
	for (int y = MAXCELY - 1; y >= 0; y--)
	{
		for (int x = MAXCELX - 1; x >= 0; x--)
		{
			if (m_decor[x][y].icon == 183)
			{
				cel->x = x;
				cel->y = y;
				return TRUE;
			}
		}
	}
	return FALSE;
}

void CDecor::AdaptDoors(BOOL bPrivate, int mission)
{
	POINT cel;
	int index;
	int icon;
	POINT newPosBlupi;

	m_bPrivate = bPrivate;
	m_mission = mission;
	if (bPrivate == FALSE)
	{
		if (mission == 1)
		{
			for (int i = 0; i < 10; ++i)
			{
				if (SearchDoor(i, &cel))
				{
					if (!m_doors[i] || m_bCheatDoors)
					{
						m_decor[cel.x][cel.y].icon = -1;
						index = MoveObjectFree();
						m_moveObject[index].type = TYPE_DOOR;
						m_moveObject[index].stepAdvance = 50;
						m_moveObject[index].stepRecede = 1;
						m_moveObject[index].timeStopStart = 0;
						m_moveObject[index].timeStopEnd = 0;
						m_moveObject[index].posStart.x = cel.x * DIMOBJX;
						m_moveObject[index].posStart.y = cel.y * DIMOBJY;
						m_moveObject[index].posEnd.x = cel.x * DIMOBJX;
						m_moveObject[index].posEnd.y = cel.y * DIMOBJY - DIMOBJY;
						m_moveObject[index].posCurrent.x = m_moveObject[index].posStart.x;
						m_moveObject[index].posCurrent.y = m_moveObject[index].posStart.y;
						m_moveObject[index].step = STEP_STOPSTART;
						m_moveObject[index].time = 0;
						m_moveObject[index].phase = 0;
						m_moveObject[index].channel = CHOBJECT;
						m_moveObject[index].icon = 0xb7;
						PlaySound(SOUND_DOOR, m_moveObject[index].posStart, FALSE);
					}
				}
			}
			for (int x = 0; x < MAXCELX; x++)
			{
				for (int y = 0; y < MAXCELY; y++)
				{
					int icon = m_decor[x][y].icon;
					if (icon >= Object::World_1 && icon <= Object::World_8 && (!m_doors[icon - Object::World_1 + 1] || m_bCheatDoors))
					{
						m_decor[x][y].icon += 8;
					}
				}
			}
		}
		else if (mission % 10 == 0)
		{
			for (int i = 0; i < 10; ++i)
			{
				if (SearchWorld(i, &cel, &newPosBlupi))
				{
					if (!m_doors[mission + i] || m_bCheatDoors)
					{
						m_decor[cel.x][cel.y].icon = -1;
						index = MoveObjectFree();
						m_moveObject[index].type = TYPE_DOOR;
						m_moveObject[index].stepAdvance = 50;
						m_moveObject[index].stepRecede = 1;
						m_moveObject[index].timeStopStart = 0;
						m_moveObject[index].timeStopEnd = 0;
						m_moveObject[index].posStart.x = cel.x * DIMOBJX;
						m_moveObject[index].posStart.y = cel.y * DIMOBJY;
						m_moveObject[index].posEnd.x = cel.x * DIMOBJX;
						m_moveObject[index].posEnd.y = cel.y * DIMOBJY - DIMOBJY;
						m_moveObject[index].posCurrent.x = m_moveObject[index].posStart.x;
						m_moveObject[index].posCurrent.y = m_moveObject[index].posStart.y;
						m_moveObject[index].step = STEP_STOPSTART;
						m_moveObject[index].time = 0;
						m_moveObject[index].phase = 0;
						m_moveObject[index].channel = CHOBJECT;
						m_moveObject[index].icon = 0xb6;
						PlaySound(SOUND_DOOR, m_moveObject[index].posStart, FALSE);
						for (int j = 0; j < MAXTEAM; j++)
						{
							m_blupiStartPos[j] = newPosBlupi;
							m_blupiStartDir[j] = (m_blupiStartPos[j].x < cel.x * DIMOBJX) ? DIR_RIGHT : DIR_LEFT;
						}
					}
				}
			}
		}
	}
}

void CDecor::OpenDoorsWin()
{
	m_doors[m_mission + 1] = 0;
}

void CDecor::OpenGoldsWin()
{
	m_doors[m_mission / 10] = 0;
}

void CDecor::DoorsLost()
{
	int mission = m_mission / 10;
	m_nbVies = 3;
	int index = mission * 10;
	int end = index + 10;
	if (index < end)
	{
		for (int i = index; i < end; i++)
		{
			m_doors[i] = 1;
		}
	}
}
