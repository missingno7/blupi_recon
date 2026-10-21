// Port compatibility header: DirectDraw *names only*.
//
// The historical CPixmap declaration (src/pixmap.h) keeps DirectDraw-typed
// members. In the port these are opaque handles owned by the port's CPixmap
// implementation (port/game/pixmap.cpp); there is no DirectDraw emulation.
#ifndef BLUPI_COMPAT_DDRAW_H
#define BLUPI_COMPAT_DDRAW_H

#include "windows.h"

struct IDirectDraw;
struct IDirectDrawSurface;
struct IDirectDrawPalette;
struct IDirectDrawClipper;

typedef struct IDirectDraw*        LPDIRECTDRAW;
typedef struct IDirectDrawSurface* LPDIRECTDRAWSURFACE;
typedef struct IDirectDrawPalette* LPDIRECTDRAWPALETTE;
typedef struct IDirectDrawClipper* LPDIRECTDRAWCLIPPER;

typedef struct tagPALETTEENTRY
{
	BYTE peRed;
	BYTE peGreen;
	BYTE peBlue;
	BYTE peFlags;
} PALETTEENTRY, *LPPALETTEENTRY;

// Layout is irrelevant to the port; only CPixmap holds one.
typedef struct _DDBLTFX
{
	DWORD dwSize;
	DWORD dwReserved[24];
} DDBLTFX;

#endif
