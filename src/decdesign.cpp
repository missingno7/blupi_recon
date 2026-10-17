// DecDesign.cpp
//

#include <def.h>
#include <decor.h>
#include <misc.h>
#include <dectables.h>

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

	int *pIcon = (int *)&m_decor[cel.x][cel.y].icon;
	int *pIconBig = (int *)&m_bigDecor[cel.x][cel.y].icon;

	if (!IsValidCel(cel)) return;

	// Tree
	if (index == 0) PlaceDecor(pIconBig, table_366b0, &m_lastDecorIndexes[0], fwKeys, bMix, TRUE);

	 // Palmtree
	if (index == 1) PlaceDecor(pIconBig, table_366c0, &m_lastDecorIndexes[1], fwKeys, bMix, TRUE);

	// Decorative plant
	if (index == 2) PlaceDecor(pIconBig, table_366d8, &m_lastDecorIndexes[2], fwKeys, bMix, TRUE);

	// Marine plant
	if (index == 3)	*pIcon = Object::Marine_1;

	// House
	if (index == 4) PlaceDecor(pIconBig, table_366e8, &m_lastDecorIndexes[3], fwKeys, bMix, TRUE);

	// Mechanical background
	if (index == 5) PlaceDecor(pIconBig, table_366f8, &m_lastDecorIndexes[4], fwKeys, bMix, TRUE);

	// Kid's stuff background
	if (index == 6) PlaceDecor(pIconBig, table_36710, &m_lastDecorIndexes[5], fwKeys, bMix, TRUE);

	// Green slime 2
	if (index == 7) PlaceDecor(pIconBig, table_36748, &m_lastDecorIndexes[6], fwKeys, bMix, TRUE);

	// Element for Palace
	if (index == 8) PlaceDecor(pIconBig, table_36520, &m_lastDecorIndexes[7], fwKeys, bMix, TRUE);

	// Support
	if (index == 9) PlaceDecor(pIcon, table_36670, &m_lastDecorIndexes[8], fwKeys, bMix, TRUE);

	// Spider's web
	if (index == 10) PlaceDecor(pIcon, table_366a0, &m_lastDecorIndexes[9], fwKeys, bMix, TRUE);
}

void CDecor::PlaceDecor2(POINT cel, int index, int fwKeys, BOOL bMix)
{
	m_liftEnd = -1;

	int *pIcon = (int *)&m_decor[cel.x][cel.y].icon;

	if (!IsValidCel(cel)) return;

	m_iconLift = index;

	// Square tech-blocks
	if (index == 0) PlaceDecor(pIcon, table_36418, &m_lastDecorIndexes[10], fwKeys, bMix, TRUE);
	
	// Special tech-blocks
	if (index == 1) PlaceDecor(pIcon, table_36458, &m_lastDecorIndexes[11], fwKeys, bMix, TRUE);
	
	// Triangular tech-blocks
	if (index == 2) PlaceDecor(pIcon, table_36488, &m_lastDecorIndexes[12], fwKeys, bMix, FALSE);

	// Secret tech-block
	if (index == 3) PlaceDecor(pIcon, table_364a0, &m_lastDecorIndexes[13], fwKeys, bMix, FALSE);

	// Mechanical square
	if (index == 4) PlaceDecor(pIcon, table_364b0, &m_lastDecorIndexes[14], fwKeys, bMix, TRUE);

	// Special mechanical
	if (index == 5) PlaceDecor(pIcon, table_364d0, &m_lastDecorIndexes[15], fwKeys, bMix, FALSE);

	// Secret mechanical
	if (index == 6) PlaceDecor(pIcon, table_364e8, &m_lastDecorIndexes[16], fwKeys, bMix, FALSE);

	// Pipes
	if (index == 7) *pIcon = Object::Pipes_1;

	AdaptBorder(cel);
}

