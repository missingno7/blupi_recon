// Frame-capture proxy for the historical rebuild (port/docs/ROADMAP.md 0.3).
//
// A 32-bit ddraw.dll placed next to the rebuilt BLUPI.EXE. It forwards every
// call to the real DirectDraw implementation (cnc-ddraw, renamed
// cnc-ddraw.dll) and wraps only the primary surface. CPixmap::Display shows a
// frame with one Blt of the whole 640x480 back buffer onto the primary; at that
// moment the proxy hashes the back buffer's raw pixel rows (8 bpp indices or
// 16 bpp words) exactly like the port's --display-log, optionally dumps the raw
// frame, and after BLUPI_ORACLE_MAX_FRAMES frames posts WM_CLOSE.
// Nothing in the game or its data is modified; the historical source is not
// involved.
//
// Environment:
//   BLUPI_ORACLE_LOG         file receiving "index bpp hash" per displayed frame
//   BLUPI_ORACLE_DUMP_DIR    directory for raw frames (optional)
//   BLUPI_ORACLE_DUMP_EVERY  dump every Nth frame (default: none)
//   BLUPI_ORACLE_MAX_FRAMES  post WM_CLOSE after this many frames (default: none)
//   BLUPI_ORACLE_INPUT       scripted input in the port's --input format
//                            ("FRAME click X Y", "FRAME key|keydown|keyup VK").
//                            Frame N is the port's N-th timer frame, i.e. the
//                            frame shown by Display number N+11; its messages
//                            are posted during Display N+10, so they are queued
//                            before that frame's WM_UPDATE, as in the port.

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <ddraw.h>

#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <vector>

