# Lunari Developer Package (Windows)

Dieses Paket ist der bereinigte Entwicklungsstand der Linguistischen Emergenz NG.
Es enthält den Quellcode, die Build- und Regressionstests, die Benutzer- und
Übersetzungsdokumentation sowie den aktuellen lokalen deutschen Lunari-Datenstand.

## Enthaltener Datenstand

`german-wordlist-only.leng` ist der persistente aktuelle Datenstand. Er enthält
die aus der deutschen Wortliste gebildeten Lunari-Wortformen und die hierfür
erforderlichen Zuordnungen. Er enthält keine Wikidata-Importdaten.

Der Datenstand ist absichtlich von der Rohwortliste getrennt: Er genügt, um die
Oberfläche zu starten und zu übersetzen. Die große ursprüngliche Eingabeliste
und alle Wikidata-, Android-, Benchmark- und historischen Forschungsartefakte
sind nicht Bestandteil dieses Entwicklerpakets.

## Voraussetzungen

- Windows 10/11
- KR C++ Toolchain, standardmäßig unter
  `H:\KR-Compiler\KR-Compiler-Windows-x64-v4.3.1`

Liegt die Toolchain an einem anderen Ort, vor dem Build in PowerShell setzen:

```powershell
$env:KR_COMPILER_ROOT = 'D:\Pfad\zur\KR-Compiler-Windows-x64-v4.3.1'
```

## Bauen und starten

Im entpackten Paketordner ausführen:

```powershell
.\scripts\build_kr_release.ps1
.\release\linguistische-emergenz-ng-user.exe
```

Die Benutzeroberfläche lädt `german-wordlist-only.leng` automatisch aus dem
Paketordner. Die technische Forschungsoberfläche lässt sich bei Bedarf mit
`release\linguistische-emergenz-ng-ui.exe` starten.

Für die Übersetzung von Kommentaren in einem beliebigen Quellprojekt steht
zusätzlich `release\lunari-developer-comments.exe` bereit. Die Anwendung
analysiert zuerst ohne Änderungen und erstellt vor einer Übersetzung eine
vollständige Sicherung unter `.lunari-comment-backups`. Siehe
`DEVELOPER_COMMENT_GUI.md`.

## Übersetzer in der Kommandozeile

Neben der Benutzeroberfläche steht der texttreue Translator direkt in der
Konsole bereit:

```powershell
# Deutsch/Englisch → Lunari
.\release\linguistische-emergenz-ng.exe --translate-text .\german-wordlist-only.leng constructed "Bett"

# Lunari → Deutsch
.\release\linguistische-emergenz-ng.exe --translate-text .\german-wordlist-only.leng de "Sedabiku"
```

`--translate-text` erhält Satzzeichen, Leerraum und Dialogstruktur. Die
Rückübersetzung verändert den Datenstand nicht. Bei einer Vorwärtsübersetzung
können unbekannte deutsche Wortformen sicher in die angegebene `.leng`-Datei
aufgenommen werden. `--translate-meaning` gehört ausschließlich zum strengen
15-Felder-Bedeutungskanal, nicht zu freien Texten.

## Ausfallsicheres Speichern

Die Engine schreibt jeden `.leng`-Zustand zunächst in eine eindeutige
temporäre `.writing-…`-Datei. Erst nach erfolgreichem Abschluss wird der alte
Zustand atomar ersetzt. Ein Abbruch kann daher den zuletzt vollständig
gespeicherten Datenstand nicht teilweise überschreiben.

## Regression prüfen

Nach dem Build:

```powershell
.\scripts\test_lunari_text_roundtrip.ps1
```

Der Test prüft die verlustfreie strukturtreue Text-Rückübersetzung und die
Lunari-Phonotaktik (offene V-/CV-Silben).

Weitere Bedienhinweise stehen in `GUI_BENUTZERHANDBUCH.md` und die technischen
Grenzen des Übersetzers in `LUNARI_TEXT_TRANSLATION.md`.

## Lizenz

Der vollständige Inhalt dieses Entwicklerpakets – einschließlich Quellcode,
Dokumentation, Lehrbuch und Datenstand – steht unter der [MIT-Lizenz](LICENSE).
