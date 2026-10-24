// Decor.h
#ifndef DECOR_H
#define DECOR_H

#include "DEF.H"
#include "jauge.h"
#include "sound.h"
#include "pixmap.h"
#include "network.h"

/////////////////////////////////////////////////////////////////////////////

#define MAXQUART		237
#define SCROLL_SPEED	8
#define SCROLL_MARGX	80
#define SCROLL_MARGY	40

#define BLUPIFLOOR		2
#define BLUPIOFFY		4 + BLUPIFLOOR
#define BLUPISURF		12
#define BLUPISUSPEND	12
#define OVERHEIGHT		80

#define TEXTDELAY	   10 // tooltip popup delay
#define NOTIFDELAY		200

typedef struct
{
	short icon;
}
Cellule;

typedef struct
{
	short   type;
	short   stepAdvance;
	short   stepRecede;
	short   timeStopStart;
	short	timeStopEnd;
	short	unknown1;
	POINT	posStart;
	POINT	posEnd;
	POINT	posCurrent;
	short	step;
	short	time;
	short	phase;
	short	channel;
	short	icon;
	short	unknown2;
}
MoveObject;

typedef struct
{
	char type;
	char data1;
	union {
		short data2;
		short posX;
	};
	union {
		short data3;
		short posY;
	};
	union {
		short data4;
		short moveObjectType;
		short soundChannel;
		short ascenseurRank;
		short cloudTeam;
		short decorIcon;
	};
}
NetMessage;

typedef struct
{
	char size;
	char type;
	union {
		short data1;
		short bPause;
		short nbVies;
		short keyPress;
	};
	int time;
	short blupiPosX;
	short blupiPosY;
	short blupiIcon;
	short blupiSec;
	char blupiChannel;
	char blupiTransport;
	char flag;
	char nbMessages;
	NetMessage messages[MAXMESSAGEPERPACKET];
}
NetPacket;

typedef struct
{
	short majRev;
	short minRev;
	int reserve1[50];
	POINT posDecor;
	POINT dimDecor;
	short world;
	short music;
	short region;
	short reserve2[51];
	POINT blupiPos[4];
	int blupiDir[4];
	char name[100];
	short reserve3[196];
}
DescFile;

typedef struct
{
	int lg;
	int majRev;
	int minRev;

	int reseve1[50];

	short decor[MAXCELX][MAXCELY];
	short bigDecor[MAXCELX][MAXCELY];
	MoveObject moveObject[MAXMOVEOBJECT];

	short reseve2[100];

	POINT posDecor;
	POINT dimDecor;
	int phase;
	int term;
	int music;
	int region;
	int time;
	char missionTitle[100];

	short reseve3[100];

	int nbRankCaisse;
	int rankCaisse[MAXMOVEOBJECT];
	int nbLinkCaisse;
	int linkCaisse[MAXMOVEOBJECT];

	short reseve4[100];

	POINT blupiPos;
	POINT blupiValidPos;
	int blupiAction;
	int blupiDir;
	int blupiPhase;
	POINT blupiVitesse;
	int blupiIcon;
	int blupiSec;
	int blupiChannel;
	POINT blupiVector;
	int blupiTransport;
	BOOL blupiFocus;
	BOOL blupiAir;
	BOOL blupiHelico;
	BOOL blupiJeep;
	BOOL blupiTank;
	BOOL blupiSkate;
	BOOL blupiNage;
	BOOL blupiSurf;
	BOOL blupiVent;
	BOOL blupiSuspend;
	BOOL blupiJumpAie;
	BOOL blupiShield;
	BOOL blupiPower;
	BOOL blupiCloud;
	BOOL blupiHide;
	POINT blupiPosHelico;
	POINT blupiPosMagic;
	BOOL blupiRestart;
	BOOL blupiFront;
	int blupiBullet;
	int blupiPerso;
	int blupiNoBarre;
	int blupiTimeShield;
	int blupiTimeFire;
	int blupiTimeOuf;
	int blupiActionOuf;
	int blupiFifoNb;
	POINT blupiFifoPos[MAXFIFOPOS];

	short reseve5[100];

	POINT blupiStartPos[MAXTEAM];
	int blupiStartDir[MAXTEAM];

	short reseve6[100];

	BOOL jaugeHide[2];
	BOOL jaugeType[2];
	BOOL jaugeLevel[2];
	int blupiLevel;
	int energyUnused;

	short reseve7[100];

	BOOL playHelicoHigh;
	BOOL playHelicoLow;
	BOOL playJeepHigh;
	BOOL playJeepLow;

	short reseve8[100];

	BOOL bFoundCle;
	BOOL bPrivate;
	BOOL bCheatDoors;
	BOOL bSuperBlupi;
	BOOL bDrawSecret;
	BOOL bJoystick;
	BOOL bDemoPlay;
	int mission;
	short doors[100];
	int nbVies;
	int nbTresor;
	int totalTresor;
	int goalPhase;

	short reseve9[100];

	BOOL scrollFollow;
	POINT scrollPoint;
	POINT scrollAdd;

	short reseve10[100];

	int voyageIcon;
	int voyageChannel;
	int voyagePhase;
	int voyageTotal;
	POINT voyageStart;
	POINT voyageEnd;

	short reseve11[100];

	int decorAction;
	int decorPhase;

	short reseve12[100];
}
DescGameFile;

