// Implementations behind the compat headers. Each function does what the game
// needs from the Win32 call on the port's single 640x480 "screen":
// client and screen coordinates are the same logical space.

#include "windows.h"
#include "mmsystem.h"
#include "direct.h"
#include "io.h"
#include "port_messages.h"

#include "platform/host.h"
#include "platform/vfs.h"

#include <deque>
#include <map>
#include <string>
#include <vector>

// ---------------------------------------------------------------------------
// Messages

static std::deque<PortMessage> g_posted;

BOOL PostMessage(HWND, UINT msg, WPARAM wParam, LPARAM lParam)
{
    g_posted.push_back(PortMessage{msg, wParam, lParam});
    return TRUE;
}

bool PortPopMessage(PortMessage& msg)
{
    if (g_posted.empty()) return false;
    msg = g_posted.front();
    g_posted.pop_front();
    return true;
}

int PortPendingMessages()
{
    return (int)g_posted.size();
}

// ---------------------------------------------------------------------------
// Cursor

static int g_cursorCount = 0;  // ShowCursor display counter, visible when >= 0

BOOL GetCursorPos(LPPOINT pt)
{
    int x, y;
    blupi::host::get_mouse(x, y);
    pt->x = x;
    pt->y = y;
    return TRUE;
}

BOOL SetCursorPos(int x, int y)
{
    blupi::host::warp_mouse(x, y);
    return TRUE;
}

BOOL ClientToScreen(HWND, LPPOINT) { return TRUE; }
BOOL ScreenToClient(HWND, LPPOINT) { return TRUE; }

int ShowCursor(BOOL bShow)
{
    g_cursorCount += bShow ? 1 : -1;
    blupi::host::show_system_cursor(g_cursorCount >= 0);
    return g_cursorCount;
}

// ---------------------------------------------------------------------------
// Rectangles (same semantics as user32)

BOOL IsRectEmpty(const RECT* r)
{
    return r->left >= r->right || r->top >= r->bottom;
}

BOOL SetRect(LPRECT r, int left, int top, int right, int bottom)
{
    r->left = left;
    r->top = top;
    r->right = right;
    r->bottom = bottom;
    return TRUE;
}

BOOL IntersectRect(LPRECT dst, const RECT* a, const RECT* b)
{
    RECT r;
    r.left = a->left > b->left ? a->left : b->left;
    r.top = a->top > b->top ? a->top : b->top;
    r.right = a->right < b->right ? a->right : b->right;
    r.bottom = a->bottom < b->bottom ? a->bottom : b->bottom;
    if (IsRectEmpty(&r)) { SetRect(dst, 0, 0, 0, 0); return FALSE; }
    *dst = r;
    return TRUE;
}

BOOL UnionRect(LPRECT dst, const RECT* a, const RECT* b)
{
    if (IsRectEmpty(a)) { if (IsRectEmpty(b)) { SetRect(dst, 0, 0, 0, 0); return FALSE; } *dst = *b; return TRUE; }
    if (IsRectEmpty(b)) { *dst = *a; return TRUE; }
    RECT r;
    r.left = a->left < b->left ? a->left : b->left;
    r.top = a->top < b->top ? a->top : b->top;
    r.right = a->right > b->right ? a->right : b->right;
    r.bottom = a->bottom > b->bottom ? a->bottom : b->bottom;
    *dst = r;
    return TRUE;
}

BOOL OffsetRect(LPRECT r, int dx, int dy)
{
    r->left += dx;
    r->right += dx;
    r->top += dy;
    r->bottom += dy;
    return TRUE;
}

BOOL PtInRect(const RECT* r, POINT pt)
{
    return pt.x >= r->left && pt.x < r->right && pt.y >= r->top && pt.y < r->bottom;
}

// ---------------------------------------------------------------------------
// Files: everything goes through the port file system (user/game roots).

BOOL DeleteFile(LPCSTR path) { return blupi::vfs::remove(path) ? TRUE : FALSE; }
BOOL CreateDirectory(LPCSTR path, LPSECURITY_ATTRIBUTES) { return blupi::vfs::make_dir(path) ? TRUE : FALSE; }
int _mkdir(const char* path) { return blupi::vfs::make_dir(path) ? 0 : -1; }
int _access(const char* path, int) { return blupi::vfs::resolve_read(path).empty() ? -1 : 0; }

