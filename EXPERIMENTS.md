# Experimente

Ein Experiment ist durch Seed, Populationsgröße, Generationen sowie Trainings- und Evaluierungsrunden definiert. Der Seed wird im Zustand gespeichert. Die CLI bietet neue Läufe, Fortsetzen, Reporting und `--compare <left.leng> <right.leng>`. Der Vergleich liest ausschließlich zwei gespeicherte Zustände und zeigt Generation sowie Kommunikations-, Semantik-, Kosten-, Aussprechbarkeits- und Robustheitsdifferenzen. In der Win32-Oberfläche wird ein Lauf generationenweise ausgeführt und kann damit zwischen zwei Schritten gespeichert, geladen oder faktisch pausiert werden; `Compare typed state` vergleicht den aktuellen Stand mit dem im Eingabefeld angegebenen gespeicherten Pfad.

Messgrößen sind Kommunikations- und semantischer Erfolg auf unbekannten Kombinationen, durchschnittliche Nachrichtenbytes, geschätzte Aussprechbarkeit, Robustheit und Ausdrucksfähigkeit. Das Kommunikationslabor misst für eine tatsächlich erzeugte Nachricht zusätzlich Byteumfang, Kodier- und Dekodierzeit in Mikrosekunden sowie die Zeichen-Nutzlast des privaten Empfängerlexikons. Diese Einzelmessungen werden beim Export in `communication.txt` festgehalten; sie sind Laufzeitmessungen und keine hardwareunabhängigen Leistungsversprechen. Trainings- und Evaluierungsgeneratoren sind getrennt.

Robustheit ist keine pauschale Schätzung: Der Evaluator ersetzt in jeder Testnachricht genau ein Akteur-Token durch ein anderes gültiges Akteur-Token. Der Empfänger dekodiert die gestörte Nachricht mit seinem privaten Lexikon; anschließend wird der Anteil weiterhin korrekter semantischer Felder gemessen. Damit bleibt die Testnachricht grammatisch vollständig und die Kennzahl erfasst den fachlichen Schaden einer konkreten Übertragungsstörung.

Für wissenschaftliche Vergleiche müssen künftige Durchläufe Parameter, Hardware, CPU-/GPU-Backend und Kandidatenfamilie vollständig protokollieren.

`--export <state> <directory>` erzeugt ein portables Forschungspaket mit `language.txt`, `report.txt`, `communication.txt`, `population.txt`, `pareto_archive.csv` und `manifest.txt`. Der Export ist eine Kopie der vorhandenen Forschungsergebnisse; er verändert den geladenen Forschungszustand nicht.