class CDecor
{
public:
	CDecor();
	~CDecor();

	// Decor.cpp
	void	Create(HWND hWnd, CSound *pSound, CPixmap *pPixmap,
		CNetwork *pNetwork);
	BOOL	LoadImages();
	void	InitGamer();
	void	InitDecor();
	void	SetTime(int time);
	int	    GetTime();
	void	PlayPrepare(BOOL bTest);
	void	BuildPrepare();
	int		IsTerminated();
	void	MoveStep();
	void	MoveStep(BOOL bFirst);
	int		GetBlupiChannelStandard();
	int		GetBlupiChannelActual();
	int		GetIconPerso();
	void	Build(RECT rect);
	void	Build(RECT rect, POINT mousePos);
	void	DrawInfo();
	POINT	DecorNextAction();
	void	KeyChange(int keys);
	void	SetJoystickEnable(BOOL bJoystick);
	void	SetDemoPlay(BOOL param_1);
	void	PlaySound(int sound, POINT pos, BOOL bLocal);
	void	StopSound(int sound);
	void	AdaptMotorVehicleSound(POINT pos);
	void	ChangePhase(int phase);

	int		GetRegion();
	void	SetRegion(int region);
	int		GetMusic();
	void	SetMusic(int music);
	POINT	GetDim();
	void	SetDim(POINT dim);
	int		GetNbVies();
	void	SetNbVies(int nbVies);
	BOOL	GetPause();
	void	ChangePause(BOOL bPause);
	void	GetDoors(short *doors);
	void	SetDoors(short* doors);
	void	SetAllMissions(BOOL bAllMissions);
	void	CheatAction(int cheat);
	BOOL	GetSuperBlupi();
	void	SetSuperBlupi(BOOL bSuperBlupi);
	BOOL	GetDrawSecret();
	void	SetDrawSecret(BOOL bDrawSecret);
	BOOL	GetNetPacked();
	void	SetNetPacked(BOOL bNetPacked);
	BOOL	GetNetMovePredict();
	void	SetNetMovePredict(BOOL bNetMovePredict);
	BOOL	GetNetDebug();
	void	SetNetDebug(BOOL bNetDebug);
	void	OutputNetDebug(char* text);
	void	SetMulti(BOOL bMulti);
	void	SetTeam(int team);
	POINT	VoyageGetPosVie(int nbVies);
	void	VoyageInit(POINT start, POINT end, int icon, int channel);
	void	VoyageStep();
	void	VoyageDraw();
	BOOL	DrawMap(BOOL bPlay, int team);

