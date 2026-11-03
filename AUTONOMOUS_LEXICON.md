# Autonomes Konzeptlexikon

Das Konzeptlexikon erweitert die Satzsemantik, ersetzt sie aber nicht. Die 15 Felder von `Meaning` bleiben eine kompakte Grammatik für reproduzierbare Kommunikationsversuche. `ConceptEntry` ist der offene Wissensraum für Dinge, Eigenschaften, Ereignisse und abgeleitete Begriffe.

## Eintragsmodell

Jeder persistierte Eintrag enthält:

- stabile `CONCEPT_*`-ID und menschlich lesbares Bedeutungslabel;
- Wortart, semantische Merkmale und explizite Beziehungen;
- Form, Wortbildungsweg und Entstehungsgeneration;
- Wissensquelle (`seeded-human-knowledge` oder `simulated-experience`);
- Prüfversuche und erfolgreiche Rekonstruktionen durch Agenten.

Der aktuelle Startbestand ist absichtlich klein und lokal kuratiert. Damit wird weder Wiktionary noch Wikidata kopiert. Der lokale UTF-8-TSV-Import nimmt pro Zeile `label`, `part_of_speech`, `semantics`, `relations`, `source` und `provenance` entgegen. Die letzten beiden Werte sind verpflichtend und werden unverändert im Forschungszustand sowie Export gespeichert. `concept-import-template.tsv` zeigt eine Wikidata/CC0-Provenienz. `search_wikidata_concepts.ps1` hilft bei der Auswahl einer eindeutigen Q-ID; `fetch_wikidata_concepts.ps1` ruft anschließend Labels, Beschreibungen und eine begrenzte Menge struktureller Kanten ab und ergänzt Quell-URL, CC0 und UTC-Abrufzeit. Die Netzaufnahme bleibt damit ein separater, überprüfbarer Vorbereitungsschritt, nicht ein verdeckter Datenabruf der Engine.

## Wortfamilien und Prüfung

Grundkonzepte erhalten aussprechbare Formen. Aus einer in simulierten Beziehungen gefundenen Verbindung entstehen Komposita mit den bekannten Bestandteilen, etwa `forest + fire` und danach `forest + fire + danger`. Der Empfänger löst eine Nachricht über seine bekannte Form-zu-Bedeutung-Zuordnung auf. Die Validierung erhält keine interne Konzept-ID als Eingabe und vergleicht nur den rekonstruierten Bedeutungsnamen.

Auch ein importierter Beziehungswert kann neue Begriffe auslösen: Verweist eine Relation auf ein schon vorhandenes Konzeptlabel, bildet die Engine vorsichtig ein `relation-compound`. So leitet die Vorlagendatenzeile `rain … can-affect:forest` den Eintrag `rain-forest` ab. Seine Provenienz führt sowohl die Ursprungs-Konzept-ID als auch die auslösende Relation auf. Unbekannte oder mehrdeutige Ziele werden übersprungen; der Import erfindet dafür kein neues Wissen.

Der Wikidata-Abruf übernimmt nur die dokumentierten Kanten `is-a`, `part-of`, `has-part`, `has-cause` und `causes`. Werden beide Endpunkte in dieselbe Engine aufgenommen, kann daraus ebenfalls ein `relation-compound` entstehen. Ein Live-Test mit den frei lizenzierten Entitäten `flood` und `rain` erzeugte aus `causes:flood` den Eintrag `rain-flood`; die abgeleitete Form und Relation bleiben im Export sichtbar. Damit ist die Aufnahme keine bloße Liste isolierter Übersetzungen, ersetzt aber keine allgemeine Ontologie- oder Wahrheitsprüfung.

Ein Eintrag gilt im aktuellen Forschungsprototyp als agentenvalidiert, wenn alle sechs unabhängigen Rekonstruktionsversuche erfolgreich sind. Das ist ein technischer Agententest, keine Aussage über menschliche Erlernbarkeit.

Zusätzlich prüft das Konzeptkommunikationslabor eine bisher nicht als Gesamtphrase gelernte Nachricht. Es sendet die Form für `forest-fire` zusammen mit der Form für `danger`; der Empfänger rekonstruiert `forest-fire danger` aus seinen bekannten Grundformen. Der Test übergibt keine `CONCEPT_*`-IDs und ist damit ein enger Nachweis für Verwendung eines neuen Konzepts im größeren Ausdruck, nicht bloß für eine einzelne Wörterbuchabfrage.

## Persistenz und Export

Das Format `LENG_STATE 11` speichert die Konzeptzeilen einschließlich Provenienz nach dem bisherigen Archiv. Frühere Zustände werden gelesen und beim nächsten Speichern um den deterministisch erzeugten anfänglichen Konzeptgraphen ergänzt. Ein Export enthält zusätzlich `evolutionary_lexicon.txt`.

Die aktuelle Implementierung ist ein überprüfbarer Kern für Wachstum und Herkunft, nicht der behauptete Zielumfang von 1.000, 10.000 oder 100.000 Konzepten. Diese Größen bleiben Forschungsziele und verlangen vor allem einen lizenzkonformen, versionierten Wissensimport sowie belastbare Skalierungs- und Humanstudien.
