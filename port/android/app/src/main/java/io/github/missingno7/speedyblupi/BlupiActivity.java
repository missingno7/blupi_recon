package io.github.missingno7.speedyblupi;

import org.libsdl.app.SDLActivity;

// The game: SDL's activity running libmain.so (port/CMakeLists.txt). The game
// data is already in app storage (SetupActivity); the native side finds it
// through SDL_GetAndroidInternalStoragePath() (platform/host.cpp data_dirs).
public class BlupiActivity extends SDLActivity {
    @Override
    protected String[] getLibraries() {
        return new String[] { "SDL3", "main" };
    }
}
