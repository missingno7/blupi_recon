// Software reference renderer for the faithful 640x480 presentation.
//
// Reproduces what the original's DirectDraw calls did to pixels, in the two
// display modes the game supports:
//   * 8 bpp  palettized (TrueColor=8): surfaces hold palette indices,
//   * 16 bpp RGB565     (TrueColor=16, also implied by Benchmark>=3100).
// Blits follow DirectDraw rules the game relies on: a blit whose source or
// destination rectangle leaves its surface fails and draws nothing (DirectDraw
// returned DDERR_INVALIDRECT; CPixmap clips beforehand), source colour keys
// are an inclusive raw-pixel range, stretching is nearest-neighbour.
//
// Plain C++ with no SDL or Win32 types, so pixel tests can run headless and a
// GPU renderer can later replace it behind the CPixmap interface.
#ifndef BLUPI_RENDER_SURFACE_H
#define BLUPI_RENDER_SURFACE_H

#include <cstdint>
#include <vector>

namespace blupi::render
{
struct Rgb
{
    uint8_t r, g, b;
};

using Palette = Rgb[256];

struct Rect
{
    int left, top, right, bottom;
    int width() const { return right - left; }
    int height() const { return bottom - top; }
};

// Source colour key (DDCKEY_SRCBLT): raw pixel values low..high inclusive.
struct ColorKey
{
    bool     enabled = false;
    uint32_t low = 0, high = 0;
};

class Surface
{
public:
    Surface() = default;
    Surface(int width, int height, int bpp);

    int  width() const { return m_width; }
    int  height() const { return m_height; }
    int  bpp() const { return m_bpp; }
    bool valid() const { return m_width > 0 && m_height > 0; }

    uint32_t pixel(int x, int y) const;
    void     set_pixel(int x, int y, uint32_t value);
    uint8_t*       row(int y) { return m_pixels.data() + (size_t)y * m_pitch; }
    const uint8_t* row(int y) const { return m_pixels.data() + (size_t)y * m_pitch; }

    void fill(uint32_t value);

private:
    int m_width = 0, m_height = 0, m_bpp = 0, m_pitch = 0;
    std::vector<uint8_t> m_pixels;
};

// DirectDraw validity rules, decidable without pixels (backends need not
// report errors): a rectangle must be non-empty and inside its surface, and
// both surfaces must have the same format.
bool blit_valid(int dstW, int dstH, int dstBpp, int dstX, int dstY,
                int srcW, int srcH, int srcBpp, const Rect& srcRect);
bool stretch_valid(int dstW, int dstH, int dstBpp, const Rect& dstRect,
                   int srcW, int srcH, int srcBpp, const Rect& srcRect);

// IDirectDrawSurface::BltFast equivalent. Returns false (nothing drawn) when
// blit_valid() fails.
bool blit(Surface& dst, int dstX, int dstY, const Surface& src, const Rect& srcRect, const ColorKey& key);

// IDirectDrawSurface::Blt with scaling (no key), nearest-neighbour.
bool stretch_blit(Surface& dst, const Rect& dstRect, const Surface& src, const Rect& srcRect);

// Pixel-format conversions shared by loading and presentation.
uint16_t rgb_to_565(Rgb c);              // truncation, as GDI stored 16 bpp DIB colours
Rgb      rgb_from_565(uint16_t v);       // bit replication
uint8_t  nearest_index(const Palette& pal, Rgb c);  // exact match first, then smallest |dr|+|dg|+|db|
uint32_t to_native(const Surface& like, const Palette& pal, Rgb c);  // surface pixel value for a colour
Rgb      from_native(const Surface& like, const Palette& pal, uint32_t v);

// Converts a whole surface to 0xAARRGGBB for presentation or screenshots.
void to_argb8888(const Surface& s, const Palette& pal, uint32_t* out, int outPitchPixels);
}  // namespace blupi::render

#endif
