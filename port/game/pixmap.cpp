// Port replacement of src/pixmap.cpp (historical: Planet Blupi-derived
// DirectDraw implementation of CPixmap).
//
// Keeps the historical class declaration (src/pixmap.h) and the historical
// method bodies: the clipping arithmetic, icon tables, colour-key juggling and
// the software mouse cursor are copied from src/pixmap.cpp. The DirectDraw
// and GDI calls are replaced by commands to a rendering device
// (port/render/device.h). What DirectDraw decided (blit validity, the colour
// key in force) is decided here from a CPU-side registry, so the device only
// executes. The reference device is the software renderer, which reproduces
// the original pixels in 8 bpp and RGB565 (port/docs/ORACLE.md).
//
// The DirectDraw handle types named by src/pixmap.h are opaque to the
// historical code; here a surface handle is a registry entry.

#include <stdlib.h>
#include <stdio.h>
#include "def.h"
#include "pixmap.h"
#include "misc.h"

#include "game/port_game.h"
#include "game/member_access.h"
#include "platform/host.h"
#include "platform/vfs.h"
#include "render/bmp.h"
#include "render/device.h"
#include "render/surface.h"

#include <cstring>
#include <memory>
#include <unordered_map>

using blupi::render::Surface;
using blupi::render::Rgb;
using blupi::render::SurfaceId;

// Rendering devices: the reference (software) device always exists; in
// command-check mode a recorder sits in front of it and every frame's
// commands are replayed on a second, independent device (ROADMAP.md stage 1).
static blupi::render::SoftwareDevice  s_reference;
static blupi::render::Device*         s_device = &s_reference;
static blupi::render::Recorder*       s_recorder = NULL;
static blupi::render::SoftwareDevice* s_shadow = NULL;
static long s_checkFrames = 0;
static long s_checkMismatches = 0;
static long s_checkFirstMismatch = -1;

static void submit(const blupi::render::Command& command)
{
	s_device->submit(command);
}

struct IDirectDraw
{
};

// Registry entry of one surface. Creating and deleting it creates and
// destroys the device surface.
struct IDirectDrawSurface
{
	IDirectDrawSurface(SurfaceId surfaceId, int w, int h, int depth)
		: id(surfaceId), width(w), height(h), bpp(depth)
	{
		submit(blupi::render::make_create(id, width, height, bpp));
	}
	~IDirectDrawSurface()
	{
		submit(blupi::render::make_destroy(id));
	}

	SurfaceId id;
	int       width, height, bpp;
	blupi::render::ColorKey key;              // DDCKEY_SRCBLT in force
	std::shared_ptr<const Surface> image;     // pixels as loaded (CPU queries)
};

struct IDirectDrawPalette
{
	blupi::render::Palette entries;
};

struct IDirectDrawClipper
{
};

extern short table_icon_object[];
extern short table_icon_element[];
extern short table_icon_blupi[];
extern short table_icon_explo[];

// Port: reads of the icon tables (src/pixtables.h, defined in dectables.cpp)
// go through IconValue. The original reads a little outside them in two
// cases: rank -1 (a negative rank passes the "rank >= count" test) and the
// last explosion icon, whose entry lacks its sixth value (the table holds
// 323 values after the count of 54). It then used whatever BLUPI.EXE has next
// to the tables; a modern compiler places other data or padding there. So the
// values around each table are those of the 1998 binary. [EVIDENCE] BLUPI.EXE
// .data: blupi 0x478dd8, object 0x479b80, element 0x47a710, explo 0x47afd8
// (consecutive, padded), then table_mouse_hotspot; read with pefile.
// Consequences: an element icon of rank -1 is 0 pixels wide (never drawn),
// and explosion icon 53 is 124x95 (its height is the hotspot table's 95).
namespace
{
struct IconTable
{
	const short* table;
	int          size;        // values, count included (tests/test_layout.cpp)
	short        before[6];   // the original's values at index -6..-1
	short        after[6];    // and at size..size+5
};

const IconTable s_iconTables[] =
{
	{ table_icon_blupi,   1747, { 17742, 18770, 67, 0, 0, 0 }, { 0, 246, 441, 475, 0, 0 } },
	{ table_icon_object,  1477, { 317, 14, 8, 37, 51, 0 },     { 0, 0, 0, 187, 198, 121 } },
	{ table_icon_element, 1123, { 0, 64, 64, 0, 0, 0 },        { 0, 54, 0, 0, 48, 51 } },
	{ table_icon_explo,   324,  { 51, 2, 32, 54, 27, 0 },      { 95, 0, 0, 0, 30, 0 } },
};

short IconValue(const short* table, int index)
{
	for ( const IconTable& t : s_iconTables )
	{
		if ( t.table != table )  continue;
		if ( index >= 0 && index < t.size )  return t.table[index];
		if ( index < 0 && index >= -6 )  return t.before[index + 6];
		if ( index >= t.size && index < t.size + 6 )  return t.after[index - t.size];
		blupi::host::log_debug("icon table read beyond the modelled neighbourhood");
		return 0;
	}
	return 0;
}
}  // namespace

// DirectDraw results used by the historical method bodies.
#define DD_OK         ((HRESULT)0)
#define DDERR_GENERIC ((HRESULT)0x80004005L)

// Host-path fopen (the forced include maps fopen to the game file system).
#pragma push_macro("fopen")
#undef fopen
static FILE* blupi_host_fopen(const char* path, const char* mode)
{
	return fopen(path, mode);
}
#pragma pop_macro("fopen")

// Cross-build oracle (port/docs/ROADMAP.md 0.3): the same raw-row hash and
// frame file format as port/oracle/ddraw_proxy.cpp, taken at every Display.
static FILE* s_displayLog = NULL;
static char  s_displayDumpDir[260] = "";
static long  s_displayDumpEvery = 0;
static long  s_displayIndex = 0;

static void PortDisplayCapture(const Surface& back, const blupi::render::Palette& pal)
{
	if ( s_displayLog == NULL && s_displayDumpEvery <= 0 )  return;
	const size_t rowBytes = (size_t)back.width() * (back.bpp() / 8);
	uint64_t hash = 1469598103934665603ULL;
	for ( int y=0 ; y<back.height() ; y++ )
	{
		const uint8_t* row = back.row(y);
		for ( size_t i=0 ; i<rowBytes ; i++ )
		{
			hash ^= row[i];
			hash *= 1099511628211ULL;
		}
	}
	if ( s_displayLog )
	{
		fprintf(s_displayLog, "%ld %d %016llx\n", s_displayIndex, back.bpp(), (unsigned long long)hash);
	}
	if ( s_displayDumpEvery > 0 && s_displayIndex % s_displayDumpEvery == 0 )
	{
		char path[300];
		snprintf(path, sizeof(path), "%s/frame_%06ld.raw", s_displayDumpDir, s_displayIndex);
		FILE* f = blupi_host_fopen(path, "wb");
		if ( f != NULL )
		{
			int header[3] = { back.width(), back.height(), back.bpp() };
			fwrite("BLPF", 1, 4, f);
			fwrite(header, sizeof(int), 3, f);
			if ( back.bpp() == 8 )
			{
				for ( int i=0 ; i<256 ; i++ )
				{
					uint8_t e[4] = { pal[i].r, pal[i].g, pal[i].b, 0 };
					fwrite(e, 1, 4, f);
				}
			}
			for ( int y=0 ; y<back.height() ; y++ )  fwrite(back.row(y), 1, rowBytes, f);
			fclose(f);
		}
	}
	s_displayIndex++;
}

void PortPixmapSetDisplayCapture(FILE* log, const char* dumpDir, long dumpEvery)
{
	s_displayLog = log;
	s_displayDumpEvery = dumpEvery;
	if ( dumpDir != NULL )  snprintf(s_displayDumpDir, sizeof(s_displayDumpDir), "%s", dumpDir);
}

// Command check: replays the commands recorded since the last Display on the
// shadow device and compares the back buffer, the screen and the palette.
static bool same_pixels(const Surface* a, const Surface* b)
{
	if ( a == NULL || b == NULL )  return a == b;
	if ( a->width() != b->width() || a->height() != b->height() || a->bpp() != b->bpp() )  return false;
	const size_t rowBytes = (size_t)a->width() * (a->bpp() / 8);
	for ( int y=0 ; y<a->height() ; y++ )
	{
		if ( memcmp(a->row(y), b->row(y), rowBytes) != 0 )  return false;
	}
	return true;
}