	// DecBlock.cpp
	int		IsWorld(POINT pos);
	int GetTypeBarre(POINT pos);
	BOOL	IsLave(POINT pos);
	BOOL	IsRessort(POINT pos);
	BOOL	IsSurfWater(POINT pos);
	BOOL	IsDeepWater(POINT pos);
	BOOL	IsOutWater(POINT pos);
	BOOL	IsPassIcon(int icon);
	BOOL	IsBlocIcon(int icon);
	BOOL	IsVentillo(POINT pos);
	void	ModifDecor(POINT pos, int icon, BOOL bMulti=TRUE);
	BOOL	IsRightBorder(int x, int y, int dx, int dy);
	void	AdaptMidBorder(int x, int y);
	void	AdaptBorder(POINT cel);

	// DecDesign.cpp
	POINT	ScreenPosToCelPos(POINT pos);
	void	SetCelHiliFromScreenPos(POINT pos);
	void	SetIconLiftAndStuff(int _foo, int _bar);
	void	DeleteCel(POINT cel);
	void	PlaceDecor(int *pCel, int *pTable, int *pLastIndex, int fwKeys, BOOL bMix, BOOL bRand);
	void	PlaceDecor1(POINT cel, int index, int fwKeys, BOOL bMix);
	void	PlaceDecor2(POINT cel, int index, int fwKeys, BOOL bMix);
	void	PlaceDecor3(POINT cel, int index, int fwKeys, BOOL bMix);
	void	PlaceDecor4(POINT cel, int index, int fwKeys, BOOL bMix);
	void	PlaceDecor5(POINT cel, int index, int fwKeys, BOOL bMix);
	void	PlaceDecor6(POINT cel, int index, int fwKeys, BOOL bMix);
	void	PlaceDecor7(POINT cel, int index, int fwKeys, BOOL bMix);
	void	PlaceDecor8(POINT cel, int index, int fwKeys, BOOL bMix);
	void	PlaceDecor9(POINT cel, int index, int fwKeys, BOOL bMix);
	void	PlaceDecor10(POINT cel, int index, int fwKeys, BOOL bMix);
	char*	GetMissionTitle();
	void	SetMissionTitle(char *str);

	// DecBlupi.cpp
	void	BlupiSearchIcon();
	RECT	BlupiRect(POINT pos);
	void	BlupiStep();
	void	BlupiDead(int action, int action2);
	POINT	GetPosDecor(POINT pos);
	void	BlupiAddFifo(POINT pos);
	BOOL	DecorDetect(RECT rect, BOOL bCaisse=TRUE);
	void	GetBlupiInfo(BOOL *pbHelico, BOOL *pbJeep, BOOL *pbSkate,
		BOOL *pbNage);

