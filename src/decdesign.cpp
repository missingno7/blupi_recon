// DecDesign.cpp
//

#include <def.h>
#include <decor.h>
#include <misc.h>
#include <dectables.h>
#include <design_tables.h>
#include <stdio.h>

POINT CDecor::ScreenPosToCelPos(POINT pos)
{
	POINT result;
	if (pos.x >= 0 && pos.x < LXIMAGE && pos.y >= 0 && pos.y < LYIMAGE)
	{
		result.x = (m_posDecor.x - m_dimCelHili.x * 32 + 32 + pos.x) / DIMOBJX;
		result.y = (m_posDecor.y - m_dimCelHili.y * 32 + 32 + pos.y) / DIMOBJY;
	}
	else
	{
		result.x = -1;
		result.y = -1;
	}
	return result;
}

void CDecor::SetCelHiliFromScreenPos(POINT pos)
{
	m_posCelHili = ScreenPosToCelPos(pos);
}

// set m_iconLift and m_dimCelHili based on the selected menu
void CDecor::SetIconLiftAndStuff(int menu, int index)
{
	m_dimCelHili.x = 1;
	m_dimCelHili.y = 1;
	if (menu == 2 && index != 3)
	{
		m_dimCelHili.x = 2;
		m_dimCelHili.y = 2;
	}
	if (menu == 3) m_iconLift = index;
	if (menu == 4) m_iconLift = index + 20;
	m_liftEnd = -1;
}

void CDecor::DeleteCel(POINT cel)
{
	m_liftEnd = -1;

	if (!IsValidCel(cel)) return;

	m_decor[cel.x][cel.y].icon = -1;
	AdaptBorder(cel);
	m_bigDecor[cel.x][cel.y].icon = -1;
	MoveObjectDelete(cel);
}

void __cdecl PlaceDecor(int *pCel, int *pTable, int *pLastIndex, int fwKeys, BOOL bMix, BOOL bRand)
{
	if (fwKeys & MK_CONTROL) {
		if (bRand) *pCel = pTable[rand() % pTable[0] + 1];
		else *pCel = pTable[*pLastIndex + 1];
	}
	else if (fwKeys & MK_LBUTTON) {
		if (*pCel == pTable[*pLastIndex + 1]) {
			if (bMix) {
				(*pLastIndex)++;
				if (*pLastIndex >= pTable[0]) *pLastIndex = 0;
			}
		}
		*pCel = pTable[*pLastIndex + 1];
	}
}

void CDecor::PlaceDecor1(POINT cel, int index, int fwKeys, BOOL bMix)
{
    m_liftEnd = -1;
    int icon;
    if (cel.x >= 0 && cel.x < 100 && cel.y >= 0 && cel.y < 100)
    {
        if (index == 0)
        {
            icon = m_bigDecor[cel.x][cel.y].icon;
            ::PlaceDecor(&icon, table_366b0, &m_lastDecorIndexes[20], fwKeys, bMix, TRUE);
            m_bigDecor[cel.x][cel.y].icon = (short)icon;
        }
        if (index == 1)
        {
            icon = m_bigDecor[cel.x][cel.y].icon;
            ::PlaceDecor(&icon, table_366c0, &m_lastDecorIndexes[21], fwKeys, bMix, TRUE);
            m_bigDecor[cel.x][cel.y].icon = (short)icon;
        }
        if (index == 2)
        {
            icon = m_bigDecor[cel.x][cel.y].icon;
            ::PlaceDecor(&icon, table_366d8, &m_lastDecorIndexes[22], fwKeys, bMix, TRUE);
            m_bigDecor[cel.x][cel.y].icon = (short)icon;
        }
        if (index == 3) m_bigDecor[cel.x][cel.y].icon = 0xcb;
        if (index == 4)
        {
            icon = m_bigDecor[cel.x][cel.y].icon;
            ::PlaceDecor(&icon, table_366e8, &m_lastDecorIndexes[23], fwKeys, bMix, TRUE);
            m_bigDecor[cel.x][cel.y].icon = (short)icon;
        }
        if (index == 5)
        {
            icon = m_bigDecor[cel.x][cel.y].icon;
            ::PlaceDecor(&icon, table_366f8, &m_lastDecorIndexes[24], fwKeys, bMix, TRUE);
            m_bigDecor[cel.x][cel.y].icon = (short)icon;
        }
        if (index == 6)
        {
            icon = m_bigDecor[cel.x][cel.y].icon;
            ::PlaceDecor(&icon, table_36710, &m_lastDecorIndexes[25], fwKeys, bMix, TRUE);
            m_bigDecor[cel.x][cel.y].icon = (short)icon;
        }
    }
}

void CDecor::PlaceDecor2(POINT cel, int index, int fwKeys, BOOL bMix)
{
    m_liftEnd = -1;
    int icon;
    if (cel.x >= 0 && cel.x < 100 && cel.y >= 0 && cel.y < 100)
    {
        m_iconLift = index;
        if (index == 0)
        {
            icon = m_decor[cel.x][cel.y].icon;
            ::PlaceDecor(&icon, table_36418, &m_lastDecorIndexes[0], fwKeys, bMix, FALSE);
            m_decor[cel.x][cel.y].icon = (short)icon;
        }
        if (index == 1)
        {
            icon = m_decor[cel.x][cel.y].icon;
            ::PlaceDecor(&icon, table_36458, &m_lastDecorIndexes[1], fwKeys, bMix, FALSE);
            m_decor[cel.x][cel.y].icon = (short)icon;
        }
        if (index == 2)
        {
            icon = m_decor[cel.x][cel.y].icon;
            ::PlaceDecor(&icon, table_36488, &m_lastDecorIndexes[2], fwKeys, bMix, FALSE);
            m_decor[cel.x][cel.y].icon = (short)icon;
        }
        if (index == 3)
        {
            icon = m_decor[cel.x][cel.y].icon;
            ::PlaceDecor(&icon, table_364a0, &m_lastDecorIndexes[3], fwKeys, bMix, TRUE);
            m_decor[cel.x][cel.y].icon = (short)icon;
        }
        if (index == 4)
        {
            icon = m_decor[cel.x][cel.y].icon;
            ::PlaceDecor(&icon, table_364b0, &m_lastDecorIndexes[4], fwKeys, bMix, TRUE);
            m_decor[cel.x][cel.y].icon = (short)icon;
        }
        if (index == 5)
        {
            icon = m_decor[cel.x][cel.y].icon;
            ::PlaceDecor(&icon, table_364d0, &m_lastDecorIndexes[5], fwKeys, bMix, FALSE);
            m_decor[cel.x][cel.y].icon = (short)icon;
        }
        if (index == 6)
        {
            icon = m_decor[cel.x][cel.y].icon;
            ::PlaceDecor(&icon, table_364e8, &m_lastDecorIndexes[6], fwKeys, bMix, FALSE);
            m_decor[cel.x][cel.y].icon = (short)icon;
        }
        if (index == 7)
        {
            icon = m_decor[cel.x][cel.y].icon;
            ::PlaceDecor(&icon, table_364f8, &m_lastDecorIndexes[7], fwKeys, bMix, TRUE);
            m_decor[cel.x][cel.y].icon = (short)icon;
        }
        if (index == 8)
        {
            icon = m_decor[cel.x][cel.y].icon;
            ::PlaceDecor(&icon, table_36510, &m_lastDecorIndexes[8], fwKeys, bMix, FALSE);
            m_decor[cel.x][cel.y].icon = (short)icon;
        }
        if (index == 9)
        {
            icon = m_decor[cel.x][cel.y].icon;
            ::PlaceDecor(&icon, table_36520, &m_lastDecorIndexes[9], fwKeys, bMix, FALSE);
            m_decor[cel.x][cel.y].icon = (short)icon;
        }
        AdaptBorder(cel);
    }
}