static void PortCommandCheck()
{
	if ( s_recorder == NULL )  return;
	blupi::render::replay(s_recorder->commands(), *s_shadow);
	s_recorder->commands().clear();
	bool same = same_pixels(s_reference.surface(blupi::render::kBackBuffer), s_shadow->surface(blupi::render::kBackBuffer)) &&
				same_pixels(s_reference.surface(blupi::render::kScreen), s_shadow->surface(blupi::render::kScreen)) &&
				memcmp(s_reference.palette(), s_shadow->palette(), sizeof(blupi::render::Palette)) == 0;
	if ( !same )
	{
		if ( s_checkFirstMismatch < 0 )  s_checkFirstMismatch = s_checkFrames;
		s_checkMismatches++;
	}
	s_checkFrames++;
}

void PortPixmapEnableCommandCheck()
{
	if ( s_recorder != NULL )  return;
	s_recorder = new blupi::render::Recorder(&s_reference);
	s_shadow = new blupi::render::SoftwareDevice;
	s_device = s_recorder;
}

bool PortPixmapCommandCheckResult(long& frames, long& mismatches, long& firstMismatch)
{
	frames = s_checkFrames;
	mismatches = s_checkMismatches;
	firstMismatch = s_checkFirstMismatch;
	return s_recorder != NULL;
}

// Command capture (scene gate, ROADMAP.md stage 3): commands submitted between
// Begin and End are collected; with execute=false they are not drawn.
struct CaptureFrame
{
	blupi::render::Device*   previous;
	blupi::render::Recorder* recorder;
};
static std::vector<CaptureFrame> s_captures;

void PortPixmapCaptureBegin(bool execute)
{
	CaptureFrame frame = { s_device, new blupi::render::Recorder(execute ? s_device : NULL) };
	s_captures.push_back(frame);
	s_device = frame.recorder;
}

void PortPixmapCaptureEnd(std::vector<blupi::render::Command>& out)
{
	CaptureFrame frame = s_captures.back();
	s_captures.pop_back();
	out = std::move(frame.recorder->commands());
	s_device = frame.previous;
	delete frame.recorder;
}

// Replays captured commands (native frame rendering, ROADMAP.md stage 4).
void PortPixmapSubmit(const std::vector<blupi::render::Command>& commands)
{
	for ( const blupi::render::Command& c : commands )  submit(c);
}

// The one screen and its palette, for presentation by the host loop.
static IDirectDrawSurface* s_primary = NULL;
static bool s_dirty = false;
static blupi::render::Palette s_defaultPalette;

// Display-rate presentation (ROADMAP.md stage 5): the back buffer to the
// screen, without the per-Display instrumentation (command check, oracle log),
// which belongs to the game's own Display calls.
static IDirectDrawSurface* s_back = NULL;

void PortPixmapPresentBack()
{
	if ( s_primary == NULL || s_back == NULL )  return;
	submit(blupi::render::make_blit(s_primary->id, 0, 0, s_back->id,
									blupi::render::Rect{0, 0, s_back->width, s_back->height},
									blupi::render::ColorKey()));
	s_dirty = true;
}


static const blupi::render::Palette& palette_of(IDirectDrawPalette* pal)
{
	return pal ? pal->entries : s_defaultPalette;
}

static void submit_palette(IDirectDrawPalette* pal)
{
	submit(blupi::render::make_palette(palette_of(pal)));
}

static Rgb rgb_of(COLORREF color)
{
	return Rgb{GetRValue(color), GetGValue(color), GetBValue(color)};
}

static blupi::render::Rect rect_of(const RECT& r)
{
	return blupi::render::Rect{(int)r.left, (int)r.top, (int)r.right, (int)r.bottom};
}

// Pixel value of a colour in a surface format (DDColorMatch through GDI).
static uint32_t native_of(int bpp, const blupi::render::Palette& pal, Rgb c)
{
	return bpp == 8 ? blupi::render::nearest_index(pal, c) : blupi::render::rgb_to_565(c);
}

// IDirectDrawSurface::BltFast: invalid rectangles draw nothing and fail.
static HRESULT blt_fast(IDirectDrawSurface* dst, int x, int y, IDirectDrawSurface* src,
						const RECT& rcRect, BOOL bColorKey)
{
	if ( dst == NULL || src == NULL )  return DDERR_GENERIC;
	blupi::render::Rect r = rect_of(rcRect);
	if ( !blupi::render::blit_valid(dst->width, dst->height, dst->bpp, x, y,
									src->width, src->height, src->bpp, r) )  return DDERR_GENERIC;
	blupi::render::ColorKey key;
	if ( bColorKey )  key = src->key;
	submit(blupi::render::make_blit(dst->id, x, y, src->id, r, key));
	return DD_OK;
}

// IDirectDrawSurface::Blt with scaling.
static BOOL blt_stretch(IDirectDrawSurface* dst, const RECT& dest, IDirectDrawSurface* src, const RECT& source)
{
	if ( dst == NULL || src == NULL )  return FALSE;
	blupi::render::Rect d = rect_of(dest), s = rect_of(source);
	if ( !blupi::render::stretch_valid(dst->width, dst->height, dst->bpp, d,
									   src->width, src->height, src->bpp, s) )  return FALSE;
	submit(blupi::render::make_stretch(dst->id, d, src->id, s));
	return TRUE;
}

// DDColorMatch + IDirectDrawSurface::SetColorKey(DDCKEY_SRCBLT).
static void set_color_key(IDirectDrawSurface* pdds, IDirectDrawPalette* pal, COLORREF rgb1, COLORREF rgb2)
{
	if ( pdds == NULL )  return;
	const blupi::render::Palette& p = palette_of(pal);
	pdds->key.enabled = true;
	pdds->key.low = native_of(pdds->bpp, p, rgb_of(rgb1));
	pdds->key.high = native_of(pdds->bpp, p, rgb_of(rgb2));
}

static bool read_file(const char* filename, std::vector<uint8_t>& data)
{
	FILE* file = blupi::vfs::open(filename, "rb");
	if ( file == NULL )  return false;
	uint8_t buffer[65536];
	size_t n;
	while ( (n = fread(buffer, 1, sizeof(buffer), file)) > 0 )  data.insert(data.end(), buffer, buffer + n);
	fclose(file);
	return true;
}

// DDLoadPalette: the colour table of an 8 bpp .blp file.
static IDirectDrawPalette* load_palette(const char* filename)
{
	std::vector<uint8_t> data;
	blupi::render::Bitmap bmp;
	if ( !read_file(filename, data) )  return NULL;
	if ( !blupi::render::decode_bmp(data.data(), data.size(), bmp) || bmp.bpp != 8 )  return NULL;
	IDirectDrawPalette* pal = new IDirectDrawPalette;
	for ( int i=0 ; i<256 ; i++ )  pal->entries[i] = bmp.palette[i];
	// CreatePalette(DDPCAPS_8BIT) without DDPCAPS_ALLOW256: DirectDraw keeps
	// entries 0 and 255 as black and white. [EVIDENCE] cross-build oracle,
	// 8 bpp run 2026-10-21: the historical primary palette has exactly this.
	pal->entries[0] = Rgb{0, 0, 0};
	pal->entries[255] = Rgb{255, 255, 255};
	return pal;
}

// DDLoadBitmap: the image converted to the display format the way GDI did
// (8 bpp: nearest palette entry; 16 bpp: truncation to RGB565), uploaded to
// the device as surface `id`.
static IDirectDrawSurface* load_bitmap(SurfaceId id, const char* filename, int bpp, IDirectDrawPalette* pal)
{
	std::vector<uint8_t> data;
	blupi::render::Bitmap bmp;
	if ( !read_file(filename, data) )  return NULL;
	if ( !blupi::render::decode_bmp(data.data(), data.size(), bmp) )  return NULL;

	Surface image(bmp.width, bmp.height, bpp);
	const blupi::render::Palette& p = palette_of(pal);
	if ( bmp.bpp == 8 )
	{
		uint32_t lut[256];
		for ( int i=0 ; i<256 ; i++ )  lut[i] = native_of(bpp, p, bmp.palette[i]);
		for ( int y=0 ; y<bmp.height ; y++ )
			for ( int x=0 ; x<bmp.width ; x++ )
				image.set_pixel(x, y, lut[bmp.indices[(size_t)y * bmp.width + x]]);
	}
	else
	{
		std::unordered_map<uint32_t, uint32_t> cache;
		for ( int y=0 ; y<bmp.height ; y++ )
			for ( int x=0 ; x<bmp.width ; x++ )
			{
				Rgb c = bmp.rgb[(size_t)y * bmp.width + x];
				uint32_t keyed = (c.r << 16) | (c.g << 8) | c.b;
				auto it = cache.find(keyed);
				if ( it == cache.end() )  it = cache.emplace(keyed, native_of(bpp, p, c)).first;
				image.set_pixel(x, y, it->second);
			}
	}

	IDirectDrawSurface* pdds = new IDirectDrawSurface(id, bmp.width, bmp.height, bpp);
	auto shared = std::make_shared<const Surface>(std::move(image));
	pdds->image = shared;
	submit(blupi::render::make_upload(id, *shared));
	return pdds;
}

