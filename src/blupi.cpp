// blupi.cpp
//

#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <windowsx.h>
#include <WTypes.h>
#include <stdlib.h>
#include <stdio.h>
#include <mmsystem.h>
#include <time.h>
#include <sys/timeb.h>
#include "def.h"
#include "resource.h"
#include "ddutil.h"
#include "pixmap.h"
#include "sound.h"
#include "decor.h"
#include "movie.h"
#include "button.h"
#include "menu.h"
#include "event.h"
#include "network.h"
#include "misc.h"

// Define Globals

#define NAME        "Blupi"
#if _EGAMES
#define TITLE       "Eggbert"
#else
#define TITLE		"Blupi"
#endif

// If FALSE, use the primitive SetTimer method for frame time (like Planet Blupi)
// If TRUE, use the more accurate (yet still deprecated) timeSetEvent method from Windows Multimedia.
// default: TRUE
#define MMTIMER     TRUE

// If TRUE, use an incomplete multithreading model
// I don't actually remember where this originates from. Oops!
// default: FALSE
#define THREAD		FALSE

// Variables Globals

HWND		g_hWnd;					// handle ? la fen?tre
char		g_CDPath[MAX_PATH];		// chemin d'acc?s au CD-Rom
MMRESULT    g_updateTimer;			// timer g?n?ral
CEvent*		g_pEvent = NULL;
CPixmap*	g_pPixmap = NULL;		// pixmap principal
CSound*		g_pSound = NULL;		// sound principal
CNetwork*   g_pNetwork = NULL;
CMovie*		g_pMovie = NULL;		// movie principal
CDecor*		g_pDecor = NULL;
BOOL		g_bFullScreen = FALSE;	// FALSE si mode de test
BOOL 		g_bTrueColorBack = FALSE;
int 		g_benchmark = 0;
int			g_timer = 0;
BOOL		g_bActive = FALSE;		// is application active ?
BOOL		g_bTermInit = FALSE;	// initialisation en cours
int			g_speedRate = 1;
int			g_timerInterval = 50;	// inverval = 50ms
int			g_mouseType = MOUSETYPEGRA;

#if THREAD
HANDLE g_hScreenMutex;
HANDLE g_hRunMutex;
int g_threadNr;
DWORD g_threadID;
HANDLE g_hThread;
#endif // THREAD

UINT		g_lastPhase = 999;

// Lit un numéro décimal.


static void FinishObjects(void);
static BOOL DoInit(HINSTANCE hInstance, LPSTR lpCmdLine, int nCmdShow);

int GetNum(char *p)
{
	int     n = 0;

	while ( *p >= '0' && *p <= '9' )
	{
		n *= 10;
		n += (*p++)-'0';
	}

	return n;
}

