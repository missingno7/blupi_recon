package io.github.missingno7.speedyblupi;

import java.io.File;
import java.io.IOException;

// Desktop check of the Android importer's recognition rule (GameFolder.java): the same files as
// the native launcher's RecognizesData, any case; SB1 and SB2 folders are told apart and the
// former DATA + IMAGE08-only false positive is refused. Synthetic trees only.
public final class GameFolderCheck {
    private static int failures;

    public static void main(String[] args) throws IOException {
        File work = new File(args[0]);
        deleteTree(work);
        File sb1 = tree(work, "sb1", "DATA/Config.def", "DATA/DEMO200.BLP", "IMAGE08/INIT.BLP");
        File lower = tree(work, "lower", "data/config.def", "data/demo200.blp", "image08/init.blp");
        File sb2 = tree(work, "sb2", "DATA/config.def", "DATA/bench.blp", "IMAGE08/init.blp");
        File folders = tree(work, "folders", "DATA/", "IMAGE08/");
        File noMarker = tree(work, "nomarker", "DATA/CONFIG.DEF", "IMAGE08/INIT.BLP");
        File cd = tree(work, "cd", "ENGLISH/DATA/CONFIG.DEF", "ENGLISH/DATA/DEMO200.BLP",
                "ENGLISH/IMAGE08/INIT.BLP", "SETUP/");
        File sb2cd = tree(work, "sb2cd", "Game/DATA/config.def", "Game/DATA/bench.blp", "Game/IMAGE08/init.blp");
        File empty = tree(work, "empty", "readme.txt");

        expect("SB1", GameFolder.recognize(sb1) == GameFolder.Kind.SB1);
        expect("lower-case SB1", GameFolder.recognize(lower) == GameFolder.Kind.SB1);
        expect("SB2", GameFolder.recognize(sb2) == GameFolder.Kind.SB2 && sb2.equals(GameFolder.find(sb2, GameFolder.Kind.SB2))
                && GameFolder.find(sb2, GameFolder.Kind.SB1) == null);
        expect("folders only", GameFolder.recognize(folders) == GameFolder.Kind.INCOMPLETE);
        expect("no edition marker", GameFolder.recognize(noMarker) == GameFolder.Kind.INCOMPLETE);
        expect("game one level below", new File(cd, "ENGLISH").equals(GameFolder.find(cd, GameFolder.Kind.SB1)));
        expect("SB2 one level below", new File(sb2cd, "Game").equals(GameFolder.find(sb2cd, GameFolder.Kind.SB2)));
        expect("nothing", GameFolder.recognize(empty) == GameFolder.Kind.NONE && GameFolder.find(empty, GameFolder.Kind.SB1) == null);
        deleteTree(work);
        if (failures != 0) {
            System.out.println(failures + " failure(s)");
            System.exit(1);
        }
        System.out.println("GameFolder: all cases pass");
    }

    private static void expect(String name, boolean ok) {
        System.out.println((ok ? "pass " : "FAIL ") + name);
        if (!ok) failures++;
    }

    private static File tree(File work, String name, String... paths) throws IOException {
        File root = new File(work, name);
        for (String path : paths) {
            File f = new File(root, path);
            if (path.endsWith("/")) {
                if (!f.isDirectory() && !f.mkdirs()) throw new IOException("mkdir " + f);
            } else {
                if (!f.getParentFile().isDirectory() && !f.getParentFile().mkdirs()) throw new IOException("mkdir " + f);
                if (!f.isFile() && !f.createNewFile()) throw new IOException("create " + f);
            }
        }
        return root;
    }

    private static void deleteTree(File file) {
        File[] children = file.listFiles();
        if (children != null) for (File c : children) deleteTree(c);
        file.delete();
    }
}