/////////////////////////////////////////////////////////////////////////////


// Constructeur.

CPixmap::CPixmap()
{
	int		i;

	m_bFullScreen  = FALSE;
	m_bTrueColorBack   = FALSE;
	m_mouseType    = MOUSETYPEGRA;
	m_bDebug       = TRUE;
	m_bPalette     = TRUE;

	m_mouseSprite  = SPRITE_WAIT;
	MouseHotSpot();
	m_mousePos.x   = LXIMAGE/2;
	m_mousePos.y   = LYIMAGE/2;
	m_mouseBackPos = m_mousePos;
	m_bMouseBack   = FALSE;
	m_bMouseShow   = TRUE;
	m_bBackDisplayed = FALSE;

	m_lpDD         = NULL;
	m_lpDDSPrimary = NULL;
	m_lpDDSBack    = NULL;
	m_lpDDSMouse   = NULL;
	m_lpDDPal      = NULL;
	m_lpClipper    = NULL;

	for ( i=0 ; i<MAXIMAGE ; i++ )
	{
		m_lpDDSurface[i] = NULL;
	}

	// initialize special effects structure
	ZeroMemory(&m_DDbltfx, sizeof(m_DDbltfx));
	m_DDbltfx.dwSize = sizeof(m_DDbltfx);
}

// Destructeur.

CPixmap::~CPixmap()
{
	int		i;

	if ( m_lpDDSPrimary == s_primary )
	{
		s_primary = NULL;
		s_back = NULL;
		PortPixmapWideRelease();
	}
	delete m_lpDDSPrimary;
	delete m_lpDDSBack;
	delete m_lpDDSMouse;
	delete m_lpDDPal;
	for ( i=0 ; i<MAXIMAGE ; i++ )
	{
		delete m_lpDDSurface[i];
		m_lpDDSurface[i] = NULL;
	}
	delete m_lpClipper;
	delete m_lpDD;
}


void CPixmap::SetDebug(BOOL bDebug)
{
	m_bDebug = bDebug;
}


// Crée l'objet DirectDraw principal.
// Retourne FALSE en cas d'erreur.
//
// Port: the screen is the port's 640x480 logical frame. Its format is the
// original exclusive-mode format: 16 bpp when bTrueColor, else 8 bpp with
// palette (the original windowed mode used the desktop format instead).

BOOL CPixmap::Create(HWND hwnd, POINT dim,
					 BOOL bFullScreen, BOOL bTrueColor, int mouseType)
{
	POINT				pos;
	int					depth;

	m_hWnd           = hwnd;
	m_bFullScreen    = bFullScreen;
	m_bTrueColorBack = bTrueColor;
	m_mouseType      = mouseType;
	m_dim            = dim;

	if ( m_mouseType == MOUSETYPEGRA )
	{
		// Cache définitivement la vilaine souris Windows.
		ShowCursor(FALSE);

		pos = m_mousePos;
		ClientToScreen(m_hWnd, &pos);
		SetCursorPos(pos.x, pos.y);  // met la souris au centre
	}

	m_clipRect.left   = 0;
	m_clipRect.top    = 0;
	m_clipRect.right  = dim.x;
	m_clipRect.bottom = dim.y;

	depth = bTrueColor ? 16 : 8;
	m_lpDD         = new IDirectDraw;
	m_lpDDSPrimary = new IDirectDrawSurface(blupi::render::kScreen, dim.x, dim.y, depth);
	m_lpDDSBack    = new IDirectDrawSurface(blupi::render::kBackBuffer, dim.x, dim.y, depth);
	m_lpDDSMouse   = new IDirectDrawSurface(blupi::render::kMouseSave, 64, 64, depth);

	s_primary = m_lpDDSPrimary;
	s_back = m_lpDDSBack;
	return TRUE;
}

// Libère les bitmaps.

BOOL CPixmap::Flush()
{
	return TRUE;
}

BOOL CPixmap::GetTrueColorBack()
{
	return m_bTrueColorBack;
}

BOOL CPixmap::GetTrueColorDecor()
{
	return m_bTrueColorBack;
}

// Restitue les bitmaps.

BOOL CPixmap::Restore()
{
	RestoreAll();
	return TRUE;
}

// Initialise la palette système.
// Port: there is no shared system palette; in 8 bpp the screen palette is
// the system palette, in 16 bpp there is none (as on a true-colour desktop).

BOOL CPixmap::InitSysPalette()
{
	int			i;

	if ( !m_bFullScreen )
	{
		m_bPalette = !m_bTrueColorBack;
	}

	for ( i=0 ; i<256 ; i++ )
	{
		Rgb c = palette_of(m_lpDDPal)[i];
		m_sysPal[i].peRed   = c.r;
		m_sysPal[i].peGreen = c.g;
		m_sysPal[i].peBlue  = c.b;
		m_sysPal[i].peFlags = 0;
	}
	return TRUE;
}

// Indique si l'on utilise une palette.

BOOL CPixmap::IsPalette()
{
	return m_bPalette;
}


// Rempli une zone rectangulaire avec une couleur uniforme.

void CPixmap::Fill(RECT rect, COLORREF color)
{
	// à faire si nécessaire ...
}


// Restore all lost objects.
// Port: software surfaces are never lost.

HRESULT CPixmap::RestoreAll()
{
	return DD_OK;
}

// Effectue un appel BltFast.
// Les modes sont 0=transparent, 1=opaque.

HRESULT CPixmap::BltFast(int chDst, int channel,
						 POINT dst, RECT rcRect, int mode)
{
	int			limit;

	// Effectue un peu de clipping.
	if ( dst.x < m_clipRect.left )
	{
		rcRect.left += m_clipRect.left-dst.x;
		dst.x = m_clipRect.left;
	}
	limit = (m_clipRect.right-dst.x)+rcRect.left;
	if ( rcRect.right > limit )
	{
		rcRect.right = limit;
	}
	if ( dst.y < m_clipRect.top )
	{
		rcRect.top += m_clipRect.top-dst.y;
		dst.y = m_clipRect.top;
	}
	limit = (m_clipRect.bottom-dst.y)+rcRect.top;
	if ( rcRect.bottom > limit )
	{
		rcRect.bottom = limit;
	}

	if ( rcRect.left >= rcRect.right ||
		 rcRect.top  >= rcRect.bottom )  return DD_OK;

	if ( chDst < 0 )
	{
		return blt_fast(m_lpDDSBack, dst.x, dst.y, m_lpDDSurface[channel], rcRect, mode == 0);
	}
	return blt_fast(m_lpDDSurface[chDst], dst.x, dst.y, m_lpDDSurface[channel], rcRect, mode == 0);
}

// Effectue un appel BltFast.
// Les modes sont 0=transparent, 1=opaque.

HRESULT CPixmap::BltFast(LPDIRECTDRAWSURFACE lpDD,
						 int channel, POINT dst, RECT rcRect, int mode)
{
	return blt_fast(lpDD, dst.x, dst.y, m_lpDDSurface[channel], rcRect, mode == 0);
}


BOOL CPixmap::DrawMap(int channel, RECT src, RECT dest)
{
	if ( channel < 0 || channel >= MAXIMAGE ) return FALSE;
	if ( m_lpDDSurface[channel] == NULL ) return FALSE;
	return blt_stretch(m_lpDDSurface[channel], dest, m_lpDDSBack, src);
}

// Sauve toute la palette de couleurs.

BOOL CPixmap::SavePalette()
{
	int		i;

	if ( m_lpDDPal == NULL )  return FALSE;

	for ( i=0 ; i<256 ; i++ )
	{
		m_pal[i].peRed   = m_lpDDPal->entries[i].r;
		m_pal[i].peGreen = m_lpDDPal->entries[i].g;
		m_pal[i].peBlue  = m_lpDDPal->entries[i].b;
		m_pal[i].peFlags = 0;
	}
	return TRUE;
}

// Restitue toute la palette de couleurs.

BOOL CPixmap::RestorePalette()
{
	int		i;

	if ( m_lpDDPal == NULL )  return FALSE;

	for ( i=0 ; i<256 ; i++ )
	{
		m_lpDDPal->entries[i] = Rgb{m_pal[i].peRed, m_pal[i].peGreen, m_pal[i].peBlue};
	}
	submit_palette(m_lpDDPal);
	s_dirty = true;
	return TRUE;
}

