// SB2 application adapter: games/sb2/src/blupi.cpp (historical origin),
// using the SDL host loop and zeroed allocation from SB1 port/main.cpp.
// A const scene plus EndOfTick replaces Build once per logical tick (M4a).
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
#include "game/scene.h"
#include "ui/settings.h"
#include "platform/audio.h"
#include "platform/music.h"
#include "platform/host.h"
#include "platform/vfs.h"
#include "presentation/timing.h"
#include "app/launcher.h"

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
static long g_drawProbeFrame = -1;
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

	// Port edition branch, from blupi.cpp's _EGAMES block.
	if (PortEdition() == "eggbert" &&
		(phase == WM_PHASE_INTRO1 || phase == WM_PHASE_INTRO2))
	{
		g_pEvent->IntroStep();
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
static bool g_bCheckView = false;
static long g_sceneFrames = 0, g_sceneMismatches = 0, g_sceneFirstMismatch = -1;
static std::string g_sceneFirstDetail;
// World area the scene records beyond Build's view: none in faithful mode,
// the interpolation margin with --interpolate (ROADMAP.md stage 7).
static blupi::scene::Coverage g_sceneCoverage;

static void PortFrameForCheck(const blupi::scene::Scene& scene);

static void PortBuildDecor(RECT rect, POINT posMouse)
{
	if (!g_bCheckScene && !g_bCheckView)
	{
		g_pDecor->Build(rect, posMouse);
		return;
	}
	blupi::scene::Scene scene;
	std::vector<blupi::render::Command> historical, flattened;
	const uint64_t before = PortStateHash();
	blupi::scene::BuildScene(*g_pDecor, rect, posMouse, scene, g_sceneCoverage);
	const bool pure = before == PortStateHash();
	PortPixmapCaptureBegin(true);
	g_pDecor->Build(rect, posMouse);
	PortPixmapCaptureEnd(historical);
	PortPixmapCaptureBegin(false);
	blupi::scene::FlattenScene(scene, g_pPixmap);
	PortPixmapCaptureEnd(flattened);
	if (g_bCheckView) { PortFrameForCheck(scene); }

	size_t i = 0;
	while (i < historical.size() && i < flattened.size() && blupi::render::same_command(historical[i], flattened[i])) i++;
	if (!pure || i != historical.size() || i != flattened.size())
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

// Port: the frame a tick produces, as data (ROADMAP.md stage 4). Rendering it
// reads no game state, so it can be drawn again at any time.
//   world/scene     the scene of CDecor::Build (world phases), or
//   background      the captured commands of the full-screen background image
//   ui              the captured commands of CEvent::DrawButtons
//   motion          with --interpolate: where the scene's items were in the
//                   previous tick's scene (ROADMAP.md stage 7)
struct PortFrame
{
	bool world = false;
	blupi::scene::Scene scene;
	blupi::scene::Motion motion;
	std::vector<blupi::render::Command> background;
	std::vector<blupi::render::Command> ui;
};
static PortFrame g_frame, g_prevFrame;
static bool g_bReference = false;   // --reference: the historical SetDecor body
static int  g_renderRepeat = 0;     // --render-repeat: extra renders per tick (test)
static bool g_frameValid = false;   // g_frame holds a tick's frame
static bool g_bInterpolate = false; // --interpolate (ROADMAP.md stage 7)
static long g_viewFrames = 0, g_viewMismatches = 0, g_viewFirstMismatch = -1;
static int  g_wideWidth = 0;        // --wide W: fixed view width (tests); 0: from the host
static bool g_bCheckWide = false;   // --check-wide
static long g_wideFrames = 0, g_wideSkipped = 0, g_wideMismatches = 0, g_wideFirstMismatch = -1;
static bool g_bCheckPresentation = false;
static long g_presentationFrames = 0, g_presentationMismatches = 0;
static bool g_bHeadless = false;
static bool g_presentDisplayRate = true;   // --present display|tick, "Frame rate"
static PortSettings g_settings;            // game/settings.h
static std::string  g_settingsPath;        // empty: not saved (headless runs)
static void PortRenderFrame(const PortFrame& frame, double alpha);
static void PortPresentWide(double alpha, int width);

// Width of the view presented now (ROADMAP.md stage 8): a fixed --wide width,
// else the host's (the window shape with automatic widescreen).
static int PortViewWidth()
{
	if (g_wideWidth > 0) return g_wideWidth;
	if (g_bHeadless) return LXIMAGE;
	return blupi::host::view_width();
}

// The tick's frame, or on the game's settings screen with the port's
// "Display options" open, its background only; then the port's screen or its
// entry button (game/options_screen.cpp). Into the back buffer.
static void PortRenderPresentation(double alpha)
{
	if (PortOptionsOpen() && !g_frame.world)
	{
		RECT clip = g_pPixmap->GetClipping();
		PortPixmapSubmit(g_frame.background);
		g_pPixmap->SetClipping(clip);
	}
	else
	{
		PortRenderFrame(g_frame, alpha);
	}
	if (PortOptionsVisible())
	{
		RECT clip = g_pPixmap->GetClipping();
		g_pPixmap->SetClipping(RECT{0, 0, LXIMAGE, LYIMAGE});
		PortOptionsDraw();
		g_pPixmap->SetClipping(clip);
	}
}

// Presentation (ROADMAP.md stages 5 and 8): draws the latest tick's frame and
// the cursor at its current position, and presents it. Reads no game state
// and changes none; the simulation keeps its fixed tick. alpha is the time
// since that tick, in ticks (used with interpolation only).
static void PortPresentNow(double alpha)
{
	const int width = PortViewWidth();
	const bool check = g_bCheckPresentation || g_bCheckView;
	const uint64_t state = check ? PortStateHash() : 0;
	const uint64_t screen = check ? PortPixmapScreenHash() : 0;
	// SB1's wide target also works at 640. Use it for every SB2 presentation
	// so intermediate positions, the options overlay and cursor never replace
	// the faithful primary surface, even between ticks at the original width.
	PortPixmapSetWide(width);
	PortPresentWide(alpha, width);
	if (check)
	{
		if (state != PortStateHash() || screen != PortPixmapScreenHash()) ++g_presentationMismatches;
		++g_presentationFrames;
	}
}

// Draws a tick's frame into the back buffer, in the original order (Build or
// the background, then the buttons). CPixmap's clip state is left as the tick
// left it, as in the original, where DrawButtons ran after Build. With
// --interpolate the world is drawn at alpha between the previous tick and
// this one; at alpha 1 that is exactly the faithful frame (--check-view).
static void PortRenderFrame(const PortFrame& frame, double alpha)
{
	RECT clip = g_pPixmap->GetClipping();
	if (frame.world && g_bInterpolate) blupi::scene::DrawView(frame.scene, &frame.motion, alpha, g_pPixmap);
	else if (frame.world) blupi::scene::FlattenScene(frame.scene, g_pPixmap);
	else PortPixmapSubmit(frame.background);
	PortPixmapSubmit(frame.ui);
	g_pPixmap->SetClipping(clip);
}

// Widescreen presentation (ROADMAP.md stage 8): the world phases (play and
// the editor) are drawn for the wider view around the frame; menus show the
// 640x480 frame centred. In play the frame, with the HUD and the UI, is
// centred; the editor anchors its frame, with its toolbar, to the view's left
// edge and shows the extra world on the right. Like PortPresentNow, it reads
// no game state and changes none.
static bool PortWideWorld(const PortFrame& frame)
{
	return frame.world;
}

#define PORT_EVENT_MEMBERS(X) X(m_index) X(SearchPhase) X(m_buttons) X(m_bDemoPlay) X(m_demoNumber)
BLUPI_MEMBER_ACCESS(PortEventAccess, CEvent, PORT_EVENT_MEMBERS)

// The editor in a wide view: its world is placed like the play view, its
// screen layer (toolbar, caption) sits at the view's left edge.
static bool PortUiLeft(const PortFrame& frame)
{
	return frame.world && frame.scene.phase == WM_PHASE_BUILD;
}

static blupi::scene::ViewSpec PortWideSpec(const PortFrame& frame, int width)
{
	blupi::scene::ViewSpec spec;
	spec.width = width;
	spec.uiLeft = PortUiLeft(frame);
	return spec;
}

// Where the screen layer lies in the presented view now; the host reports
// mouse positions relative to it.
static int PortFrameX(int width)
{
	if (width <= LXIMAGE) return 0;
	const bool left = g_frameValid && !g_bReference && PortUiLeft(g_frame);
	return blupi::scene::FrameOffset(width, left);
}

static bool PortWideEditor()
{
	return PortViewWidth() > LXIMAGE && g_frameValid && !g_bReference && PortUiLeft(g_frame) &&
		   g_pEvent != NULL && g_pEvent->GetPhase() == WM_PHASE_BUILD;
}

// The wide editor's scrolling stops where its view reaches a world edge (the
// view is clamped there like the play view), so the arrow keys never move the
// editor's camera without moving the view. Input only: the arrow key is
// released, and not passed on again, while the camera is past that point;
// CDecor::MoveStep is unchanged.
#define PORT_DECOR_MEMBERS(X) X(m_posDecor) X(m_dimDecor)
BLUPI_MEMBER_ACCESS(PortDecorAccess, CDecor, PORT_DECOR_MEMBERS)
static bool g_portArrowHeld[2] = {};   // left, right arrow keys as passed to the game

static bool PortEditorScrollBlocked(bool right)
{
	if (!PortWideEditor() || g_pDecor == NULL) return false;
	const int width = PortViewWidth(), centre = (width - LXIMAGE) / 2;
	const POINT& pos = g_pDecor->*PortDecorAccess::m_posDecor;
	const POINT& dim = g_pDecor->*PortDecorAccess::m_dimDecor;
	const int worldWidth = dim.x != 0 ? MAXCELX * DIMOBJX : LXIMAGE;
	if (worldWidth < width) return false;   // one screen wide: the game does not scroll
	return right ? pos.x >= worldWidth - width + centre : pos.x <= centre;
}

// The wide editor's mouse: a press that starts on one of the game's buttons
// (or an open button menu) goes to the game as it is; elsewhere the position
// is a place in the world, handed to the editor's tools in the frame's
// coordinates (game/editor_wide.cpp), while the game sees the mouse leave its
// buttons. The world frame's place in the view: PlaceView.
#define PORT_BUTTON_MEMBERS(X) X(m_pos) X(m_dim) X(m_bHide) X(m_nbMenu) X(m_bMouseDown)
BLUPI_MEMBER_ACCESS(PortButtonAccess, CButton, PORT_BUTTON_MEMBERS)
static int g_editorPress = 0;          // 0 none, 1 the game's buttons, 2 the world

static bool PortEditorOnButton(int x, int y)
{
	auto& buttons = g_pEvent->*PortEventAccess::m_buttons;
	for (CButton& b : buttons)
	{
		if (b.*PortButtonAccess::m_bHide) continue;
		const POINT& pos = b.*PortButtonAccess::m_pos;
		const POINT& dim = b.*PortButtonAccess::m_dim;
		if (dim.x <= 0) continue;   // not created in this phase
		int width = dim.x;
		if (b.*PortButtonAccess::m_nbMenu > 1 && b.*PortButtonAccess::m_bMouseDown)
			width += 2 + (dim.x - 1) * b.*PortButtonAccess::m_nbMenu;   // CButton::Detect
		if (x >= pos.x && x <= pos.x + width && y >= pos.y && y <= pos.y + dim.y) return true;
	}
	return false;
}

static void PortEditorMouse(UINT message, int keys, int x, int y)
{
	const bool down = message == WM_LBUTTONDOWN || message == WM_RBUTTONDOWN;
	const bool up = message == WM_LBUTTONUP || message == WM_RBUTTONUP;
	if (down && g_editorPress == 0) g_editorPress = PortEditorOnButton(x, y) ? 1 : 2;
	const bool buttons = g_editorPress == 1 || (g_editorPress == 0 && PortEditorOnButton(x, y));
	if (buttons)
	{
		WindowProc(message, (WPARAM)keys, MAKELPARAM(x, y));
	}
	else
	{
		// The game's buttons lose the mouse (hover, a press ending elsewhere).
		if (!down) WindowProc(up ? message : WM_MOUSEMOVE, 0, MAKELPARAM(-1, -1));
		const int worldX = blupi::scene::PlaceView(g_frame.scene, g_frame.scene.view, PortViewWidth()).frameX;
		PortEditorSideMouse(g_pEvent, g_pDecor, message, keys, POINT{x - worldX, y});
	}
	if (up) g_editorPress = 0;
}

// The mouse over a side area of a wide view (frame coordinates, outside the
// frame but inside the view).
static bool PortMouseInSideArea(int width, int x, int y)
{
	const int frameX = PortFrameX(width);
	return width > LXIMAGE && (x < 0 || x >= LXIMAGE) && x >= -frameX && x < width - frameX &&
		   y >= 0 && y < LYIMAGE;
}

static void PortPresentWide(double alpha, int width)
{
	const int frameX = PortFrameX(width);
	if (!g_bHeadless) blupi::host::set_frame_x(frameX);
	const bool world = g_frameValid && !g_bReference && PortWideWorld(g_frame);
	if (g_frameValid && !g_bReference && !world) PortRenderPresentation(alpha);   // the 640x480 frame
	if (!PortPixmapWideBegin(g_pPixmap)) return;
	if (world)
	{
		blupi::scene::DrawView(g_frame.scene, g_bInterpolate ? &g_frame.motion : NULL, alpha, g_pPixmap,
							   PortWideSpec(g_frame, width));
		PortPixmapWideSubmit(g_frame.ui, frameX);
	}
	else
	{
		PortPixmapWideBlitBack(frameX);
	}
	// The cursor follows the mouse over the side areas too, and everywhere in
	// the wide editor (whose mouse the game sees only over its buttons).
	POINT mouse;
	int mx = 0, my = 0;
	blupi::host::get_mouse(mx, my);
	mouse.x = mx;
	mouse.y = my;
	const bool side = !g_bHeadless && (PortMouseInSideArea(width, mx, my) || (world && PortUiLeft(g_frame)));
	PortPixmapWideCursor(g_pPixmap, frameX, side ? &mouse : NULL);
	PortPixmapWideEnd(g_pPixmap);
	PortPixmapWidePresent();
}

// World area the scene records around the frame: the interpolation margin,
// and with widescreen the extra width on either side (a view clamped at a
// world edge reaches that far beyond the frame). Automatic widescreen covers
// the widest view, so resizing the window never shows an uncovered edge.
static void PortUpdateCoverage()
{
	const int m = (g_bInterpolate || g_bCheckView) ? blupi::scene::kInterpolationMargin : 0;
	int w = 0;
	if (g_wideWidth > 0) w = g_wideWidth - LXIMAGE;
	else if (g_settings.widescreen && !g_bHeadless) w = blupi::host::kMaxViewWidth - LXIMAGE;
	g_sceneCoverage = blupi::scene::Coverage{m + w, m + w, m, m};
}

// The editor scroll pad of touch screens (platform/touch.h): game buttons with
// the button sheet's arrows, as the game's own screens use them: 0x32 and 0x33
// (left, right: the settings' volume buttons), 0x48 and 0x49 (up, down).
static void PortTouchEditorImages()
{
	static const int icons[4] = { 0x32, 0x33, 0x48, 0x49 };
	std::vector<uint32_t> released[4], pressed[4];
	for (int d = 0; d < 4; d++)
	{
		if (!PortPixmapButtonImage(g_pPixmap, icons[d], false, released[d]) ||
			!PortPixmapButtonImage(g_pPixmap, icons[d], true, pressed[d])) return;
	}
	blupi::host::set_touch_editor_images(released, pressed, DIMBUTTONX);
}

static void PortSaveSettings()
{
	if (g_settingsPath.empty()) return;
	if (!PortSettingsSave(g_settingsPath.c_str(), g_settings))
	{
		blupi::host::log_info(("cannot write " + g_settingsPath).c_str());
	}
}

// After a change on the "Display options" screen: applied at once and saved.
// A new language redraws the game's screen underneath: the original draws its
// texts when a phase starts, so the phase is entered again.
static std::string g_appliedLanguage;
static void PortApplySettings()
{
	if (g_settings.language != g_appliedLanguage)
	{
		g_appliedLanguage = g_settings.language;
		if (g_pEvent != NULL) g_pEvent->ChangePhase(g_pEvent->GetPhase());
	}
	g_presentDisplayRate = g_settings.displayRate;
	g_bInterpolate = g_settings.interpolate && g_settings.displayRate;
	if (!g_bHeadless)
	{
		if (g_settings.widescreen) g_wideWidth = 0;
		if (g_wideWidth == 0) blupi::host::set_auto_wide(g_settings.widescreen);
		blupi::host::set_fullscreen(g_settings.fullscreen);
		blupi::host::set_integer_scale(g_settings.integerScale);
	}
	PortUpdateCoverage();
	PortSaveSettings();
}

// --check-wide: where the wide view is centred on the frame (not clamped at a
// world edge), its centre 640x480 region equals the faithful frame when the
// items are chosen by Build's rules at the frame (ROADMAP.md stage 8 gate).
// Called with the faithful frame, without the cursor, in the back buffer.
static void PortCheckWide(const PortFrame& frame)
{
	if (!PortWideWorld(frame)) return;
	blupi::scene::ViewSpec spec = PortWideSpec(frame, g_wideWidth);
	spec.cullAtFrame = true;
	spec.uiAtFrame = true;   // the editor's toolbar compared at the frame, too
	const int frameX = blupi::scene::FrameOffset(g_wideWidth, false);
	if (blupi::scene::PlaceView(frame.scene, frame.scene.view, g_wideWidth).frameX != frameX)
	{
		g_wideSkipped++;
		return;
	}
	PortPixmapSetWide(g_wideWidth);
	if (!PortPixmapWideBegin(g_pPixmap)) return;
	blupi::scene::DrawView(frame.scene, NULL, 1.0, g_pPixmap, spec);
	PortPixmapWideSubmit(frame.ui, frameX);
	PortPixmapWideEnd(g_pPixmap);
	if (!PortPixmapWideRegionEqualsBack(frameX))
	{
		if (g_wideFirstMismatch < 0) g_wideFirstMismatch = g_wideFrames;
		g_wideMismatches++;
	}
	g_wideFrames++;
}

// --check-view: the view renderer at alpha 1, with and without the motion,
// issues exactly the faithful frame's commands (ROADMAP.md stage 7 gate).
static void PortCheckView(const PortFrame& frame)
{
	const uint64_t before = PortStateHash();
	std::vector<blupi::render::Command> flat, view, moved;
	RECT clip = g_pPixmap->GetClipping();
	PortPixmapCaptureBegin(false);
	blupi::scene::FlattenScene(frame.scene, g_pPixmap);
	PortPixmapCaptureEnd(flat);
	PortPixmapCaptureBegin(false);
	blupi::scene::DrawView(frame.scene, NULL, 1.0, g_pPixmap);
	PortPixmapCaptureEnd(view);
	PortPixmapCaptureBegin(false);
	blupi::scene::DrawView(frame.scene, &frame.motion, 1.0, g_pPixmap);
	PortPixmapCaptureEnd(moved);
	g_pPixmap->SetClipping(clip);

	auto same = [](const std::vector<blupi::render::Command>& a, const std::vector<blupi::render::Command>& b)
	{
		if (a.size() != b.size()) return false;
		for (size_t i = 0; i < a.size(); i++)
		{
			if (!blupi::render::same_command(a[i], b[i])) return false;
		}
		return true;
	};
	if (before != PortStateHash() || !same(flat, view) || !same(flat, moved))
	{
		if (g_viewFirstMismatch < 0) g_viewFirstMismatch = g_viewFrames;
		g_viewMismatches++;
	}
	g_viewFrames++;
}

static void PortFrameForCheck(const blupi::scene::Scene& scene)
{
	PortFrame frame;
	frame.world = true;
	frame.scene = scene;
	frame.motion.valid = true;
	frame.motion.from.resize(scene.items.size()); // deliberately ignored at alpha 1
	PortCheckView(frame);
}

void SetDecor()
{
	UINT phase;
	POINT posMouse;

	g_pPixmap->MouseBackClear();
	posMouse = g_pEvent->GetLastMousePos();

	phase = g_pEvent->GetPhase();
	PortOptionsPhase(phase);
	PortProbeApplyBeforeDraw(g_drawProbeFrame);

	if (g_bReference || g_bCheckScene)
	{
		// Historical body (src/blupi.cpp): drawing and its state changes together.
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
		return;
	}

	// Port: simulation side of the frame. Every state change happens here, in
	// the historical order; the drawing is kept as data in g_frame.
	std::swap(g_prevFrame, g_frame);    // the previous tick's frame, kept for interpolation
	const bool prevWorld = g_frameValid && g_prevFrame.world;
	if (phase == WM_PHASE_PLAY || phase == WM_PHASE_PLAYTEST || phase == WM_PHASE_BUILD)
	{
		RECT rect = { 0, 0, LXIMAGE, LYIMAGE };
		g_frame.world = true;
		const uint64_t before = g_bCheckView ? PortStateHash() : 0;
	blupi::scene::BuildScene(*g_pDecor, rect, posMouse, g_frame.scene, g_sceneCoverage);
	if (g_bCheckView && before != PortStateHash()) ++g_viewMismatches;
		EndOfTick(*g_pDecor);                       // CDecor::Build's state changes
		if (g_bInterpolate && prevWorld) blupi::scene::PairScenes(g_prevFrame.scene, g_frame.scene, g_frame.motion);
		else g_frame.motion = blupi::scene::Motion();
		if (g_bCheckView) PortCheckView(g_frame);
	}
	else
	{
		RECT rect = { 0, 0, LXIMAGE, LYIMAGE };
		g_frame.world = false;
		PortPixmapCaptureBegin(false);
		g_pPixmap->DrawImage(-1, 0, rect, 1);
		PortPixmapCaptureEnd(g_frame.background);
	}

	PortPixmapCaptureBegin(false);
	g_pEvent->DrawButtons();                        // its UI state changes happen now
	PortPixmapCaptureEnd(g_frame.ui);
	g_lastPhase = phase;
	g_frameValid = true;

	// Presentation side: the tick's own frame (alpha 1: the faithful frame).
	PortRenderFrame(g_frame, 1.0);
	for (int i = 0; i < g_renderRepeat; i++) PortRenderFrame(g_frame, 1.0);   // test: rendering changes nothing
	if (g_bCheckWide) PortCheckWide(g_frame);
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
	const UINT firstPhase = PortEdition() == "eggbert" ? WM_PHASE_INTRO1 : WM_PHASE_INIT;
	PortFirstPhaseIndex(g_pEvent, firstPhase);
	g_pEvent->ChangePhase(firstPhase);
#else
	PortFirstPhaseIndex(g_pEvent, WM_PHASE_INIT);
	g_pEvent->ChangePhase(WM_PHASE_INIT);
#endif
	g_bTermInit = TRUE;
	return TRUE;
}
// Host paths from here on; historical fopen above remains VFS-routed.
#undef fopen


// Game-specific bridge to the common launcher. Historical-derived bodies above
// retain their own headers, object order, message handling and presentation glue.
int BlupiMain(int argc, char** argv)
{
    using blupi::app::Options;
    blupi::app::Game game{};
    game.name = "blupi2-sdl3"; game.title = "Speedy Blupi 2 / Speedy Eggbert 2";
    game.dataTitle = game.title;
    game.dataEnvironment = "BLUPI2_DATA";
    game.settingsFile = "blupi2-sdl3.json"; game.userApp = "Speedy Blupi 2 SDL3";
    game.settings = &g_settings; game.settingsPath = &g_settingsPath;
    game.quit = &g_bQuit; game.timer = &g_timer; game.arrowHeld = g_portArrowHeld;
    game.recognizesData = [](const std::string& root) {
        return blupi::app::RecognizesData(root, "data/bench.blp");
    };

    extern const unsigned char g_sb2IconPng[];
    extern const unsigned g_sb2IconPngSize;
    game.icon = g_sb2IconPng; game.iconSize = g_sb2IconPngSize;
    std::string demoLogPath;
    FILE* demoLog = nullptr;
    int lastDemo = -1;
    game.extraUsage = "  --demo-log FILE     record attract-mode demo transitions\n"
                      "  --check-presentation compare state and primary pixels across presents\n";
    game.option = [&](const std::string& a, int& i, int argc, char** argv) {
        if (a == "--check-presentation") { g_bCheckPresentation = true; return 1; }
        if (a != "--demo-log") return 0;
        if (i + 1 >= argc) return -1;
        demoLogPath = argv[++i]; return 1;
    };
    // The SB2 harness has always required a finite unattended run.
    game.validateOptions = [](const Options& o) { return !o.host.headless || o.ticks >= 0; };

    game.configure = [](const Options& o) {
        g_bHeadless = o.host.headless;
        g_portColorDepth = o.colorDepth; g_wideWidth = o.wideWidth;
        g_bReference = o.reference; g_renderRepeat = o.renderRepeat;
        g_bInterpolate = g_settings.interpolate && g_settings.displayRate;
        g_presentDisplayRate = g_settings.displayRate;
        g_bCheckScene = o.checkScene; g_bCheckView = o.checkView;
        g_bCheckWide = o.checkWide && g_wideWidth > 0;
        PortUpdateCoverage();
    };
    game.initialize = [&](const Options& o) {
        // SB2 already remembers its effective root when Display options save.
        g_settings.dataFolder = blupi::vfs::game_root();
        PortLanguageInit(g_settings.language);
        g_appliedLanguage = g_settings.language;
        if (!o.probe.empty()) {
            FILE* f = fopen(o.probe.c_str(), "r");
            bool ok = f && PortProbeLoad(f);
            if (f) fclose(f);
            if (!ok) return 2;
        }
        if (!demoLogPath.empty()) { demoLog = fopen(demoLogPath.c_str(), "w"); if (!demoLog) return 2; }
        g_hWnd = nullptr; g_bActive = TRUE;
        if (o.checkCommands) PortPixmapEnableCommandCheck();
        if (!DoInit(NULL, (LPSTR)"", 0)) return 1;
        PortOptionsInit(g_pPixmap, g_pSound, &g_settings, PortApplySettings,
                        !blupi::host::touch_screen(), 70, 411);
        PortPixmapHideCursor(blupi::host::touch_screen());
        Benchmark();
        return 0;
    };
    game.finish = [&]() { FinishObjects(); if (demoLog) { fclose(demoLog); demoLog = nullptr; } };
    game.timerStep = TimerStep; game.timerInterval = []() { return g_timerInterval; };
    game.soundPoll = PortSoundPoll; game.saveSettings = PortSaveSettings;
    game.prepareTick = [](long frame) { g_drawProbeFrame = frame; PortProbeApply(frame); };
    game.completedTick = [&](long frame) {
        int demo = g_pEvent->*PortEventAccess::m_bDemoPlay ? 300 + g_pEvent->*PortEventAccess::m_demoNumber : -1;
        if (demoLog && demo != lastDemo) fprintf(demoLog, "%ld %d\n", frame, demo);
        lastDemo = demo;
    };
    game.maintenance = []() {

    };
    game.phase = []() { return g_pEvent ? g_pEvent->GetPhase() : 0u; };
    game.stateHash = PortStateHash; game.screenHash = PortPixmapScreenHash;
    game.stateDump = PortStateDump; game.screenshot = PortPixmapWriteScreenBmp;
    game.present = PortPresentNow; game.presentIfDirty = PortPixmapPresentIfDirty;
    game.viewWidth = PortViewWidth; game.displayCapture = PortPixmapSetDisplayCapture;
    game.windowProc = WindowProc;

    game.editorScrollBlocked = PortEditorScrollBlocked;
    game.wideEditor = PortWideEditor; game.editorMouse = PortEditorMouse;
    game.mouseInSideArea = PortMouseInSideArea;
    game.checkResult = [](const Options& o) {
        int result = 0;
        long displays, mismatches, first;
        if (PortPixmapCommandCheckResult(displays, mismatches, first)) {
            printf("SB2 command check: %ld displays, %ld mismatches (first %ld)\n", displays, mismatches, first);
            if (!displays || mismatches) result = 3;
        }
        if (o.checkScene)
        {
            printf("scene check: %ld Build calls compared, %ld mismatches (first at call %ld%s%s)\n",
                   g_sceneFrames, g_sceneMismatches, g_sceneFirstMismatch,
                   g_sceneFirstDetail.empty() ? "" : ": ", g_sceneFirstDetail.c_str());
            if (g_sceneMismatches != 0 || g_sceneFrames == 0) result = 4;
        }
        if (o.checkView)
        {
            printf("view check: %ld frames compared, %ld mismatches (first at frame %ld)\n",
                   g_viewFrames, g_viewMismatches, g_viewFirstMismatch);
            if (g_viewMismatches != 0 || g_viewFrames == 0) result = 5;
        }
        if (g_bCheckWide)
        {
            printf("wide check: %ld frames compared, %ld clamped frames skipped, %ld mismatches (first at frame %ld)\n",
                   g_wideFrames, g_wideSkipped, g_wideMismatches, g_wideFirstMismatch);
            if (g_wideMismatches != 0 || g_wideFrames == 0) result = 6;
        }
        if (g_bCheckPresentation || g_bCheckView)
        {
            printf("presentation check: %ld presents, %ld state/primary mismatches\n",
                   g_presentationFrames, g_presentationMismatches);
            if (g_presentationMismatches != 0 || (g_bCheckPresentation && g_presentationFrames == 0)) result = 7;
        }

        return result;
    };
    return blupi::app::Run(argc, argv, game);
}
