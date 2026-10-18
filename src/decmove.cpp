// DecMove.cpp
//

#include <def.h>
#include <misc.h>
#include <dectables.h>
#include <decor.h>

void CDecor::NetDataFlush()
{
	for (int i = 0; i < MAXNETPLAYER; i++)
	{
		m_netBlupiIcon[i] = -1;
		m_netTimes[i] = 0;
		m_netPrevTimes[i] = 0;
		m_netVitesses[i].x = 0;
		m_netVitesses[i].y = 0;
	}
	m_netPacketsSent = 0;
	m_netPacketsSent2 = 0;
	m_netPacketsReceived = 0;
	m_netPacketsReceived2 = 0;
	m_netPacketIcon = -1;
	NetMessageIndexFlush();
}

void CDecor::NetSendBarePacket(BYTE type, int num)
{
	struct BarePacket
	{
		BYTE size;
		BYTE type;
		short num;
	};
	BarePacket data;
	data.size = 4;
	data.type = type;
	data.num = num;
	m_pNetwork->Send(&data, sizeof(data), DPSEND_GUARANTEED);
}

void CDecor::TreatNetData()
{
	NetPacket pack;
	NetMessage* pMsg;
	int i, j, dt, type, speed;
	POINT pos;
	int flag;
	int player;
	char res[100];
	char text[100];
	char res2[100];
	char text2[100];
	char res3[100];
	char text3[100];

	BOOL recentSenders[MAXNETPLAYER];

	if ((m_netMessageIndex1 != 0 ||
		m_blupiIcon != m_netPacketIcon ||
		m_blupiPos.x != m_netPacketPos.x ||
		m_blupiPos.y != m_netPacketPos.y) &&
		(!m_bNetPacked || m_time % 2 == 0))
	{
		m_netPacketPos = m_blupiPos;
		m_netPacketIcon = m_blupiIcon;
		pack.time = m_time;
		pack.keyPress = m_keyPress;
		pack.blupiPosX = m_blupiPos.x;
		pack.blupiPosY = m_blupiPos.y;
		pack.blupiIcon = m_blupiIcon;
		pack.size = sizeof(pack);
		pack.type = PK_MOVE;
		pack.blupiSec = m_blupiSec;
		pack.blupiChannel = m_blupiChannel;
		pack.blupiTransport = m_blupiTransport;
		pack.nbMessages = 0;
		flag = 0;
		for (i = 0; i < MAXMESSAGEPERPACKET; i++)
		{
			if (!NetMessagePop(&pack.messages[i]))
			{
				break;
			}
			pack.nbMessages++;
			if (pack.messages[i].type == PK_OBJECTSTART ||
				pack.messages[i].type == PK_OBJECTDELETE)
			{
				if (pack.messages[i].moveObjectType != TYPE_EXPLO1 &&
					pack.messages[i].moveObjectType != TYPE_EXPLO2 &&
					pack.messages[i].moveObjectType != TYPE_EXPLO3 &&
					pack.messages[i].moveObjectType != TYPE_EXPLO4 &&
					pack.messages[i].moveObjectType != TYPE_PLOUF &&
					pack.messages[i].moveObjectType != TYPE_BLUP &&
					pack.messages[i].moveObjectType != TYPE_MAGICTRACK &&
					pack.messages[i].moveObjectType != TYPE_TIPLOUF &&
					pack.messages[i].moveObjectType != TYPE_POLLUTION)
				{
					flag = DPSEND_GUARANTEED;
				}
			}
			if (pack.messages[i].type == PK_MODIFDECOR ||
				pack.messages[i].type == PK_STOPCLOUD)
			{
				flag = DPSEND_GUARANTEED;
			}
		}
		pack.flag = flag;
		m_pNetwork->Send(&pack, sizeof(NetMessage) * pack.nbMessages + 20, flag);
		m_netPacketsSent++;
		if (flag) m_netPacketsSent2++;
	}

	ZeroMemory(&recentSenders, sizeof(recentSenders));

	for (i = 0; i < 10; i++)
	{
		if (!m_pNetwork->Receive(&pack, sizeof(pack), (LPDWORD)&player))
		{
			break;
		}

		if (pack.type == PK_MOVE)
		{
			m_netPacketsReceived++;
			if (pack.flag) m_netPacketsReceived2++;

			if (player >= 0 && player < MAXNETPLAYER)
			{
				if (pack.time > m_netTimes[player])
				{
					recentSenders[player] = TRUE;
					m_netPrevTimes[player] = m_netTimes[player];
					m_netTimes[player] = pack.time;
					m_netBlupiPos[player].x = pack.blupiPosX;
					m_netBlupiPos[player].y = pack.blupiPosY;
					m_netBlupiIcon[player] = pack.blupiIcon;
					m_netBlupiSec[player] = pack.blupiSec;
					m_netBlupiTransport[player] = pack.blupiChannel;
					m_netTransports[player] = pack.blupiTransport;
					dt = pack.time - m_netPrevTimes[player];
					if (dt > 0)
					{
						m_netVitesses[player].x = (m_netBlupiPos[player].x - m_netBlupiPrevPos[player].x) / dt;
						m_netVitesses[player].y = (m_netBlupiPos[player].y - m_netBlupiPrevPos[player].y) / dt;
					}
					else
					{
						m_netVitesses[player].x = 0;
						m_netVitesses[player].y = 0;
					}
					m_netBlupiPrevPos[player] = m_netBlupiPos[player];
					m_netTimeSincePacket[player] = 0;
				}
			}
			for (j = 0; j < pack.nbMessages; j++)
			{
				pMsg = &pack.messages[j];
				speed = pMsg->data1;
				pos.x = pMsg->posX;
				pos.y = pMsg->posY;
				type = pMsg->data4;
				if (pMsg->type == PK_OBJECTSTART)
					ObjectStart(pos, type, speed, FALSE);
				if (pMsg->type == PK_OBJECTDELETE)
					ObjectDelete(pos, type, FALSE);
				if (pMsg->type == PK_MODIFDECOR)
					ModifDecor(pos, type, FALSE);
				if (pMsg->type == PK_PLAYSOUND)
				{
					pos.x -= m_posDecor.x;
					pos.y -= m_posDecor.y;
					m_pSound->PlayImage(type, pos, -1, 0);
				}
				if (pMsg->type == PK_STOPSOUND)
					m_pSound->StopSound(type);
				if (pMsg->type == PK_ASCENSEURSTART)
					NetAscenseurSynchro(type, STEP_STOPSTART);
				if (pMsg->type == PK_ASCENSEUREND)
					NetAscenseurSynchro(type, STEP_STOPEND);
				if (pMsg->type == PK_STOPCLOUD && type == m_team)
				{
					m_blupiCloud = FALSE;
					m_jauges[JAUGE_POWER].SetHide(TRUE);
				}
			}
		}
		if (pack.type == PK_ABANDON)
		{
			LoadString(257, res, 100);
			sprintf(text, res, m_pNetwork->m_players[player].name);
			NotifPush(text);
			m_pNetwork->m_players[player].bIsPresent = FALSE;
			m_netBlupiIcon[player] = -1;
		}
		if (pack.type == PK_LOST)
		{
			LoadString(258, res2, 100);
			sprintf(text2, res2, m_pNetwork->m_players[player].name);
			NotifPush(text2);
			m_pNetwork->m_players[player].bIsPresent = 0;
			m_netBlupiIcon[player] = -1;
		}
		if (pack.type == PK_DIE)
		{
			if (pack.nbVies == 0)
			{
				LoadString(261, res3, 100);
			}
			else if (pack.nbVies == 1)
			{
				LoadString(260, res3, 100);
			}
			else
			{
				LoadString(259, res3, 100);
			}
			sprintf(text3, res3, m_pNetwork->m_players[player].name, pack.nbVies);
			NotifPush(text3);
		}
		if (pack.type == PK_PAUSE)
		{
			m_bPause = (char)pack.bPause;
		}
	}

	for (i = 0; i < MAXNETPLAYER; i++)
	{
		if (!recentSenders[i])
		{
			DoNetSmooth(i);
		}
	}
}

