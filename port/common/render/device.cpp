#include "render/device.h"

namespace blupi::render
{
void SoftwareDevice::submit(const Command& c)
{
    switch (c.kind)
    {
    case Command::Create:
        m_surfaces[c.dst] = Surface(c.width, c.height, c.bpp);
        break;
    case Command::Destroy:
        m_surfaces.erase(c.dst);
        break;
    case Command::Upload:
        m_surfaces[c.dst] = *c.pixels;
        break;
    case Command::Palette:
        for (int i = 0; i < 256; i++) m_palette[i] = c.palette[i];
        break;
    case Command::Blit:
    {
        auto d = m_surfaces.find(c.dst), s = m_surfaces.find(c.src);
        if (d != m_surfaces.end() && s != m_surfaces.end())
            blit(d->second, c.dstRect.left, c.dstRect.top, s->second, c.srcRect, c.key);
        break;
    }
    case Command::Stretch:
    {
        auto d = m_surfaces.find(c.dst), s = m_surfaces.find(c.src);
        if (d != m_surfaces.end() && s != m_surfaces.end())
            stretch_blit(d->second, c.dstRect, s->second, c.srcRect);
        break;
    }
    }
}

const Surface* SoftwareDevice::surface(SurfaceId id) const
{
    auto it = m_surfaces.find(id);
    return it == m_surfaces.end() ? nullptr : &it->second;
}

void Recorder::submit(const Command& command)
{
    m_commands.push_back(command);
    if (m_next) m_next->submit(command);
}

void replay(const std::vector<Command>& commands, Device& device)
{
    for (const Command& c : commands) device.submit(c);
}

static bool same_rect(const Rect& a, const Rect& b)
{
    return a.left == b.left && a.top == b.top && a.right == b.right && a.bottom == b.bottom;
}

bool same_command(const Command& a, const Command& b)
{
    if (a.kind != b.kind || a.dst != b.dst || a.src != b.src) return false;
    switch (a.kind)
    {
    case Command::Create:
        return a.width == b.width && a.height == b.height && a.bpp == b.bpp;
    case Command::Destroy:
        return true;
    case Command::Upload:
    {
        const Surface& x = *a.pixels;
        const Surface& y = *b.pixels;
        if (x.width() != y.width() || x.height() != y.height() || x.bpp() != y.bpp()) return false;
        for (int r = 0; r < x.height(); r++)
            for (int c = 0; c < x.width(); c++)
                if (x.pixel(c, r) != y.pixel(c, r)) return false;
        return true;
    }
    case Command::Palette:
        for (int i = 0; i < 256; i++)
            if (a.palette[i].r != b.palette[i].r || a.palette[i].g != b.palette[i].g || a.palette[i].b != b.palette[i].b)
                return false;
        return true;
    case Command::Blit:
        return same_rect(a.srcRect, b.srcRect) && same_rect(a.dstRect, b.dstRect) &&
               a.key.enabled == b.key.enabled && (!a.key.enabled || (a.key.low == b.key.low && a.key.high == b.key.high));
    case Command::Stretch:
        return same_rect(a.srcRect, b.srcRect) && same_rect(a.dstRect, b.dstRect);
    }
    return false;
}

Command make_create(SurfaceId id, int width, int height, int bpp)
{
    Command c{Command::Create};
    c.dst = id;
    c.width = width;
    c.height = height;
    c.bpp = bpp;
    return c;
}

Command make_destroy(SurfaceId id)
{
    Command c{Command::Destroy};
    c.dst = id;
    return c;
}

Command make_upload(SurfaceId id, Surface pixels)
{
    Command c{Command::Upload};
    c.dst = id;
    c.width = pixels.width();
    c.height = pixels.height();
    c.bpp = pixels.bpp();
    c.pixels = std::make_shared<const Surface>(std::move(pixels));
    return c;
}

Command make_palette(const Palette& palette)
{
    Command c{Command::Palette};
    std::shared_ptr<Rgb[]> copy(new Rgb[256]);
    for (int i = 0; i < 256; i++) copy[i] = palette[i];
    c.palette = copy;
    return c;
}

Command make_blit(SurfaceId dst, int x, int y, SurfaceId src, const Rect& srcRect, const ColorKey& key)
{
    Command c{Command::Blit};
    c.dst = dst;
    c.src = src;
    c.srcRect = srcRect;
    c.dstRect = Rect{x, y, x + srcRect.width(), y + srcRect.height()};
    c.key = key;
    return c;
}

Command make_stretch(SurfaceId dst, const Rect& dstRect, SurfaceId src, const Rect& srcRect)
{
    Command c{Command::Stretch};
    c.dst = dst;
    c.src = src;
    c.srcRect = srcRect;
    c.dstRect = dstRect;
    return c;
}
}  // namespace blupi::render
