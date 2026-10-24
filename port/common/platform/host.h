// SDL3 host: window, presentation of the 640x480 logical frame, input and
// time. Free of Win32 names; common/app/launcher.cpp turns host events into the
// window messages the historical CEvent code expects.
//
// Keys are reported with Win32 virtual-key numbering because that is the
// game's own input vocabulary (CEvent::TreatEvent switches on it); the mapping
// table from SDL keycodes lives in host.cpp.
#ifndef BLUPI_PLATFORM_HOST_H
#define BLUPI_PLATFORM_HOST_H

#include <cstdint>
#include <string>
#include <vector>

namespace blupi::host
{
struct Options
{
    bool headless = false;    // no window, no audio device, no waiting (automated tests)
    bool gamepads = false;    // headless only: gamepad events anyway (tests)
    bool fullscreen = false;  // borderless desktop fullscreen, letterboxed
    int  scale = 2;           // initial window size = logical size * scale
    bool integerScale = false;
    int  viewWidth = 0;       // widescreen: presented width (> logical width); the game's
                              // frame is centred in it and mouse positions stay frame-relative
    bool autoWide = false;    // widescreen: view width from the window shape (4:3 to 21:9)
    const unsigned char* iconPng = nullptr;  // the game's window icon (PNG bytes, kept alive
    unsigned iconPngSize = 0;                // by the game); nullptr: the built-in res/icon.png
};

// Widest automatic view: 21:9 at the frame height of 480.
constexpr int kMaxViewWidth = 1120;

enum class EventType
{
    Quit,
    FocusGained,
    FocusLost,
    KeyDown,      // key = virtual-key code, repeat set for auto-repeat
    KeyUp,
    Text,         // ch = one cp1252 character (from SDL text input)
    MouseMove,    // x, y logical; inside = within the 640x480 frame
    MouseDown,    // button: 1 left, 2 right
    MouseUp,
};

struct Event
{
    EventType type;
    int  key = 0;
    bool repeat = false;
    int  ch = 0;
    int  x = 0, y = 0;
    bool inside = true;
    int  button = 0;
    bool shift = false, control = false;
    bool leftDown = false, rightDown = false;  // button state after the event
};

bool init(const char* title, int logicalWidth, int logicalHeight, const Options& options);
void shutdown();
bool headless();

// Wait at most timeoutNs for an event; returns false on timeout.
bool wait_event(Event& event, uint64_t timeoutNs);
bool poll_event(Event& event);
uint64_t now_ns();

// Presents a full logical frame (0xAARRGGBB, logical size, tightly packed).
// width x logical height; ignored unless width is the current view width.
void present(const uint32_t* argb, int width);
// True when present() waits for the display refresh (vsync).
bool presents_with_vsync();
void toggle_fullscreen();
void set_fullscreen(bool on);
bool fullscreen();
void set_integer_scale(bool on);
// Widescreen: automatic (window shape), or a fixed view width (the logical
// width turns it off). view_width() is the width present() expects now.
void set_auto_wide(bool on);
void set_view_width(int width);
int  view_width();
// Gamepads (SDL gamepad API), numbered in the order they were connected;
// none in headless runs. Axes -32768..32767 (left stick, the d-pad overrides
// it); buttons: bit 0 south (A), 1 east (B), 2 west (X), 3 north (Y). Start
// arrives as the Escape key.
int  gamepad_count();
bool gamepad_name(int index, char* name, int size);   // in the game's code page
bool gamepad_state(int index, int& x, int& y, unsigned& buttons);

// On-screen touch controls (touch screens: Android, platform/touch.h): the
// play controls while the game is played, the scroll pad in the editor.
enum class TouchMode { Off, Play, Editor };
void set_touch_mode(TouchMode mode);
// The editor pad's buttons (left, right, up, down), released and pressed:
// size x size 0xAARRGGBB images.
void set_touch_editor_images(const std::vector<uint32_t> released[4], const std::vector<uint32_t> pressed[4],
                             int size);
// A touch-screen platform (Android): always full screen, controls on screen,
// no cursor sprite.
bool touch_screen();

// Where the game's frame lies in the view (default: centred); mouse positions
// are reported relative to it.
void set_frame_x(int x);

// Logical mouse position (may lie outside the frame, like GetCursorPos).
void get_mouse(int& x, int& y);
void warp_mouse(int x, int y);
void show_system_cursor(bool show);
enum class Cursor { Arrow, Pointer, Wait, Move, SizeNS, SizeWE, SizeNWSE, SizeNESW, Hidden };
void set_system_cursor(Cursor cursor);

void set_title(const char* title);
void message_box(const char* title, const char* text);
std::string base_path();   // directory of the executable, with trailing separator
// Per-user writable directory of a program (SDL_GetPrefPath), with trailing separator.
std::string pref_path(const char* org, const char* app);
// Extra places for the game data on this platform (Android: the app storage).
std::vector<std::string> data_dirs();
// Asks the player for a folder: a message with "Choose folder..." and "Quit",
// then the system's folder dialog. Empty when the player quits or cancels, or
// where there is no dialog (headless runs, Android).
std::string choose_folder(const char* title, const char* text);
// The folders directly inside dir (full paths).
std::vector<std::string> subdirectories(const std::string& dir);
// Text input (and so the on-screen keyboard on Android). Where it is "on
// demand", the port turns it on only while the game asks for typed text.
void set_text_input(bool on);
bool text_input_on_demand();

// Diagnostics (SDL log; debug messages are shown with --verbose).
void log_debug(const char* text);
void log_info(const char* text);
void set_verbose(bool verbose);
}  // namespace blupi::host

#endif
