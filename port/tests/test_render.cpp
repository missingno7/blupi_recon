// Reference-renderer unit tests: DirectDraw blit rules the game depends on.

#include "render/bmp.h"
#include "render/device.h"
#include "render/surface.h"

#include <cstdio>
#include <vector>

using namespace blupi::render;

static int failures = 0;

static void check(bool ok, const char* what)
{
    std::printf("%s %s\n", ok ? "ok  " : "FAIL", what);
    if (!ok) failures++;
}

int main()
{
    // Colour-keyed copy, 8 bpp.
    Surface src(4, 1, 8), dst(4, 1, 8);
    for (int x = 0; x < 4; x++) src.set_pixel(x, 0, (uint32_t)(x + 1));
    dst.fill(9);
    ColorKey key2{true, 2, 2}, none;
    check(blit(dst, 0, 0, src, Rect{0, 0, 4, 1}, key2), "keyed blit succeeds");
    check(dst.pixel(0, 0) == 1 && dst.pixel(1, 0) == 9 && dst.pixel(2, 0) == 3, "key pixel is skipped");
    check(blit(dst, 0, 0, src, Rect{0, 0, 4, 1}, none) && dst.pixel(1, 0) == 2, "opaque blit ignores key");

    // Key range is inclusive.
    dst.fill(9);
    blit(dst, 0, 0, src, Rect{0, 0, 4, 1}, ColorKey{true, 2, 3});
    check(dst.pixel(1, 0) == 9 && dst.pixel(2, 0) == 9 && dst.pixel(3, 0) == 4, "key range low..high");

    // Out-of-surface rectangles fail without drawing (DDERR_INVALIDRECT).
    dst.fill(9);
    check(!blit(dst, 2, 0, src, Rect{0, 0, 4, 1}, none) && dst.pixel(2, 0) == 9, "dst overflow rejected");
    check(!blit(dst, 0, 0, src, Rect{0, 0, 5, 1}, none), "src overflow rejected");
    check(!blit(dst, 0, 0, src, Rect{1, 0, 1, 1}, none), "empty rect rejected");

    // Mixed formats are refused.
    Surface s16(4, 1, 16);
    check(!blit(s16, 0, 0, src, Rect{0, 0, 4, 1}, none), "bpp mismatch rejected");

    // RGB565 conversion: truncation in, replication out.
    check(rgb_to_565(Rgb{0, 0, 255}) == 0x001f && rgb_to_565(Rgb{255, 255, 255}) == 0xffff, "rgb_to_565");
    Rgb w = rgb_from_565(0xffff);
    check(w.r == 255 && w.g == 255 && w.b == 255, "rgb_from_565 white");

    // Nearest-neighbour stretch 4x1 -> 2x1 takes pixels 0 and 2.
    Surface half(2, 1, 8);
    check(stretch_blit(half, Rect{0, 0, 2, 1}, src, Rect{0, 0, 4, 1}), "stretch succeeds");
    check(half.pixel(0, 0) == 1 && half.pixel(1, 0) == 3, "stretch samples floor(x*sw/dw)");

    // Palette lookup prefers an exact match.
    Palette pal = {};
    pal[5] = Rgb{0, 0, 255};
    pal[7] = Rgb{0, 0, 255};
    check(nearest_index(pal, Rgb{0, 0, 255}) == 5, "first exact palette match");

    // 2x2 8 bpp bottom-up BMP.
    std::vector<uint8_t> bmp(54 + 1024 + 8, 0);
    bmp[0] = 'B'; bmp[1] = 'M';
    bmp[10] = (uint8_t)((54 + 1024) & 0xff); bmp[11] = (uint8_t)((54 + 1024) >> 8);
    bmp[14] = 40; bmp[18] = 2; bmp[22] = 2; bmp[26] = 1; bmp[28] = 8;
    bmp[54 + 4 * 1 + 2] = 200;            // palette[1] = red 200
    bmp[54 + 1024 + 0] = 1;               // bottom row, x=0
    bmp[54 + 1024 + 4 + 1] = 1;           // top row, x=1
    Bitmap img;
    check(decode_bmp(bmp.data(), bmp.size(), img), "decode 8 bpp BMP");
    check(img.indices[1] == 1 && img.indices[2] == 1 && img.indices[0] == 0, "BMP rows are flipped to top-down");
    check(img.palette[1].r == 200 && img.palette[1].g == 0, "BMP palette is BGRA");

    // Device commands: recording and replaying reproduces direct execution,
    // and a dropped command is noticed.
    {
        Surface tiles(4, 1, 8);
        for (int x = 0; x < 4; x++) tiles.set_pixel(x, 0, (uint32_t)(x + 1));
        SoftwareDevice direct, replayed, broken;
        Recorder recorder(&direct);
        recorder.submit(make_create(kBackBuffer, 4, 1, 8));
        recorder.submit(make_upload(1, tiles));
        recorder.submit(make_blit(kBackBuffer, 0, 0, 1, Rect{0, 0, 4, 1}, ColorKey{}));
        recorder.submit(make_blit(kBackBuffer, 1, 0, 1, Rect{2, 0, 4, 1}, ColorKey{true, 3, 3}));
        replay(recorder.commands(), replayed);
        std::vector<Command> missing(recorder.commands().begin(), recorder.commands().end() - 1);
        replay(missing, broken);
        const Surface* a = direct.surface(kBackBuffer);
        const Surface* b = replayed.surface(kBackBuffer);
        const Surface* c = broken.surface(kBackBuffer);
        check(a && b && a->pixel(1, 0) == 2 && a->pixel(2, 0) == 4, "direct device draws the keyed blit");
        bool same = true, differs = false;
        for (int x = 0; x < 4; x++)
        {
            same = same && a->pixel(x, 0) == b->pixel(x, 0);
            differs = differs || a->pixel(x, 0) != c->pixel(x, 0);
        }
        check(same, "replayed commands equal direct execution");
        check(differs, "a missing command changes the result");
    }

    return failures == 0 ? 0 : 1;
}