	// DecMove.cpp
	BOOL	TestPath(RECT rect, POINT start, POINT *end);
	void	MoveObjectPollution();
	void	MoveObjectPlouf(POINT pos);
	void	MoveObjectTiplouf(POINT pos);
	void	MoveObjectBlup(POINT pos);
	void	FlushBalleTraj();
	void	SetBalleTraj(POINT pos);
	BOOL	IsBalleTraj(POINT pos);
	void	FlushMoveTraj();
	void	SetMoveTraj(POINT pos);
	BOOL	IsMoveTraj(POINT pos);
	int		SearchDistRight(POINT pos, POINT dir, int type);
	void	StartSploutchGlu(POINT pos);
	BOOL	ObjectStart(POINT pos, int type, int speed);
	BOOL	ObjectStart(POINT pos, int type, int speed, BOOL bMulti);
	BOOL	ObjectDelete(POINT pos, int type, BOOL bMulti = TRUE);
	void	MoveObjectStep();
	void	MoveObjectStepLine(int i);
	void	MoveObjectStepIcon(int i);
	void	DynamiteStart(int i, int dx, int dy);
	int		AscenseurDetect(RECT rect, POINT oldpos, POINT newpos);
	void	AscenseurVertigo(int i, BOOL *pbVertigoLeft, BOOL *pbVertigoRight);
	BOOL	AscenseurShift(int i);
	void	AscenseurSynchro(int i);
	void	UpdateCaisse();
	BOOL	TestPushCaisse(int i, POINT pos, BOOL bPop);
	BOOL	TestPushOneCaisse(int i, POINT move, int b);
	void	SearchLinkCaisse(int rank, BOOL bPop);
	BOOL	AddLinkCaisse(int rank);
	int		CaisseInFront();
	int		CaisseGetMove(int max);
	BOOL	MockeryDetect(POINT pos);
	BOOL	BlupiElectro(POINT pos);
	int		MoveObjectDetect(POINT pos, BOOL *pbNear);
	int		MoveAscenseurDetect(POINT pos, int height);
	int		MoveChargeDetect(POINT pos);
	int		MovePersoDetect(POINT pos);
	int		MoveBalleDetect(POINT pos);
	void		MoveObjectDelete(POINT pos);
	int		MoveObjectFree();
	void	MoveObjectSort();
	void	MoveObjectPriority(int i);
	int		MoveObjectSearch(POINT pos, int type);

	// DecNet.cpp
	void	NetStopCloud(int rank);
	void	NetPlaySound(short channel, POINT pos);
	void	NetStopSound(short channel);
	void	NetDataFlush();
	void	NetSendBarePacket(BYTE _foo, int _bar);
	void	TreatNetData();
	void	DoNetSmooth(int player);
	void	NetAdjustToLift();
	void	NetAscenseurSynchro(int rank, short step);
	void	NetPlayerCollide(POINT pos, int *out);
	void	NetMessageIndexFlush();
	BOOL	NetMessagePush(NetMessage *message);
	BOOL	NetMessagePop(NetMessage *message);
	void	NotifFlush();
	void	NotifPop();
	void	NotifPush(char *str);
	void	NotifStep();

	// DecIO.cpp
	void	GetMissionPath(char *out, int gamer, int mission, BOOL bUser);
	BOOL	Write(int gamer, int mission, BOOL bUser);
	BOOL	Read(int gamer, int mission, BOOL bUser);
	BOOL	CurrentWrite(int gamer, int mission);
	BOOL	CurrentWrite(int gamer, int mission, BOOL bUser);
	BOOL	CurrentRead(int gamer, int mission, int *pMission, BOOL *pbPrivate);
	BOOL	FileExist(int gamer, int mission, char* param3);

	BOOL	SearchWorld(int world, POINT *cel, POINT *newBlupiPos);
	BOOL	SearchDoor(int n, POINT *cel);
	void	AdaptDoors(BOOL bPrivate, int mission);
	void	OpenDoorsWin();
	void	OpenGoldsWin();
	void	DoorsLost();

	BOOL DeleteMission(int user, int mission, BOOL bUser);

