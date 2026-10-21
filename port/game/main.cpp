// Port replacement of src/blupi.cpp (historical: WinMain, WindowProc,
// multimedia-timer loop).
//
// Copied from the historical source and kept as close to it as possible:
// ReadConfig, UpdateFrame, SetDecor, RestoreGame, FlushGame, the WindowProc
// message switch (here WindowProc without an HWND), FinishObjects, Benchmark
// and DoInit's object creation order and image cache list.
//
// Replaced: window class/creation (SDL3 host), the CD-ROM checks (the port
// reads an installed copy of the data, never a CD) and the message pump.
//
// Timing is the historical one: timeSetEvent(g_timerInterval) fired TimerStep,
// which posted a single WM_UPDATE while none was pending (g_timer gate); each
// WM_UPDATE ran UpdateFrame + SetDecor + Display. The port drives TimerStep
// from a monotonic clock on the same fixed grid and drops missed ticks the
// same way, so the game advances at 1000/Timer (default 20) steps per second.

#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include "def.h"
#include "resource.h"
#include "pixmap.h"
#include "sound.h"
#include "decor.h"
#include "movie.h"
#include "button.h"
#include "menu.h"
#include "event.h"
#include "network.h"
#include "misc.h"
#include "port_messages.h"

#include "game/port_game.h"
#include "game/scene.h"
#include "platform/audio.h"
#include "platform/music.h"
#include "platform/host.h"
#include "platform/vfs.h"

#include <new>
#include <string>
#include <vector>

// Define Globals

#define NAME        "Blupi"
#define TITLE		"Blupi"

// Variables Globals

HWND		g_hWnd;					// handle à la fenêtre
char		g_CDPath[MAX_PATH];		// chemin d'accès au CD-Rom
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

UINT		g_lastPhase = 999;

// Port state.
static bool g_bQuit = false;
static int  g_portColorDepth = 0;	// --color: overrides config.def TrueColor

static void FinishObjects(void);

// Port: the game objects are constructed in zero-filled memory. Several
// members are only assigned on first use (voyages, network, helicopter and
// magic positions, CEvent's text fields...). The original allocated these
// large objects (CDecor is 0x258d4 bytes) from the VC5 C runtime, which serves
// such sizes with HeapAlloc early in the process, so they most plausibly
// started as fresh zeroed pages [HYPOTHESIS]. Zero-filling makes the state
// deterministic across heaps, platforms and renderers (port/docs/ROADMAP.md 0.1).
template <class T> static T* NewZeroed()
{
	void* p = calloc(1, sizeof(T));
	return p ? new (p) T : NULL;
}

template <class T> static void DeleteZeroed(T* p)
{
	if (p == NULL) return;
	p->~T();
	free(p);
}
static BOOL DoInit(HINSTANCE hInstance, LPSTR lpCmdLine, int nCmdShow);
static LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam);

// Lit un numéro décimal.

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

// Port: the "CD-Rom=" entry and the -nocd drive check are ignored; the data
// root is a normal installed copy (g_CDPath stays empty, so AddCDPath is a
// no-op exactly as for a hard-disk installation).

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

	g_CDPath[0] = 0;

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

	// Port: command-line override, applied last like a TrueColor= entry.
	if (g_portColorDepth == 8) g_bTrueColorBack = FALSE;
	if (g_portColorDepth == 16) g_bTrueColorBack = TRUE;

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

// Port: CDecor::Build, optionally checked against the scene builder
// (ROADMAP.md stage 3, --check-scene): the scene is built from the state
// before Build runs, then its commands must equal Build's commands.
static bool g_bCheckScene = false;
static long g_sceneFrames = 0, g_sceneMismatches = 0, g_sceneFirstMismatch = -1;
static std::string g_sceneFirstDetail;

