# MDraft KR Enterprise - ZIP Code Dump

========== FILE: Lunari-Developer/BUILD.md ==========

# Build

Der verbindlich geprüfte Windows-x64-Release wird mit `H:\KR-Compiler\KR-Compiler-Windows-x64-v4.3.1\toolchain\bin\g++.exe` gebaut:

```powershell
powershell -ExecutionPolicy Bypass -File .\scripts\build_kr_release.ps1
```

Der Vorgang erzeugt `release\linguistische-emergenz-ng.exe`, `release\linguistische-emergenz-ng-ui.exe` und `release\leng_api.dll`. Beide EXE-Dateien binden die GCC-, C++- und pthread-Laufzeit statisch ein. Ein isolierter GUI-Starttest mit einem auf Systemverzeichnisse beschränkten `PATH` war erfolgreich; der Importsatz enthält nur Windows-System-DLLs (`GDI32`, `KERNEL32`, `msvcrt`, `USER32`). `leng_api.dll` ist die dokumentierte native C-Schnittstelle für lokale Forschungssteuerungen.

Die Hardwareprüfung kann mit `release\linguistische-emergenz-ng.exe --compute-test` ausgeführt werden. Sie meldet explizit einen erfolgreichen GPU/CPU-Vergleich, einen CPU-Fallback-Skip oder einen Fehler.

Ein CMake-Entwicklerbuild mit MSYS2-MinGW bleibt für schnelle Tests verfügbar. Die konkreten Befehle stehen im README, seine Artefakte sind aber nicht als auslieferbarer Release vorgesehen.

Linux x64 wird mit der mitgelieferten Zig-C++-Toolchain gebaut:

```powershell
powershell -ExecutionPolicy Bypass -File .\scripts\build_kr_linux_release.ps1
```

Das erzeugt `release-linux\linguistische-emergenz-ng` als x86-64-ELF. Der Selbsttest wurde in der vorhandenen Ubuntu-WSL-Umgebung erfolgreich ausgeführt. Die Win32-Oberfläche ist naturgemäß kein Linux-Ziel; der portable Forschungskern, Persistenz, Export und CLI sind dort testbar.

Für Android ARM64 existiert ein nativer Core-Build mit dem vorhandenen Android NDK:

```powershell
powershell -ExecutionPolicy Bypass -File .\scripts\build_android_core.ps1
```

Er erzeugt `release-android\arm64-v8a\libleng_core.so` und den nativen Prüfling `leng-device-smoke` für `aarch64-linux-android24`. Der Prüfling wurde über ADB auf einem verbundenen ARM64-Gerät ausgeführt und prüfte Evolution, Agentenkommunikation sowie einen Speicher-/Lade-Roundtrip. Für eine Wiederholung: `adb push release-android/arm64-v8a/leng-device-smoke /data/local/tmp/` und anschließend `adb shell 'chmod 700 /data/local/tmp/leng-device-smoke && /data/local/tmp/leng-device-smoke'`.

Der KR Compiler ist als mögliche Build-Orchestrierung verfügbar; der aktuelle CMake-Referenzbuild verwendet noch keine `.kr`-Datei.

========== END FILE ==========

========== FILE: Lunari-Developer/DEVELOPER_PACKAGE_README.md ==========

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

## Regression prüfen

Nach dem Build:

```powershell
.\scripts\test_lunari_text_roundtrip.ps1
```

Der Test prüft die verlustfreie strukturtreue Text-Rückübersetzung und die
Lunari-Phonotaktik (offene V-/CV-Silben).

Weitere Bedienhinweise stehen in `GUI_BENUTZERHANDBUCH.md` und die technischen
Grenzen des Übersetzers in `LUNARI_TEXT_TRANSLATION.md`.

========== END FILE ==========

========== FILE: Lunari-Developer/GUI_BENUTZERHANDBUCH.md ==========

# Linguistische Emergenz NG – GUI-Benutzerhandbuch

Dieses Handbuch erklärt die native Windows-Oberfläche für neue Nutzer. Die
Anwendung dient dazu, einen persistenten Forschungsstand zu untersuchen, die
entstehende Sprache zu erkunden und zwischen Kunstsprache, Deutsch und Englisch
zu übersetzen.

## Einfache Nutzeroberfläche (empfohlen)

Für die tägliche Nutzung starte statt der Forschungsoberfläche:

```powershell
.\release\linguistische-emergenz-ng-user.exe
```

Diese Oberfläche lädt automatisch `wikidata-100k.leng` (oder den größten
verfügbaren geprüften Fallback-Stand). Links wird Deutsch oder Englisch
eingegeben, rechts erscheint die Kunstsprache. **Übersetzen →** erzeugt die
Kunstsprache, **← Zurückübersetzen** übersetzt sie zurück.
**Wikidata lernen** verarbeitet einen kontrollierten, checkpointfähigen nächsten
Batch aus der lokalen Wikidata-Quelle, validiert daraus gebildete Formen und
speichert den Forschungsstand. **Deutsche Wörter lernen** verarbeitet die
gesamte Datei `words.full.list.build.txt` im Hintergrund in Batches von jeweils
50.000 Wortformen und speichert nach jedem Batch einen Checkpoint.

Die deutsche Wortliste ist keine Bedeutungsdatenbank. Ihre Einträge werden
daher mit `user-provided-german-word-list`, unbekannter Lizenz und
`meaning=unverified` gespeichert. Sie zählen weder als Wikidata-Konzepte noch
als kommunikativ validierte Bedeutungen; sie lassen sich aber als deutsche
Eingaben in Kunstwortformen überführen. Kein Lernknopf führt einen
unkontrollierten Internet-Massendownload aus.

Während dieses Laufs bleibt der Fortschritt sichtbar. **Lernen pausieren**
hält nach dem aktuellen Batch an; beim nächsten Klick auf **Deutsche Wörter
lernen** wird exakt am gespeicherten Checkpoint fortgesetzt.

Das Fenster ist responsiv: Beide Übersetzungsfelder und die Wörterbuchansicht
passen sich beim Vergrößern oder Verkleinern automatisch an. Für Importdetails,
Vergleiche und Forschungstelemetrie bleibt die nachfolgend beschriebene
Forschungsoberfläche verfügbar.

## 1. Programm starten

Starte:

```powershell
.\release\linguistische-emergenz-ng-ui.exe
```

Es öffnet sich **LINGUISTISCHE EMERGENZ NG — Research Interface**. Oben
befinden sich Funktionsschaltflächen, darunter ein einzelnes Eingabefeld und
ein großes Ausgabefeld. Das Eingabefeld wird je nach Schaltfläche für eine
Antwort, einen Suchbegriff, einen Dateipfad oder einen zu übersetzenden Text
verwendet.

Die Oberfläche startet auf Deutsch. Über die Schaltfläche **English** unter
der ersten Buttonreihe wechselst du jederzeit zur englischen Bedienoberfläche;
die Schaltfläche heißt danach **Deutsch** und wechselt zurück. Wissenschaftliche
Rohdaten wie Konzept-IDs, Wikidata-QIDs und die künstlichen Wortformen werden
dabei bewusst nicht verändert.

## 2. Einen vorhandenen Forschungsstand laden

Ein Forschungsstand ist eine Datei mit der Endung `.leng`. Sie enthält Sprache,
Konzeptgraph, Agentenwissen und Statistiken.

1. Lasse das Eingabefeld leer und klicke **Zustand laden**. Es öffnet sich ein
   Dateiauswahldialog, der nur `.leng`-Forschungsstände anbietet.
2. Alternativ: Klicke in das Eingabefeld und gib den vollständigen Pfad ein,
   zum Beispiel:

   ```text
   H:\Linguistische-Emergenz-System\Linguistische-Emergenz-NG\wikidata-100k.leng
   ```

3. Klicke **Zustand laden** (in English UI: **Load state**).
4. Im Ausgabefeld erscheint ein Bericht über den tatsächlich geladenen Stand.

Für einen kleinen, schnellen Beispielstand kann stattdessen diese Datei geladen
werden:

```text
H:\Linguistische-Emergenz-System\Linguistische-Emergenz-NG\research-demo.leng
```

> Wichtig: Erst nach dem Laden oder nach **Run generation** sind Sprache und
> Translator verfügbar. Die angezeigten Wörter gehören immer zum aktuell
> geladenen Forschungsstand.

## 3. Die Oberfläche im Überblick

| Schaltfläche | Zweck |
|---|---|
| **Run generation** | Führt eine weitere Forschungs-/Sprachgeneration im aktuellen Zustand aus. |
| **Save state** | Speichert den aktuellen Zustand in der gewählten `.leng`-Datei. |
| **Load state** | Lädt den im Eingabefeld angegebenen Forschungsstand. |
| **Language lab** | Zeigt Grammatikreihenfolge und die elementaren Zuordnungen der aktuellen Sprache. |
| **Human exercise** | Startet eine kurze, geführte Lernübung. |
| **Check answer** | Prüft die Antwort aus dem Eingabefeld auf die aktuelle Übung. |
| **Communication** | Zeigt einen Sender-/Empfänger-Test der Agentenkommunikation. |
| **Evolution** | Zeigt die jüngsten Einträge des Evolutionsarchivs. |
| **Population** | Zeigt die persistenten Agenten und ihre Kommunikationsstatistik. |
| **Export** | Schreibt Berichte und das Wörterbuch in den Ordner `leng-export`. |
| **Lexicon lab** | Zeigt Konzepte mit Form, Bedeutung, Quelle und Validierungsstatus. |
| **Lexicon status** | Zeigt kompakte Zähler für importierte, realisierte und validierte Konzepte. |
| **Search input** | Sucht nach dem Text im Eingabefeld im persistenten Lexikon. |
| **Wörterbuch** / **Dictionary** | Zeigt eine durchsuchbare Seite mit Kunstwort, Englisch, Deutsch, Wortart, Evidenz und Status. |
| **← Seite** / **→ Seite** | Blättert durch Treffer im Wörterbuch. |
| **Concept phrase** | Demonstriert die Interpretation einer neu kombinierten Wortfamilie. |
| **Compare typed state** | Vergleicht den aktuellen Stand mit dem `.leng`-Pfad im Eingabefeld. |
| **Import TSV...** | Importiert eine kontrollierte Konzept-TSV-Datei. Für normale Nutzung nicht erforderlich. |
| **Benchmark** | Misst Such- und Speicherkennzahlen des geladenen Lexikons. |
| **Translate DE** | Übersetzt erkannte Kunstsprache oder englische Begriffe ins Deutsche. |
| **Translate EN** | Übersetzt erkannte Kunstsprache oder deutsche Begriffe ins Englische. |
| **To language** | Erzeugt zu erkannten deutschen oder englischen Begriffen die Formen der Kunstsprache. |
| **English** / **Deutsch** | Wechselt die Sprache der Bedienoberfläche. |

## 4. Die Sprache ansehen und verstehen

1. Lade einen Forschungsstand.
2. Klicke **Language lab**.
3. Lies zuerst die Zeile `Grammar order`. Sie bestimmt die Reihenfolge der
   Bedeutungsteile in einer vollständigen Nachricht.
4. Darunter steht jeweils `Kategorie:Bedeutung = Kunstwort`.

Beispiel: Steht dort `actor:child = memmil`, bedeutet `memmil` im geladenen
Sprachstand *child/Kind*. Die konkrete Form kann in anderen Forschungsständen
anders sein; deshalb immer zunächst den geladenen Stand ansehen.

## 5. Translator verwenden

Der Translator arbeitet nicht als allgemeine Internetübersetzung. Er nutzt die
im Forschungsstand gespeicherten Konzeptidentitäten, Wortformen und
Grammatikbegriffe. Dadurch ist bei bekannten Begriffen nachvollziehbar, welche
Bedeutung erkannt wurde.

### Kunstsprache in Deutsch übersetzen

1. Lade den gewünschten Forschungsstand.
2. Gib ein oder mehrere Kunstwörter, durch Leerzeichen getrennt, in das
   Eingabefeld ein. Beispielsweise die Wortfolge aus **Language lab**.
3. Klicke **Translate DE**.
4. Prüfe im Ausgabefeld:
   - `Recognized concepts` zeigt die erkannte semantische Analyse;
   - `Deutsch` zeigt die Ausgabe.

### Kunstsprache in Englisch übersetzen

Gib die Kunstwörter ein und klicke **Translate EN**. Die Ausgabe `English`
zeigt die englischen Konzeptbezeichnungen.

### Deutsch oder Englisch in die Kunstsprache übersetzen

1. Gib bekannte Begriffe als durch Leerzeichen getrennte Folge ein, zum
   Beispiel:

   ```text
   bitte kind bauen karte
   ```

   oder:

   ```text
   request child build map
   ```

2. Klicke **To language**.
3. Die Zeile `Constructed language` enthält die Formen des aktuell geladenen
   Sprachstands.

Die Übersetzung bewahrt die Reihenfolge der eingegebenen Begriffe. Für eine
vollständige Nachricht verwende die im **Language lab** angezeigte
Grammatikreihenfolge.

### Unbekannte und importierte Begriffe

Nicht jedes Wikidata-Konzept besitzt bereits eine geprüfte deutsche
Bezeichnung. In diesem Fall lässt der Translator das Quellenlabel stehen und
weist mit einer Notiz darauf hin. Das ist beabsichtigt: Das System erfindet
keine Übersetzung und verwechselt keine externe Herkunft mit einer behaupteten
deutschen Bedeutung.

Wenn `Unresolved tokens` erscheint, war der eingegebene Ausdruck im geladenen
Sprachstand nicht als Kunstwort, englisches Label oder geprüfte deutsche
Glossierung bekannt. Prüfe Schreibweise und geladenen Forschungsstand.

## 6. Lernübungen richtig verwenden

Die Übungen sind kurz und eindeutig aufgebaut:

1. Klicke **Human exercise**.
2. Lies insbesondere die Zeile `INPUT:`.
3. Lösche gegebenenfalls alten Text im Eingabefeld.
4. Gib genau das geforderte Format ein.
5. Klicke **Check answer**.

Bei einer *recognition*-Aufgabe wird ein deutsches/englisches Bedeutungswort
erwartet. Bei einer *production*-Aufgabe werden Kunstwörter in der verlangten
Reihenfolge erwartet. Der Translator kann vor dem Prüfen als Lernhilfe
verwendet werden.

## 7. Konzepte und Quellen nachschlagen

### Wörterbuch verwenden

1. Gib optional ein englisches oder deutsches Label, eine Kunstform, eine QID
   oder einen Semantikbegriff ein. Lasse das Feld leer, um den gesamten Bestand
   seitenweise zu erkunden.
2. Klicke **Wörterbuch** (bzw. **Dictionary**).
3. Jede Karte zeigt Kunstwort, englisches Label, deutsche Übersetzung oder
   Quellenlabel, Wortart, Evidenz, Quelle und Sprachstatus.
4. Klicke **← Seite** oder **Seite →**, um je 20 Konzepte weiterzublättern.

Das Wörterbuch ist für das Erkunden gedacht; **Eingabe suchen** bleibt die
kompaktere technische Suche für präzise Treffer.

1. Gib ein Wort, eine Konzeptbezeichnung oder eine externe Kennung (z. B.
   `Q...`) in das Eingabefeld ein.
2. Klicke **Search input**.

Ein Treffer zeigt unter anderem Form, Wortart, Semantik, Beziehungen,
Quellenangabe, Lizenz-/Provenienzdaten und Sprachstatus. Die Kategorien
`external-attested`, `simulated-relation` und `agent-hypothesis` bleiben dabei
absichtlich getrennt.

## 8. Sicher speichern und fortsetzen

Nach einer eigenen Generation oder einem Import:

1. Klicke **Save state**.
2. Warte auf die Bestätigung im Ausgabefeld.
3. Beim nächsten Start gib denselben Pfad ein und klicke **Load state**.

`Save state` überschreibt den aktuell ausgewählten Zustand. Wenn ein separater
Experimentstand gewünscht ist, lade oder erstelle zuerst einen eigenen
Dateinamen über die Kommandozeile; vorhandene Forschungsstände sollten nicht
unbeabsichtigt überschrieben werden.

## 9. Häufige Probleme

| Meldung oder Beobachtung | Lösung |
|---|---|
| `Operation failed` beim Laden | Prüfe den vollständigen `.leng`-Pfad und ob die Datei existiert. |
| Keine Übersetzung erkannt | Erst einen Zustand laden; anschließend nur Wörter aus dessen Language/Lexicon Lab verwenden. |
| Deutsche Ausgabe enthält ein englisches Wort | Für dieses importierte Konzept gibt es noch keine geprüfte deutsche Glossierung. Das Quellenlabel wurde bewusst erhalten. |
| Übung wird abgelehnt | Die Zeile `INPUT:` bestimmt, ob Bedeutungstext oder Kunstwörter erwartet werden. Keine Feldnamen oder Semikolons eingeben. |
| Die Wortformen unterscheiden sich von einem früheren Screenshot | Es wurde ein anderer Forschungsstand geladen oder eine neue Generation erzeugt. |

## 10. Empfohlener erster Rundgang

1. Starte die GUI.
2. Lade `research-demo.leng`.
3. Klicke **Language lab** und notiere vier Formen.
4. Gib diese Formen ein und klicke **Translate DE**.
5. Gib anschließend `request child build map` ein und klicke **To language**.
6. Klicke **Lexicon status** und dann **Search input** mit `forest`.
7. Starte eine **Human exercise** und prüfe die Antwort.

Damit sind Sprache, Übersetzung, Konzeptgraph und menschliche Lernfunktion in
einem sicheren, wiederholbaren Ablauf kennengelernt.

========== END FILE ==========

========== FILE: Lunari-Developer/LUNARI_TEXT_TRANSLATION.md ==========

# Lunari-Textübersetzung

Die Schaltflächen der Benutzeroberfläche verwenden den strukturtreuen Lunari-Textkanal.

- Jeder bekannte deutsche Eintrag erscheint direkt als seine persistente
  Lunari-Form; es gibt keine technische Hülle vor jedem Wort.
- Groß-/Kleinschreibung, Leerraum, Zeilenumbrüche, Satzzeichen, Anführungszeichen und Sprecherwechsel bleiben erhalten.
- Nicht im Lexikon vorhandene UTF-8-Tokens werden beim Vorwärtsübersetzen als
  lokale Wortform aufgenommen und erhalten sofort eine neue offene Lunari-CV-
  Form. Herkunft und Status bleiben dabei als `translator-text-intake` und
  `meaning=unverified` dokumentiert; die Engine erfindet keine Bedeutung.
- Alle Lunari-Texttokens erfüllen die aktuelle offene V-/CV-Silbenphonotaktik.

Die Tests `tests/reference_nur_noch_fuenf_minuten.txt` und
`tests/independent_lunari_text.txt` werden durch
`scripts/test_lunari_text_roundtrip.ps1` vorwärts und rückwärts geprüft.

## Wissenschaftliche Grenze

Der lokale Bestand `words.full.list.build.txt` enthält Wortformen, aber keine
deutschen Bedeutungen, Wortarten, Übersetzungen oder Sinnunterscheidungen.
Der Textkanal garantiert daher bedeutungs- und strukturtreue Übertragung durch
eine reversible Lunari-Repräsentation. Er ist keine behauptete freie deutsche
zu englische Bedeutungsübersetzung. Für eine semantisch freie Übersetzung
werden zusätzlich überprüfbare deutsche Bedeutungsdaten benötigt; diese müssen
als getrennte Quelle mit Herkunft und Lizenz aufgenommen werden.

## Bedeutungs-Kanal

Zusätzlich existiert ein strikter **Bedeutung → Lunari**-Kanal. Er verwendet
nicht die deutschen Wortformen, sondern den evolvierten 15-Felder-
Bedeutungsrahmen (Sprechakt, Akteur, Handlung, Objekt, Relation, Negation,
Zeit, Menge und weitere pragmatische Felder). Eine vollständige Lunari-
Bedeutungsnachricht kann ohne Zugriff auf den deutschen Ausgangstext wieder
in diesen Bedeutungsrahmen dekodiert werden.

In der Oberfläche ist dieser Pfad über **Bedeutung → Lunari** erreichbar.
Der gegenwärtig überprüfte Kern akzeptiert deutsche oder englische Aussagen
mit einem bekannten Akteur, einer Handlung und einem Objekt, etwa:

`Der Mensch bewegt den Stein nicht gestern nahe.`

Der Rückweg zeigt alle dekodierten Bedeutungsfelder an. Bei Texten außerhalb
dieses überprüften Kerns lehnt der Bedeutungskanal die Analyse ab, statt eine
Bedeutung zu erfinden; hierfür bleibt der reversible Textkanal zuständig.
Kommandozeilenäquivalent:

```powershell
.\release\linguistische-emergenz-ng.exe --translate-meaning .\german-wordlist-only.leng constructed "Der Mensch bewegt den Stein nicht gestern nahe."
```

## Zwei unabhängige Instanzen

Der praktische Kommunikationsnachweis verwendet zwei getrennt gestartete
Konsoleninstanzen mit derselben unveränderten Lexikondatei. Nur der Sender
erhält die deutsche Geschichte; der Empfänger erhält ausschließlich den
erzeugten Lunari-Text. Der Test prüft danach sowohl die exakte
Rückgewinnung als auch die unveränderte SHA-256-Prüfsumme des gemeinsamen
Lexikons:

```powershell
.\scripts\test_lunari_two_instance_exchange.ps1
```

Für diesen eingefrorenen Austausch müssen die Wörter der Geschichte bereits
im persistenten Lexikon vorhanden sein. Neue Wörter müssen zunächst in einen
gemeinsamen, gespeicherten Lexikonstand aufgenommen werden, bevor ein anderer
Empfänger sie ohne deutsche Vorlage verstehen kann.

========== END FILE ==========

========== FILE: Lunari-Developer/README.md ==========

# LINGUISTISCHE EMERGENZ NG

Eigenständige native Forschungsengine für evolutionäre, kompositorische Agentensprachen. Das Projekt erzeugt keine vorgegebene Kunstsprache: Es variiert die sprachliche Realisierung eines unabhängigen semantischen Aufgabenraums, lässt getrennte Agenten lernen und bewertet Kandidaten auf zuvor nicht verwendeten Bedeutungskombinationen.

## Aktueller ausführbarer Umfang

`linguistische-emergenz-ng` enthält einen reproduzierbaren CPU-Referenzpfad mit:

- strukturierten Aussagen, Fragen und Aufforderungen;
- unabhängigen Sender-/Empfänger-Wissenszuständen;
- feedbackbasiertem Lernen nach abgeschlossenen Trainingsversuchen;
- phonologisch kontrollierten Wortformen und fester Kompositionsgrammatik;
- Mutation, Elitenselektion und Pareto-Archiv;
- getrennten Trainings- und Evaluierungsaufgaben;
- persistentem, versionsbezeichnetem Forschungszustand.
- einem persistenten, offenen Konzeptlexikon innerhalb derselben Engine: Grundkonzepte, Beziehungen, Wortfamilien, Quellenherkunft und agentische Form-Rekonstruktion.
- einer nativen Win32-Forschungsoberfläche mit Lauf-, Speicher-, Lade-, Sprach-, Kommunikations-, Evolutions-, Populations-, Export- und menschlichem Übungsbereich.
- einem OpenCL-GPU-Batch für die Kandidatenreihung, der gegen dieselbe CPU-Formel geprüft wird und transparent auf CPU zurückfällt.
- einer stabilen lokalen C-ABI (`leng_api.dll`) für künftige externe Forschungssteuerungen.

Die Konzeptbasis startet bewusst klein und nachvollziehbar: sieben lokal kuratierte Grundkonzepte und zwei aus simulierten Beziehungen erzeugte Komposita. Sie ist kein kopiertes Fremdwörterbuch. Externe Wissensquellen sind als nächster Ausbau vorgesehen und müssen dann pro Eintrag Herkunft und Lizenz erhalten. Der Übungsbereich protokolliert reale Produktions- und Interpretationsantworten, ersetzt aber keine Humanstudie. Deshalb ist dies ausdrücklich **kein finaler Abschluss des Gesamtauftrags**.

## Release-Build (KR Compiler Toolchain)

Der verbindliche Windows-Release verwendet ausschließlich die im KR Compiler IDE-Paket enthaltene Toolchain und bindet die GCC/C++- und pthread-Laufzeit statisch ein:

```powershell
powershell -ExecutionPolicy Bypass -File .\scripts\build_kr_release.ps1
```

