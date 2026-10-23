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

namespace {
struct Input { long frame; std::string kind; int x = 0, y = 0; };
struct Options {
    blupi::host::Options host;
    std::string data, user, trace, soundfont, demoLog, probe;
    long frames = -1, every = 1;
    std::vector<Input> inputs;
    std::vector<std::pair<long, std::string>> shots, dumps;
    bool commands = false, reference = false, checkScene = false, checkView = false, checkWide = false;
    PortSettings settings;
    bool setWide = false, setInterpolate = false, setPresent = false, setFullscreen = false;
    bool setScale = false, setIntegerScale = false, setLanguage = false;
    int presentRepeat = 0;
    double screenshotAlpha = -1.0;
};
void Usage() {
    puts("blupi2 " BLUPI_PORT_VERSION "\n"
         "  --data DIR --user DIR --headless --frames N --color 8|16\n"
         "  --trace FILE --trace-every N --screenshot N:FILE --state-dump N:FILE\n"
         "  --input FILE --probe FILE --soundfont FILE --language CODE --check-commands --demo-log FILE\n"
         "  --reference         historical Build (test oracle)\n"
         "  --check-scene       compare const scene commands with historical Build\n"
         "  --check-view        compare DrawView at alpha 1 with FlattenScene\n"
         "  --check-presentation compare game state and faithful primary across presents\n"
         "  --wide off|auto|W|16:9 --interpolate --present tick|display --integer-scale\n"
         "  --render-repeat N --present-repeat N --check-wide --screenshot-alpha A\n"
         "  --fullscreen --windowed --scale N --verbose --version\n"
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
        else if (a == "--probe") o.probe = next();
        else if (a == "--trace-every") { o.every = atol(next()); if (o.every < 1) return false; }
        else if (a == "--soundfont") o.soundfont = next();
        else if (a == "--language") { o.settings.language = next(); o.setLanguage = true; }
        else if (a == "--fullscreen") { o.settings.fullscreen = true; o.setFullscreen = true; }
        else if (a == "--windowed") { o.settings.fullscreen = false; o.setFullscreen = true; }
        else if (a == "--integer-scale") { o.settings.integerScale = true; o.setIntegerScale = true; }
        else if (a == "--scale") { o.settings.scale = atoi(next()); o.setScale = true; if (o.settings.scale < 1 || o.settings.scale > 8) return false; }
        else if (a == "--verbose") blupi::host::set_verbose(true);
        else if (a == "--check-commands") o.commands = true;
        else if (a == "--reference") o.reference = true;
        else if (a == "--check-scene") o.checkScene = true;
        else if (a == "--check-view") o.checkView = true;
        else if (a == "--check-presentation") g_bCheckPresentation = true;
        else if (a == "--screenshot-alpha") o.screenshotAlpha = atof(next());
		else if (a == "--interpolate") { o.settings.interpolate = true; o.setInterpolate = true; }
		else if (a == "--check-wide") o.checkWide = true;
		else if (a == "--wide")
		{
			std::string v = next();
			o.setWide = true;
			if (v == "auto" || v == "off")
			{
				o.settings.widescreen = v == "auto";
				g_wideWidth = 0;
				continue;
			}
			o.settings.widescreen = false;
			size_t colon = v.find(':');
			if (colon != std::string::npos && atoi(v.c_str() + colon + 1) <= 0) return false;
			int width = colon == std::string::npos ? atoi(v.c_str())
						: (LYIMAGE * atoi(v.c_str()) + atoi(v.c_str() + colon + 1) - 1) / atoi(v.c_str() + colon + 1);
			width = (width + 1) & ~1;   // even: the frame is centred on whole pixels
			if (width < LXIMAGE || width > 4 * LXIMAGE) return false;
			g_wideWidth = width == LXIMAGE ? 0 : width;
		}
		else if (a == "--render-repeat") g_renderRepeat = atoi(next());
		else if (a == "--present")
		{
			std::string mode = next();
			if (mode == "tick") o.settings.displayRate = false;
			else if (mode == "display") o.settings.displayRate = true;
			else return false;
			o.setPresent = true;
		}
		else if (a == "--present-repeat") o.presentRepeat = atoi(next());
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
                if (in.kind != "key" && in.kind != "keydown" && in.kind != "keyup" && in.kind != "click" && in.kind != "move" &&
                    in.kind != "viewclick" && in.kind != "viewmove" && in.kind != "viewrightclick") { fclose(f); return false; }
                o.inputs.push_back(in);
            }
            fclose(f);
        } else return false;
    }
    // An unattended run always has a finite end, including on bad options/data.
    return (!o.host.headless || o.frames >= 0) && o.presentRepeat >= 0 && g_renderRepeat >= 0;
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
		if (PortOptionsKey(WM_KEYDOWN, (WPARAM)e.key)) break;
		if (e.key == VK_LEFT || e.key == VK_RIGHT)
		{
			const bool right = e.key == VK_RIGHT;
			if (PortEditorScrollBlocked(right)) break;
			g_portArrowHeld[right] = true;
		}
		WindowProc(WM_KEYDOWN, (WPARAM)e.key, (LPARAM)(1 | (e.repeat ? (1 << 30) : 0)));
		// TranslateMessage: control characters that become WM_CHAR.
		if (e.key == VK_BACK || e.key == VK_RETURN || e.key == VK_ESCAPE || e.key == VK_TAB)
			WindowProc(WM_CHAR, (WPARAM)e.key, 1);
		break;
	case EventType::KeyUp:
		if (PortOptionsKey(WM_KEYUP, (WPARAM)e.key)) break;
		if (e.key == VK_LEFT || e.key == VK_RIGHT)
		{
			const bool right = e.key == VK_RIGHT;
			if (!g_portArrowHeld[right]) break;   // already released at the editor's edge
			g_portArrowHeld[right] = false;
		}
		WindowProc(WM_KEYUP, (WPARAM)e.key, (LPARAM)(1 | (1 << 30) | (1u << 31)));
		break;
	case EventType::Text:
		if (PortOptionsKey(WM_CHAR, (WPARAM)e.ch)) break;
		WindowProc(WM_CHAR, (WPARAM)e.ch, 1);
		break;
	case EventType::MouseMove:
	case EventType::MouseDown:
	case EventType::MouseUp:
	{
		UINT message = WM_MOUSEMOVE;
		if (e.type == EventType::MouseDown) message = e.button == 1 ? WM_LBUTTONDOWN : WM_RBUTTONDOWN;
		if (e.type == EventType::MouseUp) message = e.button == 1 ? WM_LBUTTONUP : WM_RBUTTONUP;
		if (PortWideEditor() && !PortOptionsOpen() && e.x >= 0 && e.x < PortViewWidth() && e.y >= 0 && e.y < LYIMAGE)
		{
			PortEditorMouse(message, (int)MouseKeys(e), e.x, e.y);
			break;
		}
		if (!e.inside)
		{
			// Outside the game's window (the side areas of a wide view in play,
			// the letterbox): as before.
			if (!PortMouseInSideArea(PortViewWidth(), e.x, e.y) && message == WM_MOUSEMOVE)
			{
				WindowProc(WM_NCMOUSEMOVE, 0, 0);
			}
			break;
		}
		if (PortOptionsMouse(message, MouseKeys(e), MousePos(e.x, e.y))) break;
		WindowProc(message, MouseKeys(e), MousePos(e.x, e.y));
		if (message == WM_MOUSEMOVE) PortOptionsHover(MousePos(e.x, e.y));
		break;
	}
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
		if (PortOptionsCommand(msg.message)) continue;
		if ((msg.message == WM_KEYDOWN || msg.message == WM_KEYUP || msg.message == WM_CHAR) && PortOptionsKey(msg.message, msg.wParam)) continue;   // the port's own buttons
		// Scripted mouse input meets the port's screen like live input.
		if ((msg.message == WM_MOUSEMOVE || msg.message == WM_LBUTTONDOWN || msg.message == WM_LBUTTONUP ||
			 msg.message == WM_RBUTTONDOWN || msg.message == WM_RBUTTONUP) &&
			PortOptionsMouse(msg.message, msg.wParam, msg.lParam)) continue;
		WindowProc(msg.message, msg.wParam, msg.lParam);
		int ch = msg.message == WM_KEYDOWN ? TranslatedChar(msg.wParam) : 0;
		if (ch != 0 && !g_bQuit) WindowProc(WM_CHAR, (WPARAM)ch, msg.lParam);
	}
}