BOOL ReadConfig(LPSTR lpCmdLine)
{
	FILE*       file = NULL;
	char        buffer[200];
	char*       pText;
	int         nb;
	int         i;
	file = fopen("data\\config.def", "rb");
	if (file == NULL)   return FALSE;
	nb = fread(buffer, sizeof(char), 200 - 1, file);
	buffer[nb] = 0;
	fclose(file);

#if REQUIRE_CD
	pText = strstr(buffer, "CD-Rom=");
	if (pText == NULL)
	{
#if _DEMO
		GetCurrentDirectory(MAX_PATH, g_CDPath);
		i = strlen(g_CDPath);
		if (i > 0 && g_CDPath[i - 1] != '\\')
		{
			g_CDPath[i++] = '\\';
			g_CDPath[i] = 0;
		}
#else
		return FALSE;
#endif
	}
	else
	{
		pText += 7;
		char c = *pText;
		i = 0;
		if (c != 0)
		{
			nb = (int)g_CDPath - (int)pText;
			while (c != 0)
			{
				if (c == '\n' || *pText == '\r') break;
				*(char*)((int)pText + nb) = *pText;
				c = pText[1];
				i++;
				pText++;
			}
		}
		if (i > 0 && g_CDPath[i - 1] != '\\')
		{
			g_CDPath[i++] = '\\';
		}
		g_CDPath[i] = 0;
	}

#if !_DEMO && !_EGAMES
	if (strstr(lpCmdLine, "-nocd") == NULL)
	{
		char drive[10];
		drive[0] = g_CDPath[0];
		drive[1] = ':';
		drive[2] = '\\';
		drive[3] = 0;
		nb = GetDriveType(drive);
		if (nb != DRIVE_CDROM) return FALSE;
	}
#endif
#endif

	pText = strstr(buffer, "SpeedRate=");
	if (pText != NULL)
	{
		g_speedRate = GetNum(pText + 10);
		if (g_speedRate < 1) g_speedRate = 1;
		if (g_speedRate > 2) g_speedRate = 2;
	}

	pText = strstr(buffer, "Timer=");
	if (pText != NULL)
	{
		g_timerInterval = GetNum(pText + 6);
		if (g_timerInterval < 10) g_timerInterval = 10;
		if (g_timerInterval > 1000) g_timerInterval = 1000;
	}

	pText = strstr(buffer, "FullScreen=");
	if (pText != NULL)
	{
		g_bFullScreen = GetNum(pText + 11);
		if (g_bFullScreen != 0) g_bFullScreen = 1;
	}

	pText = strstr(buffer, "MouseType=");
	if (pText != NULL)
	{
		g_mouseType = GetNum(pText + 10);
		if (g_mouseType < 1) g_mouseType = 1;
		if (g_mouseType > 9) g_mouseType = 9;
	}

	pText = strstr(buffer, "Benchmark=");
	if (pText != NULL)
	{
		g_benchmark = GetNum(pText + 10);
		if (g_benchmark < 0) g_benchmark = 0;
		if (g_benchmark > 100000) g_benchmark = 100000;
		if (g_benchmark >= 3100) g_bTrueColorBack = TRUE;
	}

	pText = strstr(buffer, "TrueColor=");
	if (pText != NULL)
	{
		i = GetNum(pText + 10);
		if (i == 8) g_bTrueColorBack = FALSE;
		if (i == 16) g_bTrueColorBack = TRUE;
	}

	return TRUE;
}

void UpdateFrame(void)
{
	int phase, term, speed;

	g_pEvent->ReadInput();
	phase = g_pEvent->GetPhase();

	if (phase == WM_PHASE_PLAY || phase == WM_PHASE_PLAYTEST || phase == WM_PHASE_BUILD)
	{
		if (!g_pDecor->GetPause())
		{
			speed = g_pEvent->GetSpeed() * g_speedRate;
			for (int i = 0; i < speed; i++)
			{
				g_pDecor->MoveStep(i == 0);
				g_pEvent->DemoStep();
			}
		}
	}

	if (phase == WM_PHASE_INIT)
	{
		g_pEvent->DemoStep();  // démarre év. démo automatique
	}

	if (phase == WM_PHASE_PLAYMOVIE ||
		phase == WM_PHASE_WINMOVIE ||
		phase == WM_PHASE_WINMOVIEDESIGN ||
		phase == WM_PHASE_WINMOVIEMULTI)
	{
		g_pEvent->MovieToStart();
	}

	if (phase == WM_PHASE_INSERT)
	{
		g_pEvent->TryInsert();
	}

	if (phase == WM_PHASE_PLAY)
	{
		term = g_pDecor->IsTerminated();
		
		if (g_pEvent->IsPrivate())
		{
			if (term == -1)
			{
				g_pEvent->ChangePhase(WM_PHASE_LOSTDESIGN);
				return;
			}
			if (term != 0)
			{
				g_pEvent->ChangePhase(WM_PHASE_WINMOVIEDESIGN);
				return;
			}
		}
		else
		{
			if (g_pEvent->IsMulti())
			{
				if (term == -1)
				{
					g_pEvent->ChangePhase(WM_PHASE_LOSTMULTI);
					return;
				}
				if (term != 0)
				{
					g_pEvent->ChangePhase(WM_PHASE_WINMOVIEMULTI);
					return;
				}
			}
			else
			{
				if (term == -1)
				{
					g_pEvent->GetWorldGroup();
					g_pEvent->SetNbVies(g_pDecor->GetNbVies());
					g_pEvent->ChangePhase(WM_PHASE_LOST);
				}
				if (term == -2)
				{
					g_pEvent->SetNbVies(g_pDecor->GetNbVies());
					g_pEvent->ChangePhase(WM_PHASE_WINMOVIE);
				}
				if (term >= 1)
				{
					g_pEvent->SetNbVies(g_pDecor->GetNbVies());
					g_pEvent->SetMission(term);
					g_pEvent->ChangePhase(WM_PHASE_PLAY);
				}
			}
		}
	}
}