// Cherche une couleur dans la palette principale.
// En mode plein écran, il faut chercher dans la palette
// correspondant aux images (obtenue avec SavePalette),
// alors qu'en mode fenêtre, il faut chercher dans la
// palette système (obtenue avec InitSysPalette) !!!

int CPixmap::SearchColor(int red, int green, int blue)
{
	int		i, j, delta, min;

	if ( m_bFullScreen )
	{
		for ( i=0 ; i<256 ; i++ )
		{
			if ( red   == m_pal[i].peRed   &&
				 green == m_pal[i].peGreen &&
				 blue  == m_pal[i].peBlue  )  return i;
		}

		// Cherche la couleur la plus proche.
		min = 10000;
		j   = -1;
		for ( i=0 ; i<256 ; i++ )
		{
			delta = abs(red   - m_pal[i].peRed  )+
					abs(green - m_pal[i].peGreen)+
					abs(blue  - m_pal[i].peBlue );

			if ( delta < min )
			{
				min = delta;
				j = i;
			}
		}
	}
	else
	{
		if ( m_bPalette )
		{
			for ( i=0 ; i<256 ; i++ )
			{
				if ( red   == m_sysPal[i].peRed   &&
					 green == m_sysPal[i].peGreen &&
					 blue  == m_sysPal[i].peBlue  )  return i;
			}

			// Cherche la couleur la plus proche.
			min = 10000;
			j   = -1;
			for ( i=0 ; i<256 ; i++ )
			{
				delta = abs(red   - m_sysPal[i].peRed  )+
						abs(green - m_sysPal[i].peGreen)+
						abs(blue  - m_sysPal[i].peBlue );

				if ( delta < min )
				{
					min = delta;
					j = i;
				}
			}
		}
		else
		{
			j  =  (blue >>3)     &0x001F;
			j |= ((green>>2)<< 5)&0x07E0;
			j |= ((red  >>3)<<11)&0xF800;  // mode 5-6-5
		}
	}
	return j;
}


// Cache une image contenant des icônes.

BOOL CPixmap::Cache(int channel, char *pFilename, POINT totalDim, POINT iconDim,
					BOOL bUsePalette)
{
	IDirectDrawPalette*	pal;

	if ( channel < 0 || channel >= MAXIMAGE )  return FALSE;

	if ( m_lpDDSurface[channel] != NULL )
	{
		Flush(channel);
	}

	// Create and set the palette.
	if ( bUsePalette )
	{
		if ( m_bDebug )  OutputDebug("Use palette\n");
		pal = load_palette(pFilename);
		if ( pal )
		{
			if ( m_bDebug )  OutputDebug("Set palette\n");
			delete m_lpDDPal;
			m_lpDDPal = pal;
			submit_palette(m_lpDDPal);
			s_dirty = true;
		}
	}

	// Create the offscreen surface, by loading our bitmap.
	m_lpDDSurface[channel] = load_bitmap(channel, pFilename, m_lpDDSBack->bpp, m_lpDDPal);

	if ( m_lpDDSurface[channel] == NULL )
	{
		OutputDebug("Fatal error: DDLoadBitmap\n");
		return FALSE;
	}

	// Set the color key to white
	if ( m_bDebug )  OutputDebug("DDSetColorKey\n");
	set_color_key(m_lpDDSurface[channel], m_lpDDPal, RGB(255,255,255), RGB(255,255,255));  // blanc

	strcpy(m_filename[channel], pFilename);

	m_totalDim[channel] = totalDim;
	m_iconDim[channel]  = iconDim;

	return TRUE;
}

// Cache une image globale.

BOOL CPixmap::Cache(int channel, char *pFilename, POINT totalDim, BOOL bUsePalette)
{
	POINT		iconDim;

	if ( channel < 0 || channel >= MAXIMAGE )  return FALSE;

	iconDim.x = 0;
	iconDim.y = 0;

	return Cache(channel, pFilename, totalDim, iconDim, bUsePalette);
}

// Purge une image.

void CPixmap::Flush(int channel)
{
	if ( channel < 0 || channel >= MAXIMAGE )  return;
	if (  m_lpDDSurface[channel] == NULL )     return;

	delete m_lpDDSurface[channel];
	m_lpDDSurface[channel]= NULL;
}

// Définition de la couleur transparente.

void CPixmap::SetTransparent(int channel, COLORREF color)
{
	if ( channel < 0 || channel >= MAXIMAGE )  return;
	if (  m_lpDDSurface[channel] == NULL )     return;

	set_color_key(m_lpDDSurface[channel], m_lpDDPal, color, color);
	m_colorSurface[2*channel+0] = color;
	m_colorSurface[2*channel+1] = color;
}

// Définition de la couleur transparente.

void CPixmap::SetTransparent2(int channel, COLORREF color1, COLORREF color2)
{
	if ( channel < 0 || channel >= MAXIMAGE )  return;
	if (  m_lpDDSurface[channel] == NULL )     return;

	set_color_key(m_lpDDSurface[channel], m_lpDDPal, color1, color2);
	m_colorSurface[2*channel+0] = color1;
	m_colorSurface[2*channel+1] = color2;
}


// Modifie la région de clipping.

void CPixmap::SetClipping(RECT clip)
{
	m_clipRect = clip;
}

// Retourne la région de clipping.

RECT CPixmap::GetClipping()
{
	return m_clipRect;
}


// Teste si un point fait partie d'une icône.

void CPixmap::QuickIcon(int channel, int rank, POINT pos)
{
    int nbx, nby;
    RECT rect;
    int limit;

    if ( channel == 1 )
    {
        if ( rank >= IconValue(table_icon_object, 0) ) return;
        rect.left = IconValue(table_icon_object, rank*6+1);
        rect.right = rect.left + IconValue(table_icon_object, rank*6+5);
        rect.top = IconValue(table_icon_object, rank*6+2);
        rect.bottom = rect.top + IconValue(table_icon_object, rank*6+6);
        pos.x += IconValue(table_icon_object, rank*6+3);
        pos.y += IconValue(table_icon_object, rank*6+4);
    }
    else if ( channel == 10 )
    {
        if ( rank >= IconValue(table_icon_element, 0) ) return;
        rect.left = IconValue(table_icon_element, rank*6+1);
        rect.right = rect.left + IconValue(table_icon_element, rank*6+5);
        rect.top = IconValue(table_icon_element, rank*6+2);
        rect.bottom = rect.top + IconValue(table_icon_element, rank*6+6);
        pos.x += IconValue(table_icon_element, rank*6+3);
        pos.y += IconValue(table_icon_element, rank*6+4);
    }
    else if ( channel == 2 || channel == 11 || channel == 12 || channel == 13 )
    {
        if ( rank >= IconValue(table_icon_blupi, 0) ) return;
        rect.left = IconValue(table_icon_blupi, rank*6+1);
        rect.right = rect.left + IconValue(table_icon_blupi, rank*6+5);
        rect.top = IconValue(table_icon_blupi, rank*6+2);
        rect.bottom = rect.top + IconValue(table_icon_blupi, rank*6+6);
        pos.x += IconValue(table_icon_blupi, rank*6+3);
        pos.y += IconValue(table_icon_blupi, rank*6+4);
    }
    else if ( channel == 9 )
    {
        if ( rank >= IconValue(table_icon_explo, 0) ) return;
        rect.left = IconValue(table_icon_explo, rank*6+1);
        rect.right = rect.left + IconValue(table_icon_explo, rank*6+5);
        rect.top = IconValue(table_icon_explo, rank*6+2);
        rect.bottom = rect.top + IconValue(table_icon_explo, rank*6+6);
        pos.x += IconValue(table_icon_explo, rank*6+3);
        pos.y += IconValue(table_icon_explo, rank*6+4);
    }
    else
    {
        nbx = m_totalDim[channel].x / m_iconDim[channel].x;
        nby = m_totalDim[channel].y / m_iconDim[channel].y;
        if ( rank < 0 || rank >= nbx*nby ) return;
        rect.left   = (rank%nbx)*m_iconDim[channel].x;
        rect.top    = (rank/nbx)*m_iconDim[channel].y;
        rect.right  = rect.left + m_iconDim[channel].x;
        rect.bottom = rect.top  + m_iconDim[channel].y;
    }

	// Effectue un peu de clipping.
	if ( pos.x < m_clipRect.left )
	{
		rect.left += m_clipRect.left-pos.x;
		pos.x = m_clipRect.left;
	}
	limit = (m_clipRect.right-pos.x)+rect.left;
	if ( rect.right > limit )
	{
		rect.right = limit;
	}
	if ( pos.y < m_clipRect.top )
	{
		rect.top += m_clipRect.top-pos.y;
		pos.y = m_clipRect.top;
	}
	limit = (m_clipRect.bottom-pos.y)+rect.top;
	if ( rect.bottom > limit )
	{
		rect.bottom = limit;
	}

	if ( rect.left >= rect.right ||
		 rect.top  >= rect.bottom )  return;

	blt_fast(m_lpDDSBack, pos.x, pos.y, m_lpDDSurface[channel], rect, TRUE);
}