void CDecor::PlaceDecor3(POINT cel, int index, int fwKeys, BOOL bMix)
{
	m_liftEnd = -1;

	int *pIcon = (int *)&m_decor[cel.x][cel.y].icon;

	if (!IsValidCel(cel)) return;

	m_iconLift = index + 20;

	// Square rock
	if (index == 0) PlaceDecor(pIcon, table_36530, &m_lastDecorIndexes[20], fwKeys, bMix, FALSE);

	// Triangular rock
	if (index == 1) PlaceDecor(pIcon, table_36538, &m_lastDecorIndexes[21], fwKeys, bMix, FALSE);

	// Secret rock
	if (index == 2) PlaceDecor(pIcon, table_36548, &m_lastDecorIndexes[22], fwKeys, bMix, FALSE);

	// Cave
	if (index == 3) *pIcon = rand() % 2 ? Object::Cave_1 : Object::Cave_2;

	// Secret cave
	if (index == 4) *pIcon = Object::CaveSecret;

	// Cheese
	if (index == 5) *pIcon = Random(Object::CheeseSquare_1, Object::CheeseSquare_4);

	// Secret cheese
	if (index == 6) *pIcon = Object::CheeseSecret;

	// Green slime 2
	if (index == 7) *pIcon = Object::Slime_1;

	// Grass
	if (index == 8) PlaceDecor(pIcon, table_365d0, &m_lastDecorIndexes[23], fwKeys, bMix, FALSE);

	// Water
	if (index == 9) PlaceDecor(pIcon, table_36630, &m_lastDecorIndexes[24], fwKeys, bMix, FALSE);

	AdaptBorder(cel);
}

void CDecor::PlaceDecor4(POINT cel, int index, int fwKeys, BOOL bMix)
{
	m_liftEnd = -1;

	int *pIcon = (int *)&m_decor[cel.x][cel.y].icon;

	if (!IsValidCel(cel)) return;

	m_iconLift = index + 40;

	// Palace
	if (index == 0)
	{
		if (*pIcon == 0x182 ||
		    *pIcon == 0x183 ||
		    (*pIcon >= 0x188 && *pIcon <= 0x18d))
		{
			*pIcon = 0x18c;
		}
		PlaceDecor(pIcon, table_364f8, &m_lastDecorIndexes[30], fwKeys, bMix, FALSE);
	}

	// Element for Palace
	if (index == 1) PlaceDecor(pIcon, table_36510, &m_lastDecorIndexes[31], fwKeys, bMix, FALSE);

	// Building facade
	if (index == 2) PlaceDecor(pIcon, table_365f0, &m_lastDecorIndexes[32], fwKeys, bMix, TRUE);

	// Roof
	if (index == 3) PlaceDecor(pIcon, table_36610, &m_lastDecorIndexes[33], fwKeys, bMix, TRUE);

	// Bricks
	if (index == 4) *pIcon = 0x105;

	// Furniture
	if (index == 5) PlaceDecor(pIcon, table_365d8, &m_lastDecorIndexes[34], fwKeys, bMix, TRUE);

	// Wooden wall
	if (index == 6) PlaceDecor(pIcon, table_36550, &m_lastDecorIndexes[35], fwKeys, bMix, FALSE);

	// Kid's stuff square
	if (index == 7) PlaceDecor(pIcon, table_36570, &m_lastDecorIndexes[36], fwKeys, bMix, TRUE);

	// Special Kid's stuff
	if (index == 8) PlaceDecor(pIcon, table_365a8, &m_lastDecorIndexes[37], fwKeys, bMix, TRUE);

	// Invisible block
	if (index == 9) *pIcon = 0xd6;

	AdaptBorder(cel);
}

