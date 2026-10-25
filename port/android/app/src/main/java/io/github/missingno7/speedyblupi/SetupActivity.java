package io.github.missingno7.speedyblupi;

import android.app.Activity;
import android.content.ContentResolver;
import android.content.Intent;
import android.database.Cursor;
import android.graphics.Color;
import android.net.Uri;
import android.os.Bundle;
import android.provider.DocumentsContract;
import android.view.Gravity;
import android.widget.Button;
import android.widget.LinearLayout;
import android.widget.TextView;

import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;

// Launcher. The game data is not part of the app: each player supplies their own copy of
// Speedy Blupi or Speedy Eggbert (files/game, the SB1 program libmain.so) and of Speedy Blupi 2
// or Speedy Eggbert 2 (files/game2, libblupi2.so). On first run they choose the game's folder
// (the one with DATA, IMAGE08, IMAGE16 and SOUND, or a folder above it); the files are copied
// into the app's private storage once and the game starts. Later runs show the imported games
// and a button to import another. For development, data pushed with adb to the app's external
// files directory (files/game, files/game2) is used as well, and the extra "game" (sb1 or sb2)
// starts that game directly.
public class SetupActivity extends Activity {
    private static final int PICK_FOLDER = 1;

    private TextView status;
    private Button choose;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        final boolean sb1 = imported(GameFolder.Kind.SB1), sb2 = imported(GameFolder.Kind.SB2);
        String forced = getIntent() != null ? getIntent().getStringExtra("game") : null;
        if ("sb1".equals(forced) && sb1) { startGame(GameFolder.Kind.SB1); return; }
        if ("sb2".equals(forced) && sb2) { startGame(GameFolder.Kind.SB2); return; }

        LinearLayout layout = new LinearLayout(this);
        layout.setOrientation(LinearLayout.VERTICAL);
        layout.setGravity(Gravity.CENTER);
        layout.setPadding(48, 48, 48, 48);
        layout.setBackgroundColor(Color.BLACK);

        status = new TextView(this);
        status.setTextColor(Color.WHITE);
        status.setTextSize(18);
        status.setGravity(Gravity.CENTER);
        if (sb1 || sb2) {
            status.setText("Choose a game.");
        } else {
            status.setText("This app needs your own copy of the game: Speedy Blupi or Speedy Eggbert, "
                    + "or Speedy Blupi 2 or Speedy Eggbert 2.\n\n"
                    + "Choose the folder of an installed game (the one that contains DATA, IMAGE08, "
                    + "IMAGE16 and SOUND). Its files are copied into the app once.");
        }
        // An earlier import that the game would not start (incomplete, or another game): say so.
        // Saves and settings live outside the game folders and are kept.
        if ((!sb1 && new File(getFilesDir(), "game").exists()) || (!sb2 && new File(getFilesDir(), "game2").exists())) {
            status.setText(status.getText() + "\n\nAn earlier import is incomplete; choose its folder again. "
                    + "Your saves are kept.");
        }
        layout.addView(status);

        LinearLayout.LayoutParams params = new LinearLayout.LayoutParams(
                LinearLayout.LayoutParams.WRAP_CONTENT, LinearLayout.LayoutParams.WRAP_CONTENT);
        params.topMargin = 36;
        if (sb1) addButton(layout, params, "Speedy Blupi / Speedy Eggbert", () -> startGame(GameFolder.Kind.SB1));
        if (sb2) addButton(layout, params, "Speedy Blupi 2 / Speedy Eggbert 2", () -> startGame(GameFolder.Kind.SB2));
        choose = addButton(layout, params, sb1 || sb2 ? "Import another game folder" : "Choose game folder", () -> {
            Intent intent = new Intent(Intent.ACTION_OPEN_DOCUMENT_TREE);
            startActivityForResult(intent, PICK_FOLDER);
        });

