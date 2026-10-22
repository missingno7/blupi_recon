// Minimal reader for the game's .blp images (uncompressed Windows BMP,
// 8 bpp with colour table or 24 bpp), replacing LoadImage(LR_LOADFROMFILE).
#ifndef BLUPI_RENDER_BMP_H
#define BLUPI_RENDER_BMP_H

#include "render/surface.h"

#include <cstddef>
#include <cstdint>
#include <vector>

namespace blupi::render
{
struct Bitmap
{
    int width = 0;
    int height = 0;
    int bpp = 0;                  // 8 or 24
    Rgb palette[256] = {};        // bpp == 8
    int paletteSize = 0;
    std::vector<uint8_t> indices; // bpp == 8, top-down rows
    std::vector<Rgb> rgb;         // bpp == 24, top-down rows

    Rgb color(int x, int y) const
    {
        size_t i = (size_t)y * width + x;
        return bpp == 8 ? palette[indices[i]] : rgb[i];
    }
};

bool decode_bmp(const uint8_t* data, size_t size, Bitmap& out);
}

#endif