void CDecor::PlaceDecor3(POINT cel, int index, int fwKeys, BOOL bMix)
{
    m_liftEnd = -1;
    int icon;
    if (cel.x >= 0 && cel.x < 100 && cel.y >= 0 && cel.y < 100)
    {
        m_iconLift = index + 20;
        if (index == 0)
        {
            icon = m_decor[cel.x][cel.y].icon;
            ::PlaceDecor(&icon, table_36530, &m_lastDecorIndexes[10], fwKeys, bMix, TRUE);
            m_decor[cel.x][cel.y].icon = (short)icon;
        }
        if (index == 1)
        {
            icon = m_decor[cel.x][cel.y].icon;
            ::PlaceDecor(&icon, table_36538, &m_lastDecorIndexes[11], fwKeys, bMix, FALSE);
            m_decor[cel.x][cel.y].icon = (short)icon;
        }
        if (index == 2)
        {
            icon = m_decor[cel.x][cel.y].icon;
            ::PlaceDecor(&icon, table_36548, &m_lastDecorIndexes[12], fwKeys, bMix, TRUE);
            m_decor[cel.x][cel.y].icon = (short)icon;
        }
        if (index == 3)
        {
            icon = m_decor[cel.x][cel.y].icon;
            ::PlaceDecor(&icon, table_36550, &m_lastDecorIndexes[13], fwKeys, bMix, TRUE);
            m_decor[cel.x][cel.y].icon = (short)icon;
        }
        if (index == 4)
        {
            icon = m_decor[cel.x][cel.y].icon;
            ::PlaceDecor(&icon, table_36570, &m_lastDecorIndexes[14], fwKeys, bMix, TRUE);
            m_decor[cel.x][cel.y].icon = (short)icon;
        }
        if (index == 5)
        {
            icon = m_decor[cel.x][cel.y].icon;
            ::PlaceDecor(&icon, table_365a8, &m_lastDecorIndexes[15], fwKeys, bMix, TRUE);
            m_decor[cel.x][cel.y].icon = (short)icon;
        }
        if (index == 6)
        {
            icon = m_decor[cel.x][cel.y].icon;
            ::PlaceDecor(&icon, table_365d0, &m_lastDecorIndexes[16], fwKeys, bMix, TRUE);
            m_decor[cel.x][cel.y].icon = (short)icon;
        }
        if (index == 7)
        {
            icon = m_decor[cel.x][cel.y].icon;
            ::PlaceDecor(&icon, table_365d8, &m_lastDecorIndexes[17], fwKeys, bMix, FALSE);
            m_decor[cel.x][cel.y].icon = (short)icon;
        }
        if (index == 8)
        {
            icon = m_decor[cel.x][cel.y].icon;
            ::PlaceDecor(&icon, table_365f0, &m_lastDecorIndexes[18], fwKeys, bMix, FALSE);
            m_decor[cel.x][cel.y].icon = (short)icon;
        }
        if (index == 9) m_decor[cel.x][cel.y].icon = 0xd6;
        AdaptBorder(cel);
    }
}

