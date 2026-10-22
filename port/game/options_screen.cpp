// "Display options" screen (port/docs/ROADMAP.md stage 9): the port's own
// settings (game/settings.h), reached from a button the port adds to the
// game's settings screen. It is a presentation-layer screen: drawn over the
// tick's frame by the presentation (never into the game's own frame), with
// the game's button class, sprites, fonts and click sound, so it looks and
// behaves like the original screens. While it is open the game's settings
// screen gets no clicks or keys; the game itself keeps running untouched.

#include "def.h"
#include "pixmap.h"
#include "sound.h"
#include "decor.h"
#include "button.h"
#include "text.h"
#include "misc.h"

#include "game/port_game.h"
#include "game/settings.h"

#include <cstring>

namespace
{
// Messages of the port's buttons, posted like the game's (WM_BUTTONn) but
// handled by PortOptionsCommand before the window procedure.
enum : UINT
{
	MSG_OPEN = WM_USER + 0x7F00,
	MSG_CLOSE,
	MSG_WIDE_PREV, MSG_WIDE_NEXT,
	MSG_SMOOTH_PREV, MSG_SMOOTH_NEXT,
	MSG_RATE_PREV, MSG_RATE_NEXT,
	MSG_WINDOW, MSG_FULLSCREEN,
	MSG_FIT, MSG_WHOLE,
	MSG_LAST,
};

// Button sprites (CHBUTTON icon numbers as in the phase table of src/event.cpp).
constexpr int kIconLeft = 0x32, kIconRight = 0x33, kIconOk = 0x12, kIconDisplay = 4;

struct Control
{
	UINT message;
	int  icon;      // -1: plain square button (like the input-device choices)
	int  x, y;
};

// Layout on the settings screen background (setup.blp): three panels on the
// left with arrow pairs like the volume settings, choice buttons on the right
// like the input devices, the confirm button where the game has its own.
const Control kControls[] =
{
	{ MSG_WIDE_PREV,   kIconLeft,  70, 110 }, { MSG_WIDE_NEXT,   kIconRight, 110, 110 },
	{ MSG_SMOOTH_PREV, kIconLeft,  70, 220 }, { MSG_SMOOTH_NEXT, kIconRight, 110, 220 },
	{ MSG_RATE_PREV,   kIconLeft,  70, 330 }, { MSG_RATE_NEXT,   kIconRight, 110, 330 },
	{ MSG_WINDOW,      -1,        370, 110 }, { MSG_FULLSCREEN,  -1,         370, 150 },
	{ MSG_FIT,         -1,        370, 240 }, { MSG_WHOLE,       -1,         370, 280 },
	{ MSG_CLOSE,       kIconOk,   541, 411 },
};
constexpr int kControlCount = sizeof(kControls) / sizeof(kControls[0]);

// The entry on the game's settings screen: the empty lower-left panel.
constexpr int kEntryX = 70, kEntryY = 330;

CPixmap*      s_pixmap = NULL;
CSound*       s_sound = NULL;
PortSettings* s_settings = NULL;
void        (*s_apply)() = NULL;
bool          s_open = false;
bool          s_windowChoice = true;   // the Window / Full screen choice is offered
bool          s_setup = false;   // the game's settings screen is up
CButton       s_entry;
CButton       s_buttons[kControlCount];

CButton* Find(UINT message)
{
	for (int i = 0; i < kControlCount; i++)
	{
		if (kControls[i].message == message) return &s_buttons[i];
	}
	return NULL;
}

void Text(int x, int y, const char* text, int font)
{
	char buffer[100];
	snprintf(buffer, sizeof(buffer), "%s", text);
	DrawTextLeft(s_pixmap, POINT{x, y}, buffer, font);
}

// Enables and selects the buttons for the current settings.
void Refresh()
{
	const PortSettings& s = *s_settings;
	Find(MSG_WIDE_PREV)->SetEnable(s.widescreen);
	Find(MSG_WIDE_NEXT)->SetEnable(!s.widescreen);
	Find(MSG_SMOOTH_PREV)->SetEnable(s.displayRate && s.interpolate);
	Find(MSG_SMOOTH_NEXT)->SetEnable(s.displayRate && !s.interpolate);
	Find(MSG_RATE_PREV)->SetEnable(s.displayRate);
	Find(MSG_RATE_NEXT)->SetEnable(!s.displayRate);
	Find(MSG_WINDOW)->SetState(s.fullscreen ? 0 : 1);
	Find(MSG_FULLSCREEN)->SetState(s.fullscreen ? 1 : 0);
	Find(MSG_WINDOW)->SetHide(!s_windowChoice);
	Find(MSG_FULLSCREEN)->SetHide(!s_windowChoice);
	Find(MSG_FIT)->SetState(s.integerScale ? 0 : 1);
	Find(MSG_WHOLE)->SetState(s.integerScale ? 1 : 0);
}

// The game's cursor rule (CEvent::MousePosToSprite): the pointer over a
// button, the arrow elsewhere.
void Sprite(POINT pos, bool overButton)
{
	int sprite = overButton ? SPRITE_POINTER : SPRITE_ARROW;
	s_pixmap->SetMousePosSprite(pos, sprite, FALSE);
	ChangeSprite(sprite);
}

void Changed()
{
	Refresh();
	if (s_apply) s_apply();
}
}  // namespace

