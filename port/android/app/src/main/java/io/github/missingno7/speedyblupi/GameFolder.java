package io.github.missingno7.speedyblupi;

import java.io.File;

// Which game a folder holds, with the native launcher's recognition rule
// (common/app/launcher.cpp RecognizesData; games/sb1/game/main.cpp): DATA/CONFIG.DEF,
// IMAGE08/INIT.BLP and the edition marker, any case. Speedy Blupi 1 and Speedy Eggbert 1
// carry DATA/DEMO200.BLP; Speedy Blupi 2 and Speedy Eggbert 2 carry DATA/BENCH.BLP and are
// not part of the Android app yet. No Android dependency, so the rule is tested on the
// desktop (port/tests/android_game_folder.cmake).
final class GameFolder {
    enum Kind { SB1, SB2, INCOMPLETE, NONE }

    private GameFolder() {}

    static Kind recognize(File dir) {
        File data = child(dir, "DATA", true);
        File image = child(dir, "IMAGE08", true);
        if (data == null && image == null) return Kind.NONE;
        if (data == null || image == null) return Kind.INCOMPLETE;
        if (child(data, "CONFIG.DEF", false) == null || child(image, "INIT.BLP", false) == null) {
            return Kind.INCOMPLETE;
        }
        if (child(data, "DEMO200.BLP", false) != null) return Kind.SB1;
        if (child(data, "BENCH.BLP", false) != null) return Kind.SB2;
        return Kind.INCOMPLETE;
    }

    // The game folder: the chosen folder or one level below; an SB1 game first.
    static File find(File dir) {
        if (recognize(dir) == Kind.SB1) return dir;
        File[] children = dir.listFiles();
        if (children == null) return null;
        for (File c : children) {
            if (c.isDirectory() && recognize(c) == Kind.SB1) return c;
        }
        return null;
    }

    // The most specific reason a folder (or one below it) is not a usable SB1 game.
    static Kind explain(File dir) {
        Kind best = recognize(dir);
        File[] children = dir.listFiles();
        if (children != null) {
            for (File c : children) {
                if (!c.isDirectory()) continue;
                Kind k = recognize(c);
                if (k.ordinal() < best.ordinal()) best = k;
            }
        }
        return best;
    }

    private static File child(File dir, String name, boolean directory) {
        File[] children = dir.listFiles();
        if (children == null) return null;
        for (File c : children) {
            if (c.getName().equalsIgnoreCase(name) && c.isDirectory() == directory) return c;
        }
        return null;
    }
}
