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

## Lunari-Translator über die Kommandozeile

Der texttreue Translator ist auch ohne Oberfläche verfügbar. Für normale
Wörter, Sätze, Satzzeichen und Dialoge immer `--translate-text` verwenden.
Der vierte Parameter ist der Zielkanal: `constructed` für Lunari, `de` für
Deutsch und `en` für Englisch.

```powershell
# Deutsch oder Englisch → Lunari
.\release\linguistische-emergenz-ng.exe --translate-text .\german-wordlist-only.leng constructed "Bett"

# Lunari → Deutsch
.\release\linguistische-emergenz-ng.exe --translate-text .\german-wordlist-only.leng de "Sedabiku"

# Lunari → Englisch
.\release\linguistische-emergenz-ng.exe --translate-text .\german-wordlist-only.leng en "Sedabiku"
```

Die Vorwärtsübersetzung nimmt bisher unbekannte deutsche Wortformen sicher als
neue lokale Wortform in den angegebenen `.leng`-Zustand auf und speichert ihn.
Die Rückübersetzung verändert den Zustand nicht. Für den separaten strengen
15-Felder-Bedeutungskanal gibt es `--translate-meaning`; dieser ist nicht für
freie Texte gedacht.

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
