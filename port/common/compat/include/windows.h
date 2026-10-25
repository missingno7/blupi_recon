// Port compatibility header (NOT a Windows SDK replacement).
//
// Declares only the Win32 names that the shared historical game TUs in ../src
// use, with the values of the original SDK so that message numbers, key codes
// and serialized struct layouts are unchanged. Behaviour lives in
// port/common/compat/src/win32.cpp. Anything not listed here is intentionally absent.
//
// Case variants used by the historical source (Windows.h, WTypes.h, DEF.H) are
// resolved by the case-insensitive Windows file system; on case-sensitive
// hosts port/CMakeLists.txt generates forwarding headers in the build tree.
#ifndef BLUPI_COMPAT_WINDOWS_H
#define BLUPI_COMPAT_WINDOWS_H

#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include <ctype.h>   // the SDK windows.h exposes the ctype functions

#define WINAPI
#define CALLBACK
#define APIENTRY
#ifndef _WIN32
#define __cdecl     // a compiler keyword on Windows compilers only
#endif
#define FAR
#define NEAR

typedef int             BOOL;
typedef unsigned char   BYTE;
typedef unsigned char   byte;
typedef uint16_t        WORD;
typedef uint32_t        DWORD;
typedef unsigned int    UINT;
typedef int32_t         LONG;     // Win32 LONG is 32-bit on every ABI (LLP64)
typedef uint32_t        ULONG;
typedef int16_t         SHORT;
typedef uint16_t        USHORT;
typedef char            CHAR;
typedef int32_t         HRESULT;
typedef DWORD           COLORREF;
typedef intptr_t        LPARAM;
typedef uintptr_t       WPARAM;
typedef intptr_t        LRESULT;

typedef char*           LPSTR;
typedef const char*     LPCSTR;
typedef void*           LPVOID;
typedef const void*     LPCVOID;
typedef DWORD*          LPDWORD;
typedef BYTE*           LPBYTE;
typedef LONG*           LPLONG;

// Opaque handles. The port never dereferences them inside shared TUs.
typedef struct BlupiHWND__*      HWND;
typedef struct BlupiHINSTANCE__* HINSTANCE;
typedef struct BlupiHDC__*       HDC;
typedef struct BlupiHBITMAP__*   HBITMAP;
typedef struct BlupiHCURSOR__*   HCURSOR;
typedef void*                    HANDLE;

#ifndef TRUE
#define TRUE  1
#endif
#ifndef FALSE
#define FALSE 0
#endif

#define MAX_PATH 260
#ifndef _MAX_FNAME
#define _MAX_FNAME 256     // MSVC <stdlib.h> value; the historical source uses it
#endif

typedef struct tagPOINT { LONG x; LONG y; } POINT, *LPPOINT;
typedef struct tagRECT  { LONG left; LONG top; LONG right; LONG bottom; } RECT, *LPRECT;
typedef const RECT* LPCRECT;

typedef struct _GUID
{
	DWORD Data1;
	WORD  Data2;
	WORD  Data3;
	BYTE  Data4[8];
} GUID, *LPGUID;

typedef struct _SECURITY_ATTRIBUTES
{
	DWORD  nLength;
	LPVOID lpSecurityDescriptor;
	BOOL   bInheritHandle;
} SECURITY_ATTRIBUTES, *LPSECURITY_ATTRIBUTES;

#define LOWORD(l)   ((WORD)(((uintptr_t)(l)) & 0xffff))
#define HIWORD(l)   ((WORD)((((uintptr_t)(l)) >> 16) & 0xffff))
#define LOBYTE(w)   ((BYTE)(((uintptr_t)(w)) & 0xff))
#define MAKELONG(a, b) ((LONG)(((WORD)(((uintptr_t)(a)) & 0xffff)) | ((DWORD)((WORD)(((uintptr_t)(b)) & 0xffff))) << 16))
#define MAKELPARAM(l, h) ((LPARAM)(DWORD)MAKELONG(l, h))