void CDecor::PlaceDecor4(POINT cel, int index, int fwKeys, BOOL bMix)
{
	int *pTable;
	int iconLift, rank, icon, i, distX, distY;
	POINT pos;
	if (!bMix || !IsValidCel(cel)) return;
	iconLift = m_iconLift;
	pTable = table_368f8;
	if (iconLift == 0) pTable = table_368f8;
	if (iconLift == 1) pTable = table_368f8;
	if (iconLift == 2) pTable = table_368f8;
	if (iconLift == 3) pTable = table_36848;
	if (iconLift == 4) pTable = table_36888;
	if (iconLift == 5) pTable = table_368a8;
	if (iconLift == 6) pTable = table_36848;
	if (iconLift == 7) pTable = table_368c0;
	if (iconLift == 8) pTable = table_368e0;
	if (iconLift == 9) pTable = table_368c0;
	if (iconLift == 21) pTable = table_36908;
	if (iconLift == 22) pTable = table_36980;
	if (iconLift == 23) pTable = table_36928;
	if (iconLift == 24) pTable = table_36950;
	if (iconLift == 27) pTable = table_36968;

	rank = m_liftEnd;
	if (rank != -1)
	{
		if ((m_moveObject[rank].type == TYPE_BLUPIHELICO || m_moveObject[rank].type == TYPE_BLUPITANK) &&
			m_moveObject[rank].posStart.x == cel.x * DIMOBJX && m_moveObject[rank].posStart.y == cel.y * DIMOBJY)
		{
			icon = m_moveObject[rank].channel + 1;
			if (icon > CHBLUPI3) icon = CHBLUPI1;
			m_moveObject[rank].channel = icon;
		}
		if (m_moveObject[rank].posStart.x == cel.x * DIMOBJX && m_moveObject[rank].posStart.y == cel.y * DIMOBJY)
		{
			m_lastDecorIndexes[40]++;
			if (m_lastDecorIndexes[40] >= pTable[0]) m_lastDecorIndexes[40] = 0;
			m_moveObject[rank].icon = pTable[m_lastDecorIndexes[40]+1];
			return;
		}
		m_moveObject[rank].posEnd.x = cel.x * DIMOBJX;
		m_moveObject[rank].posEnd.y = cel.y * DIMOBJY;
		distX = abs(m_moveObject[rank].posStart.x - m_moveObject[rank].posEnd.x);
		distY = abs(m_moveObject[rank].posStart.y - m_moveObject[rank].posEnd.y);
		if (distX > distY) distY = distX;
		distY /= DIMOBJX;
		m_moveObject[rank].stepAdvance *= distY;
		m_moveObject[rank].stepRecede *= distY;
		AscenseurSynchro(rank);
		m_liftEnd = -1;
		return;
	}

	if (index == 0)
	{
		MoveObjectDelete(cel);
		rank = MoveObjectFree();
		if (rank == -1) return;
		if (m_lastDecorIndexes[40] >= pTable[0]) m_lastDecorIndexes[40] = 0;
		m_moveObject[rank].type = TYPE_ASCENSEUR;
		m_moveObject[rank].stepAdvance = 20;
		m_moveObject[rank].stepRecede = 20;
		m_moveObject[rank].timeStopStart = 20;
		m_moveObject[rank].timeStopEnd = 20;
		m_moveObject[rank].posStart.x = cel.x * DIMOBJX;
		m_moveObject[rank].posStart.y = cel.y * DIMOBJY;
		m_moveObject[rank].posEnd = m_moveObject[rank].posStart;
		m_moveObject[rank].posCurrent = m_moveObject[rank].posStart;
		m_moveObject[rank].step = STEP_STOPSTART;
		m_moveObject[rank].time = 0;
		m_moveObject[rank].phase = 0;
		m_moveObject[rank].channel = 1;
		m_moveObject[rank].icon = pTable[m_lastDecorIndexes[40]+1];
		m_liftEnd = rank;
	}

	if (index == 1)
	{
		MoveObjectDelete(cel);
		rank = MoveObjectFree();
		if (rank == -1) return;
		if (m_lastDecorIndexes[40] >= pTable[0]) m_lastDecorIndexes[40] = 0;
		m_moveObject[rank].type = TYPE_ASCENSEUR;
		m_moveObject[rank].stepAdvance = 10;
		m_moveObject[rank].stepRecede = 10;
		m_moveObject[rank].timeStopStart = 10;
		m_moveObject[rank].timeStopEnd = 10;
		m_moveObject[rank].posStart.x = cel.x * DIMOBJX;
		m_moveObject[rank].posStart.y = cel.y * DIMOBJY;
		m_moveObject[rank].posEnd = m_moveObject[rank].posStart;
		m_moveObject[rank].posCurrent = m_moveObject[rank].posStart;
		m_moveObject[rank].step = STEP_STOPSTART;
		m_moveObject[rank].time = 0;
		m_moveObject[rank].phase = 0;
		m_moveObject[rank].channel = 1;
		m_moveObject[rank].icon = pTable[m_lastDecorIndexes[40]+1];
		m_liftEnd = rank;
	}

	if (index == 2)
	{
		MoveObjectDelete(cel);
		rank = MoveObjectFree();
		if (rank == -1) return;
		if (m_lastDecorIndexes[40] >= pTable[0]) m_lastDecorIndexes[40] = 0;
		m_moveObject[rank].type = TYPE_ASCENSEUR;
		m_moveObject[rank].stepAdvance = 6;
		m_moveObject[rank].stepRecede = 6;
		m_moveObject[rank].timeStopStart = 3;
		m_moveObject[rank].timeStopEnd = 3;
		m_moveObject[rank].posStart.x = cel.x * DIMOBJX;
		m_moveObject[rank].posStart.y = cel.y * DIMOBJY;
		m_moveObject[rank].posEnd = m_moveObject[rank].posStart;
		m_moveObject[rank].posCurrent = m_moveObject[rank].posStart;
		m_moveObject[rank].step = STEP_STOPSTART;
		m_moveObject[rank].time = 0;
		m_moveObject[rank].phase = 0;
		m_moveObject[rank].channel = 1;
		m_moveObject[rank].icon = pTable[m_lastDecorIndexes[40]+1];
		m_liftEnd = rank;
	}

	if (index == 3)
	{
		MoveObjectDelete(cel);
		rank = MoveObjectFree();
		if (rank == -1) return;
		if (m_lastDecorIndexes[40] >= pTable[0]) m_lastDecorIndexes[40] = 0;
		m_moveObject[rank].type = TYPE_ASCENSEUR;
		m_moveObject[rank].stepAdvance = 5;
		m_moveObject[rank].stepRecede = 22;
		m_moveObject[rank].timeStopStart = 2;
		m_moveObject[rank].timeStopEnd = 2;
		m_moveObject[rank].posStart.x = cel.x * DIMOBJX;
		m_moveObject[rank].posStart.y = cel.y * DIMOBJY;
		m_moveObject[rank].posEnd = m_moveObject[rank].posStart;
		m_moveObject[rank].posCurrent = m_moveObject[rank].posStart;
		m_moveObject[rank].step = STEP_STOPSTART;
		m_moveObject[rank].time = 0;
		m_moveObject[rank].phase = 0;
		m_moveObject[rank].channel = 1;
		m_moveObject[rank].icon = pTable[m_lastDecorIndexes[40]+1];
		m_liftEnd = rank;
	}

	if (index == 4)
	{
		pos.x = cel.x * DIMOBJX;
		pos.y = cel.y * DIMOBJY;
		rank = MoveObjectSearch(pos, -1);
		if (rank != -1)
		{
			icon = m_moveObject[rank].icon;
			for (i = 0; i < table_369c0[0]; i++)
			{
				if (icon == table_369c0[i+1]) m_lastDecorIndexes[41] = i;
			}
			m_lastDecorIndexes[41]++;
			if (m_lastDecorIndexes[41] >= table_369c0[0]) m_lastDecorIndexes[41] = 0;
		}
		icon = table_369c0[m_lastDecorIndexes[41]+1];
		MoveObjectDelete(cel);
		rank = MoveObjectFree();
		if (rank == -1) return;
		m_moveObject[rank].type = TYPE_CAISSE;
		m_moveObject[rank].stepAdvance = 1;
		m_moveObject[rank].stepRecede = 1;
		m_moveObject[rank].timeStopStart = 0;
		m_moveObject[rank].timeStopEnd = 0;
		m_moveObject[rank].posStart.x = cel.x * DIMOBJX;
		m_moveObject[rank].posStart.y = cel.y * DIMOBJY;
		m_moveObject[rank].posEnd = m_moveObject[rank].posStart;
		m_moveObject[rank].posCurrent = m_moveObject[rank].posStart;
		m_moveObject[rank].step = STEP_STOPSTART;
		m_moveObject[rank].time = 0;
		m_moveObject[rank].phase = 0;
		m_moveObject[rank].channel = 1;
		m_moveObject[rank].icon = icon;
	}

	if (index == 5)
	{
		pos.x = cel.x * DIMOBJX;
		pos.y = cel.y * DIMOBJY;
		rank = MoveObjectSearch(pos, -1);
		if (rank != -1)
		{
			icon = m_moveObject[rank].icon;
			for (i = 0; i < table_369c8[0]; i++)
			{
				if (icon == table_369c8[i+1]) m_lastDecorIndexes[41] = i;
			}
			m_lastDecorIndexes[41]++;
			if (m_lastDecorIndexes[41] >= table_369c8[0]) m_lastDecorIndexes[41] = 0;
		}
		icon = table_369c8[m_lastDecorIndexes[41]+1];
		MoveObjectDelete(cel);
		rank = MoveObjectFree();
		if (rank == -1) return;
		m_moveObject[rank].type = TYPE_CAISSE;
		m_moveObject[rank].stepAdvance = 1;
		m_moveObject[rank].stepRecede = 1;
		m_moveObject[rank].timeStopStart = 0;
		m_moveObject[rank].timeStopEnd = 0;
		m_moveObject[rank].posStart.x = cel.x * DIMOBJX;
		m_moveObject[rank].posStart.y = cel.y * DIMOBJY;
		m_moveObject[rank].posEnd = m_moveObject[rank].posStart;
		m_moveObject[rank].posCurrent = m_moveObject[rank].posStart;
		m_moveObject[rank].step = STEP_STOPSTART;
		m_moveObject[rank].time = 0;
		m_moveObject[rank].phase = 0;
		m_moveObject[rank].channel = 1;
		m_moveObject[rank].icon = icon;
	}

	if (index == 6)
	{
		MoveObjectDelete(cel);
		rank = MoveObjectFree();
		if (rank == -1) return;
		m_moveObject[rank].type = TYPE_BULLDOZER;
		m_moveObject[rank].stepAdvance = 30;
		m_moveObject[rank].stepRecede = 30;
		m_moveObject[rank].timeStopStart = 20;
		m_moveObject[rank].timeStopEnd = 20;
		m_moveObject[rank].posStart.x = cel.x * DIMOBJX;
		m_moveObject[rank].posStart.y = cel.y * DIMOBJY;
		m_moveObject[rank].posEnd = m_moveObject[rank].posStart;
		m_moveObject[rank].posCurrent = m_moveObject[rank].posStart;
		m_moveObject[rank].step = STEP_STOPSTART;
		m_moveObject[rank].time = 0;
		m_moveObject[rank].phase = 0;
		m_moveObject[rank].channel = 10;
		m_moveObject[rank].icon = 0x39;
		m_liftEnd = rank;
	}

	if (index == 7)
	{
		MoveObjectDelete(cel);
		rank = MoveObjectFree();
		if (rank == -1) return;
		m_moveObject[rank].type = TYPE_POISSON;
		m_moveObject[rank].stepAdvance = 15;
		m_moveObject[rank].stepRecede = 15;
		m_moveObject[rank].timeStopStart = 47;
		m_moveObject[rank].timeStopEnd = 47;
		m_moveObject[rank].posStart.x = cel.x * DIMOBJX;
		m_moveObject[rank].posStart.y = cel.y * DIMOBJY;
		m_moveObject[rank].posEnd = m_moveObject[rank].posStart;
		m_moveObject[rank].posCurrent = m_moveObject[rank].posStart;
		m_moveObject[rank].step = STEP_STOPSTART;
		m_moveObject[rank].time = 0;
		m_moveObject[rank].phase = 0;
		m_moveObject[rank].channel = 10;
		m_moveObject[rank].icon = 0x4f;
		m_liftEnd = rank;
	}

	if (index == 8)
	{
		MoveObjectDelete(cel);
		rank = MoveObjectFree();
		if (rank == -1) return;
		m_moveObject[rank].type = TYPE_OISEAU;
		m_moveObject[rank].stepAdvance = 8;
		m_moveObject[rank].stepRecede = 8;
		m_moveObject[rank].timeStopStart = 8;
		m_moveObject[rank].timeStopEnd = 8;
		m_moveObject[rank].posStart.x = cel.x * DIMOBJX;
		m_moveObject[rank].posStart.y = cel.y * DIMOBJY;
		m_moveObject[rank].posEnd = m_moveObject[rank].posStart;
		m_moveObject[rank].posCurrent = m_moveObject[rank].posStart;
		m_moveObject[rank].step = STEP_STOPSTART;
		m_moveObject[rank].time = 0;
		m_moveObject[rank].phase = 0;
		m_moveObject[rank].channel = 10;
		m_moveObject[rank].icon = 0x5a;
		m_liftEnd = rank;
	}

	if (index == 9)
	{
		MoveObjectDelete(cel);
		rank = MoveObjectFree();
		if (rank == -1) return;
		m_moveObject[rank].type = TYPE_BOMBEMOVE;
		m_moveObject[rank].stepAdvance = 10;
		m_moveObject[rank].stepRecede = 10;
		m_moveObject[rank].timeStopStart = 0;
		m_moveObject[rank].timeStopEnd = 0;
		m_moveObject[rank].posStart.x = cel.x * DIMOBJX;
		m_moveObject[rank].posStart.y = cel.y * DIMOBJY;
		m_moveObject[rank].posEnd = m_moveObject[rank].posStart;
		m_moveObject[rank].posCurrent = m_moveObject[rank].posStart;
		m_moveObject[rank].step = STEP_STOPSTART;
		m_moveObject[rank].time = 0;
		m_moveObject[rank].phase = 0;
		m_moveObject[rank].channel = 10;
		m_moveObject[rank].icon = 0x45;
		m_liftEnd = rank;
	}

	if (index == 10)
	{
		MoveObjectDelete(cel);
		rank = MoveObjectFree();
		if (rank == -1) return;
		m_moveObject[rank].type = TYPE_BLUPIHELICO;
		m_moveObject[rank].stepAdvance = 8;
		m_moveObject[rank].stepRecede = 8;
		m_moveObject[rank].timeStopStart = 25;
		m_moveObject[rank].timeStopEnd = 25;
		m_moveObject[rank].posStart.x = cel.x * DIMOBJX;
		m_moveObject[rank].posStart.y = cel.y * DIMOBJY;
		m_moveObject[rank].posEnd = m_moveObject[rank].posStart;
		m_moveObject[rank].posCurrent = m_moveObject[rank].posStart;
		m_moveObject[rank].step = STEP_STOPSTART;
		m_moveObject[rank].time = 0;
		m_moveObject[rank].phase = 0;
		m_moveObject[rank].channel = 12;
		m_moveObject[rank].icon = 0x40;
		m_liftEnd = rank;
	}

	if (index == 11)
	{
		MoveObjectDelete(cel);
		rank = MoveObjectFree();
		if (rank == -1) return;
		m_moveObject[rank].type = TYPE_BLUPITANK;
		m_moveObject[rank].stepAdvance = 10;
		m_moveObject[rank].stepRecede = 10;
		m_moveObject[rank].timeStopStart = 23;
		m_moveObject[rank].timeStopEnd = 23;
		m_moveObject[rank].posStart.x = cel.x * DIMOBJX;
		m_moveObject[rank].posStart.y = cel.y * DIMOBJY;
		m_moveObject[rank].posEnd = m_moveObject[rank].posStart;
		m_moveObject[rank].posCurrent = m_moveObject[rank].posStart;
		m_moveObject[rank].step = STEP_STOPSTART;
		m_moveObject[rank].time = 0;
		m_moveObject[rank].phase = 0;
		m_moveObject[rank].channel = 12;
		m_moveObject[rank].icon = 0xf0;
		m_liftEnd = rank;
	}
}