static void PortBuildDecor(RECT rect, POINT posMouse)
{
	if (!g_bCheckScene)
	{
		g_pDecor->Build(rect, posMouse);
		return;
	}
	blupi::scene::Scene scene;
	std::vector<blupi::render::Command> historical, flattened;
	blupi::scene::BuildScene(*g_pDecor, rect, posMouse, scene);
	PortPixmapCaptureBegin(true);
	g_pDecor->Build(rect, posMouse);
	PortPixmapCaptureEnd(historical);
	PortPixmapCaptureBegin(false);
	blupi::scene::FlattenScene(scene, *g_pDecor, g_pPixmap);
	PortPixmapCaptureEnd(flattened);

	size_t i = 0;
	while (i < historical.size() && i < flattened.size() && blupi::render::same_command(historical[i], flattened[i])) i++;
	if (i != historical.size() || i != flattened.size())
	{
		if (g_sceneFirstMismatch < 0)
		{
			char text[200];
			g_sceneFirstMismatch = g_sceneFrames;
			snprintf(text, sizeof(text), "Build issued %u commands, scene %u, first difference at command %u",
					 (unsigned)historical.size(), (unsigned)flattened.size(), (unsigned)i);
			g_sceneFirstDetail = text;
		}
		g_sceneMismatches++;
	}
	g_sceneFrames++;
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
		{
			RECT rect = { 0, 0, LXIMAGE, LYIMAGE };
			PortBuildDecor(rect, posMouse);
		}
	}
	else {
		{
			RECT rect = { 0, 0, LXIMAGE, LYIMAGE };
			g_pPixmap->DrawImage(-1, 0, rect, 1);
		}
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

static LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam)
{
	POINT 			   totalDim, iconDim;

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
				g_pPixmap->Cache(CHLITTLE, "image08\\little.blp", totalDim, iconDim, TRUE);
				g_pPixmap->SetTransparent(CHLITTLE, RGB(0, 0, 255)); // bleu

				g_pPixmap->SavePalette();
				g_pPixmap->InitSysPalette();
			}
			blupi::host::set_title("Blupi");
			if (g_pSound != NULL) g_pSound->RestartMusic();
		}
		else // désactive ?
		{
			if (g_bFullScreen)
			{
				FlushGame();
			}
			blupi::host::set_title("Blupi -- stop");
			if (g_pSound != NULL) g_pSound->SuspendMusic();
		}
		return 0;

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
				sprintf(s, "wParam=%d\n", (int)wParam);
				OutputDebug(s);
			}
		}
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

	case WM_CLOSE:
		// DefWindowProc: DestroyWindow -> WM_DESTROY.
		FinishObjects();
		g_bQuit = true;
		return 0;
	}

	return 0;
}

static void FinishObjects(void)
{
	if ( g_pMovie != NULL )
	{
		g_pEvent->StopMovie();

		DeleteZeroed(g_pMovie);
		g_pMovie = NULL;
	}

	if ( g_pEvent != NULL )
	{
		DeleteZeroed(g_pEvent);
		g_pEvent = NULL;
	}

	if ( g_pDecor != NULL )
	{
		DeleteZeroed(g_pDecor);
		g_pDecor = NULL;
	}

	if (g_pSound != NULL )
	{
		g_pSound->StopMusic();

		DeleteZeroed(g_pSound);
		g_pSound = NULL;
	}

	if (g_pNetwork != NULL )
	{
		DeleteZeroed(g_pNetwork);
		g_pNetwork = NULL;
	}

	if ( g_pPixmap != NULL )
	{
		DeleteZeroed(g_pPixmap);
		g_pPixmap = NULL;
	}
}

// TimerStep: the multimedia-timer callback.

static void TimerStep()
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
	blupi::host::message_box(TITLE, buffer);

	FinishObjects();
	return FALSE;
}

