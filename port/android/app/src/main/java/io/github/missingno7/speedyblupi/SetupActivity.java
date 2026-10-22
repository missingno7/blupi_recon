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

// Launcher. The game data is not part of the app: each player supplies their
// own copy of Speedy Blupi. On first run they choose its folder (the one with
// DATA, IMAGE08, IMAGE16 and SOUND, or a folder above it); the files are
// copied into the app's private storage (files/game), and the game starts.
// Later runs start the game directly. For development, data pushed with adb to
// the app's external files directory (files/game) is used as well.
public class SetupActivity extends Activity {
    private static final int PICK_FOLDER = 1;

    private TextView status;
    private Button choose;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        if (hasGame(new File(getFilesDir(), "game")) ||
            (getExternalFilesDir(null) != null && hasGame(new File(getExternalFilesDir(null), "game")))) {
            startGame();
            return;
        }

        LinearLayout layout = new LinearLayout(this);
        layout.setOrientation(LinearLayout.VERTICAL);
        layout.setGravity(Gravity.CENTER);
        layout.setPadding(48, 48, 48, 48);
        layout.setBackgroundColor(Color.BLACK);

        status = new TextView(this);
        status.setTextColor(Color.WHITE);
        status.setTextSize(18);
        status.setGravity(Gravity.CENTER);
        status.setText("Speedy Blupi needs your own copy of the game.\n\n"
                + "Choose the folder of an installed Speedy Blupi (the one that contains "
                + "DATA, IMAGE08, IMAGE16 and SOUND). Its files are copied into the app once.");
        layout.addView(status);

        choose = new Button(this);
        choose.setText("Choose game folder");
        choose.setOnClickListener(v -> {
            Intent intent = new Intent(Intent.ACTION_OPEN_DOCUMENT_TREE);
            startActivityForResult(intent, PICK_FOLDER);
        });
        LinearLayout.LayoutParams params = new LinearLayout.LayoutParams(
                LinearLayout.LayoutParams.WRAP_CONTENT, LinearLayout.LayoutParams.WRAP_CONTENT);
        params.topMargin = 48;
        layout.addView(choose, params);

        setContentView(layout);
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

    // Copies the chosen tree into files/game.tmp, finds the game folder in it
    // (the chosen folder or one level below), and makes that files/game.
    private void importGame(Uri tree) {
        File target = new File(getFilesDir(), "game");
        File temp = new File(getFilesDir(), "game.tmp");
        try {
            deleteTree(temp);
            String rootId = DocumentsContract.getTreeDocumentId(tree);
            int[] count = { 0 };
            copyTree(getContentResolver(), tree, rootId, temp, 0, count);
            File game = findGame(temp);
            if (game == null) {
                deleteTree(temp);
                report("That folder does not contain Speedy Blupi (no DATA and IMAGE08 folders).", true);
                return;
            }
            deleteTree(target);
            if (!game.renameTo(target)) throw new IOException("cannot move the copied files into place");
            deleteTree(temp);
            runOnUiThread(this::startGame);
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

    // The game folder has DATA and IMAGE08 (any case); the chosen folder or one below.
    private static File findGame(File dir) {
        if (hasGame(dir)) return dir;
        File[] children = dir.listFiles();
        if (children == null) return null;
        for (File child : children) {
            if (child.isDirectory() && hasGame(child)) return child;
        }
        return null;
    }

    private static boolean hasGame(File dir) {
        File[] children = dir.listFiles();
        if (children == null) return false;
        boolean data = false, image = false;
        for (File child : children) {
            if (!child.isDirectory()) continue;
            if (child.getName().equalsIgnoreCase("DATA")) data = true;
            if (child.getName().equalsIgnoreCase("IMAGE08")) image = true;
        }
        return data && image;
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

    private void startGame() {
        Intent game = new Intent(this, BlupiActivity.class);
        String args = getIntent() != null ? getIntent().getStringExtra("args") : null;
        if (args != null) game.putExtra("args", args);   // development: the game's options
        startActivity(game);
        finish();
    }
}
