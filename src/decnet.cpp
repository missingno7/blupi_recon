// decnet.cpp candidate: SB2 ancestor methods translated into target TU order.

#include "def.h"
#include "decor.h"
#include "misc.h"
#include "network.h"

void CDecor::NetStopCloud(int rank)
{
	NetMessage msg;

	msg.type = PK_STOPCLOUD;
	msg.data1 = 0;
	msg.data2 = 0;
	msg.data3 = 0;
	msg.data4 = rank;
	NetMessagePush(&msg);
}

BOOL CDecor::ObjectStart(POINT pos, int type, int speed, BOOL bMulti)
{
	NetMessage msg;
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
		int dist = 0;
		POINT tinyPoint = pos;
		int num2 = speed;
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

void CDecor::ModifDecor(POINT pos, int icon, BOOL bMulti)
{
	NetMessage message;
	int index = pos.x / DIMOBJX;
	index *= MAXCELY;
	index += pos.y / DIMOBJY;
	((Cellule *)m_decor)[index].icon = icon;
	if (bMulti)
	{
		message.type = PK_MODIFDECOR;
		message.data1 = 0;
		message.posX = pos.x;
		message.posY = pos.y;
		message.decorIcon = icon;
		NetMessagePush(&message);
	}
}

void CDecor::NetPlaySound(short channel, POINT pos)
{
	NetMessage msg;

	msg.type = PK_PLAYSOUND;
	msg.data1 = 0;
	msg.data2 = pos.x;
	msg.data3 = pos.y;
	msg.data4 = channel;
	NetMessagePush(&msg);
}

void CDecor::NetStopSound(short channel)
{
	NetMessage msg;

	msg.type = PK_STOPSOUND;
	msg.data1 = 0;
	msg.data2 = 0;
	msg.data3 = 0;
	msg.data4 = channel;
	NetMessagePush(&msg);
}
