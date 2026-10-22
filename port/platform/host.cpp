#include "platform/host.h"

#include "platform/icon.h"
#include "platform/touch.h"

#include <SDL3/SDL.h>

#include <deque>
#include <vector>

namespace blupi::host
{
namespace
{
SDL_Window*   g_window = nullptr;
SDL_Renderer* g_renderer = nullptr;
SDL_Texture*  g_texture = nullptr;
SDL_Cursor*   g_cursor = nullptr;
int  g_width = 0, g_height = 0;   // the game's frame
int  g_viewWidth = 0, g_frameX = 0; // the presented view; the frame's x in it (widescreen)
bool g_autoWide = false;            // view width follows the window shape
bool g_integerScale = false;
bool g_headless = false;
bool g_headlessPads = false;   // tests: gamepads without a window
bool g_fullscreen = false;
std::vector<SDL_Gamepad*> g_pads;   // in the order they were connected
std::deque<Event> g_pending;        // synthesized events (touch), before SDL's
#ifdef SDL_PLATFORM_ANDROID
const bool g_touchControls = true;  // on-screen controls in play (platform/touch.h)
#else
const bool g_touchControls = false;
#endif
SDL_FingerID g_mouseFinger = 0;     // the finger acting as the mouse, 0: none
bool g_mouseFingerDown = false;

void open_gamepad(SDL_JoystickID id)
{
    for (SDL_Gamepad* pad : g_pads)
    {
        if (SDL_GetGamepadID(pad) == id) return;
    }
    if (SDL_Gamepad* pad = SDL_OpenGamepad(id)) g_pads.push_back(pad);
}

void close_gamepad(SDL_JoystickID id)
{
    for (size_t i = 0; i < g_pads.size(); i++)
    {
        if (SDL_GetGamepadID(g_pads[i]) != id) continue;
        SDL_CloseGamepad(g_pads[i]);
        g_pads.erase(g_pads.begin() + (long)i);
        return;
    }
}

// Sets the presented view width: texture and logical presentation follow.
void apply_view_width(int width)
{
    if (width < g_width) width = g_width;
    if (width == g_viewWidth && (g_headless || g_texture != nullptr)) return;
    g_viewWidth = width;
    g_frameX = (g_viewWidth - g_width) / 2;   // until the port places the frame (set_frame_x)
    if (g_headless || g_renderer == nullptr) return;
    SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION, "view %dx%d", g_viewWidth, g_height);
    if (g_texture) SDL_DestroyTexture(g_texture);
    g_texture = SDL_CreateTexture(g_renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING,
                                  g_viewWidth, g_height);
    if (g_texture) SDL_SetTextureScaleMode(g_texture, SDL_SCALEMODE_NEAREST);
    SDL_SetRenderLogicalPresentation(g_renderer, g_viewWidth, g_height,
                                     g_integerScale ? SDL_LOGICAL_PRESENTATION_INTEGER_SCALE
                                                    : SDL_LOGICAL_PRESENTATION_LETTERBOX);
}

// Automatic widescreen: the view as wide as the window's shape allows,
// between the frame's 4:3 and 21:9, at the frame's height. An even width keeps
// the frame on whole pixels. Other shapes are letterboxed.
void update_auto_width()
{
    if (!g_autoWide || g_headless || g_window == nullptr) return;
    int w = 0, h = 0;
    if (!SDL_GetWindowSizeInPixels(g_window, &w, &h) || w <= 0 || h <= 0) return;
    int width = (int)((long long)g_height * w / h);
    if (width > kMaxViewWidth) width = kMaxViewWidth;
    width &= ~1;
    apply_view_width(width);
}

// SDL keycode -> Win32 virtual-key code (0 = not forwarded to the game).
int to_virtual_key(SDL_Keycode k)
{
    if (k >= SDLK_A && k <= SDLK_Z) return 'A' + (int)(k - SDLK_A);
    if (k >= SDLK_0 && k <= SDLK_9) return '0' + (int)(k - SDLK_0);
    if (k >= SDLK_F1 && k <= SDLK_F12) return 0x70 + (int)(k - SDLK_F1);
    switch (k)
    {
    case SDLK_BACKSPACE: return 0x08;
    case SDLK_TAB:       return 0x09;
    case SDLK_RETURN:    return 0x0D;
    case SDLK_KP_ENTER:  return 0x0D;
    case SDLK_LSHIFT: case SDLK_RSHIFT: return 0x10;
    case SDLK_LCTRL:  case SDLK_RCTRL:  return 0x11;
    case SDLK_LALT:   case SDLK_RALT:   return 0x12;
    case SDLK_PAUSE:     return 0x13;
    case SDLK_ESCAPE:    return 0x1B;
    case SDLK_AC_BACK:   return 0x1B;   // Android Back: Escape (pause, back)
    case SDLK_SPACE:     return 0x20;
    case SDLK_PAGEUP:    return 0x21;
    case SDLK_PAGEDOWN:  return 0x22;
    case SDLK_END:       return 0x23;
    case SDLK_HOME:      return 0x24;
    case SDLK_LEFT:      return 0x25;
    case SDLK_UP:        return 0x26;
    case SDLK_RIGHT:     return 0x27;
    case SDLK_DOWN:      return 0x28;
    case SDLK_INSERT:    return 0x2D;
    case SDLK_DELETE:    return 0x2E;
    case SDLK_KP_0: return 0x60; case SDLK_KP_1: return 0x61; case SDLK_KP_2: return 0x62;
    case SDLK_KP_3: return 0x63; case SDLK_KP_4: return 0x64; case SDLK_KP_5: return 0x65;
    case SDLK_KP_6: return 0x66; case SDLK_KP_7: return 0x67; case SDLK_KP_8: return 0x68;
    case SDLK_KP_9: return 0x69;
    default: return 0;
    }
}

// Unicode code point -> cp1252 byte, 0 if not representable.
int to_cp1252(uint32_t cp)
{
    if (cp < 0x80 || (cp >= 0xA0 && cp <= 0xFF)) return (int)cp;
    static const uint16_t high[32] = {
        0x20AC, 0, 0x201A, 0x0192, 0x201E, 0x2026, 0x2020, 0x2021, 0x02C6, 0x2030, 0x0160, 0x2039, 0x0152, 0, 0x017D, 0,
        0, 0x2018, 0x2019, 0x201C, 0x201D, 0x2022, 0x2013, 0x2014, 0x02DC, 0x2122, 0x0161, 0x203A, 0x0153, 0, 0x017E, 0x0178};
    for (int i = 0; i < 32; i++)
        if (high[i] == cp) return 0x80 + i;
    return 0;
}

void fill_buttons(Event& e)
{
    SDL_MouseButtonFlags state = SDL_GetMouseState(nullptr, nullptr);
    e.leftDown = (state & SDL_BUTTON_LMASK) != 0;
    e.rightDown = (state & SDL_BUTTON_RMASK) != 0;
    SDL_Keymod mod = SDL_GetModState();
    e.shift = (mod & SDL_KMOD_SHIFT) != 0;
    e.control = (mod & SDL_KMOD_CTRL) != 0;
}

void set_position(Event& e, float x, float y)
{
    e.x = (int)SDL_floorf(x) - g_frameX;   // frame coordinates
    e.y = (int)SDL_floorf(y);
    e.inside = e.x >= 0 && e.y >= 0 && e.x < g_width && e.y < g_height;
}

// Pending text input split into single cp1252 characters.
char g_text[64];
int  g_textPos = 0, g_textLen = 0;

bool translate(SDL_Event& ev, Event& out)
{
    switch (ev.type)
    {
    case SDL_EVENT_QUIT:
    case SDL_EVENT_WINDOW_CLOSE_REQUESTED:
        out = Event{EventType::Quit};
        return true;
    case SDL_EVENT_WINDOW_FOCUS_GAINED:
        out = Event{EventType::FocusGained};
        return true;
    case SDL_EVENT_WINDOW_FOCUS_LOST:
        out = Event{EventType::FocusLost};
        return true;
    case SDL_EVENT_WINDOW_PIXEL_SIZE_CHANGED:
        update_auto_width();
        return false;
    case SDL_EVENT_FINGER_DOWN:
    case SDL_EVENT_FINGER_MOTION:
    case SDL_EVENT_FINGER_UP:
    {
        if (!g_touchControls || g_renderer == nullptr) return false;
        const int type = ev.type == SDL_EVENT_FINGER_DOWN ? 0 : ev.type == SDL_EVENT_FINGER_UP ? 2 : 1;
        int ow = 0, oh = 0, ww = 0, wh = 0;
        SDL_GetRenderOutputSize(g_renderer, &ow, &oh);
        SDL_GetWindowSize(g_window, &ww, &wh);
        std::vector<touch::KeyChange> keys;
        if (touch::finger(type, ev.tfinger.fingerID, ev.tfinger.x * ow, ev.tfinger.y * oh, ow, oh, keys))
        {
            for (const touch::KeyChange& k : keys)
            {
                Event e{k.down ? EventType::KeyDown : EventType::KeyUp};
                e.key = k.vk;
                g_pending.push_back(e);
            }
            return false;
        }
        // Any other finger (one at a time) is the mouse's left button.
        if (type == 0 && g_mouseFingerDown) return false;
        if (type != 0 && (!g_mouseFingerDown || ev.tfinger.fingerID != g_mouseFinger)) return false;
        float lx = 0, ly = 0;
        SDL_RenderCoordinatesFromWindow(g_renderer, ev.tfinger.x * ww, ev.tfinger.y * wh, &lx, &ly);
        Event move{EventType::MouseMove};
        set_position(move, lx, ly);
        if (type == 0)
        {
            g_mouseFinger = ev.tfinger.fingerID;
            g_mouseFingerDown = true;
            g_pending.push_back(move);           // the pointer first moves there
            out = Event{EventType::MouseDown};
            out.button = 1;
            set_position(out, lx, ly);
            out.leftDown = true;
            g_pending.push_back(out);
            return false;
        }
        if (type == 2)
        {
            g_mouseFingerDown = false;
            out = Event{EventType::MouseUp};
            out.button = 1;
            set_position(out, lx, ly);
            g_pending.push_back(out);
            return false;
        }
        move.leftDown = true;
        g_pending.push_back(move);
        return false;
    }
    case SDL_EVENT_GAMEPAD_ADDED:
        open_gamepad(ev.gdevice.which);
        return false;
    case SDL_EVENT_GAMEPAD_REMOVED:
        close_gamepad(ev.gdevice.which);
        return false;
    case SDL_EVENT_GAMEPAD_BUTTON_DOWN:
    case SDL_EVENT_GAMEPAD_BUTTON_UP:
        // Port convenience, not seen as a joystick button by the game: Start
        // is the Escape key (pause and back), so a gamepad alone can play.
        if (ev.gbutton.button != SDL_GAMEPAD_BUTTON_START) return false;
        out = Event{ev.type == SDL_EVENT_GAMEPAD_BUTTON_DOWN ? EventType::KeyDown : EventType::KeyUp};
        out.key = 0x1B;   // VK_ESCAPE
        fill_buttons(out);
        return true;
    case SDL_EVENT_KEY_DOWN:
    case SDL_EVENT_KEY_UP:
    {
        bool down = ev.type == SDL_EVENT_KEY_DOWN;
        // Port-level shortcut, not seen by the game: Alt+Enter toggles fullscreen.
        if (down && ev.key.key == SDLK_RETURN && (ev.key.mod & SDL_KMOD_ALT))
        {
            if (!ev.key.repeat) toggle_fullscreen();
            return false;
        }
        int vk = to_virtual_key(ev.key.key);
        if (vk == 0) return false;
        out = Event{down ? EventType::KeyDown : EventType::KeyUp};
        out.key = vk;
        out.repeat = ev.key.repeat;
        fill_buttons(out);
        return true;
    }
    case SDL_EVENT_TEXT_INPUT:
    {
        g_textLen = 0;
        g_textPos = 0;
        const char* s = ev.text.text;
        while (*s && g_textLen < (int)sizeof(g_text))
        {
            Uint32 cp = SDL_StepUTF8(&s, nullptr);
            int c = to_cp1252(cp);
            if (c >= 0x20) g_text[g_textLen++] = (char)c;
        }
        return false;  // delivered by poll_event from g_text
    }
    case SDL_EVENT_MOUSE_MOTION:
        SDL_ConvertEventToRenderCoordinates(g_renderer, &ev);
        out = Event{EventType::MouseMove};
        set_position(out, ev.motion.x, ev.motion.y);
        fill_buttons(out);
        return true;
    case SDL_EVENT_MOUSE_BUTTON_DOWN:
    case SDL_EVENT_MOUSE_BUTTON_UP:
    {
        if (ev.button.button != SDL_BUTTON_LEFT && ev.button.button != SDL_BUTTON_RIGHT) return false;
        SDL_ConvertEventToRenderCoordinates(g_renderer, &ev);
        out = Event{ev.type == SDL_EVENT_MOUSE_BUTTON_DOWN ? EventType::MouseDown : EventType::MouseUp};
        out.button = ev.button.button == SDL_BUTTON_LEFT ? 1 : 2;
        set_position(out, ev.button.x, ev.button.y);
        fill_buttons(out);
        return true;
    }
    default:
        return false;
    }
}

bool next_text(Event& event)
{
    if (g_textPos >= g_textLen) return false;
    event = Event{EventType::Text};
    event.ch = (unsigned char)g_text[g_textPos++];
    return true;
}
}  // namespace

