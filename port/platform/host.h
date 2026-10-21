// SDL3 host: window, presentation of the 640x480 logical frame, input and
// time. Free of Win32 names; port/game/main.cpp turns host events into the
// window messages the historical CEvent code expects.
//
// Keys are reported with Win32 virtual-key numbering because that is the
// game's own input vocabulary (CEvent::TreatEvent switches on it); the mapping
// table from SDL keycodes lives in host.cpp.
#ifndef BLUPI_PLATFORM_HOST_H
#define BLUPI_PLATFORM_HOST_H

#include <cstdint>
#include <string>

namespace blupi::host
{
struct Options
{
    bool headless = false;    // no window, no audio device, no waiting (automated tests)
    bool fullscreen = false;  // borderless desktop fullscreen, letterboxed
    int  scale = 2;           // initial window size = logical size * scale
    bool integerScale = false;
};

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
void present(const uint32_t* argb);
void toggle_fullscreen();

// Logical mouse position (may lie outside the frame, like GetCursorPos).
void get_mouse(int& x, int& y);
void warp_mouse(int x, int y);
void show_system_cursor(bool show);
enum class Cursor { Arrow, Pointer, Wait, Move, SizeNS, SizeWE, SizeNWSE, SizeNESW, Hidden };
void set_system_cursor(Cursor cursor);

void set_title(const char* title);
void message_box(const char* title, const char* text);
std::string base_path();   // directory of the executable, with trailing separator
std::string pref_path();   // per-user writable directory, with trailing separator

// Diagnostics (SDL log; debug messages are shown with --verbose).
void log_debug(const char* text);
void log_info(const char* text);
void set_verbose(bool verbose);
}  // namespace blupi::host

#endif