Die auslieferbaren Programme stehen danach unter `release\`. Sie benötigen keine `libstdc++-6.dll` neben der EXE oder im `PATH`.

## Entwickler-Build (MSYS2 MinGW)

```powershell
$env:PATH = 'C:\msys64\mingw64\bin;C:\msys64\usr\bin;' + $env:PATH
$env:CXX = 'g++'
cmake -S . -B build -G 'Unix Makefiles' -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel 4
ctest --test-dir build --output-on-failure
```

## Start

```powershell
.\build\linguistische-emergenz-ng.exe --run .\research-demo.leng 12
.\build\linguistische-emergenz-ng.exe --resume .\research-demo.leng 6
.\build\linguistische-emergenz-ng.exe --report .\research-demo.leng
.\build\linguistische-emergenz-ng-ui.exe
```

Kommunikations- und Evolutionsnachweise können auch ohne GUI ausgegeben werden:

```powershell
.\release\linguistische-emergenz-ng.exe --communication .\research-demo.leng
.\release\linguistische-emergenz-ng.exe --evolution .\research-demo.leng
.\release\linguistische-emergenz-ng.exe --lexicon .\research-demo.leng
.\release\linguistische-emergenz-ng.exe --concept-communication .\research-demo.leng
.\release\linguistische-emergenz-ng.exe --import-concepts .\research-demo.leng .\concept-import-template.tsv
.\release\linguistische-emergenz-ng.exe --compare .\research-demo.leng .\research-e2e.leng
.\release\linguistische-emergenz-ng.exe --export .\research-demo.leng .\research-export
```

Der Zustand wird ausschließlich in der angegebenen `.leng`-Datei gespeichert. `--resume` ergänzt Generationen, statt einen vorhandenen Forschungsstand zu überschreiben.

## Autonomes Lexikon

`--lexicon` und die Schaltfläche **Lexicon lab** zeigen den persistenten Konzeptgraphen. Jeder Eintrag enthält eine stabile Konzept-ID, Bedeutung, Wortart, semantische Merkmale, Beziehungen, Wortbildungsweg, Wissensquelle, Entstehungsgeneration sowie Anzahl und Erfolg der Agentenprüfungen.

Neue abgeleitete Konzepte erhalten keine zufällige isolierte Form: Aus den bekannten Formen für `forest` und `fire` wird beispielsweise die zusammengesetzte Form für `forest-fire`. Die Prüfung lässt Empfänger die Bestandteile der gesendeten Form zu Bedeutungsnamen zurückführen; interne `CONCEPT_*`-IDs werden dabei nicht übertragen oder verglichen. Der Export schreibt das Ergebnis nach `evolutionary_lexicon.txt`.

Für kontrollierte Wissensaufnahme importiert `--import-concepts` UTF-8-TSV-Dateien mit sechs Spalten: `label`, `part_of_speech`, `semantics`, `relations`, `source`, `provenance`. Quelle und Provenienz sind Pflichtfelder. [concept-import-template.tsv](H:/Linguistische-Emergenz-System/Linguistische-Emergenz-NG/concept-import-template.tsv) enthält zwei Wikidata/CC0-Beispiele. In der Oberfläche **Import TSV…** klicken: Bei leerem Eingabefeld öffnet sich ein Dateiauswahldialog, bei einem eingetragenen Pfad wird dieser direkt verwendet. Anschließend **Save state** drücken.

Für einen reproduzierbaren Live-Abruf frei lizenzierter Basisdaten:

```powershell
.\scripts\search_wikidata_concepts.ps1 -Search rainfall,forest,music -Limit 5
.\scripts\fetch_wikidata_concepts.ps1 -Qid Q7925,Q638 -Output .\wikidata-concepts.tsv
.\release\linguistische-emergenz-ng.exe --import-concepts .\research-demo.leng .\wikidata-concepts.tsv
```

Die Suche dient bewusst der Bedeutungsprüfung: `forest` kann beispielsweise eine Gemeinde oder einen Wald bezeichnen. Erst die ausgewählte Q-ID wird abgerufen. Der Abruf verwendet die öffentliche Wikidata-API für englische Labels, Beschreibungen und eine begrenzte Menge strukturierter Kanten (`is-a`, `part-of`, `has-part`, `has-cause`, `causes`) und schreibt `CC0`, Quell-URL sowie UTC-Abrufzeit in die Provenienzspalte. Er überträgt keine fremden Wortformen in die Kunstsprache.

Eine Beziehung auf ein bereits bekanntes Label kann dabei eine weitere Wortfamilie erzeugen: `can-affect:forest` beim importierten Konzept `rain` führt zu `rain-forest`. Diese Ableitung bleibt als `relation-compound` mit der auslösenden Relation im Lexikon nachvollziehbar.

**Concept phrase** in der Oberfläche (beziehungsweise `--concept-communication`) demonstriert die Verwendung einer abgeleiteten Form in einer neuen Mehrkonzept-Nachricht. Der Empfänger zerlegt die bekannte Form für `forest-fire`, rekonstruiert deren Bestandteile und versteht die neue Phrase `forest-fire danger`, ohne interne Konzept-IDs zu erhalten.

## Semantic translator

The **Translate DE**, **Translate EN**, and **To language** controls translate
through persistent concept identities and grammar terms. The console equivalent
is `--translate <state.leng> <de|en|constructed> <text>`. Input is recognized
as constructed forms, English labels, or reviewed German glosses. A label that
has no reviewed German gloss is retained and explicitly reported; the engine
does not fabricate a German translation for imported Wikidata concepts.

For an exact click-by-click introduction to the native interface, see the
German [GUI user manual](GUI_BENUTZERHANDBUCH.md).

## Simple user interface

`release/linguistische-emergenz-ng-user.exe` is the responsive, end-user
interface. It loads `wikidata-100k.leng` automatically when present, offers a
single checkpointed **Wissen lernen / Learn knowledge** action, a two-column
translator, and a compact dictionary. It is intentionally separate from the
technical research interface.

## CORTEX autonomous lexicon evolution

The native engine now provides a persistent, indexed concept graph and a
resumable import path for large local Wikidata exports. Use `--lexicon-status`
for evidence-separated counters, `--search-lexicon <state> <query>` for
inspection, and `--autonomous-step <state> <tsv> [rows]` for a saved batch.
`scripts/convert_wikidata_dump.ps1` converts an official local dump without
uncontrolled endpoint downloads; `scripts/collect_wikidata_range.ps1` provides
a rate-limited, checkpointed official API intake. See [LEXICON_EVOLUTION.md](LEXICON_EVOLUTION.md)
for schema, evidence rules, resumption and the measured scale result.

The non-negotiable final acceptance evidence is tracked in
[ACCEPTANCE_100K.md](ACCEPTANCE_100K.md); capacity fixtures never satisfy the
external Wikidata milestone.

========== END FILE ==========

========== FILE: Lunari-Developer/include/leng/c_api.h ==========

#pragma once

/* Stable, allocation-owning C boundary for local research orchestration. */
#ifdef _WIN32
#  ifdef LENG_API_BUILD
#    define LENG_API __declspec(dllexport)
#  else
#    define LENG_API __declspec(dllimport)
#  endif
#else
#  define LENG_API __attribute__((visibility("default")))
#endif

#ifdef __cplusplus
extern "C" {
#endif

typedef struct leng_engine leng_engine;

/* Returned strings remain valid until the next call on the same engine. */
LENG_API leng_engine* leng_engine_create(unsigned long long seed, int population,
                                         int generations, int training_rounds,
                                         int evaluation_rounds);
LENG_API void leng_engine_destroy(leng_engine* engine);
LENG_API int leng_engine_run(leng_engine* engine);
LENG_API int leng_engine_load(leng_engine* engine, const char* path);
LENG_API int leng_engine_save(const leng_engine* engine, const char* path);
LENG_API int leng_engine_export_results(const leng_engine* engine, const char* directory);
LENG_API const char* leng_engine_report(leng_engine* engine);
LENG_API const char* leng_engine_last_error(const leng_engine* engine);

#ifdef __cplusplus
}
#endif

========== END FILE ==========

========== FILE: Lunari-Developer/include/leng/compute.hpp ==========

#pragma once

#include <string>
#include <vector>

namespace leng {

struct ComputeInfo {
  bool opencl_loader_available{};
  bool device_available{};
  bool gpu_available{};
  std::vector<std::string> devices;
  std::string diagnostic;
};

struct ComputeParity {
  bool attempted{};
  bool passed{};
  bool skipped{};
  double max_absolute_error{};
  std::string detail;
};

struct RewardBatch {
  std::vector<double> values;
  bool gpu_used{};
  std::string diagnostic;
};

// Runtime probe only: it does not select a hard-coded device index and does
// not make the application depend on an OpenCL ICD at startup.
[[nodiscard]] ComputeInfo inspect_compute();
// Executes the same reward-vector calculation on a dynamically selected GPU
// and on CPU, then compares each result with a defined tolerance.
[[nodiscard]] ComputeParity run_reward_parity_test();
// Evaluates a whole candidate-fitness batch on an automatically selected GPU.
// It returns the numerically equivalent CPU result if OpenCL is unavailable.
[[nodiscard]] RewardBatch compute_reward_batch(const std::vector<double>& benefit,
                                               const std::vector<double>& cost,
                                               double penalty);

}  // namespace leng

========== END FILE ==========

========== FILE: Lunari-Developer/include/leng/engine.hpp ==========

#pragma once

#include <cstdint>
#include <filesystem>
#include <string>
#include <vector>

namespace leng {

enum class SpeechAct { statement, question, request };

struct Meaning {
  SpeechAct act{SpeechAct::statement};
  std::string actor;
  std::string action;
  std::string object;
  std::string relation;
  bool negated{false};
  int time{0};
  int quantity{1};
  std::string modality{"certain"};
  std::string probability{"likely"};
  std::string emotion{"neutral"};
  std::string social_relation{"peer"};
  std::string goal{"inform"};
  std::string proposition_level{"direct"};
  std::string abstraction{"literal"};
  [[nodiscard]] std::string key() const;
  [[nodiscard]] bool operator==(const Meaning&) const = default;
};

struct Metrics {
  double communication_success{};
  double semantic_accuracy{};
  double cost{};
  double pronounceability{};
  double grammar_complexity{};
  double robustness{};
  double expressiveness{};
  [[nodiscard]] double score() const;
};

struct CandidateSummary {
  std::uint64_t id{};
  std::uint64_t parent_id{};
  Metrics metrics{};
  std::string sample;
};

struct HumanExercise {
  std::string prompt;
  std::string expected_answer;
  std::uint64_t issued_unix_ms{};
  bool interpretation{};
};

// Persisted outcome of one bounded intake operation.  The caller can save the
// engine after every batch and resume exactly at the recorded source line.
struct ImportProgress {
  std::size_t source_line{};
  std::size_t rows_seen{};
  std::size_t rows_accepted{};
  std::size_t concepts_created{};
  std::size_t concepts_enriched{};
  std::size_t concepts_rejected{};
  std::size_t relations_created{};
  bool complete{};
  std::string source_fingerprint;
  std::string status;
};

struct RunConfig {
  std::uint64_t seed{20261101};
  int population{12};
  int generations{12};
  int training_rounds{160};
  int evaluation_rounds{80};
  // A word-list-only state must not silently add demonstration concepts.
  bool seed_concept_lexicon{true};
};

class ResearchEngine {
 public:
  explicit ResearchEngine(RunConfig config = {});
  ~ResearchEngine();
  ResearchEngine(const ResearchEngine&) = delete;
  ResearchEngine& operator=(const ResearchEngine&) = delete;
  void run();
  void save(const std::filesystem::path& path) const;
  void load(const std::filesystem::path& path);
  // Imports UTF-8 TSV rows: label, part_of_speech, semantics, relations,
  // source, provenance. The source and provenance columns are mandatory.
  [[nodiscard]] std::size_t import_concepts(const std::filesystem::path& tsv_path);
  // Bounded, resumable variant for large local or pre-fetched Wikimedia TSV
  // sources.  `max_rows` counts data rows, never header/comment rows.
  [[nodiscard]] ImportProgress import_concepts_batch(const std::filesystem::path& tsv_path,
                                                      std::size_t max_rows);
  // Imports a local German word-form list one line at a time. These entries
  // are kept separate from externally attested concepts: their meanings and
  // licence are intentionally marked unverified unless supplied elsewhere.
  [[nodiscard]] ImportProgress import_german_word_list_batch(const std::filesystem::path& word_list_path,
                                                              std::size_t max_words);
  // Runs independent sender/receiver checks for language-realised concepts
  // that have not yet received the requested number of trials.
  [[nodiscard]] std::size_t validate_concepts(std::size_t max_concepts = 0);
  void export_results(const std::filesystem::path& directory) const;
  [[nodiscard]] std::string report() const;
  [[nodiscard]] const std::vector<CandidateSummary>& archive() const;
  [[nodiscard]] std::string export_language() const;
  [[nodiscard]] std::string lexicon_trace() const;
  [[nodiscard]] std::string lexicon_status() const;
  [[nodiscard]] std::string search_lexicon(const std::string& query, std::size_t limit = 50) const;
  // Human-readable, paginated dictionary for the persistent concept forms.
  [[nodiscard]] std::string dictionary_page(const std::string& query, std::size_t page,
                                            std::size_t page_size = 20) const;
  [[nodiscard]] std::string lexicon_benchmark() const;
  // Translates through the persistent semantic lexicon. `target_language` is
  // `de`, `en`, or `constructed`; input language is detected from known forms.
  // Unknown labels are reported explicitly instead of being invented.
  [[nodiscard]] std::string translate(const std::string& text,
                                      const std::string& target_language) const;
  // Structure-preserving text channel. Unlike the semantic lab translator it
  // keeps punctuation, whitespace, case, dialogue marks, and unknown tokens.
  [[nodiscard]] std::string translate_text(const std::string& text,
                                           const std::string& target_language);
  // Strict meaning channel for the reviewed semantic core. It encodes a
  // language-independent Meaning frame rather than source word forms. A
  // source sentence is rejected when actor, action, or object cannot be
  // identified; no unknown meaning is guessed.
  [[nodiscard]] std::string translate_meaning(const std::string& text,
                                              const std::string& target_language) const;
  [[nodiscard]] std::string concept_communication_trace() const;
  [[nodiscard]] std::string productive_grammar_trace() const;
  [[nodiscard]] std::string phonotactic_trace() const;
  [[nodiscard]] std::string communication_trace() const;
  [[nodiscard]] std::string evolution_trace() const;
  [[nodiscard]] std::string population_trace() const;
  [[nodiscard]] HumanExercise create_human_exercise() const;
  [[nodiscard]] bool check_human_exercise(const HumanExercise& exercise, const std::string& answer);
  [[nodiscard]] std::string human_lab_trace() const;
  [[nodiscard]] int generation() const;

 private:
  struct Impl;
  Impl* impl_;
};

bool self_test(std::string& error);
[[nodiscard]] std::string compare_research_states(const std::filesystem::path& left,
                                                   const std::filesystem::path& right);

}  // namespace leng

========== END FILE ==========

========== FILE: Lunari-Developer/src/c_api.cpp ==========

#include "leng/c_api.h"
#include "leng/engine.hpp"

#include <exception>
#include <memory>
#include <string>

struct leng_engine {
  std::unique_ptr<leng::ResearchEngine> value;
  std::string scratch;
  std::string error;
};

namespace {
template <typename Action>
int invoke(leng_engine* engine, Action&& action) {
  if (!engine || !engine->value) return 0;
  try { action(); engine->error.clear(); return 1; }
  catch (const std::exception& exception) { engine->error = exception.what(); return 0; }
  catch (...) { engine->error = "unknown native engine error"; return 0; }
}
}

leng_engine* leng_engine_create(unsigned long long seed, int population, int generations,
                                int training_rounds, int evaluation_rounds) {
  try {
    leng::RunConfig config;
    config.seed = seed;
    config.population = population;
    config.generations = generations;
    config.training_rounds = training_rounds;
    config.evaluation_rounds = evaluation_rounds;
    auto engine = std::make_unique<leng_engine>();
    engine->value = std::make_unique<leng::ResearchEngine>(config);
    return engine.release();
  } catch (...) { return nullptr; }
}

void leng_engine_destroy(leng_engine* engine) { delete engine; }
int leng_engine_run(leng_engine* engine) { return invoke(engine, [&] { engine->value->run(); }); }
int leng_engine_load(leng_engine* engine, const char* path) {
  return path ? invoke(engine, [&] { engine->value->load(path); }) : 0;
}
int leng_engine_save(const leng_engine* engine, const char* path) {
  return (engine && path) ? invoke(const_cast<leng_engine*>(engine), [&] { engine->value->save(path); }) : 0;
}
int leng_engine_export_results(const leng_engine* engine, const char* directory) {
  return (engine && directory) ? invoke(const_cast<leng_engine*>(engine), [&] { engine->value->export_results(directory); }) : 0;
}
const char* leng_engine_report(leng_engine* engine) {
  if (!engine || !engine->value) return "";
  if (!invoke(engine, [&] { engine->scratch = engine->value->report(); })) return "";
  return engine->scratch.c_str();
}
const char* leng_engine_last_error(const leng_engine* engine) {
  return engine ? engine->error.c_str() : "invalid engine handle";
}

========== END FILE ==========

========== FILE: Lunari-Developer/src/compute.cpp ==========

#include "leng/compute.hpp"

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
#include <cstring>
#include <stdexcept>
#include <vector>

#ifdef _WIN32
#include <windows.h>
#else
#include <dlfcn.h>
#endif

namespace leng {
namespace {
using ClInt = std::int32_t;
using ClUInt = std::uint32_t;
using ClULong = std::uint64_t;
using ClPlatformId = void*;
using ClDeviceId = void*;
constexpr ClInt kSuccess = 0;
constexpr ClULong kDeviceTypeAll = 0xFFFFFFFFull;
constexpr ClULong kDeviceTypeGpu = 1ull << 2;
constexpr ClUInt kDeviceName = 0x102B;
using GetPlatforms = ClInt (*)(ClUInt, ClPlatformId*, ClUInt*);
using GetDevices = ClInt (*)(ClPlatformId, ClULong, ClUInt, ClDeviceId*, ClUInt*);
using GetDeviceInfo = ClInt (*)(ClDeviceId, ClUInt, std::size_t, void*, std::size_t*);
using ClContext = void*;
using ClQueue = void*;
using ClProgram = void*;
using ClKernel = void*;
using ClMem = void*;
using CreateContext = ClContext (*)(const std::intptr_t*, ClUInt, const ClDeviceId*, void*, void*, ClInt*);
using CreateCommandQueue = ClQueue (*)(ClContext, ClDeviceId, ClULong, ClInt*);
using CreateProgramWithSource = ClProgram (*)(ClContext, ClUInt, const char**, const std::size_t*, ClInt*);
using BuildProgram = ClInt (*)(ClProgram, ClUInt, const ClDeviceId*, const char*, void*, void*);
using CreateKernel = ClKernel (*)(ClProgram, const char*, ClInt*);
using CreateBuffer = ClMem (*)(ClContext, ClULong, std::size_t, void*, ClInt*);
using SetKernelArg = ClInt (*)(ClKernel, ClUInt, std::size_t, const void*);
using EnqueueWriteBuffer = ClInt (*)(ClQueue, ClMem, ClUInt, std::size_t, std::size_t, const void*, ClUInt, const void*, void*);
using EnqueueReadBuffer = EnqueueWriteBuffer;
using EnqueueNdRangeKernel = ClInt (*)(ClQueue, ClKernel, ClUInt, const std::size_t*, const std::size_t*, const std::size_t*, ClUInt, const void*, void*);
using Finish = ClInt (*)(ClQueue);
using Release = ClInt (*)(void*);
constexpr ClULong kMemReadOnly = 1ull << 2;
constexpr ClULong kMemWriteOnly = 1ull << 1;
constexpr ClUInt kTrue = 1;

class DynamicLibrary {
 public:
  DynamicLibrary() {
#ifdef _WIN32
    handle_ = LoadLibraryA("OpenCL.dll");
#else
    handle_ = dlopen("libOpenCL.so.1", RTLD_LAZY | RTLD_LOCAL);
    if (!handle_) handle_ = dlopen("libOpenCL.so", RTLD_LAZY | RTLD_LOCAL);
#endif
  }
  ~DynamicLibrary() {
#ifdef _WIN32
    if (handle_) FreeLibrary(handle_);
#else
    if (handle_) dlclose(handle_);
#endif
  }
  [[nodiscard]] bool available() const { return handle_ != nullptr; }
  template <typename Function>
  [[nodiscard]] Function symbol(const char* name) const {
#ifdef _WIN32
    const auto source = GetProcAddress(handle_, name);
#else
    const auto source = dlsym(handle_, name);
#endif
    Function target{};
    static_assert(sizeof(target) == sizeof(source), "unsupported function pointer ABI");
    std::memcpy(&target, &source, sizeof(target));
    return target;
  }
 private:
#ifdef _WIN32
  HMODULE handle_{};
#else
  void* handle_{};
#endif
};
}  // namespace

ComputeInfo inspect_compute() {
  ComputeInfo result;
  DynamicLibrary library;
  if (!library.available()) { result.diagnostic = "OpenCL loader not installed; CPU reference backend selected."; return result; }
  result.opencl_loader_available = true;
  const auto get_platforms = library.symbol<GetPlatforms>("clGetPlatformIDs");
  const auto get_devices = library.symbol<GetDevices>("clGetDeviceIDs");
  const auto get_device_info = library.symbol<GetDeviceInfo>("clGetDeviceInfo");
  if (!get_platforms || !get_devices || !get_device_info) { result.diagnostic = "OpenCL loader is incomplete; CPU reference backend selected."; return result; }
  ClUInt count{};
  if (get_platforms(0, nullptr, &count) != kSuccess || count == 0) { result.diagnostic = "No OpenCL platforms found; CPU reference backend selected."; return result; }
  std::vector<ClPlatformId> platforms(count);
  if (get_platforms(count, platforms.data(), nullptr) != kSuccess) { result.diagnostic = "OpenCL platform enumeration failed; CPU reference backend selected."; return result; }
  for (const auto platform : platforms) {
    ClUInt device_count{};
    if (get_devices(platform, kDeviceTypeAll, 0, nullptr, &device_count) != kSuccess || device_count == 0) continue;
    std::vector<ClDeviceId> devices(device_count);
    if (get_devices(platform, kDeviceTypeAll, device_count, devices.data(), nullptr) != kSuccess) continue;
    for (const auto device : devices) {
      std::size_t bytes{};
      if (get_device_info(device, kDeviceName, 0, nullptr, &bytes) != kSuccess || bytes == 0) continue;
      std::string name(bytes, '\0');
      if (get_device_info(device, kDeviceName, name.size(), name.data(), nullptr) != kSuccess) continue;
      if (!name.empty() && name.back() == '\0') name.pop_back();
      result.devices.push_back(name);
    }
    ClUInt gpu_count{};
    if (get_devices(platform, kDeviceTypeGpu, 0, nullptr, &gpu_count) == kSuccess && gpu_count > 0) result.gpu_available = true;
  }
  result.device_available = !result.devices.empty();
  result.diagnostic = result.device_available ? (result.gpu_available ? "OpenCL GPU available; CPU reference backend remains selectable." : "OpenCL non-GPU device available; CPU reference backend remains selectable.") : "No usable OpenCL devices found; CPU reference backend selected.";
  return result;
}

ComputeParity run_reward_parity_test() {
  ComputeParity result;
  DynamicLibrary library;
  if (!library.available()) { result.skipped = true; result.detail = "Skipped: OpenCL loader unavailable; CPU fallback is active."; return result; }
  const auto get_platforms = library.symbol<GetPlatforms>("clGetPlatformIDs");
  const auto get_devices = library.symbol<GetDevices>("clGetDeviceIDs");
  const auto create_context = library.symbol<CreateContext>("clCreateContext");
  const auto create_queue = library.symbol<CreateCommandQueue>("clCreateCommandQueue");
  const auto create_program = library.symbol<CreateProgramWithSource>("clCreateProgramWithSource");
  const auto build_program = library.symbol<BuildProgram>("clBuildProgram");
  const auto create_kernel = library.symbol<CreateKernel>("clCreateKernel");
  const auto create_buffer = library.symbol<CreateBuffer>("clCreateBuffer");
  const auto set_arg = library.symbol<SetKernelArg>("clSetKernelArg");
  const auto write_buffer = library.symbol<EnqueueWriteBuffer>("clEnqueueWriteBuffer");
  const auto read_buffer = library.symbol<EnqueueReadBuffer>("clEnqueueReadBuffer");
  const auto enqueue_kernel = library.symbol<EnqueueNdRangeKernel>("clEnqueueNDRangeKernel");
  const auto finish = library.symbol<Finish>("clFinish");
  const auto release_context = library.symbol<Release>("clReleaseContext");
  const auto release_queue = library.symbol<Release>("clReleaseCommandQueue");
  const auto release_program = library.symbol<Release>("clReleaseProgram");
  const auto release_kernel = library.symbol<Release>("clReleaseKernel");
  const auto release_mem = library.symbol<Release>("clReleaseMemObject");
  if (!get_platforms || !get_devices || !create_context || !create_queue || !create_program || !build_program || !create_kernel || !create_buffer || !set_arg || !write_buffer || !read_buffer || !enqueue_kernel || !finish || !release_context || !release_queue || !release_program || !release_kernel || !release_mem) { result.skipped = true; result.detail = "Skipped: required OpenCL 1.2 symbols are unavailable."; return result; }
  ClUInt platform_count{};
  if (get_platforms(0, nullptr, &platform_count) != kSuccess || platform_count == 0) { result.skipped = true; result.detail = "Skipped: no OpenCL platform."; return result; }
  std::vector<ClPlatformId> platforms(platform_count);
  if (get_platforms(platform_count, platforms.data(), nullptr) != kSuccess) { result.skipped = true; result.detail = "Skipped: platform enumeration failed."; return result; }
  ClDeviceId device{};
  for (const auto platform : platforms) { ClUInt count{}; if (get_devices(platform, kDeviceTypeGpu, 0, nullptr, &count) == kSuccess && count > 0) { std::vector<ClDeviceId> candidates(count); if (get_devices(platform, kDeviceTypeGpu, count, candidates.data(), nullptr) == kSuccess) { device = candidates.front(); break; } } }
  if (!device) { result.skipped = true; result.detail = "Skipped: no OpenCL GPU device; CPU fallback is active."; return result; }
  result.attempted = true;
  ClInt error{};
  ClContext context = create_context(nullptr, 1, &device, nullptr, nullptr, &error);
  if (!context || error != kSuccess) { result.detail = "GPU parity test failed: context creation."; return result; }
  ClQueue queue = create_queue(context, device, 0, &error);
  if (!queue || error != kSuccess) { release_context(context); result.detail = "GPU parity test failed: command queue creation."; return result; }
  constexpr char kernel_source[] = "__kernel void reward(__global const float* success,__global const float* cost,__global float* out,float penalty){size_t i=get_global_id(0);out[i]=success[i]-penalty*cost[i];}";
  const char* source_ptr = kernel_source;
  const std::size_t source_size = sizeof(kernel_source) - 1;
  ClProgram program = create_program(context, 1, &source_ptr, &source_size, &error);
  if (!program || error != kSuccess || build_program(program, 1, &device, "", nullptr, nullptr) != kSuccess) { if (program) release_program(program); release_queue(queue); release_context(context); result.detail = "GPU parity test failed: kernel compilation."; return result; }
  ClKernel kernel = create_kernel(program, "reward", &error);
  if (!kernel || error != kSuccess) { release_program(program); release_queue(queue); release_context(context); result.detail = "GPU parity test failed: kernel creation."; return result; }
  constexpr std::size_t n = 1024;
  std::array<float, n> success{}; std::array<float, n> cost{}; std::array<float, n> gpu{}; std::array<float, n> cpu{};
  constexpr float penalty = 0.125f;
  for (std::size_t i = 0; i < n; ++i) { success[i] = static_cast<float>((i * 17) % 101) / 100.0f; cost[i] = static_cast<float>((i * 31) % 97) / 97.0f; cpu[i] = success[i] - penalty * cost[i]; }
  const std::size_t bytes = sizeof(float) * n;
  ClMem success_mem = create_buffer(context, kMemReadOnly, bytes, nullptr, &error); ClMem cost_mem = create_buffer(context, kMemReadOnly, bytes, nullptr, &error); ClMem out_mem = create_buffer(context, kMemWriteOnly, bytes, nullptr, &error);
  const bool buffers_ok = success_mem && cost_mem && out_mem && error == kSuccess;
  const bool io_ok = buffers_ok && write_buffer(queue, success_mem, kTrue, 0, bytes, success.data(), 0, nullptr, nullptr) == kSuccess && write_buffer(queue, cost_mem, kTrue, 0, bytes, cost.data(), 0, nullptr, nullptr) == kSuccess;
  const bool args_ok = io_ok && set_arg(kernel, 0, sizeof(success_mem), &success_mem) == kSuccess && set_arg(kernel, 1, sizeof(cost_mem), &cost_mem) == kSuccess && set_arg(kernel, 2, sizeof(out_mem), &out_mem) == kSuccess && set_arg(kernel, 3, sizeof(penalty), &penalty) == kSuccess;
  const std::size_t global = n;
  const bool run_ok = args_ok && enqueue_kernel(queue, kernel, 1, nullptr, &global, nullptr, 0, nullptr, nullptr) == kSuccess && finish(queue) == kSuccess && read_buffer(queue, out_mem, kTrue, 0, bytes, gpu.data(), 0, nullptr, nullptr) == kSuccess;
  if (success_mem) release_mem(success_mem);
  if (cost_mem) release_mem(cost_mem);
  if (out_mem) release_mem(out_mem);
  release_kernel(kernel);
  release_program(program);
  release_queue(queue);
  release_context(context);
  if (!run_ok) { result.detail = "GPU parity test failed: buffer transfer or kernel execution."; return result; }
  for (std::size_t i = 0; i < n; ++i) result.max_absolute_error = std::max(result.max_absolute_error, static_cast<double>(std::abs(cpu[i] - gpu[i])));
  result.passed = result.max_absolute_error <= 1e-6;
  result.detail = result.passed ? "GPU/CPU reward parity passed." : "GPU/CPU reward parity exceeded tolerance.";
  return result;
}

RewardBatch compute_reward_batch(const std::vector<double>& benefit, const std::vector<double>& cost, double penalty) {
  if (benefit.size() != cost.size()) throw std::invalid_argument("reward batch sizes differ");
  RewardBatch result;
  result.values.resize(benefit.size());
  for (std::size_t i = 0; i < benefit.size(); ++i) result.values[i] = benefit[i] - penalty * cost[i];
  if (benefit.empty()) { result.diagnostic = "empty CPU reward batch"; return result; }
  // The parity invocation exercises the same OpenCL 1.2 kernel ABI, and guards
  // selection against a driver whose arithmetic is not equivalent to the CPU.
  const auto parity = run_reward_parity_test();
  if (!parity.attempted || !parity.passed) { result.diagnostic = "CPU reward batch: " + parity.detail; return result; }
  DynamicLibrary library;
  const auto get_platforms = library.symbol<GetPlatforms>("clGetPlatformIDs");
  const auto get_devices = library.symbol<GetDevices>("clGetDeviceIDs");
  const auto create_context = library.symbol<CreateContext>("clCreateContext");
  const auto create_queue = library.symbol<CreateCommandQueue>("clCreateCommandQueue");
  const auto create_program = library.symbol<CreateProgramWithSource>("clCreateProgramWithSource");
  const auto build_program = library.symbol<BuildProgram>("clBuildProgram");
  const auto create_kernel = library.symbol<CreateKernel>("clCreateKernel");
  const auto create_buffer = library.symbol<CreateBuffer>("clCreateBuffer");
  const auto set_arg = library.symbol<SetKernelArg>("clSetKernelArg");
  const auto write_buffer = library.symbol<EnqueueWriteBuffer>("clEnqueueWriteBuffer");
  const auto read_buffer = library.symbol<EnqueueReadBuffer>("clEnqueueReadBuffer");
  const auto enqueue_kernel = library.symbol<EnqueueNdRangeKernel>("clEnqueueNDRangeKernel");
  const auto finish = library.symbol<Finish>("clFinish");
  const auto release_context = library.symbol<Release>("clReleaseContext"); const auto release_queue = library.symbol<Release>("clReleaseCommandQueue");
  const auto release_program = library.symbol<Release>("clReleaseProgram"); const auto release_kernel = library.symbol<Release>("clReleaseKernel"); const auto release_mem = library.symbol<Release>("clReleaseMemObject");
  if (!get_platforms || !get_devices || !create_context || !create_queue || !create_program || !build_program || !create_kernel || !create_buffer || !set_arg || !write_buffer || !read_buffer || !enqueue_kernel || !finish || !release_context || !release_queue || !release_program || !release_kernel || !release_mem) { result.diagnostic = "CPU reward batch: incomplete OpenCL loader"; return result; }
  ClUInt platforms{}; if (get_platforms(0, nullptr, &platforms) != kSuccess || !platforms) { result.diagnostic = "CPU reward batch: no OpenCL platform"; return result; }
  std::vector<ClPlatformId> ids(platforms); if (get_platforms(platforms, ids.data(), nullptr) != kSuccess) { result.diagnostic = "CPU reward batch: platform enumeration failed"; return result; }
  ClDeviceId device{}; for (auto id : ids) { ClUInt count{}; if (get_devices(id, kDeviceTypeGpu, 0, nullptr, &count) == kSuccess && count) { std::vector<ClDeviceId> choices(count); if (get_devices(id, kDeviceTypeGpu, count, choices.data(), nullptr) == kSuccess) { device = choices.front(); break; } } }
  if (!device) { result.diagnostic = "CPU reward batch: no GPU"; return result; }
  ClInt error{}; auto context = create_context(nullptr, 1, &device, nullptr, nullptr, &error); if (!context || error != kSuccess) { result.diagnostic = "CPU reward batch: context creation failed"; return result; }
  auto queue = create_queue(context, device, 0, &error); if (!queue || error != kSuccess) { release_context(context); result.diagnostic = "CPU reward batch: queue creation failed"; return result; }
  constexpr char source[] = "__kernel void reward(__global const float*b,__global const float*c,__global float*o,float p){size_t i=get_global_id(0);o[i]=b[i]-p*c[i];}"; const char* source_ptr = source; const std::size_t source_len = sizeof(source) - 1;
  auto program = create_program(context, 1, &source_ptr, &source_len, &error); if (!program || error != kSuccess || build_program(program, 1, &device, "", nullptr, nullptr) != kSuccess) { if (program) release_program(program); release_queue(queue); release_context(context); result.diagnostic = "CPU reward batch: kernel build failed"; return result; }
  auto kernel = create_kernel(program, "reward", &error); if (!kernel || error != kSuccess) { release_program(program); release_queue(queue); release_context(context); result.diagnostic = "CPU reward batch: kernel creation failed"; return result; }
  std::vector<float> b(benefit.begin(), benefit.end()), c(cost.begin(), cost.end()), out(benefit.size()); const std::size_t bytes = b.size() * sizeof(float); const float p = static_cast<float>(penalty);
  auto bm = create_buffer(context, kMemReadOnly, bytes, nullptr, &error); auto cm = create_buffer(context, kMemReadOnly, bytes, nullptr, &error); auto om = create_buffer(context, kMemWriteOnly, bytes, nullptr, &error);
  const std::size_t global = b.size(); const bool ok = bm && cm && om && error == kSuccess && write_buffer(queue,bm,kTrue,0,bytes,b.data(),0,nullptr,nullptr)==kSuccess && write_buffer(queue,cm,kTrue,0,bytes,c.data(),0,nullptr,nullptr)==kSuccess && set_arg(kernel,0,sizeof(bm),&bm)==kSuccess && set_arg(kernel,1,sizeof(cm),&cm)==kSuccess && set_arg(kernel,2,sizeof(om),&om)==kSuccess && set_arg(kernel,3,sizeof(p),&p)==kSuccess && enqueue_kernel(queue,kernel,1,nullptr,&global,nullptr,0,nullptr,nullptr)==kSuccess && finish(queue)==kSuccess && read_buffer(queue,om,kTrue,0,bytes,out.data(),0,nullptr,nullptr)==kSuccess;
  if (bm) release_mem(bm);
  if (cm) release_mem(cm);
  if (om) release_mem(om);
  release_kernel(kernel);
  release_program(program);
  release_queue(queue);
  release_context(context);
  if (!ok) { result.diagnostic = "CPU reward batch: GPU execution failed"; return result; }
  for (std::size_t i = 0; i < out.size(); ++i) { if (std::abs(result.values[i] - static_cast<double>(out[i])) > 1e-5) { result.diagnostic = "CPU reward batch: parity tolerance exceeded"; return result; } result.values[i] = out[i]; }
  result.gpu_used = true; result.diagnostic = "OpenCL GPU reward batch"; return result;
}
}  // namespace leng

========== END FILE ==========

========== FILE: Lunari-Developer/src/engine.cpp ==========

#include "leng/engine.hpp"
#include "leng/compute.hpp"

#include <algorithm>
#include <array>
#include <chrono>
#include <cctype>
#include <cmath>
#include <fstream>
#include <iomanip>
#include <map>
#include <optional>
#include <random>
#include <set>
#include <sstream>
#include <stdexcept>
#include <string_view>
#include <unordered_map>
#include <unordered_set>

namespace leng {
namespace {
constexpr std::array<const char*, 5> kActors{"human", "machine", "child", "researcher", "agent"};
constexpr std::array<const char*, 5> kActions{"move", "give", "observe", "build", "ask"};
constexpr std::array<const char*, 5> kObjects{"stone", "signal", "map", "water", "idea"};
constexpr std::array<const char*, 4> kRelations{"near", "before", "because", "if"};
constexpr std::array<const char*, 3> kModalities{"certain", "possible", "required"};
constexpr std::array<const char*, 3> kProbabilities{"unlikely", "likely", "unknown"};
constexpr std::array<const char*, 3> kEmotions{"neutral", "joy", "concern"};
constexpr std::array<const char*, 3> kSocialRelations{"peer", "ally", "observer"};
constexpr std::array<const char*, 3> kGoals{"inform", "assist", "coordinate"};
constexpr std::array<const char*, 2> kPropositionLevels{"direct", "reported"};
constexpr std::array<const char*, 2> kAbstractions{"literal", "metaphoric"};
// Lunari phoneme inventory.  Every generated syllable is V or CV; codas and
// consonant clusters are deliberately outside the initial research space.
constexpr std::array<const char*, 14> kOnsets{"m", "n", "l", "r", "p", "t", "k", "b", "d", "g", "f", "s", "h", "v"};
constexpr std::array<const char*, 5> kVowels{"a", "e", "i", "o", "u"};
constexpr const char* kConceptMorphologyRuleVersion = "morphology-v3-lunari-open-syllables;relation-compound-v2";

std::string act_name(SpeechAct act) {
  switch (act) { case SpeechAct::statement: return "statement"; case SpeechAct::question: return "question"; case SpeechAct::request: return "request"; }
  return "statement";
}
SpeechAct parse_act(const std::string& value) {
  if (value == "question") return SpeechAct::question;
  if (value == "request") return SpeechAct::request;
  return SpeechAct::statement;
}
std::vector<std::string> atoms(const Meaning& m) {
  return {"act:" + act_name(m.act), "actor:" + m.actor, "action:" + m.action,
          "object:" + m.object, "relation:" + m.relation, std::string("neg:") + (m.negated ? "1" : "0"),
          "time:" + std::to_string(m.time), "qty:" + std::to_string(m.quantity),
          "modality:" + m.modality, "probability:" + m.probability, "emotion:" + m.emotion,
          "social:" + m.social_relation, "goal:" + m.goal, "proposition:" + m.proposition_level, "abstraction:" + m.abstraction};
}
const std::vector<std::string>& default_order() {
  static const std::vector<std::string> value{"act", "actor", "action", "object", "relation", "neg", "time", "qty", "modality", "probability", "emotion", "social", "goal", "proposition", "abstraction"};
  return value;
}
Meaning random_meaning(std::mt19937_64& rng, bool held_out) {
  std::uniform_int_distribution<int> act(0, 2), actor(0, 4), action(0, 4), object(0, 4), relation(0, 3), bit(0, 1), time(0, 2), quantity(1, 3);
  std::uniform_int_distribution<int> modality(0, 2), probability(0, 2), emotion(0, 2), social(0, 2), goal(0, 2), proposition(0, 1), abstraction(0, 1);
  Meaning m{static_cast<SpeechAct>(act(rng)), kActors[actor(rng)], kActions[action(rng)], kObjects[object(rng)], kRelations[relation(rng)], bit(rng) != 0, time(rng), quantity(rng), kModalities[modality(rng)], kProbabilities[probability(rng)], kEmotions[emotion(rng)], kSocialRelations[social(rng)], kGoals[goal(rng)], kPropositionLevels[proposition(rng)], kAbstractions[abstraction(rng)]};
  // Evaluation combines known primitives in combinations deliberately excluded from training.
  if (held_out) { m.relation = "if"; m.quantity = 3; m.negated = m.action == "ask" || m.object == "idea"; m.modality = "required"; m.proposition_level = "reported"; m.abstraction = "metaphoric"; }
  return m;
}
bool is_vowel(char value) { return std::string_view{"aeiou"}.find(value) != std::string_view::npos; }
bool is_onset(char value) { return std::string_view{"mnlrptkbdgfshv"}.find(value) != std::string_view::npos; }

struct PhonotacticScore { double structure{}; double transition{}; double length{}; double acoustic{}; double combined{}; bool open_syllables{}; };
PhonotacticScore phonotactic_score(const std::string& word) {
  PhonotacticScore score{1.0, 1.0, 1.0, 1.0, 0.0, true};
  if (word.empty()) return {};
  std::vector<std::string> syllables;
  std::string current;
  bool expecting_vowel = false;
  bool at_boundary = false;
  for (const char raw : word) {
    if (raw == '-') {
      if (expecting_vowel || syllables.empty() || at_boundary) return {};
      at_boundary = true; continue;
    }
    at_boundary = false;
    const char ch = static_cast<char>(std::tolower(static_cast<unsigned char>(raw)));
    if (!expecting_vowel) {
      if (is_vowel(ch)) { current.assign(1, ch); syllables.push_back(current); current.clear(); }
      else if (is_onset(ch)) { current.assign(1, ch); expecting_vowel = true; }
      else return {};
    } else {
      if (!is_vowel(ch)) return {};
      current.push_back(ch); syllables.push_back(current); current.clear(); expecting_vowel = false;
    }
  }
  if (expecting_vowel || (!current.empty() && !is_vowel(current.back())) || syllables.empty() || at_boundary) return {};
  // Each unit ends in a vowel, so word and morpheme joins cannot create a
  // consonant cluster. Repeated adjacent syllables remain possible but are
  // penalised as acoustically confusable.
  for (std::size_t i = 1; i < syllables.size(); ++i) {
    if (syllables[i] == syllables[i - 1]) { score.transition -= 0.14; score.acoustic -= 0.20; }
    if (syllables[i].back() == syllables[i - 1].back()) { score.transition -= 0.03; score.acoustic -= 0.04; }
    if (syllables[i].front() == syllables[i - 1].front()) score.acoustic -= 0.03;
  }
  const std::size_t count = syllables.size();
  score.length = count <= 2 ? 1.0 : count == 3 ? 0.98 : count == 4 ? 0.94 : count == 5 ? 0.86 : 0.72;
  score.transition = std::clamp(score.transition, 0.0, 1.0);
  score.acoustic = std::clamp(score.acoustic, 0.0, 1.0);
  score.combined = 0.30 * score.structure + 0.30 * score.transition + 0.20 * score.length + 0.20 * score.acoustic;
  return score;
}

std::string new_word(std::mt19937_64& rng, std::set<std::string>& used) {
  std::uniform_int_distribution<int> o(0, static_cast<int>(kOnsets.size() - 1)), v(0, static_cast<int>(kVowels.size() - 1)), syl(2, 3);
  for (;;) {
    std::string out;
    const int n = syl(rng);
    for (int i = 0; i < n; ++i) out += std::string(kOnsets[o(rng)]) + kVowels[v(rng)];
    if (phonotactic_score(out).combined >= 0.88 && used.insert(out).second) return out;
  }
}
double word_quality(const std::string& word) {
  return phonotactic_score(word).combined;
}

std::string new_concept_word(std::mt19937_64& rng, std::unordered_set<std::string>& used) {
  std::uniform_int_distribution<int> onset(0, static_cast<int>(kOnsets.size() - 1));
  std::uniform_int_distribution<int> vowel(0, static_cast<int>(kVowels.size() - 1));
  // Four CV syllables provide more than 24 million open-syllable forms.
  for (;;) {
    std::string out;
    for (int i = 0; i < 4; ++i) out += std::string(kOnsets[onset(rng)]) + kVowels[vowel(rng)];
    if (phonotactic_score(out).combined >= 0.84 && used.insert(out).second) return out;
  }
}

struct Candidate {
  std::uint64_t id{};
  std::uint64_t parent{};
  std::map<std::string, std::string> lexicon;
  // A small, separate inventory of reusable morphology.  These forms are not
  // semantic-frame slots and therefore do not force every message to carry a
  // person/number/tense value.
  std::map<std::string, std::string> morphology;
  std::vector<std::string> order;
  Metrics metrics{};
  double selection_score{};
};
void ensure_morphology(Candidate& candidate, std::mt19937_64& rng) {
  if (candidate.morphology.size() == 5) return;
  std::set<std::string> used;
  for (const auto& [_, word] : candidate.lexicon) used.insert(word);
  for (const auto& [_, word] : candidate.morphology) used.insert(word);
  for (const std::string key : {"person:3", "number:sg", "number:pl", "time:present", "time:past"})
    if (!candidate.morphology.contains(key)) candidate.morphology.emplace(key, new_word(rng, used));
}
std::vector<std::string> ordered_atoms(const Candidate& candidate, const Meaning& meaning) {
  std::map<std::string, std::string> by_field;
  for (const auto& atom : atoms(meaning)) by_field.emplace(atom.substr(0, atom.find(':')), atom);
  const auto& order = candidate.order.empty() ? default_order() : candidate.order;
  std::vector<std::string> result;
  result.reserve(order.size());
  for (const auto& field : order) result.push_back(by_field.at(field));
  return result;
}

struct AgentState {
  std::uint64_t id{};
  std::map<std::string, std::string> private_reverse_lexicon;
  std::uint64_t successful_interpretations{};
  std::uint64_t failed_interpretations{};
};

// This is deliberately separate from Meaning's compact sentence frame.  A
// concept can be added without adding another compulsory slot to every
// utterance, and its history remains available after a research run.
struct ConceptEntry {
  std::string id;
  // Stable external identity where available (for example Wikidata Q42).
  // Locally-derived entries deliberately leave this empty.
  std::string external_id;
  std::string label;
  std::string part_of_speech;
  std::string form;
  std::string derivation;
  std::string source;
  std::string provenance;
  std::string source_revision;
  // Categories are explicit evidence classes, never inferred from a label.
  // external-attested | simulated-relation | agent-hypothesis
  std::string evidence_class;
  // pending | realised | communicatively-validated | rejected
  std::string language_status;
  std::string semantics;
  std::string relations;
  int introduced_generation{};
  std::uint64_t validation_attempts{};
  std::uint64_t validation_successes{};
  std::uint64_t validation_failures{};
};

struct ImportCheckpoint {
  std::string source_path;
  std::string fingerprint;
  std::size_t next_line{1};
  std::size_t rows_seen{};
  std::size_t rows_accepted{};
  std::size_t rows_rejected{};
  bool complete{};
  std::string status{"idle"};
};

std::string join(const std::vector<std::string>& values, const char* separator = ",") {
  std::ostringstream out;
  for (const auto& value : values) { if (out.tellp() > 0) out << separator; out << value; }
  return out.str();
}

std::string trim(std::string value) {
  const auto not_space = [](unsigned char ch) { return !std::isspace(ch); };
  const auto begin = std::find_if(value.begin(), value.end(), not_space);
  const auto end = std::find_if(value.rbegin(), value.rend(), not_space).base();
  return begin >= end ? "" : std::string(begin, end);
}

std::string canonical_concept_key(std::string value) {
  value = trim(std::move(value));
  std::string out;
  bool pending_space = false;
  for (const unsigned char ch : value) {
    if (std::isspace(ch)) { pending_space = !out.empty(); continue; }
    if (pending_space) { out.push_back(' '); pending_space = false; }
    out.push_back(static_cast<char>(std::tolower(ch)));
  }
  return out;
}

std::vector<std::string> words(std::string value) {
  for (char& ch : value) {
    const unsigned char byte = static_cast<unsigned char>(ch);
    if (!std::isalnum(byte) && ch != '-' && byte < 128) ch = ' ';
  }
  std::istringstream in(canonical_concept_key(std::move(value)));
  std::vector<std::string> result;
  for (std::string word; in >> word;) result.push_back(std::move(word));
  return result;
}

bool text_token_byte(unsigned char value) {
  // UTF-8 bytes are retained together in a lexical token; only ASCII layout
  // and punctuation separate tokens. This keeps German umlauts and quotes.
  return value >= 128 || std::isalnum(value) || value == '_';
}
std::vector<std::pair<bool, std::string>> text_segments(const std::string& text) {
  std::vector<std::pair<bool, std::string>> result;
  std::string current; bool lexical = false;
  for (std::size_t index = 0; index < text.size();) {
    const unsigned char ch = static_cast<unsigned char>(text[index]);
    // German typographic quotation marks and dashes are separators, not
    // unknown words. Preserve their UTF-8 bytes exactly as layout text.
    std::size_t width = 1;
    bool next_lexical = text_token_byte(ch);
    if (ch == 0xE2 && index + 2 < text.size() && static_cast<unsigned char>(text[index + 1]) == 0x80) { width = 3; next_lexical = false; }
    if (ch == 0xC2 && index + 1 < text.size() && (static_cast<unsigned char>(text[index + 1]) == 0xAB || static_cast<unsigned char>(text[index + 1]) == 0xBB)) { width = 2; next_lexical = false; }
    if (!current.empty() && next_lexical != lexical) { result.emplace_back(lexical, std::move(current)); current.clear(); }
    lexical = next_lexical; current.append(text, index, width); index += width;
  }
  if (!current.empty()) result.emplace_back(lexical, std::move(current));
  return result;
}
std::string case_marker(const std::string& token) {
  bool has_alpha = false, all_upper = true, first_upper = false;
  for (std::size_t i = 0; i < token.size(); ++i) {
    const unsigned char ch = static_cast<unsigned char>(token[i]);
    if (!std::isalpha(ch) || ch >= 128) continue;
    has_alpha = true; if (std::islower(ch)) all_upper = false; if (i == 0 && std::isupper(ch)) first_upper = true;
  }
  if (!has_alpha) return "ma";       // unchanged / lower case
  if (all_upper) return "mi";         // upper case
  if (first_upper) return "me";       // title case
  return "ma";                        // lower case
}
std::string apply_case_marker(std::string value, const std::string& marker) {
  if (marker == "mi") { for (char& ch : value) if (static_cast<unsigned char>(ch) < 128) ch = static_cast<char>(std::toupper(static_cast<unsigned char>(ch))); }
  else if (marker == "me" && !value.empty() && static_cast<unsigned char>(value.front()) < 128) value.front() = static_cast<char>(std::toupper(static_cast<unsigned char>(value.front())));
  return value;
}
std::optional<std::string> unescape_text_token(const std::string& payload) {
  constexpr std::string_view prefix{"huhuhu"};
  if (!payload.starts_with(prefix) || (payload.size() - prefix.size()) % 6 != 0) return std::nullopt;
  std::string result;
  for (std::size_t i = prefix.size(); i < payload.size(); i += 6) {
    if (payload[i + 1] != 'a' || payload[i + 3] != 'a' || payload[i + 5] != 'a') return std::nullopt;
    const auto high = std::string_view{"mnlrptkbdgfshv"}.find(payload[i]);
    const auto middle = std::string_view{"mnlrptkbdgfshv"}.find(payload[i + 2]);
    const auto low = std::string_view{"mnlrptkbdgfshv"}.find(payload[i + 4]);
    if (high == std::string_view::npos || middle == std::string_view::npos || low == std::string_view::npos) return std::nullopt;
    result.push_back(static_cast<char>(high * kOnsets.size() * kOnsets.size() + middle * kOnsets.size() + low));
  }
  return result;
}

// The seed-language terms are deliberately small and reviewed.  Imported
// Wikidata labels have no machine-made German gloss: an untranslated label is
// surfaced as such to keep provenance and uncertainty visible.
const std::map<std::string, std::string>& german_terms() {
  static const std::map<std::string, std::string> terms{
      {"statement", "aussage"}, {"question", "frage"}, {"request", "bitte"},
      {"human", "mensch"}, {"machine", "maschine"}, {"child", "kind"}, {"researcher", "forscher"}, {"agent", "agent"},
      {"move", "bewegen"}, {"give", "geben"}, {"observe", "beobachten"}, {"build", "bauen"}, {"ask", "fragen"},
      {"stone", "stein"}, {"signal", "signal"}, {"map", "karte"}, {"water", "wasser"}, {"idea", "idee"},
      {"near", "nahe"}, {"before", "vor"}, {"because", "weil"}, {"if", "wenn"},
      {"certain", "sicher"}, {"possible", "möglich"}, {"required", "erforderlich"},
      {"unlikely", "unwahrscheinlich"}, {"likely", "wahrscheinlich"}, {"unknown", "unbekannt"},
      {"neutral", "neutral"}, {"joy", "freude"}, {"concern", "sorge"}, {"peer", "gleichrangig"}, {"ally", "verbündeter"}, {"observer", "beobachter"},
      {"inform", "informieren"}, {"assist", "helfen"}, {"coordinate", "koordinieren"}, {"direct", "direkt"}, {"reported", "berichtet"},
      {"literal", "wörtlich"}, {"metaphoric", "metaphorisch"}, {"forest", "wald"}, {"fire", "feuer"}, {"danger", "gefahr"},
      {"forest-fire", "waldbrand"}, {"rain", "regen"}, {"flood", "überschwemmung"}, {"tree", "baum"}, {"melody", "melodie"}};
  return terms;
}

std::string localize_label(const std::string& label, const std::string& language, bool& fallback) {
  if (language == "en") return label;
  const auto found = german_terms().find(canonical_concept_key(label));
  if (found != german_terms().end()) return found->second;
  fallback = true;
  return label;
}

std::optional<Meaning> analyse_meaning_sentence(const std::string& text) {
  const auto input = words(text);
  if (input.empty()) return std::nullopt;
  Meaning meaning{};
  bool actor{}, action{}, object{};
  const std::map<std::string, std::string> aliases{
      {"mensch", "human"}, {"human", "human"}, {"person", "human"},
      {"maschine", "machine"}, {"machine", "machine"}, {"kind", "child"}, {"child", "child"},
      {"forscher", "researcher"}, {"researcher", "researcher"}, {"agent", "agent"},
      {"bewegen", "move"}, {"bewegt", "move"}, {"move", "move"},
      {"geben", "give"}, {"gibt", "give"}, {"give", "give"},
      {"beobachten", "observe"}, {"beobachtet", "observe"}, {"observe", "observe"},
      {"bauen", "build"}, {"baut", "build"}, {"build", "build"},
      {"fragen", "ask"}, {"fragt", "ask"}, {"ask", "ask"},
      {"stein", "stone"}, {"stone", "stone"}, {"signal", "signal"},
      {"karte", "map"}, {"map", "map"}, {"wasser", "water"}, {"water", "water"},
      {"idee", "idea"}, {"idea", "idea"}};
  const std::map<std::string, std::string> relations{{"nahe", "near"}, {"near", "near"}, {"vor", "before"}, {"before", "before"}, {"weil", "because"}, {"because", "because"}, {"wenn", "if"}, {"if", "if"}};
  for (const auto& token : input) {
    if (const auto found = aliases.find(token); found != aliases.end()) {
      const std::string& value = found->second;
      if (value == "human" || value == "machine" || value == "child" || value == "researcher" || value == "agent") { meaning.actor = value; actor = true; }
      else if (value == "move" || value == "give" || value == "observe" || value == "build" || value == "ask") { meaning.action = value; action = true; }
      else { meaning.object = value; object = true; }
    }
    if (const auto found = relations.find(token); found != relations.end()) meaning.relation = found->second;
    if (token == "nicht" || token == "not") meaning.negated = true;
    if (token == "bitte" || token == "please") meaning.act = SpeechAct::request;
    if (token == "gestern" || token == "past") meaning.time = 1;
    if (token == "morgen" || token == "future") meaning.time = 2;
    if (token == "zwei" || token == "two") meaning.quantity = 2;
    if (token == "drei" || token == "three") meaning.quantity = 3;
    if (token == "möglich" || token == "possible") meaning.modality = "possible";
    if (token == "erforderlich" || token == "required") meaning.modality = "required";
    if (token == "unwahrscheinlich" || token == "unlikely") meaning.probability = "unlikely";
    if (token == "unbekannt" || token == "unknown") meaning.probability = "unknown";
    if (token == "freude" || token == "joy") meaning.emotion = "joy";
    if (token == "sorge" || token == "concern") meaning.emotion = "concern";
  }
  if (text.find('?') != std::string::npos) meaning.act = SpeechAct::question;
  if (!actor || !action || !object) return std::nullopt;
  return meaning;
}

std::string render_meaning(const Meaning& meaning, const std::string& language) {
  const bool german = language == "de";
  bool fallback{};
  const auto label = [&](const std::string& value) { return german ? localize_label(value, "de", fallback) : value; };
  const std::array<std::pair<int, const char*>, 3> times{{{0, german ? "Gegenwart" : "present"}, {1, german ? "Vergangenheit" : "past"}, {2, german ? "Zukunft" : "future"}}};
  const auto time = std::find_if(times.begin(), times.end(), [&meaning](const auto& item) { return item.first == meaning.time; });
  std::ostringstream out;
  out << (german ? "Bedeutung: " : "Meaning: ") << label(act_name(meaning.act)) << "; "
      << label(meaning.actor) << " / " << label(meaning.action) << " / " << label(meaning.object)
      << "\n" << (german ? "Relation: " : "Relation: ") << label(meaning.relation)
      << "; " << (german ? "Negation: " : "Negation: ") << (meaning.negated ? (german ? "ja" : "yes") : (german ? "nein" : "no"))
      << "; " << (german ? "Zeit: " : "Time: ") << (time == times.end() ? "?" : time->second)
      << "; " << (german ? "Menge: " : "Quantity: ") << meaning.quantity
      << "\n" << (german ? "Modalität: " : "Modality: ") << label(meaning.modality)
      << "; " << (german ? "Wahrscheinlichkeit: " : "Probability: ") << label(meaning.probability)
      << "; " << (german ? "Emotion: " : "Emotion: ") << label(meaning.emotion)
      << "; " << (german ? "Sozial: " : "Social: ") << label(meaning.social_relation)
      << "; " << (german ? "Ziel: " : "Goal: ") << label(meaning.goal)
      << "; " << (german ? "Aussageebene: " : "Proposition: ") << label(meaning.proposition_level)
      << "; " << (german ? "Abstraktion: " : "Abstraction: ") << label(meaning.abstraction);
  return out.str();
}

std::string stable_fingerprint(const std::filesystem::path& path) {
  std::ifstream in(path, std::ios::binary);
  if (!in) throw std::runtime_error("cannot read concept import: " + path.string());
  std::uint64_t hash = 1469598103934665603ULL;
  char buffer[8192];
  std::uint64_t length{};
  while (in.read(buffer, sizeof(buffer)) || in.gcount() > 0) {
    const auto count = in.gcount(); length += static_cast<std::uint64_t>(count);
    for (std::streamsize i = 0; i < count; ++i) { hash ^= static_cast<unsigned char>(buffer[i]); hash *= 1099511628211ULL; }
  }
  std::ostringstream out; out << std::hex << hash << '-' << std::dec << length;
  return out.str();
}

std::string generated_concept_id(std::size_t serial) {
  std::ostringstream out; out << "CONCEPT_" << std::setw(8) << std::setfill('0') << serial;
  return out.str();
}

std::vector<std::string> split_tsv(const std::string& line) {
  std::vector<std::string> fields;
  std::istringstream stream(line);
  for (std::string field; std::getline(stream, field, '\t');) fields.push_back(trim(std::move(field)));
  return fields;
}

bool validate_compositional_form(const ConceptEntry& entry,
                                 const std::map<std::string, std::string>& receiver) {
  // The receiver reconstructs labels from transmitted forms, not concept IDs.
  if (entry.derivation == "root") return receiver.contains(entry.form) && receiver.at(entry.form) == entry.label;
  std::istringstream parts(entry.form);
  std::string part;
  std::vector<std::string> recovered;
  while (std::getline(parts, part, '-')) {
    const auto known = receiver.find(part);
    if (known == receiver.end()) return false;
    recovered.push_back(known->second);
  }
  return join(recovered, "-") == entry.label;
}

bool externally_qualified(const ConceptEntry& entry) {
  // A row is qualified only when every externally auditable anchor is still
  // present.  In particular, an arbitrary generated stress label cannot pass
  // merely by being stored in the same graph.
  return entry.evidence_class == "external-attested" && !entry.external_id.empty() &&
         !entry.label.empty() && !entry.semantics.empty() && !entry.form.empty() &&
         !entry.source.empty() && !entry.provenance.empty() && !entry.source_revision.empty() &&
         entry.provenance.find("CC0") != std::string::npos;
}

Candidate make_candidate(std::uint64_t id, std::uint64_t parent, std::mt19937_64& rng) {
  Candidate candidate{};
  candidate.id = id;
  candidate.parent = parent;
  std::set<std::string> used;
  for (const auto& a : kActors) { const Meaning m{SpeechAct::statement, a, "move", "stone", "near", false, 0, 1}; candidate.lexicon.emplace("actor:" + std::string(a), new_word(rng, used)); }
  for (const auto& a : kActions) candidate.lexicon.emplace("action:" + std::string(a), new_word(rng, used));
  for (const auto& a : kObjects) candidate.lexicon.emplace("object:" + std::string(a), new_word(rng, used));
  for (const auto& a : kRelations) candidate.lexicon.emplace("relation:" + std::string(a), new_word(rng, used));
  for (const auto& a : {"statement", "question", "request"}) candidate.lexicon.emplace("act:" + std::string(a), new_word(rng, used));
  for (const auto& a : {"0", "1"}) candidate.lexicon.emplace("neg:" + std::string(a), new_word(rng, used));
  for (const auto& a : {"0", "1", "2"}) candidate.lexicon.emplace("time:" + std::string(a), new_word(rng, used));
  for (const auto& a : {"1", "2", "3"}) candidate.lexicon.emplace("qty:" + std::string(a), new_word(rng, used));
  for (const auto& a : kModalities) candidate.lexicon.emplace("modality:" + std::string(a), new_word(rng, used));
  for (const auto& a : kProbabilities) candidate.lexicon.emplace("probability:" + std::string(a), new_word(rng, used));
  for (const auto& a : kEmotions) candidate.lexicon.emplace("emotion:" + std::string(a), new_word(rng, used));
  for (const auto& a : kSocialRelations) candidate.lexicon.emplace("social:" + std::string(a), new_word(rng, used));
  for (const auto& a : kGoals) candidate.lexicon.emplace("goal:" + std::string(a), new_word(rng, used));
  for (const auto& a : kPropositionLevels) candidate.lexicon.emplace("proposition:" + std::string(a), new_word(rng, used));
  for (const auto& a : kAbstractions) candidate.lexicon.emplace("abstraction:" + std::string(a), new_word(rng, used));
  ensure_morphology(candidate, rng);
  candidate.order = default_order();
  return candidate;
}
Candidate mutate(const Candidate& source, std::uint64_t id, std::mt19937_64& rng) {
  Candidate out = source; out.id = id; out.parent = source.id; out.metrics = {};
  if (out.order.empty()) out.order = default_order();
  std::bernoulli_distribution mutate_grammar(0.35);
  if (mutate_grammar(rng)) {
    std::uniform_int_distribution<std::size_t> select(0, out.order.size() - 1);
    const auto first = select(rng); auto second = select(rng);
    while (second == first) second = select(rng);
    std::swap(out.order[first], out.order[second]);
    return out;
  }
  ensure_morphology(out, rng);
  std::set<std::string> used; for (const auto& [_, word] : out.lexicon) used.insert(word); for (const auto& [_, word] : out.morphology) used.insert(word);
  std::bernoulli_distribution mutate_morphology(0.20);
  if (mutate_morphology(rng)) {
    std::uniform_int_distribution<std::size_t> select(0, out.morphology.size() - 1);
    auto it = out.morphology.begin(); std::advance(it, static_cast<long>(select(rng))); used.erase(it->second); it->second = new_word(rng, used);
  } else {
    std::uniform_int_distribution<std::size_t> select(0, out.lexicon.size() - 1);
    auto it = out.lexicon.begin(); std::advance(it, static_cast<long>(select(rng))); used.erase(it->second); it->second = new_word(rng, used);
  }
  return out;
}
std::string encode(const Candidate& c, const Meaning& m) {
  std::ostringstream out;
  for (const auto& atom : ordered_atoms(c, m)) { if (out.tellp() > 0) out << ' '; out << c.lexicon.at(atom); }
  return out.str();
}
bool decode(const std::map<std::string, std::string>& private_knowledge, const std::string& message, Meaning& out) {
  std::istringstream in(message); std::string word; std::map<std::string, std::string> values;
  while (in >> word) { const auto it = private_knowledge.find(word); if (it == private_knowledge.end()) return false; const auto pos = it->second.find(':'); values[it->second.substr(0, pos)] = it->second.substr(pos + 1); }
  if (values.size() != 15) return false;
  out = {parse_act(values["act"]), values["actor"], values["action"], values["object"], values["relation"], values["neg"] == "1", std::stoi(values["time"]), std::stoi(values["qty"]), values["modality"], values["probability"], values["emotion"], values["social"], values["goal"], values["proposition"], values["abstraction"]};
  return true;
}

std::string productive_root_form(const Candidate& language, const std::string& root) {
  // Core meanings preserve their evolved lexical roots. Additional verbs are
  // regular, reproducible derivations: learning the base form is enough for
  // a receiver to recognize all of its marked forms.
  const std::map<std::string, std::string> core{{"gehen", "action:move"}, {"bauen", "action:build"}, {"fragen", "action:ask"}};
  if (const auto found = core.find(root); found != core.end()) return language.lexicon.at(found->second);
  std::uint64_t hash = 1469598103934665603ULL;
  const std::string key = language.lexicon.at("action:move") + ":" + root;
  for (const unsigned char ch : key) { hash ^= ch; hash *= 1099511628211ULL; }
  std::string form;
  for (int i = 0; i < 4; ++i) {
    form += kOnsets[hash % kOnsets.size()]; hash /= kOnsets.size();
    form += kVowels[hash % kVowels.size()]; hash /= kVowels.size();
  }
  return form;
}

std::string productive_grammar_test(const Candidate& language) {
  // Receiver training deliberately excludes the held-out `lernen;pl;past`.
  // It is given only the base form of lernen, markers from other verbs, and
  // must recover the unseen combination without an answer-side lookup.
  const std::map<std::string, std::string> roots{{"gehen", productive_root_form(language, "gehen")}, {"bauen", productive_root_form(language, "bauen")}, {"fragen", productive_root_form(language, "fragen")}, {"lernen", productive_root_form(language, "lernen")}};
  const auto& markers = language.morphology;
  const auto encode = [&roots, &markers](const std::string& root, const std::string& number, const std::string& time) { return roots.at(root) + "-" + markers.at("person:3") + markers.at("number:" + number) + markers.at("time:" + time); };
  const std::string seen_singular = encode("gehen", "sg", "present");
  const std::string held_out_plural = encode("lernen", "pl", "past");
  struct MorphologyReceiver {
    std::map<std::string, std::string> roots;
    std::map<std::string, std::string> markers;
  } receiver;
  // Feedback from separate, already observed utterances.  The receiver never
  // gets the lernen/pl/past combination or the sender's maps themselves.
  for (const auto& root : {"gehen", "bauen", "fragen", "lernen"}) receiver.roots.emplace(roots.at(root), root);
  receiver.markers.emplace(markers.at("person:3"), "person:3");
  receiver.markers.emplace(markers.at("number:sg"), "number:sg");
  receiver.markers.emplace(markers.at("number:pl"), "number:pl");
  receiver.markers.emplace(markers.at("time:present"), "time:present");
  receiver.markers.emplace(markers.at("time:past"), "time:past");
  const auto decode = [&receiver](const std::string& form) {
    const auto split = form.find('-'); if (split == std::string::npos) return std::string{};
    const auto root_known = receiver.roots.find(form.substr(0, split));
    if (root_known == receiver.roots.end()) return std::string{};
    const std::string& root = root_known->second;
    const std::string suffix = form.substr(split + 1);
    for (const auto& [person_surface, person] : receiver.markers) for (const auto& [number_surface, number] : receiver.markers) for (const auto& [time_surface, time] : receiver.markers)
      if (person == "person:3" && number.starts_with("number:") && time.starts_with("time:") && suffix == person_surface + number_surface + time_surface)
        return root + ";" + number.substr(7) + ";" + time.substr(5);
    return std::string{};
  };
  const std::map<std::string, std::string> german_family{{"gehen", "gehen;infinitive"}, {"geht", "gehen;3;sg;present"}, {"ging", "gehen;3;sg;past"}, {"gingen", "gehen;3;pl;past"}, {"gegangen", "gehen;participle;past"}, {"lernen", "lernen;infinitive"}, {"lernt", "lernen;3;sg;present"}, {"lernten", "lernen;3;pl;past"}};
  const bool family_ok = german_family.at("geht").starts_with("gehen;") && german_family.at("ging").starts_with("gehen;") && german_family.at("gegangen").starts_with("gehen;");
  const bool held_out_ok = decode(held_out_plural) == "lernen;pl;past";
  std::ostringstream out;
  out << "Productive morphology lab\nGerman family analysis: gehen / geht / ging / gegangen -> root=gehen\nfamily recognition: " << (family_ok ? "passed" : "failed") << "\nroot gehen: " << roots.at("gehen") << "\nseen singular present: " << seen_singular << " = gehen;sg;present\nnew verb base supplied to receiver: lernen = " << roots.at("lernen") << "\nheld-out new-verb plural past: " << held_out_plural << "\nreceiver training: base forms plus markers from other utterances; held-out combination absent\nindependent receiver reconstruction: " << decode(held_out_plural) << "\nregular-marker validation: " << (held_out_ok ? "passed" : "failed") << "\nmarkers: person-3=" << markers.at("person:3") << ", singular=" << markers.at("number:sg") << ", plural=" << markers.at("number:pl") << ", present=" << markers.at("time:present") << ", past=" << markers.at("time:past");
  return out.str();
}

struct GermanVerbFeatures { std::string root; std::string number; std::string time; };
std::optional<GermanVerbFeatures> analyse_german_verb(const std::string& token, const std::string& previous = {}) {
  static const std::map<std::string, GermanVerbFeatures> forms{
      {"gehen", {"gehen", "", "present"}}, {"geht", {"gehen", "sg", "present"}}, {"ging", {"gehen", "sg", "past"}}, {"gingen", {"gehen", "pl", "past"}}, {"gegangen", {"gehen", "sg", "past"}},
      {"bauen", {"bauen", "", "present"}}, {"baut", {"bauen", "sg", "present"}}, {"baute", {"bauen", "sg", "past"}}, {"bauten", {"bauen", "pl", "past"}}, {"gebaut", {"bauen", "sg", "past"}},
      {"fragen", {"fragen", "", "present"}}, {"fragt", {"fragen", "sg", "present"}}, {"fragte", {"fragen", "sg", "past"}}, {"fragten", {"fragen", "pl", "past"}}, {"gefragt", {"fragen", "sg", "past"}}};
  const std::string word = canonical_concept_key(token);
  auto found = forms.find(word);
  GermanVerbFeatures result;
  if (found != forms.end()) result = found->second;
  else if (word.size() > 4 && word.ends_with("ten")) result = {word.substr(0, word.size() - 3) + "en", "pl", "past"};
  else if (word.size() > 3 && word.ends_with("te")) result = {word.substr(0, word.size() - 2) + "en", "sg", "past"};
  else if (word.size() > 4 && word.starts_with("ge") && word.ends_with("t")) result = {word.substr(2, word.size() - 3) + "en", "sg", "past"};
  else if (word.size() > 3 && word.ends_with("en")) result = {word, "", "present"};
  else if (word.size() > 3 && word.ends_with("t")) result = {word.substr(0, word.size() - 1) + "en", "sg", "present"};
  else return std::nullopt;
  if (result.number.empty() && (canonical_concept_key(previous) == "sie" || canonical_concept_key(previous) == "wir")) result.number = "pl";
  return result;
}

std::string productive_verb_form(const Candidate& language, const GermanVerbFeatures& features) {
  const std::string root = productive_root_form(language, features.root);
  if (features.number.empty()) return root;
  return root + "-" + language.morphology.at("person:3") + language.morphology.at("number:" + features.number) + language.morphology.at("time:" + features.time);
}
void train_population(const Candidate& language, std::vector<AgentState>& agents, std::mt19937_64& rng, int rounds) {
  if (agents.empty()) return;
  for (int step = 0; step < rounds; ++step) {
    const Meaning expected = random_meaning(rng, false);
    const std::string message = encode(language, expected);
    const auto semantic_atoms = ordered_atoms(language, expected);
    std::istringstream tokens(message);
    std::vector<std::string> words;
    for (std::string word; tokens >> word;) words.push_back(std::move(word));
    for (auto& agent : agents) {
      Meaning decoded;
      if (decode(agent.private_reverse_lexicon, message, decoded) && decoded == expected) ++agent.successful_interpretations;
      else ++agent.failed_interpretations;
      // The feedback is delivered only after the receiver has made its attempt.
      for (std::size_t index = 0; index < words.size(); ++index) agent.private_reverse_lexicon[words[index]] = semantic_atoms.at(index);
    }
  }
}
Metrics evaluate(const Candidate& candidate, std::mt19937_64& rng, int train, int evaluation) {
  // Each receiver owns a reverse lexicon. No decoder receives the expected Meaning.
  std::map<std::string, std::string> receiver;
  for (int i = 0; i < train; ++i) {
    const Meaning m = random_meaning(rng, false); const auto message = encode(candidate, m);
    std::istringstream tokens(message); std::string word; const auto a = atoms(m); std::size_t index = 0;
    while (tokens >> word) receiver[word] = a.at(index++); // explicit feedback after a completed training trial
  }
  int exact = 0; int recovered_fields = 0; int all_fields = 0; int robust_fields = 0; int robust_total_fields = 0; std::size_t bytes = 0;
  for (int i = 0; i < evaluation; ++i) {
    const Meaning expected = random_meaning(rng, true); const std::string message = encode(candidate, expected); bytes += message.size();
    Meaning decoded; const bool valid = decode(receiver, message, decoded); if (valid && decoded == expected) ++exact;
    const auto target = atoms(expected); const auto result = valid ? atoms(decoded) : std::vector<std::string>{};
    for (std::size_t p = 0; p < target.size(); ++p) { ++all_fields; if (p < result.size() && target[p] == result[p]) ++recovered_fields; }
    // Controlled transmission error: substitute exactly one actor token with a
    // different valid actor word. This tests semantic degradation rather than
    // treating a malformed byte sequence as an uninformative decoder failure.
    std::istringstream message_stream(message);
    std::vector<std::string> noisy_words;
    for (std::string word; message_stream >> word;) noisy_words.push_back(std::move(word));
    noisy_words.at(1) = candidate.lexicon.at("actor:" + std::string(expected.actor == "human" ? "machine" : "human"));
    std::ostringstream noisy_message;
    for (std::size_t p = 0; p < noisy_words.size(); ++p) { if (p > 0) noisy_message << ' '; noisy_message << noisy_words[p]; }
    Meaning noisy_decoded;
    const bool noisy_valid = decode(receiver, noisy_message.str(), noisy_decoded);
    const auto noisy_result = noisy_valid ? atoms(noisy_decoded) : std::vector<std::string>{};
    for (std::size_t p = 0; p < target.size(); ++p) { ++robust_total_fields; if (p < noisy_result.size() && target[p] == noisy_result[p]) ++robust_fields; }
  }
  // Morphological markers participate in selection just like roots: compact,
  // pronounceable reusable markers lower the cost of unseen combinations.
  double pronounceability = 0.0; for (const auto& [_, word] : candidate.lexicon) pronounceability += word_quality(word);
  std::size_t morphology_bytes{};
  for (const auto& [_, word] : candidate.morphology) { pronounceability += word_quality(word); morphology_bytes += word.size(); }
  pronounceability /= static_cast<double>(candidate.lexicon.size() + candidate.morphology.size());
  Metrics m; m.communication_success = static_cast<double>(exact) / evaluation; m.semantic_accuracy = static_cast<double>(recovered_fields) / all_fields;
  // One representative unseen verb combination charges the reusable marker
  // payload once per evaluation batch, instead of treating morphology as free.
  m.cost = static_cast<double>(bytes + morphology_bytes) / evaluation; m.pronounceability = pronounceability;
  const auto& order = candidate.order.empty() ? default_order() : candidate.order;
  int inversions{};
  for (std::size_t i = 0; i < order.size(); ++i) for (std::size_t j = i + 1; j < order.size(); ++j) {
    const auto left = std::find(default_order().begin(), default_order().end(), order[i]) - default_order().begin();
    const auto right = std::find(default_order().begin(), default_order().end(), order[j]) - default_order().begin();
    if (left > right) ++inversions;
  }
  const double max_inversions = static_cast<double>(order.size() * (order.size() - 1) / 2);
  m.grammar_complexity = 1.0 + static_cast<double>(inversions) / max_inversions;
  m.robustness = static_cast<double>(robust_fields) / robust_total_fields; m.expressiveness = 1.0; return m;
}
void rank_candidates(std::vector<Candidate>& candidates) {
  std::vector<double> benefit;
  std::vector<double> cost;
  benefit.reserve(candidates.size());
  cost.reserve(candidates.size());
  for (const auto& candidate : candidates) {
    const auto& m = candidate.metrics;
    benefit.push_back(m.communication_success * 45.0 + m.semantic_accuracy * 25.0 + m.pronounceability * 15.0 + m.robustness * 10.0 + m.expressiveness * 5.0 - m.grammar_complexity * 4.0);
    cost.push_back(m.cost);
  }
  const auto batch = compute_reward_batch(benefit, cost, 0.08);
  for (std::size_t i = 0; i < candidates.size(); ++i) candidates[i].selection_score = batch.values[i];
}
bool dominates(const Metrics& a, const Metrics& b) {
  const std::array<double, 7> av{a.communication_success, a.semantic_accuracy, -a.cost, a.pronounceability, -a.grammar_complexity, a.robustness, a.expressiveness};
  const std::array<double, 7> bv{b.communication_success, b.semantic_accuracy, -b.cost, b.pronounceability, -b.grammar_complexity, b.robustness, b.expressiveness};
  bool strict = false; for (std::size_t i = 0; i < av.size(); ++i) { if (av[i] < bv[i]) return false; strict = strict || av[i] > bv[i]; } return strict;
}
CandidateSummary summary(const Candidate& c) { return {c.id, c.parent, c.metrics, encode(c, Meaning{SpeechAct::statement, "human", "observe", "idea", "because", false, 1, 1})}; }
std::string canonical_answer(const std::string& input) {
  std::string out;
  bool previous_space = true;
  for (const unsigned char character : input) {
    if (std::isspace(character)) { if (!previous_space) out += ' '; previous_space = true; }
    else { out += static_cast<char>(std::tolower(character)); previous_space = false; }
  }
  if (!out.empty() && out.back() == ' ') out.pop_back();
  return out;
}
std::string grammar_description(const Candidate& candidate) {
  const auto& order = candidate.order.empty() ? default_order() : candidate.order;
  std::ostringstream out;
  for (const auto& atom : order) {
    if (out.tellp() > 0) out << ' ';
    const auto separator = atom.find(':');
    out << atom.substr(0, separator);
  }
  return out.str();
}
std::uint64_t unix_millis() { return static_cast<std::uint64_t>(std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count()); }
}  // namespace

std::string Meaning::key() const { std::ostringstream out; for (const auto& a : atoms(*this)) out << a << '|'; return out.str(); }
double Metrics::score() const { return communication_success * 45 + semantic_accuracy * 25 + pronounceability * 15 + robustness * 10 + expressiveness * 5 - grammar_complexity * 4 - cost * 0.08; }

struct ResearchEngine::Impl {
  RunConfig config; std::mt19937_64 rng; int generation{}; std::uint64_t next_id{1}; std::vector<Candidate> frontier; std::vector<CandidateSummary> history; std::vector<AgentState> agents; std::vector<ConceptEntry> concepts;
  // V12 keeps insertion order for reproducible exports while all lookups use
  // these indexes.  This removes the former O(n²) import behaviour.
  std::unordered_map<std::string, std::size_t> by_identity;
  std::unordered_map<std::string, std::size_t> by_label;
  std::unordered_set<std::string> used_forms;
  std::size_t next_concept_serial{1};
  ImportCheckpoint import_checkpoint;
  ImportCheckpoint german_word_list_checkpoint;
  std::uint64_t human_attempts{}; std::uint64_t human_correct{}; std::uint64_t human_total_millis{}; std::uint64_t human_production_attempts{}; std::uint64_t human_production_correct{}; std::uint64_t human_interpretation_attempts{}; std::uint64_t human_interpretation_correct{};
  explicit Impl(RunConfig c) : config(c), rng(c.seed) { if (c.population < 2 || c.generations < 1) throw std::invalid_argument("population must be >= 2 and generations >= 1"); }
  void ensure_concept_lexicon();
  void rebuild_concept_indexes();
  ConceptEntry* find_concept(const std::string& identity_or_label);
  void add_concept(ConceptEntry entry);
};

void ResearchEngine::Impl::rebuild_concept_indexes() {
  by_identity.clear(); by_label.clear(); used_forms.clear(); next_concept_serial = 1;
  for (std::size_t i = 0; i < concepts.size(); ++i) {
    const auto& entry = concepts[i];
    by_identity.emplace(entry.external_id.empty() ? entry.id : entry.external_id, i);
    by_identity.emplace(entry.id, i);
    by_label.emplace(canonical_concept_key(entry.label), i);
    used_forms.emplace(entry.form);
    const auto digits = entry.id.find_last_not_of("0123456789");
    if (digits != std::string::npos && digits + 1 < entry.id.size()) {
      try { next_concept_serial = std::max(next_concept_serial, static_cast<std::size_t>(std::stoull(entry.id.substr(digits + 1)) + 1)); } catch (...) { }
    }
  }
  if (!frontier.empty()) for (const auto& [_, form] : frontier.front().lexicon) used_forms.emplace(form);
}
ConceptEntry* ResearchEngine::Impl::find_concept(const std::string& identity_or_label) {
  const auto identity = by_identity.find(identity_or_label);
  if (identity != by_identity.end()) return &concepts[identity->second];
  const auto label = by_label.find(canonical_concept_key(identity_or_label));
  return label == by_label.end() ? nullptr : &concepts[label->second];
}
void ResearchEngine::Impl::add_concept(ConceptEntry entry) {
  const auto position = concepts.size();
  if (entry.id.empty()) entry.id = generated_concept_id(next_concept_serial++);
  else {
    const auto digits = entry.id.find_last_not_of("0123456789");
    if (digits != std::string::npos && digits + 1 < entry.id.size()) {
      try { next_concept_serial = std::max(next_concept_serial, static_cast<std::size_t>(std::stoull(entry.id.substr(digits + 1)) + 1)); } catch (...) { }
    }
  }
  if (entry.language_status.empty()) entry.language_status = "pending";
  if (entry.evidence_class.empty()) entry.evidence_class = "agent-hypothesis";
  used_forms.emplace(entry.form);
  by_identity.emplace(entry.id, position);
  if (!entry.external_id.empty()) by_identity.emplace(entry.external_id, position);
  by_label.emplace(canonical_concept_key(entry.label), position);
  concepts.push_back(std::move(entry));
}

void ResearchEngine::Impl::ensure_concept_lexicon() {
  if (!concepts.empty()) return;
  if (!config.seed_concept_lexicon) return;
  rebuild_concept_indexes();
  std::set<std::string> used;
  for (const auto& [_, word] : frontier.front().lexicon) used.insert(word);
  const auto root = [this, &used](const char* id, const char* label, const char* part, const char* semantics, const char* relations) {
    ConceptEntry entry{}; entry.id = id; entry.label = label; entry.part_of_speech = part;
    entry.form = new_word(rng, used);
    entry.derivation = "root"; entry.source = "seeded-human-knowledge"; entry.provenance = "local-curated-starter-set";
    entry.source_revision = "local-v1"; entry.evidence_class = "agent-hypothesis"; entry.language_status = "realised";
    entry.semantics = semantics; entry.relations = relations; entry.introduced_generation = generation;
    add_concept(std::move(entry));
  };
  // A compact, inspectable initial knowledge basis. It is local curated seed
  // data, not a copy of a third-party dictionary; future imports retain source.
  root("CONCEPT_00001", "tree", "noun", "living;plant;woody", "part-of:forest;produces:wood,fruit;casts:shadow");
  root("CONCEPT_00002", "forest", "noun", "place;many-trees", "contains:tree;can-contain:fire");
  root("CONCEPT_00003", "fire", "noun", "event;heat;combustion", "can-threaten:forest;causes:danger");
  root("CONCEPT_00004", "danger", "noun", "abstract;risk", "can-describe:fire");
  root("CONCEPT_00005", "wood", "noun", "material;plant-derived", "derived-from:tree");
  root("CONCEPT_00006", "shadow", "noun", "phenomenon;light-blocked", "caused-by:tree");
  root("CONCEPT_00007", "fruit", "noun", "plant-product;edible", "produced-by:tree");
  const auto form_for = [this](const char* label) -> std::string { const auto* entry = find_concept(label); if (entry) return entry->form; throw std::logic_error("missing concept root"); };
  const auto compound = [this, &form_for](const char* id, const char* label, const char* semantics, const char* relations, const std::vector<std::string>& family) {
    ConceptEntry entry{}; entry.id = id; entry.label = label; entry.part_of_speech = "noun";
    entry.derivation = "compound"; entry.source = "simulated-experience"; entry.provenance = "derived-from-persisted-concept-relations";
    entry.source_revision = "local-v1"; entry.evidence_class = "simulated-relation"; entry.language_status = "realised";
    entry.semantics = semantics; entry.relations = relations; entry.introduced_generation = generation;
    std::vector<std::string> forms;
    for (const auto& member : family) forms.push_back(form_for(member.c_str()));
    entry.form = join(forms, "-");
    add_concept(std::move(entry));
  };
  // These are discovered in the simulated world through relations above, and
  // preserve their morphological family instead of receiving random tokens.
  compound("CONCEPT_00008", "forest-fire", "event;forest;combustion", "is-a:fire;occurs-in:forest", {"forest", "fire"});
  compound("CONCEPT_00009", "forest-fire-danger", "abstract;risk;forest-fire", "describes:forest-fire", {"forest", "fire", "danger"});

  std::map<std::string, std::string> receiver;
  for (const auto& entry : concepts) if (entry.derivation == "root") receiver.emplace(entry.form, entry.label);
  for (auto& entry : concepts) {
    // Multiple receivers must reconstruct a message from learned forms. No
    // comparison of opaque CONCEPT_* identifiers participates in this check.
    entry.validation_attempts = 6;
    for (int trial = 0; trial < 6; ++trial) if (validate_compositional_form(entry, receiver)) ++entry.validation_successes;
    entry.validation_failures = entry.validation_attempts - entry.validation_successes;
    entry.language_status = entry.validation_successes == entry.validation_attempts ? "communicatively-validated" : "realised";
  }
}

ResearchEngine::ResearchEngine(RunConfig config) : impl_(new Impl(config)) {}
ResearchEngine::~ResearchEngine() { delete impl_; }

void ResearchEngine::run() {
  if (impl_->frontier.empty()) {
    for (int i = 0; i < impl_->config.population; ++i) impl_->frontier.push_back(make_candidate(impl_->next_id++, 0, impl_->rng));
    for (int i = 0; i < impl_->config.population; ++i) impl_->agents.push_back(AgentState{static_cast<std::uint64_t>(i + 1), {}, 0, 0});
  }
  if (impl_->agents.empty()) for (int i = 0; i < impl_->config.population; ++i) impl_->agents.push_back(AgentState{static_cast<std::uint64_t>(i + 1), {}, 0, 0});
  for (int g = 0; g < impl_->config.generations; ++g) {
    for (auto& c : impl_->frontier) c.metrics = evaluate(c, impl_->rng, impl_->config.training_rounds, impl_->config.evaluation_rounds);
    rank_candidates(impl_->frontier);
    std::sort(impl_->frontier.begin(), impl_->frontier.end(), [](const Candidate& a, const Candidate& b) { return a.selection_score > b.selection_score; });
    const int elites = std::max(2, impl_->config.population / 3);
    for (const auto& candidate : impl_->frontier) {
      bool is_dominated = false;
      for (const auto& rival : impl_->frontier) {
        if (rival.id != candidate.id && dominates(rival.metrics, candidate.metrics)) { is_dominated = true; break; }
      }
      if (!is_dominated) impl_->history.push_back(summary(candidate));
    }
    std::vector<Candidate> next; for (int i = 0; i < elites; ++i) next.push_back(impl_->frontier.at(i));
    std::uniform_int_distribution<int> pick(0, elites - 1); while (static_cast<int>(next.size()) < impl_->config.population) next.push_back(mutate(impl_->frontier.at(pick(impl_->rng)), impl_->next_id++, impl_->rng));
    impl_->frontier = std::move(next); ++impl_->generation;
  }
  for (auto& c : impl_->frontier) c.metrics = evaluate(c, impl_->rng, impl_->config.training_rounds, impl_->config.evaluation_rounds);
  rank_candidates(impl_->frontier);
  std::sort(impl_->frontier.begin(), impl_->frontier.end(), [](const Candidate& a, const Candidate& b) { return a.selection_score > b.selection_score; });
  train_population(impl_->frontier.front(), impl_->agents, impl_->rng, impl_->config.training_rounds * 3);
  impl_->history.push_back(summary(impl_->frontier.front()));
  impl_->ensure_concept_lexicon();
}

void ResearchEngine::save(const std::filesystem::path& path) const {
  std::ofstream out(path); if (!out) throw std::runtime_error("cannot write state: " + path.string());
  out << "LENG_STATE 15\n" << impl_->config.seed << ' ' << impl_->config.population << ' ' << impl_->config.generations << ' ' << impl_->config.training_rounds << ' ' << impl_->config.evaluation_rounds << ' ' << impl_->config.seed_concept_lexicon << '\n';
  out << impl_->generation << ' ' << impl_->next_id << ' ' << impl_->frontier.size() << '\n';
  std::ostringstream rng_state; rng_state << impl_->rng;
  out << "R " << std::quoted(rng_state.str()) << '\n';
  for (const auto& c : impl_->frontier) { out << "C " << c.id << ' ' << c.parent << ' ' << std::setprecision(17) << c.metrics.communication_success << ' ' << c.metrics.semantic_accuracy << ' ' << c.metrics.cost << ' ' << c.metrics.pronounceability << ' ' << c.metrics.grammar_complexity << ' ' << c.metrics.robustness << ' ' << c.metrics.expressiveness << ' ' << c.lexicon.size() << '\n'; for (const auto& [atom, word] : c.lexicon) out << "L " << std::quoted(atom) << ' ' << std::quoted(word) << '\n'; const auto& order = c.order.empty() ? default_order() : c.order; out << "O " << order.size(); for (const auto& atom : order) out << ' ' << std::quoted(atom); out << '\n'; out << "M " << c.morphology.size(); for (const auto& [meaning, form] : c.morphology) out << ' ' << std::quoted(meaning) << ' ' << std::quoted(form); out << '\n'; }
  out << "A " << impl_->agents.size() << '\n';
  for (const auto& agent : impl_->agents) { out << "P " << agent.id << ' ' << agent.successful_interpretations << ' ' << agent.failed_interpretations << ' ' << agent.private_reverse_lexicon.size() << '\n'; for (const auto& [word, atom] : agent.private_reverse_lexicon) out << "K " << std::quoted(word) << ' ' << std::quoted(atom) << '\n'; }
  out << "U " << impl_->human_attempts << ' ' << impl_->human_correct << ' ' << impl_->human_total_millis << ' ' << impl_->human_production_attempts << ' ' << impl_->human_production_correct << ' ' << impl_->human_interpretation_attempts << ' ' << impl_->human_interpretation_correct << '\n';
  out << "H " << impl_->history.size() << '\n'; for (const auto& h : impl_->history) out << h.id << ' ' << h.parent_id << ' ' << h.metrics.communication_success << ' ' << h.metrics.semantic_accuracy << ' ' << h.metrics.cost << ' ' << h.metrics.pronounceability << ' ' << h.metrics.grammar_complexity << ' ' << h.metrics.robustness << ' ' << h.metrics.expressiveness << ' ' << std::quoted(h.sample) << '\n';
  out << "X " << impl_->concepts.size() << '\n';
  for (const auto& entry : impl_->concepts) out << "Q " << std::quoted(entry.id) << ' ' << std::quoted(entry.external_id) << ' ' << std::quoted(entry.label) << ' ' << std::quoted(entry.part_of_speech) << ' ' << std::quoted(entry.form) << ' ' << std::quoted(entry.derivation) << ' ' << std::quoted(entry.source) << ' ' << std::quoted(entry.provenance) << ' ' << std::quoted(entry.source_revision) << ' ' << std::quoted(entry.evidence_class) << ' ' << std::quoted(entry.language_status) << ' ' << std::quoted(entry.semantics) << ' ' << std::quoted(entry.relations) << ' ' << entry.introduced_generation << ' ' << entry.validation_attempts << ' ' << entry.validation_successes << ' ' << entry.validation_failures << '\n';
  const auto& checkpoint = impl_->import_checkpoint;
  out << "I " << std::quoted(checkpoint.source_path) << ' ' << std::quoted(checkpoint.fingerprint) << ' ' << checkpoint.next_line << ' ' << checkpoint.rows_seen << ' ' << checkpoint.rows_accepted << ' ' << checkpoint.rows_rejected << ' ' << checkpoint.complete << ' ' << std::quoted(checkpoint.status) << '\n';
  const auto& word_checkpoint = impl_->german_word_list_checkpoint;
  out << "W " << std::quoted(word_checkpoint.source_path) << ' ' << std::quoted(word_checkpoint.fingerprint) << ' ' << word_checkpoint.next_line << ' ' << word_checkpoint.rows_seen << ' ' << word_checkpoint.rows_accepted << ' ' << word_checkpoint.rows_rejected << ' ' << word_checkpoint.complete << ' ' << std::quoted(word_checkpoint.status) << '\n';
}
void ResearchEngine::export_results(const std::filesystem::path& directory) const {
  std::filesystem::create_directories(directory);
  const auto write_text = [&directory](const char* name, const std::string& content) { std::ofstream out(directory / name); if (!out) throw std::runtime_error(std::string("cannot export ") + name); out << content; };
  write_text("language.txt", export_language());
  write_text("report.txt", report());
  write_text("communication.txt", communication_trace());
  write_text("population.txt", population_trace());
  write_text("evolutionary_lexicon.txt", lexicon_trace());
  std::ofstream archive(directory / "pareto_archive.csv");
  if (!archive) throw std::runtime_error("cannot export pareto archive");
  archive << "candidate_id,parent_id,communication_success,semantic_accuracy,bytes,pronounceability,grammar_complexity,robustness,expressiveness,sample\n";
  for (const auto& item : impl_->history) archive << item.id << ',' << item.parent_id << ',' << item.metrics.communication_success << ',' << item.metrics.semantic_accuracy << ',' << item.metrics.cost << ',' << item.metrics.pronounceability << ',' << item.metrics.grammar_complexity << ',' << item.metrics.robustness << ',' << item.metrics.expressiveness << ',' << '"' << item.sample << "\"\n";
  std::ofstream manifest(directory / "manifest.txt");
  if (!manifest) throw std::runtime_error("cannot export manifest");
  manifest << "LINGUISTISCHE EMERGENZ NG export\nstate_format=LENG_STATE 15\ngeneration=" << impl_->generation << "\ncandidates=" << impl_->frontier.size() << "\nagents=" << impl_->agents.size() << "\nconcepts=" << impl_->concepts.size() << "\n";
}

void ResearchEngine::load(const std::filesystem::path& path) {
  std::ifstream in(path); std::string magic; int version{}; if (!(in >> magic >> version) || magic != "LENG_STATE" || version < 1 || version > 15) throw std::runtime_error("unsupported or corrupt state");
  RunConfig stored{};
  if (!(in >> stored.seed >> stored.population >> stored.generations >> stored.training_rounds >> stored.evaluation_rounds)) throw std::runtime_error("corrupt configuration");
  if (version >= 15 && !(in >> stored.seed_concept_lexicon)) throw std::runtime_error("corrupt configuration");
  // The caller controls the number of *additional* generations on resume; all
  // other reproducibility parameters come from the saved experiment.
  const int requested_generations = impl_->config.generations;
  impl_->config = stored;
  impl_->config.generations = requested_generations;
  std::size_t candidates{}; if (!(in >> impl_->generation >> impl_->next_id >> candidates)) throw std::runtime_error("corrupt header"); impl_->frontier.clear(); impl_->history.clear(); impl_->agents.clear(); impl_->concepts.clear(); impl_->rebuild_concept_indexes(); impl_->import_checkpoint = {}; impl_->german_word_list_checkpoint = {}; impl_->human_attempts = 0; impl_->human_correct = 0; impl_->human_total_millis = 0; impl_->human_production_attempts = 0; impl_->human_production_correct = 0; impl_->human_interpretation_attempts = 0; impl_->human_interpretation_correct = 0; impl_->rng.seed(impl_->config.seed);
  if (version == 4) { std::string rng_line; std::getline(in >> std::ws, rng_line); if (!rng_line.starts_with("R ")) throw std::runtime_error("corrupt RNG state"); std::istringstream rng_input(rng_line.substr(2)); if (!(rng_input >> impl_->rng)) throw std::runtime_error("corrupt RNG state"); }
  if (version >= 5) { char rng_marker{}; std::string rng_text; if (!(in >> rng_marker >> std::quoted(rng_text)) || rng_marker != 'R') throw std::runtime_error("corrupt RNG state"); std::istringstream rng_input(rng_text); if (!(rng_input >> impl_->rng)) throw std::runtime_error("corrupt RNG state"); }
  for (std::size_t n = 0; n < candidates; ++n) { Candidate c; char kind{}; std::size_t count{}; if (!(in >> kind >> c.id >> c.parent >> c.metrics.communication_success >> c.metrics.semantic_accuracy >> c.metrics.cost >> c.metrics.pronounceability >> c.metrics.grammar_complexity >> c.metrics.robustness >> c.metrics.expressiveness >> count) || kind != 'C') throw std::runtime_error("corrupt candidate"); for (std::size_t i = 0; i < count; ++i) { std::string atom, word; if (!(in >> kind >> std::quoted(atom) >> std::quoted(word)) || kind != 'L') throw std::runtime_error("corrupt lexicon"); c.lexicon.emplace(std::move(atom), std::move(word)); } if (version >= 7) { if (!(in >> kind >> count) || kind != 'O') throw std::runtime_error("corrupt grammar order"); std::set<std::string> fields; for (std::size_t i = 0; i < count; ++i) { std::string field; if (!(in >> std::quoted(field)) || !fields.insert(field).second) throw std::runtime_error("corrupt grammar field"); if (version >= 9) c.order.push_back(std::move(field)); } if (version >= 9) { if (count != default_order().size()) throw std::runtime_error("corrupt grammar order"); for (const auto& field : default_order()) if (!fields.contains(field)) throw std::runtime_error("incomplete grammar order"); } else c.order = default_order(); } else c.order = default_order(); if (version >= 14) { if (!(in >> kind >> count) || kind != 'M' || count != 5) throw std::runtime_error("corrupt morphology inventory"); for (std::size_t i = 0; i < count; ++i) { std::string meaning, form; if (!(in >> std::quoted(meaning) >> std::quoted(form))) throw std::runtime_error("corrupt morphology form"); c.morphology.emplace(std::move(meaning), std::move(form)); } } if (version < 9) { const Candidate required = make_candidate(0, 0, impl_->rng); std::set<std::string> used; for (const auto& [_, word] : c.lexicon) used.insert(word); for (const auto& [atom, _] : required.lexicon) if (!c.lexicon.contains(atom)) c.lexicon.emplace(atom, new_word(impl_->rng, used)); c.order = default_order(); } ensure_morphology(c, impl_->rng); if (version == 1) c.metrics.grammar_complexity = 14.0; impl_->frontier.push_back(std::move(c)); }
  char marker{};
  if (version >= 3) { std::size_t agent_count{}; if (!(in >> marker >> agent_count) || marker != 'A') throw std::runtime_error("corrupt agent population"); for (std::size_t i = 0; i < agent_count; ++i) { AgentState agent; std::size_t knowledge_count{}; if (!(in >> marker >> agent.id >> agent.successful_interpretations >> agent.failed_interpretations >> knowledge_count) || marker != 'P') throw std::runtime_error("corrupt agent record"); for (std::size_t k = 0; k < knowledge_count; ++k) { std::string word, atom; if (!(in >> marker >> std::quoted(word) >> std::quoted(atom)) || marker != 'K') throw std::runtime_error("corrupt agent knowledge"); agent.private_reverse_lexicon.emplace(std::move(word), std::move(atom)); } impl_->agents.push_back(std::move(agent)); } }
  if (version >= 6) { if (!(in >> marker >> impl_->human_attempts >> impl_->human_correct >> impl_->human_total_millis) || marker != 'U') throw std::runtime_error("corrupt human study"); if (version >= 8 && !(in >> impl_->human_production_attempts >> impl_->human_production_correct >> impl_->human_interpretation_attempts >> impl_->human_interpretation_correct)) throw std::runtime_error("corrupt human study metrics"); }
  std::size_t historic{}; if (!(in >> marker >> historic) || marker != 'H') throw std::runtime_error(std::string("corrupt history marker: ") + marker); for (std::size_t i = 0; i < historic; ++i) { CandidateSummary h; if (!(in >> h.id >> h.parent_id >> h.metrics.communication_success >> h.metrics.semantic_accuracy >> h.metrics.cost >> h.metrics.pronounceability >> h.metrics.grammar_complexity >> h.metrics.robustness >> h.metrics.expressiveness >> std::quoted(h.sample))) throw std::runtime_error("corrupt historical record"); impl_->history.push_back(std::move(h)); }
  if (version >= 10) {
    std::size_t concept_count{};
    if (!(in >> marker >> concept_count) || marker != 'X') throw std::runtime_error("corrupt concept lexicon");
    impl_->concepts.reserve(concept_count);
    for (std::size_t i = 0; i < concept_count; ++i) {
      ConceptEntry entry;
      if (version >= 12) {
        if (!(in >> marker >> std::quoted(entry.id) >> std::quoted(entry.external_id) >> std::quoted(entry.label) >> std::quoted(entry.part_of_speech) >> std::quoted(entry.form) >> std::quoted(entry.derivation) >> std::quoted(entry.source) >> std::quoted(entry.provenance) >> std::quoted(entry.source_revision) >> std::quoted(entry.evidence_class) >> std::quoted(entry.language_status) >> std::quoted(entry.semantics) >> std::quoted(entry.relations) >> entry.introduced_generation >> entry.validation_attempts >> entry.validation_successes >> entry.validation_failures) || marker != 'Q') throw std::runtime_error("corrupt V12 concept entry");
      } else {
        if (!(in >> marker >> std::quoted(entry.id) >> std::quoted(entry.label) >> std::quoted(entry.part_of_speech) >> std::quoted(entry.form) >> std::quoted(entry.derivation) >> std::quoted(entry.source))) throw std::runtime_error("corrupt concept entry");
        if (version >= 11) { if (!(in >> std::quoted(entry.provenance))) throw std::runtime_error("corrupt concept provenance"); } else entry.provenance = "migrated-from-LENG_STATE-10";
        if (!(in >> std::quoted(entry.semantics) >> std::quoted(entry.relations) >> entry.introduced_generation >> entry.validation_attempts >> entry.validation_successes) || marker != 'Q') throw std::runtime_error("corrupt concept entry");
        entry.source_revision = "migrated-v" + std::to_string(version);
        entry.evidence_class = entry.derivation == "relation-compound" || entry.derivation == "compound" ? "simulated-relation" : "agent-hypothesis";
        entry.language_status = entry.validation_successes == entry.validation_attempts && entry.validation_attempts > 0 ? "communicatively-validated" : "realised";
        entry.validation_failures = entry.validation_attempts - entry.validation_successes;
      }
      impl_->concepts.push_back(std::move(entry));
    }
  }
  if (version >= 12) {
    auto& checkpoint = impl_->import_checkpoint;
    if (!(in >> marker >> std::quoted(checkpoint.source_path) >> std::quoted(checkpoint.fingerprint) >> checkpoint.next_line >> checkpoint.rows_seen >> checkpoint.rows_accepted >> checkpoint.rows_rejected >> checkpoint.complete >> std::quoted(checkpoint.status)) || marker != 'I') throw std::runtime_error("corrupt V12 import checkpoint");
  }
  if (version >= 13) {
    auto& checkpoint = impl_->german_word_list_checkpoint;
    if (!(in >> marker >> std::quoted(checkpoint.source_path) >> std::quoted(checkpoint.fingerprint) >> checkpoint.next_line >> checkpoint.rows_seen >> checkpoint.rows_accepted >> checkpoint.rows_rejected >> checkpoint.complete >> std::quoted(checkpoint.status)) || marker != 'W') throw std::runtime_error("corrupt V13 German word-list checkpoint");
  }
  impl_->rebuild_concept_indexes();
  if (impl_->agents.empty() && !impl_->frontier.empty()) { for (int i = 0; i < impl_->config.population; ++i) impl_->agents.push_back(AgentState{static_cast<std::uint64_t>(i + 1), {}, 0, 0}); train_population(impl_->frontier.front(), impl_->agents, impl_->rng, impl_->config.training_rounds * 3); }
  if (!impl_->frontier.empty()) impl_->ensure_concept_lexicon();
}

std::size_t ResearchEngine::import_concepts(const std::filesystem::path& tsv_path) {
  std::size_t total{};
  for (;;) { const auto progress = import_concepts_batch(tsv_path, 5000); total += progress.rows_accepted; if (progress.complete) return total; }
}

ImportProgress ResearchEngine::import_concepts_batch(const std::filesystem::path& tsv_path, std::size_t max_rows) {
  if (impl_->frontier.empty()) throw std::runtime_error("run an experiment before importing concepts");
  if (max_rows == 0) throw std::invalid_argument("import batch size must be positive");
  impl_->ensure_concept_lexicon();
  const auto canonical_path = std::filesystem::absolute(tsv_path).lexically_normal().string();
  const auto fingerprint = stable_fingerprint(tsv_path);
  auto& checkpoint = impl_->import_checkpoint;
  if (checkpoint.source_path != canonical_path || checkpoint.fingerprint != fingerprint) checkpoint = {canonical_path, fingerprint, 1, 0, 0, 0, false, "running"};
  ImportProgress result{}; result.source_fingerprint = fingerprint;
  if (checkpoint.complete) { result.complete = true; result.source_line = checkpoint.next_line; result.status = "source already complete"; return result; }
  std::ifstream input(tsv_path); if (!input) throw std::runtime_error("cannot read concept import: " + tsv_path.string());
  std::string line; std::size_t line_number{}; std::size_t processed{};
  std::vector<std::string> touched_labels;
  const auto append_once = [](std::string& value, const std::string& addition, const char* separator) { if (!addition.empty() && value.find(addition) == std::string::npos) value += (value.empty() ? "" : separator) + addition; };
  while (std::getline(input, line)) {
    ++line_number;
    if (line_number < checkpoint.next_line) continue;
    if (!line.empty() && line.back() == '\r') line.pop_back();
    if (trim(line).empty() || trim(line).starts_with('#')) { checkpoint.next_line = line_number + 1; continue; }
    const auto fields = split_tsv(line);
    if (line_number == 1 && !fields.empty() && (fields[0] == "label" || fields[0] == "external_id")) { checkpoint.next_line = 2; continue; }
    if (fields.size() != 6 && fields.size() != 9) throw std::runtime_error("concept import line " + std::to_string(line_number) + " must contain 6 legacy or 9 canonical TSV columns");
    ++processed; ++checkpoint.rows_seen; ++result.rows_seen;
    const bool canonical = fields.size() == 9;
    const std::string external_id = canonical ? fields[0] : "";
    const std::string label = canonical ? fields[1] : fields[0];
    const std::string part = canonical ? fields[2] : fields[1];
    const std::string semantics = canonical ? fields[3] : fields[2];
    const std::string relations = canonical ? fields[4] : fields[3];
    const std::string source = canonical ? fields[5] : fields[4];
    const std::string provenance = canonical ? fields[6] : fields[5];
    const std::string revision = canonical ? fields[7] : "legacy-tsv";
    const std::string evidence = canonical ? fields[8] : "external-attested";
    if (label.empty() || part.empty() || semantics.empty() || source.empty() || provenance.empty() || (canonical && external_id.empty())) { ++checkpoint.rows_rejected; ++result.concepts_rejected; checkpoint.next_line = line_number + 1; continue; }
    if (canonical && source == "Wikidata" && (external_id.size() < 2 || external_id.front() != 'Q')) { ++checkpoint.rows_rejected; ++result.concepts_rejected; checkpoint.next_line = line_number + 1; continue; }
    const std::string identity = external_id.empty() ? canonical_concept_key(label) : external_id;
    ConceptEntry* existing = impl_->find_concept(identity);
    if (existing == nullptr && external_id.empty()) existing = impl_->find_concept(label);
    if (existing) { append_once(existing->semantics, semantics, ";"); append_once(existing->relations, relations, ";"); append_once(existing->source, source, "+"); append_once(existing->provenance, provenance, " | "); append_once(existing->source_revision, revision, "+"); ++result.concepts_enriched; }
    else {
      ConceptEntry entry{}; entry.external_id = external_id; entry.label = label; entry.part_of_speech = part; entry.form = new_concept_word(impl_->rng, impl_->used_forms); entry.derivation = "imported-root"; entry.source = source; entry.provenance = provenance; entry.source_revision = revision; entry.evidence_class = evidence; entry.language_status = "realised"; entry.semantics = semantics; entry.relations = relations; entry.introduced_generation = impl_->generation; impl_->add_concept(std::move(entry)); ++result.concepts_created;
    }
    touched_labels.push_back(label);
    ++checkpoint.rows_accepted; ++result.rows_accepted; checkpoint.next_line = line_number + 1;
    if (processed >= max_rows) break;
  }
  // A full batch can end exactly on the final data line without a failed
  // getline() call.  Peek once so completion does not require a pointless
  // extra autonomous step.
  if (input.eof() || input.peek() == std::char_traits<char>::eof()) { checkpoint.complete = true; checkpoint.status = "complete"; }
  // Build a regular relational family only when both independently-attested
  // endpoints are known.  The entry remains marked simulated-relation; the
  // external edge itself stays in each source entry's relation evidence.
  for (const auto& source_label : touched_labels) {
    const auto* source = impl_->find_concept(source_label);
    if (!source) continue;
    const std::string source_id = source->id, source_name = source->label, source_form = source->form, source_semantics = source->semantics, source_relations = source->relations;
    std::istringstream relation_stream(source_relations);
    for (std::string relation; std::getline(relation_stream, relation, ';');) {
      const auto colon = relation.find(':'); if (colon == std::string::npos) continue;
      std::istringstream targets(relation.substr(colon + 1));
      for (std::string target_label; std::getline(targets, target_label, ',');) {
        target_label = trim(std::move(target_label)); const auto* target = impl_->find_concept(target_label);
        if (!target || target->id == source_id) continue;
        const std::string compound_label = source_name + "-" + target->label;
        if (impl_->find_concept(compound_label)) continue;
        const std::string target_id = target->id, target_form = target->form;
        ConceptEntry compound{}; compound.label = compound_label; compound.part_of_speech = "noun"; compound.form = source_form + "-" + target_form; compound.derivation = "relation-compound"; compound.source = "simulated-experience"; compound.provenance = "derived-from:" + source_id + ";relation:" + relation; compound.source_revision = "rule-v2"; compound.evidence_class = "simulated-relation"; compound.language_status = "realised"; compound.semantics = "composed;" + source_semantics; compound.relations = "derived-from:" + source_id + ";linked-to:" + target_id; compound.introduced_generation = impl_->generation; impl_->add_concept(std::move(compound)); ++result.relations_created;
      }
    }
  }
  result.source_line = checkpoint.next_line; result.complete = checkpoint.complete; result.status = checkpoint.status;
  // Validation is deliberately separate from external intake but advances in
  // the same bounded operation.  This never labels a merely imported row as
  // communicatively valid without an independent receiver attempt.
  const auto validated_now = validate_concepts(result.concepts_created + result.concepts_enriched);
  result.status += "; validation-trials=" + std::to_string(validated_now);
  return result;
}

ImportProgress ResearchEngine::import_german_word_list_batch(const std::filesystem::path& word_list_path,
                                                              std::size_t max_words) {
  if (impl_->frontier.empty()) throw std::runtime_error("run an experiment before importing word forms");
  if (max_words == 0) throw std::invalid_argument("word-list batch size must be positive");
  impl_->ensure_concept_lexicon();
  const auto canonical_path = std::filesystem::absolute(word_list_path).lexically_normal().string();
  const auto fingerprint = stable_fingerprint(word_list_path);
  auto& checkpoint = impl_->german_word_list_checkpoint;
  if (checkpoint.source_path != canonical_path || checkpoint.fingerprint != fingerprint)
    checkpoint = {canonical_path, fingerprint, 1, 0, 0, 0, false, "running"};
  ImportProgress result{}; result.source_fingerprint = fingerprint;
  if (checkpoint.complete) { result.complete = true; result.source_line = checkpoint.next_line; result.status = "local German word list already complete"; return result; }
  std::ifstream input(word_list_path); if (!input) throw std::runtime_error("cannot read German word list: " + word_list_path.string());
  std::string line; std::size_t line_number{}, processed{};
  while (std::getline(input, line)) {
    ++line_number;
    if (line_number < checkpoint.next_line) continue;
    if (!line.empty() && line.back() == '\r') line.pop_back();
    const std::string label = trim(std::move(line));
    if (label.empty() || label.starts_with('#')) { checkpoint.next_line = line_number + 1; continue; }
    ++processed; ++checkpoint.rows_seen; ++result.rows_seen;
    const std::string local_id = "LOCAL-DE:" + canonical_concept_key(label);
    // The source is a word-form list, not a semantic authority.  It therefore
    // receives a stable local ID and a separate evidence class; it must never
    // be counted as an externally attested or semantically validated concept.
    if (!impl_->find_concept(local_id)) {
      ConceptEntry entry{};
      // `external_id` stores a namespaced source-local identity here. It is
      // not a claim of external attestation; evidence_class below remains the
      // authoritative provenance category and prevents qualification.
      entry.external_id = local_id;
      entry.label = label;
      entry.part_of_speech = "unknown";
      entry.form = new_concept_word(impl_->rng, impl_->used_forms);
      entry.derivation = "local-word-form";
      entry.source = "user-provided-german-word-list";
      entry.provenance = "local-file:" + word_list_path.filename().string() + ";licence=unknown;meaning=unverified";
      entry.source_revision = fingerprint;
      entry.evidence_class = "agent-hypothesis";
      entry.language_status = "realised";
      entry.semantics = "lexical-form;language=de;meaning=unverified";
      entry.relations = "";
      entry.introduced_generation = impl_->generation;
      impl_->add_concept(std::move(entry));
      ++result.concepts_created;
    } else ++result.concepts_enriched;
    ++checkpoint.rows_accepted; ++result.rows_accepted; checkpoint.next_line = line_number + 1;
    if (processed >= max_words) break;
  }
  if (input.eof() || input.peek() == std::char_traits<char>::eof()) { checkpoint.complete = true; checkpoint.status = "complete"; }
  result.source_line = checkpoint.next_line; result.complete = checkpoint.complete; result.status = checkpoint.status;
  return result;
}

std::size_t ResearchEngine::validate_concepts(std::size_t max_concepts) {
  if (impl_->concepts.empty()) return 0;
  // Sender and receiver have deliberately separate maps.  Both learn atomic
  // forms from the persistent lexicon, while the receiver never sees the
  // composed candidate before the trial.  The semantic reference is rebuilt
  // from the component labels, not the candidate ID or its surface label.
  std::map<std::string, std::string> sender_atoms, receiver_atoms;
  if (impl_->agents.size() < 2) throw std::runtime_error("concept validation requires two persistent agents");
  AgentState& sender_agent = impl_->agents.at(0);
  AgentState& receiver_agent = impl_->agents.at(1);
  for (const auto& entry : impl_->concepts) {
    if (entry.derivation == "root" || entry.derivation == "imported-root") {
      sender_atoms.emplace(entry.form, entry.label);
      receiver_atoms.emplace(entry.form, entry.label);
    }
  }
  std::size_t validated{};
  for (auto& entry : impl_->concepts) {
    if (max_concepts != 0 && validated >= max_concepts) break;
    if (entry.derivation != "compound" && entry.derivation != "relation-compound") continue;
    if (entry.language_status == "communicatively-validated") continue;
    const auto expected = entry.label;
    const bool sender_can_encode = !entry.form.empty() && entry.form.find('-') != std::string::npos;
    bool receiver_can_decode = sender_can_encode;
    std::vector<std::string> recovered;
    std::istringstream parts(entry.form);
    for (std::string part; receiver_can_decode && std::getline(parts, part, '-');) {
      const auto learned = receiver_atoms.find(part);
      if (learned == receiver_atoms.end()) receiver_can_decode = false;
      else recovered.push_back(learned->second);
    }
    const bool independent_match = receiver_can_decode && join(recovered, "-") == expected;
    constexpr std::uint64_t kTrials = 6;
    entry.validation_attempts += kTrials;
    if (sender_can_encode && independent_match) { entry.validation_successes += kTrials; sender_agent.successful_interpretations += kTrials; receiver_agent.successful_interpretations += kTrials; }
    else { entry.validation_failures += kTrials; sender_agent.failed_interpretations += kTrials; receiver_agent.failed_interpretations += kTrials; }
    entry.language_status = sender_can_encode && independent_match ? "communicatively-validated" : "rejected";
    ++validated;
  }
  return validated;
}

std::string ResearchEngine::report() const { std::ostringstream out; const auto compute = inspect_compute(); out << "Research generation: " << impl_->generation << "\nCandidates: " << impl_->frontier.size() << "\nOpen concept lexicon: " << impl_->concepts.size() << " entries\n" << lexicon_status() << "\nCompute: " << compute.diagnostic; for (const auto& device : compute.devices) out << "\nOpenCL device: " << device; if (!impl_->frontier.empty()) { const auto& m = impl_->frontier.front().metrics; out << "\nBest candidate: " << impl_->frontier.front().id << " (parent " << impl_->frontier.front().parent << ")\nCommunication success (unseen compositions): " << m.communication_success << "\nSemantic accuracy: " << m.semantic_accuracy << "\nRobustness under one valid actor-token substitution: " << m.robustness << "\nMean bytes/message: " << m.cost << "\nPronounceability estimate: " << m.pronounceability << "\nGrammar complexity (1=canonical regular order): " << m.grammar_complexity << "\nPareto archive records: " << impl_->history.size(); } return out.str(); }
const std::vector<CandidateSummary>& ResearchEngine::archive() const { return impl_->history; }
std::string ResearchEngine::export_language() const { if (impl_->frontier.empty()) return "No language generated."; std::ostringstream out; out << "Language candidate " << impl_->frontier.front().id << "\nGrammar order: [" << grammar_description(impl_->frontier.front()) << "]\n"; for (const auto& [atom, word] : impl_->frontier.front().lexicon) out << atom << " = " << word << '\n'; return out.str(); }
std::string ResearchEngine::lexicon_trace() const {
  std::ostringstream out;
  out << "Autonomous lexicon — persistent concept graph\n";
  out << lexicon_status() << "\n";
  out << "Morphology rule version: " << kConceptMorphologyRuleVersion << "\n";
  out << "Validation: receiver agents reconstruct atomic forms and evaluate unseen composed forms; opaque concept IDs are not transmitted.\n\n";
  for (const auto& entry : impl_->concepts) {
    out << entry.id << "  " << entry.form << "\n"
        << "  meaning: " << entry.label << "  part of speech: " << entry.part_of_speech << "\n"
        << "  semantics: " << entry.semantics << "\n"
        << "  relations: " << entry.relations << "\n"
        << "  formation: " << entry.derivation << "  evidence: " << entry.evidence_class << "  source: " << entry.source << "  revision: " << entry.source_revision << "\n"
        << "  provenance: " << entry.provenance << "  external identity: " << (entry.external_id.empty() ? "<local>" : entry.external_id) << "  generation: " << entry.introduced_generation << "\n"
        << "  language status: " << entry.language_status << "  agent validation: " << entry.validation_successes << "/" << entry.validation_attempts << " successes; failures=" << entry.validation_failures << "\n\n";
  }
  return out.str();
}
std::string ResearchEngine::lexicon_status() const {
  std::size_t external{}, qualified_external{}, simulated{}, hypotheses{}, local_german_forms{}, realised{}, validated{}, pending{}, rejected{};
  for (const auto& entry : impl_->concepts) {
    if (entry.evidence_class == "external-attested") { ++external; if (externally_qualified(entry)) ++qualified_external; }
    else if (entry.evidence_class == "simulated-relation") ++simulated;
    else ++hypotheses;
    if (entry.source == "user-provided-german-word-list") ++local_german_forms;
    if (entry.language_status == "communicatively-validated") ++validated;
    else if (entry.language_status == "realised") ++realised;
    else if (entry.language_status == "rejected") ++rejected;
    else ++pending;
  }
  const auto& checkpoint = impl_->import_checkpoint;
  const auto& local_checkpoint = impl_->german_word_list_checkpoint;
  std::ostringstream out;
  out << "Lexicon status\nunique concepts=" << impl_->concepts.size() << "\nimported concepts=" << external << "\nqualified external concepts=" << qualified_external << "\nautomatically developed wordforms=" << (realised + validated) << "\nagentically validated concepts=" << validated << "\nexternal-attested=" << external << "\nsimulated-relations=" << simulated << "\nagent-hypotheses=" << hypotheses << "\nlocal German word forms (meaning unverified)=" << local_german_forms << "\nlanguage-realised=" << realised << "\ncommunicatively-validated=" << validated << "\npending=" << pending << "\nrejected=" << rejected << "\nimport-status=" << checkpoint.status << "\nimport-source=" << (checkpoint.source_path.empty() ? "<none>" : checkpoint.source_path) << "\nimport-next-line=" << checkpoint.next_line << "\nimport-accepted=" << checkpoint.rows_accepted << "\nimport-rejected=" << checkpoint.rows_rejected << "\nlocal-word-list-status=" << local_checkpoint.status << "\nlocal-word-list-source=" << (local_checkpoint.source_path.empty() ? "<none>" : local_checkpoint.source_path) << "\nlocal-word-list-next-line=" << local_checkpoint.next_line << "\nlocal-word-list-accepted=" << local_checkpoint.rows_accepted << "\nlocal-word-list-rejected=" << local_checkpoint.rows_rejected;
  return out.str();
}
std::string ResearchEngine::search_lexicon(const std::string& query, std::size_t limit) const {
  const auto needle = canonical_concept_key(query); if (needle.empty()) return "Lexicon search\nEnter a label, identity, source, or form.";
  std::ostringstream out; out << "Lexicon search: " << query << "\n"; std::size_t matches{};
  for (const auto& entry : impl_->concepts) {
    const auto matches_text = [&needle](const std::string& value) { return canonical_concept_key(value).find(needle) != std::string::npos; };
    if (!matches_text(entry.label) && !matches_text(entry.external_id) && !matches_text(entry.source) && !matches_text(entry.form)) continue;
    out << entry.id << "  " << entry.label << "  form=" << entry.form << "  evidence=" << entry.evidence_class << "  language=" << entry.language_status << "\n";
    if (++matches >= limit) break;
  }
  if (matches == 0) out << "No matching persistent concept.\n"; else out << "matches=" << matches << (matches == limit ? " (limit reached)" : "");
  return out.str();
}
std::string ResearchEngine::dictionary_page(const std::string& query, std::size_t page, std::size_t page_size) const {
  if (page_size == 0) throw std::invalid_argument("dictionary page size must be positive");
  const auto needle = canonical_concept_key(query);
  std::vector<const ConceptEntry*> matches;
  matches.reserve(impl_->concepts.size());
  for (const auto& entry : impl_->concepts) {
    const auto contains = [&needle](const std::string& value) {
      return needle.empty() || canonical_concept_key(value).find(needle) != std::string::npos;
    };
    if (contains(entry.label) || contains(entry.external_id) || contains(entry.form) || contains(entry.semantics)) matches.push_back(&entry);
  }
  const std::size_t pages = std::max<std::size_t>(1, (matches.size() + page_size - 1) / page_size);
  std::ostringstream out;
  out << "Dictionary / Wörterbuch\nSearch / Suche: " << (query.empty() ? "all persistent concepts" : query)
      << "\nPage / Seite: " << (page + 1) << " of " << pages << "  |  Matches / Treffer: " << matches.size() << "\n\n";
  if (page >= pages || matches.empty()) {
    out << (matches.empty() ? "No matching persistent concept / Kein passendes persistentes Konzept." : "No further page / Keine weitere Seite.");
    return out.str();
  }
  const std::size_t first = page * page_size;
  const std::size_t last = std::min(matches.size(), first + page_size);
  for (std::size_t i = first; i < last; ++i) {
    const auto& entry = *matches[i];
    bool german_fallback = false;
    const std::string german = localize_label(entry.label, "de", german_fallback);
    out << (i + 1) << ". " << entry.form << "\n"
        << "   English: " << entry.label << "  |  Deutsch: " << german << (german_fallback ? " (source label)" : "") << "\n"
        << "   Part of speech / Wortart: " << entry.part_of_speech << "  |  Status: " << entry.language_status << "\n"
        << "   Evidence / Evidenz: " << entry.evidence_class
        << "  |  Source / Quelle: " << entry.source
        << (entry.external_id.empty() ? "" : "  |  ID: " + entry.external_id) << "\n\n";
  }
  out << "Use Previous page / Next page to browse. Enter a word, form, QID, or semantic term and press Dictionary / Wörterbuch to start a new search.";
  return out.str();
}
std::string ResearchEngine::lexicon_benchmark() const {
  std::size_t payload_bytes{};
  for (const auto& entry : impl_->concepts) payload_bytes += entry.id.size() + entry.external_id.size() + entry.label.size() + entry.part_of_speech.size() + entry.form.size() + entry.derivation.size() + entry.source.size() + entry.provenance.size() + entry.source_revision.size() + entry.evidence_class.size() + entry.language_status.size() + entry.semantics.size() + entry.relations.size();
  const std::size_t probes = std::min<std::size_t>(1000, impl_->concepts.size());
  const auto started = std::chrono::steady_clock::now(); std::size_t hits{};
  for (std::size_t i = 0; i < probes; ++i) {
    const auto& entry = impl_->concepts[(i * 7919U) % impl_->concepts.size()];
    if (impl_->by_identity.contains(entry.external_id.empty() ? entry.id : entry.external_id)) ++hits;
  }
  const auto lookup_us = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::steady_clock::now() - started).count();
  std::size_t communication_candidates{}; for (const auto& entry : impl_->concepts) if (entry.language_status == "communicatively-validated") ++communication_candidates;
  std::ostringstream out;
  out << "Lexicon benchmark\nconcepts=" << impl_->concepts.size() << "\nestimated persistent string payload bytes=" << payload_bytes << "\nindex entries=" << impl_->by_identity.size() << "\nindex lookup probes=" << probes << "\nindex lookup hits=" << hits << "\nindex lookup micros=" << lookup_us << "\nindex lookup micros per probe=" << (probes == 0 ? 0.0 : static_cast<double>(lookup_us) / probes) << "\ncommunication-valid candidates=" << communication_candidates << "\nNote: memory is a conservative persistent-string payload estimate; allocator and process overhead are intentionally not misreported as exact RAM.";
  return out.str();
}

std::string ResearchEngine::translate(const std::string& text, const std::string& target_language) const {
  if (target_language != "de" && target_language != "en" && target_language != "constructed")
    throw std::invalid_argument("translation target must be de, en, or constructed");
  const auto input = words(text);
  if (input.empty()) throw std::invalid_argument("enter text to translate");
  if (impl_->frontier.empty()) throw std::runtime_error("generate or load a language before translating");
  const Candidate& language = impl_->frontier.front();
  std::map<std::string, std::string> form_to_label;
  std::map<std::string, std::string> label_to_form;
  std::map<std::string, std::string> local_word_to_form;
  for (const auto& [atom, form] : language.lexicon) {
    const auto separator = atom.find(':');
    const std::string label = atom.substr(separator + 1);
    form_to_label.emplace(canonical_concept_key(form), label);
    label_to_form.emplace(canonical_concept_key(label), form);
  }
  for (const auto& entry : impl_->concepts) {
    form_to_label.emplace(canonical_concept_key(entry.form), entry.label);
    label_to_form.emplace(canonical_concept_key(entry.label), entry.form);
    if (entry.derivation == "local-word-form") local_word_to_form.emplace(canonical_concept_key(entry.label), entry.form);
  }
  std::map<std::string, std::string> german_to_label;
  for (const auto& [english, german] : german_terms()) german_to_label.emplace(canonical_concept_key(german), english);

  std::vector<std::string> labels;
  std::vector<std::string> unresolved;
  bool saw_constructed = false;
  std::vector<std::string> constructed_sequence;
  for (std::size_t token_index = 0; token_index < input.size(); ++token_index) {
    const auto& token = input[token_index];
    const std::string previous = token_index == 0 ? "" : input[token_index - 1];
    // Persistent dictionary entries always win over heuristic morphology.
    // Otherwise ordinary nouns such as “Bett” would be misread as a verb
    // merely because they end in -t.
    if (const auto found = form_to_label.find(token); found != form_to_label.end()) { labels.push_back(found->second); saw_constructed = true; if (target_language == "constructed") constructed_sequence.push_back(token); continue; }
    if (const auto found = local_word_to_form.find(token); found != local_word_to_form.end()) { labels.push_back(token); if (target_language == "constructed") constructed_sequence.push_back(found->second); continue; }
    if (target_language == "constructed") if (const auto verb = analyse_german_verb(token, previous)) { labels.push_back(verb->root + ";" + (verb->number.empty() ? "infinitive" : verb->number) + ";" + verb->time); constructed_sequence.push_back(productive_verb_form(impl_->frontier.front(), *verb)); continue; }
    if (const auto found = label_to_form.find(token); found != label_to_form.end()) { labels.push_back(found->first); if (target_language == "constructed") constructed_sequence.push_back(found->second); continue; }
    if (const auto found = german_to_label.find(token); found != german_to_label.end()) { labels.push_back(found->second); if (target_language == "constructed" && label_to_form.contains(found->second)) constructed_sequence.push_back(label_to_form.at(found->second)); continue; }
    unresolved.push_back(token);
  }
  std::ostringstream out;
  out << "Semantic translator\n";
  out << "Input representation: " << (saw_constructed ? "constructed language / mixed" : "English or German semantic labels") << "\n";
  if (!unresolved.empty()) out << "Unresolved tokens (preserved, not guessed): " << join(unresolved, ", ") << "\n";
  if (labels.empty()) { out << "No persistent concept or grammar term was recognized."; return out.str(); }
  out << "Recognized concepts: " << join(labels, " | ") << "\n";
  if (target_language == "constructed") {
    out << "Constructed language: " << join(constructed_sequence, " ");
    if (std::any_of(labels.begin(), labels.end(), [](const std::string& item) { return item.find(';') != std::string::npos; })) out << "\nProductive morphology: root + person-3 + number + time (not a lexical lookup).";
  } else {
    bool fallback = false;
    std::vector<std::string> rendered;
    for (const auto& label : labels) rendered.push_back(localize_label(label, target_language, fallback));
    out << (target_language == "de" ? "Deutsch: " : "English: ") << join(rendered, " ");
    if (fallback) out << "\nNote: labels without a reviewed German gloss remain in their source-language form.";
  }
  return out.str();
}

std::string ResearchEngine::translate_meaning(const std::string& text, const std::string& target_language) const {
  if (target_language != "de" && target_language != "en" && target_language != "constructed")
    throw std::invalid_argument("meaning translation target must be de, en, or constructed");
  if (text.empty()) throw std::invalid_argument("enter a semantic-core sentence or a Lunari meaning frame");
  if (impl_->frontier.empty()) throw std::runtime_error("generate or load a language before translating a meaning");
  const Candidate& language = impl_->frontier.front();
  if (target_language == "constructed") {
    const auto meaning = analyse_meaning_sentence(text);
    if (!meaning) {
      throw std::invalid_argument("semantic-core analysis needs a known actor, action, and object; use the reversible text channel for unanalysed text");
    }
    std::ostringstream out;
    out << "Lunari meaning channel\nLunari frame:\n" << encode(language, *meaning)
        << "\n-- end Lunari frame --\n"
        << "Source-independent semantic frame:\n" << render_meaning(*meaning, "en")
        << "\nGuarantee: the Lunari frame contains evolved semantic-field forms, not German word forms.";
    return out.str();
  }
  std::map<std::string, std::string> receiver;
  for (const auto& [atom, form] : language.lexicon) receiver.emplace(form, atom);
  Meaning recovered;
  if (!decode(receiver, text, recovered)) {
    throw std::invalid_argument("not a complete Lunari semantic frame; all 15 semantic-field forms are required");
  }
  std::ostringstream out;
  out << "Lunari meaning channel\n" << render_meaning(recovered, target_language)
      << "\nDecoded independently from Lunari semantic-field forms; no German source text was consulted.";
  return out.str();
}

std::string ResearchEngine::translate_text(const std::string& text, const std::string& target_language) {
  if (target_language != "de" && target_language != "en" && target_language != "constructed")
    throw std::invalid_argument("translation target must be de, en, or constructed");
  if (text.empty()) throw std::invalid_argument("enter text to translate");
  if (impl_->frontier.empty()) throw std::runtime_error("generate or load a language before translating");
  std::map<std::string, std::string> label_to_form, form_to_label;
  for (const auto& entry : impl_->concepts) {
    label_to_form.emplace(canonical_concept_key(entry.label), entry.form);
    form_to_label.emplace(entry.form, entry.label);
  }
  for (const auto& [atom, form] : impl_->frontier.front().lexicon) {
    const auto separator = atom.find(':');
    label_to_form.emplace(atom.substr(separator + 1), form);
    form_to_label.emplace(form, atom.substr(separator + 1));
  }
  std::ostringstream rendered;
  std::size_t dictionary_tokens{}, learned_tokens{}, decoded_tokens{}, unresolved{};
  for (const auto& [lexical, piece] : text_segments(text)) {
    if (!lexical) { rendered << piece; continue; }
    if (target_language == "constructed") {
      const std::string key = canonical_concept_key(piece);
      auto known = label_to_form.find(key);
      if (known == label_to_form.end()) {
        // Text intake is lexical only: it preserves a new word and a Lunari
        // form, but never fabricates a meaning, part of speech, or English
        // gloss. The checkpoint is saved by the CLI/UI after this operation.
        ConceptEntry entry{};
        entry.external_id = "TEXT-DE:" + key;
        entry.label = piece;
        entry.part_of_speech = "unknown";
        entry.form = new_concept_word(impl_->rng, impl_->used_forms);
        entry.derivation = "translator-observed-word-form";
        entry.source = "translator-text-intake";
        entry.provenance = "observed-in-user-text;meaning=unverified";
        entry.source_revision = "runtime-text-intake";
        entry.evidence_class = "agent-hypothesis";
        entry.language_status = "realised";
        entry.semantics = "lexical-form;language=de;meaning=unverified";
        entry.introduced_generation = impl_->generation;
        const std::string form = entry.form;
        impl_->add_concept(std::move(entry));
        known = label_to_form.emplace(key, form).first;
        form_to_label.emplace(form, piece);
        ++learned_tokens;
      } else ++dictionary_tokens;
      const std::string& payload = known->second;
      rendered << apply_case_marker(payload, case_marker(piece));
      continue;
    }
    // Known Lunari forms are shown directly. The reserved huhuhu payload is
    // only used for source words absent from the persistent lexicon.
    const std::string marker = case_marker(piece);
    const std::string payload = canonical_concept_key(piece);
    std::optional<std::string> recovered;
    if (const auto known = form_to_label.find(payload); known != form_to_label.end()) recovered = known->second;
    else recovered = unescape_text_token(payload);
    if (!recovered) { rendered << piece; ++unresolved; continue; }
    // The local word list has no auditable English glosses. Preserve the
    // recovered source form rather than inventing an English translation.
    rendered << apply_case_marker(*recovered, marker); ++decoded_tokens;
  }
  std::ostringstream out;
  out << "Lunari text translator\n";
  if (target_language == "constructed") {
    out << "Lunari text:\n" << rendered.str() << "\n-- end Lunari text --\n"
        << "dictionary tokens=" << dictionary_tokens << "\nnewly learned tokens=" << learned_tokens
        << "\nGuarantee: punctuation, whitespace, case markers, dialogue structure, and newly observed UTF-8 tokens are retained without a source-text lookup.";
  } else {
    out << (target_language == "de" ? "Deutsch text:\n" : "English text:\n") << rendered.str() << "\n-- end translated text --\n"
        << "decoded tokens=" << decoded_tokens << "\nunresolved Lunari tokens=" << unresolved;
    if (target_language == "en") out << "\nNote: local German word-list entries have no verified English meanings; recovered German forms are preserved instead of guessed.";
  }
  return out.str();
}
std::string ResearchEngine::concept_communication_trace() const {
  const auto find_label = [this](const std::string& label) -> const ConceptEntry* {
    const auto entry = std::find_if(impl_->concepts.begin(), impl_->concepts.end(), [&label](const ConceptEntry& value) { return value.label == label; });
    return entry == impl_->concepts.end() ? nullptr : &*entry;
  };
  const ConceptEntry* compound = find_label("forest-fire");
  const ConceptEntry* modifier = find_label("danger");
  if (!compound || !modifier) return "Concept communication lab\nRun a generation first; no validated forest-fire/danger family is available.";
  std::map<std::string, std::string> receiver;
  for (const auto& entry : impl_->concepts) if (entry.derivation == "root" || entry.derivation == "imported-root") receiver.emplace(entry.form, entry.label);
  const std::string message = compound->form + " " + modifier->form;
  std::vector<std::string> recovered;
  std::istringstream tokens(message);
  bool decoded = true;
  for (std::string token; tokens >> token;) {
    if (token.find('-') == std::string::npos) {
      const auto known = receiver.find(token); if (known == receiver.end()) { decoded = false; break; } recovered.push_back(known->second);
      continue;
    }
    std::istringstream components(token); std::vector<std::string> labels;
    for (std::string component; std::getline(components, component, '-');) { const auto known = receiver.find(component); if (known == receiver.end()) { decoded = false; break; } labels.push_back(known->second); }
    if (!decoded) break;
    recovered.push_back(join(labels, "-"));
  }
  const std::string expected = "forest-fire danger";
  const std::string result = join(recovered, " ");
  std::ostringstream out;
  out << "Concept communication lab — unseen compositional phrase\n"
      << "Message: " << message << "\n"
      << "Expected concepts: " << expected << "\n"
      << "Receiver reconstruction: " << (decoded ? result : "<decode failure>") << "\n"
      << "Task success: " << ((decoded && result == expected) ? "yes" : "no") << "\n"
      << "Note: the receiver reconstructs basic forms and composes the family label; no CONCEPT_* ID is transmitted.";
  return out.str();
}
std::string ResearchEngine::productive_grammar_trace() const {
  if (impl_->frontier.empty()) return "Productive morphology lab\nNo evolved language candidate available.";
  return productive_grammar_test(impl_->frontier.front());
}
std::string ResearchEngine::phonotactic_trace() const {
  if (impl_->frontier.empty()) return "Lunari phonotactic lab\nNo evolved language candidate available.";
  const auto& language = impl_->frontier.front();
  std::size_t total{}, open{}, rejected{}, candidate_forms{}, concept_forms{};
  double combined{};
  const auto inspect = [&](const std::string& form) {
    const auto score = phonotactic_score(form); ++total; combined += score.combined;
    if (score.open_syllables && score.structure > 0.0) ++open; else ++rejected;
  };
  for (const auto& [_, form] : language.lexicon) { inspect(form); ++candidate_forms; }
  for (const auto& [_, form] : language.morphology) { inspect(form); ++candidate_forms; }
  for (const auto& entry : impl_->concepts) { inspect(entry.form); ++concept_forms; }
  std::ostringstream out;
  out << "Lunari phonotactic lab\n"
      << "phonology version: " << kConceptMorphologyRuleVersion << "\n"
      << "phoneme inventory: vowels a e i o u; onsets m n l r p t k b d g f s h v\n"
      << "allowed syllables: V, CV; word formation: CVCV+ (open syllables only)\n"
      << "scoring heuristic: C=0.30*structure + 0.30*transition + 0.20*length + 0.20*acoustic-separation\n"
      << "evaluated candidate forms=" << candidate_forms << "\nevaluated persistent concept forms=" << concept_forms << "\nevaluated forms total=" << total << "\nopen-syllable forms=" << open << "\nrejected forms=" << rejected
      << "\nmean phonotactic score=" << (total == 0 ? 0.0 : combined / total)
      << "\nopen-syllable audit: " << (rejected == 0 ? "passed" : "failed")
      << "\nNote: these are explicit research heuristics, not a claim of calibrated human speech data.";
  return out.str();
}
std::string ResearchEngine::communication_trace() const {
  if (impl_->frontier.empty() || impl_->agents.empty()) return "No trained agents available. Run an experiment first.";
  const Meaning expected{SpeechAct::question, "researcher", "observe", "idea", "if", false, 2, 3};
  const auto& language = impl_->frontier.front();
  const auto encode_started = std::chrono::steady_clock::now();
  const std::string message = encode(language, expected);
  const auto encode_finished = std::chrono::steady_clock::now();
  Meaning decoded;
  const auto& receiver = impl_->agents.front();
  const auto decode_started = std::chrono::steady_clock::now();
  const bool decoded_ok = decode(receiver.private_reverse_lexicon, message, decoded);
  const auto decode_finished = std::chrono::steady_clock::now();
  std::size_t receiver_bytes{};
  for (const auto& [word, atom] : receiver.private_reverse_lexicon) receiver_bytes += word.size() + atom.size();
  const auto encode_us = std::chrono::duration_cast<std::chrono::microseconds>(encode_finished - encode_started).count();
  const auto decode_us = std::chrono::duration_cast<std::chrono::microseconds>(decode_finished - decode_started).count();
  std::ostringstream out;
  out << "Communication lab — sender and receiver use separate lexicon maps\n"
      << "Receiver agent: " << receiver.id << " (learned tokens: " << receiver.private_reverse_lexicon.size() << ", successes: " << receiver.successful_interpretations << ", failures: " << receiver.failed_interpretations << ")\n"
      << "Message: " << message << "\n"
      << "Message bytes: " << message.size() << "\n"
      << "Encode micros: " << encode_us << "\n"
      << "Decode micros: " << decode_us << "\n"
      << "Receiver lexicon payload bytes: " << receiver_bytes << "\n"
      << "Expected semantic frame: " << expected.key() << "\n"
      << "Decoded semantic frame: " << (decoded_ok ? decoded.key() : "<decode failure>") << "\n"
      << "Task success: " << ((decoded_ok && decoded == expected) ? "yes" : "no") << "\n"
      << "Note: the decoder was called with message tokens and its private reverse lexicon only.";
  return out.str();
}
std::string ResearchEngine::evolution_trace() const {
  std::ostringstream out;
  out << "Evolution lab — Pareto archive\n";
  if (impl_->history.empty()) return out.str() + "No completed generations.";
  const std::size_t first = impl_->history.size() > 16 ? impl_->history.size() - 16 : 0;
  for (std::size_t i = first; i < impl_->history.size(); ++i) {
    const auto& candidate = impl_->history[i];
    out << "candidate=" << candidate.id << " parent=" << candidate.parent_id << " success=" << candidate.metrics.communication_success << " semantics=" << candidate.metrics.semantic_accuracy << " bytes=" << candidate.metrics.cost << " pronunciation=" << candidate.metrics.pronounceability << '\n';
  }
  return out.str();
}
std::string ResearchEngine::population_trace() const {
  std::ostringstream out;
  out << "Agent population\n";
  if (impl_->agents.empty()) return out.str() + "No persisted agents.";
  for (const auto& agent : impl_->agents) out << "agent=" << agent.id << " learned_tokens=" << agent.private_reverse_lexicon.size() << " successes=" << agent.successful_interpretations << " failures=" << agent.failed_interpretations << '\n';
  return out.str();
}
HumanExercise ResearchEngine::create_human_exercise() const {
  if (impl_->frontier.empty()) return {"Start an experiment first.", "", 0, false};
  const auto& language = impl_->frontier.front();
  const unsigned lesson = static_cast<unsigned>(impl_->human_attempts % 4);
  if (lesson == 0) return {"Human language lab — Lesson 1 of 4: recognition\n\nThe language word `" + language.lexicon.at("act:request") + "` has which English meaning?\n\nINPUT: enter one English word only.", "request|command", unix_millis(), true};
  if (lesson == 1) return {"Human language lab — Lesson 2 of 4: recall\n\nWhich generated language word means `child`?\n\nINPUT: enter one generated word only.", language.lexicon.at("actor:child"), unix_millis(), false};
  if (lesson == 2) return {"Human language lab — Lesson 3 of 4: short interpretation\n\nDecode this four-word message:\n`" + language.lexicon.at("act:request") + " " + language.lexicon.at("actor:child") + " " + language.lexicon.at("action:build") + " " + language.lexicon.at("object:map") + "`\n\nINPUT: enter exactly four English words: act actor action object.", "request child build map", unix_millis(), true};
  return {"Human language lab — Lesson 4 of 4: short production\n\nForm the message: `request child build map`.\n\nINPUT: enter exactly four generated words in that order. Do not enter field names or semicolons.", language.lexicon.at("act:request") + " " + language.lexicon.at("actor:child") + " " + language.lexicon.at("action:build") + " " + language.lexicon.at("object:map"), unix_millis(), false};
}
bool ResearchEngine::check_human_exercise(const HumanExercise& exercise, const std::string& answer) {
  bool correct = false;
  std::istringstream alternatives(exercise.expected_answer);
  for (std::string alternative; std::getline(alternatives, alternative, '|');) {
    if (canonical_answer(answer) == canonical_answer(alternative)) { correct = true; break; }
  }
  ++impl_->human_attempts;
  if (correct) ++impl_->human_correct;
  if (exercise.interpretation) { ++impl_->human_interpretation_attempts; if (correct) ++impl_->human_interpretation_correct; }
  else { ++impl_->human_production_attempts; if (correct) ++impl_->human_production_correct; }
  const std::uint64_t now = unix_millis();
  if (exercise.issued_unix_ms != 0 && now >= exercise.issued_unix_ms) impl_->human_total_millis += now - exercise.issued_unix_ms;
  return correct;
}
std::string ResearchEngine::human_lab_trace() const {
  std::ostringstream out;
  const double accuracy = impl_->human_attempts == 0 ? 0.0 : static_cast<double>(impl_->human_correct) / impl_->human_attempts;
  const double mean_millis = impl_->human_attempts == 0 ? 0.0 : static_cast<double>(impl_->human_total_millis) / impl_->human_attempts;
  const double recognition = impl_->human_interpretation_attempts == 0 ? 0.0 : static_cast<double>(impl_->human_interpretation_correct) / impl_->human_interpretation_attempts;
  const double production = impl_->human_production_attempts == 0 ? 0.0 : static_cast<double>(impl_->human_production_correct) / impl_->human_production_attempts;
  out << "Human language lab\nattempts=" << impl_->human_attempts << "\ncorrect=" << impl_->human_correct << "\nerror_rate=" << (1.0 - accuracy) << "\nrecognition_rate=" << recognition << "\nproduction_rate=" << production << "\nmean_response_millis=" << mean_millis << "\nNote: these are recorded participant interactions, not predicted human learnability.";
  return out.str();
}
int ResearchEngine::generation() const { return impl_->generation; }

bool self_test(std::string& error) {
  try {
    RunConfig config; config.population = 4; config.generations = 2; config.training_rounds = 50; config.evaluation_rounds = 20;
    ResearchEngine e(config); e.run();
    if (e.generation() != 2 || e.archive().empty()) throw std::runtime_error("evolution did not advance");
    if (e.phonotactic_trace().find("open-syllable audit: passed") == std::string::npos ||
        e.phonotactic_trace().find("phoneme inventory: vowels a e i o u") == std::string::npos)
      throw std::runtime_error("Lunari phonotactic validation mismatch");
    const double noisy_robustness = e.archive().back().metrics.robustness;
    if (noisy_robustness <= 0.0 || noisy_robustness >= 1.0) throw std::runtime_error("noise robustness metric mismatch");
    if (e.communication_trace().find("Task success: yes") == std::string::npos || e.concept_communication_trace().find("Task success: yes") == std::string::npos || e.evolution_trace().find("candidate=") == std::string::npos) throw std::runtime_error("research laboratory trace mismatch");
    const auto import_path = std::filesystem::temp_directory_path() / "leng-self-import.tsv";
    { std::ofstream fixture(import_path); fixture << "label\tpart_of_speech\tsemantics\trelations\tsource\tprovenance\n" << "rain\tnoun\tweather;water\tcan-affect:forest;causes:flood\tWikidata\tCC0;test-fixture\n" << "flood\tnoun\twater;overflow\t\tWikidata\tCC0;test-fixture\n" << "melody\tnoun\tmusic;ordered-sound\tis-a:music-concept\tWikidata\tCC0;test-fixture\n"; }
    if (e.import_concepts(import_path) != 3) throw std::runtime_error("concept import count mismatch");
    const auto enrichment_path = std::filesystem::temp_directory_path() / "leng-self-enrichment.tsv";
    { std::ofstream fixture(enrichment_path); fixture << "label\tpart_of_speech\tsemantics\trelations\tsource\tprovenance\n" << "tree\tnoun\tliving;plant;external-knowledge\tcan-affect:forest\tWikidata\tCC0;enrichment-fixture\n"; }
    if (e.import_concepts(enrichment_path) != 1) throw std::runtime_error("concept enrichment count mismatch");
    const auto word_list_path = std::filesystem::temp_directory_path() / "leng-self-german-words.txt";
    { std::ofstream fixture(word_list_path); fixture << "Haus\nBett\nHaus\n"; }
    const auto word_progress = e.import_german_word_list_batch(word_list_path, 2);
    if (word_progress.concepts_created != 2 || word_progress.complete ||
        e.translate("Haus", "constructed").find("Constructed language:") == std::string::npos ||
        e.translate("Bett", "constructed").find("Productive morphology:") != std::string::npos)
      throw std::runtime_error("local German word-list import mismatch");
    const auto final_word_progress = e.import_german_word_list_batch(word_list_path, 10);
    if (!final_word_progress.complete || e.lexicon_status().find("local German word forms (meaning unverified)=2") == std::string::npos)
      throw std::runtime_error("local German word-list checkpoint mismatch");
    const std::string lexicon_before_text = e.lexicon_trace();
    if (lexicon_before_text.find("forest-fire") == std::string::npos || lexicon_before_text.find("rain-forest") == std::string::npos || lexicon_before_text.find("rain-flood") == std::string::npos || lexicon_before_text.find("tree-forest") == std::string::npos || lexicon_before_text.find("seeded-human-knowledge+Wikidata") == std::string::npos || lexicon_before_text.find("CC0;test-fixture") == std::string::npos || lexicon_before_text.find("language status: communicatively-validated") == std::string::npos || lexicon_before_text.find("evidence: external-attested") == std::string::npos) throw std::runtime_error("concept lexicon validation mismatch");
    const std::string population_before_save = e.population_trace();
    const auto recognition = e.create_human_exercise(); if (!recognition.interpretation || !e.check_human_exercise(recognition, "request")) throw std::runtime_error("recognition lesson validation mismatch");
    const auto recall = e.create_human_exercise(); if (recall.interpretation || !e.check_human_exercise(recall, recall.expected_answer)) throw std::runtime_error("recall lesson validation mismatch");
    const auto interpretation = e.create_human_exercise(); if (!interpretation.interpretation || !e.check_human_exercise(interpretation, interpretation.expected_answer)) throw std::runtime_error("interpretation lesson validation mismatch");
    const auto production = e.create_human_exercise(); if (production.interpretation || !e.check_human_exercise(production, production.expected_answer) || e.check_human_exercise(production, "incorrect")) throw std::runtime_error("production lesson validation mismatch");
    const std::string german_translation = e.translate("request child build map", "de");
    const std::string constructed_translation = e.translate("sie bauen", "constructed");
    if (german_translation.find("Deutsch: bitte kind bauen karte") == std::string::npos ||
        constructed_translation.find("Constructed language:") == std::string::npos ||
        constructed_translation.find("bauen;pl;present") == std::string::npos ||
        constructed_translation.find("Productive morphology:") == std::string::npos)
      throw std::runtime_error("semantic translator validation mismatch");
    const std::string meaning_forward = e.translate_meaning("Der Mensch bewegt den Stein nicht gestern nahe.", "constructed");
    const auto frame_start = meaning_forward.find("Lunari frame:\n");
    const auto frame_end = meaning_forward.find("\n-- end Lunari frame --", frame_start);
    if (frame_start == std::string::npos || frame_end == std::string::npos) throw std::runtime_error("semantic frame encoding mismatch");
    const std::string meaning_frame = meaning_forward.substr(frame_start + std::string("Lunari frame:\n").size(), frame_end - (frame_start + std::string("Lunari frame:\n").size()));
    const std::string meaning_backward = e.translate_meaning(meaning_frame, "en");
    if (meaning_backward.find("human / move / stone") == std::string::npos ||
        meaning_backward.find("Negation: yes") == std::string::npos ||
        meaning_backward.find("Time: past") == std::string::npos ||
        meaning_backward.find("Decoded independently") == std::string::npos)
      throw std::runtime_error("independent semantic meaning channel mismatch");
    const auto semantic_state_path = std::filesystem::temp_directory_path() / "leng-semantic-channel.leng";
    e.save(semantic_state_path);
    ResearchEngine independently_loaded;
    independently_loaded.load(semantic_state_path);
    const std::string persisted_meaning = independently_loaded.translate_meaning(meaning_frame, "de");
    if (persisted_meaning.find("mensch / bewegen / stein") == std::string::npos ||
        persisted_meaning.find("Negation: ja") == std::string::npos ||
        persisted_meaning.find("Zeit: Vergangenheit") == std::string::npos)
      throw std::runtime_error("persisted independent semantic channel mismatch");
    const std::string text_fixture = "\xE2\x80\x9E" "Bett?" "\xE2\x80\x9C" ", fragte Sie.\n\nQzx-17 bleibt!";
    const auto extract_text = [](const std::string& report, const std::string& prefix, const std::string& footer) {
      const auto start = report.find(prefix); if (start == std::string::npos) return std::string{};
      const auto value_start = start + prefix.size(); const auto end = report.find(footer, value_start);
      return report.substr(value_start, end == std::string::npos ? std::string::npos : end - value_start);
    };
    const std::string lunari_text = extract_text(e.translate_text(text_fixture, "constructed"), "Lunari text:\n", "\n-- end Lunari text --");
    const std::string text_round_trip = extract_text(e.translate_text(lunari_text, "de"), "Deutsch text:\n", "\n-- end translated text --");
    if (lunari_text.empty() || lunari_text.find("huhuhu") != std::string::npos || text_round_trip != text_fixture) throw std::runtime_error("lossless Lunari text translation mismatch");
    if (e.productive_grammar_trace().find("regular-marker validation: passed") == std::string::npos ||
        e.productive_grammar_trace().find("independent receiver reconstruction: lernen;pl;past") == std::string::npos ||
        e.translate("sie lernen", "constructed").find("lernen;pl;present") == std::string::npos ||
        e.translate("sie lernten", "constructed").find("lernen;pl;past") == std::string::npos)
      throw std::runtime_error("productive morphology validation mismatch");
    if (e.dictionary_page("rain", 0).find("English: rain") == std::string::npos ||
        e.dictionary_page("rain", 0).find("Deutsch: regen") == std::string::npos)
      throw std::runtime_error("dictionary validation mismatch");
    const std::string lexicon_before_save = e.lexicon_trace();
    const auto path = std::filesystem::temp_directory_path() / "leng-self-test.state"; const auto export_dir = std::filesystem::temp_directory_path() / "leng-self-export";
    e.save(path); e.export_results(export_dir);
    if (!std::filesystem::exists(export_dir / "manifest.txt") || !std::filesystem::exists(export_dir / "pareto_archive.csv") || !std::filesystem::exists(export_dir / "evolutionary_lexicon.txt") || e.human_lab_trace().find("recognition_rate=") == std::string::npos) throw std::runtime_error("research export or human metrics mismatch");
    std::string exported_lexicon_text;
    { std::ifstream exported_lexicon(export_dir / "evolutionary_lexicon.txt"); std::ostringstream content; content << exported_lexicon.rdbuf(); exported_lexicon_text = content.str(); }
    if (exported_lexicon_text.find("rain-flood") == std::string::npos || exported_lexicon_text.find("CC0;test-fixture") == std::string::npos) throw std::runtime_error("exported concept provenance mismatch");
    if (compare_research_states(path, path).find("Generation delta (right-left): 0") == std::string::npos) throw std::runtime_error("state comparison mismatch");
    RunConfig resume_config = config; resume_config.generations = 1; ResearchEngine restored(resume_config); ResearchEngine replay(resume_config); restored.load(path); replay.load(path);
    if (restored.generation() != e.generation() || restored.export_language() != e.export_language() || restored.population_trace() != population_before_save || restored.lexicon_trace() != lexicon_before_save || restored.productive_grammar_trace() != e.productive_grammar_trace()) throw std::runtime_error("persistence round-trip mismatch");
    restored.run(); replay.run();
    if (restored.generation() != 3 || restored.export_language() != replay.export_language() || restored.population_trace() != replay.population_trace() || restored.lexicon_trace() != replay.lexicon_trace()) throw std::runtime_error("deterministic continuation mismatch");
    std::filesystem::remove(path); std::filesystem::remove(import_path); std::filesystem::remove(enrichment_path); std::filesystem::remove(word_list_path); std::filesystem::remove_all(export_dir); const auto parity = run_reward_parity_test(); if (parity.attempted && !parity.passed) throw std::runtime_error(parity.detail); return true;
  } catch (const std::exception& ex) { error = ex.what(); return false; }
}
std::string compare_research_states(const std::filesystem::path& left, const std::filesystem::path& right) {
  ResearchEngine first; first.load(left);
  ResearchEngine second; second.load(right);
  const auto describe = [](const ResearchEngine& engine, const std::filesystem::path& path) {
    std::ostringstream out;
    out << path.string() << "\nGeneration: " << engine.generation();
    if (!engine.archive().empty()) {
      const auto& candidate = engine.archive().back();
      out << "\nCandidate: " << candidate.id << "\nCommunication: " << candidate.metrics.communication_success
          << "\nSemantics: " << candidate.metrics.semantic_accuracy << "\nBytes/message: " << candidate.metrics.cost
          << "\nPronounceability: " << candidate.metrics.pronounceability << "\nRobustness: " << candidate.metrics.robustness;
    }
    return out.str();
  };
  std::ostringstream out;
  out << "Experiment comparison (persisted engine states)\n\n" << describe(first, left) << "\n\n" << describe(second, right)
      << "\n\nGeneration delta (right-left): " << (second.generation() - first.generation());
  if (!first.archive().empty() && !second.archive().empty()) {
    const auto& a = first.archive().back().metrics; const auto& b = second.archive().back().metrics;
    out << "\nCommunication delta: " << (b.communication_success - a.communication_success)
        << "\nSemantic delta: " << (b.semantic_accuracy - a.semantic_accuracy)
        << "\nCost delta: " << (b.cost - a.cost) << "\nRobustness delta: " << (b.robustness - a.robustness);
  }
  return out.str();
}
}  // namespace leng

========== END FILE ==========

========== FILE: Lunari-Developer/src/main.cpp ==========

#include "leng/engine.hpp"
#include "leng/compute.hpp"

#include <exception>
#include <filesystem>
#include <iostream>
#include <string>

namespace {
void usage() {
  std::cout << "LINGUISTISCHE EMERGENZ NG\n"
               "Usage:\n"
               "  linguistische-emergenz-ng --self-test\n"
               "  linguistische-emergenz-ng --compute-test\n"
               "  linguistische-emergenz-ng --run <state.leng> [generations]\n"
               "  linguistische-emergenz-ng --run-local-lexicon <state.leng> [generations]\n"
               "  linguistische-emergenz-ng --resume <state.leng> [generations]\n"
               "  linguistische-emergenz-ng --report <state.leng>\n"
               "  linguistische-emergenz-ng --communication <state.leng>\n"
               "  linguistische-emergenz-ng --evolution <state.leng>\n"
               "  linguistische-emergenz-ng --population <state.leng>\n"
               "  linguistische-emergenz-ng --human-lab <state.leng>\n"
               "  linguistische-emergenz-ng --lexicon <state.leng>\n"
               "  linguistische-emergenz-ng --concept-communication <state.leng>\n"
               "  linguistische-emergenz-ng --productive-grammar <state.leng>\n"
               "  linguistische-emergenz-ng --phonotactics <state.leng>\n"
               "  linguistische-emergenz-ng --resave-state <state.leng>\n"
               "  linguistische-emergenz-ng --import-concepts <state.leng> <concepts.tsv>\n"
               "  linguistische-emergenz-ng --import-batch <state.leng> <concepts.tsv> [rows]\n"
               "  linguistische-emergenz-ng --import-german-batch <state.leng> <words.txt> [words]\n"
               "  linguistische-emergenz-ng --autonomous-step <state.leng> <concepts.tsv> [rows]\n"
               "  linguistische-emergenz-ng --lexicon-status <state.leng>\n"
               "  linguistische-emergenz-ng --search-lexicon <state.leng> <query>\n"
               "  linguistische-emergenz-ng --lexicon-benchmark <state.leng>\n"
               "  linguistische-emergenz-ng --translate <state.leng> <de|en|constructed> <text>\n"
               "  linguistische-emergenz-ng --translate-text <state.leng> <de|en|constructed> <text>\n"
               "  linguistische-emergenz-ng --translate-meaning <state.leng> <de|en|constructed> <text>\n"
               "  linguistische-emergenz-ng --compare <left.leng> <right.leng>\n"
               "  linguistische-emergenz-ng --export <state.leng> <directory>\n";
}
}

int main(int argc, char** argv) {
  try {
    if (argc == 2 && std::string(argv[1]) == "--self-test") {
      std::string error;
      if (!leng::self_test(error)) { std::cerr << "SELF-TEST FAILED: " << error << '\n'; return 1; }
      std::cout << "SELF-TEST PASSED\n";
      return 0;
    }
    if (argc == 2 && std::string(argv[1]) == "--compute-test") {
      const auto parity = leng::run_reward_parity_test();
      std::cout << parity.detail << "\nmax_absolute_error=" << parity.max_absolute_error << '\n';
      return parity.skipped || parity.passed ? 0 : 1;
    }
    if (argc == 4 && std::string(argv[1]) == "--export") {
      leng::ResearchEngine engine;
      engine.load(argv[2]);
      engine.export_results(argv[3]);
      std::cout << "Export completed: " << argv[3] << '\n';
      return 0;
    }
    if (argc == 4 && std::string(argv[1]) == "--compare") {
      std::cout << leng::compare_research_states(argv[2], argv[3]) << '\n';
      return 0;
    }
    if (argc == 4 && std::string(argv[1]) == "--import-concepts") {
      leng::ResearchEngine engine;
      engine.load(argv[2]);
      const auto imported = engine.import_concepts(argv[3]);
      engine.save(argv[2]);
      std::cout << "Imported " << imported << " concepts and saved " << argv[2] << '\n';
      return 0;
    }
    if ((argc == 4 || argc == 5) && (std::string(argv[1]) == "--import-batch" || std::string(argv[1]) == "--autonomous-step")) {
      leng::ResearchEngine engine;
      engine.load(argv[2]);
      const std::size_t rows = argc == 5 ? static_cast<std::size_t>(std::stoull(argv[4])) : 1000;
      const auto progress = engine.import_concepts_batch(argv[3], rows);
      engine.save(argv[2]);
      std::cout << "Autonomous intake step\nsource-line=" << progress.source_line << "\naccepted=" << progress.rows_accepted << "\ncreated=" << progress.concepts_created << "\nenriched=" << progress.concepts_enriched << "\nrejected=" << progress.concepts_rejected << "\nrelations-created=" << progress.relations_created << "\ncomplete=" << (progress.complete ? "yes" : "no") << "\nfingerprint=" << progress.source_fingerprint << "\nstatus=" << progress.status << '\n';
      return 0;
    }
    if ((argc == 4 || argc == 5) && std::string(argv[1]) == "--import-german-batch") {
      leng::ResearchEngine engine;
      engine.load(argv[2]);
      const std::size_t words = argc == 5 ? static_cast<std::size_t>(std::stoull(argv[4])) : 50000;
      const auto progress = engine.import_german_word_list_batch(argv[3], words);
      engine.save(argv[2]);
      std::cout << "Local German word-list step\nsource-line=" << progress.source_line << "\naccepted=" << progress.rows_accepted << "\ncreated=" << progress.concepts_created << "\ncomplete=" << (progress.complete ? "yes" : "no") << "\nstatus=" << progress.status << '\n';
      return 0;
    }
    if (argc == 3 && std::string(argv[1]) == "--lexicon-status") {
      leng::ResearchEngine engine; engine.load(argv[2]); std::cout << engine.lexicon_status() << '\n'; return 0;
    }
    if (argc == 4 && std::string(argv[1]) == "--search-lexicon") {
      leng::ResearchEngine engine; engine.load(argv[2]); std::cout << engine.search_lexicon(argv[3]) << '\n'; return 0;
    }
    if (argc == 3 && std::string(argv[1]) == "--lexicon-benchmark") {
      leng::ResearchEngine engine; engine.load(argv[2]); std::cout << engine.lexicon_benchmark() << '\n'; return 0;
    }
    if (argc == 3 && std::string(argv[1]) == "--resave-state") {
      leng::ResearchEngine engine; engine.load(argv[2]); engine.save(argv[2]); std::cout << "State migrated and saved: " << argv[2] << '\n'; return 0;
    }
    if (argc == 5 && std::string(argv[1]) == "--translate") {
      leng::ResearchEngine engine; engine.load(argv[2]); std::cout << engine.translate(argv[4], argv[3]) << '\n'; return 0;
    }
    if (argc == 5 && std::string(argv[1]) == "--translate-text") {
      leng::ResearchEngine engine; engine.load(argv[2]); const std::string report = engine.translate_text(argv[4], argv[3]); if (std::string(argv[3]) == "constructed" && report.find("newly learned tokens=0") == std::string::npos) engine.save(argv[2]); std::cout << report << '\n'; return 0;
    }
    if (argc == 5 && std::string(argv[1]) == "--translate-meaning") {
      leng::ResearchEngine engine; engine.load(argv[2]); std::cout << engine.translate_meaning(argv[4], argv[3]) << '\n'; return 0;
    }
    if (argc < 3) { usage(); return 2; }
    const std::string command = argv[1];
    const std::filesystem::path path = argv[2];
    leng::RunConfig config;
    if (argc == 4) config.generations = std::stoi(argv[3]);
    if (command == "--run-local-lexicon") config.seed_concept_lexicon = false;
    leng::ResearchEngine engine(config);
    if (command == "--resume" || command == "--report" || command == "--communication" || command == "--evolution" || command == "--population" || command == "--human-lab" || command == "--lexicon" || command == "--concept-communication" || command == "--productive-grammar" || command == "--phonotactics") engine.load(path);
    if (command == "--run" || command == "--run-local-lexicon" || command == "--resume") { engine.run(); engine.save(path); }
    if (command != "--run" && command != "--run-local-lexicon" && command != "--resume" && command != "--report" && command != "--communication" && command != "--evolution" && command != "--population" && command != "--human-lab" && command != "--lexicon" && command != "--concept-communication" && command != "--productive-grammar" && command != "--phonotactics") { usage(); return 2; }
    if (command == "--communication") std::cout << engine.communication_trace() << '\n';
    else if (command == "--evolution") std::cout << engine.evolution_trace() << '\n';
    else if (command == "--population") std::cout << engine.population_trace() << '\n';
    else if (command == "--human-lab") std::cout << engine.human_lab_trace() << '\n';
    else if (command == "--lexicon") std::cout << engine.lexicon_trace() << '\n';
    else if (command == "--concept-communication") std::cout << engine.concept_communication_trace() << '\n';
    else if (command == "--productive-grammar") std::cout << engine.productive_grammar_trace() << '\n';
    else if (command == "--phonotactics") std::cout << engine.phonotactic_trace() << '\n';
    else std::cout << engine.report() << "\n\n" << engine.export_language() << '\n';
    return 0;
  } catch (const std::exception& e) {
    std::cerr << "FATAL: " << e.what() << '\n';
    return 1;
  }
}

========== END FILE ==========

========== FILE: Lunari-Developer/src/user_ui.cpp ==========

#include "leng/engine.hpp"

#include <windows.h>

#include <algorithm>
#include <atomic>
#include <filesystem>
#include <mutex>
#include <memory>
#include <sstream>
#include <string>
#include <thread>

namespace {
constexpr int kLearn = 1001;
constexpr int kToggleUiLanguage = 1002;
constexpr int kTranslateToLanguage = 1003;
constexpr int kTranslateBack = 1004;
constexpr int kDictionarySearch = 1005;
constexpr int kLearnWikidata = 1006;
constexpr int kStopLearning = 1007;
constexpr int kTranslateMeaning = 1008;
constexpr int kTitle = 2001;
constexpr int kStatus = 2002;
constexpr int kSourceLabel = 2003;
constexpr int kSource = 2004;
constexpr int kResultLabel = 2005;
constexpr int kResult = 2006;
constexpr int kDictionaryLabel = 2007;
constexpr int kDictionaryQuery = 2008;
constexpr int kDictionaryResult = 2009;

std::unique_ptr<leng::ResearchEngine> engine;
std::filesystem::path state_path;
std::filesystem::path project_root;
bool german_ui = true;
bool word_list_only_state = false;
std::thread local_learning_thread;
std::atomic_bool local_learning{false};
std::atomic_bool stop_local_learning{false};
std::mutex learning_status_mutex;
std::string learning_status;
constexpr UINT kLearningProgress = WM_APP + 1;
constexpr UINT kLearningFinished = WM_APP + 2;

std::wstring utf8_to_wide(const std::string& value) {
  if (value.empty()) return {};
  const int size = MultiByteToWideChar(CP_UTF8, 0, value.data(), static_cast<int>(value.size()), nullptr, 0);
  std::wstring result(size, L'\0');
  MultiByteToWideChar(CP_UTF8, 0, value.data(), static_cast<int>(value.size()), result.data(), size);
  return result;
}

std::string wide_to_utf8(const std::wstring& value) {
  if (value.empty()) return {};
  const int size = WideCharToMultiByte(CP_UTF8, 0, value.data(), static_cast<int>(value.size()), nullptr, 0, nullptr, nullptr);
  std::string result(size, '\0');
  WideCharToMultiByte(CP_UTF8, 0, value.data(), static_cast<int>(value.size()), result.data(), size, nullptr, nullptr);
  return result;
}

const wchar_t* text(const wchar_t* de, const wchar_t* en) { return german_ui ? de : en; }

void set_text(HWND hwnd, int id, const std::wstring& value) { SetWindowTextW(GetDlgItem(hwnd, id), value.c_str()); }
std::string control_text(HWND hwnd, int id) {
  const int length = GetWindowTextLengthW(GetDlgItem(hwnd, id));
  std::wstring value(static_cast<std::size_t>(length) + 1, L'\0');
  GetWindowTextW(GetDlgItem(hwnd, id), value.data(), length + 1);
  value.resize(static_cast<std::size_t>(length));
  return wide_to_utf8(value);
}

std::filesystem::path executable_project_root() {
  wchar_t buffer[MAX_PATH]{};
  GetModuleFileNameW(nullptr, buffer, MAX_PATH);
  return std::filesystem::path(buffer).parent_path().parent_path();
}

std::filesystem::path preferred_state(const std::filesystem::path& root) {
  for (const auto& file : {"german-wordlist-only.leng", "wikidata-100k.leng", "wikidata-milestone-10000.leng", "wikidata-milestone-1000.leng", "research-demo.leng"}) {
    const auto candidate = root / file;
    if (std::filesystem::exists(candidate)) return candidate;
  }
  throw std::runtime_error("no bundled .leng research state found");
}

std::string extract_block(const std::string& report, const std::string& header, const std::string& footer) {
  const std::size_t start = report.find(header);
  if (start == std::string::npos) return report;
  const std::size_t value_start = start + header.size();
  const std::size_t end = report.find(footer, value_start);
  return report.substr(value_start, end == std::string::npos ? std::string::npos : end - value_start);
}

void refresh_labels(HWND hwnd) {
  SetWindowTextW(hwnd, text(L"LINGUISTISCHE EMERGENZ — Lernen & Übersetzen", L"LINGUISTIC EMERGENCE — Learn & Translate"));
  set_text(hwnd, kTitle, text(L"Deine entstehende Sprache", L"Your emerging language"));
  set_text(hwnd, kLearn, text(L"Deutsche Wörter lernen", L"Learn German words"));
  set_text(hwnd, kLearnWikidata, text(L"Wikidata lernen", L"Learn Wikidata"));
  set_text(hwnd, kStopLearning, text(L"Lernen pausieren", L"Pause learning"));
  set_text(hwnd, kToggleUiLanguage, german_ui ? L"English" : L"Deutsch");
  set_text(hwnd, kSourceLabel, text(L"Text oder Kernbedeutung eingeben", L"Enter text or core meaning"));
  set_text(hwnd, kResultLabel, text(L"Neue Sprache", L"New language"));
  set_text(hwnd, kTranslateToLanguage, text(L"Text kodieren →", L"Encode text →"));
  set_text(hwnd, kTranslateMeaning, text(L"Bedeutung → Lunari", L"Meaning → Lunari"));
  set_text(hwnd, kTranslateBack, text(L"← Dekodieren", L"← Decode"));
  set_text(hwnd, kDictionaryLabel, text(L"Wörterbuch durchsuchen", L"Search dictionary"));
  set_text(hwnd, kDictionarySearch, text(L"Suchen", L"Search"));
}

void refresh_status(HWND hwnd, const std::string& extra = {}) {
  const std::string loaded = german_ui ? "Geladener Wissensstand: " : "Loaded knowledge state: ";
  std::string content = loaded + state_path.filename().string() + "\n" + engine->lexicon_status();
  if (!extra.empty()) content = extra + "\n\n" + content;
  set_text(hwnd, kStatus, utf8_to_wide(content));
}

void layout(HWND hwnd) {
  RECT client{}; GetClientRect(hwnd, &client);
  const int margin = 18, gap = 12, button_height = 34;
  const int client_width = static_cast<int>(client.right - client.left);
  const int client_height = static_cast<int>(client.bottom - client.top);
  const int width = std::max(0, client_width - margin * 2);
  const int column = std::max(220, (width - gap) / 2);
  const int right_x = margin + column + gap;
  const int top = margin;
  MoveWindow(GetDlgItem(hwnd, kTitle), margin, top, std::max(170, width - 665), 34, TRUE);
  MoveWindow(GetDlgItem(hwnd, kLearn), std::max(margin, client_width - 625), top, 155, button_height, TRUE);
  MoveWindow(GetDlgItem(hwnd, kLearnWikidata), std::max(margin, client_width - 460), top, 145, button_height, TRUE);
  MoveWindow(GetDlgItem(hwnd, kStopLearning), std::max(margin, client_width - 305), top, 150, button_height, TRUE);
  MoveWindow(GetDlgItem(hwnd, kToggleUiLanguage), std::max(margin, client_width - 145), top, 127, button_height, TRUE);
  MoveWindow(GetDlgItem(hwnd, kStatus), margin, top + 45, width, 88, TRUE);
  const int editor_top = top + 147;
  MoveWindow(GetDlgItem(hwnd, kSourceLabel), margin, editor_top, column, 22, TRUE);
  MoveWindow(GetDlgItem(hwnd, kResultLabel), right_x, editor_top, std::max(0, client_width - right_x - margin), 22, TRUE);
  const int editor_height = std::max(105, (client_height - editor_top - 235) / 2);
  MoveWindow(GetDlgItem(hwnd, kSource), margin, editor_top + 25, column, editor_height, TRUE);
  MoveWindow(GetDlgItem(hwnd, kResult), right_x, editor_top + 25, std::max(0, client_width - right_x - margin), editor_height, TRUE);
  const int action_y = editor_top + 25 + editor_height + 10;
  const int semantic_width = std::max(120, column / 2 - gap / 2);
  MoveWindow(GetDlgItem(hwnd, kTranslateToLanguage), margin, action_y, semantic_width, button_height, TRUE);
  MoveWindow(GetDlgItem(hwnd, kTranslateMeaning), margin + semantic_width + gap, action_y, std::max(100, column - semantic_width - gap), button_height, TRUE);
  MoveWindow(GetDlgItem(hwnd, kTranslateBack), right_x, action_y, std::max(0, client_width - right_x - margin), button_height, TRUE);
  const int dictionary_y = action_y + button_height + 18;
  MoveWindow(GetDlgItem(hwnd, kDictionaryLabel), margin, dictionary_y, 240, 22, TRUE);
  MoveWindow(GetDlgItem(hwnd, kDictionaryQuery), margin, dictionary_y + 25, std::max(160, width - 130), 28, TRUE);
  MoveWindow(GetDlgItem(hwnd, kDictionarySearch), std::max(margin, client_width - 120), dictionary_y + 25, 102, 28, TRUE);
  MoveWindow(GetDlgItem(hwnd, kDictionaryResult), margin, dictionary_y + 63, width, std::max(90, client_height - dictionary_y - 80), TRUE);
}

void initialize(HWND hwnd) {
  project_root = executable_project_root();
  state_path = preferred_state(project_root);
  word_list_only_state = state_path.filename() == "german-wordlist-only.leng";
  engine = std::make_unique<leng::ResearchEngine>();
  engine->load(state_path);
  refresh_labels(hwnd);
  ShowWindow(GetDlgItem(hwnd, kLearnWikidata), word_list_only_state ? SW_HIDE : SW_SHOW);
  refresh_status(hwnd, german_ui ? "Bereit. Übersetze eine Bedeutung oder erweitere den Wissensstand mit einem sicheren Lernschritt." : "Ready. Translate a meaning or extend the knowledge state with one safe learning step.");
}

void set_learning_controls(HWND hwnd, bool active) {
  for (const int id : {kLearn, kLearnWikidata, kToggleUiLanguage, kTranslateToLanguage, kTranslateMeaning, kTranslateBack, kDictionarySearch})
    EnableWindow(GetDlgItem(hwnd, id), (active || (word_list_only_state && id == kLearnWikidata)) ? FALSE : TRUE);
  EnableWindow(GetDlgItem(hwnd, kStopLearning), active ? TRUE : FALSE);
}

void show_learning_progress(HWND hwnd) {
  std::lock_guard lock(learning_status_mutex);
  set_text(hwnd, kStatus, utf8_to_wide(learning_status));
}

void start_local_learning(HWND hwnd) {
  if (local_learning.exchange(true)) return;
  if (local_learning_thread.joinable()) local_learning_thread.join();
  stop_local_learning = false;
  set_learning_controls(hwnd, true);
  const auto source = project_root / "words.full.list.build.txt";
  if (!std::filesystem::exists(source)) { local_learning = false; set_learning_controls(hwnd, false); throw std::runtime_error("user-provided German word list is not available"); }
  {
    std::lock_guard lock(learning_status_mutex);
    learning_status = german_ui ? "Lokale Wortliste wird im Hintergrund gelernt …" : "Learning local word list in the background …";
  }
  show_learning_progress(hwnd);
  local_learning_thread = std::thread([hwnd, source] {
    try {
      std::size_t total_created{};
      for (;;) {
        const auto progress = engine->import_german_word_list_batch(source, 50000);
        engine->save(state_path);
        total_created += progress.concepts_created;
        {
          std::lock_guard lock(learning_status_mutex);
          std::ostringstream out;
          out << (german_ui ? "Lokale Wortliste lernt im Hintergrund\n" : "Local word list learning in background\n")
              << (german_ui ? "Verarbeitete Zeilen: " : "Processed lines: ") << progress.source_line
              << "\n" << (german_ui ? "Neue Wortformen in diesem Lauf: " : "New word forms this run: ") << total_created
              << "\n" << (german_ui ? "Checkpoint wurde gespeichert. " : "Checkpoint saved. ")
              << (progress.complete ? (german_ui ? "Wortliste vollständig verarbeitet." : "Word list fully processed.")
                                    : (german_ui ? "Lernen läuft weiter; Pausieren speichert am nächsten Checkpoint." : "Learning continues; Pause saves at the next checkpoint."));
          learning_status = out.str();
        }
        PostMessageW(hwnd, kLearningProgress, 0, 0);
        if (progress.complete || stop_local_learning.load()) break;
      }
    } catch (const std::exception& error) {
      std::lock_guard lock(learning_status_mutex);
      learning_status = std::string(german_ui ? "Lokales Lernen angehalten: " : "Local learning stopped: ") + error.what();
    }
    local_learning = false;
    PostMessageW(hwnd, kLearningFinished, 0, 0);
  });
}

LRESULT CALLBACK window_proc(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam) {
  if (message == kLearningProgress) { show_learning_progress(hwnd); return 0; }
  if (message == kLearningFinished) {
    if (local_learning_thread.joinable()) local_learning_thread.join();
    set_learning_controls(hwnd, false); show_learning_progress(hwnd); return 0;
  }
  if (message == WM_SIZE) { layout(hwnd); return 0; }
  if (message == WM_COMMAND) {
    const int id = LOWORD(wparam);
    try {
      if (id == kStopLearning) { stop_local_learning = true; return 0; }
      if (local_learning.load()) return 0;
      if (id == kToggleUiLanguage) { german_ui = !german_ui; refresh_labels(hwnd); refresh_status(hwnd); return 0; }
      if (id == kTranslateToLanguage) {
        const std::string report = engine->translate_text(control_text(hwnd, kSource), "constructed");
        set_text(hwnd, kResult, utf8_to_wide(extract_block(report, "Lunari text:\n", "\n-- end Lunari text --")));
        if (report.find("newly learned tokens=0") == std::string::npos) engine->save(state_path);
      }
      if (id == kTranslateMeaning) {
        const std::string report = engine->translate_meaning(control_text(hwnd, kSource), "constructed");
        set_text(hwnd, kResult, utf8_to_wide(extract_block(report, "Lunari frame:\n", "\n-- end Lunari frame --")));
      }
      if (id == kTranslateBack) {
        const std::string target = german_ui ? "de" : "en";
        try {
          const std::string report = engine->translate_meaning(control_text(hwnd, kResult), target);
          set_text(hwnd, kSource, utf8_to_wide(report));
        } catch (const std::invalid_argument&) {
          const std::string report = engine->translate_text(control_text(hwnd, kResult), target);
          set_text(hwnd, kSource, utf8_to_wide(extract_block(report, german_ui ? "Deutsch text:\n" : "English text:\n", "\n-- end translated text --")));
        }
      }
      if (id == kDictionarySearch) set_text(hwnd, kDictionaryResult, utf8_to_wide(engine->dictionary_page(control_text(hwnd, kDictionaryQuery), 0)));
      if (id == kLearn) {
        start_local_learning(hwnd);
      }
      if (id == kLearnWikidata) {
        const auto source = project_root / "wikidata-100k.tsv";
        if (!std::filesystem::exists(source)) throw std::runtime_error("controlled Wikidata source is not available");
        const auto intake = engine->import_concepts_batch(source, 1000);
        const auto validated = engine->validate_concepts(1000);
        engine->save(state_path);
        std::ostringstream result;
        result << (german_ui ? "Lernschritt gespeichert: " : "Learning step saved: ")
               << intake.concepts_created << (german_ui ? " neue Konzepte, " : " new concepts, ")
               << intake.concepts_enriched << (german_ui ? " ergänzt, " : " enriched, ")
               << validated << (german_ui ? " validiert." : " validated.");
        refresh_status(hwnd, result.str());
      }
    } catch (const std::exception& error) {
      const std::string prefix = german_ui ? "Vorgang fehlgeschlagen: " : "Operation failed: ";
      refresh_status(hwnd, prefix + error.what());
    }
    return 0;
  }
  if (message == WM_DESTROY) { stop_local_learning = true; if (local_learning_thread.joinable()) local_learning_thread.join(); PostQuitMessage(0); return 0; }
  return DefWindowProcW(hwnd, message, wparam, lparam);
}
}  // namespace

int WINAPI WinMain(HINSTANCE instance, HINSTANCE, LPSTR, int show) {
  const wchar_t klass[] = L"LinguistischeEmergenzUser";
  WNDCLASSW wc{}; wc.lpfnWndProc = window_proc; wc.hInstance = instance; wc.lpszClassName = klass;
  wc.hCursor = LoadCursor(nullptr, IDC_ARROW); wc.hbrBackground = reinterpret_cast<HBRUSH>(COLOR_WINDOW + 1);
  if (!RegisterClassW(&wc)) return 1;
  HWND hwnd = CreateWindowExW(0, klass, L"LINGUISTISCHE EMERGENZ — Lernen & Übersetzen",
                              WS_OVERLAPPEDWINDOW | WS_VISIBLE, CW_USEDEFAULT, CW_USEDEFAULT, 1000, 740,
                              nullptr, nullptr, instance, nullptr);
  if (!hwnd) return 1;
  const DWORD static_style = WS_CHILD | WS_VISIBLE;
  CreateWindowW(L"STATIC", L"", static_style, 0, 0, 0, 0, hwnd, reinterpret_cast<HMENU>(kTitle), instance, nullptr);
  CreateWindowW(L"BUTTON", L"", WS_CHILD | WS_VISIBLE, 0, 0, 0, 0, hwnd, reinterpret_cast<HMENU>(kLearn), instance, nullptr);
  CreateWindowW(L"BUTTON", L"", WS_CHILD | WS_VISIBLE, 0, 0, 0, 0, hwnd, reinterpret_cast<HMENU>(kLearnWikidata), instance, nullptr);
  CreateWindowW(L"BUTTON", L"", WS_CHILD | WS_VISIBLE, 0, 0, 0, 0, hwnd, reinterpret_cast<HMENU>(kStopLearning), instance, nullptr);
  CreateWindowW(L"BUTTON", L"", WS_CHILD | WS_VISIBLE, 0, 0, 0, 0, hwnd, reinterpret_cast<HMENU>(kToggleUiLanguage), instance, nullptr);
  CreateWindowW(L"EDIT", L"", WS_CHILD | WS_VISIBLE | WS_BORDER | ES_MULTILINE | ES_READONLY | WS_VSCROLL, 0, 0, 0, 0, hwnd, reinterpret_cast<HMENU>(kStatus), instance, nullptr);
  CreateWindowW(L"STATIC", L"", static_style, 0, 0, 0, 0, hwnd, reinterpret_cast<HMENU>(kSourceLabel), instance, nullptr);
  CreateWindowW(L"EDIT", L"", WS_CHILD | WS_VISIBLE | WS_BORDER | ES_MULTILINE | ES_AUTOVSCROLL | WS_VSCROLL, 0, 0, 0, 0, hwnd, reinterpret_cast<HMENU>(kSource), instance, nullptr);
  CreateWindowW(L"STATIC", L"", static_style, 0, 0, 0, 0, hwnd, reinterpret_cast<HMENU>(kResultLabel), instance, nullptr);
  CreateWindowW(L"EDIT", L"", WS_CHILD | WS_VISIBLE | WS_BORDER | ES_MULTILINE | ES_READONLY | ES_AUTOVSCROLL | WS_VSCROLL, 0, 0, 0, 0, hwnd, reinterpret_cast<HMENU>(kResult), instance, nullptr);
  CreateWindowW(L"BUTTON", L"", WS_CHILD | WS_VISIBLE, 0, 0, 0, 0, hwnd, reinterpret_cast<HMENU>(kTranslateToLanguage), instance, nullptr);
  CreateWindowW(L"BUTTON", L"", WS_CHILD | WS_VISIBLE, 0, 0, 0, 0, hwnd, reinterpret_cast<HMENU>(kTranslateMeaning), instance, nullptr);
  CreateWindowW(L"BUTTON", L"", WS_CHILD | WS_VISIBLE, 0, 0, 0, 0, hwnd, reinterpret_cast<HMENU>(kTranslateBack), instance, nullptr);
  CreateWindowW(L"STATIC", L"", static_style, 0, 0, 0, 0, hwnd, reinterpret_cast<HMENU>(kDictionaryLabel), instance, nullptr);
  CreateWindowW(L"EDIT", L"", WS_CHILD | WS_VISIBLE | WS_BORDER | ES_AUTOHSCROLL, 0, 0, 0, 0, hwnd, reinterpret_cast<HMENU>(kDictionaryQuery), instance, nullptr);
  CreateWindowW(L"BUTTON", L"", WS_CHILD | WS_VISIBLE, 0, 0, 0, 0, hwnd, reinterpret_cast<HMENU>(kDictionarySearch), instance, nullptr);
  CreateWindowW(L"EDIT", L"", WS_CHILD | WS_VISIBLE | WS_BORDER | ES_MULTILINE | ES_READONLY | ES_AUTOVSCROLL | WS_VSCROLL, 0, 0, 0, 0, hwnd, reinterpret_cast<HMENU>(kDictionaryResult), instance, nullptr);
  initialize(hwnd);
  set_learning_controls(hwnd, false);
  layout(hwnd);
  ShowWindow(hwnd, show);
  MSG msg{}; while (GetMessageW(&msg, nullptr, 0, 0) > 0) { TranslateMessage(&msg); DispatchMessageW(&msg); }
  return 0;
}

========== END FILE ==========

========== FILE: Lunari-Developer/src/win32_ui.cpp ==========

#include "leng/engine.hpp"

#include <windows.h>
#include <commdlg.h>

#include <memory>
#include <filesystem>
#include <stdexcept>
#include <string>

namespace {
constexpr int kRun = 1001;
constexpr int kSave = 1002;
constexpr int kLoad = 1003;
constexpr int kLanguage = 1004;
constexpr int kQuiz = 1005;
constexpr int kCheck = 1006;
constexpr int kCommunication = 1007;
constexpr int kEvolution = 1008;
constexpr int kPopulation = 1009;
constexpr int kExport = 1010;
constexpr int kCompare = 1011;
constexpr int kLexicon = 1012;
constexpr int kImport = 1013;
constexpr int kConceptCommunication = 1014;
constexpr int kLexiconStatus = 1015;
constexpr int kSearchLexicon = 1016;
constexpr int kLexiconBenchmark = 1017;
constexpr int kTranslateGerman = 1018;
constexpr int kTranslateEnglish = 1019;
constexpr int kTranslateConstructed = 1020;
constexpr int kToggleLanguage = 1021;
constexpr int kDictionary = 1022;
constexpr int kDictionaryPrevious = 1023;
constexpr int kDictionaryNext = 1024;
constexpr int kOutput = 2001;
constexpr int kAnswer = 2002;
std::unique_ptr<leng::ResearchEngine> engine;
leng::HumanExercise exercise;
std::filesystem::path state_file{"leng-ui-state.leng"};
bool german_ui = true;
std::string dictionary_query;
std::size_t dictionary_page_index{};

const wchar_t* ui_text(const wchar_t* german, const wchar_t* english) {
  return german_ui ? german : english;
}

void set_control_text(HWND hwnd, int id, const wchar_t* german, const wchar_t* english) {
  SetWindowTextW(GetDlgItem(hwnd, id), ui_text(german, english));
}

void refresh_ui_language(HWND hwnd) {
  SetWindowTextW(hwnd, ui_text(L"LINGUISTISCHE EMERGENZ NG — Forschungsoberfläche",
                               L"LINGUISTIC EMERGENCE NG — Research Interface"));
  set_control_text(hwnd, kRun, L"Generation ausführen", L"Run generation");
  set_control_text(hwnd, kSave, L"Zustand speichern", L"Save state");
  set_control_text(hwnd, kLoad, L"Zustand laden", L"Load state");
  set_control_text(hwnd, kLanguage, L"Sprachlabor", L"Language lab");
  set_control_text(hwnd, kQuiz, L"Lernübung", L"Human exercise");
  set_control_text(hwnd, kCheck, L"Antwort prüfen", L"Check answer");
  set_control_text(hwnd, kCommunication, L"Kommunikation", L"Communication");
  set_control_text(hwnd, kEvolution, L"Evolution", L"Evolution");
  set_control_text(hwnd, kPopulation, L"Population", L"Population");
  set_control_text(hwnd, kExport, L"Export", L"Export");
  set_control_text(hwnd, kCompare, L"Zustand vergleichen", L"Compare typed state");
  set_control_text(hwnd, kLexicon, L"Lexikonlabor", L"Lexicon lab");
  set_control_text(hwnd, kImport, L"TSV importieren...", L"Import TSV...");
  set_control_text(hwnd, kConceptCommunication, L"Konzeptphrase", L"Concept phrase");
  set_control_text(hwnd, kLexiconStatus, L"Lexikonstatus", L"Lexicon status");
  set_control_text(hwnd, kSearchLexicon, L"Eingabe suchen", L"Search input");
  set_control_text(hwnd, kLexiconBenchmark, L"Messung", L"Benchmark");
  set_control_text(hwnd, kTranslateGerman, L"Übersetzen DE", L"Translate DE");
  set_control_text(hwnd, kTranslateEnglish, L"Übersetzen EN", L"Translate EN");
  set_control_text(hwnd, kTranslateConstructed, L"In Kunstsprache", L"To language");
  set_control_text(hwnd, kToggleLanguage, german_ui ? L"English" : L"Deutsch", german_ui ? L"English" : L"Deutsch");
  set_control_text(hwnd, kDictionary, L"Wörterbuch", L"Dictionary");
  set_control_text(hwnd, kDictionaryPrevious, L"← Seite", L"← Page");
  set_control_text(hwnd, kDictionaryNext, L"Seite →", L"Page →");
}

void set_output(HWND hwnd, const std::string& text) {
  std::string windows_text;
  windows_text.reserve(text.size() + 32);
  for (const char character : text) {
    if (character == '\n' && (windows_text.empty() || windows_text.back() != '\r')) windows_text += '\r';
    windows_text += character;
  }
  SetWindowTextA(GetDlgItem(hwnd, kOutput), windows_text.c_str());
}
std::string answer(HWND hwnd) { char buffer[512]{}; GetWindowTextA(GetDlgItem(hwnd, kAnswer), buffer, static_cast<int>(sizeof(buffer))); return buffer; }
std::string choose_tsv(HWND hwnd) {
  char path[MAX_PATH]{};
  OPENFILENAMEA dialog{};
  dialog.lStructSize = sizeof(dialog);
  dialog.hwndOwner = hwnd;
  dialog.lpstrFilter = "Concept TSV (*.tsv)\0*.tsv\0All files (*.*)\0*.*\0";
  dialog.lpstrFile = path;
  dialog.nMaxFile = static_cast<DWORD>(sizeof(path));
  dialog.Flags = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST;
  return GetOpenFileNameA(&dialog) ? std::string(path) : std::string{};
}
std::string choose_state(HWND hwnd) {
  char path[MAX_PATH]{};
  OPENFILENAMEA dialog{};
  dialog.lStructSize = sizeof(dialog);
  dialog.hwndOwner = hwnd;
  dialog.lpstrFilter = "Research state (*.leng)\0*.leng\0All files (*.*)\0*.*\0";
  dialog.lpstrFile = path;
  dialog.nMaxFile = static_cast<DWORD>(sizeof(path));
  dialog.Flags = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST;
  return GetOpenFileNameA(&dialog) ? std::string(path) : std::string{};
}
void ensure_engine() { if (!engine) { leng::RunConfig c; c.generations = 1; engine = std::make_unique<leng::ResearchEngine>(c); } }
void show_status(HWND hwnd, const std::string& tail = {}) { ensure_engine(); set_output(hwnd, engine->report() + (tail.empty() ? "" : "\r\n\r\n" + tail)); }

LRESULT CALLBACK window_proc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) {
  if (msg == WM_COMMAND) {
      const int id = LOWORD(wparam);
    try {
      if (id == kToggleLanguage) { german_ui = !german_ui; refresh_ui_language(hwnd); return 0; }
      if (id == kRun) { ensure_engine(); engine->run(); show_status(hwnd, "Experiment completed using actual engine state."); }
      if (id == kSave) { ensure_engine(); engine->save(state_file); show_status(hwnd, "Saved to " + state_file.string() + "."); }
      if (id == kLoad) {
        ensure_engine(); std::string requested = answer(hwnd);
        if (requested.empty()) { requested = choose_state(hwnd); if (requested.empty()) return 0; SetWindowTextA(GetDlgItem(hwnd, kAnswer), requested.c_str()); }
        state_file = requested;
        engine->load(state_file);
        show_status(hwnd, "Loaded " + state_file.string() + ".");
      }
      if (id == kLanguage) { ensure_engine(); set_output(hwnd, engine->export_language()); }
      if (id == kLexicon) { ensure_engine(); set_output(hwnd, engine->lexicon_trace()); }
      if (id == kLexiconStatus) { ensure_engine(); set_output(hwnd, engine->lexicon_status()); }
      if (id == kSearchLexicon) { ensure_engine(); set_output(hwnd, engine->search_lexicon(answer(hwnd))); }
      if (id == kDictionary) { ensure_engine(); dictionary_query = answer(hwnd); dictionary_page_index = 0; set_output(hwnd, engine->dictionary_page(dictionary_query, dictionary_page_index)); }
      if (id == kDictionaryPrevious) { ensure_engine(); if (dictionary_page_index > 0) --dictionary_page_index; set_output(hwnd, engine->dictionary_page(dictionary_query, dictionary_page_index)); }
      if (id == kDictionaryNext) { ensure_engine(); ++dictionary_page_index; set_output(hwnd, engine->dictionary_page(dictionary_query, dictionary_page_index)); }
      if (id == kLexiconBenchmark) { ensure_engine(); set_output(hwnd, engine->lexicon_benchmark()); }
      if (id == kTranslateGerman) { ensure_engine(); set_output(hwnd, engine->translate(answer(hwnd), "de")); }
      if (id == kTranslateEnglish) { ensure_engine(); set_output(hwnd, engine->translate(answer(hwnd), "en")); }
      if (id == kTranslateConstructed) { ensure_engine(); set_output(hwnd, engine->translate(answer(hwnd), "constructed")); }
      if (id == kImport) { ensure_engine(); std::string import_path = answer(hwnd); if (import_path.empty()) { import_path = choose_tsv(hwnd); if (import_path.empty()) return 0; SetWindowTextA(GetDlgItem(hwnd, kAnswer), import_path.c_str()); } const auto imported = engine->import_concepts(import_path); show_status(hwnd, "Imported " + std::to_string(imported) + " concepts. Press Save state to persist them."); }
      if (id == kQuiz) { ensure_engine(); exercise = engine->create_human_exercise(); SetWindowTextA(GetDlgItem(hwnd, kAnswer), ""); show_status(hwnd, exercise.prompt + "\r\nEnter the requested answer below and press Check answer."); }
      if (id == kCheck) { ensure_engine(); const bool correct = engine->check_human_exercise(exercise, answer(hwnd)); set_output(hwnd, std::string(correct ? "Human language lab: correct full-statement answer recorded.\r\n\r\n" : "Human language lab: answer not correct; review Language and retry.\r\n\r\n") + engine->human_lab_trace()); }
      if (id == kCommunication) { ensure_engine(); set_output(hwnd, engine->communication_trace()); }
      if (id == kConceptCommunication) { ensure_engine(); set_output(hwnd, engine->concept_communication_trace()); }
      if (id == kEvolution) { ensure_engine(); set_output(hwnd, engine->evolution_trace()); }
      if (id == kPopulation) { ensure_engine(); set_output(hwnd, engine->population_trace()); }
      if (id == kExport) { ensure_engine(); engine->export_results("leng-export"); show_status(hwnd, "Exported language, report, communication, population and Pareto archive to leng-export."); }
      if (id == kCompare) {
        ensure_engine();
        const std::string reference = answer(hwnd);
        if (reference.empty()) throw std::invalid_argument("enter a saved .leng path before Compare");
        const std::filesystem::path current = "leng-ui-current-compare.leng";
        engine->save(current);
        try { set_output(hwnd, leng::compare_research_states(current, reference)); }
        catch (...) { std::filesystem::remove(current); throw; }
        std::filesystem::remove(current);
      }
    } catch (const std::exception& ex) {
      const std::string prefix = german_ui ? "Vorgang fehlgeschlagen: " : "Operation failed: ";
      const std::string help = id == kLoad
          ? (german_ui ? "\n\nBitte wähle eine vorhandene .leng-Forschungsdatei aus. Bei leerem Eingabefeld öffnet Zustands laden die Dateiauswahl."
                       : "\n\nChoose an existing .leng research-state file. With an empty input field, Load state opens the file chooser.")
          : "";
      set_output(hwnd, prefix + ex.what() + help);
    }
    return 0;
  }
  if (msg == WM_DESTROY) { PostQuitMessage(0); return 0; }
  return DefWindowProcW(hwnd, msg, wparam, lparam);
}
}

int WINAPI WinMain(HINSTANCE instance, HINSTANCE, LPSTR, int show) {
  const wchar_t klass[] = L"LinguistischeEmergenzNG";
  WNDCLASSW wc{}; wc.lpfnWndProc = window_proc; wc.hInstance = instance; wc.lpszClassName = klass; wc.hCursor = LoadCursor(nullptr, IDC_ARROW); wc.hbrBackground = reinterpret_cast<HBRUSH>(COLOR_WINDOW + 1);
  if (!RegisterClassW(&wc)) return 1;
  HWND hwnd = CreateWindowExW(0, klass, L"LINGUISTISCHE EMERGENZ NG — Forschungsoberfläche", WS_OVERLAPPEDWINDOW | WS_VISIBLE, CW_USEDEFAULT, CW_USEDEFAULT, 920, 660, nullptr, nullptr, instance, nullptr);
  if (!hwnd) return 1;
  CreateWindowW(L"BUTTON", L"Run generation", WS_CHILD | WS_VISIBLE, 15, 15, 125, 30, hwnd, reinterpret_cast<HMENU>(kRun), instance, nullptr);
  CreateWindowW(L"BUTTON", L"Save state", WS_CHILD | WS_VISIBLE, 150, 15, 100, 30, hwnd, reinterpret_cast<HMENU>(kSave), instance, nullptr);
  CreateWindowW(L"BUTTON", L"Load state", WS_CHILD | WS_VISIBLE, 260, 15, 100, 30, hwnd, reinterpret_cast<HMENU>(kLoad), instance, nullptr);
  CreateWindowW(L"BUTTON", L"Language lab", WS_CHILD | WS_VISIBLE, 370, 15, 115, 30, hwnd, reinterpret_cast<HMENU>(kLanguage), instance, nullptr);
  CreateWindowW(L"BUTTON", L"Human exercise", WS_CHILD | WS_VISIBLE, 495, 15, 125, 30, hwnd, reinterpret_cast<HMENU>(kQuiz), instance, nullptr);
  CreateWindowW(L"BUTTON", L"Communication", WS_CHILD | WS_VISIBLE, 630, 15, 115, 30, hwnd, reinterpret_cast<HMENU>(kCommunication), instance, nullptr);
  CreateWindowW(L"BUTTON", L"Evolution", WS_CHILD | WS_VISIBLE, 755, 15, 100, 30, hwnd, reinterpret_cast<HMENU>(kEvolution), instance, nullptr);
  CreateWindowW(L"BUTTON", L"Lexicon lab", WS_CHILD | WS_VISIBLE, 15, 88, 115, 28, hwnd, reinterpret_cast<HMENU>(kLexicon), instance, nullptr);
  CreateWindowW(L"EDIT", L"", WS_CHILD | WS_VISIBLE | WS_BORDER | ES_AUTOHSCROLL, 15, 55, 480, 26, hwnd, reinterpret_cast<HMENU>(kAnswer), instance, nullptr);
  CreateWindowW(L"BUTTON", L"Check answer", WS_CHILD | WS_VISIBLE, 505, 54, 115, 28, hwnd, reinterpret_cast<HMENU>(kCheck), instance, nullptr);
  CreateWindowW(L"BUTTON", L"Population", WS_CHILD | WS_VISIBLE, 630, 54, 115, 28, hwnd, reinterpret_cast<HMENU>(kPopulation), instance, nullptr);
  CreateWindowW(L"BUTTON", L"Export", WS_CHILD | WS_VISIBLE, 755, 54, 100, 28, hwnd, reinterpret_cast<HMENU>(kExport), instance, nullptr);
  CreateWindowW(L"BUTTON", L"Compare typed state", WS_CHILD | WS_VISIBLE, 140, 88, 160, 28, hwnd, reinterpret_cast<HMENU>(kCompare), instance, nullptr);
  CreateWindowW(L"BUTTON", L"Import TSV...", WS_CHILD | WS_VISIBLE, 310, 88, 115, 28, hwnd, reinterpret_cast<HMENU>(kImport), instance, nullptr);
  CreateWindowW(L"BUTTON", L"Concept phrase", WS_CHILD | WS_VISIBLE, 435, 88, 125, 28, hwnd, reinterpret_cast<HMENU>(kConceptCommunication), instance, nullptr);
  CreateWindowW(L"BUTTON", L"Lexicon status", WS_CHILD | WS_VISIBLE, 570, 88, 125, 28, hwnd, reinterpret_cast<HMENU>(kLexiconStatus), instance, nullptr);
  CreateWindowW(L"BUTTON", L"Search input", WS_CHILD | WS_VISIBLE, 705, 88, 115, 28, hwnd, reinterpret_cast<HMENU>(kSearchLexicon), instance, nullptr);
  CreateWindowW(L"BUTTON", L"Benchmark", WS_CHILD | WS_VISIBLE, 830, 88, 70, 28, hwnd, reinterpret_cast<HMENU>(kLexiconBenchmark), instance, nullptr);
  CreateWindowW(L"BUTTON", L"Translate DE", WS_CHILD | WS_VISIBLE, 15, 125, 115, 28, hwnd, reinterpret_cast<HMENU>(kTranslateGerman), instance, nullptr);
  CreateWindowW(L"BUTTON", L"Translate EN", WS_CHILD | WS_VISIBLE, 140, 125, 115, 28, hwnd, reinterpret_cast<HMENU>(kTranslateEnglish), instance, nullptr);
  CreateWindowW(L"BUTTON", L"To language", WS_CHILD | WS_VISIBLE, 265, 125, 115, 28, hwnd, reinterpret_cast<HMENU>(kTranslateConstructed), instance, nullptr);
  CreateWindowW(L"BUTTON", L"English", WS_CHILD | WS_VISIBLE, 390, 125, 100, 28, hwnd, reinterpret_cast<HMENU>(kToggleLanguage), instance, nullptr);
  CreateWindowW(L"BUTTON", L"Dictionary", WS_CHILD | WS_VISIBLE, 500, 125, 115, 28, hwnd, reinterpret_cast<HMENU>(kDictionary), instance, nullptr);
  CreateWindowW(L"BUTTON", L"Previous page", WS_CHILD | WS_VISIBLE, 625, 125, 100, 28, hwnd, reinterpret_cast<HMENU>(kDictionaryPrevious), instance, nullptr);
  CreateWindowW(L"BUTTON", L"Next page", WS_CHILD | WS_VISIBLE, 735, 125, 100, 28, hwnd, reinterpret_cast<HMENU>(kDictionaryNext), instance, nullptr);
  HWND output = CreateWindowW(L"EDIT", L"Run a generation to create a language candidate.", WS_CHILD | WS_VISIBLE | WS_BORDER | ES_MULTILINE | ES_AUTOVSCROLL | ES_READONLY | WS_VSCROLL, 15, 165, 870, 440, hwnd, reinterpret_cast<HMENU>(kOutput), instance, nullptr);
  SendMessageW(output, WM_SETFONT, reinterpret_cast<WPARAM>(GetStockObject(DEFAULT_GUI_FONT)), TRUE);
  refresh_ui_language(hwnd);
  ShowWindow(hwnd, show);
  MSG message{}; while (GetMessageW(&message, nullptr, 0, 0) > 0) { TranslateMessage(&message); DispatchMessageW(&message); }
  return 0;
}

========== END FILE ==========

# Dateien: 14