namespace
{
typedef HRESULT(WINAPI* DirectDrawCreateFn)(GUID*, LPDIRECTDRAW*, IUnknown*);

FILE* g_log = nullptr;
long  g_frame = 0;
long  g_dumpEvery = 0;
long  g_maxFrames = 0;
char  g_dumpDir[MAX_PATH] = "";
HWND  g_hwnd = nullptr;

struct Input
{
    long frame;
    int  kind;   // 0 click, 1 key, 2 keydown, 3 keyup
    int  x, y, vk;
};
std::vector<Input> g_inputs;

uint64_t fnv(uint64_t h, const unsigned char* p, size_t n)
{
    for (size_t i = 0; i < n; i++)
    {
        h ^= p[i];
        h *= 1099511628211ULL;
    }
    return h;
}

void init_env()
{
    static bool done = false;
    if (done) return;
    done = true;
    if (const char* p = getenv("BLUPI_ORACLE_LOG")) g_log = fopen(p, "w");
    if (const char* p = getenv("BLUPI_ORACLE_DUMP_DIR")) strncpy(g_dumpDir, p, MAX_PATH - 1);
    if (const char* p = getenv("BLUPI_ORACLE_DUMP_EVERY")) g_dumpEvery = atol(p);
    if (const char* p = getenv("BLUPI_ORACLE_MAX_FRAMES")) g_maxFrames = atol(p);
    if (const char* p = getenv("BLUPI_ORACLE_INPUT"))
    {
        if (FILE* f = fopen(p, "r"))
        {
            char line[200];
            while (fgets(line, sizeof(line), f))
            {
                Input in = {};
                char kind[16] = {};
                if (line[0] == '#' || sscanf(line, "%ld %15s", &in.frame, kind) != 2) continue;
                if (strcmp(kind, "click") == 0 && sscanf(line, "%ld %15s %d %d", &in.frame, kind, &in.x, &in.y) == 4)
                    in.kind = 0;
                else if (sscanf(line, "%ld %15s %d", &in.frame, kind, &in.vk) == 3)
                    in.kind = strcmp(kind, "key") == 0 ? 1 : strcmp(kind, "keydown") == 0 ? 2 : strcmp(kind, "keyup") == 0 ? 3 : -1;
                else
                    continue;
                if (in.kind >= 0) g_inputs.push_back(in);
            }
            fclose(f);
        }
    }
}

void debug_line(const char* text)
{
    if (const char* d = getenv("BLUPI_ORACLE_DEBUG"))
    {
        if (FILE* f = fopen(d, "a"))
        {
            fprintf(f, "%s\n", text);
            fclose(f);
        }
    }
}

void post_input(long frame)
{
    if (frame == 0)
    {
        char text[128];
        snprintf(text, sizeof(text), "inputs %u hwnd %p", (unsigned)g_inputs.size(), (void*)g_hwnd);
        debug_line(text);
    }
    for (const Input& in : g_inputs)
    {
        if (in.frame != frame || g_hwnd == nullptr) continue;
        if (in.kind == 0)
        {
            LPARAM pos = MAKELPARAM(in.x, in.y);
            BOOL ok = PostMessageA(g_hwnd, WM_MOUSEMOVE, 0, pos);
            if (const char* d = getenv("BLUPI_ORACLE_DEBUG"))
            {
                if (FILE* f = fopen(d, "a"))
                {
                    char cls[64] = "";
                    GetClassNameA(g_hwnd, cls, sizeof(cls));
                    fprintf(f, "frame %ld click %d,%d hwnd %p class '%s' visible %d post %d err %lu fg %p thread %lu/%lu\n",
                            in.frame, in.x, in.y, (void*)g_hwnd, cls, IsWindowVisible(g_hwnd), ok, GetLastError(),
                            (void*)GetForegroundWindow(), GetWindowThreadProcessId(g_hwnd, nullptr), GetCurrentThreadId());
                    fclose(f);
                }
            }
            PostMessageA(g_hwnd, WM_LBUTTONDOWN, MK_LBUTTON, pos);
            PostMessageA(g_hwnd, WM_LBUTTONUP, 0, pos);
        }
        if (in.kind == 1 || in.kind == 2) PostMessageA(g_hwnd, WM_KEYDOWN, (WPARAM)in.vk, 1);
        if (in.kind == 1 || in.kind == 3) PostMessageA(g_hwnd, WM_KEYUP, (WPARAM)in.vk, (LPARAM)0xC0000001);
    }
}

// Raw frame file: "BLPF", width, height, bpp, then 256 RGBX palette entries
// (8 bpp only), then width*bpp/8 bytes per row, top to bottom.
void capture(IDirectDrawSurface* primary, IDirectDrawSurface* back)
{
    DDSURFACEDESC sd;
    memset(&sd, 0, sizeof(sd));
    sd.dwSize = sizeof(sd);
    if (back->Lock(nullptr, &sd, DDLOCK_WAIT | DDLOCK_READONLY, nullptr) != DD_OK) return;
    const int bpp = (int)sd.ddpfPixelFormat.dwRGBBitCount;
    const int width = (int)sd.dwWidth, height = (int)sd.dwHeight;
    const size_t rowBytes = (size_t)width * (bpp / 8);
    uint64_t hash = 1469598103934665603ULL;
    std::vector<unsigned char> frame;
    bool dump = g_dumpEvery > 0 && g_dumpDir[0] && (g_frame % g_dumpEvery) == 0;
    for (int y = 0; y < height; y++)
    {
        const unsigned char* row = static_cast<const unsigned char*>(sd.lpSurface) + (size_t)y * sd.lPitch;
        hash = fnv(hash, row, rowBytes);
        if (dump) frame.insert(frame.end(), row, row + rowBytes);
    }
    back->Unlock(nullptr);

    if (g_log)
    {
        fprintf(g_log, "%ld %d %016llx\n", g_frame, bpp, (unsigned long long)hash);
        fflush(g_log);
    }
    if (dump)
    {
        char path[MAX_PATH + 32];
        snprintf(path, sizeof(path), "%s\\frame_%06ld.raw", g_dumpDir, g_frame);
        if (FILE* f = fopen(path, "wb"))
        {
            int header[3] = {width, height, bpp};
            fwrite("BLPF", 1, 4, f);
            fwrite(header, sizeof(int), 3, f);
            if (bpp == 8)
            {
                PALETTEENTRY pal[256] = {};
                IDirectDrawPalette* p = nullptr;
                if (primary->GetPalette(&p) == DD_OK && p)
                {
                    p->GetEntries(0, 0, 256, pal);
                    p->Release();
                }
                fwrite(pal, sizeof(pal), 1, f);
            }
            fwrite(frame.data(), 1, frame.size(), f);
            fclose(f);
        }
    }
    post_input(g_frame - 10);
    g_frame++;
    if (g_maxFrames > 0 && g_frame == g_maxFrames && g_hwnd) PostMessageA(g_hwnd, WM_CLOSE, 0, 0);
}

// Forwarding wrapper for the primary surface.
class PrimarySurface : public IDirectDrawSurface
{
public:
    explicit PrimarySurface(IDirectDrawSurface* real) : m_real(real) {}
    IDirectDrawSurface* real() { return m_real; }