void CDecor::PlaceDecor5(POINT cel, int index, int fwKeys, BOOL bMix)
{
	m_liftEnd = -1;
	int rank, i;
	POINT pos;
	if (!IsValidCel(cel)) return;

	if (index == 0)
	{
		MoveObjectDelete(cel);
		rank = MoveObjectFree();
		if (rank == -1) return;
		m_moveObject[rank].type = TYPE_TRESOR;
		m_moveObject[rank].stepAdvance = 1;
		m_moveObject[rank].stepRecede = 1;
		m_moveObject[rank].timeStopStart = 0;
		m_moveObject[rank].timeStopEnd = 0;
		m_moveObject[rank].posStart.x = cel.x * DIMOBJX + 2;
		m_moveObject[rank].posStart.y = cel.y * DIMOBJY + 4;
		m_moveObject[rank].posEnd = m_moveObject[rank].posStart;
		m_moveObject[rank].posCurrent = m_moveObject[rank].posStart;
		m_moveObject[rank].step = STEP_STOPSTART;
		m_moveObject[rank].time = 0;
		m_moveObject[rank].phase = 0;
		m_moveObject[rank].channel = 10;
		m_moveObject[rank].icon = 0x0;
	}

	if (index == 1)
	{
		MoveObjectDelete(cel);
		rank = MoveObjectFree();
		if (rank == -1) return;
		m_moveObject[rank].type = TYPE_EGG;
		m_moveObject[rank].stepAdvance = 1;
		m_moveObject[rank].stepRecede = 1;
		m_moveObject[rank].timeStopStart = 0;
		m_moveObject[rank].timeStopEnd = 0;
		m_moveObject[rank].posStart.x = cel.x * DIMOBJX + 2;
		m_moveObject[rank].posStart.y = cel.y * DIMOBJY + 8;
		m_moveObject[rank].posEnd = m_moveObject[rank].posStart;
		m_moveObject[rank].posCurrent = m_moveObject[rank].posStart;
		m_moveObject[rank].step = STEP_STOPSTART;
		m_moveObject[rank].time = 0;
		m_moveObject[rank].phase = 0;
		m_moveObject[rank].channel = 10;
		m_moveObject[rank].icon = 0x15;
	}

	if (index == 2)
	{
		MoveObjectDelete(cel);
		rank = MoveObjectFree();
		if (rank == -1) return;
		m_moveObject[rank].type = TYPE_POWER;
		m_moveObject[rank].stepAdvance = 1;
		m_moveObject[rank].stepRecede = 1;
		m_moveObject[rank].timeStopStart = 0;
		m_moveObject[rank].timeStopEnd = 0;
		m_moveObject[rank].posStart.x = cel.x * DIMOBJX + 2;
		m_moveObject[rank].posStart.y = cel.y * DIMOBJY + 8;
		m_moveObject[rank].posEnd = m_moveObject[rank].posStart;
		m_moveObject[rank].posCurrent = m_moveObject[rank].posStart;
		m_moveObject[rank].step = STEP_STOPSTART;
		m_moveObject[rank].time = 0;
		m_moveObject[rank].phase = 0;
		m_moveObject[rank].channel = 10;
		m_moveObject[rank].icon = 0x88;
	}

	if (index == 3)
	{
		MoveObjectDelete(cel);
		rank = MoveObjectFree();
		if (rank == -1) return;
		m_moveObject[rank].type = TYPE_SHIELD;
		m_moveObject[rank].stepAdvance = 1;
		m_moveObject[rank].stepRecede = 1;
		m_moveObject[rank].timeStopStart = 0;
		m_moveObject[rank].timeStopEnd = 0;
		m_moveObject[rank].posStart.x = cel.x * DIMOBJX + 2;
		m_moveObject[rank].posStart.y = cel.y * DIMOBJY + 8;
		m_moveObject[rank].posEnd = m_moveObject[rank].posStart;
		m_moveObject[rank].posCurrent = m_moveObject[rank].posStart;
		m_moveObject[rank].step = STEP_STOPSTART;
		m_moveObject[rank].time = 0;
		m_moveObject[rank].phase = 0;
		m_moveObject[rank].channel = 10;
		m_moveObject[rank].icon = 0x90;
	}

	if (index == 4)
	{
		MoveObjectDelete(cel);
		rank = MoveObjectFree();
		if (rank == -1) return;
		m_moveObject[rank].type = TYPE_DRINK;
		m_moveObject[rank].stepAdvance = 1;
		m_moveObject[rank].stepRecede = 1;
		m_moveObject[rank].timeStopStart = 0;
		m_moveObject[rank].timeStopEnd = 0;
		m_moveObject[rank].posStart.x = cel.x * DIMOBJX + 2;
		m_moveObject[rank].posStart.y = cel.y * DIMOBJY + 6;
		m_moveObject[rank].posEnd = m_moveObject[rank].posStart;
		m_moveObject[rank].posCurrent = m_moveObject[rank].posStart;
		m_moveObject[rank].step = STEP_STOPSTART;
		m_moveObject[rank].time = 0;
		m_moveObject[rank].phase = 0;
		m_moveObject[rank].channel = 10;
		m_moveObject[rank].icon = 0xb2;
	}

	if (index == 5)
	{
		MoveObjectDelete(cel);
		rank = MoveObjectFree();
		if (rank == -1) return;
		m_moveObject[rank].type = TYPE_BULLET;
		m_moveObject[rank].stepAdvance = 1;
		m_moveObject[rank].stepRecede = 1;
		m_moveObject[rank].timeStopStart = 0;
		m_moveObject[rank].timeStopEnd = 0;
		m_moveObject[rank].posStart.x = cel.x * DIMOBJX + 2;
		m_moveObject[rank].posStart.y = cel.y * DIMOBJY + 6;
		m_moveObject[rank].posEnd = m_moveObject[rank].posStart;
		m_moveObject[rank].posCurrent = m_moveObject[rank].posStart;
		m_moveObject[rank].step = STEP_STOPSTART;
		m_moveObject[rank].time = 0;
		m_moveObject[rank].phase = 0;
		m_moveObject[rank].channel = 10;
		m_moveObject[rank].icon = 0xb1;
	}

	if (index == 6)
	{
		MoveObjectDelete(cel);
		rank = MoveObjectFree();
		if (rank == -1) return;
		m_moveObject[rank].type = TYPE_CHARGE;
		m_moveObject[rank].stepAdvance = 1;
		m_moveObject[rank].stepRecede = 1;
		m_moveObject[rank].timeStopStart = 0;
		m_moveObject[rank].timeStopEnd = 0;
		m_moveObject[rank].posStart.x = cel.x * DIMOBJX;
		m_moveObject[rank].posStart.y = cel.y * DIMOBJY;
		m_moveObject[rank].posEnd = m_moveObject[rank].posStart;
		m_moveObject[rank].posCurrent = m_moveObject[rank].posStart;
		m_moveObject[rank].step = STEP_STOPSTART;
		m_moveObject[rank].time = 0;
		m_moveObject[rank].phase = 0;
		m_moveObject[rank].channel = 1;
		m_moveObject[rank].icon = 0xee;
	}

	if (index >= 7 && index <= 10)
	{
		if (!bMix) return;
		rank = index - 7;
		if (fwKeys & MK_CONTROL) rank = 0;
		pos.x = cel.x * DIMOBJX + 2;
		pos.y = cel.y * DIMOBJY + 6;
		if (pos.x == m_blupiStartPos[rank].x && pos.y == m_blupiStartPos[rank].y)
		{
			if (m_blupiStartDir[rank] == DIR_LEFT) m_blupiStartDir[rank] = DIR_RIGHT;
			else m_blupiStartDir[rank] = DIR_LEFT;
		}
		else m_blupiStartPos[rank] = pos;
		if (fwKeys & MK_CONTROL)
		{
			for (i = 1; i < 4; i++)
			{
				m_blupiStartPos[i] = m_blupiStartPos[0];
				m_blupiStartDir[i] = m_blupiStartDir[0];
			}
		}
	}

	if (index == 11)
	{
		MoveObjectDelete(cel);
		for (i = 0; i < MAXMOVEOBJECT; i++)
		{
			if (m_moveObject[i].type == TYPE_GOAL || m_moveObject[i].type == TYPE_CLE) m_moveObject[i].type = 0;
		}
		rank = MoveObjectFree();
		if (rank == -1) return;
		m_moveObject[rank].type = TYPE_GOAL;
		m_moveObject[rank].stepAdvance = 1;
		m_moveObject[rank].stepRecede = 1;
		m_moveObject[rank].timeStopStart = 0;
		m_moveObject[rank].timeStopEnd = 0;
		m_moveObject[rank].posStart.x = cel.x * DIMOBJX + 2;
		m_moveObject[rank].posStart.y = cel.y * DIMOBJY + 4;
		m_moveObject[rank].posEnd = m_moveObject[rank].posStart;
		m_moveObject[rank].posCurrent = m_moveObject[rank].posStart;
		m_moveObject[rank].step = STEP_STOPSTART;
		m_moveObject[rank].time = 0;
		m_moveObject[rank].phase = 0;
		m_moveObject[rank].channel = 10;
		m_moveObject[rank].icon = 0x1d;
	}
	AdaptBorder(cel);
}