void PostInput(const Options& o, long frame) {
    for (const Input& in : o.inputs) {
        if (in.frame != frame) continue;
        // Presentation-coordinate editor smoke checks use the same route as
        // live host input; ordinary scripts keep their historical coordinates.
        if (in.kind == "viewclick" || in.kind == "viewmove" || in.kind == "viewrightclick") {
            blupi::host::Event e{};
            e.x = in.x; e.y = in.y;
            e.inside = in.x >= 0 && in.x < LXIMAGE && in.y >= 0 && in.y < LYIMAGE;
            e.type = blupi::host::EventType::MouseMove;
            TranslateHostEvent(e);
            if (in.kind != "viewmove") {
                e.button = in.kind == "viewrightclick" ? 3 : 1;
                e.type = blupi::host::EventType::MouseDown;
                e.leftDown = e.button == 1; e.rightDown = e.button != 1;
                TranslateHostEvent(e);
                e.type = blupi::host::EventType::MouseUp;
                e.leftDown = e.rightDown = false;
                TranslateHostEvent(e);
            }
            continue;
        }
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
    if (!o.probe.empty()) {
        FILE* file = fopen(o.probe.c_str(), "r");
        bool ok = file && PortProbeLoad(file);
        if (file) fclose(file);
        if (!ok) return 2;
    }
	// Port settings (game/settings.h): the file, then this run's options.
	// Headless runs use defaults and options only, and save nothing.
	g_bHeadless = o.host.headless;
	{
		PortSettings fromFile;
		if (!g_bHeadless)
		{
			std::string user = o.user.empty() ? blupi::host::pref_path("Epsitec", "Speedy Blupi 2 SDL3") : o.user + "/";
			g_settingsPath = user + "blupi2-sdl3.json";
			PortSettingsLoad(g_settingsPath.c_str(), fromFile);
		}
		PortSettings& s = o.settings;
		if (!o.setWide) s.widescreen = fromFile.widescreen;
		if (!o.setInterpolate) s.interpolate = fromFile.interpolate;
		if (!o.setPresent) s.displayRate = fromFile.displayRate;
		if (!o.setFullscreen) s.fullscreen = fromFile.fullscreen;
		if (!o.setScale) s.scale = fromFile.scale;
		if (!o.setIntegerScale) s.integerScale = fromFile.integerScale;
		s.dataFolder = fromFile.dataFolder;
		if (!o.setLanguage) s.language = fromFile.language;
		g_settings = s;
	}
	o.host.fullscreen = g_settings.fullscreen;
	o.host.scale = g_settings.scale;
	o.host.integerScale = g_settings.integerScale;
	o.host.autoWide = g_settings.widescreen && g_wideWidth == 0;
	o.host.viewWidth = g_wideWidth;
	g_bInterpolate = g_settings.interpolate && g_settings.displayRate;
	g_presentDisplayRate = g_settings.displayRate;
    if (!blupi::host::init("Speedy Blupi 2 / Speedy Eggbert 2", LXIMAGE, LYIMAGE, o.host)) return 1;
    if (o.data.empty()) { if (const char* p = getenv("BLUPI2_DATA")) o.data = p; }
    if (o.data.empty()) o.data = g_settings.dataFolder;
    if (o.data.empty()) { blupi::host::message_box("Blupi 2", "Pass --data <SB2 game folder> or set BLUPI2_DATA."); blupi::host::shutdown(); return 1; }
    if (o.user.empty()) o.user = blupi::host::pref_path("Epsitec", "Speedy Blupi 2 SDL3");
    blupi::vfs::set_roots(o.data, o.user);
    if (blupi::vfs::resolve_read("data/config.def").empty() || blupi::vfs::resolve_read("image08/init.blp").empty()) {
        blupi::host::message_box("Blupi 2", "SB2 game data not found."); blupi::host::shutdown(); return 1;
    }
    g_settings.dataFolder = o.data;
    PortLanguageInit(g_settings.language);
    g_appliedLanguage = g_settings.language;
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
    g_bReference = o.reference;
    g_bCheckScene = o.checkScene;
    g_bCheckView = o.checkView;
    g_bCheckWide = o.checkWide && g_wideWidth > 0;
    PortUpdateCoverage();
    if (o.commands) PortPixmapEnableCommandCheck();
    int result = 0;
    if (!DoInit(NULL, (LPSTR)"", 0)) result = 1;
    else {
        PortOptionsInit(g_pPixmap, g_pSound, &g_settings, PortApplySettings, !blupi::host::touch_screen(), 70, 411);
        PortPixmapHideCursor(blupi::host::touch_screen());
        Benchmark();
        const uint64_t interval = (uint64_t)g_timerInterval*1000000;
        uint64_t next = blupi::host::now_ns()+interval;
        long frame = 0;
        int lastDemo = -1;
        uint64_t lastTick = blupi::host::now_ns();
        PortPerf perf;
        while (!g_bQuit && (o.frames < 0 || frame < o.frames)) {
            blupi::host::Event e;
            while (!g_bQuit && blupi::host::poll_event(e)) TranslateHostEvent(e);
            if (g_bQuit) break;
            for (int right = 0; right < 2; ++right) {
                if (g_portArrowHeld[right] && PortEditorScrollBlocked(right != 0)) {
                    g_portArrowHeld[right] = false;
                    WindowProc(WM_KEYUP, right ? VK_RIGHT : VK_LEFT, (LPARAM)(1 | (1 << 30) | (1u << 31)));
                }
            }
            PortSoundPoll(); DispatchPosted();
            if (g_bQuit) break;
            uint64_t now = blupi::host::now_ns();
            if (o.host.headless || now >= next) {
                g_drawProbeFrame = frame;
                PortProbeApply(frame);
                PostInput(o, frame); DispatchPosted();
                if (g_bQuit) break;
                const uint64_t tickStart = blupi::host::now_ns();
                TimerStep(); perf.Tick(blupi::host::now_ns() - tickStart); lastTick = now; int pending = g_timer; DispatchPosted();
                if (g_bQuit) break;
                next = PortNextTick(next, now, interval);
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
                    for (const auto& p : o.shots) if (p.first == frame) {
                        PortPresentNow(o.screenshotAlpha >= 0.0 ? o.screenshotAlpha : 1.0);
                        if (!PortPixmapWriteScreenBmp(p.second.c_str())) result = 2;
                    }
                    for (const auto& p : o.dumps) if (p.first == frame) {
                        FILE* f = fopen(p.second.c_str(), "w"); if (f) { PortStateDump(f); fclose(f); } else result = 2;
                    }
                    for (int i = 0; i < o.presentRepeat; ++i) PortPresentNow(g_bInterpolate ? (i + 1.0) / o.presentRepeat : 1.0);
                    if (!g_presentDisplayRate && !o.host.headless) PortPresentNow(1.0);
                }
            }
            if (!o.host.headless && blupi::host::fullscreen() != g_settings.fullscreen) {
                g_settings.fullscreen = blupi::host::fullscreen(); PortOptionsSync(); PortSaveSettings();
            }
            if (g_presentDisplayRate && !o.host.headless) {
                const uint64_t t0 = blupi::host::now_ns();
                PortPresentNow(PortFrameAlpha(t0, lastTick, interval));
                const uint64_t t1 = blupi::host::now_ns();
                PortPixmapPresentIfDirty();
                const uint64_t t2 = blupi::host::now_ns();
                perf.Present(t1 - t0, t2 - t1, t2, frame, PortViewWidth());
                if (!blupi::host::presents_with_vsync()) {
                    now = blupi::host::now_ns();
                    uint64_t pause = 4000000;
                    if (now < next && next - now < pause) pause = next - now;
                    if (now < next && blupi::host::wait_event(e, pause)) TranslateHostEvent(e);
                }
                continue;
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
    FinishObjects();
    blupi::audio::shutdown(); blupi::music::shutdown(); blupi::host::shutdown();
    return result;
}