// Port: GDI GetPixel on the surface DC returned the stored pixel expanded to
// a COLORREF; the comparison against the key colours is historical.

BOOL CPixmap::IsIconPixel(int channel, int rank, POINT pos)
{
	int			nbx, nby;
	COLORREF	rgb;
	Rgb			c;

	if ( channel < 0 || channel >= MAXIMAGE )  return FALSE;
	if (  m_lpDDSurface[channel] == NULL )     return FALSE;

	if ( m_iconDim[channel].x == 0 ||
		 m_iconDim[channel].y == 0 )  return FALSE;

	nbx = m_totalDim[channel].x / m_iconDim[channel].x;
	nby = m_totalDim[channel].y / m_iconDim[channel].y;

	if ( rank < 0 || rank >= nbx*nby )  return FALSE;

	pos.x += (rank%nbx)*m_iconDim[channel].x;
	pos.y += (rank/nbx)*m_iconDim[channel].y;

	// Port: current pixels from the reference device, else the pixels as loaded.
	const Surface* s = s_reference.surface(m_lpDDSurface[channel]->id);
	if ( s == NULL )  s = m_lpDDSurface[channel]->image.get();
	if ( s == NULL )  return FALSE;
	if ( pos.x < 0 || pos.y < 0 || pos.x >= s->width() || pos.y >= s->height() )  return FALSE;
	c = blupi::render::from_native(*s, palette_of(m_lpDDPal), s->pixel(pos.x, pos.y));
	rgb = RGB(c.r, c.g, c.b);

	if ( rgb == m_colorSurface[2*channel+0] ||
		 rgb == m_colorSurface[2*channel+1] )  return FALSE;

	return TRUE;
}


// Dessine une partie d'image rectangulaire.
// Les modes sont 0=transparent, 1=opaque.

BOOL CPixmap::DrawIcon(int chDst, int channel, int rank, POINT pos,
                       int mode, BOOL bMask)
{
    int nbx, nby;
    RECT rect;
    HRESULT ddrval;
    COLORREF oldColor1, oldColor2;

    if ( channel == 1 )
    {
        if ( rank >= IconValue(table_icon_object, 0) ) return FALSE;
        rect.left = IconValue(table_icon_object, rank*6+1);
        rect.right = rect.left + IconValue(table_icon_object, rank*6+5);
        rect.top = IconValue(table_icon_object, rank*6+2);
        rect.bottom = rect.top + IconValue(table_icon_object, rank*6+6);
        pos.x += IconValue(table_icon_object, rank*6+3);
        pos.y += IconValue(table_icon_object, rank*6+4);
    }
    else if ( channel == 10 )
    {
        if ( rank >= IconValue(table_icon_element, 0) ) return FALSE;
        rect.left = IconValue(table_icon_element, rank*6+1);
        rect.right = rect.left + IconValue(table_icon_element, rank*6+5);
        rect.top = IconValue(table_icon_element, rank*6+2);
        rect.bottom = rect.top + IconValue(table_icon_element, rank*6+6);
        pos.x += IconValue(table_icon_element, rank*6+3);
        pos.y += IconValue(table_icon_element, rank*6+4);
    }
    else if ( channel == 2 || channel == 11 || channel == 12 || channel == 13 )
    {
        if ( rank >= IconValue(table_icon_blupi, 0) ) return FALSE;
        rect.left = IconValue(table_icon_blupi, rank*6+1);
        rect.right = rect.left + IconValue(table_icon_blupi, rank*6+5);
        rect.top = IconValue(table_icon_blupi, rank*6+2);
        rect.bottom = rect.top + IconValue(table_icon_blupi, rank*6+6);
        pos.x += IconValue(table_icon_blupi, rank*6+3);
        pos.y += IconValue(table_icon_blupi, rank*6+4);
    }
    else if ( channel == 9 )
    {
        if ( rank >= IconValue(table_icon_explo, 0) ) return FALSE;
        rect.left = IconValue(table_icon_explo, rank*6+1);
        rect.right = rect.left + IconValue(table_icon_explo, rank*6+5);
        rect.top = IconValue(table_icon_explo, rank*6+2);
        rect.bottom = rect.top + IconValue(table_icon_explo, rank*6+6);
        pos.x += IconValue(table_icon_explo, rank*6+3);
        pos.y += IconValue(table_icon_explo, rank*6+4);
    }
    else
    {
        if ( channel < 0 || channel >= MAXIMAGE ) return FALSE;
        if ( m_lpDDSurface[channel] == NULL ) return FALSE;
        if ( m_iconDim[channel].x == 0 || m_iconDim[channel].y == 0 ) return FALSE;
        nbx = m_totalDim[channel].x / m_iconDim[channel].x;
        nby = m_totalDim[channel].y / m_iconDim[channel].y;
        if ( rank < 0 || rank >= nbx*nby ) return FALSE;
        rect.left = (rank%nbx)*m_iconDim[channel].x;
        rect.top = (rank/nbx)*m_iconDim[channel].y;
        rect.right = rect.left + m_iconDim[channel].x;
        rect.bottom = rect.top + m_iconDim[channel].y;
    }

    oldColor1 = m_colorSurface[2*channel+0];
    oldColor2 = m_colorSurface[2*channel+1];
    if ( bMask ) SetTransparent(channel, 0x000000ff);
    ddrval = BltFast(chDst, channel, pos, rect, mode);
    if ( bMask ) SetTransparent2(channel, oldColor1, oldColor2);
    if ( ddrval != DD_OK ) return FALSE;
    return TRUE;
}

BOOL CPixmap::DrawIconDemi(int chDst, int channel, int rank, POINT pos,
						   int mode, BOOL bMask)
{
	int			nbx, nby;
	RECT		rect;
	HRESULT		ddrval;
	COLORREF	oldColor1, oldColor2;

	if ( channel < 0 || channel >= MAXIMAGE )  return FALSE;
	if (  m_lpDDSurface[channel] == NULL )     return FALSE;

	if ( m_iconDim[channel].x == 0 ||
		 m_iconDim[channel].y == 0 )  return FALSE;

	nbx = m_totalDim[channel].x /  m_iconDim[channel].x;
	nby = m_totalDim[channel].y / (m_iconDim[channel].y/2);

	rank = (rank/32)*32+((rank%32)/2)+((rank%2)*16);

	if ( rank < 0 || rank >= nbx*nby )  return FALSE;

	rect.left   = (rank%nbx)* m_iconDim[channel].x;
	rect.top    = (rank/nbx)*(m_iconDim[channel].y/2);
	rect.right  = rect.left + m_iconDim[channel].x;
	rect.bottom = rect.top  +(m_iconDim[channel].y/2);

	oldColor1 = m_colorSurface[2*channel+0];
	oldColor2 = m_colorSurface[2*channel+1];
	if ( bMask )  SetTransparent(channel, 0x000000ff);  // blanc
	ddrval = BltFast(chDst, channel, pos, rect, mode);
	if ( bMask )  SetTransparent2(channel, oldColor1, oldColor2);

	if ( ddrval != DD_OK )  return FALSE;
	return TRUE;
}

// Dessine une partie d'image rectangulaire.
// Les modes sont 0=transparent, 1=opaque.

BOOL CPixmap::DrawIconPart(int chDst, int channel, int rank, POINT pos,
						   int startY, int endY,
						   int mode, BOOL bMask)
{
	int			nbx, nby;
	RECT		rect;
	HRESULT		ddrval;
	COLORREF	oldColor1, oldColor2;

	if ( channel < 0 || channel >= MAXIMAGE )  return FALSE;
	if (  m_lpDDSurface[channel] == NULL )     return FALSE;

	if ( m_iconDim[channel].x == 0 ||
		 m_iconDim[channel].y == 0 )  return FALSE;

	nbx = m_totalDim[channel].x / m_iconDim[channel].x;
	nby = m_totalDim[channel].y / m_iconDim[channel].y;

	if ( rank < 0 || rank >= nbx*nby )  return FALSE;

	rect.left   = (rank%nbx)*m_iconDim[channel].x;
	rect.top    = (rank/nbx)*m_iconDim[channel].y;
	rect.right  = rect.left + m_iconDim[channel].x;
	rect.bottom = rect.top  + endY;

	pos.y    += startY;
	rect.top += startY;

	oldColor1 = m_colorSurface[2*channel+0];
	oldColor2 = m_colorSurface[2*channel+1];
	if ( bMask )  SetTransparent(channel, 0x000000ff);  // blanc
	ddrval = BltFast(chDst, channel, pos, rect, mode);
	if ( bMask )  SetTransparent2(channel, oldColor1, oldColor2);

	if ( ddrval != DD_OK )  return FALSE;
	return TRUE;
}