void CDecor::PlaceDecor6(POINT cel, int index, int fwKeys, BOOL bMix)
{
	m_liftEnd = -1;
	int rank, icon, type;
	if (!IsValidCel(cel)) return;

	if (index == 0)
	{
		MoveObjectDelete(cel);
		rank = MoveObjectFree();
		if (rank == -1) return;
		m_moveObject[rank].type = TYPE_BOMBEDOWN;
		m_moveObject[rank].stepAdvance = 1;
		m_moveObject[rank].stepRecede = 1;
		m_moveObject[rank].timeStopStart = 0;
		m_moveObject[rank].timeStopEnd = 0;
		m_moveObject[rank].posStart.x = cel.x * DIMOBJX + 2;
		m_moveObject[rank].posStart.y = cel.y * DIMOBJY + 4;
		m_moveObject[rank].posEnd = m_moveObject[rank].posStart;
		m_moveObject[rank].posCurrent = m_moveObject[rank].posStart;
		m_moveObject[rank].step = STEP_STOPSTART;
		m_moveObject[rank].time = 0;
		m_moveObject[rank].phase = 0;
		m_moveObject[rank].channel = 10;
		m_moveObject[rank].icon = 0xc;
	}

	if (index == 1)
	{
		MoveObjectDelete(cel);
		rank = MoveObjectFree();
		if (rank == -1) return;
		m_moveObject[rank].type = TYPE_BOMBEUP;
		m_moveObject[rank].stepAdvance = 1;
		m_moveObject[rank].stepRecede = 1;
		m_moveObject[rank].timeStopStart = 0;
		m_moveObject[rank].timeStopEnd = 0;
		m_moveObject[rank].posStart.x = cel.x * DIMOBJX + 2;
		m_moveObject[rank].posStart.y = cel.y * DIMOBJY;
		m_moveObject[rank].posEnd = m_moveObject[rank].posStart;
		m_moveObject[rank].posCurrent = m_moveObject[rank].posStart;
		m_moveObject[rank].step = STEP_STOPSTART;
		m_moveObject[rank].time = 0;
		m_moveObject[rank].phase = 0;
		m_moveObject[rank].channel = 10;
		m_moveObject[rank].icon = 0x30;
	}

	if (index == 2)
	{
		icon = m_decor[cel.x][cel.y].icon;
		::PlaceDecor(&icon, table_design_lava, &m_lastDecorIndexes[80], fwKeys, bMix, FALSE);
		m_decor[cel.x][cel.y].icon = (short)icon;
	}

	if (index == 3)
	{
		icon = m_decor[cel.x][cel.y].icon;
		::PlaceDecor(&icon, table_design_marine, &m_lastDecorIndexes[81], fwKeys, bMix, FALSE);
		m_decor[cel.x][cel.y].icon = (short)icon;
	}

	if (index == 4)
	{
		icon = m_decor[cel.x][cel.y].icon;
		::PlaceDecor(&icon, table_design_fan, &m_lastDecorIndexes[82], fwKeys, bMix, FALSE);
		m_decor[cel.x][cel.y].icon = (short)icon;
	}

	if (index == 5)
	{
		icon = m_decor[cel.x][cel.y].icon;
		::PlaceDecor(&icon, table_design_wind, &m_lastDecorIndexes[83], fwKeys, bMix, FALSE);
		m_decor[cel.x][cel.y].icon = (short)icon;
	}
	if (index == 6) m_decor[cel.x][cel.y].icon = 0x8a;
	if (index == 7) m_decor[cel.x][cel.y].icon = 0xd3;

	if (index == 8)
	{
		{
			POINT pos = { cel.x * DIMOBJX + 2, cel.y * DIMOBJY + 6 };
			rank = MoveObjectSearch(pos, -1);
		}
		if (rank != -1 && bMix)
		{
			type = m_moveObject[rank].type + 1;
			if (type > TYPE_BOMBEPERSO4) type = TYPE_BOMBEPERSO1;
			m_lastDecorIndexes[88] = type - TYPE_BOMBEPERSO1;
		}
		type = m_lastDecorIndexes[88] + TYPE_BOMBEPERSO1;
		MoveObjectDelete(cel);
		rank = MoveObjectFree();
		if (rank == -1) return;
		m_moveObject[rank].type = type;
		m_moveObject[rank].stepAdvance = 1;
		m_moveObject[rank].stepRecede = 1;
		m_moveObject[rank].timeStopStart = 0;
		m_moveObject[rank].timeStopEnd = 0;
		m_moveObject[rank].posStart.x = cel.x * DIMOBJX + 2;
		m_moveObject[rank].posStart.y = cel.y * DIMOBJY + 6;
		m_moveObject[rank].posEnd = m_moveObject[rank].posStart;
		m_moveObject[rank].posCurrent = m_moveObject[rank].posStart;
		m_moveObject[rank].step = STEP_STOPSTART;
		m_moveObject[rank].time = 0;
		m_moveObject[rank].phase = 0;
		m_moveObject[rank].channel = 2;
		if (type == TYPE_BOMBEPERSO2) m_moveObject[rank].channel = CHBLUPI1;
		if (type == TYPE_BOMBEPERSO3) m_moveObject[rank].channel = CHBLUPI2;
		if (type == TYPE_BOMBEPERSO4) m_moveObject[rank].channel = CHBLUPI3;
		m_moveObject[rank].icon = 0x101;
	}

	if (index == 9)
	{
		MoveObjectDelete(cel);
		rank = MoveObjectFree();
		if (rank == -1) return;
		m_moveObject[rank].type = TYPE_HELICO;
		m_moveObject[rank].stepAdvance = 1;
		m_moveObject[rank].stepRecede = 1;
		m_moveObject[rank].timeStopStart = 0;
		m_moveObject[rank].timeStopEnd = 0;
		m_moveObject[rank].posStart.x = cel.x * DIMOBJX + 2;
		m_moveObject[rank].posStart.y = cel.y * DIMOBJY + 4;
		m_moveObject[rank].posEnd = m_moveObject[rank].posStart;
		m_moveObject[rank].posCurrent = m_moveObject[rank].posStart;
		m_moveObject[rank].step = STEP_STOPSTART;
		m_moveObject[rank].time = 0;
		m_moveObject[rank].phase = 0;
		m_moveObject[rank].channel = 10;
		m_moveObject[rank].icon = 0x44;
	}

	if (index == 10)
	{
		MoveObjectDelete(cel);
		rank = MoveObjectFree();
		if (rank == -1) return;
		m_moveObject[rank].type = TYPE_SKATE;
		m_moveObject[rank].stepAdvance = 1;
		m_moveObject[rank].stepRecede = 1;
		m_moveObject[rank].timeStopStart = 0;
		m_moveObject[rank].timeStopEnd = 0;
		m_moveObject[rank].posStart.x = cel.x * DIMOBJX + 2;
		m_moveObject[rank].posStart.y = cel.y * DIMOBJY + 5;
		m_moveObject[rank].posEnd = m_moveObject[rank].posStart;
		m_moveObject[rank].posCurrent = m_moveObject[rank].posStart;
		m_moveObject[rank].step = STEP_STOPSTART;
		m_moveObject[rank].time = 0;
		m_moveObject[rank].phase = 0;
		m_moveObject[rank].channel = 10;
		m_moveObject[rank].icon = 0x81;
	}

	if (index == 11)
	{
		MoveObjectDelete(cel);
		rank = MoveObjectFree();
		if (rank == -1) return;
		m_moveObject[rank].type = TYPE_JEEP;
		m_moveObject[rank].stepAdvance = 1;
		m_moveObject[rank].stepRecede = 1;
		m_moveObject[rank].timeStopStart = 0;
		m_moveObject[rank].timeStopEnd = 0;
		m_moveObject[rank].posStart.x = cel.x * DIMOBJX + 2;
		m_moveObject[rank].posStart.y = cel.y * DIMOBJY + 4;
		m_moveObject[rank].posEnd = m_moveObject[rank].posStart;
		m_moveObject[rank].posCurrent = m_moveObject[rank].posStart;
		m_moveObject[rank].step = STEP_STOPSTART;
		m_moveObject[rank].time = 0;
		m_moveObject[rank].phase = 0;
		m_moveObject[rank].channel = 10;
		m_moveObject[rank].icon = 0x59;
	}

	if (index == 12)
	{
		MoveObjectDelete(cel);
		rank = MoveObjectFree();
		if (rank == -1) return;
		m_moveObject[rank].type = TYPE_TANK;
		m_moveObject[rank].stepAdvance = 1;
		m_moveObject[rank].stepRecede = 1;
		m_moveObject[rank].timeStopStart = 0;
		m_moveObject[rank].timeStopEnd = 0;
		m_moveObject[rank].posStart.x = cel.x * DIMOBJX + 2;
		m_moveObject[rank].posStart.y = cel.y * DIMOBJY + 6;
		m_moveObject[rank].posEnd = m_moveObject[rank].posStart;
		m_moveObject[rank].posCurrent = m_moveObject[rank].posStart;
		m_moveObject[rank].step = STEP_STOPSTART;
		m_moveObject[rank].time = 0;
		m_moveObject[rank].phase = 0;
		m_moveObject[rank].channel = 10;
		m_moveObject[rank].icon = 0xa7;
	}
	AdaptBorder(cel);
}

