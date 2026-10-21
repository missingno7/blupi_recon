// Rendering device: the seam between CPixmap (port/game/pixmap.cpp) and the
// backends that produce pixels (port/docs/ROADMAP.md stage 1).
//
// CPixmap decides everything DirectDraw decided (which blits are valid, which
// colour key applies) from its own CPU-side registry of surface sizes and
// keys, then submits only valid, self-contained operations. A backend never
// reports errors and holds no state besides surface contents and the screen
// palette, so the same operations can be recorded, replayed, or executed by a
// GPU backend later.
//
// Surfaces are identified by number: CPixmap's image channels 0..99 and the
// three surfaces CPixmap owns (back buffer, screen, saved mouse background).
#ifndef BLUPI_RENDER_DEVICE_H
#define BLUPI_RENDER_DEVICE_H

#include "render/surface.h"

#include <cstdint>
#include <map>
#include <memory>
#include <vector>

namespace blupi::render
{
using SurfaceId = int;
constexpr SurfaceId kChannelCount = 100;
constexpr SurfaceId kBackBuffer = 100;
constexpr SurfaceId kScreen = 101;
constexpr SurfaceId kMouseSave = 102;

struct Command
{
    enum Kind : uint8_t { Create, Destroy, Upload, Palette, Blit, Stretch };
    Kind      kind;
    SurfaceId dst = -1;
    SurfaceId src = -1;
    int       width = 0, height = 0, bpp = 0;   // Create
    Rect      srcRect = {};                     // Blit, Stretch
    Rect      dstRect = {};                     // Blit: left/top = destination; Stretch: full rect
    ColorKey  key;                              // Blit
    std::shared_ptr<const Surface> pixels;      // Upload: whole-surface contents
    std::shared_ptr<const Rgb[]>   palette;     // Palette: 256 entries
};

class Device
{
public:
    virtual ~Device() = default;
    virtual void submit(const Command& command) = 0;
};

// Executes commands on software surfaces (the reference renderer).
class SoftwareDevice : public Device
{
public:
    void submit(const Command& command) override;

    const Surface* surface(SurfaceId id) const;
    const Palette& palette() const { return m_palette; }

private:
    std::map<SurfaceId, Surface> m_surfaces;
    Palette m_palette = {};
};

// Appends commands to a list (for replay and inspection) and forwards them.
class Recorder : public Device
{
public:
    explicit Recorder(Device* next) : m_next(next) {}
    void submit(const Command& command) override;

    std::vector<Command>& commands() { return m_commands; }

private:
    Device* m_next;
    std::vector<Command> m_commands;
};

void replay(const std::vector<Command>& commands, Device& device);

// Field-by-field equality (uploads and palettes compare contents).
bool same_command(const Command& a, const Command& b);

// Convenience constructors.
Command make_create(SurfaceId id, int width, int height, int bpp);
Command make_destroy(SurfaceId id);
Command make_upload(SurfaceId id, Surface pixels);
Command make_palette(const Palette& palette);
Command make_blit(SurfaceId dst, int x, int y, SurfaceId src, const Rect& srcRect, const ColorKey& key);
Command make_stretch(SurfaceId dst, const Rect& dstRect, SurfaceId src, const Rect& srcRect);
}  // namespace blupi::render

#endif