// Dessine une partie d'image n'importe où.
// Les modes sont 0=transparent, 1=opaque.

BOOL CPixmap::DrawPart(int chDst, int channel, POINT dest, RECT rect,
                       int mode, BOOL bMask)
{
    HRESULT ddrval;
    COLORREF oldColor1, oldColor2;

    if ( channel < 0 || channel >= MAXIMAGE ) return FALSE;
    if ( m_lpDDSurface[channel] == NULL ) return FALSE;
    oldColor1 = m_colorSurface[2*channel+0];
    oldColor2 = m_colorSurface[2*channel+1];
    if ( bMask ) SetTransparent(channel, 0x000000ff);
    ddrval = BltFast(chDst, channel, dest, rect, mode);
    if ( bMask ) SetTransparent2(channel, oldColor1, oldColor2);
    if ( ddrval != DD_OK ) return FALSE;
    return TRUE;
}

BOOL CPixmap::DrawImage(int chDst, int channel, RECT rect, int mode)
{
	POINT		dst;
	HRESULT		ddrval;

	if ( channel < 0 || channel >= MAXIMAGE )  return FALSE;
	if (  m_lpDDSurface[channel] == NULL )     return FALSE;

	dst.x = rect.left;
	dst.y = rect.top;

	ddrval = BltFast(chDst, channel, dst, rect, mode);

	if ( ddrval != DD_OK )  return FALSE;

	if ( channel == CHBACK )
	{
		MouseBackSave();  // sauve ce qui sera sous la souris
		m_bBackDisplayed = FALSE;
	}

	return TRUE;
}


// Construit une icône en utilisant un masque.

BOOL CPixmap::BuildIconMask(int channelMask, int rankMask,
							int channel, int rankSrc, int rankDst)
{
	int			nbx, nby;
	POINT		posDst;
	RECT		rect;
	HRESULT		ddrval;

	if ( channel < 0 || channel >= MAXIMAGE )  return FALSE;
	if (  m_lpDDSurface[channel] == NULL )     return FALSE;

	if ( m_iconDim[channel].x == 0 ||
		 m_iconDim[channel].y == 0 )  return FALSE;

	nbx = m_totalDim[channel].x / m_iconDim[channel].x;
	nby = m_totalDim[channel].y / m_iconDim[channel].y;

	if ( rankSrc < 0 || rankSrc >= nbx*nby )  return FALSE;
	if ( rankDst < 0 || rankDst >= nbx*nby )  return FALSE;

	rect.left   = (rankSrc%nbx)*m_iconDim[channel].x;
	rect.top    = (rankSrc/nbx)*m_iconDim[channel].y;
	rect.right  = rect.left + m_iconDim[channel].x;
	rect.bottom = rect.top  + m_iconDim[channel].y;
	posDst.x    = (rankDst%nbx)*m_iconDim[channel].x;
	posDst.y    = (rankDst/nbx)*m_iconDim[channel].y;
	ddrval = BltFast(m_lpDDSurface[channel], channel, posDst, rect, 1);
	if ( ddrval != DD_OK )  return FALSE;

	if ( m_iconDim[channelMask].x == 0 ||
		 m_iconDim[channelMask].y == 0 )  return FALSE;

	nbx = m_totalDim[channelMask].x / m_iconDim[channelMask].x;
	nby = m_totalDim[channelMask].y / m_iconDim[channelMask].y;

	if ( rankMask < 0 || rankMask >= nbx*nby )  return FALSE;

	rect.left   = (rankMask%nbx)*m_iconDim[channelMask].x;
	rect.top    = (rankMask/nbx)*m_iconDim[channelMask].y;
	rect.right  = rect.left + m_iconDim[channelMask].x;
	rect.bottom = rect.top  + m_iconDim[channelMask].y;
	ddrval = BltFast(m_lpDDSurface[channel], channelMask, posDst, rect, 0);
	if ( ddrval != DD_OK )  return FALSE;

	return TRUE;
}


// Affiche le pixmap à l'écran.
// Retourne FALSE en cas d'erreur.
// Port: the "screen" is the primary surface, presented by the host loop.

BOOL CPixmap::Display()
{
	RECT		MapRect;

	m_bBackDisplayed = TRUE;

	MapRect.left   = 0;
	MapRect.top    = 0;
	MapRect.right  = m_dim.x;
	MapRect.bottom = m_dim.y;

	if ( blt_fast(m_lpDDSPrimary, 0, 0, m_lpDDSBack, MapRect, FALSE) != DD_OK )  return FALSE;
	s_dirty = true;
	PortCommandCheck();
	PortDisplayCapture(*s_reference.surface(blupi::render::kBackBuffer), s_reference.palette());
	return TRUE;
}


// Positionne la souris et change le lutin.

void CPixmap::SetMousePosSprite(POINT pos, int sprite, BOOL bDemoPlay)
{
	if ( m_mousePos.x == pos.x &&
		 m_mousePos.y == pos.y &&
		 m_mouseSprite == sprite )  return;

	m_mousePos    = pos;
	m_mouseSprite = sprite;
	MouseHotSpot();

	if ( !bDemoPlay )
	{
		MouseUpdate();
	}
}

// Positionne la souris.

void CPixmap::SetMousePos(POINT pos, BOOL bDemoPlay)
{
	if ( m_mousePos.x == pos.x &&
		 m_mousePos.y == pos.y )  return;

	m_mousePos = pos;

	if ( !bDemoPlay )
	{
		MouseUpdate();
	}
}

// Change le lutin de la souris.

void CPixmap::SetMouseSprite(int sprite, BOOL bDemoPlay)
{
	if ( m_mouseSprite == sprite )  return;

	m_mouseSprite = sprite;
	MouseHotSpot();

	if ( !bDemoPlay )
	{
		MouseUpdate();
	}
}

// Montre ou cache la souris.

void CPixmap::MouseShow(BOOL bShow)
{
	m_bMouseShow = bShow;
}

// Met à jour le dessin de la souris.

void CPixmap::MouseUpdate()
{
	RECT	oldRect, newRect, rcRect;

	if ( m_lpDDSurface[CHPIXMAP_CURSOR] == NULL )  return;
	if ( m_mouseType != MOUSETYPEGRA )  return;
	if ( m_mouseSprite == SPRITE_EMPTY )  return;
	if ( m_mouseSprite == SPRITE_DISABLE )  return;
	if ( !m_bMouseShow )  return;

	oldRect.left   = m_mouseBackPos.x;
	oldRect.top    = m_mouseBackPos.y;
	oldRect.right  = m_mouseBackPos.x + DIMBLUPIX;
	oldRect.bottom = m_mouseBackPos.y + DIMBLUPIY;

	newRect.left   = m_mousePos.x - m_mouseHotSpot.x;
	newRect.top    = m_mousePos.y - m_mouseHotSpot.y;
	newRect.right  = newRect.left + DIMBLUPIX;
	newRect.bottom = newRect.top  + DIMBLUPIY;

	MouseBackRestore();  // enlève la souris dans m_lpDDSBack
	MouseBackDraw();     // dessine la souris dans m_lpDDSBack

	if ( m_bBackDisplayed )
	{
		if ( IntersectRect(&rcRect, &oldRect, &newRect) )
		{
			UnionRect(&rcRect, &oldRect, &newRect);
			MouseQuickDraw(rcRect);
		}
		else
		{
			MouseQuickDraw(oldRect);
			MouseQuickDraw(newRect);
		}
	}
}

// Dessine rapidement la souris dans l'écran.
// Il s'agit en fait de dessiner un petit morceau rectangulaire
// de m_lpDDSBack dans l'écran.

BOOL CPixmap::MouseQuickDraw(RECT rect)
{
	if ( rect.left   < 0       )  rect.left   = 0;
	if ( rect.right  > LXIMAGE )  rect.right  = LXIMAGE;
	if ( rect.top    < 0       )  rect.top    = 0;
	if ( rect.bottom > LYIMAGE )  rect.bottom = LYIMAGE;

	if ( blt_fast(m_lpDDSPrimary, rect.left, rect.top, m_lpDDSBack, rect, FALSE) != DD_OK )  return FALSE;
	s_dirty = true;
	return TRUE;
}