void SetDecor()
{
	UINT phase;
	POINT posMouse;

	g_pPixmap->MouseBackClear();
	posMouse = g_pEvent->GetLastMousePos();

	phase = g_pEvent->GetPhase();

	if (phase == WM_PHASE_PLAY || phase == WM_PHASE_PLAYTEST || phase == WM_PHASE_BUILD)
	{
		g_pDecor->Build(RECT(0, 0, LXIMAGE, LYIMAGE), posMouse);
	}
	else {
		g_pPixmap->DrawImage(-1, 0, RECT(0, 0, LXIMAGE, LYIMAGE), 1);
	}

	g_pEvent->DrawButtons();
	g_lastPhase = phase;
	g_pPixmap->MouseBackDraw();
}

BOOL RestoreGame()
{
	if (g_pPixmap == NULL) return FALSE;

	g_pEvent->RestoreGame();
	return g_pPixmap->Restore();
}

BOOL FlushGame()
{
	if (g_pPixmap == NULL) return FALSE;

	return g_pPixmap->Flush();
}

LRESULT CALLBACK WindowProc(HWND hWnd, UINT message,
	WPARAM wParam, LPARAM lParam)
{
	static HINSTANCE   hInstance;
	POINT 			   mousePos, totalDim, iconDim;

#if 0
	if (message != WM_TIMER)
	{
		char s[100];
		sprintf(s, "message=%d,%d\n", message, wParam);
		OutputDebug(s);
	}
#endif

	// La touche F10 envoie un autre message pour activer
	// le menu dans les applications Windows standard !
	if (message == WM_SYSKEYDOWN && wParam == VK_F10)
	{
		message = WM_KEYDOWN;
	}
	if (message == WM_SYSKEYUP && wParam == VK_F10)
	{
		message = WM_KEYUP;
	}

	if (g_pEvent != NULL &&
		g_pEvent->TreatEvent(message, wParam, lParam)) return 0;

	switch (message)
	{
	case WM_TIMER:
	case WM_UPDATE:
		if (g_pEvent != NULL && !g_pEvent->IsMovie() && !g_pEvent->IsMouseRelease())  // pas de film en cours ?
		{
			g_pEvent->GetPhase(); // ?
			if (g_bActive)
			{
				UpdateFrame();
				SetDecor();
			}
			g_pPixmap->Display();
		}
		if (g_timer > 0) g_timer--;
		break;
	case WM_CREATE:
		hInstance = ((LPCREATESTRUCT)lParam)->hInstance;
		return 0;
		break;
	case WM_ACTIVATEAPP:
		g_bActive = (wParam != 0);
		if (g_pEvent != NULL)
		{
			g_pEvent->FlushInput();
		}
		if (g_bActive)
		{
			if (g_bFullScreen)
			{
				RestoreGame();
				g_lastPhase = 999;
			}
			if (!g_bFullScreen && g_bTermInit)
			{
				totalDim.x = 256;
				totalDim.y = 96;
				iconDim.x = DIMLITTLEX;
				iconDim.y = DIMLITTLEY;
				g_pPixmap->BackgroundCache(CHLITTLE, "image08\\little.blp", totalDim, iconDim, TRUE);
				g_pPixmap->SetTransparent(CHLITTLE, RGB(0, 0, 255)); // bleu

				g_pPixmap->SavePalette();
				g_pPixmap->InitSysPalette();
			}
			SetWindowText(hWnd, "Blupi");
			if (g_pSound != NULL) g_pSound->RestartMusic();
		}
		else // désactive ?
		{
			if (g_bFullScreen)
			{
				FlushGame();
			}
			SetWindowText(hWnd, "Blupi -- stop");
			if (g_pSound != NULL) g_pSound->SuspendMusic();
		}
		return 0;

	case WM_SYSCOLORCHANGE:
		OutputDebug("Event WM_SYSCOLORCHANGE\n");
		break;

	case WM_QUERYNEWPALETTE:
		OutputDebug("Event WM_QUERYNEWPALETTE\n");
		break;

	case WM_PALETTECHANGED:
		OutputDebug("Event WM_PALETTECHANGED\n");
		break;

	case WM_DISPLAYCHANGE:
		OutputDebug("Event WM_DISPLAYCHANGE\n");
		break;

	case MM_MCINOTIFY:
		OutputDebug("Event MM_MCINOTIFY\n");
		if (g_pEvent->IsMovie()) // film en cours ?
		{
			if (wParam == MCI_NOTIFY_SUCCESSFUL)
			{
				g_pEvent->StopMovie();
			}
		}
		else
		{
			// music over, play it again
			g_pSound->SuspendMusic();
			// if music finished, play it again. Otherwise assume that
			// it was aborted by the user or otherwise
			if (wParam == MCI_NOTIFY_SUCCESSFUL)
			{
				OutputDebug("Event MCI_NOTIFY_SUCCESSFUL\n");
				g_pSound->RestartMusic();
			}
			else
			{
				char s[50];
				sprintf(s, "wParam=%d\n", wParam);
				OutputDebug(s);
			}
		}
		break;

	case WM_SETCURSOR:
		//			ChangeSprite();
		//			SetCursor(NULL); // pas de souris visible !
		return TRUE;

	case WM_LBUTTONDOWN:
		//?			Benchmark();
		GetCursorPos(&mousePos);
		ScreenToClient(hWnd, &mousePos);
		break;

	case WM_RBUTTONDOWN:
		break;

	case WM_MOUSEMOVE:
		break;

	case WM_KEYDOWN:
		switch (wParam)
		{
		case VK_F5:
			g_pEvent->SetSpeed(1);
			break;
		case VK_F6:
			g_pEvent->SetSpeed(2);
			break;
		case VK_F7:
			g_pEvent->SetSpeed(4);
			break;
		case VK_F8:
			g_pEvent->SetSpeed(8);
			break;
		}
		break;

	case WM_DESTROY:
#if MMTIMER
		timeKillEvent((UINT)g_updateTimer);
#else
		KillTimer(g_hWnd, 1);
#endif
		FinishObjects();
		PostQuitMessage(0);
		break;

	}

	return DefWindowProc(hWnd, message, wParam, lParam);
}

