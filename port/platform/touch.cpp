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

bool g_active = false;
SDL_FRect g_frame = {};         // the game frame, from the last draw
bool g_overlay = true;          // no room beside the frame

std::map<SDL_FingerID, Button> g_buttonFingers;
int g_buttonHeld[BUTTONS] = {};
SDL_FingerID g_stickFinger = 0;
bool g_stickHeld = false;
float g_knobX = 0, g_knobY = 0; // knob offset from the centre, in base radii (clamped to 1)
bool g_dirHeld[DIRS] = {};

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

void set_active(bool on, std::vector<KeyChange>& keys)
{
    if (on == g_active) return;
    g_active = on;
    if (on) return;
    release_stick(keys);
    for (int i = 0; i < BUTTONS; i++)
    {
        if (g_buttonHeld[i] > 0) keys.push_back({ kButtonKeys[i], false });
        g_buttonHeld[i] = 0;
    }
    g_buttonFingers.clear();
}

bool active()
{
    return g_active;
}

bool finger(int type, SDL_FingerID id, float x, float y, int w, int h, std::vector<KeyChange>& keys)
{
    const Layout l = layout(w, h);
    if (type == 0)
    {
        if (!g_active) return false;
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

void draw(SDL_Renderer* renderer, int w, int h, const SDL_FRect& frame)
{
    g_frame = frame;
    if (!g_active) return;
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