bool init(const char* title, int logicalWidth, int logicalHeight, const Options& options)
{
    g_width = logicalWidth;
    g_height = logicalHeight;
    g_viewWidth = options.viewWidth > logicalWidth ? options.viewWidth : logicalWidth;
    g_frameX = (g_viewWidth - logicalWidth) / 2;
    g_headless = options.headless;
    g_autoWide = options.autoWide;
    g_integerScale = options.integerScale;
    g_fullscreen = g_touchControls;   // touch screens: created full screen
    g_headlessPads = options.headless && options.gamepads;
    if (g_headless) return SDL_Init(g_headlessPads ? SDL_INIT_GAMEPAD : 0);

    SDL_SetHint(SDL_HINT_MOUSE_FOCUS_CLICKTHROUGH, "1");
#ifdef SDL_PLATFORM_ANDROID
    // A landscape game; without the hint SDL lets a resizable window rotate freely.
    SDL_SetHint(SDL_HINT_ORIENTATIONS, "LandscapeLeft LandscapeRight");
#endif
    // Touch: our own controls and mouse (above), not SDL's mouse emulation.
    if (g_touchControls) SDL_SetHint(SDL_HINT_TOUCH_MOUSE_EVENTS, "0");
    if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_GAMEPAD))
    {
        SDL_Log("SDL_Init failed: %s", SDL_GetError());
        return false;
    }
    int scale = options.scale < 1 ? 1 : options.scale;
    // Automatic widescreen opens a 16:9 window; resizing changes the shape.
    const int windowWidth = g_autoWide ? (logicalHeight * 16 / 9 + 1) & ~1 : g_viewWidth;