static void FinishObjects(void)
{
	if ( g_pMovie != NULL )
	{
		g_pEvent->StopMovie();

		delete g_pMovie;
		g_pMovie = NULL;
	}

	if ( g_pEvent != NULL )
	{
		delete g_pEvent;
		g_pEvent = NULL;
	}

	if ( g_pDecor != NULL )
	{
		delete g_pDecor;
		g_pDecor = NULL;
	}

	if (g_pSound != NULL )
	{
		g_pSound->StopMusic();

		delete g_pSound;
		g_pSound = NULL;
	}

	if (g_pNetwork != NULL )
	{
		delete g_pNetwork;
		g_pNetwork = NULL;
	}

	if ( g_pPixmap != NULL )
	{
		delete g_pPixmap;
		g_pPixmap = NULL;
	}
}

void CALLBACK TimerStep(UINT wTimerID, UINT msg,
	DWORD dwUser, DWORD dw1, DWORD dw2)
{
	if (g_bActive && g_timer == 0)
	{
		g_timer = 1;
		PostMessage(g_hWnd, WM_UPDATE, 0, 0);
	}
}

BOOL InitFail(const char *msg, BOOL bDirectX)
{
	char	buffer[100];

	if ( bDirectX ) strcpy(buffer, "DirectX Init FAILED\n(while ");
	else 			strcpy(buffer, "Error (");
	strcat(buffer, msg);
	strcat(buffer, ")");
	MessageBoxA(g_hWnd, buffer, TITLE, MB_OK);

	FinishObjects();
	DestroyWindow(g_hWnd);
	return FALSE;
}