void CDecor::PlaceDecor5(POINT cel, int index, int fwKeys, BOOL bMix)
{
	m_liftEnd = -1;

	int rank;
	MoveObject* pMob;

	int *pIcon = (int *)&m_decor[cel.x][cel.y].icon;

	if (!IsValidCel(cel)) return;

	// Bomb
	if (index == 0)
	{
		MoveObjectDelete(cel);
		rank = MoveObjectFree();
		if (rank == -1) return;
		pMob = &m_moveObject[rank];
		pMob->type = TYPE_BOMBEDOWN;
		pMob->stepAdvance = 1;
		pMob->stepRecede = 1;
		pMob->timeStopStart = 0;
		pMob->timeStopEnd = 0;
		pMob->posStart = POINT(cel.x * DIMOBJX + 2, cel.y * DIMOBJY + 4);
		pMob->posEnd = pMob->posStart;
		pMob->posCurrent = pMob->posStart;
		pMob->step = STEP_STOPSTART;
		pMob->time = 0;
		pMob->phase = 0;
		pMob->channel = CHELEMENT;
		pMob->icon = 0x0c;
	}

	// Hanging bomb
	if (index == 1)
	{
		MoveObjectDelete(cel);
		rank = MoveObjectFree();
		if (rank == -1) return;
		pMob = &m_moveObject[rank];
		pMob->type = TYPE_BOMBEUP;
		pMob->stepAdvance = 1;
		pMob->stepRecede = 1;
		pMob->timeStopStart = 0;
		pMob->timeStopEnd = 0;
		pMob->posStart = POINT(cel.x * DIMOBJX + 2, cel.y * DIMOBJY);
		pMob->posEnd = pMob->posStart;
		pMob->posCurrent = pMob->posStart;
		pMob->step = STEP_STOPSTART;
		pMob->time = 0;
		pMob->phase = 0;
		pMob->channel = CHELEMENT;
		pMob->icon = 0x30;
	}

	// Homing bomb
	if (index == 2)
	{
		MoveObjectDelete(cel);
		rank = MoveObjectFree();
		if (rank == -1) return;
		pMob = &m_moveObject[rank];
		pMob->type = TYPE_BOMBEFOLLOW1;
		pMob->stepAdvance = 1;
		pMob->stepRecede = 1;
		pMob->timeStopStart = 0;
		pMob->timeStopEnd = 0;
		pMob->posStart = POINT(cel.x * DIMOBJX, cel.y * DIMOBJY);
		pMob->posEnd = pMob->posStart;
		pMob->posCurrent = pMob->posStart;
		pMob->step = STEP_STOPSTART;
		pMob->time = 0;
		pMob->phase = 0;
		pMob->channel = CHELEMENT;
		pMob->icon = 0x100;
	}

	// Lava
	if (index == 3) PlaceDecor(pIcon, table_decor_lave, &m_lastDecorIndexes[40], fwKeys, bMix, FALSE);

	// Fan
	if (index == 4) PlaceDecor(pIcon, table_decor_ventillog, &m_lastDecorIndexes[41], fwKeys, bMix, FALSE);

	// Wind
	if (index == 5) PlaceDecor(pIcon, table_decor_ventg, &m_lastDecorIndexes[42], fwKeys, bMix, FALSE);

	// Slime trap (floor)
	if (index == 6) *pIcon = Object::SlimeTrapFloor_1;

	// Slime trap (ceiling)
	if (index == 7) *pIcon = Object::SlimeTrapCeiling_1;

	// Circular saw
	if (index == 8) PlaceDecor(pIcon, table_decor_scie, &m_lastDecorIndexes[43], fwKeys, bMix, FALSE);

	// Inverter
	if (index == 9)
	{
		MoveObjectDelete(cel);
		rank = MoveObjectFree();
		if (rank == -1) return;
		pMob = &m_moveObject[rank];
		pMob->type = TYPE_INVERT;
		pMob->stepAdvance = 1;
		pMob->stepRecede = 1;
		pMob->timeStopStart = 0;
		pMob->timeStopEnd = 0;
		pMob->posStart = POINT(cel.x * DIMOBJX + 2, cel.y * DIMOBJY + 8);
		pMob->posEnd = pMob->posStart;
		pMob->posCurrent = pMob->posStart;
		pMob->step = STEP_STOPSTART;
		pMob->time = 0;
		pMob->phase = 0;
		pMob->channel = CHELEMENT;
		pMob->icon = 0xBB;
	}

	// Lightning
	if (index == 10) *pIcon = Object::LightningMachine;

	// Crusher
	if (index == 11) *pIcon = Object::Crusher_1;

	AdaptBorder(cel);
}