void CDecor::DoNetSmooth(int player)
{
	POINT end;
	RECT rect;

	if (m_bNetMovePredict && m_netBlupiIcon[player] != -1)
	{
		if (m_netTimeSincePacket[player] < 5)
		{
			end.x = m_netBlupiPos[player].x + m_netVitesses[player].x;
			end.y = m_netBlupiPos[player].y + m_netVitesses[player].y;
			rect.left = m_netBlupiPos[player].x + 12;
			rect.top = m_netBlupiPos[player].y + 11;
			rect.right = m_netBlupiPos[player].x + DIMBLUPIX - 12;
			rect.bottom = m_netBlupiPos[player].y + DIMBLUPIY - 2;
			TestPath(rect, m_netBlupiPos[player], &end);
			m_netBlupiPos[player] = end;
		}
		m_netTimeSincePacket[player]++;
	}
}

void CDecor::NetAdjustToLift()
{
	for (int i = 0; i < MAXNETPLAYER; i++)
	{
		if (m_netBlupiIcon[i] != -1 && m_netTransports[i] >= 0 && m_netTransports[i] < MAXMOVEOBJECT)
		{
			m_netBlupiPos[i].y = m_moveObject[m_netTransports[i]].posCurrent.y - (DIMBLUPIY - 2);
		}
	}
}

