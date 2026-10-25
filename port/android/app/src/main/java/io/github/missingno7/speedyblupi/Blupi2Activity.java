package io.github.missingno7.speedyblupi;

import java.io.File;

// Speedy Blupi 2 / Speedy Eggbert 2: SDL's activity running libblupi2.so (port/games/sb2), in its
// own process (AndroidManifest.xml) so that each game has its own SDL state. Its game data is the
// import in files/game2 (SetupActivity). SDL's pref path is the same app folder for both games on
// Android, so SB2's saves and settings get their own user folder, files/sb2.
public class Blupi2Activity extends BlupiActivity {
    @Override
    protected String[] getLibraries() {
        return new String[] { "SDL3", "blupi2" };
    }

    @Override
    protected String[] getArguments() {
        File data = new File(getFilesDir(), "game2");
        File external = getExternalFilesDir(null);
        if (GameFolder.recognize(data) != GameFolder.Kind.SB2 && external != null &&
            GameFolder.recognize(new File(external, "game2")) == GameFolder.Kind.SB2) {
            data = new File(external, "game2");   // development: data pushed with adb
        }
        File user = new File(getFilesDir(), "sb2");
        if (!user.isDirectory()) user.mkdirs();
        String[] development = super.getArguments();
        String[] args = new String[development.length + 4];
        args[0] = "--data";
        args[1] = data.getPath();
        args[2] = "--user";
        args[3] = user.getPath();
        System.arraycopy(development, 0, args, 4, development.length);
        return args;
    }
}
