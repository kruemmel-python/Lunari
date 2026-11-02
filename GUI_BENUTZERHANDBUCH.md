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
