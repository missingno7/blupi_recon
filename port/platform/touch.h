// On-screen controls for touch screens (port/docs/ROADMAP.md stage 6).
//
// While the game is being played, a joystick (left) and jump, fire and pause
// buttons (right) are drawn over the presented frame, in the bars beside the
// 4:3 frame where there are any. The joystick holds the arrow keys its knob
// points to (up to two, for diagonals); a button holds its key. They are the
// keyboard's keys (plain VK numbers): the game's own key handling sees no
// difference. Fingers elsewhere, and all fingers outside play, act as the
// mouse (host.cpp). Presentation and input only: nothing here reads game state.
#ifndef BLUPI_PLATFORM_TOUCH_H
#define BLUPI_PLATFORM_TOUCH_H

#include <SDL3/SDL.h>

#include <vector>

namespace blupi::host::touch
{
struct KeyChange
{
    int  vk;     // VK numbering (arrows 0x25-0x28, jump = Ctrl 0x11, fire = Space 0x20, pause = Escape 0x1B)
    bool down;
};

// Shown and used only while active; turning it off releases held keys.
void set_active(bool on, std::vector<KeyChange>& keys);
bool active();

// A finger event in render output pixels (w x h: the output size). Returns
// true when a control owns the finger; the keys it pressed or released are
// appended. type: 0 down, 1 motion, 2 up.
bool finger(int type, SDL_FingerID id, float x, float y, int w, int h, std::vector<KeyChange>& keys);

// Draws the controls over the frame, in render output pixels.
void draw(SDL_Renderer* renderer, int w, int h, const SDL_FRect& frame);
}  // namespace blupi::host::touch

#endif