	inline BOOL IsValidCel(POINT cel);
	inline void MoveObjectCopy(MoveObject *src, MoveObject *dest);
	inline void StopVehicleSound();
	inline BOOL IsDeadAction(int action);
	inline void StopBlupi(BOOL bFocus = FALSE);
	inline BOOL IsBlupiVehicle();
	inline BOOL IsBlupiMotorVehicle();
	inline BOOL IsBlupiStanding();

protected:
	HWND		m_hWnd;
	CSound*		m_pSound;
	CPixmap*	m_pPixmap;
	CNetwork*	m_pNetwork;
	Cellule		m_decor[MAXCELX][MAXCELY];
	Cellule		m_bigDecor[MAXCELX][MAXCELY];
	MoveObject  m_moveObject[MAXMOVEOBJECT];
	int         m_keyPress;
	int         m_lastKeyPress;
	POINT       m_posDecor;
	POINT       m_dimDecor;
	POINT       m_posCelHili;
	POINT		m_dimCelHili;
	int			m_liftEnd;
	int		    m_phase;
	int         m_term;
	int			m_music;
	int			m_region;
	int			m_lastRegion;
	int			m_iconLift;
	int			m_time;
	char		m_missionTitle[100];
	BOOL		m_bPause;
	int			m_nbRankCaisse;
	int			m_rankCaisse[MAXMOVEOBJECT];
	int			m_nbLinkCaisse;
	int			m_linkCaisse[MAXMOVEOBJECT];
	POINT       m_blupiPos;
	POINT       m_blupiValidPos;
	int			m_blupiAction;
	int         m_blupiDir;
	int         m_blupiPhase;
	POINT       m_blupiVitesse;
	int			m_blupiIcon;
	int			m_blupiSec;
	int			m_blupiChannel;
	POINT		m_blupiVector;
	int			m_blupiTransport;
	BOOL		m_blupiFocus;
	BOOL        m_blupiAir;
	BOOL        m_blupiHelico;
	BOOL        m_blupiJeep;
	BOOL        m_blupiTank;
	BOOL        m_blupiSkate;
	BOOL        m_blupiNage;
	BOOL        m_blupiSurf;
	BOOL        m_blupiVent;
	BOOL        m_blupiSuspend;
	BOOL        m_blupiJumpAie;
	BOOL        m_blupiShield;
	BOOL        m_blupiPower;
	BOOL        m_blupiCloud;
	BOOL        m_blupiHide;
	POINT		m_blupiPosHelico;
	POINT		m_blupiPosMagic;
	BOOL		m_blupiRestart;
	BOOL		m_blupiFront;
	int			m_blupiBullet;
	int			m_blupiPerso;
	int			m_blupiNoBarre;
	int			m_blupiTimeShield;
	int			m_blupiTimeFire;
	int			m_blupiTimeMockery;
	int			m_blupiTimeOuf;
	int			m_blupiActionOuf;
	int			m_blupiFifoNb;
	POINT		m_blupiFifoPos[10];
	POINT		m_blupiStartPos[4];
	int			m_blupiStartDir[4];
	BOOL		m_bMulti;
	int			m_team;
	int			m_netPacketsSent;
	int			m_netPacketsSent2;
	int			m_netPacketsReceived;
	int			m_netPacketsReceived2;
	POINT		m_netBlupiPos[MAXNETPLAYER];
	int			m_netBlupiIcon[MAXNETPLAYER];
	int			m_netBlupiSec[MAXNETPLAYER];
	int			m_netBlupiTransport[MAXNETPLAYER];
	int			m_netTransports[MAXNETPLAYER]; // index of lift ridden by each player, or -1
	int			m_netTimes[MAXNETPLAYER];
	int			m_netPrevTimes[MAXNETPLAYER];
	int			m_netTimeSincePacket[MAXNETPLAYER];
	POINT		m_netVitesses[MAXNETPLAYER];
	POINT		m_netBlupiPrevPos[MAXNETPLAYER];
	POINT		m_netPacketPos;
	int			m_netPacketIcon;
	NetMessage	m_netMessages[MAXNETMESSAGE];
	int			m_netMessageIndex1;
	int			m_netMessageIndex2;
	int			m_netMessageIndex3;
	char		m_notifText[MAXNOTIF][100];
	int			m_notifTime;
	CJauge		m_jauges[2];
	int			m_blupiLevel;
	int			m_blupiEnergyUnused;
	BOOL		m_bHelicoMarch;
	BOOL		m_bHelicoStop;
	BOOL		m_bJeepMarch;
	BOOL		m_bJeepStop;
	BOOL		m_bFoundCle;
	BOOL		m_bPrivate;
	BOOL		m_bCheatDoors;		// opendoors
	BOOL		m_bSuperBlupi;		// megablupi
	BOOL        m_bDrawSecret;  	// showsecret
	BOOL        m_bNetPacked;		// netpacked
	BOOL        m_bNetMovePredict;  // ynosmooth
	BOOL        m_bNetDebug;
	BOOL		m_bJoystick;
	int			m_bDemoPlay;
	int         m_mission;
	short       m_doors[100];
	int         m_nbVies;
	int         m_nbTresor;
	int         m_totalTresor;
	int			m_goalPhase;
	BOOL			m_scrollFollow;
	POINT       m_scrollPoint;
	POINT       m_scrollAdd;
	int         m_voyageIcon;
	int         m_voyageChannel;
	int         m_voyagePhase;
	int         m_voyageTotal;
	POINT       m_voyageStart;
	POINT       m_voyageEnd;
	int 		m_decorAction;
	int 		m_decorPhase;
	int		m_lastDecorIndexes[200];
	// [EVIDENCE] Original storage of unknown layout: `new CDecor` allocates 0x258d4 bytes in the SB1 demo,
	// release and eGames builds, and SB2 has the same 100,044-byte block after this array
	// (docs/LAYOUT_DEBT.md, follow-up 3). No build accesses it. One array represents the size only,
	// not the original member list.
	// [HYPOTHESIS, scratch only] SB2-derived trajectory storage; no SB1 access proves these names/offsets.
	int         m_blupiTimeNoAsc;
	BYTE        m_balleTraj[MAXCELX * MAXCELY / 8 + 1];
	BYTE        m_moveTraj[MAXCELX * MAXCELY / 8 + 1];
	// [EVIDENCE] The complete tail remains the independently proven 100,044 bytes.
	BYTE        m_unknownTail[100044 - sizeof(int) - 2 * (MAXCELX * MAXCELY / 8 + 1)];
};

