# Architektur

Der CPU-Referenzpfad besteht aus einer gemeinsamen `ResearchEngine` und stellt keine versteckte Bedeutung zwischen Kommunikationspartnern bereit.

```text
Semantic task -> Sender lexicon -> message tokens -> Receiver private lexicon -> decoded meaning -> task comparison
                      ^                 |                     ^
                      |                 +-- feedback only after training trial
                      +-- evolutionary candidate / phonology
```

`Meaning` ist eine strukturierte Repräsentation mit Sprechakt, Akteur, Handlung, Objekt, Relation, Negation, Zeit, Quantität, Modalität, Wahrscheinlichkeit, Emotion, sozialer Relation, Ziel, Aussagenebene und Abstraktion. Sie wird nie als menschlicher Text verglichen. Die Nachricht enthält ausschließlich Laut-/Schreibformen; `decode` erhält keine erwartete Bedeutung.

Ein Kandidat hat eine atomare lexikalische Zuordnung und eine persistierte, mutierbare Reihenfolge der fünfzehn Felder `[act actor action object relation neg time qty modality probability emotion social goal proposition abstraction]`. Diese explizite, kleine Referenzgrammatik ist ein Ausgangspunkt für Evolutionsexperimente, keine Behauptung einer vollständigen universellen Grammatik.

Kandidaten werden nach Verständigung, semantischer Genauigkeit, Kosten, Aussprechbarkeit, Robustheit und Ausdrucksfähigkeit bewertet. Nicht dominierte Kandidaten werden im Forschungsarchiv gesichert; die Reproduktion verwendet zusätzlich eine gewichtete Auswahl.

Das Zustandformat `LENG_STATE 11` speichert Konfiguration, Generation, Kandidatenlexika, deren Grammatikreihenfolge, Metriken, Pareto-Archiv, private Reverse-Lexika und Erfolgs-/Fehlerzähler sämtlicher Agenten, den vollständigen Zustand des Zufallsgenerators, getrennte Humanlabor-Metriken sowie den offenen Konzeptgraphen. Ein Konzeptsatz enthält Bedeutung, Beziehungen, Wortbildung, Quelle, Provenienz/Lizenz, Generation und Agentenvalidierung. Es ist textuell und versioniert; unbekannte Versionen werden abgelehnt. Ältere Versionen werden bewusst migriert, ohne ein vorhandenes Experiment bei bloßem Laden zu überschreiben.

Unter Windows greift `linguistische-emergenz-ng-ui.exe` direkt auf dieselbe C++-Engine zu. Seine Bedienelemente starten Generationen, speichern/laden und vergleichen Zustände, zeigen die real erzeugte Sprache und führen menschliche Produktions- sowie Interpretations-Transferübungen aus. Das **Lexicon lab** zeigt denselben persistenten Konzeptgraphen wie der Konsolenbefehl `--lexicon`. Die Oberfläche enthält keine dekorativen Metriken.

`inspect_compute()` lädt OpenCL nur zur Laufzeit, enumeriert Plattformen und alle Geräte ohne fest codierten Index und meldet die Verfügbarkeit im Forschungsbericht. Fehlt der Loader oder ein Gerät, bleibt der CPU-Referenzpfad verwendbar. `run_reward_parity_test()` kompiliert einen OpenCL-Kernel zur parallelen Berechnung von Agenten-Belohnungswerten und vergleicht jedes Ergebnis gegen dieselbe CPU-Formel. Die Kandidatenreihung verwendet anschließend denselben GPU-Batch für die mehrzielige Nutzen-minus-Kommunikationskosten-Bewertung; bei fehlendem oder abweichendem GPU-Ergebnis greift sie nachvollziehbar auf exakt diese CPU-Formel zurück.