void CDecor::PlaceDecor7(POINT cel, int index, int fwKeys, BOOL bMix)
{
	m_liftEnd = -1;
	int icon, rank, i;
	if (!IsValidCel(cel)) return;

	if (index == 0)
	{
		icon = m_decor[cel.x][cel.y].icon;
		::PlaceDecor(&icon, table_design_key, &m_lastDecorIndexes[100], fwKeys, bMix, FALSE);
		m_decor[cel.x][cel.y].icon = (short)icon;
	}

	if (index == 1)
	{
		icon = m_decor[cel.x][cel.y].icon;
		::PlaceDecor(&icon, table_design_door, &m_lastDecorIndexes[101], fwKeys, bMix, FALSE);
		m_decor[cel.x][cel.y].icon = (short)icon;
	}

	if (index == 2)
	{
		MoveObjectDelete(cel);
		for (i = 0; i < MAXMOVEOBJECT; i++)
		{
			if (m_moveObject[i].type == TYPE_GOAL || m_moveObject[i].type == TYPE_CLE) m_moveObject[i].type = 0;
		}
		rank = MoveObjectFree();
		if (rank == -1) return;
		m_moveObject[rank].type = TYPE_CLE;
		m_moveObject[rank].stepAdvance = 1;
		m_moveObject[rank].stepRecede = 1;
		m_moveObject[rank].timeStopStart = 0;
		m_moveObject[rank].timeStopEnd = 0;
		m_moveObject[rank].posStart.x = cel.x * DIMOBJX + 2;
		m_moveObject[rank].posStart.y = cel.y * DIMOBJY + 4;
		m_moveObject[rank].posEnd = m_moveObject[rank].posStart;
		m_moveObject[rank].posCurrent = m_moveObject[rank].posStart;
		m_moveObject[rank].step = STEP_STOPSTART;
		m_moveObject[rank].time = 0;
		m_moveObject[rank].phase = 0;
		m_moveObject[rank].channel = 10;
		m_moveObject[rank].icon = 0x7d;
	}
	AdaptBorder(cel);
}

