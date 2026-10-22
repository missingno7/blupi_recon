// The port's icon (res/icon.png, made from the game's waving Blupi sprite by
// tools/make_icon.py, embedded at build time) as an SDL surface, for the
// window where the executable carries no icon.
#ifndef BLUPI_PLATFORM_ICON_H
#define BLUPI_PLATFORM_ICON_H

#include <SDL3/SDL.h>

namespace blupi::host
{
// The icon, or nullptr; the caller destroys it.
SDL_Surface* game_icon();
}  // namespace blupi::host

#endif