void CDecor::PlaceDecor6(POINT cel, int index, int fwKeys, BOOL bMix)
{
	int* pTable;
	int nbTable;
	int iconLift;
	int rank;
	int distX, distY;
	MoveObject* pMob;

	if (!IsValidCel(cel) || !bMix) return;

	iconLift = m_iconLift;
	pTable = table_368f8;
	
	if (iconLift == 0) pTable = table_36848;
	if (iconLift == 1) pTable = table_36888;
	if (iconLift == 2) pTable = table_368a8;
	if (iconLift == 3) pTable = table_36848;
	if (iconLift == 4) pTable = table_368c0;
	if (iconLift == 5) pTable = table_368e0;
	if (iconLift == 6) pTable = table_368c0;
	if (iconLift == 7) pTable = table_369a0;
	if (iconLift == 20) pTable = table_368f8;
	if (iconLift == 21) pTable = table_368f8;
	if (iconLift == 22) pTable = table_368f8;
	if (iconLift == 23) pTable = table_36980;
	if (iconLift == 24) pTable = table_36980;
	if (iconLift == 25) pTable = table_369c0;
	if (iconLift == 26) pTable = table_369c0;
	if (iconLift == 27) pTable = table_369c8;
	if (iconLift == 28) pTable = table_36968;
	if (iconLift == 40) pTable = table_369b0;
	if (iconLift == 41) pTable = table_369b0;
	if (iconLift == 42) pTable = table_369e8;
	if (iconLift == 43) pTable = table_369e8;
	if (iconLift == 44) pTable = table_36988;
	if (iconLift == 45) pTable = table_369d0;
	if (iconLift == 46) pTable = table_36908;
	if (iconLift == 47) pTable = table_36928;
	if (iconLift == 48) pTable = table_36950;

	nbTable = pTable[0];

	rank = m_liftEnd;
	pMob = &m_moveObject[rank];
	if (rank == -1)
	{
		// Slow lift
		if (index == 0)
		{
			MoveObjectDelete(cel);
			rank = MoveObjectFree();
			if (rank == -1) return;
			if (m_lastDecorIndexes[50] >= nbTable) m_lastDecorIndexes[50] = 0;
			pMob->type = TYPE_ASCENSEUR;
			pMob->stepAdvance = 20;
			pMob->stepRecede = 20;
			pMob->timeStopStart = 20;
			pMob->timeStopEnd = 20;
			pMob->posStart.x = cel.x * DIMOBJX;
			pMob->posStart.y = cel.y * DIMOBJY;
			pMob->posEnd = pMob->posStart;
			pMob->posCurrent = pMob->posStart;
			pMob->step = STEP_STOPSTART;
			pMob->time = 0;
			pMob->phase = 0;
			pMob->channel = CHOBJECT;
			pMob->icon = pTable[m_lastDecorIndexes[50] + 1];
			m_liftEnd = rank;
		}

		// Normal lift
		if (index == 1)
		{
			MoveObjectDelete(cel);
			rank = MoveObjectFree();
			if (rank == -1) return;
			if (m_lastDecorIndexes[50] >= nbTable) m_lastDecorIndexes[50] = 0;
			pMob->type = TYPE_ASCENSEUR;
			pMob->stepAdvance = 10;
			pMob->stepRecede = 10;
			pMob->timeStopStart = 10;
			pMob->timeStopEnd = 10;
			pMob->posStart.x = cel.x * DIMOBJX;
			pMob->posStart.y = cel.y * DIMOBJY;
			pMob->posEnd = pMob->posStart;
			pMob->posCurrent = pMob->posStart;
			pMob->step = STEP_STOPSTART;
			pMob->time = 0;
			pMob->phase = 0;
			pMob->channel = CHOBJECT;
			pMob->icon = pTable[m_lastDecorIndexes[50] + 1];
			m_liftEnd = rank;
		}

		// Fast lift
		if (index == 2)
		{
			MoveObjectDelete(cel);
			rank = MoveObjectFree();
			if (rank == -1) return;
			if (m_lastDecorIndexes[50] >= nbTable) m_lastDecorIndexes[50] = 0;
			pMob->type = TYPE_ASCENSEUR;
			pMob->stepAdvance = 6;
			pMob->stepRecede = 6;
			pMob->timeStopStart = 3;
			pMob->timeStopEnd = 3;
			pMob->posStart.x = cel.x * DIMOBJX;
			pMob->posStart.y = cel.y * DIMOBJY;
			pMob->posEnd = pMob->posStart;
			pMob->posCurrent = pMob->posStart;
			pMob->step = STEP_STOPSTART;
			pMob->time = 0;
			pMob->phase = 0;
			pMob->channel = CHOBJECT;
			pMob->icon = pTable[m_lastDecorIndexes[50] + 1];
			m_liftEnd = rank;
		}

		// Fast/Slow lift
		if (index == 3)
		{
			MoveObjectDelete(cel);
			rank = MoveObjectFree();
			if (rank == -1) return;
			if (m_lastDecorIndexes[50] >= nbTable) m_lastDecorIndexes[50] = 0;
			pMob->type = TYPE_ASCENSEUR;
			pMob->stepAdvance = 5;
			pMob->stepRecede = 22;
			pMob->timeStopStart = 2;
			pMob->timeStopEnd = 2;
			pMob->posStart.x = cel.x * DIMOBJX;
			pMob->posStart.y = cel.y * DIMOBJY;
			pMob->posEnd = pMob->posStart;
			pMob->posCurrent = pMob->posStart;
			pMob->step = STEP_STOPSTART;
			pMob->time = 0;
			pMob->phase = 0;
			pMob->channel = CHOBJECT;
			pMob->icon = pTable[m_lastDecorIndexes[50] + 1];
			m_liftEnd = rank;
		}

		// Lift with conveyor belt
		if (index == 4) {}

		// Bulldozer
		if (index == 5) {}

		// Fish
		if (index == 6) {}

		// Bird
		if (index == 7) {}

		// Wasp
		if (index == 8) {}

		// Slime creature
		if (index == 9) {}

		// Moving bomb
		if (index == 10) {}

		// Heliported ennemy
		if (index == 11) {}

		// Motorized ennemy
		if (index == 12) {}
	}
	else
	{
		if ((pMob->type == TYPE_BLUPIHELICO || pMob->type == TYPE_BLUPITANK) &&
			pMob->posStart.x == cel.x * DIMOBJX && pMob->posStart.y == cel.y * DIMOBJY)
		{
			pMob->channel++;
			if (pMob->channel > CHBLUPI3) pMob->channel = CHBLUPI1;
		}
		if (pMob->posStart.x == cel.x * DIMOBJX && pMob->posStart.y == cel.y * DIMOBJY)
		{
			m_lastDecorIndexes[50]++;
			if (m_lastDecorIndexes[50] >= *pTable) m_lastDecorIndexes[50] = 0;
			return;
		}
		pMob->posEnd.x = cel.x * DIMOBJX;
		pMob->posEnd.y = cel.y * DIMOBJY;
		distX = abs(pMob->posEnd.x - pMob->posStart.x);
		distY = abs(pMob->posEnd.y - pMob->posStart.y);
		if (distY < distX) distY = distX;
		distY /= DIMOBJX;
		pMob->stepAdvance *= distY;
		pMob->stepRecede *= distY;
		AscenseurSynchro(rank);
		m_liftEnd = -1;
	}

}