void CDecor::PlaceDecor8(POINT cel, int index, int fwKeys, BOOL bMix)
{
	m_liftEnd = -1;

	int *pIcon = (int *)&m_decor[cel.x][cel.y].icon;

	if (!IsValidCel(cel)) return;

	if (index == 0) {} // Key
	if (index == 1) {} // Door
	if (index == 2) {} // Closed while not all chests have been found
	if (index == 3) {} // Teleporter
	if (index == 4) {} // Bar to hang from
	if (index == 5) {} // Spring
	if (index == 6) {} // Vanishing bloc
	if (index == 7) {} // Fragile bridge
	if (index == 8) {} // Wooden case
	if (index == 9) {} // Secret wooden case
}

void CDecor::PlaceDecor9(POINT cel, int index, int fwKeys, BOOL bMix)
{
	m_liftEnd = -1;

	int *pIcon = (int *)&m_decor[cel.x][cel.y].icon;

	if (!IsValidCel(cel)) return;

	if (index == 0) {} // Hovercraft
	if (index == 1) {} // Helicopter
	if (index == 2) {} // Skateboard
	if (index == 3) {} // Jeep
	if (index == 4) {} // Glue tank
}

void CDecor::PlaceDecor10(POINT cel, int index, int fwKeys, BOOL bMix)
{
	m_liftEnd = -1;

	int *pIcon = (int *)&m_decor[cel.x][cel.y].icon;

	if (!IsValidCel(cel)) return;

	if (index == 0) {} // Goal
	if (index == 1) {} // Yellow Blupi
	if (index == 2) {} // Orange Blupi
	if (index == 3) {} // Blue Blupi
	if (index == 4) {} // Green Blupi
	if (index == 5) {} // Door
	if (index == 6) {} // Level start
	if (index == 7) {} // Key
}

char* CDecor::GetMissionTitle()
{
	return m_missionTitle;
}

void CDecor::SetMissionTitle(char *str)
{
	strcpy(m_missionTitle, str);
}