#ifdef SDL_PLATFORM_ANDROID
    const SDL_WindowFlags windowFlags = SDL_WINDOW_FULLSCREEN;   // the whole screen, system bars hidden
#else
    const SDL_WindowFlags windowFlags = SDL_WINDOW_RESIZABLE;
#endif
    if (!SDL_CreateWindowAndRenderer(title, windowWidth * scale, logicalHeight * scale,
                                     windowFlags, &g_window, &g_renderer))
    {
        SDL_Log("SDL_CreateWindowAndRenderer failed: %s", SDL_GetError());
        return false;
    }
    SDL_SetWindowMinimumSize(g_window, logicalWidth / 2, logicalHeight / 2);
#if !defined(SDL_PLATFORM_WINDOWS) && !defined(SDL_PLATFORM_ANDROID)
    // (Windows takes it from the executable's resources.)
    if (SDL_Surface* icon = game_icon())
    {
        SDL_SetWindowIcon(g_window, icon);
        SDL_DestroySurface(icon);
    }
#endif
    SDL_SetRenderLogicalPresentation(g_renderer, g_viewWidth, logicalHeight,
                                     options.integerScale ? SDL_LOGICAL_PRESENTATION_INTEGER_SCALE
                                                          : SDL_LOGICAL_PRESENTATION_LETTERBOX);
    SDL_SetRenderVSync(g_renderer, 1);
    g_texture = SDL_CreateTexture(g_renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING,
                                  g_viewWidth, logicalHeight);
    if (g_texture == nullptr) return false;
    SDL_SetTextureScaleMode(g_texture, SDL_SCALEMODE_NEAREST);
    if (options.fullscreen) toggle_fullscreen();
    update_auto_width();
    if (!text_input_on_demand()) SDL_StartTextInput(g_window);
    return true;
}

