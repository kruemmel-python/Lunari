package org.leng.research;

import android.app.Activity;
import android.os.Bundle;
import android.graphics.Color;
import android.view.Gravity;
import android.view.View;
import android.view.WindowManager;
import android.widget.Button;
import android.widget.EditText;
import android.widget.LinearLayout;
import android.widget.ScrollView;
import android.widget.TextView;
import android.widget.Toast;

import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;

public final class MainActivity extends Activity {
  static { System.loadLibrary("leng_core"); }

  private TextView status;
  private EditText source;
  private EditText lunari;
  private String lexiconPath;

  private native String translateNative(String text, String lexiconFile, boolean intoLunari);
  private native void closeNative();

  private int dp(int value) {
    return (int) (value * getResources().getDisplayMetrics().density + 0.5f);
  }

  private TextView label(String value) {
    TextView result = new TextView(this);
    result.setText(value);
    result.setTextSize(16);
    result.setTextColor(Color.rgb(25, 25, 30));
    result.setPadding(0, dp(12), 0, dp(4));
    return result;
  }

  private EditText editor(String hint) {
    EditText result = new EditText(this);
    result.setHint(hint);
    result.setTextSize(17);
    result.setGravity(Gravity.TOP | Gravity.START);
    result.setMinLines(5);
    result.setPadding(dp(10), dp(8), dp(10), dp(8));
    result.setBackgroundColor(Color.WHITE);
    return result;
  }

  private void translate(final boolean intoLunari) {
    final String input = intoLunari ? source.getText().toString() : lunari.getText().toString();
    if (input.trim().isEmpty()) {
      Toast.makeText(this, intoLunari ? "Bitte deutschen Text eingeben." : "Bitte Lunari eingeben.", Toast.LENGTH_SHORT).show();
      return;
    }
    status.setText("Neu-Lunari-Lexikon wird verwendet …");
    new Thread(() -> {
      final String output = translateNative(input, lexiconPath, intoLunari);
      runOnUiThread(() -> {
        if (output.startsWith("FEHLER: ")) {
          status.setText(output);
          return;
        }
        if (intoLunari) lunari.setText(output); else source.setText(output);
        status.setText(intoLunari
            ? "Deutsch wurde nach Neu-Lunari v1 übersetzt und neue Wörter wurden gespeichert."
            : "Neu-Lunari v1 wurde nach Deutsch zurückübersetzt.");
      });
    }).start();
  }

  private String installLexicon() throws IOException {
    File lexicon = new File(getFilesDir(), "neu-lunari-german-v1.tsv");
    if (lexicon.exists() && lexicon.length() > 0) return lexicon.getAbsolutePath();
    try (InputStream input = getAssets().open("neu-lunari-german-v1.tsv");
         FileOutputStream output = new FileOutputStream(lexicon)) {
      byte[] buffer = new byte[64 * 1024];
      int read;
      while ((read = input.read(buffer)) != -1) output.write(buffer, 0, read);
    }
    return lexicon.getAbsolutePath();
  }

  @Override public void onCreate(Bundle savedInstanceState) {
    super.onCreate(savedInstanceState);
    getWindow().setStatusBarColor(Color.rgb(31, 33, 38));
    getWindow().setSoftInputMode(WindowManager.LayoutParams.SOFT_INPUT_STATE_ALWAYS_HIDDEN);
    getWindow().getDecorView().setSystemUiVisibility(
        View.SYSTEM_UI_FLAG_LIGHT_STATUS_BAR | View.SYSTEM_UI_FLAG_LIGHT_NAVIGATION_BAR);
    ScrollView scroll = new ScrollView(this);
    scroll.setFocusableInTouchMode(true);
    LinearLayout content = new LinearLayout(this);
    content.setOrientation(LinearLayout.VERTICAL);
    content.setPadding(dp(16), dp(36), dp(16), dp(24));
    content.setBackgroundColor(Color.rgb(246, 246, 250));
    scroll.addView(content);

    TextView title = new TextView(this);
    title.setText("Neu-Lunari v1 — Deutsch ↔ Lunari");
    title.setTextSize(22);
    title.setTextColor(Color.rgb(30, 30, 36));
    content.addView(title);

    status = new TextView(this);
    status.setTextSize(14);
    status.setPadding(0, dp(8), 0, dp(8));
    content.addView(status);

    content.addView(label("Deutsch eingeben"));
    source = editor("Zum Beispiel: Ist heute Donnerstag?");
    content.addView(source, new LinearLayout.LayoutParams(LinearLayout.LayoutParams.MATCH_PARENT, LinearLayout.LayoutParams.WRAP_CONTENT));

    Button forward = new Button(this);
    forward.setText("Deutsch → Lunari");
    forward.setOnClickListener(view -> translate(true));
    content.addView(forward);

    content.addView(label("Lunari eingeben oder Ergebnis"));
    lunari = editor("Lunari-Text hier eingeben");
    content.addView(lunari, new LinearLayout.LayoutParams(LinearLayout.LayoutParams.MATCH_PARENT, LinearLayout.LayoutParams.WRAP_CONTENT));

    Button back = new Button(this);
    back.setText("Lunari → Deutsch");
    back.setOnClickListener(view -> translate(false));
    content.addView(back);

    try {
      lexiconPath = installLexicon();
      status.setText("Bereit. Neu-Lunari v1 mit persistentem Lexikon.");
    } catch (IOException error) {
      lexiconPath = "";
      status.setText("FEHLER: Neu-Lunari-Lexikon konnte nicht installiert werden: " + error.getMessage());
    }
    setContentView(scroll);
    scroll.requestFocus();
    scroll.post(() -> scroll.scrollTo(0, 0));
  }

  @Override protected void onDestroy() {
    closeNative();
    super.onDestroy();
  }
}
