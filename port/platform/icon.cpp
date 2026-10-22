#include "platform/icon.h"

namespace blupi::host
{
namespace
{
const unsigned char k_icon[] = {
#include "icon_png.inc"
};
}  // namespace

SDL_Surface* game_icon()
{
    return SDL_LoadPNG_IO(SDL_IOFromConstMem(k_icon, sizeof(k_icon)), true);
}
}  // namespace blupi::host