void CDecor::NetAscenseurSynchro(int index, short step)
{
	int i;
	int dx, dy;
	int type;
	POINT otherDelta;

	m_moveObject[index].step = step;
	m_moveObject[index].time = 0;
	type = m_moveObject[index].type;
	dx = abs(m_moveObject[index].posEnd.x - m_moveObject[index].posStart.x);
	dy = abs(m_moveObject[index].posEnd.y - m_moveObject[index].posStart.y);
	for (i = 0; i < MAXMOVEOBJECT; i++)
	{
		if (i != index && m_moveObject[i].type == type)
		{
			otherDelta.x = abs(m_moveObject[i].posEnd.x - m_moveObject[i].posStart.x);
			otherDelta.y = abs(m_moveObject[i].posEnd.y - m_moveObject[i].posStart.y);
			if (dx == otherDelta.x && dy == otherDelta.y &&
				m_moveObject[i].stepAdvance == m_moveObject[index].stepAdvance &&
				m_moveObject[i].stepRecede == m_moveObject[index].stepRecede &&
				m_moveObject[i].timeStopStart == m_moveObject[index].timeStopStart &&
				m_moveObject[i].timeStopEnd == m_moveObject[index].timeStopEnd)
			{
				m_moveObject[i].step = step;
				m_moveObject[i].time = 0;
			}
		}
	}
}

void CDecor::NetPlayerCollide(POINT pos, int* out)
{
	int i;
	RECT rect1;
	RECT rect2;
	RECT rect3;

	rect2.left = pos.x + 16;
	rect2.right = pos.x + DIMBLUPIX - 16;
	rect2.top = pos.y + 11;
	rect2.bottom = pos.y + DIMBLUPIY - 2;

	for (i = 0; i < MAXNETPLAYER; i++)
	{
		if (m_netBlupiIcon[i] != -1)
		{
			rect1.left = m_netBlupiPos[i].x + 16;
			rect1.right = m_netBlupiPos[i].x + DIMBLUPIX - 16;
			rect1.top = m_netBlupiPos[i].y + 11;
			rect1.bottom = m_netBlupiPos[i].y + DIMBLUPIY - 2;
			if (m_netBlupiSec[i] == SEC_CLOUD)
			{
				rect1.left -= 40;
				rect1.right += 40;
				rect1.top -= 40;
				rect1.bottom += 40;
			}
			if (IntersectRect(&rect3, &rect1, &rect2))
			{
				*out = i;
				return;
			}
		}
	}
	*out = -1;
}

void CDecor::NetMessageIndexFlush()
{
	m_netMessageIndex1 = 0;
	m_netMessageIndex2 = 0;
	m_netMessageIndex3 = 0;
}

BOOL CDecor::NetMessagePush(NetMessage* message)
{
	if (m_netMessageIndex1 == MAXNETMESSAGE) return FALSE;
	CopyMemory(&m_netMessages[m_netMessageIndex2], message, sizeof(NetMessage));
	m_netMessageIndex1++;
	m_netMessageIndex2++;
	if (m_netMessageIndex2 == MAXNETMESSAGE) m_netMessageIndex2 = 0;
	return TRUE;
}

BOOL CDecor::NetMessagePop(NetMessage* out_msg)
{
	if (m_netMessageIndex1 == 0) return FALSE;
	CopyMemory(out_msg, &m_netMessages[m_netMessageIndex3], sizeof(NetMessage));
	m_netMessageIndex1--;
	m_netMessageIndex3++;
	if (m_netMessageIndex3 == MAXNETMESSAGE) m_netMessageIndex3 = 0;
	return TRUE;
}

