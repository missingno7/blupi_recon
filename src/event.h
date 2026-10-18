// Event.h
#ifndef EVENT_H
#define EVENT_H

/////////////////////////////////////////////////////////////////////////////

#include "movie.h"
#include "menu.h"
#include "button.h"
#include "text.h"

class CNetwork;

typedef struct
{
	UINT	message;
	int		type;
	int		iconMenu[20];
	int		x, y;
	int		toolTips[20];
}
Button;

typedef struct
{
	UINT	phase;
	char	backName[32];
	BOOL	bCDrom;
	BOOL	bLocked;
	Button	buttons[MAXBUTTON];
}
Phase;

typedef struct
{
	short	majRev;
	short	minRev;
	short	reserve[100];
}
DemoHeader;

typedef struct
{
	short	time;
	short	input;
}
DemoEvent;



class CEvent
{
public:
	CEvent();
	~CEvent();


	void	OutputNetDebug(const char* str);
	POINT	GetMousePos();
	void	Create(HINSTANCE hInstance, HWND hWnd, CPixmap *pPixmap, CDecor *pDecor, CSound *pSound, CNetwork *pNetwork, CMovie *pMovie );
	void	SetFullScreen(BOOL bFullScreen);
	void	SetMouseType(int mouseType);
	int		GetWorld();
	void	GetWorldGroup();
	void	SetMission(int index);
	BOOL	ChangePhase(UINT phase);
	UINT    GetPhase();
	void	MovieToStart();
	void	ReadInput();
	void	TryInsert();
	//void	SomethingUserMissions(LPCSTR lpFileName, LPCSTR thing);
	static BOOL	CopyMission(char* pMission, char* pFile);
	void	RestoreGame();

	int		GetButtonIndex(int button);
	int		GetState(int button);
	void	SetState(int button, int state);
	BOOL	GetEnable(int button);
	void	SetEnable(int button, BOOL bEnable);
	void	SetDemo(int button, BOOL bDemo);
	void	SetIconMenu(int button, int* pIcons, int nbMenu);
	BOOL	GetHide(int button);
	void	SetHide(int button, BOOL bHide);
	int		GetMenu(int button);
	void	SetMenu(int button, int menu);
	BOOL	IsMouseRelease();
	static void	AddUserExportPath(char* dst, char* pFileName);

	void	ChangePause(BOOL bPause, BOOL bMulti);
	void	NetSendLobby();
	void	DrawMap();

	BOOL	DrawButtons();
	int		MousePosToSprite(POINT pos);
	void	MouseSprite(POINT pos);
	void	WaitMouse(BOOL bWait);
	void	HideMouse(BOOL bHide);
	void	DisableMouse(int bFill);
	POINT	GetLastMousePos();
	BOOL	TreatEvent(UINT message, WPARAM wParam, LPARAM lParam);
	BOOL	TreatEventBase(UINT message, WPARAM wParam, LPARAM lParam);
	
	BOOL	StartMovie(char *pFilename);
	void	StopMovie();
	BOOL	IsMovie();

	void	FlushInput();

	void	SetSpeed(int speed);
	int		GetSpeed();
	BOOL	GetPause();
	void	GetDoors(int doors);
	BOOL    IsMulti();
	BOOL	IsPrivate();

	void	DemoStep();

	void	IntroStep();
	void	SetNbVies(int lives);

	void	ReadAll();
	void	AddUserExportPath(char* lpFilename, LPCSTR fileSomething);

	BOOL	ClearGamer(int gamer);

protected:
	void	DrawTextCenter(int res, int x, int y, int font=0);
	BOOL	CreateButtons();
	BOOL	EventButtons(UINT message, WPARAM wParam, LPARAM lParam);
	BOOL	MouseOnButton(POINT pos);
	int		SearchPhase(UINT phase);

	void	ChangeButtons(int message);
	
	BOOL	BuildDown(POINT pos, int fwKeys, BOOL bMix=TRUE);
	BOOL	BuildMove(POINT pos, int fwKeys);
	BOOL	BuildUp(POINT pos, int fwKeys);