#define RGB(r, g, b) ((COLORREF)(((BYTE)(r) | ((WORD)((BYTE)(g)) << 8)) | (((DWORD)(BYTE)(b)) << 16)))
#define GetRValue(rgb) ((BYTE)(rgb))
#define GetGValue(rgb) ((BYTE)(((WORD)(rgb)) >> 8))
#define GetBValue(rgb) ((BYTE)((rgb) >> 16))

#define ZeroMemory(p, n) memset((p), 0, (n))
#define CopyMemory(d, s, n) memcpy((d), (s), (n))

#define SUCCEEDED(hr) (((HRESULT)(hr)) >= 0)
#define FAILED(hr)    (((HRESULT)(hr)) < 0)
#define S_OK    ((HRESULT)0)
#define S_FALSE ((HRESULT)1)
#define E_FAIL  ((HRESULT)0x80004005L)

// Window messages (values from the Win32 SDK).
#define WM_CREATE        0x0001
#define WM_DESTROY       0x0002
#define WM_CLOSE         0x0010
#define WM_ACTIVATEAPP   0x001C
#define WM_SETCURSOR     0x0020
#define WM_NCMOUSEMOVE   0x00A0
#define WM_KEYDOWN       0x0100
#define WM_KEYUP         0x0101
#define WM_CHAR          0x0102
#define WM_SYSKEYDOWN    0x0104
#define WM_SYSKEYUP      0x0105
#define WM_TIMER         0x0113
#define WM_MOUSEMOVE     0x0200
#define WM_LBUTTONDOWN   0x0201
#define WM_LBUTTONUP     0x0202
#define WM_RBUTTONDOWN   0x0204
#define WM_RBUTTONUP     0x0205
#define WM_USER          0x0400

#define MK_LBUTTON  0x0001
#define MK_RBUTTON  0x0002
#define MK_SHIFT    0x0004
#define MK_CONTROL  0x0008

// Virtual key codes.
#define VK_BACK     0x08
#define VK_TAB      0x09
#define VK_RETURN   0x0D
#define VK_SHIFT    0x10
#define VK_CONTROL  0x11
#define VK_MENU     0x12
#define VK_PAUSE    0x13
#define VK_ESCAPE   0x1B
#define VK_SPACE    0x20
#define VK_PRIOR    0x21
#define VK_NEXT     0x22
#define VK_END      0x23
#define VK_HOME     0x24
#define VK_LEFT     0x25
#define VK_UP       0x26
#define VK_RIGHT    0x27
#define VK_DOWN     0x28
#define VK_INSERT   0x2D
#define VK_DELETE   0x2E
#define VK_F1       0x70
#define VK_F2       0x71
#define VK_F3       0x72
#define VK_F4       0x73
#define VK_F5       0x74
#define VK_F6       0x75
#define VK_F7       0x76
#define VK_F8       0x77
#define VK_F9       0x78
#define VK_F10      0x79
#define VK_F11      0x7A
#define VK_F12      0x7B

// Functions implemented by the port (port/common/compat/src/win32.cpp).
BOOL    PostMessage(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
BOOL    GetCursorPos(LPPOINT pt);
BOOL    SetCursorPos(int x, int y);
BOOL    ClientToScreen(HWND hWnd, LPPOINT pt);
BOOL    ScreenToClient(HWND hWnd, LPPOINT pt);
int     ShowCursor(BOOL bShow);
BOOL    IntersectRect(LPRECT dst, const RECT* a, const RECT* b);
BOOL    UnionRect(LPRECT dst, const RECT* a, const RECT* b);
BOOL    SetRect(LPRECT r, int left, int top, int right, int bottom);
BOOL    OffsetRect(LPRECT r, int dx, int dy);
BOOL    PtInRect(const RECT* r, POINT pt);
BOOL    IsRectEmpty(const RECT* r);
BOOL    DeleteFile(LPCSTR path);
BOOL    CreateDirectory(LPCSTR path, LPSECURITY_ATTRIBUTES att);
void    OutputDebugString(LPCSTR text);
DWORD   GetTickCount(void);

// Like the SDK header, pull in winmm unless WIN32_LEAN_AND_MEAN is set.
#ifndef WIN32_LEAN_AND_MEAN
#include "mmsystem.h"
#endif

#endif
