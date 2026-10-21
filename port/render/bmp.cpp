#include "render/bmp.h"

namespace blupi::render
{
namespace
{
uint32_t u32(const uint8_t* p) { return p[0] | (p[1] << 8) | (p[2] << 16) | ((uint32_t)p[3] << 24); }
uint16_t u16(const uint8_t* p) { return (uint16_t)(p[0] | (p[1] << 8)); }
}

bool decode_bmp(const uint8_t* data, size_t size, Bitmap& out)
{
    if (size < 54 || data[0] != 'B' || data[1] != 'M') return false;
    uint32_t pixelOffset = u32(data + 10);
    uint32_t headerSize = u32(data + 14);
    int32_t width = (int32_t)u32(data + 18);
    int32_t height = (int32_t)u32(data + 22);
    uint16_t bpp = u16(data + 28);
    uint32_t compression = u32(data + 30);
    uint32_t colorsUsed = u32(data + 46);
    if (compression != 0 || width <= 0 || height == 0 || (bpp != 8 && bpp != 24)) return false;

    bool topDown = height < 0;
    if (topDown) height = -height;
    size_t stride = ((size_t)width * bpp / 8 + 3) & ~(size_t)3;
    if (pixelOffset + stride * height > size) return false;

    out.width = width;
    out.height = height;
    out.bpp = bpp;
    if (bpp == 8)
    {
        out.paletteSize = colorsUsed ? (int)colorsUsed : 256;
        if (out.paletteSize > 256) return false;
        const uint8_t* table = data + 14 + headerSize;
        if (table + out.paletteSize * 4 > data + size) return false;
        for (int i = 0; i < out.paletteSize; i++)
            out.palette[i] = Rgb{table[i * 4 + 2], table[i * 4 + 1], table[i * 4 + 0]};
        out.indices.resize((size_t)width * height);
    }
    else
    {
        out.rgb.resize((size_t)width * height);
    }

    for (int y = 0; y < height; y++)
    {
        int srcRow = topDown ? y : height - 1 - y;
        const uint8_t* p = data + pixelOffset + stride * srcRow;
        for (int x = 0; x < width; x++)
        {
            size_t i = (size_t)y * width + x;
            if (bpp == 8) out.indices[i] = p[x];
            else out.rgb[i] = Rgb{p[x * 3 + 2], p[x * 3 + 1], p[x * 3 + 0]};
        }
    }
    return true;
}
}