int Benchmark()
{
	int        j, t1, t2, time;

	t1 = (int)(blupi::host::now_ns() / 1000000);

	j = 10;
	do
	{
		UpdateFrame();
		SetDecor();
		g_pPixmap->Display();
	}
	while (--j != 0);

	t2 = (int)(blupi::host::now_ns() / 1000000);
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

static BOOL DoInit(HINSTANCE hInstance, LPSTR lpCmdLine, int nCmdShow)
{
	BOOL bOK;
	POINT totalDim, iconDim;

	bOK = ReadConfig(lpCmdLine);
	InitHInstance(hInstance);
	ChangeSprite(SPRITE_WAIT);
	if (!bOK) return InitFail("Game not correctly installed", FALSE);

	g_pPixmap = NewZeroed<CPixmap>();
	if (g_pPixmap == NULL) return InitFail("New pixmap", TRUE);
	totalDim.x = LXIMAGE;
	totalDim.y = LYIMAGE;
	if (!g_pPixmap->Create(g_hWnd, totalDim, g_bFullScreen, g_bTrueColorBack, g_mouseType))
		return InitFail("Create pixmap", TRUE);
	OutputDebug("Image: init\n");
	totalDim.x = LXIMAGE;
	totalDim.y = LYIMAGE;
	iconDim.x = 0;
	iconDim.y = 0;
	if (!g_pPixmap->Cache(0, "image08\\init.blp", totalDim, iconDim, TRUE)) return FALSE;
	OutputDebug("SavePalette\n");
	g_pPixmap->SavePalette();
	OutputDebug("InitSysPalette\n");
	g_pPixmap->InitSysPalette();
	g_pPixmap->SetDebug(FALSE);
	{
		RECT rect = { 0, 0, LXIMAGE, LYIMAGE };
		g_pPixmap->DrawImage(-1, 0, rect, 1);
	}
	g_pPixmap->Display();

	totalDim.x = 0x400;
	totalDim.y = 0x500;
	iconDim.x = 0x40;
	iconDim.y = 0x40;
	if (!g_pPixmap->Cache(1, "image08\\object.blp", totalDim, iconDim, FALSE))
		return InitFail("Cache object.blp", TRUE);
	g_pPixmap->SetTransparent(1, RGB(0, 0, 255));
	totalDim.x = 0x3c0;
	totalDim.y = 0x4b0;
	iconDim.x = 0x3c;
	iconDim.y = 0x3c;
	if (!g_pPixmap->Cache(2, "image08\\blupi000.blp", totalDim, iconDim, FALSE))
		return InitFail("Cache blupi.blp", TRUE);
	g_pPixmap->SetTransparent(2, RGB(0, 0, 255));
	totalDim.x = 0x3c0;
	totalDim.y = 0x4b0;
	iconDim.x = 0x3c;
	iconDim.y = 0x3c;
	if (!g_pPixmap->Cache(11, "image08\\blupi001.blp", totalDim, iconDim, FALSE))
		return InitFail("Cache blupi001.blp", TRUE);
	g_pPixmap->SetTransparent(11, RGB(0, 0, 255));
	totalDim.x = 0x3c0;
	totalDim.y = 0x4b0;
	iconDim.x = 0x3c;
	iconDim.y = 0x3c;
	if (!g_pPixmap->Cache(12, "image08\\blupi002.blp", totalDim, iconDim, FALSE))
		return InitFail("Cache blupi002.blp", TRUE);
	g_pPixmap->SetTransparent(12, RGB(0, 0, 255));
	totalDim.x = 0x3c0;
	totalDim.y = 0x4b0;
	iconDim.x = 0x3c;
	iconDim.y = 0x3c;
	if (!g_pPixmap->Cache(13, "image08\\blupi003.blp", totalDim, iconDim, FALSE))
		return InitFail("Cache blupi003.blp", TRUE);
	g_pPixmap->SetTransparent(13, RGB(0, 0, 255));
	totalDim.x = 0x40;
	totalDim.y = 0x40;
	iconDim.x = 0x40;
	iconDim.y = 0x40;
	if (!g_pPixmap->Cache(14, "image08\\temp.blp", totalDim, iconDim, FALSE))
		return InitFail("Cache temp.blp", TRUE);
	g_pPixmap->SetTransparent(14, RGB(0, 0, 255));
	totalDim.x = 0xd0;
	totalDim.y = 0xa0;
	iconDim.x = 0xd0;
	iconDim.y = 0xa0;
	if (!g_pPixmap->Cache(8, "image08\\map.blp", totalDim, iconDim, FALSE))
		return InitFail("Cache map.blp", TRUE);
	g_pPixmap->SetTransparent(8, RGB(0, 0, 255));
	totalDim.x = 0x3c0;
	totalDim.y = 0x4b0;
	iconDim.x = 0x3c;
	iconDim.y = 0x3c;
	if (!g_pPixmap->Cache(10, "image08\\element.blp", totalDim, iconDim, FALSE))
		return InitFail("Cache element.blp", TRUE);
	g_pPixmap->SetTransparent(10, RGB(0, 0, 255));
	totalDim.x = 0x800;
	totalDim.y = 0x280;
	iconDim.x = 0x80;
	iconDim.y = 0x80;
	if (!g_pPixmap->Cache(9, "image08\\explo.blp", totalDim, iconDim, FALSE))
		return InitFail("Cache explo.blp", TRUE);
	g_pPixmap->SetTransparent(9, RGB(0, 0, 255));
	totalDim.x = LXIMAGE;
	totalDim.y = LYIMAGE;
	iconDim.x = 0;
	iconDim.y = 0;
	if (!g_pPixmap->Cache(3, "image08\\decor000.blp", totalDim, iconDim, FALSE)) return FALSE;
	totalDim.x = 0xf0;
	totalDim.y = 0x348;
	iconDim.x = 0x28;
	iconDim.y = 0x28;
	if (!g_pPixmap->Cache(4, "image08\\button00.blp", totalDim, iconDim, FALSE))
		return InitFail("Cache button00.blp", TRUE);
	g_pPixmap->SetTransparent(4, RGB(0, 0, 255));
	totalDim.x = 0x7c;
	totalDim.y = 0x58;
	iconDim.x = 0x7c;
	iconDim.y = 0x16;
	if (!g_pPixmap->Cache(5, "image08\\jauge.blp", totalDim, iconDim, FALSE))
		return InitFail("Cache jauge.blp", TRUE);
	g_pPixmap->SetTransparent(5, RGB(0, 0, 255));
	totalDim.x = 0x100;
	totalDim.y = 0x180;
	iconDim.x = 0x10;
	iconDim.y = 0x10;
	if (!g_pPixmap->Cache(6, "image08\\text.blp", totalDim, iconDim, FALSE))
		return InitFail("Cache text.blp", TRUE);
	g_pPixmap->SetTransparent(6, RGB(0, 0, 255));
	totalDim.x = 0x100;
	totalDim.y = 0x60;
	iconDim.x = 0x10;
	iconDim.y = 0x0c;
	if (!g_pPixmap->Cache(7, "image08\\little.blp", totalDim, iconDim, FALSE))
		return InitFail("Cache little.blp", TRUE);
	g_pPixmap->SetTransparent(7, RGB(0, 0, 255));

	g_pSound = NewZeroed<CSound>();
	if (g_pSound == NULL) return InitFail("New sound", TRUE);
	g_pSound->Create(g_hWnd);
	g_pSound->CacheAll();
	g_pSound->SetState(TRUE);
	g_pNetwork = NewZeroed<CNetwork>();
	if (g_pNetwork == NULL) return InitFail("New network", TRUE);
	g_pMovie = NewZeroed<CMovie>();
	if (g_pMovie == NULL) return InitFail("New movie", FALSE);
	g_pMovie->Create();
	g_pDecor = NewZeroed<CDecor>();
	if (g_pDecor == NULL) return InitFail("New decor", FALSE);
	g_pDecor->Create(g_hWnd, g_pSound, g_pPixmap, g_pNetwork);
	g_pEvent = NewZeroed<CEvent>();
	if (g_pEvent == NULL) return InitFail("New event", FALSE);
	g_pEvent->Create(hInstance, g_hWnd, g_pPixmap, g_pDecor, g_pSound, g_pNetwork, g_pMovie);
	g_pEvent->SetFullScreen(g_bFullScreen);
	g_pEvent->SetMouseType(g_mouseType);
	g_pEvent->ChangePhase(WM_PHASE_INIT);
	g_bTermInit = TRUE;
	return TRUE;
}


/////////////////////////////////////////////////////////////////////////////
// Port host loop. Paths below are host paths, not game paths.

#undef fopen

namespace
{
struct ScriptedInput
{
	long tick;
	int  kind;    // 0 click at (x, y), 1 key press, 2 key down, 3 key up
	int  x, y, vk;
};

struct Options
{
	blupi::host::Options host;
	std::string dataRoot;
	std::string userRoot;
	std::string soundFont;
	int         colorDepth = 0;       // 0 = from config.def
	long        ticks = -1;           // stop after this many WM_UPDATE frames
	std::string trace;                // per-frame "frame phase statehash screenhash"
	long        traceEvery = 1;
	std::vector<std::pair<long, std::string>> stateDumps;
	std::string displayLog;
	std::string displayDumpDir;
	long        displayDumpEvery = 0;
	bool        checkCommands = false;
	bool        checkScene = false;
	std::vector<std::pair<long, std::string>> screenshots;
	std::vector<ScriptedInput> inputs;
	bool        verbose = false;
};

void Usage()
{
	printf(
		"blupi-sdl3 [options]\n"
		"  --data DIR          game root holding DATA/, IMAGE08/, IMAGE16/, SOUND/ (read-only)\n"
		"  --user DIR          writable directory for saves and user missions\n"
		"  --fullscreen        start in desktop fullscreen (Alt+Enter toggles)\n"
		"  --scale N           initial window scale (default 2)\n"
		"  --integer-scale     scale only by whole factors\n"
		"  --color 8|16        override TrueColor from config.def\n"
		"  --soundfont FILE    SoundFont (.sf2) for the MIDI music; default: BLUPI_SOUNDFONT,\n"
		"                      then soundfont.sf2 in the user directory or next to the program\n"
		"  --verbose           log the game's debug output\n"
		"test options:\n"
		"  --headless          no window, no audio device, frames run back to back\n"
		"  --frames N          exit after N game frames (WM_UPDATE)\n"
		"  --trace FILE        write 'frame phase state-hash screen-hash' per frame\n"
		"  --trace-every N     write only every Nth frame (checkpoints)\n"
		"  --state-dump N:FILE write every hashed state field after frame N\n"
		"  --display-log FILE  raw back-buffer hash at every Display (cross-build oracle)\n"
		"  --display-dump DIR:N  raw frame file at every Nth Display\n"
		"  --check-commands    replay every frame's rendering commands on a second device\n"
		"                      and require identical pixels (exit code 3 otherwise)\n"
		"  --check-scene       build the frame scene before CDecor::Build and require the\n"
		"                      same rendering commands (exit code 4 otherwise)\n"
		"  --screenshot N:FILE write the screen after frame N as a 24-bit BMP\n"
		"  --input FILE        scripted input: lines 'FRAME click X Y' or 'FRAME key VK'\n");
}

bool LoadInputScript(const std::string& path, std::vector<ScriptedInput>& inputs)
{
	FILE* file = fopen(path.c_str(), "r");
	if (file == NULL) return false;
	char line[200];
	while (fgets(line, sizeof(line), file))
	{
		ScriptedInput in = {};
		char kind[16] = {};
		if (line[0] == '#' || sscanf(line, "%ld %15s", &in.tick, kind) != 2) continue;
		if (strcmp(kind, "click") == 0 && sscanf(line, "%ld %15s %d %d", &in.tick, kind, &in.x, &in.y) == 4)
		{
			in.kind = 0;
			inputs.push_back(in);
			continue;
		}
		in.kind = strcmp(kind, "key") == 0 ? 1 : strcmp(kind, "keydown") == 0 ? 2 : strcmp(kind, "keyup") == 0 ? 3 : -1;
		if (in.kind > 0 && sscanf(line, "%ld %15s %d", &in.tick, kind, &in.vk) == 3)
		{
			inputs.push_back(in);
		}
	}
	fclose(file);
	return true;
}

bool ParseOptions(int argc, char** argv, Options& o)
{
	for (int i = 1; i < argc; i++)
	{
		std::string a = argv[i];
		auto next = [&]() -> const char* { return i + 1 < argc ? argv[++i] : ""; };
		if (a == "--data") o.dataRoot = next();
		else if (a == "--user") o.userRoot = next();
		else if (a == "--fullscreen") o.host.fullscreen = true;
		else if (a == "--scale") o.host.scale = atoi(next());
		else if (a == "--integer-scale") o.host.integerScale = true;
		else if (a == "--color") o.colorDepth = atoi(next());
		else if (a == "--soundfont") o.soundFont = next();
		else if (a == "--verbose") o.verbose = true;
		else if (a == "--headless") o.host.headless = true;
		else if (a == "--frames") o.ticks = atol(next());
		else if (a == "--trace") o.trace = next();
		else if (a == "--trace-every") { o.traceEvery = atol(next()); if (o.traceEvery < 1) o.traceEvery = 1; }
		else if (a == "--display-log") o.displayLog = next();
		else if (a == "--check-commands") o.checkCommands = true;
		else if (a == "--check-scene") o.checkScene = true;
		else if (a == "--display-dump")
		{
			std::string v = next();
			size_t colon = v.rfind(':');
			if (colon == std::string::npos || colon < 2) return false;
			o.displayDumpDir = v.substr(0, colon);
			o.displayDumpEvery = atol(v.substr(colon + 1).c_str());
		}
		else if (a == "--state-dump")
		{
			std::string v = next();
			size_t colon = v.find(':');
			if (colon == std::string::npos) return false;
			o.stateDumps.push_back({atol(v.substr(0, colon).c_str()), v.substr(colon + 1)});
		}
		else if (a == "--screenshot")
		{
			std::string v = next();
			size_t colon = v.find(':');
			if (colon == std::string::npos) return false;
			o.screenshots.push_back({atol(v.substr(0, colon).c_str()), v.substr(colon + 1)});
		}
		else if (a == "--input")
		{
			if (!LoadInputScript(next(), o.inputs)) return false;
		}
		else if (a == "--help" || a == "-h") return false;
		else if (a == "-nocd") {}  // historical switch, meaningless here
		else
		{
			printf("unknown option: %s\n", a.c_str());
			return false;
		}
	}
	return true;
}

bool LooksLikeGameRoot(const std::string& root)
{
	if (root.empty()) return false;
	blupi::vfs::set_roots(root, "");
	return !blupi::vfs::resolve_read("data\\config.def").empty() &&
		   !blupi::vfs::resolve_read("image08\\init.blp").empty();
}

LPARAM MousePos(int x, int y)
{
	return MAKELPARAM(x, y);
}

WPARAM MouseKeys(const blupi::host::Event& e)
{
	WPARAM w = 0;
	if (e.leftDown) w |= MK_LBUTTON;
	if (e.rightDown) w |= MK_RBUTTON;
	if (e.shift) w |= MK_SHIFT;
	if (e.control) w |= MK_CONTROL;
	return w;
}

// Host event -> the window messages Windows would have delivered.
void TranslateHostEvent(const blupi::host::Event& e)
{
	using blupi::host::EventType;
	switch (e.type)
	{
	case EventType::Quit:
		WindowProc(WM_CLOSE, 0, 0);
		break;
	case EventType::FocusGained:
		WindowProc(WM_ACTIVATEAPP, 1, 0);
		break;
	case EventType::FocusLost:
		WindowProc(WM_ACTIVATEAPP, 0, 0);
		break;
	case EventType::KeyDown:
		WindowProc(WM_KEYDOWN, (WPARAM)e.key, (LPARAM)(1 | (e.repeat ? (1 << 30) : 0)));
		// TranslateMessage: control characters that become WM_CHAR.
		if (e.key == VK_BACK || e.key == VK_RETURN || e.key == VK_ESCAPE || e.key == VK_TAB)
			WindowProc(WM_CHAR, (WPARAM)e.key, 1);
		break;
	case EventType::KeyUp:
		WindowProc(WM_KEYUP, (WPARAM)e.key, (LPARAM)(1 | (1 << 30) | (1u << 31)));
		break;
	case EventType::Text:
		WindowProc(WM_CHAR, (WPARAM)e.ch, 1);
		break;
	case EventType::MouseMove:
		if (e.inside) WindowProc(WM_MOUSEMOVE, MouseKeys(e), MousePos(e.x, e.y));
		else WindowProc(WM_NCMOUSEMOVE, 0, 0);
		break;
	case EventType::MouseDown:
		if (!e.inside) break;
		WindowProc(e.button == 1 ? WM_LBUTTONDOWN : WM_RBUTTONDOWN, MouseKeys(e), MousePos(e.x, e.y));
		break;
	case EventType::MouseUp:
		if (!e.inside) break;
		WindowProc(e.button == 1 ? WM_LBUTTONUP : WM_RBUTTONUP, MouseKeys(e), MousePos(e.x, e.y));
		break;
	}
}

// TranslateMessage for posted key presses (scripted input): the historical
// message loop turned them into a following WM_CHAR. Unshifted keyboard state.
// Live keyboard input gets its WM_CHAR from TranslateHostEvent instead.
int TranslatedChar(WPARAM vk)
{
	if (vk == VK_BACK || vk == VK_TAB || vk == VK_RETURN || vk == VK_ESCAPE || vk == VK_SPACE) return (int)vk;
	if (vk >= '0' && vk <= '9') return (int)vk;
	if (vk >= 'A' && vk <= 'Z') return (int)(vk - 'A' + 'a');
	return 0;
}

void DispatchPosted()
{
	PortMessage msg;
	while (!g_bQuit && PortPopMessage(msg))
	{
		WindowProc(msg.message, msg.wParam, msg.lParam);
		int ch = msg.message == WM_KEYDOWN ? TranslatedChar(msg.wParam) : 0;
		if (ch != 0 && !g_bQuit) WindowProc(WM_CHAR, (WPARAM)ch, msg.lParam);
	}
}

// Posts the scripted messages due before this frame, like the historical
// smoke-test harness did with PostMessage.
void PostScriptedInput(const Options& o, long frame)
{
	for (const ScriptedInput& in : o.inputs)
	{
		if (in.tick != frame) continue;
		if (in.kind == 0)
		{
			LPARAM pos = MousePos(in.x, in.y);
			PostMessage(g_hWnd, WM_MOUSEMOVE, 0, pos);
			PostMessage(g_hWnd, WM_LBUTTONDOWN, MK_LBUTTON, pos);
			PostMessage(g_hWnd, WM_LBUTTONUP, 0, pos);
		}
		if (in.kind == 1 || in.kind == 2)
		{
			PostMessage(g_hWnd, WM_KEYDOWN, (WPARAM)in.vk, 1);
		}
		if (in.kind == 1 || in.kind == 3)
		{
			PostMessage(g_hWnd, WM_KEYUP, (WPARAM)in.vk, (LPARAM)(1 | (1 << 30) | (1u << 31)));
		}
	}
}
}  // namespace

int BlupiMain(int argc, char** argv)
{
	Options o;
	if (!ParseOptions(argc, argv, o))
	{
		Usage();
		return 2;
	}
	if (!blupi::host::init(TITLE, LXIMAGE, LYIMAGE, o.host)) return 1;
	blupi::host::set_verbose(o.verbose);

	// Locate the read-only game data and the writable user directory.
	std::vector<std::string> candidates;
	if (!o.dataRoot.empty()) candidates.push_back(o.dataRoot);
	if (const char* env = getenv("BLUPI_DATA")) candidates.push_back(env);
	candidates.push_back(blupi::host::base_path());
	candidates.push_back(".");
	std::string root;
	for (const std::string& c : candidates)
	{
		if (LooksLikeGameRoot(c)) { root = c; break; }
		if (!o.dataRoot.empty()) break;  // an explicit --data must be right
	}
	if (root.empty())
	{
		blupi::host::message_box(TITLE, "Speedy Blupi data not found.\n"
			"Pass --data <folder containing DATA, IMAGE08, IMAGE16 and SOUND>.");
		blupi::host::shutdown();
		return 1;
	}
	std::string user = o.userRoot.empty() ? blupi::host::pref_path() : o.userRoot;
	blupi::vfs::set_roots(root, user);
	blupi::audio::init(!o.host.headless);

	// SoundFont for the MIDI music. Headless runs load one only on request, so
	// automated runs stay independent of the audio clock.
	{
		std::vector<std::string> fonts;
		if (!o.soundFont.empty()) fonts.push_back(o.soundFont);
		else if (!o.host.headless)
		{
			if (const char* env = getenv("BLUPI_SOUNDFONT")) fonts.push_back(env);
			fonts.push_back(user + "/soundfont.sf2");
			fonts.push_back(blupi::host::base_path() + "soundfont.sf2");
		}
		for (const std::string& f : fonts)
		{
			if (blupi::music::load_soundfont(f))
			{
				blupi::host::log_info(("MIDI music: SoundFont " + f).c_str());
				break;
			}
		}
	}

	FILE* trace = NULL;
	if (!o.trace.empty()) trace = fopen(o.trace.c_str(), "w");
	FILE* displayLog = o.displayLog.empty() ? NULL : fopen(o.displayLog.c_str(), "w");
	PortPixmapSetDisplayCapture(displayLog, o.displayDumpDir.c_str(), o.displayDumpEvery);

	g_hWnd = NULL;
	g_bActive = TRUE;  // the window is created focused (WM_ACTIVATEAPP)

	g_portColorDepth = o.colorDepth;
	if (o.checkCommands) PortPixmapEnableCommandCheck();
	g_bCheckScene = o.checkScene;
	if (!DoInit(NULL, (LPSTR)"", 0))
	{
		blupi::audio::shutdown();
		blupi::host::shutdown();
		return 1;
	}

	Benchmark();

	const uint64_t interval = (uint64_t)g_timerInterval * 1000000ull;
	uint64_t nextTick = blupi::host::now_ns() + interval;
	long frame = 0;
	int lastTimer = 0;

	while (!g_bQuit)
	{
		blupi::host::Event e;
		while (!g_bQuit && blupi::host::poll_event(e)) TranslateHostEvent(e);
		if (g_bQuit) break;

		uint64_t now = blupi::host::now_ns();
		// Messages already queued (and those they post, e.g. a button's command
		// after WM_LBUTTONUP) are handled before the timer's WM_UPDATE, as in
		// the original message loop, where the timer thread posted WM_UPDATE
		// behind them. [EVIDENCE] cross-build oracle, scripted tour 2026-10-21.
		PortSoundPoll();
		DispatchPosted();
		if (g_bQuit) break;
		if (o.host.headless || now >= nextTick)
		{
			PostScriptedInput(o, frame);
			DispatchPosted();
			if (g_bQuit) break;
			TimerStep();
			nextTick += interval;
			if (nextTick <= now) nextTick = now + interval;  // missed ticks are dropped
		}
		lastTimer = g_timer;
		DispatchPosted();
		if (g_bQuit) break;

		if (lastTimer == 1 && g_timer == 0)  // a WM_UPDATE frame completed
		{
			frame++;
			if (o.host.headless)
			{
				// No audio device: advance the synthesizer by one timer interval per
				// frame so music, and its end-of-song notification, follow game time.
				static std::vector<float> discard;
				int frames = 44100 * g_timerInterval / 1000;
				discard.assign((size_t)frames * 2, 0.0f);
				blupi::music::render_mix(discard.data(), frames, 44100);
			}
			if (trace && frame % o.traceEvery == 0)
			{
				fprintf(trace, "%ld %u %016llx %016llx\n", frame, g_pEvent ? g_pEvent->GetPhase() : 0u,
						(unsigned long long)PortStateHash(),
						(unsigned long long)PortPixmapScreenHash());
			}
			for (const auto& dump : o.stateDumps)
			{
				if (dump.first != frame) continue;
				if (FILE* f = fopen(dump.second.c_str(), "w")) { PortStateDump(f); fclose(f); }
			}
			for (const auto& shot : o.screenshots)
			{
				if (shot.first == frame) PortPixmapWriteScreenBmp(shot.second.c_str());
			}
			if (o.ticks >= 0 && frame >= o.ticks)
			{
				WindowProc(WM_CLOSE, 0, 0);
				break;
			}
		}

		PortPixmapPresentIfDirty();

		if (!o.host.headless)
		{
			now = blupi::host::now_ns();
			if (now < nextTick && blupi::host::wait_event(e, nextTick - now)) TranslateHostEvent(e);
		}
	}

	if (trace) fclose(trace);
	PortPixmapSetDisplayCapture(NULL, NULL, 0);
	if (displayLog) fclose(displayLog);
	int result = 0;
	long checkFrames, checkMismatches, checkFirst;
	if (PortPixmapCommandCheckResult(checkFrames, checkMismatches, checkFirst))
	{
		printf("command check: %ld displays replayed, %ld mismatches (first at display %ld)\n",
			   checkFrames, checkMismatches, checkFirst);
		if (checkMismatches != 0 || checkFrames == 0) result = 3;
	}
	if (o.checkScene)
	{
		printf("scene check: %ld Build calls compared, %ld mismatches (first at call %ld%s%s)\n",
			   g_sceneFrames, g_sceneMismatches, g_sceneFirstMismatch,
			   g_sceneFirstDetail.empty() ? "" : ": ", g_sceneFirstDetail.c_str());
		if (g_sceneMismatches != 0 || g_sceneFrames == 0) result = 4;
	}
	blupi::audio::shutdown();
	blupi::music::shutdown();
	blupi::host::shutdown();
	return result;
}