POINT GetCel(int x, int y);
POINT GetCel(POINT cel, int x, int y);
int SortGetType(int type);

inline BOOL CDecor::IsValidCel(POINT cel)
{
	return cel.x >= 0 && cel.x < MAXCELX && cel.y >= 0 && cel.y < MAXCELY;
}

inline void CDecor::MoveObjectCopy(MoveObject *dest, MoveObject *src)
{
	memcpy(dest, src, sizeof(*dest));
}

inline void CDecor::StopVehicleSound()
{
	StopSound(SOUND_HELICOHIGH);
	StopSound(SOUND_HELICOLOW);
	StopSound(SOUND_JEEPHIGH);
	StopSound(SOUND_JEEPLOW);
}

inline BOOL CDecor::IsDeadAction(int action)
{
	return action == ACTION_CLEAR1 || (action >= ACTION_CLEAR2 && action <= ACTION_CLEAR8);
}

inline void CDecor::StopBlupi(BOOL bSetFocus)
{
	m_blupiAction = ACTION_STOP;
	m_blupiPhase = 0;
	if (bSetFocus) m_blupiFocus = TRUE;
}

inline BOOL CDecor::IsBlupiVehicle()
{
	return IsBlupiMotorVehicle() || m_blupiSkate;
}

inline BOOL CDecor::IsBlupiMotorVehicle()
{
	return m_blupiHelico || m_blupiJeep || m_blupiTank;
}

inline BOOL CDecor::IsBlupiStanding()
{
	return !(m_blupiAction == ACTION_TURN ||
		m_blupiAction == ACTION_JUMP ||
		m_blupiAction == ACTION_AIR ||
		m_blupiAction == ACTION_VERTIGO ||
		m_blupiAction == ACTION_ADVANCE ||
		m_blupiAction == ACTION_RECEDE) &&
		!IsBlupiVehicle() &&
		!(m_blupiAir ||
		m_blupiNage || m_blupiSurf || m_blupiSuspend);		
}

#endif