void shutdown()
{
    if (g_cursor) SDL_DestroyCursor(g_cursor);
    if (g_texture) SDL_DestroyTexture(g_texture);
    if (g_renderer) SDL_DestroyRenderer(g_renderer);
    if (g_window) SDL_DestroyWindow(g_window);
    g_cursor = nullptr;
    g_texture = nullptr;
    g_renderer = nullptr;
    g_window = nullptr;
    SDL_Quit();
}

bool headless() { return g_headless; }

uint64_t now_ns() { return SDL_GetTicksNS(); }

bool poll_event(Event& event)
{
    if (!g_pending.empty())
    {
        event = g_pending.front();
        g_pending.pop_front();
        return true;
    }
    if (g_headless && !g_headlessPads) return false;
    if (next_text(event)) return true;
    SDL_Event ev;
    while (SDL_PollEvent(&ev))
    {
        if (translate(ev, event)) return true;
        if (next_text(event)) return true;
        if (!g_pending.empty()) return poll_event(event);
    }
    return false;
}

bool wait_event(Event& event, uint64_t timeoutNs)
{
    if (g_headless) return false;
    if (poll_event(event)) return true;
    Sint32 ms = (Sint32)(timeoutNs / 1000000);
    SDL_Event ev;
    if (!SDL_WaitEventTimeout(&ev, ms)) return false;
    if (translate(ev, event)) return true;
    return next_text(event);
}