static std::map<intptr_t, std::pair<std::vector<std::string>, size_t>> g_finds;
static intptr_t g_nextFind = 1;

static void fill_find(const std::string& name, struct _finddata_t* data)
{
    memset(data, 0, sizeof(*data));
    strncpy(data->name, name.c_str(), sizeof(data->name) - 1);
    data->attrib = _A_NORMAL;
}

intptr_t _findfirst(const char* pattern, struct _finddata_t* data)
{
    std::vector<std::string> names = blupi::vfs::list(pattern);
    if (names.empty()) return -1;
    intptr_t handle = g_nextFind++;
    fill_find(names[0], data);
    g_finds[handle] = {names, 1};
    return handle;
}

int _findnext(intptr_t handle, struct _finddata_t* data)
{
    auto it = g_finds.find(handle);
    if (it == g_finds.end() || it->second.second >= it->second.first.size()) return -1;
    fill_find(it->second.first[it->second.second++], data);
    return 0;
}

int _findclose(intptr_t handle)
{
    return g_finds.erase(handle) ? 0 : -1;
}

// ---------------------------------------------------------------------------
// Misc

void OutputDebugString(LPCSTR text)
{
    blupi::host::log_debug(text);
}

DWORD GetTickCount(void) { return (DWORD)(blupi::host::now_ns() / 1000000); }
DWORD timeGetTime(void) { return GetTickCount(); }

// Joystick (winmm) on SDL gamepads. The game lists the devices on its
// settings screen (joyGetNumDevs, joyGetDevCaps) and, once one is chosen,
// reads it every tick with joyGetPosEx: X and Y on winmm's 0..65535 scale
// (it turns them into directions at 16384 and 49152) and buttons 1 to 4.
// Gamepad buttons south, east, west, north are joystick buttons 1 to 4; the
// d-pad counts as the stick.
UINT joyGetNumDevs(void)
{
    return (UINT)blupi::host::gamepad_count();
}

MMRESULT joyGetDevCaps(UINT id, LPJOYCAPS caps, UINT size)
{
    if (caps == NULL || size < sizeof(JOYCAPS)) return JOYERR_PARMS;
    char name[MAXPNAMELEN];
    if (!blupi::host::gamepad_name((int)id, name, sizeof(name))) return JOYERR_PARMS;
    memset(caps, 0, size);
    strcpy(caps->szPname, name);
    caps->wXmax = caps->wYmax = 65535;
    caps->wNumButtons = caps->wMaxButtons = 4;
    caps->wNumAxes = caps->wMaxAxes = 2;
    return JOYERR_NOERROR;
}

MMRESULT joyGetPosEx(UINT id, LPJOYINFOEX info)
{
    int x = 0, y = 0;
    unsigned buttons = 0;
    if (info == NULL) return JOYERR_PARMS;
    if (!blupi::host::gamepad_state((int)id, x, y, buttons)) return JOYERR_UNPLUGGED;
    info->dwXpos = (DWORD)(x + 32768);
    info->dwYpos = (DWORD)(y + 32768);
    info->dwZpos = info->dwRpos = info->dwUpos = info->dwVpos = 32768;
    info->dwButtons = buttons;
    info->dwButtonNumber = 0;
    for (unsigned b = buttons; b != 0; b &= b - 1) info->dwButtonNumber++;
    info->dwPOV = 65535;   // JOY_POVCENTERED
    return JOYERR_NOERROR;
}

// ---------------------------------------------------------------------------
// C runtime random generator of the original (Microsoft LIBC rand/srand).

static unsigned int g_randSeed = 1;

extern "C" void blupi_srand(unsigned int seed) { g_randSeed = seed; }

// For the state hash (game/state_hash.cpp).
extern "C" unsigned int blupi_rand_state(void) { return g_randSeed; }

extern "C" int blupi_rand(void)
{
    g_randSeed = g_randSeed * 214013u + 2531011u;
    return (int)((g_randSeed >> 16) & 0x7fff);
}