        setContentView(layout);
    }

    private Button addButton(LinearLayout layout, LinearLayout.LayoutParams params, String text, Runnable action) {
        Button button = new Button(this);
        button.setText(text);
        button.setOnClickListener(v -> action.run());
        layout.addView(button, params);
        return button;
    }

    // The game's import folder: files/game (SB1) or files/game2 (SB2).
    static String folderName(GameFolder.Kind kind) {
        return kind == GameFolder.Kind.SB2 ? "game2" : "game";
    }

    private boolean imported(GameFolder.Kind kind) {
        if (GameFolder.recognize(new File(getFilesDir(), folderName(kind))) == kind) return true;
        File external = getExternalFilesDir(null);
        return external != null && GameFolder.recognize(new File(external, folderName(kind))) == kind;
    }

    @Override
    protected void onActivityResult(int requestCode, int resultCode, Intent data) {
        super.onActivityResult(requestCode, resultCode, data);
        if (requestCode != PICK_FOLDER || resultCode != RESULT_OK || data == null || data.getData() == null) return;
        final Uri tree = data.getData();
        choose.setEnabled(false);
        status.setText("Copying the game files...");
        new Thread(() -> importGame(tree)).start();
    }

    // Copies the chosen tree into files/game.tmp, finds the game folder in it (the chosen folder
    // or one level below), and makes that the game's import folder.
    private void importGame(Uri tree) {
        File temp = new File(getFilesDir(), "game.tmp");
        try {
            deleteTree(temp);
            String rootId = DocumentsContract.getTreeDocumentId(tree);
            int[] count = { 0 };
            copyTree(getContentResolver(), tree, rootId, temp, 0, count);
            GameFolder.Kind kind = GameFolder.Kind.SB1;
            File game = GameFolder.find(temp, kind);
            if (game == null) { kind = GameFolder.Kind.SB2; game = GameFolder.find(temp, kind); }
            if (game == null) {
                GameFolder.Kind why = GameFolder.explain(temp);
                deleteTree(temp);
                report(why == GameFolder.Kind.INCOMPLETE
                        ? "That folder's game files are incomplete (DATA/CONFIG.DEF, IMAGE08/INIT.BLP and "
                          + "DATA/DEMO200.BLP or DATA/BENCH.BLP are needed)."
                        : "That folder does not contain the game (no DATA and IMAGE08 folders).", true);
                return;
            }
            File target = new File(getFilesDir(), folderName(kind));
            deleteTree(target);
            if (!game.renameTo(target)) throw new IOException("cannot move the copied files into place");
            deleteTree(temp);
            final GameFolder.Kind started = kind;
            runOnUiThread(() -> startGame(started));
        } catch (Exception e) {
            deleteTree(temp);
            report("Copying failed: " + e.getMessage(), true);
        }
    }

    private void copyTree(ContentResolver resolver, Uri tree, String parentId, File dir, int depth, int[] count)
            throws IOException {
        if (depth > 6) return;
        if (!dir.isDirectory() && !dir.mkdirs()) throw new IOException("cannot create " + dir);
        Uri children = DocumentsContract.buildChildDocumentsUriUsingTree(tree, parentId);
        String[] columns = {
            DocumentsContract.Document.COLUMN_DOCUMENT_ID,
            DocumentsContract.Document.COLUMN_DISPLAY_NAME,
            DocumentsContract.Document.COLUMN_MIME_TYPE,
        };
        try (Cursor c = resolver.query(children, columns, null, null, null)) {
            if (c == null) return;
            while (c.moveToNext()) {
                String id = c.getString(0);
                String name = c.getString(1);
                String mime = c.getString(2);
                if (name == null || name.contains("/") || name.equals("..") || name.equals(".")) continue;
                File out = new File(dir, name);
                if (DocumentsContract.Document.MIME_TYPE_DIR.equals(mime)) {
                    copyTree(resolver, tree, id, out, depth + 1, count);
                } else {
                    Uri file = DocumentsContract.buildDocumentUriUsingTree(tree, id);
                    try (InputStream in = resolver.openInputStream(file);
                         OutputStream os = new FileOutputStream(out)) {
                        if (in == null) continue;
                        byte[] buffer = new byte[65536];
                        int n;
                        while ((n = in.read(buffer)) > 0) os.write(buffer, 0, n);
                    }
                    count[0]++;
                    if (count[0] % 50 == 0) report("Copying the game files... " + count[0], false);
                }
            }
        }
    }

    private static void deleteTree(File file) {
        File[] children = file.listFiles();
        if (children != null) for (File child : children) deleteTree(child);
        file.delete();
    }

    private void report(String text, boolean done) {
        runOnUiThread(() -> {
            status.setText(text);
            if (done) choose.setEnabled(true);
        });
    }

    private void startGame(GameFolder.Kind kind) {
        Intent game = new Intent(this, kind == GameFolder.Kind.SB2 ? Blupi2Activity.class : BlupiActivity.class);
        String args = getIntent() != null ? getIntent().getStringExtra("args") : null;
        if (args != null) game.putExtra("args", args);   // development: the game's options
        startActivity(game);
        finish();
    }
}