void present(const uint32_t* argb, int width)
{
    if (g_headless || g_texture == nullptr) return;
    if (width != g_viewWidth) return;   // composed before a resize: the next one fits
    SDL_UpdateTexture(g_texture, nullptr, argb, g_viewWidth * 4);
    SDL_SetRenderDrawColor(g_renderer, 0, 0, 0, 255);
    SDL_RenderClear(g_renderer);
    SDL_RenderTexture(g_renderer, g_texture, nullptr, nullptr);
    if (touch::mode() != touch::Mode::Off)
    {
        // The controls in output pixels, also over the bars beside the frame.
        SDL_FRect frame{};
        SDL_GetRenderLogicalPresentationRect(g_renderer, &frame);
        SDL_SetRenderLogicalPresentation(g_renderer, 0, 0, SDL_LOGICAL_PRESENTATION_DISABLED);
        int ow = 0, oh = 0;
        SDL_GetRenderOutputSize(g_renderer, &ow, &oh);
        touch::draw(g_renderer, ow, oh, frame);
        SDL_SetRenderLogicalPresentation(g_renderer, g_viewWidth, g_height,
                                         g_integerScale ? SDL_LOGICAL_PRESENTATION_INTEGER_SCALE
                                                        : SDL_LOGICAL_PRESENTATION_LETTERBOX);
    }
    SDL_RenderPresent(g_renderer);
}

