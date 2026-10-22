package io.github.missingno7.speedyblupi;

import org.libsdl.app.SDLActivity;

// The game: SDL's activity running libmain.so (port/CMakeLists.txt). The game
// data is already in app storage (SetupActivity); the native side finds it
// through SDL_GetAndroidInternalStoragePath() (platform/host.cpp data_dirs).
//
// Development: the game's command-line options can be passed as one string,
//   adb shell am start -n io.github.missingno7.speedyblupi/.SetupActivity --es args "--verbose"
public class BlupiActivity extends SDLActivity {
    @Override
    protected String[] getLibraries() {
        return new String[] { "SDL3", "main" };
    }

    @Override
    protected String[] getArguments() {
        String args = getIntent() != null ? getIntent().getStringExtra("args") : null;
        if (args == null || args.trim().isEmpty()) return new String[0];
        return args.trim().split("\\s+");
    }
}
