// On-screen controls for touch screens (port/docs/ROADMAP.md stage 6).
//
// Play: a joystick (left) and jump, fire and pause buttons (right) are drawn
// over the presented frame, in the bars beside the 4:3 frame where there are
// any. The joystick holds the arrow keys its knob points to (up to two, for
// diagonals); a button holds its key.
//
// Editor: a pad of four game-style arrow buttons (bottom right) holds the
// arrow keys, which scroll the editor's view (CDecor::MoveStep).
//
// They are the keyboard's keys (plain VK numbers): the game's own key handling
// sees no difference. Fingers elsewhere, and all fingers outside these modes,
// act as the mouse (host.cpp). Presentation and input only: nothing here
// reads game state.
#ifndef BLUPI_PLATFORM_TOUCH_H
#define BLUPI_PLATFORM_TOUCH_H

#include <SDL3/SDL.h>

#include <cstdint>
#include <vector>

namespace blupi::host::touch
{
struct KeyChange
{
    int  vk;     // VK numbering (arrows 0x25-0x28, jump = Ctrl 0x11, fire = Space 0x20, pause = Escape 0x1B)
    bool down;
};

enum class Mode { Off, Play, Editor };

// Shown and used only in a mode; changing it releases held keys.
void set_mode(Mode mode, std::vector<KeyChange>& keys);
Mode mode();

// Images of the editor pad's buttons (left, right, up, down), released and
// pressed: size x size, 0xAARRGGBB. Without them the pad is drawn plainly.
void set_editor_images(const std::vector<uint32_t> released[4], const std::vector<uint32_t> pressed[4], int size);

// A finger event in render output pixels (w x h: the output size). Returns
// true when a control owns the finger; the keys it pressed or released are
// appended. type: 0 down, 1 motion, 2 up.
bool finger(int type, SDL_FingerID id, float x, float y, int w, int h, std::vector<KeyChange>& keys);

// Draws the controls over the frame, in render output pixels.
void draw(SDL_Renderer* renderer, int w, int h, const SDL_FRect& frame);
}  // namespace blupi::host::touch

#endif