    STDMETHOD(QueryInterface)(REFIID riid, LPVOID* out) override
    {
        if (riid == IID_IUnknown || riid == IID_IDirectDrawSurface)
        {
            AddRef();
            *out = this;
            return S_OK;
        }
        return m_real->QueryInterface(riid, out);
    }
    STDMETHOD_(ULONG, AddRef)() override { m_refs++; return m_real->AddRef(); }
    STDMETHOD_(ULONG, Release)() override
    {
        ULONG r = m_real->Release();
        if (--m_refs == 0) delete this;
        return r;
    }
    STDMETHOD(AddAttachedSurface)(LPDIRECTDRAWSURFACE s) override { return m_real->AddAttachedSurface(s); }
    STDMETHOD(AddOverlayDirtyRect)(LPRECT r) override { return m_real->AddOverlayDirtyRect(r); }
    STDMETHOD(Blt)(LPRECT dst, LPDIRECTDRAWSURFACE src, LPRECT srcRect, DWORD flags, LPDDBLTFX fx) override
    {
        if (src && srcRect && srcRect->left == 0 && srcRect->top == 0 &&
            srcRect->right == 640 && srcRect->bottom == 480)
        {
            capture(m_real, src);
        }
        return m_real->Blt(dst, src, srcRect, flags, fx);
    }
    STDMETHOD(BltBatch)(LPDDBLTBATCH b, DWORD n, DWORD f) override { return m_real->BltBatch(b, n, f); }
    STDMETHOD(BltFast)(DWORD x, DWORD y, LPDIRECTDRAWSURFACE s, LPRECT r, DWORD t) override
    {
        return m_real->BltFast(x, y, s, r, t);
    }
    STDMETHOD(DeleteAttachedSurface)(DWORD f, LPDIRECTDRAWSURFACE s) override { return m_real->DeleteAttachedSurface(f, s); }
    STDMETHOD(EnumAttachedSurfaces)(LPVOID c, LPDDENUMSURFACESCALLBACK cb) override { return m_real->EnumAttachedSurfaces(c, cb); }
    STDMETHOD(EnumOverlayZOrders)(DWORD f, LPVOID c, LPDDENUMSURFACESCALLBACK cb) override { return m_real->EnumOverlayZOrders(f, c, cb); }
    STDMETHOD(Flip)(LPDIRECTDRAWSURFACE s, DWORD f) override { return m_real->Flip(s, f); }
    STDMETHOD(GetAttachedSurface)(LPDDSCAPS c, LPDIRECTDRAWSURFACE* s) override { return m_real->GetAttachedSurface(c, s); }
    STDMETHOD(GetBltStatus)(DWORD f) override { return m_real->GetBltStatus(f); }
    STDMETHOD(GetCaps)(LPDDSCAPS c) override { return m_real->GetCaps(c); }
    STDMETHOD(GetClipper)(LPDIRECTDRAWCLIPPER* c) override { return m_real->GetClipper(c); }
    STDMETHOD(GetColorKey)(DWORD f, LPDDCOLORKEY k) override { return m_real->GetColorKey(f, k); }
    STDMETHOD(GetDC)(HDC* dc) override { return m_real->GetDC(dc); }
    STDMETHOD(GetFlipStatus)(DWORD f) override { return m_real->GetFlipStatus(f); }
    STDMETHOD(GetOverlayPosition)(LPLONG x, LPLONG y) override { return m_real->GetOverlayPosition(x, y); }
    STDMETHOD(GetPalette)(LPDIRECTDRAWPALETTE* p) override { return m_real->GetPalette(p); }
    STDMETHOD(GetPixelFormat)(LPDDPIXELFORMAT f) override { return m_real->GetPixelFormat(f); }
    STDMETHOD(GetSurfaceDesc)(LPDDSURFACEDESC d) override { return m_real->GetSurfaceDesc(d); }
    STDMETHOD(Initialize)(LPDIRECTDRAW dd, LPDDSURFACEDESC d) override { return m_real->Initialize(dd, d); }
    STDMETHOD(IsLost)() override { return m_real->IsLost(); }
    STDMETHOD(Lock)(LPRECT r, LPDDSURFACEDESC d, DWORD f, HANDLE e) override { return m_real->Lock(r, d, f, e); }
    STDMETHOD(ReleaseDC)(HDC dc) override { return m_real->ReleaseDC(dc); }
    STDMETHOD(Restore)() override { return m_real->Restore(); }
    STDMETHOD(SetClipper)(LPDIRECTDRAWCLIPPER c) override { return m_real->SetClipper(c); }
    STDMETHOD(SetColorKey)(DWORD f, LPDDCOLORKEY k) override { return m_real->SetColorKey(f, k); }
    STDMETHOD(SetOverlayPosition)(LONG x, LONG y) override { return m_real->SetOverlayPosition(x, y); }
    STDMETHOD(SetPalette)(LPDIRECTDRAWPALETTE p) override { return m_real->SetPalette(p); }
    STDMETHOD(Unlock)(LPVOID p) override { return m_real->Unlock(p); }
    STDMETHOD(UpdateOverlay)(LPRECT a, LPDIRECTDRAWSURFACE s, LPRECT b, DWORD f, LPDDOVERLAYFX fx) override
    {
        return m_real->UpdateOverlay(a, s, b, f, fx);
    }
    STDMETHOD(UpdateOverlayDisplay)(DWORD f) override { return m_real->UpdateOverlayDisplay(f); }
    STDMETHOD(UpdateOverlayZOrder)(DWORD f, LPDIRECTDRAWSURFACE s) override { return m_real->UpdateOverlayZOrder(f, s); }

private:
    IDirectDrawSurface* m_real;
    ULONG m_refs = 1;
};

// Forwarding wrapper for the DirectDraw object; wraps the primary surface.
class ProxyDirectDraw : public IDirectDraw
{
public:
    explicit ProxyDirectDraw(IDirectDraw* real) : m_real(real) {}