bool presents_with_vsync()
{
    if (g_headless || g_renderer == nullptr) return false;
    int vsync = 0;
    return SDL_GetRenderVSync(g_renderer, &vsync) && vsync != 0;
}

void toggle_fullscreen()
{
    set_fullscreen(!g_fullscreen);
}

void set_fullscreen(bool on)
{
    if (g_window == nullptr || g_touchControls) return;   // touch screens: always full screen
    g_fullscreen = on;
    SDL_SetWindowFullscreen(g_window, g_fullscreen);
}

bool fullscreen() { return g_fullscreen; }

void set_integer_scale(bool on)
{
    g_integerScale = on;
    if (g_renderer == nullptr) return;
    SDL_SetRenderLogicalPresentation(g_renderer, g_viewWidth, g_height,
                                     g_integerScale ? SDL_LOGICAL_PRESENTATION_INTEGER_SCALE
                                                    : SDL_LOGICAL_PRESENTATION_LETTERBOX);
}

void set_auto_wide(bool on)
{
    g_autoWide = on;
    if (on) update_auto_width();
    else apply_view_width(g_width);
}

void set_view_width(int width)
{
    g_autoWide = false;
    apply_view_width(width);
}

int view_width() { return g_viewWidth; }

bool touch_screen()
{
    return g_touchControls;
}

void set_touch_mode(TouchMode mode)
{
    std::vector<touch::KeyChange> keys;
    touch::Mode m = mode == TouchMode::Play ? touch::Mode::Play
                  : mode == TouchMode::Editor ? touch::Mode::Editor : touch::Mode::Off;
    if (!g_touchControls || g_headless) m = touch::Mode::Off;
    touch::set_mode(m, keys);
    for (const touch::KeyChange& k : keys)
    {
        Event e{k.down ? EventType::KeyDown : EventType::KeyUp};
        e.key = k.vk;
        g_pending.push_back(e);
    }
}

void set_touch_editor_images(const std::vector<uint32_t> released[4], const std::vector<uint32_t> pressed[4],
                             int size)
{
    touch::set_editor_images(released, pressed, size);
}

int gamepad_count()
{
    return (int)g_pads.size();
}

bool gamepad_name(int index, char* name, int size)
{
    if (index < 0 || index >= (int)g_pads.size() || size <= 0) return false;
    const char* utf8 = SDL_GetGamepadName(g_pads[(size_t)index]);
    if (utf8 == nullptr) utf8 = "Gamepad";
    int n = 0;
    while (*utf8 && n < size - 1)
    {
        int c = to_cp1252(SDL_StepUTF8(&utf8, nullptr));
        if (c >= 0x20) name[n++] = (char)c;
    }
    name[n] = 0;
    return true;
}