// Invalide la copie sous la souris.

void CPixmap::MouseInvalidate()
{
	m_bMouseBack = FALSE;
}

// Enlève la souris dans m_lpDDSBack.

void CPixmap::MouseBackClear()
{
	if ( m_mouseType != MOUSETYPEGRA )  return;
	if ( m_mouseSprite == SPRITE_DISABLE )  return;
	MouseBackRestore();  // enlève la souris dans m_lpDDSBack
}

// Dessine la souris dans m_lpDDSBack.

// Port: touch screens show no cursor; a tap is the click itself.
static bool s_cursorHidden = false;

void PortPixmapHideCursor(bool hidden)
{
	s_cursorHidden = hidden;
}

void CPixmap::MouseBackDraw()
{
	POINT	dst;
	RECT	rcRect;

	if ( s_cursorHidden )  return;   // port (touch screens)
	if ( m_lpDDSurface[CHPIXMAP_CURSOR] == NULL )  return;
	if ( m_mouseType != MOUSETYPEGRA )  return;
	if ( m_mouseSprite == SPRITE_EMPTY )  return;
	if ( m_mouseSprite == SPRITE_DISABLE )  return;
	if ( !m_bMouseShow )  return;

	MouseBackSave();  // sauve ce qui sera sous la souris

	dst.x  = m_mousePos.x - m_mouseHotSpot.x;
	dst.y  = m_mousePos.y - m_mouseHotSpot.y;
	MouseRectSprite(&rcRect, &dst);

	if ( dst.x < 0 )
	{
		rcRect.left -= dst.x;
		dst.x = 0;
	}
	if ( rcRect.right-rcRect.left+dst.x > LXIMAGE )
	{
		rcRect.right = rcRect.left-dst.x+LXIMAGE;
	}
	if ( dst.y < 0 )
	{
		rcRect.top -= dst.y;
		dst.y = 0;
	}
	if ( rcRect.bottom-rcRect.top+dst.y > LYIMAGE )
	{
		rcRect.bottom = rcRect.top-dst.y+LYIMAGE;
	}

	// Dessine le lutin dans m_lpDDSBack.
	BltFast(m_lpDDSBack, CHPIXMAP_CURSOR, dst, rcRect, 0);
}

// Sauve le fond sous la souris.
// m_lpDDSMouse <- m_lpDDSBack

void CPixmap::MouseBackSave()
{
	POINT		dst;
	RECT		rcRect;

	if ( m_lpDDSurface[CHPIXMAP_CURSOR] == NULL )  return;
	if ( m_mouseType != MOUSETYPEGRA )  return;
	if ( m_mouseSprite == SPRITE_EMPTY )  return;
	if ( m_mouseSprite == SPRITE_DISABLE )  return;
	if ( !m_bMouseShow )  return;

	m_mouseBackPos.x = m_mousePos.x - m_mouseHotSpot.x;
	m_mouseBackPos.y = m_mousePos.y - m_mouseHotSpot.y;
	m_bMouseBack = TRUE;

	dst.x = 0;
	dst.y = 0;

	rcRect.left   = m_mouseBackPos.x;
	rcRect.top    = m_mouseBackPos.y;
	rcRect.right  = m_mouseBackPos.x + DIMBLUPIX;
	rcRect.bottom = m_mouseBackPos.y + DIMBLUPIY;

	if ( rcRect.left < 0 )
	{
		dst.x -= rcRect.left;
		rcRect.left = 0;
	}
	if ( rcRect.right > LXIMAGE )
	{
		rcRect.right = LXIMAGE;
	}
	if ( rcRect.top < 0 )
	{
		dst.y -= rcRect.top;
		rcRect.top = 0;
	}
	if ( rcRect.bottom > LYIMAGE )
	{
		rcRect.bottom = LYIMAGE;
	}

	blt_fast(m_lpDDSMouse, dst.x, dst.y, m_lpDDSBack, rcRect, FALSE);
}

// Restitue le fond sous la souris.
// m_lpDDSBack <- m_lpDDSMouse

void CPixmap::MouseBackRestore()
{
	POINT		dst;
	RECT		rcRect;

	if ( m_lpDDSurface[CHPIXMAP_CURSOR] == NULL )  return;
	if ( !m_bMouseBack )  return;

	dst.x = m_mouseBackPos.x;
	dst.y = m_mouseBackPos.y;

	rcRect.left   = 0;
	rcRect.top    = 0;
	rcRect.right  = DIMBLUPIX;
	rcRect.bottom = DIMBLUPIY;

	if ( dst.x < 0 )
	{
		rcRect.left -= dst.x;
		dst.x = 0;
	}
	if ( dst.x+DIMBLUPIX > LXIMAGE )
	{
		rcRect.right -= (dst.x+DIMBLUPIX)-LXIMAGE;
	}
	if ( dst.y < 0 )
	{
		rcRect.top -= dst.y;
		dst.y = 0;
	}
	if ( dst.y+DIMBLUPIY > LYIMAGE )
	{
		rcRect.bottom -= (dst.y+DIMBLUPIY)-LYIMAGE;
	}

	blt_fast(m_lpDDSBack, dst.x, dst.y, m_lpDDSMouse, rcRect, FALSE);
}

// Returns the source rectangle and on-screen origin for the mouse sprite.
void CPixmap::MouseRectSprite(RECT *rcRect, POINT *pos)
{
    int rank;

    rank = 37;
    if ( m_mouseSprite == SPRITE_ARROW ) rank = 37;
    if ( m_mouseSprite == SPRITE_POINTER ) rank = 38;
    if ( m_mouseSprite == SPRITE_WAIT ) rank = 39;

    rcRect->left = IconValue(table_icon_element, rank*6+1);
    rcRect->right = rcRect->left + IconValue(table_icon_element, rank*6+5);
    rcRect->top = IconValue(table_icon_element, rank*6+2);
    rcRect->bottom = rcRect->top + IconValue(table_icon_element, rank*6+6);
    pos->x += IconValue(table_icon_element, rank*6+3);
    pos->y += IconValue(table_icon_element, rank*6+4);
}

void CPixmap::MouseHotSpot()
{
	int				rank;

	static int table_mouse_hotspot[4*2] =
	{
		95, 0,
		30, 30,		// SPRITE_ARROW
		20, 15,		// SPRITE_POINTER
		30, 30,
	};

	if ( m_mouseSprite >= SPRITE_ARROW &&
		 m_mouseSprite <= SPRITE_WAIT  )
	{
		rank = m_mouseSprite;

		// Port: the table has 4 entries, but the original indexes it up to
		// SPRITE_WAIT (11). [EVIDENCE] In BLUPI.EXE the table (.data
		// 0x47b260) is followed by text, so ranks 4..11 read hotspots around
		// 1.7e9 (SPRITE_WAIT: 1701867625, 1769414770): every blit of such a
		// sprite is clipped to an empty, invalid rectangle and the sprite is
		// never drawn. The port reproduces that effect instead of reading
		// past the array (undefined, and fatal on Linux).
		if ( rank*2+1 < (int)(sizeof(table_mouse_hotspot)/sizeof(table_mouse_hotspot[0])) )
		{
			m_mouseHotSpot.x = table_mouse_hotspot[rank*2+0];
			m_mouseHotSpot.y = table_mouse_hotspot[rank*2+1];
		}
		else
		{
			m_mouseHotSpot.x = 1701867625;
			m_mouseHotSpot.y = 1769414770;
		}
	}
	else
	{
		m_mouseHotSpot.x = 0;
		m_mouseHotSpot.y = 0;
	}
}


/////////////////////////////////////////////////////////////////////////////
// Port presentation helpers (port_game.h). Host paths, not game paths.

#undef fopen

bool PortPixmapHasPrimary()
{
	return s_primary != NULL;
}

bool PortPixmapCopyScreen(std::vector<uint32_t>& argb, int& width, int& height)
{
	const Surface* s = s_reference.surface(blupi::render::kScreen);
	if ( s_primary == NULL || s == NULL )  return false;
	width  = s->width();
	height = s->height();
	argb.resize((size_t)width * height);
	blupi::render::to_argb8888(*s, s_reference.palette(), argb.data(), width);
	return true;
}

void PortPixmapPresentIfDirty()
{
	static std::vector<uint32_t> frame;
	int width, height;

	if ( PortPixmapWidePresentIfDirty() )  return;

	if ( !s_dirty )  return;
	s_dirty = false;
	if ( PortPixmapCopyScreen(frame, width, height) )
	{
		blupi::host::present(frame.data(), width);
	}
}