int CDecor::GetRegion() { return m_region; }
void CDecor::SetRegion(int region) { m_region = region; }
int CDecor::GetMusic() { return m_music; }
void CDecor::SetMusic(int music) { m_music = music; }
POINT CDecor::GetDim() { return m_dimDecor; }
void CDecor::SetDim(POINT dim) { m_dimDecor = dim; }
int CDecor::GetNbVies() { return m_nbVies; }
void CDecor::SetNbVies(int nbVies) { m_nbVies = nbVies; }
BOOL CDecor::GetPause() { return m_bPause; }
void CDecor::ChangePause(BOOL bPause) { m_bPause = bPause; }
void CDecor::GetDoors(short *out)
{
    for (int i = 0; i < 100; i++) out[i] = m_doors[i];
}
void CDecor::SetDoors(short *in)
{
    for (int i = 0; i < 100; i++) m_doors[i] = in[i];
}
void CDecor::SetAllMissions(BOOL bAllMissions)
{
    m_bCheatDoors = bAllMissions;
    AdaptDoors(m_bPrivate, m_mission);
}
void CDecor::CheatAction(int cheat)
{
	int i;
	if (cheat == 2)
	{
		for (i = 0; i < MAXMOVEOBJECT; i++)
		{
			if (m_moveObject[i].type == TYPE_BOMBEDOWN || m_moveObject[i].type == TYPE_BOMBEUP ||
				m_moveObject[i].type == TYPE_BULLDOZER || m_moveObject[i].type == TYPE_BOMBEMOVE ||
				m_moveObject[i].type == TYPE_POISSON || m_moveObject[i].type == TYPE_OISEAU ||
				m_moveObject[i].type == TYPE_BLUPIHELICO || m_moveObject[i].type == TYPE_BLUPITANK)
			{
				m_decorAction = 1;
				m_decorPhase = 0;
				m_moveObject[i].type = TYPE_EXPLO1;
				m_moveObject[i].phase = 0;
				m_moveObject[i].posCurrent.x -= 34;
				m_moveObject[i].posCurrent.y -= 34;
				m_moveObject[i].posStart = m_moveObject[i].posCurrent;
				m_moveObject[i].posEnd = m_moveObject[i].posCurrent;
				MoveObjectStepIcon(i);
				PlaySound(SOUND_BOUM, m_moveObject[i].posCurrent, FALSE);
			}
		}
	}
	if (cheat == 6)
	{
		m_blupiAir = FALSE;
		m_blupiHelico = FALSE;
		m_blupiJeep = FALSE;
		m_blupiTank = FALSE;
		m_blupiSkate = TRUE;
		m_blupiNage = FALSE;
		m_blupiSurf = FALSE;
		m_blupiVent = FALSE;
		m_blupiSuspend = FALSE;
		StopVehicleSound();
	}
	if (cheat == 7)
	{
		m_blupiAir = FALSE;
		m_blupiHelico = TRUE;
		m_blupiJeep = FALSE;
		m_blupiTank = FALSE;
		m_blupiSkate = FALSE;
		m_blupiNage = FALSE;
		m_blupiSurf = FALSE;
		m_blupiVent = FALSE;
		m_blupiSuspend = FALSE;
		StopVehicleSound();
		PlaySound(SOUND_HELICOSTART, m_blupiPos, FALSE);
		PlaySound(SOUND_HELICOLOW, m_blupiPos, TRUE);
	}
	if (cheat == 8)
	{
		m_blupiAir = FALSE;
		m_blupiHelico = FALSE;
		m_blupiJeep = TRUE;
		m_blupiTank = FALSE;
		m_blupiSkate = FALSE;
		m_blupiNage = FALSE;
		m_blupiSurf = FALSE;
		m_blupiVent = FALSE;
		m_blupiSuspend = FALSE;
		m_blupiCloud = FALSE;
		m_blupiHide = FALSE;
		StopVehicleSound();
		PlaySound(SOUND_JEEPLOW, m_blupiPos, TRUE);
	}
	if (cheat == 9)
	{
		for (i = 0; i < MAXMOVEOBJECT; i++)
		{
			if (m_moveObject[i].type == TYPE_TRESOR)
			{
				m_moveObject[i].type = 0;
				m_nbTresor++;
				PlaySound(SOUND_TRESOR, m_moveObject[i].posCurrent, FALSE);
			}
		}
	}
	if (cheat == 10)
	{
		for (i = 0; i < MAXMOVEOBJECT; i++)
		{
			if (m_moveObject[i].type == TYPE_GOAL || m_moveObject[i].type == TYPE_CLE)
			{
				m_blupiPos = m_moveObject[i].posCurrent;
				if (m_nbTresor >= m_totalTresor)
		{
					if (m_moveObject[i].type == TYPE_CLE) m_bFoundCle = TRUE;
					StopVehicleSound();
					PlaySound(SOUND_ENDOK, m_moveObject[i].posCurrent, FALSE);
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
				}
		else
		{
					PlaySound(SOUND_ENDKO, m_moveObject[i].posCurrent, FALSE);
				}
				m_goalPhase = 50;
			}
		}
	}
	if (cheat == 12)
	{
		PlaySound(SOUND_STARTSHIELD, m_blupiPos, FALSE);
		m_blupiShield = TRUE;
		m_blupiPower = FALSE;
		m_blupiCloud = FALSE;
		m_blupiHide = FALSE;
		m_blupiTimeShield = 100;
		m_blupiPosMagic = m_blupiPos;
		m_jauges[1].SetHide(FALSE);
	}
	if (cheat == 13)
	{
		m_blupiAction = ACTION_SUCETTE;
		m_blupiPhase = 0;
		m_blupiHelico = FALSE;
		m_blupiJeep = FALSE;
		m_blupiTank = FALSE;
		m_blupiSkate = FALSE;
		m_blupiShield = FALSE;
		m_blupiPower = FALSE;
		m_blupiCloud = FALSE;
		m_blupiHide = FALSE;
		m_blupiFocus = FALSE;
		PlaySound(50, m_blupiPos, FALSE);
	}
	if (cheat == 14)
	{
		m_blupiPerso = 10;
		PlaySound(60, m_blupiPos, FALSE);
	}
	if (cheat == 15)
	{
		m_blupiBullet = 10;
	}
	if (cheat == 16)
	{
		m_blupiAir = FALSE;
		m_blupiHelico = FALSE;
		m_blupiJeep = FALSE;
		m_blupiTank = TRUE;
		m_blupiSkate = FALSE;
		m_blupiNage = FALSE;
		m_blupiSurf = FALSE;
		m_blupiVent = FALSE;
		m_blupiSuspend = FALSE;
		m_blupiCloud = FALSE;
		m_blupiHide = FALSE;
		StopVehicleSound();
		PlaySound(SOUND_JEEPLOW, m_blupiPos, TRUE);
	}
	if (cheat == 17)
	{
		m_blupiAction = ACTION_CHARGE;
		m_blupiPhase = 0;
		m_blupiHelico = FALSE;
		m_blupiJeep = FALSE;
		m_blupiTank = FALSE;
		m_blupiSkate = FALSE;
		m_blupiShield = FALSE;
		m_blupiPower = FALSE;
		m_blupiCloud = FALSE;
		m_blupiHide = FALSE;
		m_blupiJumpAie = FALSE;
		m_blupiFocus = FALSE;
		PlaySound(58, m_blupiPos, FALSE);
	}
	if (cheat == 18)
	{
		m_blupiAction = ACTION_DRINK;
		m_blupiPhase = 0;
		m_blupiHelico = FALSE;
		m_blupiJeep = FALSE;
		m_blupiTank = FALSE;
		m_blupiSkate = FALSE;
		m_blupiShield = FALSE;
		m_blupiPower = FALSE;
		m_blupiCloud = FALSE;
		m_blupiHide = FALSE;
		m_blupiJumpAie = FALSE;
		m_blupiFocus = FALSE;
		PlaySound(57, m_blupiPos, FALSE);
	}
	if (!m_blupiShield && !m_blupiHide && !m_blupiCloud && !m_blupiPower) m_jauges[1].SetHide(TRUE);
	if (!m_blupiHelico)
	{
		StopSound(SOUND_HELICOHIGH);
		StopSound(SOUND_HELICOLOW);
	}
	if (!m_blupiJeep && !m_blupiTank)
	{
		StopSound(SOUND_JEEPHIGH);
		StopSound(SOUND_JEEPLOW);
	}
}

BOOL CDecor::GetSuperBlupi() { return m_bSuperBlupi; }
void CDecor::SetSuperBlupi(BOOL bSuperBlupi) { m_bSuperBlupi = bSuperBlupi; }
BOOL CDecor::GetDrawSecret() { return m_bDrawSecret; }
void CDecor::SetDrawSecret(BOOL bDrawSecret) { m_bDrawSecret = bDrawSecret; }
BOOL CDecor::GetNetPacked() { return m_bNetPacked; }
void CDecor::SetNetPacked(BOOL bNetPacked) { m_bNetPacked = bNetPacked; }
BOOL CDecor::GetNetMovePredict() { return m_bNetMovePredict; }
void CDecor::SetNetMovePredict(BOOL bNetMovePredict) { m_bNetMovePredict = bNetMovePredict; }

BOOL CDecor::GetNetDebug()
{
    return m_bNetDebug;
}
void CDecor::SetNetDebug(BOOL bNetDebug)
{
    m_bNetDebug = bNetDebug;
    if (bNetDebug) remove("debug.txt");
}

void CDecor::OutputNetDebug(char *text)
{
    char textbuffer[100];
    if (!m_bNetDebug) return;
    sprintf(textbuffer, " / snd=%d(%d) rcv=%d(%d)", m_netPacketsSent,
        m_netPacketsSent2, m_netPacketsReceived, m_netPacketsReceived2);
    strcat(text, textbuffer);
}
void CDecor::SetMulti(BOOL multi) { m_bMulti = multi; }
void CDecor::SetTeam(int team) { m_team = team; }

void CDecor::BlupiSearchIcon()

{
	int i = 0;
	int phase;
	int action = m_blupiAction;
	int icon = 2;

	if (m_blupiVent && !m_blupiHelico)
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
	if (action == ACTION_CLEAR1 || action == ACTION_GLU || (action == 57 && icon < 0x10a))
	{
		m_blupiChannel = CHELEMENT;
	}
	else
	{
		m_blupiChannel = 2;
	}
	if (m_blupiDir == DIR_LEFT && m_blupiChannel == 2)
	{
		if (action == ACTION_STOPSUSPEND)
		{
			if (icon == 0x90) icon = 0x9e;
			if (icon == 0x8f) icon = 0x91;
			if (icon == 0x97) icon = 0x92;
		}
		if (icon >= 0 && icon < 0x120) icon = table_mirror[icon];
	}
	if (m_blupiDir == DIR_LEFT && m_blupiChannel == CHELEMENT && icon >= 0xa8 && icon <= 0xab)
	{
		icon += 4;
	}
	m_blupiIcon = icon;
	m_blupiPhase++;
}