int Benchmark()
{
	struct timeb tstruct;
	int        j, t1, t2, time;

	ftime(&tstruct);
	t1 = tstruct.millitm;

	j = 10;
	do
	{
		UpdateFrame();
		SetDecor();
		g_pPixmap->Display();
	}
	while (--j != 0);

	ftime(&tstruct);
	t2 = tstruct.millitm;

	if (t1 > t2)  t2 += 1000;
	time = (t2 - t1) / 10;

	FILE* file = NULL;
	char        string[100];
	sprintf(string, "CheckTime = %d\r\n", time);
	file = fopen("data\\time.blp", "wb");
	if (file != NULL)
	{
		fwrite(string, strlen(string), 1, file);
		fclose(file);
	}

	return time;
}

int WINAPI WinMain (HINSTANCE hInstance, HINSTANCE hPrevInstance,
					LPSTR lpCmdLine, int nCmdShow)
{
	MSG		msg;
	LPTIMECALLBACK timeStep;

	if ( !DoInit(hInstance, lpCmdLine, nCmdShow) )
	{
		return FALSE;
	}

	Benchmark();
	
#if MMTIMER
	g_updateTimer = timeSetEvent(g_timerInterval, g_timerInterval / 4, TimerStep, NULL, TIME_PERIODIC);
#else
	SetTimer(g_hWnd, 1, g_timerInterval, NULL);
#endif

#if THREAD
	g_hScreenMutex = CreateMutex(NULL, FALSE, NULL);
	g_hRunMutex = CreateMutex(NULL, TRUE, NULL);
	g_threadNr = 0;

	g_hThread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)ThreadDisplay, 0, DETACHED_PROCESS, &g_threadID);