void PortOptionsInit(CPixmap* pixmap, CSound* sound, PortSettings* settings, void (*apply)(),
					 bool windowChoice)
{
	s_windowChoice = windowChoice;
	s_pixmap = pixmap;
	s_sound = sound;
	s_settings = settings;
	s_apply = apply;
	s_entry.Create(NULL, pixmap, sound, POINT{kEntryX, kEntryY}, 0, FALSE, MSG_OPEN);
	int icon = kIconDisplay;
	s_entry.SetIconMenu(&icon, 1);
	for (int i = 0; i < kControlCount; i++)
	{
		s_buttons[i].Create(NULL, pixmap, sound, POINT{kControls[i].x, kControls[i].y}, 0, FALSE, kControls[i].message);
		if (kControls[i].icon >= 0)
		{
			icon = kControls[i].icon;
			s_buttons[i].SetIconMenu(&icon, 1);
		}
	}
	Refresh();
}

void PortOptionsPhase(UINT phase)
{
	s_setup = phase == WM_PHASE_SETUP || phase == WM_PHASE_SETUPp;
	if (!s_setup) s_open = false;
}

bool PortOptionsOpen()
{
	return s_open;
}

bool PortOptionsVisible()
{
	return s_pixmap != NULL && s_setup;
}

bool PortOptionsMouse(UINT message, WPARAM wParam, LPARAM lParam)
{
	if (!PortOptionsVisible()) return false;
	if (s_open)
	{
		// Every button gets every event (CButton needs the button-up to
		// release). The game's screen underneath gets none, so its own
		// buttons keep their state; the cursor is moved here instead.
		const POINT pos = ConvLongToPos(lParam);
		bool over = false;
		for (CButton& b : s_buttons)
		{
			b.TreatEvent(message, wParam, lParam);
			if (b.MouseOnButton(pos)) over = true;
		}
		Sprite(pos, over);
		return true;
	}
	BOOL used = s_entry.TreatEvent(message, wParam, lParam);
	return used && (message == WM_LBUTTONDOWN || message == WM_RBUTTONDOWN);
}

void PortOptionsHover(LPARAM lParam)
{
	if (!PortOptionsVisible() || s_open) return;
	const POINT pos = ConvLongToPos(lParam);
	if (s_entry.MouseOnButton(pos)) Sprite(pos, true);
}

bool PortOptionsKey(UINT message, WPARAM wParam)
{
	if (!s_open) return false;
	if (message == WM_KEYDOWN && (wParam == VK_ESCAPE || wParam == VK_RETURN))
	{
		s_open = false;
	}
	return true;   // no keys for the game's screen underneath
}

bool PortOptionsCommand(UINT message)
{
	if (message < MSG_OPEN || message >= MSG_LAST) return false;
	if (s_settings == NULL) return true;
	PortSettings& s = *s_settings;
	switch (message)
	{
	case MSG_OPEN:        s_open = s_setup; Refresh(); return true;
	case MSG_CLOSE:       s_open = false; return true;
	case MSG_WIDE_PREV:   s.widescreen = false; break;
	case MSG_WIDE_NEXT:   s.widescreen = true; break;
	case MSG_SMOOTH_PREV: s.interpolate = false; break;
	case MSG_SMOOTH_NEXT: s.interpolate = true; break;
	case MSG_RATE_PREV:   s.displayRate = false; break;
	case MSG_RATE_NEXT:   s.displayRate = true; break;
	case MSG_WINDOW:      s.fullscreen = false; break;
	case MSG_FULLSCREEN:  s.fullscreen = true; break;
	case MSG_FIT:         s.integerScale = false; break;
	case MSG_WHOLE:       s.integerScale = true; break;
	}
	Changed();
	return true;
}

void PortOptionsDraw()
{
	if (!PortOptionsVisible()) return;
	if (!s_open)
	{
		s_entry.Draw();
		Text(kEntryX + 50, kEntryY + 12, "Display options", FONTWHITE);
		return;
	}

	const PortSettings& s = *s_settings;
	char title[] = "Display options";
	Text(LXIMAGE / 2 - GetTextWidth(title, 0) / 2, 36, title, FONTGOLD);

	Text(70, 80, "Widescreen", FONTWHITE);
	Text(160, 122, s.widescreen ? "Window shape" : "Off (4:3)", FONTWHITE);
	Text(70, 190, "Smooth motion", FONTWHITE);
	Text(160, 232, !s.displayRate ? "-" : s.interpolate ? "On" : "Off", FONTWHITE);
	Text(70, 300, "Frame rate", FONTWHITE);
	Text(160, 342, s.displayRate ? "Monitor" : "Original", FONTWHITE);

	if (s_windowChoice)
	{
		Text(370, 80, "Display", FONTWHITE);
		Text(420, 122, "Window", FONTWHITE);
		Text(420, 162, "Full screen", FONTWHITE);
	}
	Text(370, 210, "Scaling", FONTWHITE);
	Text(420, 252, "Fit the window", FONTWHITE);
	Text(420, 292, "Whole pixels", FONTWHITE);
	Text(70, 440, "SDL3 port " BLUPI_PORT_VERSION, FONTLITTLE);

	for (CButton& b : s_buttons) b.Draw();
}

void PortOptionsSync()
{
	if (s_settings != NULL) Refresh();
}