bool gamepad_state(int index, int& x, int& y, unsigned& buttons)
{
    if (index < 0 || index >= (int)g_pads.size()) return false;
    SDL_Gamepad* pad = g_pads[(size_t)index];
    if (!SDL_GamepadConnected(pad)) return false;
    x = SDL_GetGamepadAxis(pad, SDL_GAMEPAD_AXIS_LEFTX);
    y = SDL_GetGamepadAxis(pad, SDL_GAMEPAD_AXIS_LEFTY);
    if (SDL_GetGamepadButton(pad, SDL_GAMEPAD_BUTTON_DPAD_LEFT)) x = -32768;
    if (SDL_GetGamepadButton(pad, SDL_GAMEPAD_BUTTON_DPAD_RIGHT)) x = 32767;
    if (SDL_GetGamepadButton(pad, SDL_GAMEPAD_BUTTON_DPAD_UP)) y = -32768;
    if (SDL_GetGamepadButton(pad, SDL_GAMEPAD_BUTTON_DPAD_DOWN)) y = 32767;
    buttons = 0;
    if (SDL_GetGamepadButton(pad, SDL_GAMEPAD_BUTTON_SOUTH)) buttons |= 1;
    if (SDL_GetGamepadButton(pad, SDL_GAMEPAD_BUTTON_EAST)) buttons |= 2;
    if (SDL_GetGamepadButton(pad, SDL_GAMEPAD_BUTTON_WEST)) buttons |= 4;
    if (SDL_GetGamepadButton(pad, SDL_GAMEPAD_BUTTON_NORTH)) buttons |= 8;
    return true;
}

void set_frame_x(int x)
{
    g_frameX = x;
}

void get_mouse(int& x, int& y)
{
    if (g_headless) { x = g_width / 2; y = g_height / 2; return; }
    float wx = 0, wy = 0, lx = 0, ly = 0;
    SDL_GetMouseState(&wx, &wy);
    SDL_RenderCoordinatesFromWindow(g_renderer, wx, wy, &lx, &ly);
    x = (int)SDL_floorf(lx) - g_frameX;
    y = (int)SDL_floorf(ly);
}

void warp_mouse(int x, int y)
{
    if (g_headless || !(SDL_GetWindowFlags(g_window) & SDL_WINDOW_INPUT_FOCUS)) return;
    float wx = 0, wy = 0;
    SDL_RenderCoordinatesToWindow(g_renderer, (float)(x + g_frameX) + 0.5f, (float)y + 0.5f, &wx, &wy);
    SDL_WarpMouseInWindow(g_window, wx, wy);
}

void show_system_cursor(bool show)
{
    if (g_headless) return;
    if (show) SDL_ShowCursor();
    else SDL_HideCursor();
}

void set_system_cursor(Cursor cursor)
{
    if (g_headless) return;
    SDL_SystemCursor id = SDL_SYSTEM_CURSOR_DEFAULT;
    switch (cursor)
    {
    case Cursor::Pointer:  id = SDL_SYSTEM_CURSOR_POINTER; break;
    case Cursor::Wait:     id = SDL_SYSTEM_CURSOR_WAIT; break;
    case Cursor::Move:     id = SDL_SYSTEM_CURSOR_MOVE; break;
    case Cursor::SizeNS:   id = SDL_SYSTEM_CURSOR_NS_RESIZE; break;
    case Cursor::SizeWE:   id = SDL_SYSTEM_CURSOR_EW_RESIZE; break;
    case Cursor::SizeNWSE: id = SDL_SYSTEM_CURSOR_NWSE_RESIZE; break;
    case Cursor::SizeNESW: id = SDL_SYSTEM_CURSOR_NESW_RESIZE; break;
    case Cursor::Hidden:   SDL_HideCursor(); return;
    default: break;
    }
    SDL_Cursor* c = SDL_CreateSystemCursor(id);
    if (c == nullptr) return;
    SDL_SetCursor(c);
    if (g_cursor) SDL_DestroyCursor(g_cursor);
    g_cursor = c;
}

void set_title(const char* title)
{
    if (g_window) SDL_SetWindowTitle(g_window, title);
}

void message_box(const char* title, const char* text)
{
    SDL_Log("%s: %s", title, text);
    if (!g_headless) SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, title, text, g_window);
}

std::string base_path()
{
    const char* p = SDL_GetBasePath();
    return p ? std::string(p) : std::string();
}