    STDMETHOD(QueryInterface)(REFIID riid, LPVOID* out) override
    {
        if (riid == IID_IUnknown || riid == IID_IDirectDraw)
        {
            AddRef();
            *out = this;
            return S_OK;
        }
        return m_real->QueryInterface(riid, out);
    }
    STDMETHOD_(ULONG, AddRef)() override { m_refs++; return m_real->AddRef(); }
    STDMETHOD_(ULONG, Release)() override
    {
        ULONG r = m_real->Release();
        if (--m_refs == 0) delete this;
        return r;
    }
    STDMETHOD(Compact)() override { return m_real->Compact(); }
    STDMETHOD(CreateClipper)(DWORD f, LPDIRECTDRAWCLIPPER* c, IUnknown* u) override { return m_real->CreateClipper(f, c, u); }
    STDMETHOD(CreatePalette)(DWORD f, LPPALETTEENTRY e, LPDIRECTDRAWPALETTE* p, IUnknown* u) override
    {
        return m_real->CreatePalette(f, e, p, u);
    }
    STDMETHOD(CreateSurface)(LPDDSURFACEDESC d, LPDIRECTDRAWSURFACE* s, IUnknown* u) override
    {
        HRESULT hr = m_real->CreateSurface(d, s, u);
        if (hr == DD_OK && d && (d->dwFlags & DDSD_CAPS) && (d->ddsCaps.dwCaps & DDSCAPS_PRIMARYSURFACE))
        {
            *s = new PrimarySurface(*s);
        }
        return hr;
    }
    STDMETHOD(DuplicateSurface)(LPDIRECTDRAWSURFACE a, LPDIRECTDRAWSURFACE* b) override { return m_real->DuplicateSurface(a, b); }
    STDMETHOD(EnumDisplayModes)(DWORD f, LPDDSURFACEDESC d, LPVOID c, LPDDENUMMODESCALLBACK cb) override
    {
        return m_real->EnumDisplayModes(f, d, c, cb);
    }
    STDMETHOD(EnumSurfaces)(DWORD f, LPDDSURFACEDESC d, LPVOID c, LPDDENUMSURFACESCALLBACK cb) override
    {
        return m_real->EnumSurfaces(f, d, c, cb);
    }
    STDMETHOD(FlipToGDISurface)() override { return m_real->FlipToGDISurface(); }
    STDMETHOD(GetCaps)(LPDDCAPS a, LPDDCAPS b) override { return m_real->GetCaps(a, b); }
    STDMETHOD(GetDisplayMode)(LPDDSURFACEDESC d) override { return m_real->GetDisplayMode(d); }
    STDMETHOD(GetFourCCCodes)(LPDWORD n, LPDWORD c) override { return m_real->GetFourCCCodes(n, c); }
    STDMETHOD(GetGDISurface)(LPDIRECTDRAWSURFACE* s) override { return m_real->GetGDISurface(s); }
    STDMETHOD(GetMonitorFrequency)(LPDWORD f) override { return m_real->GetMonitorFrequency(f); }
    STDMETHOD(GetScanLine)(LPDWORD l) override { return m_real->GetScanLine(l); }
    STDMETHOD(GetVerticalBlankStatus)(LPBOOL b) override { return m_real->GetVerticalBlankStatus(b); }
    STDMETHOD(Initialize)(GUID* g) override { return m_real->Initialize(g); }
    STDMETHOD(RestoreDisplayMode)() override { return m_real->RestoreDisplayMode(); }
    STDMETHOD(SetCooperativeLevel)(HWND w, DWORD f) override
    {
        g_hwnd = w;
        return m_real->SetCooperativeLevel(w, f);
    }
    STDMETHOD(SetDisplayMode)(DWORD w, DWORD h, DWORD bpp) override { return m_real->SetDisplayMode(w, h, bpp); }
    STDMETHOD(WaitForVerticalBlank)(DWORD f, HANDLE e) override { return m_real->WaitForVerticalBlank(f, e); }

private:
    IDirectDraw* m_real;
    ULONG m_refs = 1;
};
}  // namespace

extern "C" __declspec(dllexport) HRESULT WINAPI DirectDrawCreate(GUID* guid, LPDIRECTDRAW* dd, IUnknown* outer)
{
    init_env();
    static DirectDrawCreateFn real = nullptr;
    if (real == nullptr)
    {
        char path[MAX_PATH];
        HMODULE self = nullptr;
        GetModuleHandleExA(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
                           reinterpret_cast<LPCSTR>(&DirectDrawCreate), &self);
        GetModuleFileNameA(self, path, MAX_PATH);
        char* slash = strrchr(path, '\\');
        if (slash) strcpy(slash + 1, "cnc-ddraw.dll");
        HMODULE lib = LoadLibraryA(path);
        if (lib) real = reinterpret_cast<DirectDrawCreateFn>(GetProcAddress(lib, "DirectDrawCreate"));
        if (real == nullptr) return DDERR_GENERIC;
    }
    HRESULT hr = real(guid, dd, outer);
    if (hr == DD_OK && dd && *dd) *dd = new ProxyDirectDraw(*dd);
    return hr;
}
