// Program entry. SDL_main.h supplies the platform entry point (WinMain on
// Windows, the Android activity glue later) and calls main() below. Kept in its
// own TU because the game TUs see the port's Win32 compatibility names.
#include <SDL3/SDL_main.h>

int BlupiMain(int argc, char** argv);

int main(int argc, char** argv)
{
    return BlupiMain(argc, argv);
}
