#include "render/surface.h"

#include <cstdlib>
#include <cstring>

namespace blupi::render
{
Surface::Surface(int width, int height, int bpp)
    : m_width(width), m_height(height), m_bpp(bpp), m_pitch(width * (bpp / 8)),
      m_pixels((size_t)width * height * (bpp / 8), 0)
{
}

uint32_t Surface::pixel(int x, int y) const
{
    const uint8_t* p = row(y);
    return m_bpp == 8 ? p[x] : reinterpret_cast<const uint16_t*>(p)[x];
}

void Surface::set_pixel(int x, int y, uint32_t value)
{
    uint8_t* p = row(y);
    if (m_bpp == 8) p[x] = (uint8_t)value;
    else reinterpret_cast<uint16_t*>(p)[x] = (uint16_t)value;
}

void Surface::fill(uint32_t value)
{
    for (int y = 0; y < m_height; y++)
        for (int x = 0; x < m_width; x++) set_pixel(x, y, value);
}

namespace
{
bool inside(const Rect& r, int w, int h)
{
    return r.left >= 0 && r.top >= 0 && r.right <= w && r.bottom <= h &&
           r.left < r.right && r.top < r.bottom;
}

template <typename Pixel>
void copy_rows(Surface& dst, int dx, int dy, const Surface& src, const Rect& sr,
               bool key, uint32_t low, uint32_t high)
{
    const int w = sr.width();
    for (int y = 0; y < sr.height(); y++)
    {
        const Pixel* s = reinterpret_cast<const Pixel*>(src.row(sr.top + y)) + sr.left;
        Pixel* d = reinterpret_cast<Pixel*>(dst.row(dy + y)) + dx;
        if (!key)
        {
            std::memmove(d, s, (size_t)w * sizeof(Pixel));
            continue;
        }
        for (int x = 0; x < w; x++)
        {
            Pixel v = s[x];
            if (v < low || v > high) d[x] = v;
        }
    }
}
}  // namespace

bool blit_valid(int dstW, int dstH, int dstBpp, int dstX, int dstY,
                int srcW, int srcH, int srcBpp, const Rect& srcRect)
{
    if (dstW <= 0 || dstH <= 0 || srcW <= 0 || srcH <= 0 || dstBpp != srcBpp) return false;
    if (!inside(srcRect, srcW, srcH)) return false;
    Rect dr = {dstX, dstY, dstX + srcRect.width(), dstY + srcRect.height()};
    return inside(dr, dstW, dstH);
}

bool stretch_valid(int dstW, int dstH, int dstBpp, const Rect& dstRect,
                   int srcW, int srcH, int srcBpp, const Rect& srcRect)
{
    if (dstW <= 0 || dstH <= 0 || srcW <= 0 || srcH <= 0 || dstBpp != srcBpp) return false;
    return inside(srcRect, srcW, srcH) && inside(dstRect, dstW, dstH);
}

bool blit(Surface& dst, int dstX, int dstY, const Surface& src, const Rect& srcRect, const ColorKey& key)
{
    if (!blit_valid(dst.width(), dst.height(), dst.bpp(), dstX, dstY, src.width(), src.height(), src.bpp(), srcRect))
        return false;
    if (src.bpp() == 8) copy_rows<uint8_t>(dst, dstX, dstY, src, srcRect, key.enabled, key.low, key.high);
    else copy_rows<uint16_t>(dst, dstX, dstY, src, srcRect, key.enabled, key.low, key.high);
    return true;
}

bool stretch_blit(Surface& dst, const Rect& dstRect, const Surface& src, const Rect& srcRect)
{
    if (!stretch_valid(dst.width(), dst.height(), dst.bpp(), dstRect, src.width(), src.height(), src.bpp(), srcRect))
        return false;
    const int dw = dstRect.width(), dh = dstRect.height();
    const int sw = srcRect.width(), sh = srcRect.height();
    for (int y = 0; y < dh; y++)
    {
        int sy = srcRect.top + (int)((long long)y * sh / dh);
        for (int x = 0; x < dw; x++)
        {
            int sx = srcRect.left + (int)((long long)x * sw / dw);
            dst.set_pixel(dstRect.left + x, dstRect.top + y, src.pixel(sx, sy));
        }
    }
    return true;
}

uint16_t rgb_to_565(Rgb c)
{
    return (uint16_t)(((c.r >> 3) << 11) | ((c.g >> 2) << 5) | (c.b >> 3));
}

Rgb rgb_from_565(uint16_t v)
{
    uint8_t r = (v >> 11) & 0x1f, g = (v >> 5) & 0x3f, b = v & 0x1f;
    return Rgb{(uint8_t)((r << 3) | (r >> 2)), (uint8_t)((g << 2) | (g >> 4)), (uint8_t)((b << 3) | (b >> 2))};
}

uint8_t nearest_index(const Palette& pal, Rgb c)
{
    for (int i = 0; i < 256; i++)
        if (pal[i].r == c.r && pal[i].g == c.g && pal[i].b == c.b) return (uint8_t)i;
    int best = 0, bestDelta = 1 << 30;
    for (int i = 0; i < 256; i++)
    {
        int d = std::abs(c.r - pal[i].r) + std::abs(c.g - pal[i].g) + std::abs(c.b - pal[i].b);
        if (d < bestDelta) { bestDelta = d; best = i; }
    }
    return (uint8_t)best;
}

uint32_t to_native(const Surface& like, const Palette& pal, Rgb c)
{
    return like.bpp() == 8 ? nearest_index(pal, c) : rgb_to_565(c);
}

Rgb from_native(const Surface& like, const Palette& pal, uint32_t v)
{
    return like.bpp() == 8 ? pal[v & 0xff] : rgb_from_565((uint16_t)v);
}

void to_argb8888(const Surface& s, const Palette& pal, uint32_t* out, int outPitchPixels)
{
    for (int y = 0; y < s.height(); y++)
    {
        uint32_t* o = out + (size_t)y * outPitchPixels;
        for (int x = 0; x < s.width(); x++)
        {
            Rgb c = from_native(s, pal, s.pixel(x, y));
            o[x] = 0xff000000u | ((uint32_t)c.r << 16) | ((uint32_t)c.g << 8) | c.b;
        }
    }
}
}  // namespace blupi::render