namespace
{
struct FolderChoice
{
    SDL_AtomicInt done;
    std::string path;
};

void SDLCALL folder_chosen(void* userdata, const char* const* list, int)
{
    FolderChoice* choice = static_cast<FolderChoice*>(userdata);
    if (list == nullptr) SDL_Log("folder dialog: %s", SDL_GetError());
    else if (list[0] != nullptr) choice->path = list[0];
    SDL_SetAtomicInt(&choice->done, 1);
}

SDL_EnumerationResult SDLCALL add_subdirectory(void* userdata, const char* dirname, const char* fname)
{
    std::string path = std::string(dirname) + fname;
    SDL_PathInfo info;
    if (SDL_GetPathInfo(path.c_str(), &info) && info.type == SDL_PATHTYPE_DIRECTORY)
    {
        static_cast<std::vector<std::string>*>(userdata)->push_back(path);
    }
    return SDL_ENUM_CONTINUE;
}
}  // namespace

std::string choose_folder(const char* title, const char* text)
{
#ifdef SDL_PLATFORM_ANDROID
    (void)title;
    (void)text;
    return std::string();
#else
    if (g_headless) return std::string();
    const SDL_MessageBoxButtonData buttons[] = {
        {SDL_MESSAGEBOX_BUTTON_RETURNKEY_DEFAULT, 1, "Choose folder..."},
        {SDL_MESSAGEBOX_BUTTON_ESCAPEKEY_DEFAULT, 0, "Quit"},
    };
    const SDL_MessageBoxData box = {SDL_MESSAGEBOX_INFORMATION, g_window, title, text,
                                    SDL_arraysize(buttons), buttons, nullptr};
    int button = 0;
    if (!SDL_ShowMessageBox(&box, &button) || button != 1) return std::string();
    // The dialog may answer from another thread: wait for it, keeping the
    // window responsive.
    FolderChoice choice;
    SDL_SetAtomicInt(&choice.done, 0);
    SDL_ShowOpenFolderDialog(folder_chosen, &choice, g_window, nullptr, false);
    while (SDL_GetAtomicInt(&choice.done) == 0)
    {
        SDL_PumpEvents();
        SDL_Delay(10);
    }
    return choice.path;
#endif
}

std::vector<std::string> subdirectories(const std::string& dir)
{
    std::vector<std::string> dirs;
    SDL_EnumerateDirectory(dir.c_str(), add_subdirectory, &dirs);
    return dirs;
}

std::vector<std::string> data_dirs()
{
    std::vector<std::string> dirs;
#ifdef SDL_PLATFORM_ANDROID
    // The player's own copy of the game, imported into app storage by the
    // launcher activity (port/android), or pushed by adb for development.
    if (const char* p = SDL_GetAndroidInternalStoragePath()) dirs.push_back(std::string(p) + "/game");
    if (const char* p = SDL_GetAndroidExternalStoragePath()) dirs.push_back(std::string(p) + "/game");
#endif
    return dirs;
}

void set_text_input(bool on)
{
    if (g_headless || g_window == nullptr) return;
    if (on == SDL_TextInputActive(g_window)) return;
    if (on) SDL_StartTextInput(g_window);
    else SDL_StopTextInput(g_window);
}

bool text_input_on_demand()
{
#ifdef SDL_PLATFORM_ANDROID
    return true;    // the on-screen keyboard covers the game
#else
    return false;   // always on: names, and the cheat codes typed while playing
#endif
}

std::string pref_path()
{
    char* p = SDL_GetPrefPath("Epsitec", "Speedy Blupi SDL3");
    std::string result = p ? std::string(p) : std::string();
    SDL_free(p);
    return result;
}

void log_debug(const char* text) { SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION, "%s", text); }
void log_info(const char* text) { SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "%s", text); }
void set_verbose(bool verbose)
{
    SDL_SetLogPriority(SDL_LOG_CATEGORY_APPLICATION, verbose ? SDL_LOG_PRIORITY_DEBUG : SDL_LOG_PRIORITY_INFO);
}
}  // namespace blupi::host
