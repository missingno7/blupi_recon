// SB2 application adapter: games/sb2/src/blupi.cpp (historical origin),
// using the SDL host loop and zeroed allocation from SB1 port/main.cpp.
// Build runs exactly once per logical tick; no scene extraction/enhancements.
// Port replacement of games/sb2/src/blupi.cpp (historical: WinMain, WindowProc,
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
#include "game/member_access.h"
#include "platform/audio.h"
#include "platform/music.h"
#include "platform/host.h"
#include "platform/vfs.h"

#include <new>
#include <string>
#include <utility>
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
BOOL g_bTrueColorBack = FALSE;
BOOL g_bTrueColorDecor = FALSE;
BOOL g_bBenchmarkSuccess = FALSE;
BOOL g_bCDAudio = FALSE;
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
// large objects (SB2 CDecor was 0x26320 bytes on Win32) from the VC5 C runtime, which serves
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
	int 		i;


	file = fopen("data\\config.def", "rb");
	if (file == NULL)   return FALSE;
	nb = fread(buffer, sizeof(char), 200 - 1, file);
	buffer[nb] = 0;
	fclose(file);


	g_CDPath[0] = 0; // VFS supplies a read-only folder.

	pText = strstr(buffer, "SpeedRate=");
	if (pText != NULL)
	{
		g_speedRate = GetNum(pText + 10);
		if (g_speedRate < 1)  g_speedRate = 1;
		if (g_speedRate > 2)  g_speedRate = 2;
	}

	pText = strstr(buffer, "Timer=");
	if (pText != NULL)
	{
		g_timerInterval = GetNum(pText + 6);
		if (g_timerInterval <   10)  g_timerInterval = 10;
		if (g_timerInterval > 1000)  g_timerInterval = 1000;
	}

	pText = strstr(buffer, "FullScreen=");
	if (pText != NULL)
	{
		g_bFullScreen = GetNum(pText + 11);
		if (g_bFullScreen != 0)  g_bFullScreen = 1;
	}

	pText = strstr(buffer, "MouseType=");
	if (pText != NULL)
	{
		g_mouseType = GetNum(pText + 10);
		if (g_mouseType < 1)  g_mouseType = 1;
		if (g_mouseType > 9)  g_mouseType = 9;
	}

	pText = strstr(buffer, "Benchmark=");
	if (pText != NULL)
	{
		g_benchmark = GetNum(pText + 10);
		if (g_benchmark < 0) g_benchmark = 0;
		if (g_benchmark > 100000) g_benchmark = 100000;
		if (g_benchmark >= 3100)
		{
			g_bBenchmarkSuccess = TRUE;
			g_bTrueColorBack = TRUE;
			g_bTrueColorDecor = TRUE;
		}
	}

	pText = strstr(buffer, "TrueColor=");
	if (pText != NULL)
	{
		i = GetNum(pText + 10);
		if (i == 8) g_bTrueColorBack = FALSE;
		if (i == 16) g_bTrueColorBack = TRUE;
		g_bTrueColorDecor = g_bTrueColorBack;
	}

	pText = strstr(buffer, "TrueColorBack=");
	if (pText != NULL)
	{
		i = GetNum(pText + 14);
		if (i == 8) g_bTrueColorBack = FALSE;
		if (i == 16) g_bTrueColorBack = TRUE;
	}

	pText = strstr(buffer, "TrueColorDecor=");
	if (pText != NULL)
	{
		i = GetNum(pText + 15);
		if (i == 8) g_bTrueColorDecor = FALSE;
		if (i == 16) g_bTrueColorDecor = TRUE;
	}

	pText = strstr(buffer, "CDAudio=");
	if (pText != NULL)
	{
		i = GetNum(pText + 8);
		g_bCDAudio = (i == TRUE);
	}



	if (!g_bBenchmarkSuccess)
	{
		g_bTrueColorBack = FALSE;
		g_bTrueColorDecor = FALSE;
	}

    if (g_portColorDepth == 8 || g_portColorDepth == 16) {
        g_bTrueColorBack = g_bTrueColorDecor = (g_portColorDepth == 16);
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

#if _EGAMES  // Only Eggbert 2 has the publisher intro screens.
	if (phase == WM_PHASE_INTRO1 || phase == WM_PHASE_INTRO2)
	{
		g_pEvent->IntroStep();
	}
#endif

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
		{
			RECT rect = { 0, 0, LXIMAGE, LYIMAGE };
			g_pDecor->Build(rect, posMouse);
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
		blupi::host::log_debug(wParam != 0 ? "Port: WM_ACTIVATEAPP active" : "Port: WM_ACTIVATEAPP inactive");
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
		if (g_pEvent != NULL) g_pEvent->StopMovie();

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
#define PORT_EVENT_MEMBERS(X) X(m_index) X(SearchPhase) X(m_bDemoPlay) X(m_demoNumber)
BLUPI_MEMBER_ACCESS(PortEventAccess, CEvent, PORT_EVENT_MEMBERS)
static void PortFirstPhaseIndex(CEvent* event, UINT phase)
{
	int index = (event->*PortEventAccess::SearchPhase)(phase);
	if (index >= 0) event->*PortEventAccess::m_index = index;
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
    g_pPixmap->SetBenchmarkSuccess(g_bBenchmarkSuccess);
    if (!g_pPixmap->CacheAll(TRUE, g_hWnd, g_bFullScreen, g_bTrueColorBack,
                             g_bTrueColorDecor, g_mouseType, "init.blp", 0))
        return InitFail("CacheAll", TRUE);

	g_pSound = NewZeroed<CSound>();
	if (g_pSound == NULL) return InitFail("New sound", TRUE);
	g_pSound->Create(g_hWnd);
	g_pSound->CacheAll();
	g_pSound->SetState(TRUE);
	g_pSound->SetCDAudio(g_bCDAudio);
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
#if _INTRO
	g_pEvent->ChangePhase(WM_PHASE_INTRO1);
#else
	PortFirstPhaseIndex(g_pEvent, WM_PHASE_INIT);
	g_pEvent->ChangePhase(WM_PHASE_INIT);
#endif
	g_bTermInit = TRUE;
	return TRUE;
}
// Host paths from here on; historical fopen above remains VFS-routed.
#undef fopen

namespace {
struct Input { long frame; std::string kind; int x = 0, y = 0; };
struct Options {
    blupi::host::Options host;
    std::string data, user, trace, soundfont, language, demoLog;
    long frames = -1, every = 1;
    std::vector<Input> inputs;
    std::vector<std::pair<long, std::string>> shots, dumps;
    bool commands = false;
};
void Usage() {
    puts("blupi2 " BLUPI_PORT_VERSION "\n"
         "  --data DIR --user DIR --headless --frames N --color 8|16\n"
         "  --trace FILE --trace-every N --screenshot N:FILE --state-dump N:FILE\n"
         "  --input FILE --soundfont FILE --language CODE --check-commands --demo-log FILE\n"
         "  --fullscreen --scale N --verbose --version\n"
         "Data is read-only; all game writes go to --user (use build/ for testing).");
}
bool Parse(int argc, char** argv, Options& o) {
    for (int i = 1; i < argc; ++i) {
        std::string a = argv[i];
        auto next = [&]() -> const char* { return i+1 < argc ? argv[++i] : ""; };
        if (a == "--data") o.data = next();
        else if (a == "--user") o.user = next();
        else if (a == "--headless") o.host.headless = true;
        else if (a == "--frames") o.frames = atol(next());
        else if (a == "--color") { g_portColorDepth = atoi(next()); if (g_portColorDepth != 8 && g_portColorDepth != 16) return false; }
        else if (a == "--trace") o.trace = next();
        else if (a == "--demo-log") o.demoLog = next();
        else if (a == "--trace-every") { o.every = atol(next()); if (o.every < 1) return false; }
        else if (a == "--soundfont") o.soundfont = next();
        else if (a == "--language") o.language = next();
        else if (a == "--fullscreen") o.host.fullscreen = true;
        else if (a == "--scale") o.host.scale = atoi(next());
        else if (a == "--verbose") blupi::host::set_verbose(true);
        else if (a == "--check-commands") o.commands = true;
        else if (a == "--screenshot" || a == "--state-dump") {
            std::string v = next(); size_t colon = v.find(':');
            if (colon == std::string::npos) return false;
            auto p = std::make_pair(atol(v.substr(0, colon).c_str()), v.substr(colon+1));
            if (a == "--screenshot") o.shots.push_back(p); else o.dumps.push_back(p);
        } else if (a == "--input") {
            FILE* f = fopen(next(), "r"); if (!f) return false;
            char line[256], kind[16];
            while (fgets(line, sizeof(line), f)) {
                Input in;
                if (line[0] == '#' || sscanf(line, "%ld %15s %d %d", &in.frame, kind, &in.x, &in.y) < 3) continue;
                in.kind = kind;
                if (in.kind != "key" && in.kind != "keydown" && in.kind != "keyup" && in.kind != "click" && in.kind != "move") { fclose(f); return false; }
                o.inputs.push_back(in);
            }
            fclose(f);
        } else return false;
    }
    // An unattended run always has a finite end, including on bad options/data.
    return !o.host.headless || o.frames >= 0;
}
int TranslatedChar(WPARAM vk) {
    if (vk == VK_BACK || vk == VK_TAB || vk == VK_RETURN || vk == VK_ESCAPE || vk == VK_SPACE) return (int)vk;
    if (vk >= '0' && vk <= '9') return (int)vk;
    if (vk >= 'A' && vk <= 'Z') return (int)(vk-'A'+'a');
    return 0;
}
void DispatchPosted() {
    PortMessage m;
    while (!g_bQuit && PortPopMessage(m)) {
        WindowProc(m.message, m.wParam, m.lParam);
        int ch = m.message == WM_KEYDOWN ? TranslatedChar(m.wParam) : 0;
        if (ch && !g_bQuit) WindowProc(WM_CHAR, ch, m.lParam);
    }
}
void TranslateHostEvent(const blupi::host::Event& e) {
    using blupi::host::EventType;
    switch (e.type) {
    case EventType::Quit: WindowProc(WM_CLOSE, 0, 0); break;
    case EventType::FocusGained: WindowProc(WM_ACTIVATEAPP, 1, 0); break;
    case EventType::FocusLost: WindowProc(WM_ACTIVATEAPP, 0, 0); break;
    case EventType::KeyDown:
        WindowProc(WM_KEYDOWN, e.key, 1 | (e.repeat ? (1<<30) : 0));
        if (e.key == VK_BACK || e.key == VK_RETURN || e.key == VK_ESCAPE || e.key == VK_TAB) WindowProc(WM_CHAR, e.key, 1);
        break;
    case EventType::KeyUp: WindowProc(WM_KEYUP, e.key, 1 | (1u<<30) | (1u<<31)); break;
    case EventType::Text: WindowProc(WM_CHAR, e.ch, 1); break;
    case EventType::MouseMove: case EventType::MouseDown: case EventType::MouseUp: {
        UINT msg = WM_MOUSEMOVE;
        if (e.type == EventType::MouseDown) msg = e.button == 1 ? WM_LBUTTONDOWN : WM_RBUTTONDOWN;
        if (e.type == EventType::MouseUp) msg = e.button == 1 ? WM_LBUTTONUP : WM_RBUTTONUP;
        if (!e.inside) { if (msg == WM_MOUSEMOVE) WindowProc(WM_NCMOUSEMOVE, 0, 0); break; }
        WPARAM keys = (e.leftDown ? MK_LBUTTON : 0) | (e.rightDown ? MK_RBUTTON : 0) | (e.shift ? MK_SHIFT : 0) | (e.control ? MK_CONTROL : 0);
        WindowProc(msg, keys, MAKELPARAM(e.x, e.y));
        break;
    }
    }
}
void PostInput(const Options& o, long frame) {
    for (const Input& in : o.inputs) {
        if (in.frame != frame) continue;
        if (in.kind == "move" || in.kind == "click") {
            SetCursorPos(in.x, in.y);
            PostMessage(g_hWnd, WM_MOUSEMOVE, 0, MAKELPARAM(in.x, in.y));
            if (in.kind == "click") {
                PostMessage(g_hWnd, WM_LBUTTONDOWN, MK_LBUTTON, MAKELPARAM(in.x, in.y));
                PostMessage(g_hWnd, WM_LBUTTONUP, 0, MAKELPARAM(in.x, in.y));
            }
        } else {
            if (in.kind != "keyup") PostMessage(g_hWnd, WM_KEYDOWN, in.x, 1);
            if (in.kind != "keydown") PostMessage(g_hWnd, WM_KEYUP, in.x, 1 | (1u<<30) | (1u<<31));
        }
    }
}
} // namespace

int BlupiMain(int argc, char** argv) {
    if (argc == 2 && std::string(argv[1]) == "--version") { Usage(); return 0; }
    if (argc == 2 && (std::string(argv[1]) == "--help" || std::string(argv[1]) == "-h")) { Usage(); return 0; }
    Options o;
    if (!Parse(argc, argv, o)) { Usage(); return 2; }
    if (!blupi::host::init("Speedy Blupi 2 / Speedy Eggbert 2", LXIMAGE, LYIMAGE, o.host)) return 1;
    if (o.data.empty()) { if (const char* p = getenv("BLUPI2_DATA")) o.data = p; }
    if (o.data.empty()) { blupi::host::message_box("Blupi 2", "Pass --data <SB2 game folder> or set BLUPI2_DATA."); blupi::host::shutdown(); return 1; }
    if (o.user.empty()) o.user = blupi::host::pref_path("Epsitec", "Speedy Blupi 2 SDL3");
    blupi::vfs::set_roots(o.data, o.user);
    if (blupi::vfs::resolve_read("data/config.def").empty() || blupi::vfs::resolve_read("image08/init.blp").empty()) {
        blupi::host::message_box("Blupi 2", "SB2 game data not found."); blupi::host::shutdown(); return 1;
    }
    PortLanguageInit(o.language);
    blupi::audio::init(!o.host.headless);
    if (!o.soundfont.empty()) blupi::music::load_soundfont(o.soundfont);
    else if (!o.host.headless) {
        const char* env = getenv("BLUPI_SOUNDFONT");
        if (!env || !blupi::music::load_soundfont(env)) blupi::music::load_soundfont(blupi::host::base_path()+"soundfont.sf2");
    }
    FILE* trace = o.trace.empty() ? NULL : fopen(o.trace.c_str(), "w");
    FILE* demoLog = o.demoLog.empty() ? NULL : fopen(o.demoLog.c_str(), "w");
    if ((!o.trace.empty() && !trace) || (!o.demoLog.empty() && !demoLog)) {
        if (trace) fclose(trace);
        if (demoLog) fclose(demoLog);
        blupi::audio::shutdown(); blupi::music::shutdown(); blupi::host::shutdown(); return 2;
    }
    g_bActive = TRUE;
    if (o.commands) PortPixmapEnableCommandCheck();
    int result = 0;
    if (!DoInit(NULL, (LPSTR)"", 0)) result = 1;
    else {
        PortPixmapHideCursor(blupi::host::touch_screen());
        Benchmark();
        const uint64_t interval = (uint64_t)g_timerInterval*1000000;
        uint64_t next = blupi::host::now_ns()+interval;
        long frame = 0;
        int lastDemo = -1;
        while (!g_bQuit && (o.frames < 0 || frame < o.frames)) {
            blupi::host::Event e;
            while (!g_bQuit && blupi::host::poll_event(e)) TranslateHostEvent(e);
            if (g_bQuit) break;
            PortSoundPoll(); DispatchPosted();
            if (g_bQuit) break;
            uint64_t now = blupi::host::now_ns();
            if (o.host.headless || now >= next) {
                PostInput(o, frame); DispatchPosted();
                if (g_bQuit) break;
                TimerStep(); int pending = g_timer; DispatchPosted();
                if (g_bQuit) break;
                next += interval; if (next <= now) next = now+interval;
                if (pending == 1 && g_timer == 0) {
                    ++frame;
                    int demo = g_pEvent->*PortEventAccess::m_bDemoPlay ? 300 + g_pEvent->*PortEventAccess::m_demoNumber : -1;
                    if (demoLog && demo != lastDemo) fprintf(demoLog, "%ld %d\n", frame, demo);
                    lastDemo = demo;
                    if (o.host.headless) {
                        std::vector<float> discard((size_t)(44100*g_timerInterval/1000)*2);
                        blupi::music::render_mix(discard.data(), (int)discard.size()/2, 44100);
                    }
                    if (trace && frame%o.every == 0) fprintf(trace, "%ld %u %016llx %016llx\n", frame, g_pEvent->GetPhase(), (unsigned long long)PortStateHash(), (unsigned long long)PortPixmapScreenHash());
                    for (const auto& p : o.shots) if (p.first == frame && !PortPixmapWriteScreenBmp(p.second.c_str())) result = 2;
                    for (const auto& p : o.dumps) if (p.first == frame) {
                        FILE* f = fopen(p.second.c_str(), "w"); if (f) { PortStateDump(f); fclose(f); } else result = 2;
                    }
                }
            }
            PortPixmapPresentIfDirty();
            now = blupi::host::now_ns();
            if (!o.host.headless && now < next && blupi::host::wait_event(e, next-now)) TranslateHostEvent(e);
        }
    }
    if (trace) fclose(trace);
    if (demoLog) fclose(demoLog);
    long displays, mismatches, first;
    if (PortPixmapCommandCheckResult(displays, mismatches, first)) {
        printf("SB2 command check: %ld displays, %ld mismatches (first %ld)\n", displays, mismatches, first);
        if (!displays || mismatches) result = 3;
    }
    FinishObjects();
    blupi::audio::shutdown(); blupi::music::shutdown(); blupi::host::shutdown();
    return result;
}