void CDecor::NotifFlush()
{
	for (int i = 0; i < MAXNOTIF; i++)
	{
		m_notifText[i][0] = '\0';
	}
	m_notifTime = 0;
}

void CDecor::NotifPop()
{
	for (int i = 0; i < MAXNOTIF - 1; i++)
	{
		strcpy(m_notifText[i], m_notifText[i + 1]);
	}
	m_notifText[MAXNOTIF - 1][0] = '\0';
	m_notifTime = NOTIFDELAY;
}

void CDecor::NotifPush(char* str)
{
	int i;
	char* pText;
	for (i = 0; i < MAXNOTIF; i++)
	{
		if (m_notifText[i][0] == '\0')
		{
			pText = m_notifText[i];
			goto Push;
		}
	}
	NotifPop();
	pText = m_notifText[MAXNOTIF - 1];
Push:
	strcpy(pText, str);
	m_notifTime = NOTIFDELAY;
	{
		POINT pos = { LXIMAGE / 2, LYIMAGE / 2 };
		m_pSound->PlayImage(SOUND_TRESOR, pos, -1, 0);
	}
}
void CDecor::NotifStep()
{
	if (m_notifTime == 0)
	{
		NotifPop();
	}
	else
	{
		m_notifTime--;
	}
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
	PlaySound(SOUND_GLU, pos, FALSE);
}

BOOL CDecor::ObjectStart(POINT pos, int type, int speed)
{
	return ObjectStart(pos, type, speed, FALSE);
}





void CDecor::MoveObjectStep()
{
	int i, num, type;
	POINT posCurrent;
	m_blupiVector.x = 0;
	m_blupiVector.y = 0;
	m_blupiTransport = -1;
	for (i = 0; i < MAXMOVEOBJECT; i++)
	{
		if (m_moveObject[i].type != 0)
		{
			MoveObjectStepLine(i);
			MoveObjectStepIcon(i);
			if (m_moveObject[i].type == TYPE_BULLDOZER ||
				m_moveObject[i].type == TYPE_BLUPITANK ||
				m_moveObject[i].type == TYPE_BLUPIHELICO)
			{
				num = MovePersoDetect(m_moveObject[i].posCurrent);
				if (num != -1)
				{
					posCurrent = m_moveObject[i].posCurrent;
					posCurrent.x -= 34;
					posCurrent.y -= 34;
					ObjectStart(posCurrent, TYPE_EXPLO1, 0, TRUE);
					PlaySound(SOUND_BOUM, m_moveObject[i].posCurrent, FALSE);
					m_decorAction = 1;
					m_decorPhase = 0;
					posCurrent = m_moveObject[i].posCurrent;
					posCurrent.x += 2;
					posCurrent.y += BLUPIOFFY;
					ObjectDelete(m_moveObject[i].posCurrent, m_moveObject[i].type);
					ObjectStart(posCurrent, TYPE_CLEAR, 0, TRUE);
					ObjectDelete(m_moveObject[num].posCurrent, m_moveObject[num].type);
				}
				if (BlupiElectro(m_moveObject[i].posCurrent))
				{
					posCurrent = m_moveObject[i].posCurrent;
					posCurrent.x += 2;
					posCurrent.y += BLUPIOFFY;
					ObjectDelete(m_moveObject[i].posCurrent, m_moveObject[i].type);
					ObjectStart(posCurrent, TYPE_ELECTRO, 55, TRUE);
					PlaySound(SOUND_ELECTRO, posCurrent, FALSE);
				}
			}
			if (m_moveObject[i].type == TYPE_BALLE)
			{
				num = MoveBalleDetect(m_moveObject[i].posCurrent);
				if (num != -1)
				{
					posCurrent = m_moveObject[i].posCurrent;
					posCurrent.x -= 34;
					posCurrent.y -= 34;
					ObjectStart(posCurrent, TYPE_EXPLO3, 0, TRUE);
					PlaySound(SOUND_BOUM, m_moveObject[i].posCurrent, FALSE);
					m_decorAction = 1;
					m_decorPhase = 0;
					ObjectDelete(m_moveObject[i].posCurrent, m_moveObject[i].type);
					type = m_moveObject[num].type;
					posCurrent = m_moveObject[num].posCurrent;
					ObjectDelete(posCurrent, type);
					if (type == TYPE_BLUPIHELICO || type == TYPE_BLUPITANK)
					{
						posCurrent.x += 2;
						posCurrent.y += BLUPIOFFY;
						ObjectStart(posCurrent, TYPE_GLU, 55, TRUE);
					}
				}
			}
		}
	}
}