void CDecor::PlaceDecor7(POINT cel, int index, int fwKeys, BOOL bMix)
{
	m_liftEnd = -1;

	MoveObject* pMob;
	int mobId;

	int *pIcon = (int *)&m_decor[cel.x][cel.y].icon;

	if (!IsValidCel(cel)) return;

	// Chest
	if (index == 0)
	{
		MoveObjectDelete(cel);
		mobId = MoveObjectFree();
		if (mobId == -1) return;
		pMob = &m_moveObject[mobId];
		pMob->type = TYPE_TRESOR;
		pMob->stepAdvance = 1;
		pMob->stepRecede = 1;
		pMob->timeStopStart = 0;
		pMob->timeStopEnd = 0;
		pMob->posStart = POINT(cel.x * DIMOBJX + 2, cel.y * DIMOBJY + 4);
		pMob->posEnd = pMob->posStart;
		pMob->posCurrent = pMob->posStart;
		pMob->step = STEP_STOPSTART;
		pMob->time = 0;
		pMob->phase = 0;
		pMob->channel = CHELEMENT;
		pMob->icon = 0x00;
	}

	// Egg
	if (index == 1)
	{
		MoveObjectDelete(cel);
		mobId = MoveObjectFree();
		if (mobId == -1) return;
		pMob = &m_moveObject[mobId];
		pMob->type = TYPE_EGG;
		pMob->stepAdvance = 1;
		pMob->stepRecede = 1;
		pMob->timeStopStart = 0;
		pMob->timeStopEnd = 0;
		pMob->posStart = POINT(cel.x * DIMOBJX + 2, cel.y * DIMOBJY + 8);
		pMob->posEnd = pMob->posStart;
		pMob->posCurrent = pMob->posStart;
		pMob->step = STEP_STOPSTART;
		pMob->time = 0;
		pMob->phase = 0;
		pMob->channel = CHELEMENT;
		pMob->icon = 0x15;
	}

	// Lollipop
	if (index == 2)
	{
		MoveObjectDelete(cel);
		mobId = MoveObjectFree();
		if (mobId == -1) return;
		pMob = &m_moveObject[mobId];
		pMob->type = TYPE_POWER;
		pMob->stepAdvance = 1;
		pMob->stepRecede = 1;
		pMob->timeStopStart = 0;
		pMob->timeStopEnd = 0;
		pMob->posStart = POINT(cel.x * DIMOBJX + 2, cel.y * DIMOBJY + 8);
		pMob->posEnd = pMob->posStart;
		pMob->posCurrent = pMob->posStart;
		pMob->step = STEP_STOPSTART;
		pMob->time = 0;
		pMob->phase = 0;
		pMob->channel = CHELEMENT;
		pMob->icon = 0x88;
	}

	// Shield
	if (index == 3)
	{
		MoveObjectDelete(cel);
		mobId = MoveObjectFree();
		if (mobId == -1) return;
		pMob = &m_moveObject[mobId];
		pMob->type = TYPE_SHIELD;
		pMob->stepAdvance = 1;
		pMob->stepRecede = 1;
		pMob->timeStopStart = 0;
		pMob->timeStopEnd = 0;
		pMob->posStart = POINT(cel.x * DIMOBJX + 2, cel.y * DIMOBJY + 8);
		pMob->posEnd = pMob->posStart;
		pMob->posCurrent = pMob->posStart;
		pMob->step = STEP_STOPSTART;
		pMob->time = 0;
		pMob->phase = 0;
		pMob->channel = CHELEMENT;
		pMob->icon = 0x90;
	}

	// Invisibility potion
	if (index == 4)
	{
		MoveObjectDelete(cel);
		mobId = MoveObjectFree();
		if (mobId == -1) return;
		pMob = &m_moveObject[mobId];
		pMob->type = TYPE_DRINK;
		pMob->stepAdvance = 1;
		pMob->stepRecede = 1;
		pMob->timeStopStart = 0;
		pMob->timeStopEnd = 0;
		pMob->posStart = POINT(cel.x * DIMOBJX + 2, cel.y * DIMOBJY + 6);
		pMob->posEnd = pMob->posStart;
		pMob->posCurrent = pMob->posStart;
		pMob->step = STEP_STOPSTART;
		pMob->time = 0;
		pMob->phase = 0;
		pMob->channel = CHELEMENT;
		pMob->icon = 0xB2;
	}

	// Glue supply
	if (index == 5)
	{
		MoveObjectDelete(cel);
		mobId = MoveObjectFree();
		if (mobId == -1) return;
		pMob = &m_moveObject[mobId];
		pMob->type = TYPE_BULLET;
		pMob->stepAdvance = 1;
		pMob->stepRecede = 1;
		pMob->timeStopStart = 0;
		pMob->timeStopEnd = 0;
		pMob->posStart = POINT(cel.x * DIMOBJX + 2, cel.y * DIMOBJY + 4);
		pMob->posEnd = pMob->posStart;
		pMob->posCurrent = pMob->posStart;
		pMob->step = STEP_STOPSTART;
		pMob->time = 0;
		pMob->phase = 0;
		pMob->channel = CHELEMENT;
		pMob->icon = 0xB1;
	}

	// Recharging device
	if (index == 6)
	{
		MoveObjectDelete(cel);
		mobId = MoveObjectFree();
		if (mobId == -1) return;
		pMob = &m_moveObject[mobId];
		pMob->type = TYPE_CHARGE;
		pMob->stepAdvance = 1;
		pMob->stepRecede = 1;
		pMob->timeStopStart = 0;
		pMob->timeStopEnd = 0;
		pMob->posStart = POINT(cel.x * DIMOBJX + 2, cel.y * DIMOBJY + 4);
		pMob->posEnd = pMob->posStart;
		pMob->posCurrent = pMob->posStart;
		pMob->step = STEP_STOPSTART;
		pMob->time = 0;
		pMob->phase = 0;
		pMob->channel = CHELEMENT;
		pMob->icon = 0xEE;
	}

	// Personal bomb
	if (index == 7)
	{
		int type;
		mobId = MoveObjectSearch(cel, -1);
		if (mobId != -1 && bMix != 0) {
			if (mobId >= TYPE_BOMBEPERSO4) {
				m_lastDecorIndexes[60] = 0;
			}
			else {
				m_lastDecorIndexes[60] = m_moveObject[mobId].type - TYPE_BOMBEPERSO1;
			}
		}
		type = m_lastDecorIndexes[60] + TYPE_BOMBEPERSO1;
		MoveObjectDelete(cel);
		mobId = MoveObjectFree();
		if (mobId == -1) return;
		pMob = &m_moveObject[mobId];
		pMob->type = type;
		pMob->stepAdvance = 1;
		pMob->stepRecede = 1;
		pMob->timeStopStart = 0;
		pMob->timeStopEnd = 0;
		pMob->posStart = POINT(cel.x * DIMOBJX + 2, cel.y * DIMOBJY + 6);
		pMob->posEnd = pMob->posStart;
		pMob->posCurrent = pMob->posStart;
		pMob->step = STEP_STOPSTART;
		pMob->time = 0;
		pMob->phase = 0;
		switch (type) {
		default:
		case TYPE_BOMBEPERSO1:
			pMob->channel = CHBLUPI;
			break;
		case TYPE_BOMBEPERSO2:
			pMob->channel = CHBLUPI1;
			break;
		case TYPE_BOMBEPERSO3:
			pMob->channel = CHBLUPI2;
			break;
		case TYPE_BOMBEPERSO4:
			pMob->channel = CHBLUPI3;
			break;
		}
		pMob->icon = 0x101;
	}

	// Dynamite
	if (index == 8)
	{
		MoveObjectDelete(cel);
		mobId = MoveObjectFree();
		if (mobId == -1) return;
		pMob = &m_moveObject[mobId];
		pMob->type = TYPE_DYNAMITE;
		pMob->stepAdvance = 1;
		pMob->stepRecede = 1;
		pMob->timeStopStart = 0;
		pMob->timeStopEnd = 0;
		pMob->posStart = POINT( cel.x * DIMOBJX + 2, cel.y * DIMOBJY + 6 );
		pMob->posEnd = pMob->posStart;
		pMob->posCurrent = pMob->posStart;
		pMob->step = STEP_STOPSTART;
		pMob->time = 0;
		pMob->phase = 0;
		pMob->channel = CHELEMENT;
		pMob->icon = 0xFC;
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
