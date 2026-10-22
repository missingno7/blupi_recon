// On-screen touch controls (see touch.h).

#include "platform/touch.h"

#include <cmath>
#include <map>

namespace blupi::host::touch
{
namespace
{
// VK numbering, as the keyboard sends it (platform/host.cpp to_virtual_key).
enum Dir { LEFT, RIGHT, UP, DOWN, DIRS };
const int kDirKeys[DIRS] = { 0x25, 0x27, 0x26, 0x28 };
enum Button { JUMP, FIRE, PAUSE, BUTTONS, NONE = -1 };
const int kButtonKeys[BUTTONS] = { 0x11, 0x20, 0x1B };   // Ctrl, Space, Escape

// Joystick: how far from the centre (in base radii) a direction counts.
// Sideways is the main movement; up (doors, ladders) and down need a clearer
// push, so walking with the thumb slightly off the axis does not also climb.
constexpr float kSideways = 0.30f;
constexpr float kVertical = 0.55f;

struct Circle
{
    float x, y, r;
};

struct Layout
{
    Circle stick;               // joystick base
    Circle buttons[BUTTONS];
    float  stickZoneRight;      // a finger down left of this (and low enough) takes the joystick
};

Mode g_mode = Mode::Off;
SDL_FRect g_frame = {};         // the game frame, from the last draw
bool g_overlay = true;          // no room beside the frame

std::map<SDL_FingerID, Button> g_buttonFingers;
int g_buttonHeld[BUTTONS] = {};
SDL_FingerID g_stickFinger = 0;
bool g_stickHeld = false;
float g_knobX = 0, g_knobY = 0; // knob offset from the centre, in base radii (clamped to 1)
bool g_dirHeld[DIRS] = {};

// Editor pad: one finger per arrow button; a finger may slide to another.
std::map<SDL_FingerID, int> g_padFingers;   // finger -> Dir
int g_padHeld[DIRS] = {};
std::vector<uint32_t> g_padImages[2][DIRS]; // [pressed][dir]
int g_padImageSize = 0;
SDL_Texture* g_padTextures[2][DIRS] = {};
bool g_padTexturesStale = false;

// With bars beside the frame wide enough, the joystick fills the left bar and
// jump, fire and pause the right one. Otherwise (widescreen, a 4:3 screen)
// they lie over the frame's corners, smaller and lighter.
Layout layout(int w, int h)
{
    Layout l;
    const float m = h * 0.04f;
    const float left = g_frame.w > 0 ? g_frame.x : 0;
    const float right = g_frame.w > 0 ? w - (g_frame.x + g_frame.w) : 0;
    const float bar = left < right ? left : right;
    float s = (bar - 2 * m) / 4.6f;           // unit size
    g_overlay = s < h * 0.055f;
    if (g_overlay) s = h * 0.1f;
    if (s > h * 0.13f) s = h * 0.13f;

    const float stickR = 2.0f * s;
    const float cx = g_overlay ? m + stickR * 1.1f : left / 2;
    l.stick = { cx, h - m - stickR * 1.15f, stickR };
    l.stickZoneRight = g_overlay ? cx + stickR * 1.5f : left;

    const float rx = g_overlay ? w - m - 1.4f * s : w - right / 2;
    l.buttons[JUMP]  = { rx, h - m - 1.4f * s, s * 1.3f };
    l.buttons[FIRE]  = { rx, h - m - 4.3f * s, s };
    l.buttons[PAUSE] = { rx, m + 0.7f * s, s * 0.6f };
    return l;
}

Button hit_button(const Layout& l, float x, float y)
{
    Button best = NONE;
    float bestD = 0;
    for (int i = 0; i < BUTTONS; i++)
    {
        const Circle& c = l.buttons[i];
        float d = std::hypot(x - c.x, y - c.y) / c.r;
        if (d <= 1.3f && (best == NONE || d < bestD)) { best = (Button)i; bestD = d; }
    }
    return best;
}

// Editor pad: a cross of four buttons, bottom right (the right bar, or the
// frame's corner). Button sides are whole multiples of the 40-pixel sprite
// where they fit, so the game's pixels stay sharp.
struct Pad
{
    SDL_FRect rects[DIRS];
};

Pad pad_layout(int w, int h)
{
    const float m = h * 0.04f;
    const float right = g_frame.w > 0 ? w - (g_frame.x + g_frame.w) : 0;
    float unit = (right - 2 * m) / 3.1f;                 // cross: 3 buttons wide (and a little gap)
    const bool overlay = unit < h * 0.08f;
    if (overlay) unit = h * 0.12f;
    if (unit > h * 0.2f) unit = h * 0.2f;
    float b = unit >= 80 ? 40.0f * SDL_floorf(unit / 40.0f) : unit;   // button side
    const float gap = b * 0.05f;
    const float cx = overlay ? w - m - 1.5f * b - gap : w - right / 2;
    const float cy = h - m - 1.5f * b - gap;
    Pad p;
    p.rects[LEFT]  = { cx - 1.5f * b - gap, cy - 0.5f * b, b, b };
    p.rects[RIGHT] = { cx + 0.5f * b + gap, cy - 0.5f * b, b, b };
    p.rects[UP]    = { cx - 0.5f * b, cy - 1.5f * b - gap, b, b };
    p.rects[DOWN]  = { cx - 0.5f * b, cy + 0.5f * b + gap, b, b };
    return p;
}

int hit_pad(const Pad& p, float x, float y)
{
    for (int d = 0; d < DIRS; d++)
    {
        const SDL_FRect& r = p.rects[d];
        const float g = r.w * 0.1f;   // a little beyond the edge counts
        if (x >= r.x - g && x < r.x + r.w + g && y >= r.y - g && y < r.y + r.h + g) return d;
    }
    return -1;
}

void pad_press(int d, std::vector<KeyChange>& keys)
{
    if (d >= 0 && g_padHeld[d]++ == 0) keys.push_back({ kDirKeys[d], true });
}

void pad_release(int d, std::vector<KeyChange>& keys)
{
    if (d >= 0 && g_padHeld[d] > 0 && --g_padHeld[d] == 0) keys.push_back({ kDirKeys[d], false });
}

void release_pad(std::vector<KeyChange>& keys)
{
    for (int d = 0; d < DIRS; d++)
    {
        if (g_padHeld[d] > 0) keys.push_back({ kDirKeys[d], false });
        g_padHeld[d] = 0;
    }
    g_padFingers.clear();
}

bool in_stick_zone(const Layout& l, float x, float y, int h)
{
    if (std::hypot(x - l.stick.x, y - l.stick.y) <= l.stick.r * 1.5f) return true;
    return x < l.stickZoneRight && y > h * 0.3f;
}

void set_dir(int d, bool on, std::vector<KeyChange>& keys)
{
    if (g_dirHeld[d] == on) return;
    g_dirHeld[d] = on;
    keys.push_back({ kDirKeys[d], on });
}

// The knob position decides the held direction keys (up to two: diagonals).
void update_stick(const Layout& l, float x, float y, std::vector<KeyChange>& keys)
{
    float dx = (x - l.stick.x) / l.stick.r, dy = (y - l.stick.y) / l.stick.r;
    const float len = std::hypot(dx, dy);
    if (len > 1.0f) { dx /= len; dy /= len; }
    g_knobX = dx;
    g_knobY = dy;
    set_dir(LEFT, dx < -kSideways, keys);
    set_dir(RIGHT, dx > kSideways, keys);
    set_dir(UP, dy < -kVertical, keys);
    set_dir(DOWN, dy > kVertical, keys);
}

void release_stick(std::vector<KeyChange>& keys)
{
    for (int d = 0; d < DIRS; d++) set_dir(d, false, keys);
    g_stickHeld = false;
    g_knobX = g_knobY = 0;
}

void fill_circle(SDL_Renderer* r, const Circle& c, SDL_FColor color)
{
    const int n = 40;
    SDL_Vertex v[n + 2];
    int idx[n * 3];
    v[0] = { { c.x, c.y }, color, { 0, 0 } };
    for (int i = 0; i <= n; i++)
    {
        float a = 6.2831853f * i / n;
        v[i + 1] = { { c.x + c.r * std::cos(a), c.y + c.r * std::sin(a) }, color, { 0, 0 } };
    }
    for (int i = 0; i < n; i++) { idx[i * 3] = 0; idx[i * 3 + 1] = i + 1; idx[i * 3 + 2] = i + 2; }
    SDL_RenderGeometry(r, nullptr, v, n + 2, idx, n * 3);
}

void triangle(SDL_Renderer* r, float x0, float y0, float x1, float y1, float x2, float y2, SDL_FColor color)
{
    SDL_Vertex v[3] = { { { x0, y0 }, color, { 0, 0 } }, { { x1, y1 }, color, { 0, 0 } },
                        { { x2, y2 }, color, { 0, 0 } } };
    SDL_RenderGeometry(r, nullptr, v, 3, nullptr, 0);
}

// Small arrow at (x, y) pointing along (dx, dy).
void arrow(SDL_Renderer* r, float x, float y, float size, float dx, float dy, SDL_FColor color)
{
    const float px = -dy, py = dx;
    triangle(r, x + dx * size, y + dy * size,
                x - dx * size * 0.6f + px * size * 0.8f, y - dy * size * 0.6f + py * size * 0.8f,
                x - dx * size * 0.6f - px * size * 0.8f, y - dy * size * 0.6f - py * size * 0.8f, color);
}
}  // namespace

void set_mode(Mode mode, std::vector<KeyChange>& keys)
{
    if (mode == g_mode) return;
    g_mode = mode;
    release_stick(keys);
    for (int i = 0; i < BUTTONS; i++)
    {
        if (g_buttonHeld[i] > 0) keys.push_back({ kButtonKeys[i], false });
        g_buttonHeld[i] = 0;
    }
    g_buttonFingers.clear();
    release_pad(keys);
}

Mode mode()
{
    return g_mode;
}

void set_editor_images(const std::vector<uint32_t> released[4], const std::vector<uint32_t> pressed[4], int size)
{
    for (int d = 0; d < DIRS; d++)
    {
        g_padImages[0][d] = released[d];
        g_padImages[1][d] = pressed[d];
    }
    g_padImageSize = size;
    g_padTexturesStale = true;   // made into textures at the next draw
}

bool finger(int type, SDL_FingerID id, float x, float y, int w, int h, std::vector<KeyChange>& keys)
{
    if (g_mode == Mode::Editor)
    {
        const Pad p = pad_layout(w, h);
        auto it = g_padFingers.find(id);
        if (type == 0)
        {
            const int d = hit_pad(p, x, y);
            if (d < 0) return false;
            g_padFingers[id] = d;
            pad_press(d, keys);
            return true;
        }
        if (it == g_padFingers.end()) return false;
        if (type == 2)
        {
            pad_release(it->second, keys);
            g_padFingers.erase(it);
            return true;
        }
        const int d = hit_pad(p, x, y);
        if (d >= 0 && d != it->second)
        {
            pad_release(it->second, keys);
            pad_press(d, keys);
            it->second = d;
        }
        return true;
    }

    const Layout l = layout(w, h);
    if (type == 0)
    {
        if (g_mode != Mode::Play) return false;
        Button b = hit_button(l, x, y);
        if (b != NONE)
        {
            g_buttonFingers[id] = b;
            if (g_buttonHeld[b]++ == 0) keys.push_back({ kButtonKeys[b], true });
            return true;
        }
        if (!g_stickHeld && in_stick_zone(l, x, y, h))
        {
            g_stickHeld = true;
            g_stickFinger = id;
            update_stick(l, x, y, keys);
            return true;
        }
        return false;
    }
    if (g_stickHeld && id == g_stickFinger)
    {
        if (type == 2) release_stick(keys);
        else update_stick(l, x, y, keys);
        return true;
    }
    auto it = g_buttonFingers.find(id);
    if (it == g_buttonFingers.end()) return false;   // not ours (the mouse finger)
    if (type == 2)
    {
        if (g_buttonHeld[it->second] > 0 && --g_buttonHeld[it->second] == 0)
            keys.push_back({ kButtonKeys[it->second], false });
        g_buttonFingers.erase(it);
    }
    return true;
}

namespace
{
void draw_pad(SDL_Renderer* renderer, int w, int h)
{
    if (g_padTexturesStale)
    {
        for (int s = 0; s < 2; s++)
        {
            for (int d = 0; d < DIRS; d++)
            {
                if (g_padTextures[s][d]) SDL_DestroyTexture(g_padTextures[s][d]);
                g_padTextures[s][d] = nullptr;
                const std::vector<uint32_t>& img = g_padImages[s][d];
                if (g_padImageSize <= 0 || img.size() != (size_t)g_padImageSize * g_padImageSize) continue;
                SDL_Texture* t = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STATIC,
                                                   g_padImageSize, g_padImageSize);
                if (t == nullptr) continue;
                SDL_UpdateTexture(t, nullptr, img.data(), g_padImageSize * 4);
                SDL_SetTextureBlendMode(t, SDL_BLENDMODE_BLEND);
                SDL_SetTextureScaleMode(t, SDL_SCALEMODE_NEAREST);
                g_padTextures[s][d] = t;
            }
        }
        g_padTexturesStale = false;
    }

