// DecMove.cpp
//

#include <def.h>
#include <decor.h>
#include <misc.h>
#include <dectables.h>

void CDecor::MoveObjectPollution()
{
	POINT pos = m_blupiPos;
	POINT offset = { 0, 0 };
	BOOL bPollution = FALSE;
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
		if (m_keyPress & (KEY_JUMP | KEY_UP))
		{
			if (m_time % 20 != 0 &&
				m_time % 20 != 2 &&
				m_time % 20 != 5 &&
				m_time % 20 != 8 &&
				m_time % 20 != 11 &&
				m_time % 20 != 13 &&
				m_time % 20 != 14 &&
				m_time % 20 != 18)
			{
				return;
			}
			speed = 58;
			offset.x = Random(-10, 10);
			offset.y = 22;
		}
		else
		{
			if (m_time % 50 != 0 &&
				m_time % 50 != 12 &&
				m_time % 50 != 30)
			{
				return;
			}
			speed = 20;
			offset.x = 30;
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
	PlaySound(SOUND_PLOUF, pos);
	ObjectStart(pos, TYPE_PLOUF, 0);
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
	PlaySound(SOUND_TIPLOUF, pos);
	ObjectStart(pos, TYPE_TIPLOUF, 0);
}

void CDecor::MoveObjectBlup(POINT pos)
{
	PlaySound(SOUND_BLUP, pos);
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


void CDecor::FlushBalleTraj()
{
	ZeroMemory(m_balleTraj, sizeof(m_balleTraj));
}

void CDecor::SetBalleTraj(POINT cel)
{
	if (!IsValidCel(cel)) return;
	int num = cel.y * TRAJBYTEX;
	num += cel.x / 8;
	int num2 = cel.x & 7;
	m_balleTraj[num] |= 1 << num2;
}

BOOL CDecor::IsBalleTraj(POINT pos)
{
	POINT cel;
	pos.x = (pos.x + DIMOBJX / 2) / DIMOBJX;
	pos.y = (pos.y + DIMOBJY / 2) / DIMOBJY;
	cel.x = pos.x / DIMOBJX;
	cel.y = pos.y / DIMOBJY;
	if (!IsValidCel(cel)) return FALSE;
	int num = cel.y * TRAJBYTEX;
	num += cel.x / 8;
	int num2 = cel.x & 7;
	return (m_balleTraj[num] & 1 << num2) != 0;
}

void CDecor::FlushMoveTraj()
{
	memset(m_moveTraj, 0, sizeof(m_moveTraj));
}

void CDecor::SetMoveTraj(POINT cel)
{

	if (!IsValidCel(cel)) return;
	int num = cel.y * TRAJBYTEX;
	num += cel.x / 8;
	int num2 = cel.x & 7;
	m_moveTraj[num] |= 1 << num2;
}

BOOL CDecor::IsMoveTraj(POINT pos)
{
	POINT cel;
	pos.x = (pos.x + DIMOBJX / 2) / DIMOBJX;
	pos.y = (pos.y + DIMOBJY / 2) / DIMOBJY;
	cel.x = pos.x / DIMOBJX;
	cel.y = pos.y / DIMOBJY;
	if (!IsValidCel(cel)) return FALSE;
	int num = cel.y * TRAJBYTEX;
	num += cel.x / 8;
	int num2 = cel.x & 7;
	return (m_moveTraj[num] & 1 << num2) != 0;
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


void CDecor::StartSploutchGlu(POINT pos)
{
	POINT pos2;
	pos2.x = pos.x;
	pos2.y = pos.y;
	ObjectStart(pos2, TYPE_SPLOUTCH1, 0);
	pos2.x = pos.x + 15;
	pos2.y = pos.y + 20;
	ObjectStart(pos2, TYPE_SPLOUTCH2, 0);
	pos2.x = pos.x - 20;
	pos2.y = pos.y + 18;
	ObjectStart(pos2, TYPE_SPLOUTCH2, 0);
	pos2.x = pos.x + 23;
	pos2.y = pos.y - 18;
	ObjectStart(pos2, TYPE_SPLOUTCH2, 0);
	pos2.x = pos.x - 15;
	pos2.y = pos.y - 18;
	ObjectStart(pos2, TYPE_SPLOUTCH2, 0);
	pos2.x = pos.x + 32;
	pos2.y = pos.y + 10;
	ObjectStart(pos2, TYPE_SPLOUTCH3, 0);
	pos2.x = pos.x - 28;
	pos2.y = pos.y + 15;
	ObjectStart(pos2, TYPE_SPLOUTCH3, 0);
	StopVehicleSound();
	PlaySound(SOUND_GLU, pos);
}

BOOL CDecor::ObjectStart(POINT pos, int type, int speed)
{
	return ObjectStart(pos, type, speed, FALSE);
}

BOOL CDecor::ObjectStart(POINT pos, int type, int speed, BOOL bMulti)
{
	int index = MoveObjectFree();
	if (index == -1)
	{
		return FALSE;
	}
	m_moveObject[index].type = type;
	m_moveObject[index].phase = 0;
	m_moveObject[index].posCurrent = pos;
	m_moveObject[index].posStart = pos;
	m_moveObject[index].posEnd = pos;
	MoveObjectStepIcon(index);
	if (speed != 0)
	{
		POINT tinyPoint = pos;
		int num2 = speed;
		int dist = 0;
		if (num2 > 50)
		{
			num2 -= 50;
			POINT dir;
			dir.x = 0;
			dir.y = 1;
			dist = SearchDistRight(tinyPoint, dir, type);
			tinyPoint.y += dist;
		}
		else if (num2 < -50)
		{
			num2 += 50;
			POINT dir;
			dir.x = 0;
			dir.y = -1;
			dist = SearchDistRight(tinyPoint, dir, type);
			tinyPoint.y -= dist;
		}
		else if (num2 > 0)
		{
			POINT dir;
			dir.x = 1;
			dir.y = 0;
			dist = SearchDistRight(tinyPoint, dir, type);
			tinyPoint.x += dist;
		}
		else if (num2 < 0)
		{
			POINT dir;
			dir.x = -1;
			dir.y = 0;
			dist = SearchDistRight(tinyPoint, dir, type);
			tinyPoint.x -= dist;
		}
		if (dist == 0)
		{
			if (type == TYPE_BALLE)
			{
				m_moveObject[index].type = 0;
				return TRUE;
			}
		}
		else
		{
			m_moveObject[index].posEnd = tinyPoint;
			m_moveObject[index].timeStopStart = 0;
			m_moveObject[index].stepAdvance = abs(dist * num2 / DIMOBJX);
			m_moveObject[index].step = STEP_ADVANCE;
			m_moveObject[index].time = 0;
		}
	}
	MoveObjectPriority(index);
	if (bMulti)
	{
		NetMessage msg;
		msg.type = PK_OBJECTSTART;
		msg.data1 = speed;
		msg.data2 = pos.x;
		msg.data3 = pos.y;
		msg.data4 = type;
		NetMessagePush(&msg);
	}
	return TRUE;
}

BOOL CDecor::ObjectDelete(POINT pos, int type, BOOL bMulti)
{
	NetMessage msg;

	int num = MoveObjectSearch(pos, type);
	if (num == -1)
	{
		return FALSE;
	}
	m_moveObject[num].type = 0;
	if (bMulti)
	{
		msg.type = PK_OBJECTDELETE;
		msg.data1 = 0;
		msg.data2 = pos.x;
		msg.data3 = pos.y;
		msg.data4 = type;

		NetMessagePush(&msg);
	}
	return TRUE;
}

void CDecor::MoveObjectStep()
{
	m_blupiVector = POINT( 0, 0 );
	m_blupiTransport = -1;
	for (int i = 0; i < MAXMOVEOBJECT; i++)
	{
		if (m_moveObject[i].type != 0)
		{
			MoveObjectStepLine(i);
			MoveObjectStepIcon(i);
			if (m_moveObject[i].type == TYPE_BULLDOZER ||
				m_moveObject[i].type == TYPE_BLUPITANK ||
				m_moveObject[i].type == TYPE_BLUPIHELICO)
			{
				int num = MovePersoDetect(m_moveObject[i].posCurrent);
				if (num != -1)
				{
					POINT posCurrent = m_moveObject[i].posCurrent;
					posCurrent.x -= 34;
					posCurrent.y -= 34;
					ObjectStart(posCurrent, TYPE_EXPLO1, 0, TRUE);
					PlaySound(SOUND_BOUM, m_moveObject[i].posCurrent);
					m_decorAction = 1;
					m_decorPhase = 0;
					posCurrent = m_moveObject[i].posCurrent;
					posCurrent.x += 2;
					posCurrent.y += BLUPIOFFY;
					ObjectDelete(m_moveObject[i].posCurrent, m_moveObject[i].type);
					ObjectStart(posCurrent, TYPE_CLEAR, 0);
					ObjectDelete(m_moveObject[num].posCurrent, m_moveObject[num].type);
				}
				if (BlupiElectro(m_moveObject[i].posCurrent))
				{
					POINT posCurrent = m_moveObject[i].posCurrent;
					posCurrent.x += 2;
					posCurrent.y += BLUPIOFFY;
					ObjectDelete(m_moveObject[i].posCurrent, m_moveObject[i].type);
					ObjectStart(posCurrent, TYPE_ELECTRO, 55);
					PlaySound(SOUND_ELECTRO, posCurrent);
				}
			}
		}
	}
}

void CDecor::MoveObjectStepLine(int i)
{
	MoveObject moveObject;
	POINT tinyPoint;
	BOOL flag = FALSE;
	RECT tinyRect = { 0, 0, 0, 0 };
	if (m_moveObject[i].type == TYPE_ASCENSEUR ||
		m_moveObject[i].type == TYPE_ASCENSEURs ||
		m_moveObject[i].type == TYPE_ASCENSEURsi &&
		!m_blupiSuspend)
	{
		RECT src;
		src.left = m_blupiPos.x + 20;
		src.right = m_blupiPos.x + DIMBLUPIX - 20;
		src.top = m_blupiPos.y + DIMBLUPIY - 2;
		src.bottom = m_blupiPos.y + DIMBLUPIY - 1;
		tinyRect.left = m_moveObject[i].posCurrent.x;
		tinyRect.right = m_moveObject[i].posCurrent.x + DIMOBJX;
		tinyRect.top = m_moveObject[i].posCurrent.y;
		tinyRect.bottom = m_moveObject[i].posCurrent.y + 16;
		RECT tinyRect2;
		flag = IntersectRect(&tinyRect2, &tinyRect, &src);
		tinyPoint = m_moveObject[i].posCurrent;
	}
	POINT posCurrent;
	if (m_blupiFocus && !m_blupiHide && m_moveObject[i].type == TYPE_BOMBEFOLLOW2)
	{
		posCurrent = m_moveObject[i].posCurrent;
		if (posCurrent.x < m_blupiPos.x)
		{
			posCurrent.x++;
		}
		if (posCurrent.x > m_blupiPos.x)
		{
			posCurrent.x--;
		}
		if (posCurrent.y < m_blupiPos.y)
		{
			posCurrent.y++;
		}
		if (posCurrent.y > m_blupiPos.y)
		{
			posCurrent.y--;
		}
		tinyRect.left = posCurrent.x + 10;
		tinyRect.right = posCurrent.x + DIMBLUPIX - 10;
		tinyRect.top = posCurrent.y + 10;
		tinyRect.bottom = posCurrent.y + DIMBLUPIX - 10;
		if (TestPath(tinyRect, m_moveObject[i].posCurrent, &posCurrent))
		{
			m_moveObject[i].posCurrent = posCurrent;
			m_moveObject[i].posStart = posCurrent;
			m_moveObject[i].posEnd = posCurrent;
		}
		else
		{
			ObjectDelete(m_moveObject[i].posCurrent, m_moveObject[i].type);
			posCurrent.x -= DIMOBJX / 2 + 2;
			posCurrent.y -= DIMOBJY / 2 + 2;
			ObjectStart(posCurrent, TYPE_EXPLO2, 0);
			PlaySound(SOUND_BOUM, posCurrent);
			m_decorAction = 1;
			m_decorPhase = 0;
		}
	}
	if (m_moveObject[i].posStart.x != m_moveObject[i].posEnd.x || m_moveObject[i].posStart.y != m_moveObject[i].posEnd.y)
	{
		if (m_moveObject[i].step == STEP_STOPSTART)
		{
			if (m_moveObject[i].time < m_moveObject[i].timeStopStart)
			{
				m_moveObject[i].time ++;
			}
			else
			{
				m_moveObject[i].step = STEP_ADVANCE;
				m_moveObject[i].time = 0;
			}
		}
		else if (m_moveObject[i].step == STEP_ADVANCE)
		{
			if (m_moveObject[i].posCurrent.x != m_moveObject[i].posEnd.x || m_moveObject[i].posCurrent.y != m_moveObject[i].posEnd.y)
			{
				m_moveObject[i].time ++;
				if (m_moveObject[i].stepAdvance != 0)
				{
					m_moveObject[i].posCurrent = (m_moveObject[i].posEnd - m_moveObject[i].posStart) * m_moveObject[i].time / m_moveObject[i].stepAdvance + m_moveObject[i].posStart;
				}

			}
			else if (m_moveObject[i].type == TYPE_BLUP || m_moveObject[i].type == TYPE_BALLE)
			{
				m_moveObject[i].type = 0;
			}
			else if (m_moveObject[i].type == TYPE_GLU)
			{
				m_moveObject[i].posStart = m_moveObject[i].posCurrent;
				m_moveObject[i].posEnd = m_moveObject[i].posCurrent;
				m_moveObject[i].step = STEP_STOPEND;
				m_moveObject[i].time = 0;
			}
			else
			{
				m_moveObject[i].step = STEP_STOPEND;
				m_moveObject[i].time = 0;
			}
		}
		else if (m_moveObject[i].step == STEP_STOPEND)
		{
			if (m_moveObject[i].time < m_moveObject[i].timeStopEnd)
			{
				m_moveObject[i].time ++;
			}
			else
			{
				m_moveObject[i].step = STEP_RECEDE;
				m_moveObject[i].time = 0;
			}
		}
		else if (m_moveObject[i].step == STEP_RECEDE)
		{
			if (m_moveObject[i].posCurrent.x != m_moveObject[i].posStart.x || m_moveObject[i].posCurrent.y != m_moveObject[i].posStart.y)
			{
				m_moveObject[i].time ++;
				if (m_moveObject[i].stepRecede != 0)
				{
					m_moveObject[i].posCurrent = (m_moveObject[i].posStart - m_moveObject[i].posEnd) * m_moveObject[i].time / m_moveObject[i].stepRecede + m_moveObject[i].posEnd;
				}
			}
			else
			{
				m_moveObject[i].step = STEP_STOPSTART;
				m_moveObject[i].time = 0;
			}
		}
	}
	if (m_moveObject[i].type == TYPE_DOOR && m_moveObject[i].step == STEP_STOPEND)
	{
		m_moveObject[i].type = 0;
	}
	posCurrent = m_moveObject[i].posCurrent;
	if (m_moveObject[i].type == TYPE_ASCENSEUR ||
		m_moveObject[i].type == TYPE_ASCENSEURs ||
		m_moveObject[i].type == TYPE_ASCENSEURsi)
	{
		posCurrent.y -= DIMOBJY;
	}
	posCurrent.x = (posCurrent.x + DIMOBJX / 2) / DIMOBJX;
	posCurrent.y = (posCurrent.y + DIMOBJY / 2) / DIMOBJY;
	SetMoveTraj(posCurrent);
	if (flag)
	{
		m_blupiVector.x = m_moveObject[i].posCurrent.x - tinyPoint.x;
		m_blupiVector.y = m_moveObject[i].posCurrent.y - (m_blupiPos.y + DIMBLUPIY - BLUPIFLOOR);
		if (m_moveObject[i].type == TYPE_ASCENSEURs)
		{
			m_blupiVector.x += 2;
		}
		if (m_moveObject[i].type == TYPE_ASCENSEURsi)
		{
			m_blupiVector.x -= 2;
		}
		if (m_blupiTimeNoAsc == 0)
		{
			m_blupiTransport = i;
		}
	}
}

void CDecor::MoveObjectStepIcon(int i)
{
	POINT pos;
	if (m_moveObject[i].type == TYPE_ASCENSEURs)
	{
		m_moveObject[i].icon = table_chenille[m_moveObject[i].phase / 1 % 6];
	}
	if (m_moveObject[i].type == TYPE_ASCENSEURsi)
	{
		m_moveObject[i].icon = table_chenillei[m_moveObject[i].phase / 1 % 6];
	}
	if (m_moveObject[i].type == TYPE_BOMBEDOWN)
	{
		m_moveObject[i].icon = 12 + m_moveObject[i].phase / 2 % 9;
		m_moveObject[i].channel = CHELEMENT;
	}
	if (m_moveObject[i].type == TYPE_BOMBEUP)
	{
		m_moveObject[i].icon = 48 + m_moveObject[i].phase / 2 % 9;
		m_moveObject[i].channel = CHELEMENT;
	}
	if (m_moveObject[i].type == TYPE_BOMBEMOVE)
	{
		m_moveObject[i].icon = 69 + m_moveObject[i].phase / 1 % 9;
		m_moveObject[i].channel = CHELEMENT;
	}
	if (m_moveObject[i].type == TYPE_BOMBEFOLLOW1)
	{
		m_moveObject[i].icon = table_follow1[m_moveObject[i].phase / 1 % 26];
		m_moveObject[i].channel = CHELEMENT;
	}
	if (m_moveObject[i].type == TYPE_BOMBEFOLLOW2)
	{
		m_moveObject[i].icon = table_follow2[m_moveObject[i].phase / 1 % 5];
		m_moveObject[i].channel = CHELEMENT;
	}
	if (m_moveObject[i].type == TYPE_BOMBEPERSO1)
	{
		m_moveObject[i].icon = 257 + m_moveObject[i].phase / 1 % 6;
		m_moveObject[i].channel = CHBLUPI;
	}
	if (m_moveObject[i].type == TYPE_BOMBEPERSO2)
	{
		m_moveObject[i].icon = 257 + m_moveObject[i].phase / 1 % 6;
		m_moveObject[i].channel = CHBLUPI1;
	}
	if (m_moveObject[i].type == TYPE_BOMBEPERSO3)
	{
		m_moveObject[i].icon = 257 + m_moveObject[i].phase / 1 % 6;
		m_moveObject[i].channel = CHBLUPI2;
	}
	if (m_moveObject[i].type == TYPE_BOMBEPERSO4)
	{
		m_moveObject[i].icon = 257 + m_moveObject[i].phase / 1 % 6;
		m_moveObject[i].channel = CHBLUPI3;
	}
	if (m_moveObject[i].type == TYPE_DYNAMITE)
	{
		m_moveObject[i].icon = 252;
		m_moveObject[i].channel = CHELEMENT;
	}
	if (m_moveObject[i].type == TYPE_DYNAMITEf)
	{
		m_moveObject[i].icon = table_dynamitef[m_moveObject[i].phase / 1 % 100];
		m_moveObject[i].channel = CHELEMENT;
		if (m_moveObject[i].phase == 50)
		{
			DynamiteStart(i, 0, 0);
		}
		if (m_moveObject[i].phase == 53)
		{
			DynamiteStart(i, -100, 8);
		}
		if (m_moveObject[i].phase == 55)
		{
			DynamiteStart(i, 80, 10);
		}
		if (m_moveObject[i].phase == 56)
		{
			DynamiteStart(i, -15, -100);
		}
		if (m_moveObject[i].phase == 59)
		{
			DynamiteStart(i, 20, 70);
		}
		if (m_moveObject[i].phase == 62)
		{
			DynamiteStart(i, 30, -50);
		}
		if (m_moveObject[i].phase == 64)
		{
			DynamiteStart(i, -40, 30);
		}
		if (m_moveObject[i].phase == 67)
		{
			DynamiteStart(i, -180, 10);
		}
		if (m_moveObject[i].phase == 69)
		{
			DynamiteStart(i, 200, -10);
		}
		if (m_moveObject[i].phase >= 70)
		{
			m_moveObject[i].type = 0;
		}
	}
	if (m_moveObject[i].type == TYPE_TRESOR)
	{
		if (m_moveObject[i].phase / 3 % 22 < 11)
		{
			m_moveObject[i].icon = m_moveObject[i].phase / 3 % 11;
		}
		else
		{
			m_moveObject[i].icon = 11 - m_moveObject[i].phase / 3 % 11;
		}
		m_moveObject[i].channel = CHELEMENT;
	}
	if (m_moveObject[i].type == TYPE_EGG)
	{
		m_moveObject[i].icon = 21 + m_moveObject[i].phase / 4 % 8;
		m_moveObject[i].channel = CHELEMENT;
	}
	if (m_moveObject[i].type == TYPE_GOAL)
	{
		m_moveObject[i].icon = 29 + m_moveObject[i].phase / 3 % 8;
		m_moveObject[i].channel = CHELEMENT;
	}
	if (m_moveObject[i].type == TYPE_CLE)
	{
		m_moveObject[i].icon = table_cle[m_moveObject[i].phase / 3 % 12];
		m_moveObject[i].channel = CHELEMENT;
	}
	if (m_moveObject[i].type == TYPE_CLE1)
	{
		m_moveObject[i].icon = table_cle1[m_moveObject[i].phase / 3 % 12];
		m_moveObject[i].channel = CHELEMENT;
	}
	if (m_moveObject[i].type == TYPE_CLE2)
	{
		m_moveObject[i].icon = table_cle2[m_moveObject[i].phase / 3 % 12];
		m_moveObject[i].channel = CHELEMENT;
	}
	if (m_moveObject[i].type == TYPE_CLE3)
	{
		m_moveObject[i].icon = table_cle3[m_moveObject[i].phase / 3 % 12];
		m_moveObject[i].channel = CHELEMENT;
	}
	if (m_moveObject[i].type == TYPE_SKATE)
	{
		m_moveObject[i].icon = table_skate[m_moveObject[i].phase / 1 % 34];
		m_moveObject[i].channel = CHELEMENT;
	}
	if (m_moveObject[i].type == TYPE_SHIELD)
	{
		m_moveObject[i].icon = table_shield[m_moveObject[i].phase / 2 % 16];
		m_moveObject[i].channel = CHELEMENT;
	}
	if (m_moveObject[i].type == TYPE_POWER)
	{
		m_moveObject[i].icon = table_power[m_moveObject[i].phase / 2 % 8];
		m_moveObject[i].channel = CHELEMENT;
	}
	if (m_moveObject[i].type == TYPE_INVERT)
	{
		m_moveObject[i].icon = table_invert[m_moveObject[i].phase / 2 % 20];
		m_moveObject[i].channel = CHELEMENT;
	}
	if (m_moveObject[i].type == TYPE_CHARGE)
	{
		m_moveObject[i].icon = table_charge[m_moveObject[i].phase / 2 % 6];
		m_moveObject[i].channel = CHOBJECT;
	}
	if (m_moveObject[i].type == TYPE_MAGICTRACK)
	{
		m_moveObject[i].icon = table_magictrack[m_moveObject[i].phase / 1 % 24];
		m_moveObject[i].channel = CHELEMENT;
		if (m_moveObject[i].phase >= 24)
		{
			m_moveObject[i].type = 0;
		}
	}
	if (m_moveObject[i].type == TYPE_SHIELDTRACK)
	{
		m_moveObject[i].icon = table_shieldtrack[m_moveObject[i].phase / 1 % 20];
		m_moveObject[i].channel = CHELEMENT;
		if (m_moveObject[i].phase >= 20)
		{
			m_moveObject[i].type = 0;
		}
	}
	if (m_moveObject[i].type == TYPE_TRESORTRACK)
	{
		m_moveObject[i].icon = table_tresortrack[m_moveObject[i].phase / 1 % 11];
		m_moveObject[i].channel = CHELEMENT;
		if (m_moveObject[i].phase >= 11)
		{
			m_moveObject[i].type = 0;
		}
	}
	if (m_moveObject[i].type == TYPE_HIDETRACK && m_moveObject[i].phase >= 20)
	{
		m_moveObject[i].type = 0;
	}
	if (m_moveObject[i].type == TYPE_EXPLO1)
	{
		if (m_moveObject[i].phase >= 36)
		{
			m_moveObject[i].type = 0;
		}
		else
		{
			m_moveObject[i].icon = table_explo1[m_moveObject[i].phase];
			m_moveObject[i].channel = CHEXPLO;
		}
	}
	if (m_moveObject[i].type == TYPE_EXPLO2)
	{
		if (m_moveObject[i].phase >= 20)
		{
			m_moveObject[i].type = 0;
		}
		else
		{
			m_moveObject[i].icon = table_explo2[m_moveObject[i].phase % 20];
			m_moveObject[i].channel = CHEXPLO;
		}
	}
	if (m_moveObject[i].type == TYPE_EXPLO3)
	{
		if (m_moveObject[i].phase >= 20)
		{
			m_moveObject[i].type = 0;
		}
		else
		{
			m_moveObject[i].icon = table_explo3[m_moveObject[i].phase / 1 % 20];
			m_moveObject[i].channel = CHEXPLO;
		}
	}
	if (m_moveObject[i].type == TYPE_EXPLO4)
	{
		if (m_moveObject[i].phase >= 9)
		{
			m_moveObject[i].type = 0;
		}
		else
		{
			m_moveObject[i].icon = table_explo4[m_moveObject[i].phase / 1 % 9];
			m_moveObject[i].channel = CHEXPLO;
		}
	}
	if (m_moveObject[i].type == TYPE_EXPLO5)
	{
		if (m_moveObject[i].phase >= 12)
		{
			m_moveObject[i].type = 0;
		}
		else
		{
			m_moveObject[i].icon = table_explo5[m_moveObject[i].phase / 1 % 12];
			m_moveObject[i].channel = CHEXPLO;
		}
	}
	if (m_moveObject[i].type == TYPE_EXPLO6)
	{
		if (m_moveObject[i].phase >= 6)
		{
			m_moveObject[i].type = 0;
		}
		else
		{
			m_moveObject[i].icon = table_explo6[m_moveObject[i].phase / 1 % 6];
			m_moveObject[i].channel = CHEXPLO;
		}
	}
	if (m_moveObject[i].type == TYPE_EXPLO7)
	{
		if (m_moveObject[i].phase >= 128)
		{
			m_moveObject[i].type = 0;
		}
		else
		{
			m_moveObject[i].icon = table_explo7[m_moveObject[i].phase / 1 % 128];
			m_moveObject[i].channel = CHEXPLO;
		}
	}
	if (m_moveObject[i].type == TYPE_EXPLO8)
	{
		if (m_moveObject[i].phase >= 5)
		{
			m_moveObject[i].type = 0;
		}
		else
		{
			m_moveObject[i].icon = table_explo8[m_moveObject[i].phase / 1 % 5];
			m_moveObject[i].channel = CHEXPLO;
		}
	}
	if (m_moveObject[i].type == TYPE_SPLOUTCH1)
	{
		if (m_moveObject[i].phase >= 10)
		{
			m_moveObject[i].type = 0;
		}
		else
		{
			m_moveObject[i].icon = table_sploutch1[m_moveObject[i].phase / 1 % 10];
			m_moveObject[i].channel = CHEXPLO;
		}
	}
	if (m_moveObject[i].type == TYPE_SPLOUTCH2)
	{
		if (m_moveObject[i].phase >= 13)
		{
			m_moveObject[i].type = 0;
		}
		else
		{
			m_moveObject[i].icon = table_sploutch2[m_moveObject[i].phase / 1 % 13];
			m_moveObject[i].channel = CHEXPLO;
		}
	}
	if (m_moveObject[i].type == TYPE_SPLOUTCH3)
	{
		if (m_moveObject[i].phase >= 18)
		{
			m_moveObject[i].type = 0;
		}
		else
		{
			m_moveObject[i].icon = table_sploutch3[m_moveObject[i].phase / 1 % 18];
			m_moveObject[i].channel = CHEXPLO;
		}
	}
	if (m_moveObject[i].type == TYPE_TENTACULE)
	{
		if (m_moveObject[i].phase >= 90)
		{
			m_moveObject[i].type = 0;
		}
		else
		{
			m_moveObject[i].icon = table_tentacule[m_moveObject[i].phase / 2 % 45];
			m_moveObject[i].channel = CHEXPLO;
		}
	}
	if (m_moveObject[i].type == TYPE_BRIDGE)
	{
		if (m_moveObject[i].phase == 0)
		{
			PlaySound(SOUND_BRIDGE1, m_moveObject[i].posStart);
		}
		if (m_moveObject[i].phase == 137)
		{
			PlaySound(SOUND_BRIDGE2, m_moveObject[i].posStart);
		}
		if (m_moveObject[i].phase >= 157)
		{
			m_moveObject[i].type = 0;
		}
		else
		{
			m_moveObject[i].icon = table_bridge[m_moveObject[i].phase / 1 % 157];
			m_moveObject[i].channel = CHOBJECT;
			pos.x = m_moveObject[i].posStart.x / DIMOBJX;
			pos.y = m_moveObject[i].posStart.y / DIMOBJY;
			m_decor[pos.x][pos.y].icon = m_moveObject[i].icon;
		}
	}
	if (m_moveObject[i].type == TYPE_POLLUTION)
	{
		if (m_moveObject[i].phase >= 16)
		{
			m_moveObject[i].type = 0;
		}
		else
		{
			m_moveObject[i].icon = table_pollution[m_moveObject[i].phase / 2 % 8];
			m_moveObject[i].channel = CHELEMENT;
		}
	}
	if (m_moveObject[i].type == TYPE_INVERTSTART)
	{
		if (m_moveObject[i].phase >= 16)
		{
			m_moveObject[i].type = 0;
		}
		else
		{
			m_moveObject[i].icon = table_invertstart[m_moveObject[i].phase / 2 % 8];
			m_moveObject[i].channel = CHELEMENT;
		}
	}
	if (m_moveObject[i].type == TYPE_INVERTSTOP)
	{
		if (m_moveObject[i].phase >= 16)
		{
			m_moveObject[i].type = 0;
		}
		else
		{
			m_moveObject[i].icon = table_invertstop[m_moveObject[i].phase / 2 % 8];
			m_moveObject[i].channel = CHELEMENT;
		}
	}
	if (m_moveObject[i].type == TYPE_PLOUF)
	{
		if (m_moveObject[i].phase >= 14)
		{
			m_moveObject[i].type = 0;
		}
		else
		{
			m_moveObject[i].icon = table_plouf[m_moveObject[i].phase / 2 % 7];
			m_moveObject[i].channel = CHOBJECT;
		}
	}
	if (m_moveObject[i].type == TYPE_TIPLOUF)
	{
		if (m_moveObject[i].phase >= 6)
		{
			m_moveObject[i].type = 0;
		}
		else
		{
			m_moveObject[i].icon = table_tiplouf[m_moveObject[i].phase / 2 % 7];
			m_moveObject[i].channel = CHOBJECT;
		}
	}
	if (m_moveObject[i].type == TYPE_BLUP)
	{
		m_moveObject[i].icon = table_blup[m_moveObject[i].phase / 2 % 20];
		m_moveObject[i].channel = CHOBJECT;
	}
	if (m_moveObject[i].type == TYPE_BULLDOZER)
	{
		if (m_moveObject[i].posStart.x > m_moveObject[i].posEnd.x)
		{
			if (m_moveObject[i].step == STEP_STOPSTART)
			{
				m_moveObject[i].icon = table_bulldozer_turn2l[m_moveObject[i].time % 22];
			}
			if (m_moveObject[i].step == STEP_STOPEND)
			{
				m_moveObject[i].icon = table_bulldozer_turn2r[m_moveObject[i].time % 22];
			}
			if (m_moveObject[i].step == STEP_ADVANCE)
			{
				m_moveObject[i].icon = table_bulldozer_left[m_moveObject[i].time % 8];
			}
			if (m_moveObject[i].step == STEP_RECEDE)
			{
				m_moveObject[i].icon = table_bulldozer_right[m_moveObject[i].time % 8];
			}
		}
		else
		{
			if (m_moveObject[i].step == STEP_STOPSTART)
			{
				m_moveObject[i].icon = table_bulldozer_turn2r[m_moveObject[i].time % 22];
			}
			if (m_moveObject[i].step == STEP_STOPEND)
			{
				m_moveObject[i].icon = table_bulldozer_turn2l[m_moveObject[i].time % 22];
			}
			if (m_moveObject[i].step == STEP_ADVANCE)
			{
				m_moveObject[i].icon = table_bulldozer_right[m_moveObject[i].time % 8];
			}
			if (m_moveObject[i].step == STEP_RECEDE)
			{
				m_moveObject[i].icon = table_bulldozer_left[m_moveObject[i].time % 8];
			}
		}
		m_moveObject[i].channel = CHELEMENT;
	}
	if (m_moveObject[i].type == TYPE_POISSON)
	{
		if (m_moveObject[i].posStart.x > m_moveObject[i].posEnd.x)
		{
			if (m_moveObject[i].step == STEP_STOPSTART)
			{
				m_moveObject[i].icon = table_poisson_turn2l[m_moveObject[i].time % 48];
			}
			if (m_moveObject[i].step == STEP_STOPEND)
			{
				m_moveObject[i].icon = table_poisson_turn2r[m_moveObject[i].time % 48];
			}
			if (m_moveObject[i].step == STEP_ADVANCE)
			{
				m_moveObject[i].icon = table_poisson_left[m_moveObject[i].time % 8];
			}
			if (m_moveObject[i].step == STEP_RECEDE)
			{
				m_moveObject[i].icon = table_poisson_right[m_moveObject[i].time % 8];
			}
		}
		else
		{
			if (m_moveObject[i].step == STEP_STOPSTART)
			{
				m_moveObject[i].icon = table_poisson_turn2r[m_moveObject[i].time % 48];
			}
			if (m_moveObject[i].step == STEP_STOPEND)
			{
				m_moveObject[i].icon = table_poisson_turn2l[m_moveObject[i].time % 48];
			}
			if (m_moveObject[i].step == STEP_ADVANCE)
			{
				m_moveObject[i].icon = table_poisson_right[m_moveObject[i].time % 8];
			}
			if (m_moveObject[i].step == STEP_RECEDE)
			{
				m_moveObject[i].icon = table_poisson_left[m_moveObject[i].time % 8];
			}
		}
		m_moveObject[i].channel = CHELEMENT;
	}
	if (m_moveObject[i].type == TYPE_OISEAU)
	{
		if (m_moveObject[i].posStart.x > m_moveObject[i].posEnd.x)
		{
			if (m_moveObject[i].step == STEP_STOPSTART)
			{
				m_moveObject[i].icon = table_oiseau_turn2l[m_moveObject[i].time % 10];
			}
			if (m_moveObject[i].step == STEP_STOPEND)
			{
				m_moveObject[i].icon = table_oiseau_turn2r[m_moveObject[i].time % 10];
			}
			if (m_moveObject[i].step == STEP_ADVANCE)
			{
				m_moveObject[i].icon = table_oiseau_left[m_moveObject[i].time % 8];
			}
			if (m_moveObject[i].step == STEP_RECEDE)
			{
				m_moveObject[i].icon = table_oiseau_right[m_moveObject[i].time % 8];
			}
		}
		else
		{
			if (m_moveObject[i].step == STEP_STOPSTART)
			{
				m_moveObject[i].icon = table_oiseau_turn2r[m_moveObject[i].time % 10];
			}
			if (m_moveObject[i].step == STEP_STOPEND)
			{
				m_moveObject[i].icon = table_oiseau_turn2l[m_moveObject[i].time % 10];
			}
			if (m_moveObject[i].step == STEP_ADVANCE)
			{
				m_moveObject[i].icon = table_oiseau_right[m_moveObject[i].time % 8];
			}
			if (m_moveObject[i].step == STEP_RECEDE)
			{
				m_moveObject[i].icon = table_oiseau_left[m_moveObject[i].time % 8];
			}
		}
		m_moveObject[i].channel = CHELEMENT;
	}
	if (m_moveObject[i].type == TYPE_GUEPE)
	{
		if (m_moveObject[i].posStart.x > m_moveObject[i].posEnd.x)
		{
			if (m_moveObject[i].step == STEP_STOPSTART)
			{
				m_moveObject[i].icon = table_guepe_turn2l[m_moveObject[i].time % 5];
			}
			if (m_moveObject[i].step == STEP_STOPEND)
			{
				m_moveObject[i].icon = table_guepe_turn2r[m_moveObject[i].time % 5];
			}
			if (m_moveObject[i].step == STEP_ADVANCE)
			{
				m_moveObject[i].icon = table_guepe_left[m_moveObject[i].time % 6];
			}
			if (m_moveObject[i].step == STEP_RECEDE)
			{
				m_moveObject[i].icon = table_guepe_right[m_moveObject[i].time % 6];
			}
		}
		else
		{
			if (m_moveObject[i].step == STEP_STOPSTART)
			{
				m_moveObject[i].icon = table_guepe_turn2r[m_moveObject[i].time % 5];
			}
			if (m_moveObject[i].step == STEP_STOPEND)
			{
				m_moveObject[i].icon = table_guepe_turn2l[m_moveObject[i].time % 5];
			}
			if (m_moveObject[i].step == STEP_ADVANCE)
			{
				m_moveObject[i].icon = table_guepe_right[m_moveObject[i].time % 6];
			}
			if (m_moveObject[i].step == STEP_RECEDE)
			{
				m_moveObject[i].icon = table_guepe_left[m_moveObject[i].time % 6];
			}
		}
		m_moveObject[i].channel = CHELEMENT;
	}
	if (m_moveObject[i].type == TYPE_CREATURE)
	{
		if (m_moveObject[i].posStart.x > m_moveObject[i].posEnd.x)
		{
			if (m_moveObject[i].step == STEP_STOPSTART)
			{
				m_moveObject[i].icon = table_creature_turn2[m_moveObject[i].time % 200];
			}
			if (m_moveObject[i].step == STEP_STOPEND)
			{
				m_moveObject[i].icon = table_creature_turn2[m_moveObject[i].time % 200];
			}
			if (m_moveObject[i].step == STEP_ADVANCE)
			{
				m_moveObject[i].icon = table_creature_left[m_moveObject[i].time % 8];
			}
			if (m_moveObject[i].step == STEP_RECEDE)
			{
				m_moveObject[i].icon = table_creature_right[m_moveObject[i].time % 8];
			}
		}
		else
		{
			if (m_moveObject[i].step == STEP_STOPSTART)
			{
				m_moveObject[i].icon = table_creature_turn2[m_moveObject[i].time % 200];
			}
			if (m_moveObject[i].step == STEP_STOPEND)
			{
				m_moveObject[i].icon = table_creature_turn2[m_moveObject[i].time % 200];
			}
			if (m_moveObject[i].step == STEP_ADVANCE)
			{
				m_moveObject[i].icon = table_creature_right[m_moveObject[i].time % 8];
			}
			if (m_moveObject[i].step == STEP_RECEDE)
			{
				m_moveObject[i].icon = table_creature_left[m_moveObject[i].time % 8];
			}
		}
		m_moveObject[i].channel = CHELEMENT;
	}
	if (m_moveObject[i].type == TYPE_BLUPIHELICO)
	{
		if (m_moveObject[i].posStart.x > m_moveObject[i].posEnd.x)
		{
			if (m_moveObject[i].step == STEP_STOPSTART)
			{
				m_moveObject[i].icon = table_blupih_turn2l[m_moveObject[i].time % 26];
			}
			if (m_moveObject[i].step == STEP_STOPEND)
			{
				m_moveObject[i].icon = table_blupih_turn2r[m_moveObject[i].time % 26];
			}
			if (m_moveObject[i].step == STEP_ADVANCE)
			{
				m_moveObject[i].icon = table_blupih_left[m_moveObject[i].time % 8];
			}
			if (m_moveObject[i].step == STEP_RECEDE)
			{
				m_moveObject[i].icon = table_blupih_right[m_moveObject[i].time % 8];
			}
		}
		else
		{
			if (m_moveObject[i].step == STEP_STOPSTART)
			{
				m_moveObject[i].icon = table_blupih_turn2r[m_moveObject[i].time % 26];
			}
			if (m_moveObject[i].step == STEP_STOPEND)
			{
				m_moveObject[i].icon = table_blupih_turn2l[m_moveObject[i].time % 26];
			}
			if (m_moveObject[i].step == STEP_ADVANCE)
			{
				m_moveObject[i].icon = table_blupih_right[m_moveObject[i].time % 8];
			}
			if (m_moveObject[i].step == STEP_RECEDE)
			{
				m_moveObject[i].icon = table_blupih_left[m_moveObject[i].time % 8];
			}
		}
		if ((m_moveObject[i].step == STEP_STOPSTART || m_moveObject[i].step == STEP_STOPEND) && m_moveObject[i].time == 21)
		{
			pos.x = m_moveObject[i].posCurrent.x;
			pos.y = m_moveObject[i].posCurrent.y + 40;
			if (ObjectStart(pos, TYPE_BALLE, 55) != -1)
			{
				PlaySound(SOUND_FIREOK, pos);
			}
		}
	}
	if (m_moveObject[i].type == TYPE_BLUPITANK)
	{
		if (m_moveObject[i].posStart.x > m_moveObject[i].posEnd.x)
		{
			if (m_moveObject[i].step == STEP_STOPSTART)
			{
				m_moveObject[i].icon = table_blupit_turn2l[m_moveObject[i].time % 24];
			}
			if (m_moveObject[i].step == STEP_STOPEND)
			{
				m_moveObject[i].icon = table_blupit_turn2r[m_moveObject[i].time % 24];
			}
			if (m_moveObject[i].step == STEP_ADVANCE)
			{
				m_moveObject[i].icon = table_blupit_left[m_moveObject[i].time % 8];
			}
			if (m_moveObject[i].step == STEP_RECEDE)
			{
				m_moveObject[i].icon = table_blupit_right[m_moveObject[i].time % 8];
			}
		}
		else
		{
			if (m_moveObject[i].step == STEP_STOPSTART)
			{
				m_moveObject[i].icon = table_blupit_turn2r[m_moveObject[i].time % 24];
			}
			if (m_moveObject[i].step == STEP_STOPEND)
			{
				m_moveObject[i].icon = table_blupit_turn2l[m_moveObject[i].time % 24];
			}
			if (m_moveObject[i].step == STEP_ADVANCE)
			{
				m_moveObject[i].icon = table_blupit_right[m_moveObject[i].time % 8];
			}
			if (m_moveObject[i].step == STEP_RECEDE)
			{
				m_moveObject[i].icon = table_blupit_left[m_moveObject[i].time % 8];
			}
		}
		if ((m_moveObject[i].step == STEP_STOPSTART || m_moveObject[i].step == STEP_STOPEND) && m_moveObject[i].time == 3)
		{
			int speed;
			if ((m_moveObject[i].posStart.x < m_moveObject[i].posEnd.x && m_moveObject[i].step == STEP_STOPSTART) || (m_moveObject[i].posStart.x > m_moveObject[i].posEnd.x && m_moveObject[i].step == STEP_STOPEND))
			{
				pos.x = m_moveObject[i].posCurrent.x - 30;
				pos.y = m_moveObject[i].posCurrent.y + BLUPIOFFY;
				speed = -5;
			}
			else
			{
				pos.x = m_moveObject[i].posCurrent.x + 30;
				pos.y = m_moveObject[i].posCurrent.y + BLUPIOFFY;
				speed = 5;
			}
			if (ObjectStart(pos, TYPE_BALLE, speed) != -1)
			{
				PlaySound(SOUND_FIREOK, pos);
			}
		}
		if ((m_moveObject[i].step == STEP_STOPSTART || m_moveObject[i].step == STEP_STOPEND) && m_moveObject[i].time == 21)
		{
			int speed;
			if ((m_moveObject[i].posStart.x < m_moveObject[i].posEnd.x && m_moveObject[i].step == STEP_STOPSTART) || (m_moveObject[i].posStart.x > m_moveObject[i].posEnd.x && m_moveObject[i].step == STEP_STOPEND))
			{
				pos.x = m_moveObject[i].posCurrent.x + 30;
				pos.y = m_moveObject[i].posCurrent.y + BLUPIOFFY;
				speed = 5;
			}
			else
			{
				pos.x = m_moveObject[i].posCurrent.x - 30;
				pos.y = m_moveObject[i].posCurrent.y + BLUPIOFFY;
				speed = -5;
			}
			if (ObjectStart(pos, TYPE_BALLE, speed) != -1)
			{
				PlaySound(SOUND_FIREOK, pos);
			}
		}
	}
	if (m_moveObject[i].type == TYPE_GLU)
	{
		m_moveObject[i].icon = table_glu[m_moveObject[i].phase / 1 % 25];
		m_moveObject[i].channel = CHELEMENT;
	}
	if (m_moveObject[i].type == TYPE_CLEAR)
	{
		if (m_moveObject[i].phase >= 70)
		{
			m_moveObject[i].type = 0;
		}
		else
		{
			m_moveObject[i].icon = table_clear[m_moveObject[i].phase / 1 % 70];
			m_moveObject[i].channel = CHELEMENT;
		}
	}
	if (m_moveObject[i].type == TYPE_ELECTRO)
	{
		if (m_moveObject[i].phase >= 90)
		{
			m_moveObject[i].type = 0;
		}
		else
		{
			m_moveObject[i].icon = table_electro[m_moveObject[i].phase / 1 % 90];
			if (m_moveObject[i].phase < 30)
			{
				m_moveObject[i].channel = CHBLUPI2;
			}
			else
			{
				m_moveObject[i].channel = CHELEMENT;
			}
		}
	}
	if (m_moveObject[i].type == TYPE_HELICO)
	{
		m_moveObject[i].icon = 68;
		m_moveObject[i].channel = CHELEMENT;
	}
	if (m_moveObject[i].type == TYPE_OVER)
	{
		m_moveObject[i].icon = 208;
		m_moveObject[i].channel = CHELEMENT;
	}
	if (m_moveObject[i].type == TYPE_JEEP)
	{
		m_moveObject[i].icon = 89;
		m_moveObject[i].channel = CHELEMENT;
	}
	if (m_moveObject[i].type == TYPE_TANK)
	{
		m_moveObject[i].icon = 167;
		m_moveObject[i].channel = CHELEMENT;
	}
	if (m_moveObject[i].type == TYPE_BALLE)
	{
		m_moveObject[i].icon = 176;
		m_moveObject[i].channel = CHELEMENT;
	}
	if (m_moveObject[i].type == TYPE_BULLET)
	{
		m_moveObject[i].icon = 177;
		m_moveObject[i].channel = CHELEMENT;
	}
	if (m_moveObject[i].type == TYPE_DRINK)
	{
		m_moveObject[i].icon = 178;
		m_moveObject[i].channel = CHELEMENT;
	}
	m_moveObject[i].phase ++;
	if (m_moveObject[i].phase > 32700)
	{
		m_moveObject[i].phase = 0;
	}
}

void CDecor::DynamiteStart(int i, int dx, int dy)
{
	POINT posStart = m_moveObject[i].posStart;
	posStart.x -= 34;
	posStart.y -= 34;
	posStart.x += dx;
	posStart.y -= dy;
	ObjectStart(posStart, TYPE_EXPLO1, 0);
	if (dx == 0 && dy == 0)
	{
		PlaySound(SOUND_BOUM, posStart);
		m_decorAction = 1;
		m_decorPhase = 0;
	}
	RECT src;
	src.left = posStart.x;
	src.right = posStart.x + DIMOBJX * 2;
	src.top = posStart.y;
	src.bottom = posStart.y + DIMOBJY * 2;
	POINT tinyPoint;
	tinyPoint.y = posStart.y / DIMOBJY;
	for (int j = 0; j < 2; j++)
	{
		tinyPoint.x = posStart.x / DIMOBJX;
		for (int k = 0; k < 2; k++)
		{
			if (tinyPoint.x >= 0 && tinyPoint.x < MAXCELX && tinyPoint.y >= 0 && tinyPoint.y < MAXCELY)
			{
				int icon = m_decor[tinyPoint.x][tinyPoint.y].icon;
				if (icon == 378 || icon == 379 || icon == 404 || icon == 410)
				{
					POINT pos;
					pos.x = tinyPoint.x * DIMOBJX;
					pos.y = tinyPoint.y * DIMOBJY;
					ModifDecor(pos, -1);
				}
			}
			tinyPoint.x++;
		}
		tinyPoint.y++;
	}
	for (i = 0; i < MAXMOVEOBJECT; i++)
	{
		if (m_moveObject[i].type == TYPE_BOMBEDOWN ||
			m_moveObject[i].type == TYPE_BOMBEUP ||
			m_moveObject[i].type == TYPE_BOMBEFOLLOW1 ||
			m_moveObject[i].type == TYPE_BOMBEFOLLOW2 ||
			m_moveObject[i].type == TYPE_BULLDOZER ||
			m_moveObject[i].type == TYPE_EGG ||
			m_moveObject[i].type == TYPE_CAISSE ||
			m_moveObject[i].type == TYPE_HELICO ||
			m_moveObject[i].type == TYPE_BOMBEMOVE ||
			m_moveObject[i].type == TYPE_POISSON ||
			m_moveObject[i].type == TYPE_TOMATES ||
			m_moveObject[i].type == TYPE_JEEP ||
			m_moveObject[i].type == TYPE_OISEAU ||
			m_moveObject[i].type == TYPE_SKATE ||
			m_moveObject[i].type == TYPE_SHIELD ||
			m_moveObject[i].type == TYPE_POWER ||
			m_moveObject[i].type == TYPE_TANK ||
			m_moveObject[i].type == TYPE_DRINK ||
			m_moveObject[i].type == TYPE_BLUPIHELICO ||
			m_moveObject[i].type == TYPE_BLUPITANK ||
			m_moveObject[i].type == TYPE_GLU ||
			m_moveObject[i].type == TYPE_INVERT ||
			m_moveObject[i].type == TYPE_GUEPE ||
			m_moveObject[i].type == TYPE_OVER ||
			m_moveObject[i].type == TYPE_BRIDGE ||
			m_moveObject[i].type == TYPE_CREATURE ||
			m_moveObject[i].type == TYPE_BOMBEPERSO1 ||
			m_moveObject[i].type == TYPE_BOMBEPERSO2 ||
			m_moveObject[i].type == TYPE_BOMBEPERSO3 ||
			m_moveObject[i].type == TYPE_BOMBEPERSO4)
		{
			RECT src2;
			src2.left = m_moveObject[i].posCurrent.x;
			src2.right = m_moveObject[i].posCurrent.x + 60;
			src2.top = m_moveObject[i].posCurrent.y;
			src2.bottom = m_moveObject[i].posCurrent.y + 20;
			RECT tinyRect = { 0, 0, 0, 0 };
			if (IntersectRect(&tinyRect, &src2, &src))
			{
				if (m_moveObject[i].type == TYPE_CAISSE)
				{
					SearchLinkCaisse(i, TRUE);
					for (int L = 0; L < m_nbLinkCaisse; L++)
					{
						int channel = m_moveObject[m_linkCaisse[L]].channel;
						int icon2 = m_moveObject[m_linkCaisse[L]].icon;
						POINT posCurrent = m_moveObject[m_linkCaisse[L]].posCurrent;
						m_moveObject[m_linkCaisse[L]].type = 0;
					}
					ObjectDelete(m_moveObject[i].posCurrent, m_moveObject[i].type);
					UpdateCaisse();
				}
				else
				{
					ObjectDelete(m_moveObject[i].posCurrent, m_moveObject[i].type);
				}
			}
		}
	}
	if (m_blupiFocus && !m_blupiShield && !m_blupiHide && !m_bSuperBlupi && m_blupiPos.x > posStart.x - 30 && m_blupiPos.x < posStart.x + 30 + DIMOBJX && m_blupiPos.y > posStart.y - 30 && m_blupiPos.y < posStart.y + 30 + DIMOBJY)
	{
		BlupiDead(ACTION_CLEAR1, -1);
		m_blupiAir = TRUE;
	}

}

int CDecor::AscenseurDetect(RECT rect, POINT oldpos, POINT newpos)
{
	if (m_blupiTimeNoAsc != 0)
	{
		return -1;
	}
	int dy = abs(newpos.y - oldpos.y);
	int dirY = newpos.y < oldpos.y ? -1 : 1;

	for (int i = 0; i < MAXMOVEOBJECT; i++)
	{
		if (m_moveObject[i].type == TYPE_ASCENSEUR ||
			m_moveObject[i].type == TYPE_ASCENSEURs ||
			m_moveObject[i].type == TYPE_ASCENSEURsi)
		{
			RECT src;
			src.left = m_moveObject[i].posCurrent.x;
			src.right = m_moveObject[i].posCurrent.x + DIMOBJX;
			src.top = m_moveObject[i].posCurrent.y;
			src.bottom = m_moveObject[i].posCurrent.y + 16;
			if (dy < (DIMBLUPIY / 2))
			{
				RECT dest = { 0, 0, 0, 0 };
				if (IntersectRect(&dest, &src, &rect))
				{
					return i;
				}
			}
			else
			{
				RECT src2 = rect;
				src2.top -= dy / (DIMBLUPIY / 2) * (DIMBLUPIY / 2) * dirY;
				src2.bottom -= dy / (DIMBLUPIY / 2) * (DIMBLUPIY / 2) * dirY;
				for (int j = 0; j <= dy / (DIMBLUPIY / 2); j++)
				{
					RECT dest = { 0, 0, 0, 0 };
					if (IntersectRect(&dest, &src, &src2))
					{
						return i;
					}
					src2.top += dirY * (DIMBLUPIY / 2);
					src2.bottom += dy;
				}
			}

		}
	}
	return -1;
}

void CDecor::AscenseurVertigo(int i, BOOL* pbVertigoLeft, BOOL* pbVertigoRight)
{
	*pbVertigoLeft = FALSE;
	
	*pbVertigoRight = FALSE;
	if (m_blupiPos.x + 20 + 4 < m_moveObject[i].posCurrent.x)
	{
		*pbVertigoLeft = TRUE;
	}
	if (m_blupiPos.x + DIMBLUPIX - 20 - 4 > m_moveObject[i].posCurrent.x + DIMOBJX)
	{
		*pbVertigoRight = TRUE;
	}
	if (AscenseurShift(i))
	{
		if (*pbVertigoLeft)
		{
			*pbVertigoLeft = FALSE;
			*pbVertigoRight = TRUE;
			m_blupiTimeNoAsc = 10;
			return;
		}
		if (*pbVertigoRight)
		{
			*pbVertigoRight = FALSE;
			*pbVertigoLeft = TRUE;
			m_blupiTimeNoAsc = 10;
		}
	}
}

BOOL CDecor::AscenseurShift(int i)
{
	return i != -1 && m_moveObject[i].icon >= 311 && m_moveObject[i].icon <= 316;
}

void CDecor::AscenseurSynchro(int i)
{
	for (i = 0; i < MAXMOVEOBJECT; i++)
	{
		m_moveObject[i].posCurrent = m_moveObject[i].posStart;
		m_moveObject[i].step = STEP_STOPSTART;
		m_moveObject[i].time = 0;
		m_moveObject[i].phase = 0;
	}
}

void CDecor::UpdateCaisse()
{
	m_nbRankCaisse = 0;
	for (int i = 0; i < MAXMOVEOBJECT; i++)
	{
		if (m_moveObject[i].type == TYPE_CAISSE)
		{
			m_rankCaisse[m_nbRankCaisse++] = i;
		}
	}
}

BOOL CDecor::TestPushCaisse(int i, POINT pos, BOOL bPop)
{
	POINT move;
	int j;

	move.x = pos.x - m_moveObject[i].posCurrent.x;
	move.y = 0;
	SearchLinkCaisse(i, bPop);
	int y = m_moveObject[i].posCurrent.y;
	for (j = 0; j < m_nbLinkCaisse; j++)
	{
		i = m_linkCaisse[j];
		if (!TestPushOneCaisse(i, move, y))
		{
			return FALSE;
		}
	}
	for (j = 0; j < m_nbLinkCaisse; j++)
	{
		i = m_linkCaisse[j];
		int num = i;
		m_moveObject[num].posCurrent.x = m_moveObject[num].posCurrent.x + move.x;
		m_moveObject[num].posStart.x = m_moveObject[num].posStart.x + move.x;
		int num2 = i;
		m_moveObject[num].posEnd.x = m_moveObject[num].posEnd.x + move.x;
	}
	return TRUE;
}

BOOL CDecor::TestPushOneCaisse(int i, POINT move, int b)
{
	int num;
	RECT rect;
	num = m_moveObject[i].posCurrent.x + move.x;
	rect.left = num;
	rect.right = num + DIMOBJX;
	rect.top = m_moveObject[i].posCurrent.y;
	rect.bottom = m_moveObject[i].posCurrent.y + DIMOBJY;
	if (DecorDetect(rect, FALSE))
	{
		return FALSE;
	}
	if (m_moveObject[i].posCurrent.y != b)
	{
		return TRUE;
	}
	rect.left = num;
	rect.right = num + 20;
	rect.top = m_moveObject[i].posCurrent.y + DIMOBJY;
	rect.bottom = m_moveObject[i].posCurrent.y + DIMOBJY + 2;
	if (!DecorDetect(rect))
	{
		return FALSE;
	}
	rect.left = num + DIMOBJX - 20;
	rect.right = num + DIMOBJX;
	rect.top = m_moveObject[i].posCurrent.y + DIMOBJY;
	rect.bottom = m_moveObject[i].posCurrent.y + DIMOBJY + 2;
	return DecorDetect(rect) != 0;
}

void CDecor::SearchLinkCaisse(int rank, BOOL bPop)
{
	m_nbLinkCaisse = 0;
	AddLinkCaisse(rank);
	POINT posCurrent = m_moveObject[rank].posCurrent;

	bool flag;
	do
	{
		flag = FALSE;
		int i = 0;
		int* pLinkCaisse = m_linkCaisse;
		for (; i < m_nbLinkCaisse; i++, pLinkCaisse++)
		{
			int num = *pLinkCaisse;
			if (m_moveObject[num].posCurrent.y <=
				posCurrent.y && (!bPop || (m_moveObject
					[num].posCurrent.x >= posCurrent.x - DIMOBJX / 2 &&
					m_moveObject[num].posCurrent.x <=
					posCurrent.x + DIMOBJX / 2)))
			{
				RECT src;
				src.left = m_moveObject[num].posCurrent.x - 1;
				src.top = m_moveObject[num].posCurrent.y - 1;
				src.right = src.left + DIMOBJX + 1;
				src.bottom = src.top + DIMOBJY + 1;
				int j = 0;
				int* pRankCaisse = m_rankCaisse;
				for (; j < m_nbRankCaisse; j++, pRankCaisse++)
				{
					int num2 = *pRankCaisse;
					if (num2 != num && m_moveObject[num2].posCurrent.y <= posCurrent.y && (!bPop || (m_moveObject[num2].posCurrent.x >= posCurrent.x - 32 && m_moveObject[num2].posCurrent.x <= posCurrent.x + 32)))
					{
						RECT src2;
						src2.left = m_moveObject[num2].posCurrent.x - 1;
						src2.top = m_moveObject[num2].posCurrent.y - 1;
						src2.right = src2.left + DIMOBJX + 1;
						src2.bottom = src2.top + DIMOBJY + 1;
						RECT tinyRect;
						if (IntersectRect(&tinyRect, &src2, &src) && AddLinkCaisse(num2))
						{
							flag = TRUE;
						}
					}
				}
			}
		}
	} while (flag);
}

BOOL CDecor::AddLinkCaisse(int rank)
{
	for (int i = 0; i < m_nbLinkCaisse; i++)
	{
		if (m_linkCaisse[i] == rank)
		{
			return FALSE;
		}
	}
	m_linkCaisse[m_nbLinkCaisse] = rank;
	m_nbLinkCaisse++;
	return TRUE;
}

int CDecor::CaisseInFront()
{
	POINT tinyPoint;
	if (m_blupiDir == DIR_LEFT)
	{
		tinyPoint.x = m_blupiPos.x + 16 - DIMOBJX / 2;
		tinyPoint.y = m_blupiPos.y;
	}
	else
	{
		tinyPoint.x = m_blupiPos.x + DIMBLUPIX - 16 + DIMOBJX / 2;
		tinyPoint.y = m_blupiPos.y;
	}
	for (int i = 0; i < m_nbRankCaisse; i++)
	{
		int num = m_rankCaisse[i];
		if (tinyPoint.x > m_moveObject[num].posCurrent.x &&
			tinyPoint.x < m_moveObject[num].posCurrent.x + DIMOBJX &&
			tinyPoint.y > m_moveObject[num].posCurrent.y &&
			tinyPoint.y < m_moveObject[num].posCurrent.y + DIMOBJY)
		{
			return num;
		}
	}
	return -1;
}

int CDecor::CaisseGetMove(int max)
{
	max -= (m_nbLinkCaisse - 1) / 2;
	if (max < 1)
	{
		max = 1;
	}
	if (m_blupiPower)
	{
		max *= 2;
	}
	if (m_blupiPhase < 20)
	{
		max = max * m_blupiPhase / 20;
		if (max == 0)
		{
			max++;
		}
	}
	return max;
}

int CDecor::MockeryDetect(POINT pos)
{
	if (m_blupiTimeMockery > 0)
	{
		return 0;
	}
	if (m_blupiAir)
	{
		POINT tinyPoint;
		tinyPoint.x = pos.x + 30;
		tinyPoint.y = pos.y + 30 + DIMOBJY;
		if (tinyPoint.x >= 0 && tinyPoint.x < DIMOBJX*MAXCELX && tinyPoint.y >= 0 && tinyPoint.y < DIMOBJY*MAXCELY)
		{
			int icon = m_decor[tinyPoint.x / DIMOBJX][tinyPoint.y / DIMOBJY].icon;
			if (icon == 68 || icon == 317)
			{
				return ACTION_MOCKERYi;
			}
		}
	}
	RECT src;
	src.left = pos.x;
	src.right = pos.x + DIMBLUPIX;
	src.top = pos.y + 11;
	src.bottom = pos.y + DIMBLUPIY;
	if (m_blupiAir)
	{
		src.bottom += 90;
	}
	for (int i = 0; i < MAXMOVEOBJECT; i++)
	{
		if (m_moveObject[i].type == TYPE_BOMBEDOWN ||
			m_moveObject[i].type == TYPE_BOMBEMOVE ||
			m_moveObject[i].type == TYPE_BOMBEFOLLOW1 ||
			m_moveObject[i].type == TYPE_BOMBEFOLLOW2 ||
			m_moveObject[i].type == TYPE_BULLDOZER ||
			m_moveObject[i].type == TYPE_OISEAU ||
			m_moveObject[i].type == TYPE_GUEPE ||
			m_moveObject[i].type == TYPE_CREATURE ||
			m_moveObject[i].type == TYPE_BALLE ||
			m_moveObject[i].type == TYPE_BLUPIHELICO ||
			m_moveObject[i].type == TYPE_BLUPITANK)
		{
			RECT src2;
			src2.left = m_moveObject[i].posCurrent.x;
			src2.right = m_moveObject[i].posCurrent.x + DIMBLUPIX;
			src2.top = m_moveObject[i].posCurrent.y + 36;
			src2.bottom = m_moveObject[i].posCurrent.y + DIMBLUPIY;
			RECT tinyRect = { 0,0,0,0 };
			if (IntersectRect(&tinyRect, &src2, &src))
			{
				if (m_moveObject[i].type == TYPE_CREATURE)
				{
					return ACTION_MOCKERYp;
				}
				if (m_blupiDir == DIR_RIGHT)
				{
					if (pos.x >= src2.left)
					{
						return ACTION_MOCKERYi;
					}
					if (m_moveObject[i].type == TYPE_BOMBEDOWN)
					{
						return 0;
					}
					return ACTION_MOCKERY;
				}
				else
				{
					if (pos.x < src2.left)
					{
						return ACTION_MOCKERYi;
					}
					if (m_moveObject[i].type == TYPE_BOMBEDOWN)
					{
						return 0;
					}
					return ACTION_MOCKERY;
				}
			}
		}
	}
	return 0;
}

BOOL CDecor::BlupiElectro(POINT pos)
{
	if (m_phase == WM_PHASE_BUILD) return FALSE;
	if (!m_blupiCloud) return FALSE;

	RECT src;
	src.left = pos.x + 16;
	src.right = pos.x + DIMBLUPIX - 16;
	src.top = pos.y + 11;
	src.bottom = pos.y + DIMBLUPIY - 2;
	RECT src2;
	src2.left = m_blupiPos.x - 16 - 40;
	src2.right = m_blupiPos.x + DIMBLUPIX + 16 + 40;
	src2.top = m_blupiPos.y + 11 - 40;
	src2.bottom = m_blupiPos.y + DIMBLUPIY - 2 + 40;
	RECT tinyRect = { 0,0,0,0 };
	return IntersectRect(&tinyRect, &src, &src2);
}

void CDecor::MoveObjectFollow(POINT pos)
{
	if (m_blupiHide) return;

	RECT src = BlupiRect(pos);
	src.left = pos.x + 16;
	src.right = pos.x + DIMBLUPIX - 16;
	for (int i = 0; i < MAXMOVEOBJECT; i++)
	{
		if (m_moveObject[i].type == TYPE_BOMBEFOLLOW1)
		{
			RECT src2;
			src2.left = m_moveObject[i].posCurrent.x - 100;
			src2.right = m_moveObject[i].posCurrent.x + DIMBLUPIX + 100;
			src2.top = m_moveObject[i].posCurrent.y - 100;
			src2.bottom = m_moveObject[i].posCurrent.y + DIMBLUPIY + 100;
			RECT tinyRect = { 0, 0, 0, 0 };
			if (IntersectRect(&tinyRect, &src2, &src))
			{
				m_moveObject[i].type = TYPE_BOMBEFOLLOW2;
				PlaySound(SOUND_FOLLOW, m_moveObject[i].posCurrent);
			}
		}
	}
}

int CDecor::MoveObjectDetect(POINT pos, BOOL* pbNear)
{
	RECT src = BlupiRect(pos);
	src.left = pos.x + 16;
	src.right = pos.x + DIMBLUPIX - 16;
	RECT src2;
	src2.left = src.left - 20;
	src2.right = src.right + 20;
	src2.top = src.top - 40;
	src2.bottom = src.bottom + 30;

	for (int i = 0; i < MAXMOVEOBJECT; i++)
	{
		short type = m_moveObject[i].type;
		if (type != 0 &&
			type != TYPE_MAGICTRACK &&
			type != TYPE_SHIELDTRACK &&
			type != TYPE_TRESORTRACK &&
			type != TYPE_HIDETRACK &&
			type != TYPE_GLU &&
			type != TYPE_CLEAR &&
			type != TYPE_ELECTRO &&
			((m_blupiAction != ACTION_PUSH && m_blupiAction != ACTION_POP) ||
			type != TYPE_HELICO))
		{
			if ((type == TYPE_POISSON ||
				type == TYPE_OISEAU ||
				type == TYPE_GUEPE ||
				type == TYPE_CREATURE) &&
				m_moveObject[i].posCurrent.x == m_blupiPos.x &&
				m_moveObject[i].posCurrent.y <= m_blupiPos.y &&
				m_blupiPos.y - 2 <= m_moveObject[i].posCurrent.y)
			{
				continue;
			}
			RECT src3;
			src3.left = m_moveObject[i].posCurrent.x + 16;
			src3.right = m_moveObject[i].posCurrent.x + DIMBLUPIX - 16;
			src3.top = m_moveObject[i].posCurrent.y + 36;
			src3.bottom = m_moveObject[i].posCurrent.y + DIMBLUPIY;
			if (type == TYPE_BOMBEUP)
			{
				if (m_blupiAction == ACTION_DOWN)
				{
					continue;
				}
				src3.top = m_moveObject[i].posCurrent.y;
				src3.bottom = m_moveObject[i].posCurrent.y + 69 - 36;
			}
			if (type == TYPE_CAISSE)
			{
				src3.left = m_moveObject[i].posCurrent.x - 16;
				src3.right = m_moveObject[i].posCurrent.x + DIMOBJX + 16;
				src3.top = m_moveObject[i].posCurrent.y;
				src3.bottom = m_moveObject[i].posCurrent.y + DIMOBJY;
				if (m_blupiDir == DIR_LEFT)
				{
					src3.left += 20;
				}
				else
				{
					src3.right -= 20;
				}
			}
			if (type == TYPE_POISSON ||
				type == TYPE_OISEAU ||
				type == TYPE_GUEPE ||
				type == TYPE_CREATURE)
			{
				src3.top = m_moveObject[i].posCurrent.y + 16;
				src3.bottom = m_moveObject[i].posCurrent.y + DIMBLUPIY - 16;
			}
			if (type == TYPE_BALLE)
			{
				src3.left = m_moveObject[i].posCurrent.x + 24;
				src3.right = m_moveObject[i].posCurrent.x + DIMBLUPIX - 24;
				src3.top = m_moveObject[i].posCurrent.y + 10;
				src3.bottom = m_moveObject[i].posCurrent.y + DIMBLUPIY - 32;
			}
			RECT rect;
			if (IntersectRect(&rect, &src3, &src))
			{
				*pbNear = TRUE;
				return i;
			}
			if (m_moveObject[i].type == TYPE_BOMBEDOWN && IntersectRect(&rect, &src3, &src2))
			{
				*pbNear = FALSE;
				return i;
			}
		}
	}
	*pbNear = FALSE;
	return -1;
}

int CDecor::MoveAscenseurDetect(POINT pos, int height)
{
	if (m_blupiTimeNoAsc != 0)
	{
		return -1;
	}
	RECT src;
	src.left = pos.x + 12;
	src.right = pos.x + DIMBLUPIX - 12;
	src.top = pos.y + DIMBLUPIY - 2;
	src.bottom = pos.y + DIMBLUPIY + height - 1;
	for (int i = 0; i < MAXMOVEOBJECT; i++)
	{
		if (m_moveObject[i].type == TYPE_ASCENSEUR || m_moveObject[i].type == TYPE_ASCENSEURs || m_moveObject[i].type == TYPE_ASCENSEURsi)
		{
			RECT src2;
			src2.left = m_moveObject[i].posCurrent.x;
			src2.right = m_moveObject[i].posCurrent.x + DIMOBJX;
			src2.top = m_moveObject[i].posCurrent.y;
			src2.bottom = m_moveObject[i].posCurrent.y + 16;
			RECT tinyRect = { 0, 0, 0, 0 };
			if (IntersectRect(&tinyRect, &src2, &src))
			{
				return i;
			}
		}
	}
	return -1;
}

int CDecor::MoveChargeDetect(POINT pos)
{
	RECT src;
	src.left = pos.x + 16;
	src.right = pos.x + DIMBLUPIX - 16;
	src.top = pos.y + 11;
	src.bottom = pos.y + DIMBLUPIY - 2;
	for (int i = 0; i < MAXMOVEOBJECT; i++)
	{
		if (m_moveObject[i].type == TYPE_CHARGE)
		{
			RECT src2;
			src2.left = m_moveObject[i].posCurrent.x - 10;
			src2.right = m_moveObject[i].posCurrent.x + DIMBLUPIX + 10;
			src2.top = m_moveObject[i].posCurrent.y + 36;
			src2.bottom = m_moveObject[i].posCurrent.y + DIMBLUPIY;
			RECT tinyRect = { 0, 0, 0, 0 };
			if (IntersectRect(&tinyRect, &src2, &src))
			{
				return i;
			}
		}
	}
	return -1;
}

int CDecor::MovePersoDetect(POINT pos)
{
	RECT src;
	if (m_phase == WM_PHASE_BUILD) return -1;
	src.left = pos.x + 16;
	src.right = pos.x + DIMBLUPIX - 16;
	src.top = pos.y + 11;
	src.bottom = pos.y + DIMBLUPIY - 2;
	for (int i = 0; i < MAXMOVEOBJECT; i++)
	{
		if (m_moveObject[i].type >= TYPE_BOMBEPERSO1 && m_moveObject[i].type <= TYPE_BOMBEPERSO4)
		{
			RECT src2;
			src2.left = m_moveObject[i].posCurrent.x - 16;
			src2.right = m_moveObject[i].posCurrent.x + DIMBLUPIX + 16;
			src2.top = m_moveObject[i].posCurrent.y + 36;
			src2.bottom = m_moveObject[i].posCurrent.y + DIMBLUPIY;
			RECT tinyRect = { 0, 0, 0, 0 };
			if (IntersectRect(&tinyRect, &src2, &src))
			{
				return i;
			}
		}
	}
	return -1;
}

int CDecor::MoveBalleDetect(POINT pos)
{
	RECT rcSrc1, rcSrc2;
	tagRECT rcDst;
	int i;
	if (m_phase == WM_PHASE_BUILD)
		return -1;
	rcSrc2.right = pos.x + DIMBLUPIX - 16;
	rcSrc2.left = pos.x + 20;
	rcSrc2.top = pos.y + 10;
	rcSrc2.bottom = pos.y + 28;
	for (i = 0; i < MAXMOVEOBJECT - 1; i++)
	{
		if (m_moveObject->type == TYPE_BULLDOZER ||
			m_moveObject->type == TYPE_POISSON ||
			m_moveObject->type == TYPE_OISEAU ||
			m_moveObject->type == TYPE_GUEPE ||
			m_moveObject->type == TYPE_BLUPIHELICO ||
			m_moveObject->type == TYPE_BLUPITANK)
		{
			rcSrc1.right = m_moveObject->posCurrent.x + DIMBLUPIX - 16;
			rcSrc1.left = m_moveObject->posCurrent.x + 16;
			rcSrc1.bottom = m_moveObject->posCurrent.y + DIMBLUPIY - 10;
			rcSrc1.top = m_moveObject->posCurrent.y + 16;
			rcDst = RECT( 0, 0, 0, 0 );
			if (IntersectRect(&rcDst, &rcSrc1, &rcSrc2))
				return i;
		}
	}
	return -1;
}

void CDecor::MoveObjectDelete(POINT cel)
{
	for (int i = 0; i < MAXMOVEOBJECT; i++)
	{
		if (m_moveObject[i].type != 0)
		{
			if (cel.x == m_moveObject[i].posStart.x / DIMOBJX && cel.y == m_moveObject[i].posStart.y / DIMOBJY)
			{
				m_moveObject[i].type = 0;
			}
			if (cel.x == m_moveObject[i].posEnd.x / DIMOBJX && cel.y == m_moveObject[i].posEnd.y / DIMOBJY)
			{
				m_moveObject[i].type = 0;
			}
		}
	}
}

int CDecor::MoveObjectFree()
{
	for (int i = 0; i < MAXMOVEOBJECT; i++)
	{
		if (m_moveObject[i].type == 0)
		{
			ZeroMemory(&m_moveObject[i], sizeof(MoveObject));
			return i;
		}
	}
	return -1;
}

int __cdecl SortGetType(int type)
{
	if (type == TYPE_BOMBEDOWN || type == TYPE_BOMBEUP)
	{
		return 1;
	}
	if (type == TYPE_CAISSE)
	{
		return 2;
	}
	return 3;
}

void CDecor::MoveObjectSort()
{
	MoveObject dst;
	int i;
	int num = 0;

	for (i = 0; i < MAXMOVEOBJECT; i++)
	{
		if (m_moveObject[i].type != 0)
		{
			MoveObjectCopy(&m_moveObject[num++], &m_moveObject[i]);
		}
	}
	for (i = num; i < MAXMOVEOBJECT; i++)
	{
		m_moveObject[i].type = 0;
	}
	if (num <= 1)
	{
		return;
	}
	BOOL flag;
	do
	{
		flag = FALSE;
		for (i = 0; i < num - 1; i++)
		{
			int typeCurrent = SortGetType(m_moveObject[i].type);
			int typeNext = SortGetType(m_moveObject[i + 1].type);
			if (typeNext < typeCurrent)
			{
				MoveObjectCopy(&dst, &m_moveObject[i]);
				MoveObjectCopy(&m_moveObject[i], &m_moveObject[i + 1]);
				MoveObjectCopy(&m_moveObject[i + 1], &dst);
				flag = TRUE;
			}
		}
	} while (flag);
}

void CDecor::MoveObjectPriority(int rank)
{
	if (rank != 0 && m_moveObject[rank].type == TYPE_BALLE)
	{
		int i = 0;
		while (m_moveObject[i].type == TYPE_BALLE)
		{
			i++;
			if (i >= MAXMOVEOBJECT) return;
		}
		if (i <= rank)
		{
			MoveObject tempMob = m_moveObject[rank];
			m_moveObject[rank] = m_moveObject[i];
			m_moveObject[i] = tempMob;
			if (m_moveObject[rank].type == TYPE_CAISSE || m_moveObject[i].type == TYPE_CAISSE)
			{
				UpdateCaisse();
			}
		}
	}
}

int CDecor::MoveObjectSearch(POINT pos, int type)
{
	for (int i = 0; i < MAXMOVEOBJECT; i++)
	{
		if (m_moveObject[i].type != 0 && (type == -1 ||
			m_moveObject[i].type == type))
		{
			if (m_moveObject[i].type == TYPE_BALLE &&
				m_moveObject[i].posStart.x != m_moveObject[i].posEnd.x)
			{
				if (m_moveObject[i].posCurrent.x >= pos.x - 100 &&
					m_moveObject[i].posCurrent.x <= pos.x + 100 &&
					m_moveObject[i].posCurrent.y == pos.y)
				{
					return i;
				}
			}
			else if (m_moveObject[i].type == TYPE_BALLE &&
				m_moveObject[i].posStart.y != m_moveObject[i].posEnd.y)
			{
				if (m_moveObject[i].posCurrent.y >= pos.y - 100 &&
					m_moveObject[i].posCurrent.y <= pos.y + 100 &&
					m_moveObject[i].posCurrent.x == pos.x)
				{
					return i;
				}
			}
			else if (m_moveObject[i].posCurrent.x == pos.x &&
				m_moveObject[i].posCurrent.y == pos.y)
			{
				return i;
			}
		}
	}
	return -1;
}