uint64_t PortPixmapScreenHash()
{
	std::vector<uint32_t> frame;
	int width, height;
	uint64_t hash = 1469598103934665603ULL;

	if ( !PortPixmapCopyScreen(frame, width, height) )  return 0;
	for ( uint32_t px : frame )
	{
		for ( int i=0 ; i<3 ; i++ )
		{
			hash ^= (px >> (i*8)) & 0xff;
			hash *= 1099511628211ULL;
		}
	}
	return hash;
}

bool PortPixmapWriteScreenBmp(const char* path)
{
	std::vector<uint32_t> frame;
	int width, height;

	if ( !PortPixmapCopyWide(frame, width, height) &&
		 !PortPixmapCopyScreen(frame, width, height) )  return false;   // the view, else the screen
	FILE* file = fopen(path, "wb");
	if ( file == NULL )  return false;
	const size_t stride = ((size_t)width * 3 + 3) & ~(size_t)3;   // rows padded to 4 bytes
	uint32_t imageSize = (uint32_t)(stride * height);
	uint8_t header[54] = {'B', 'M'};
	uint32_t fileSize = 54 + imageSize;
	auto put32 = [&](int at, uint32_t v) { for ( int i=0 ; i<4 ; i++ ) header[at+i] = (uint8_t)(v >> (i*8)); };
	put32(2, fileSize);
	put32(10, 54);
	put32(14, 40);
	put32(18, (uint32_t)width);
	put32(22, (uint32_t)height);
	header[26] = 1;
	header[28] = 24;
	put32(34, imageSize);
	fwrite(header, 1, sizeof(header), file);
	std::vector<uint8_t> line(stride, 0);
	for ( int y=height-1 ; y>=0 ; y-- )
	{
		for ( int x=0 ; x<width ; x++ )
		{
			uint32_t px = frame[(size_t)y*width + x];
			line[x*3+0] = (uint8_t)(px);
			line[x*3+1] = (uint8_t)(px >> 8);
			line[x*3+2] = (uint8_t)(px >> 16);
		}
		fwrite(line.data(), 1, line.size(), file);
	}
	fclose(file);
	return true;
}


// Widescreen view (ROADMAP.md stage 8).
// A target wider than the game's back buffer. While it is being drawn,
// CPixmap's back surface is the target, so the historical drawing code
// (QuickIcon, DrawPart, texts) draws into it with its own clipping; the game's
// back buffer and screen are left alone.

namespace
{
#define WIDE_PIXMAP_MEMBERS(X) \
	X(m_lpDDSBack) X(m_lpDDSurface) X(m_clipRect) X(m_mousePos) X(m_mouseHotSpot) \
	X(m_mouseType) X(m_mouseSprite) X(m_bMouseShow) X(MouseRectSprite)
BLUPI_MEMBER_ACCESS(PixmapAccess, CPixmap, WIDE_PIXMAP_MEMBERS)
}  // namespace

static int                 s_wideWidth = 0;      // 0: the host shows the screen
static IDirectDrawSurface* s_wide = NULL;
static IDirectDrawSurface* s_wideSavedBack = NULL;
static RECT                s_wideSavedClip;
static bool                s_wideDirty = false;

void PortPixmapSetWide(int width)
{
	s_wideWidth = width;
}

bool PortPixmapWideBegin(CPixmap* pixmap)
{
	IDirectDrawSurface*& back = pixmap->*PixmapAccess::m_lpDDSBack;
	if ( s_wideWidth <= 0 || back == NULL || s_wideSavedBack != NULL )  return false;
	// A fresh surface each time: black where nothing is drawn.
	delete s_wide;
	s_wide = new IDirectDrawSurface(blupi::render::kWideView, s_wideWidth, back->height, back->bpp);
	s_wideSavedBack = back;
	s_wideSavedClip = pixmap->*PixmapAccess::m_clipRect;
	back = s_wide;
	pixmap->*PixmapAccess::m_clipRect = RECT{0, 0, s_wide->width, s_wide->height};
	return true;
}

void PortPixmapWideEnd(CPixmap* pixmap)
{
	if ( s_wideSavedBack == NULL )  return;
	pixmap->*PixmapAccess::m_lpDDSBack = s_wideSavedBack;
	pixmap->*PixmapAccess::m_clipRect = s_wideSavedClip;
	s_wideSavedBack = NULL;
}

void PortPixmapWideSubmit(const std::vector<blupi::render::Command>& commands, int dx)
{
	if ( s_wide == NULL )  return;
	for ( blupi::render::Command c : commands )
	{
		if ( (c.kind == blupi::render::Command::Blit || c.kind == blupi::render::Command::Stretch) &&
			 c.dst == blupi::render::kBackBuffer )
		{
			c.dst = s_wide->id;
			c.dstRect.left  += dx;
			c.dstRect.right += dx;
		}
		submit(c);
	}
}

void PortPixmapWideBlitBack(int dx)
{
	if ( s_wide == NULL || s_back == NULL )  return;
	submit(blupi::render::make_blit(s_wide->id, dx, 0, s_back->id,
									blupi::render::Rect{0, 0, s_back->width, s_back->height},
									blupi::render::ColorKey()));
}

// CPixmap::MouseBackDraw without the save of what is under the sprite (the
// view is drawn again for every presentation), in the target's bounds.
void PortPixmapWideCursor(CPixmap* pixmap, int dx, const POINT* at)
{
	IDirectDrawSurface* sprite = (pixmap->*PixmapAccess::m_lpDDSurface)[CHPIXMAP_CURSOR];
	POINT dst;
	RECT  rcRect;

	if ( s_wide == NULL || sprite == NULL || s_cursorHidden )  return;
	if ( pixmap->*PixmapAccess::m_mouseType != MOUSETYPEGRA )  return;
	if ( pixmap->*PixmapAccess::m_mouseSprite == SPRITE_EMPTY )  return;
	if ( pixmap->*PixmapAccess::m_mouseSprite == SPRITE_DISABLE )  return;
	if ( !(pixmap->*PixmapAccess::m_bMouseShow) )  return;

	const POINT mouse = at != NULL ? *at : pixmap->*PixmapAccess::m_mousePos;
	dst.x = mouse.x - (pixmap->*PixmapAccess::m_mouseHotSpot).x;
	dst.y = mouse.y - (pixmap->*PixmapAccess::m_mouseHotSpot).y;
	(pixmap->*PixmapAccess::MouseRectSprite)(&rcRect, &dst);
	dst.x += dx;

	if ( dst.x < 0 )
	{
		rcRect.left -= dst.x;
		dst.x = 0;
	}
	if ( rcRect.right-rcRect.left+dst.x > s_wide->width )
	{
		rcRect.right = rcRect.left-dst.x+s_wide->width;
	}
	if ( dst.y < 0 )
	{
		rcRect.top -= dst.y;
		dst.y = 0;
	}
	if ( rcRect.bottom-rcRect.top+dst.y > s_wide->height )
	{
		rcRect.bottom = rcRect.top-dst.y+s_wide->height;
	}
	blt_fast(s_wide, dst.x, dst.y, sprite, rcRect, TRUE);
}

void PortPixmapWidePresent()
{
	s_wideDirty = true;
}

bool PortPixmapCopyWide(std::vector<uint32_t>& argb, int& width, int& height)
{
	if ( s_wideWidth <= 0 || s_wide == NULL )  return false;
	const Surface* s = s_reference.surface(s_wide->id);
	if ( s == NULL )  return false;
	width  = s->width();
	height = s->height();
	argb.resize((size_t)width * height);
	blupi::render::to_argb8888(*s, s_reference.palette(), argb.data(), width);
	return true;
}

bool PortPixmapWidePresentIfDirty()
{
	static std::vector<uint32_t> frame;
	int width, height;

	if ( s_wideWidth <= 0 )  return false;
	if ( !s_wideDirty )  return true;
	s_wideDirty = false;
	if ( PortPixmapCopyWide(frame, width, height) )  blupi::host::present(frame.data(), width);
	return true;
}

void PortPixmapWideRelease()
{
	delete s_wide;
	s_wide = NULL;
	s_wideSavedBack = NULL;
}

bool PortPixmapWideRegionEqualsBack(int x)
{
	if ( s_wide == NULL || s_back == NULL )  return false;
	const Surface* wide = s_reference.surface(s_wide->id);
	const Surface* back = s_reference.surface(s_back->id);
	if ( wide == NULL || back == NULL || x < 0 || x + back->width() > wide->width() )  return false;
	const size_t pixel = (size_t)back->bpp() / 8;
	for ( int y=0 ; y<back->height() ; y++ )
	{
		if ( memcmp(wide->row(y) + x * pixel, back->row(y), back->width() * pixel) != 0 )  return false;
	}
	return true;
}