#endif

	while ( TRUE )
	{
		if ( PeekMessage(&msg, NULL, 0,0, PM_NOREMOVE) )
		{
			if ( !GetMessage(&msg, NULL, 0, 0) )
			{
				return msg.wParam;
			}
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		else
		{
			// make sure we go to sleep if we have nothing else to do
			if ( !g_bActive ) WaitMessage();
		}
	}

	return msg.wParam;
}
static BOOL DoInit(HINSTANCE hInstance, LPSTR lpCmdLine, int nCmdShow)
{
	WNDCLASSA wc;
	BOOL bOK;

	bOK = ReadConfig(lpCmdLine);
	InitHInstance(hInstance);
	wc.style = CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc = WindowProc;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hInstance = hInstance;
	wc.hIcon = LoadIcon(hInstance, "IDR_MAINFRAME");
	wc.hCursor = LoadCursor(hInstance, "IDC_POINTER");
	wc.hbrBackground = GetStockBrush(BLACK_BRUSH);
	wc.lpszMenuName = NAME;
	wc.lpszClassName = NAME;
	RegisterClass(&wc);

	if (g_bFullScreen)
	{
		g_hWnd = CreateWindowEx(8, NAME, TITLE, WS_POPUP,
			0, 0, GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN),
			NULL, NULL, hInstance, NULL);
	}
	else
	{
		int sx, sy;
		RECT windowRect;
		sx = GetSystemMetrics(SM_CXSCREEN);
		sy = GetSystemMetrics(SM_CYSCREEN);
		SetRect(&windowRect, (sx - LXIMAGE) / 2, (sy - LYIMAGE) / 2,
			(sx + LXIMAGE) / 2, (sy + LYIMAGE) / 2);
		AdjustWindowRect(&windowRect, WS_POPUPWINDOW | WS_CAPTION, TRUE);
		windowRect.top += GetSystemMetrics(SM_CYCAPTION);
		g_hWnd = CreateWindowEx(0, NAME, TITLE,
			WS_POPUPWINDOW | WS_CAPTION | WS_VISIBLE,
			(sx - LXIMAGE) / 2, (sy - LYIMAGE) / 2,
			windowRect.right - windowRect.left, windowRect.bottom - windowRect.top,
			HWND_DESKTOP, NULL, hInstance, NULL);
	}
	if (!g_hWnd) return FALSE;
	ShowWindow(g_hWnd, nCmdShow);
	UpdateWindow(g_hWnd);
	SetFocus(g_hWnd);
	ChangeSprite(SPRITE_WAIT);
	if (!bOK) return InitFail("Game not correctly installed", FALSE);

	g_pPixmap = new CPixmap;
	if (g_pPixmap == NULL) return InitFail("New pixmap", TRUE);
	if (!g_pPixmap->Create(g_hWnd, POINT(LXIMAGE, LYIMAGE), g_bFullScreen, g_bTrueColorBack, g_mouseType))
		return InitFail("Create pixmap", TRUE);
	OutputDebug("Image: init\n");
	if (!g_pPixmap->Cache(0, "image08\\init.blp", POINT(LXIMAGE, LYIMAGE), POINT(0, 0), TRUE)) return FALSE;
	OutputDebug("SavePalette");
	g_pPixmap->SavePalette();
	OutputDebug("InitSysPalette");
	g_pPixmap->InitSysPalette();
	g_pPixmap->SetDebug(FALSE);
	g_pPixmap->DrawImage(-1, 0, RECT(0, 0, LXIMAGE, LYIMAGE), 1);
	g_pPixmap->Display();

	if (!g_pPixmap->Cache(1, "image08\\object.blp", POINT(0x400, 0x500), POINT(0x40, 0x40), FALSE))
		return InitFail("Cache object.blp", TRUE);
	g_pPixmap->SetTransparent(1, RGB(0, 0, 255));
	if (!g_pPixmap->Cache(2, "image08\\blupi000.blp", POINT(0x3c0, 0x4b0), POINT(0x3c, 0x3c), FALSE))
		return InitFail("Cache blupi.blp", TRUE);
	g_pPixmap->SetTransparent(2, RGB(0, 0, 255));
	if (!g_pPixmap->Cache(11, "image08\\blupi001.blp", POINT(0x3c0, 0x4b0), POINT(0x3c, 0x3c), FALSE))
		return InitFail("Cache blupi001.blp", TRUE);
	g_pPixmap->SetTransparent(11, RGB(0, 0, 255));
	if (!g_pPixmap->Cache(12, "image08\\blupi002.blp", POINT(0x3c0, 0x4b0), POINT(0x3c, 0x3c), FALSE))
		return InitFail("Cache blupi002.blp", TRUE);
	g_pPixmap->SetTransparent(12, RGB(0, 0, 255));
	if (!g_pPixmap->Cache(13, "image08\\blupi003.blp", POINT(0x3c0, 0x4b0), POINT(0x3c, 0x3c), FALSE))
		return InitFail("Cache blupi003.blp", TRUE);
	g_pPixmap->SetTransparent(13, RGB(0, 0, 255));
	if (!g_pPixmap->Cache(14, "image08\\temp.blp", POINT(0x40, 0x40), POINT(0x40, 0x40), FALSE))
		return InitFail("Cache temp.blp", TRUE);
	g_pPixmap->SetTransparent(14, RGB(0, 0, 255));
	if (!g_pPixmap->Cache(8, "image08\\map.blp", POINT(0xd0, 0xa0), POINT(0xd0, 0xa0), FALSE))
		return InitFail("Cache map.blp", TRUE);
	g_pPixmap->SetTransparent(8, RGB(0, 0, 255));
	if (!g_pPixmap->Cache(10, "image08\\element.blp", POINT(0x3c0, 0x4b0), POINT(0x3c, 0x3c), FALSE))
		return InitFail("Cache element.blp", TRUE);
	g_pPixmap->SetTransparent(10, RGB(0, 0, 255));
	if (!g_pPixmap->Cache(9, "image08\\explo.blp", POINT(0x800, 0x280), POINT(0x80, 0x80), FALSE))
		return InitFail("Cache explo.blp", TRUE);
	g_pPixmap->SetTransparent(9, RGB(0, 0, 255));
	if (!g_pPixmap->Cache(3, "image08\\decor000.blp", POINT(LXIMAGE, LYIMAGE), POINT(0, 0), FALSE)) return FALSE;
	if (!g_pPixmap->Cache(4, "image08\\button00.blp", POINT(0xf0, 0x348), POINT(0x28, 0x28), FALSE))
		return InitFail("Cache button00.blp", TRUE);
	g_pPixmap->SetTransparent(4, RGB(0, 0, 255));
	if (!g_pPixmap->Cache(5, "image08\\jauge.blp", POINT(0x7c, 0x58), POINT(0x7c, 0x16), FALSE))
		return InitFail("Cache jauge.blp", TRUE);
	g_pPixmap->SetTransparent(5, RGB(0, 0, 255));
	if (!g_pPixmap->Cache(6, "image08\\text.blp", POINT(0x100, 0x180), POINT(0x10, 0x10), FALSE))
		return InitFail("Cache text.blp", TRUE);
	g_pPixmap->SetTransparent(6, RGB(0, 0, 255));
	if (!g_pPixmap->Cache(7, "image08\\little.blp", POINT(0x100, 0x60), POINT(0x10, 0x0c), FALSE))
		return InitFail("Cache little.blp", TRUE);
	g_pPixmap->SetTransparent(7, RGB(0, 0, 255));

	g_pSound = new CSound;
	if (g_pSound == NULL) return InitFail("New sound", TRUE);
	g_pSound->Create(g_hWnd);
	g_pSound->CacheAll();
	g_pSound->SetState(TRUE);
	g_pNetwork = new CNetwork;
	if (g_pNetwork == NULL) return InitFail("New network", TRUE);
	g_pMovie = new CMovie;
	if (g_pMovie == NULL) return InitFail("New movie", FALSE);
	g_pMovie->Create();
	g_pDecor = new CDecor;
	if (g_pDecor == NULL) return InitFail("New decor", FALSE);
	g_pDecor->Create(g_hWnd, g_pSound, g_pPixmap, g_pNetwork);
	g_pEvent = new CEvent;
	if (g_pEvent == NULL) return InitFail("New event", FALSE);
	g_pEvent->Create(hInstance, g_hWnd, g_pPixmap, g_pDecor, g_pSound, g_pNetwork, g_pMovie);
	g_pEvent->SetFullScreen(g_bFullScreen);
	g_pEvent->SetMouseType(g_mouseType);
#if _INTRO
	g_pEvent->ChangePhase(WM_PHASE_INTRO1);
#else
	g_pEvent->ChangePhase(WM_PHASE_INIT);
#endif
	g_bTermInit = TRUE;
	return TRUE;
}