void CDecor::MoveObjectStepLine(int i)
{
	NetMessage msg;
	POINT tinyPoint;
	BOOL flag = FALSE;
	RECT tinyRect;
	RECT src;
	RECT tinyRect2;
	if (m_moveObject[i].type == TYPE_ASCENSEUR &&
		!m_blupiSuspend)
	{
		src.left = m_blupiPos.x + 20;
		src.right = m_blupiPos.x + DIMBLUPIX - 20;
		src.top = m_blupiPos.y + DIMBLUPIY - 2;
		src.bottom = m_blupiPos.y + DIMBLUPIY - 1;
		tinyRect.left = m_moveObject[i].posCurrent.x;
		tinyRect.right = m_moveObject[i].posCurrent.x + DIMOBJX;
		tinyRect.top = m_moveObject[i].posCurrent.y;
		tinyRect.bottom = m_moveObject[i].posCurrent.y + 16;
		flag = IntersectRect(&tinyRect2, &tinyRect, &src);
		tinyPoint = m_moveObject[i].posCurrent;
	}
	if (m_moveObject[i].posStart.x != m_moveObject[i].posEnd.x || m_moveObject[i].posStart.y != m_moveObject[i].posEnd.y)
	{
		if (m_moveObject[i].step == STEP_STOPSTART)
		{
			if (m_moveObject[i].time == 0 && m_pNetwork->IsHost())
			{
				msg.type = PK_ASCENSEURSTART;
				msg.data2 = 0;
				msg.data3 = 0;
				msg.data4 = i;
				NetMessagePush(&msg);
			}
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
					m_moveObject[i].posCurrent.x = (m_moveObject[i].posEnd.x - m_moveObject[i].posStart.x) * m_moveObject[i].time / m_moveObject[i].stepAdvance + m_moveObject[i].posStart.x;
					m_moveObject[i].posCurrent.y = (m_moveObject[i].posEnd.y - m_moveObject[i].posStart.y) * m_moveObject[i].time / m_moveObject[i].stepAdvance + m_moveObject[i].posStart.y;
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
			if (m_moveObject[i].time == 0 && m_pNetwork->IsHost())
			{
				msg.type = PK_ASCENSEUREND;
				msg.data2 = 0;
				msg.data3 = 0;
				msg.data4 = i;
				NetMessagePush(&msg);
			}
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
					m_moveObject[i].posCurrent.x = (m_moveObject[i].posStart.x - m_moveObject[i].posEnd.x) * m_moveObject[i].time / m_moveObject[i].stepRecede + m_moveObject[i].posEnd.x;
					m_moveObject[i].posCurrent.y = (m_moveObject[i].posStart.y - m_moveObject[i].posEnd.y) * m_moveObject[i].time / m_moveObject[i].stepRecede + m_moveObject[i].posEnd.y;
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
	if (flag)
	{
		m_blupiVector.x = m_moveObject[i].posCurrent.x - tinyPoint.x;
		m_blupiVector.y = m_moveObject[i].posCurrent.y - (m_blupiPos.y + DIMBLUPIY - BLUPIFLOOR);
		m_blupiTransport = i;
	}
}

void CDecor::MoveObjectStepIcon(int i)
{
	POINT pos;
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
	if (m_moveObject[i].type == TYPE_BOMBEPERSO1)
	{
		m_moveObject[i].icon = 257 + m_moveObject[i].phase / 1 % 6;
		m_moveObject[i].channel = 2;
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
	if (m_moveObject[i].type == TYPE_SKATE)
	{
		m_moveObject[i].icon = table_skate[m_moveObject[i].phase / 1 % 34];
		m_moveObject[i].channel = CHELEMENT;
	}
	if (m_moveObject[i].type == TYPE_SHIELD)
	{
		m_moveObject[i].icon = table_shield[m_moveObject[i].phase / 2 % 14];
		m_moveObject[i].channel = CHELEMENT;
	}
	if (m_moveObject[i].type == TYPE_POWER)
	{
		m_moveObject[i].icon = table_power[m_moveObject[i].phase / 2 % 8];
		m_moveObject[i].channel = CHELEMENT;
	}
	if (m_moveObject[i].type == TYPE_CHARGE)
	{
		m_moveObject[i].icon = table_charge[m_moveObject[i].phase / 2 % 6];
		m_moveObject[i].channel = 1;
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
	if (m_moveObject[i].type == TYPE_EXPLO1)
	{
		if (m_moveObject[i].phase >= 36)
		{
			m_moveObject[i].type = 0;
		}
		else
		{
			m_moveObject[i].icon = m_moveObject[i].phase / 3 % 12;
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
	if (m_moveObject[i].type == TYPE_PLOUF)
	{
		if (m_moveObject[i].phase >= 14)
		{
			m_moveObject[i].type = 0;
		}
		else
		{
			m_moveObject[i].icon = table_plouf[m_moveObject[i].phase / 2 % 7];
			m_moveObject[i].channel = 1;
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
			m_moveObject[i].channel = 1;
		}
	}
	if (m_moveObject[i].type == TYPE_BLUP)
	{
		m_moveObject[i].icon = table_blup[m_moveObject[i].phase / 2 % 20];
		m_moveObject[i].channel = 1;
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
		if (m_phase != WM_PHASE_BUILD && (m_moveObject[i].step == STEP_STOPSTART || m_moveObject[i].step == STEP_STOPEND) && m_moveObject[i].time == 21)
		{
			pos.x = m_moveObject[i].posCurrent.x;
			pos.y = m_moveObject[i].posCurrent.y + 40;
			if (ObjectStart(pos, TYPE_BALLE, 55, TRUE))
			{
				PlaySound(SOUND_FIREOK, pos, FALSE);
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
		if (m_phase != WM_PHASE_BUILD && (m_moveObject[i].step == STEP_STOPSTART || m_moveObject[i].step == STEP_STOPEND) && m_moveObject[i].time == 3)
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
			if (ObjectStart(pos, TYPE_BALLE, speed, TRUE))
			{
				PlaySound(SOUND_FIREOK, pos, FALSE);
			}
		}
		if (m_phase != WM_PHASE_BUILD && (m_moveObject[i].step == STEP_STOPSTART || m_moveObject[i].step == STEP_STOPEND) && m_moveObject[i].time == 21)
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
			if (ObjectStart(pos, TYPE_BALLE, speed, TRUE))
			{
				PlaySound(SOUND_FIREOK, pos, FALSE);
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
		PlaySound(SOUND_BOUM, posStart, FALSE);
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
	if (m_blupiFocus && !m_blupiCloud && !m_blupiPosHelico.x && !m_bSuperBlupi && m_blupiPos.x > posStart.x - 30 && m_blupiPos.x < posStart.x + 30 + DIMOBJX && m_blupiPos.y > posStart.y - 30 && m_blupiPos.y < posStart.y + 30 + DIMOBJY)
	{
		BlupiDead(ACTION_CLEAR1, -1);
		m_blupiAir = TRUE;
	}

}

int CDecor::AscenseurDetect(RECT rect, POINT oldpos, POINT newpos)
{
	int i, j;
	RECT src, src2, dest;
	int dy = newpos.y - oldpos.y;
	int dirY = dy < 0 ? -(DIMBLUPIY / 2) : (DIMBLUPIY / 2);
	dy = abs(dy);

	for (i = 0; i < MAXMOVEOBJECT; i++)
	{
		if (m_moveObject[i].type == TYPE_ASCENSEUR)
		{
			src.left = m_moveObject[i].posCurrent.x;
			src.right = m_moveObject[i].posCurrent.x + DIMOBJX;
			src.top = m_moveObject[i].posCurrent.y;
			src.bottom = m_moveObject[i].posCurrent.y + 16;
			if (dy < (DIMBLUPIY / 2))
			{
				if (IntersectRect(&dest, &src, &rect))
				{
					return i;
				}
			}
			else
			{
				src2 = rect;
				src2.top -= dy / (DIMBLUPIY / 2) * dirY;
				src2.bottom -= dy / (DIMBLUPIY / 2) * dirY;
				for (j = 0; j <= dy / (DIMBLUPIY / 2); j++)
				{
					if (IntersectRect(&dest, &src, &src2))
					{
						return i;
					}
					src2.top += dirY;
					src2.bottom += dirY;
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
	int i, j, num, num2;
	RECT src, src2, tinyRect;
	m_nbLinkCaisse = 0;
	AddLinkCaisse(rank);
	POINT posCurrent = m_moveObject[rank].posCurrent;

	BOOL flag;
	do
	{
		flag = FALSE;
		i = 0;
		for (; i < m_nbLinkCaisse; i++)
		{
			num = m_linkCaisse[i];
			if (m_moveObject[num].posCurrent.y <=
				posCurrent.y && (!bPop || (m_moveObject
					[num].posCurrent.x >= posCurrent.x - DIMOBJX / 2 &&
					m_moveObject[num].posCurrent.x <=
					posCurrent.x + DIMOBJX / 2)))
			{
				src.left = m_moveObject[num].posCurrent.x - 1;
				src.top = m_moveObject[num].posCurrent.y - 1;
				src.right = src.left + DIMOBJX + 1;
				src.bottom = src.top + DIMOBJY + 1;
				j = 0;
				for (; j < m_nbRankCaisse; j++)
				{
					num2 = m_rankCaisse[j];
					if (num2 != num && m_moveObject[num2].posCurrent.y <= posCurrent.y && (!bPop || (m_moveObject[num2].posCurrent.x >= posCurrent.x - 32 && m_moveObject[num2].posCurrent.x <= posCurrent.x + 32)))
					{
						src2.left = m_moveObject[num2].posCurrent.x - 1;
						src2.top = m_moveObject[num2].posCurrent.y - 1;
						src2.right = src2.left + DIMOBJX + 1;
						src2.bottom = src2.top + DIMOBJY + 1;
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
			if (icon == 68)
			{
				return ACTION_MOCKERYi;
			}
		}
		tinyPoint.y += DIMOBJY;
		if (tinyPoint.x >= 0 && tinyPoint.x < DIMOBJX*MAXCELX && tinyPoint.y >= 0 && tinyPoint.y < DIMOBJY*MAXCELY)
		{
			int icon = m_decor[tinyPoint.x / DIMOBJX][tinyPoint.y / DIMOBJY].icon;
			if (icon == 68)
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
			m_moveObject[i].type == TYPE_BULLDOZER ||
			m_moveObject[i].type == TYPE_OISEAU ||
			m_moveObject[i].type == TYPE_BALLE ||
			m_moveObject[i].type == TYPE_BLUPIHELICO ||
			m_moveObject[i].type == TYPE_BLUPITANK)
		{
			RECT src2;
			src2.left = m_moveObject[i].posCurrent.x;
			src2.right = m_moveObject[i].posCurrent.x + DIMBLUPIX;
			src2.top = m_moveObject[i].posCurrent.y + 36;
			src2.bottom = m_moveObject[i].posCurrent.y + DIMBLUPIY;
			RECT tinyRect;
			if (IntersectRect(&tinyRect, &src2, &src))
			{
				if (m_blupiDir == DIR_RIGHT)
				{
					if (pos.x >= src2.left) goto mockeryi;
					if (m_moveObject[i].type == TYPE_BOMBEDOWN) return 0;
					return ACTION_MOCKERY;
				}
				else
				{
					if (pos.x < src2.left)
					{
mockeryi:
						return ACTION_MOCKERYi;
					}
					if (m_moveObject[i].type == TYPE_BOMBEDOWN) return 0;
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

	RECT src2;
	RECT src;
	src.left = pos.x + 16;
	src.right = pos.x + DIMBLUPIX - 16;
	src.top = pos.y + 11;
	src.bottom = pos.y + DIMBLUPIY - 2;
	src2.left = m_blupiPos.x - 16 - 40;
	src2.right = m_blupiPos.x + DIMBLUPIX + 16 + 40;
	src2.top = m_blupiPos.y + 11 - 40;
	src2.bottom = m_blupiPos.y + DIMBLUPIY - 2 + 40;
	RECT tinyRect;
	return IntersectRect(&tinyRect, &src, &src2) != 0;
}

int CDecor::MoveObjectDetect(POINT pos, BOOL* pbNear)
{
	RECT src3;
	RECT src;
	RECT rect;
	src.left = pos.x + 16;
	src.right = pos.x + DIMBLUPIX - 16;
	src.top = pos.y + 11;
	src.bottom = pos.y + DIMBLUPIY - 2;
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
			type != TYPE_GLU &&
			type != TYPE_CLEAR &&
			type != TYPE_ELECTRO &&
			((m_blupiAction != ACTION_PUSH && m_blupiAction != ACTION_POP) ||
			type != TYPE_CAISSE))
		{
			if ((type == TYPE_HELICO || type == TYPE_JEEP || type == TYPE_SKATE || type == TYPE_TANK) &&
				m_moveObject[i].posCurrent.x == m_blupiPosHelico.x &&
				m_moveObject[i].posCurrent.y <= m_blupiPosHelico.y &&
				m_moveObject[i].posCurrent.y >= m_blupiPosHelico.y - 2)
			{
				continue;
			}
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
				src3.bottom = m_moveObject[i].posCurrent.y + 24;
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
				type == TYPE_OISEAU)
			{
				src3.top = m_moveObject[i].posCurrent.y + 16;
				src3.bottom = m_moveObject[i].posCurrent.y + DIMBLUPIY - 16;
			}
			if (type == TYPE_BALLE)
			{
				src3.left = m_moveObject[i].posCurrent.x + 24;
				src3.right = m_moveObject[i].posCurrent.x + DIMOBJX - 24;
				src3.top = m_moveObject[i].posCurrent.y + 10;
				src3.bottom = m_moveObject[i].posCurrent.y + DIMBLUPIY - 32;
			}
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
			RECT tinyRect;
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
			src2.left = m_moveObject[i].posCurrent.x + 16;
			src2.right = m_moveObject[i].posCurrent.x + DIMBLUPIX - 16;
			src2.top = m_moveObject[i].posCurrent.y + 36;
			src2.bottom = m_moveObject[i].posCurrent.y + DIMBLUPIY;
			RECT tinyRect;
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
	rcSrc2.left = pos.x + 20;
	rcSrc2.right = pos.x + DIMBLUPIX - 16;
	rcSrc2.top = pos.y + 10;
	rcSrc2.bottom = pos.y + 28;
	for (i = 0; i < MAXMOVEOBJECT; i++)
	{
		if (m_moveObject[i].type == TYPE_BULLDOZER ||
			m_moveObject[i].type == TYPE_POISSON ||
			m_moveObject[i].type == TYPE_OISEAU ||
			m_moveObject[i].type == TYPE_BLUPIHELICO ||
			m_moveObject[i].type == TYPE_BLUPITANK)
		{
			rcSrc1.left = m_moveObject[i].posCurrent.x + 16;
			rcSrc1.right = m_moveObject[i].posCurrent.x + DIMBLUPIX - 16;
			rcSrc1.top = m_moveObject[i].posCurrent.y + 16;
			rcSrc1.bottom = m_moveObject[i].posCurrent.y + DIMBLUPIY - 10;
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
	BOOL flag;
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
	if (num > 1)
	{
		do
		{
			flag = FALSE;
			for (i = 0; i < num - 1; i++)
			{
				if (::SortGetType(m_moveObject[i].type) > ::SortGetType(m_moveObject[i + 1].type))
				{
					MoveObjectCopy(&dst, &m_moveObject[i]);
					MoveObjectCopy(&m_moveObject[i], &m_moveObject[i + 1]);
					MoveObjectCopy(&m_moveObject[i + 1], &dst);
					flag = TRUE;
				}
			}
		} while (flag);
	}
}

void CDecor::MoveObjectPriority(int rank)
{
	if (rank != 0 && m_moveObject[rank].type == TYPE_BALLE)
	{
		int i = 0;
		for (; i < MAXMOVEOBJECT; i++)
		{
			if (m_moveObject[i].type != TYPE_BALLE) goto MoveObjectPriorityFound;
		}
		return;
MoveObjectPriorityFound:
		if (i <= rank)
		{
			MoveObject tempMob;
			MoveObjectCopy(&tempMob, &m_moveObject[rank]);
			MoveObjectCopy(&m_moveObject[rank], &m_moveObject[i]);
			MoveObjectCopy(&m_moveObject[i], &tempMob);
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
