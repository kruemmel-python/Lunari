# Lunari Entwickler-Kommentare

`lunari-developer-comments.exe` übersetzt Kommentare eines ausgewählten Quellprojekts nach Lunari.

1. Starte die Anwendung aus dem Release-Ordner. Sie lädt automatisch `german-wordlist-only.leng`.
2. Wähle den Projektordner.
3. Wähle **Analysieren**, um Anzahl und betroffene Dateien ohne Änderung zu prüfen.
4. Wähle **Übersetzen**, um die gefundenen Kommentare als `Lunari: ...` zu speichern.

Vor jeder Änderung liegt eine bytegenaue Kopie unter
`<Projekt>/.lunari-comment-backups/<Zeitstempel>/`.

Die Anwendung verarbeitet übliche Code- und Konfigurationsdateien. Sie erkennt `//`, `/* ... */` und – in passenden Skript-/Konfigurationsdateien – `#`-Kommentare. Kommentarzeichen in gewöhnlichen Zeichenketten werden nicht als Kommentare behandelt. Bereits mit `Lunari:` markierte Kommentare werden übersprungen, sodass ein zweiter Lauf sie nicht erneut übersetzt. Binärdateien, Dateien über 5 MB sowie `.git`, `node_modules`, Build-/Release-Ordner und frühere Sicherungen werden nicht angefasst.