	BOOL	PlayDown(POINT pos, int fwKeys);
	BOOL	PlayMove(POINT pos, int fwKeys);
	BOOL	PlayUp(POINT pos, int fwKeys);

	BOOL	WriteInfo(int gamer);
	BOOL	ReadInfo(int gamer);
	BOOL	LoadState(BOOL save);
	void	PutTextInputBox(POINT pos);
	

	void	DemoRecStart();
	void	DemoRecStop();
	BOOL	DemoPlayStart();
	void	DemoPlayStop();
	void	DemoRecEvent();

	void	ChatFlush();
	void	ChatPop();
	void	ChatPush(char* str);
	void	ChatSend();
	BOOL	TreatTextInput(UINT message, WPARAM wParam, LPARAM lParam);
	BOOL	TextInputDelete();

	//		Network Functions
	BOOL	NetCreate(int session);
	BOOL	NetEnumSessions();
	int		NetSearchPlayer(DPID dpid);
	void	NetStartPlay();
	void	NetSendBarePacket(int message, short data);
	void	NetDraw();

	void	MouseRelease();
	void	MouseCapture();

	BOOL	CheckWorld1();
	void	NetAdjustLobbyButtons();

	BOOL	CurrentRead(BOOL bPrivate);
	BOOL	CurrentWrite(int rank);


protected:
	int m_joyIndex;
	int m_joyID;
	int m_nbJoy;
	char m_joyNames[10][100];
	int m_joyIDList[10];
	int m_gamer;
	int m_textHiliStart;
	int m_textHiliEnd;
	int m_textCursorIndex;
	char m_textInput[100];
	char m_name[100];
	char m_names[10][100];
	BOOL m_bNamesExist[10];
	char m_filenameBuffer[256][100];
	int m_nbFiles;
	int m_filePageOffset;
	int m_fileIndex;
	int m_saveIndex;
	int m_nbVies;
	int m_speed;
	int m_mission;
	int m_private;
	int m_multi;
	UINT m_phase;
	int m_index;
	int m_6d30;
	BOOL m_bDrawMap;
	BOOL m_bPrivate;
	BOOL m_bMulti;
	BOOL m_bBuildOfficialMissions;
	BOOL m_bFullScreen;
	int m_mouseType;
	HINSTANCE m_hInstance;
	HWND m_hWnd;
	CPixmap* m_pPixmap;
	CDecor* m_pDecor;
	CSound* m_pSound;
	CNetwork* m_pNetwork;
	CMovie* m_pMovie;
	char m_movieToStart[260];
	UINT m_phaseAfterMovie;
	CButton m_buttons[40];
	BOOL m_bRunMovie;
	int m_keyPress;
	CMenu m_menu;
	BOOL m_96b4;
	byte m_96b8[344];
	POINT m_oldMousePos;
	BOOL m_bMousePress;
	BOOL m_bHili;
	int m_fileWorld[10];
	int m_fileTime[10];
	POINT m_posToolTips;
	char m_textToolTips[50];
	char m_textToolTips2[50];
	int m_mouseSprite;
	BOOL m_bWaitMouse;
	BOOL m_bHideMouse;
	BOOL m_bDisableMouse;
	BOOL m_bShowMouse;
	BOOL m_bMouseRelease;
	int m_rankCheat;
	int m_posCheat;
	BOOL m_bMovie;
	BOOL m_9950;
	BOOL m_bAllMissions;
	BOOL m_bHiliInfoButton;
	UINT m_tryPhase;
	int m_tryInsertCount;
	BOOL m_bDemoRec;
	BOOL m_bDemoPlay;
	DemoEvent* m_pDemoBuffer;
	int m_demoTime;
	int m_demoIndex;
	int m_demoEnd;
	int m_demoNumber;
	int m_introTime;
	BOOL m_bInfoHelp;
	char m_chatZone[6][100];
};

#endif