    const Pad p = pad_layout(w, h);
    for (int d = 0; d < DIRS; d++)
    {
        const bool down = g_padHeld[d] > 0;
        SDL_Texture* t = g_padTextures[down ? 1 : 0][d];
        if (t)
        {
            SDL_RenderTexture(renderer, t, nullptr, &p.rects[d]);
            continue;
        }
        // No game images: a plain button.
        const SDL_FRect& r = p.rects[d];
        fill_circle(renderer, Circle{ r.x + r.w / 2, r.y + r.h / 2, r.w / 2 },
                    SDL_FColor{ 1, 1, 1, down ? 0.45f : 0.22f });
        const float dx = d == LEFT ? -1.0f : d == RIGHT ? 1.0f : 0.0f;
        const float dy = d == UP ? -1.0f : d == DOWN ? 1.0f : 0.0f;
        arrow(renderer, r.x + r.w / 2, r.y + r.h / 2, r.w * 0.25f, dx, dy, SDL_FColor{ 1, 1, 1, 0.8f });
    }
}
}  // namespace

void draw(SDL_Renderer* renderer, int w, int h, const SDL_FRect& frame)
{
    g_frame = frame;
    if (g_mode == Mode::Editor) { draw_pad(renderer, w, h); return; }
    if (g_mode != Mode::Play) return;
    const Layout l = layout(w, h);
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    const float dim = g_overlay ? 0.6f : 1.0f;   // lighter over the game itself

    // Joystick: base, direction marks lit while held, knob.
    const Circle& s = l.stick;
    fill_circle(renderer, s, SDL_FColor{ 1, 1, 1, 0.16f * dim });
    const float a = s.r * 0.12f, off = s.r * 0.78f;
    const SDL_FColor lit{ 1, 1, 1, 0.9f * dim }, unlit{ 1, 1, 1, 0.45f * dim };
    arrow(renderer, s.x - off, s.y, a, -1, 0, g_dirHeld[LEFT] ? lit : unlit);
    arrow(renderer, s.x + off, s.y, a, 1, 0, g_dirHeld[RIGHT] ? lit : unlit);
    arrow(renderer, s.x, s.y - off, a, 0, -1, g_dirHeld[UP] ? lit : unlit);
    arrow(renderer, s.x, s.y + off, a, 0, 1, g_dirHeld[DOWN] ? lit : unlit);
    const Circle knob{ s.x + g_knobX * s.r * 0.55f, s.y + g_knobY * s.r * 0.55f, s.r * 0.42f };
    fill_circle(renderer, knob, SDL_FColor{ 1, 1, 1, (g_stickHeld ? 0.55f : 0.35f) * dim });

    for (int i = 0; i < BUTTONS; i++)
    {
        const Circle& c = l.buttons[i];
        const bool down = g_buttonHeld[i] > 0;
        fill_circle(renderer, c, SDL_FColor{ 1, 1, 1, (down ? 0.45f : 0.22f) * dim });
        const SDL_FColor mark{ 1, 1, 1, (down ? 0.95f : 0.7f) * dim };
        switch (i)
        {
        case JUMP:
            arrow(renderer, c.x, c.y + c.r * 0.12f, c.r * 0.38f, 0, -1, mark);
            arrow(renderer, c.x, c.y - c.r * 0.3f, c.r * 0.38f, 0, -1, mark);
            break;
        case FIRE:
            fill_circle(renderer, Circle{ c.x, c.y, c.r * 0.35f }, mark);
            break;
        case PAUSE:
        {
            const float bw = c.r * 0.22f, bh = c.r * 0.9f;
            SDL_SetRenderDrawColorFloat(renderer, mark.r, mark.g, mark.b, mark.a);
            SDL_FRect bars[2] = { { c.x - bw * 1.6f, c.y - bh / 2, bw, bh },
                                  { c.x + bw * 0.6f, c.y - bh / 2, bw, bh } };
            SDL_RenderFillRects(renderer, bars, 2);
            break;
        }
        }
    }
}
}  // namespace blupi::host::touch
