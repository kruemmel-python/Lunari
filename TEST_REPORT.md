# Testbericht

Ausgeführt am 2026-11-01 in der bereitgestellten Windows-Umgebung:

```text
ctest --test-dir build --output-on-failure
1/1 Test #1: engine_self_test ... Passed
100% tests passed, 0 tests failed
```

Der Selbsttest führt zwei Generationen aus, prüft Fortschritt/Archiv, eine menschliche Wortzuordnungsübung, Persistenz-Roundtrip und einen Fortsetzungslauf mit explizitem zusätzlichen Generationslimit.

Ein zusätzlicher Lauf mit vier Generationen erzeugte `research-demo.leng`. Der gemeldete Spitzenkandidat erreichte im simulierten Test bisher ungesehener Bedeutungszusammensetzungen `communication_success=1` und `semantic_accuracy=1`. Das belegt nur die implementierte Agentenkommunikation unter diesem reproduzierbaren Aufgabenraum; keine GPU-Leistung und keine menschliche Erlernbarkeit wurden gemessen.

Der native Win32-Client `linguistische-emergenz-ng-ui.exe` wurde zusätzlich gestartet und blieb in einem zweisekündigen Smoke-Test aktiv. Der Prozess wurde danach gezielt beendet; dies ist ein Starttest, kein Ersatz für eine vollständige manuelle UI-Abnahme.

Nach einem gemeldeten `libstdc++-6.dll`-Einsprungpunktfehler wurde der Release auf die KR-Compiler-Toolchain umgestellt und statisch gelinkt. `release\linguistische-emergenz-ng.exe --self-test` bestand; der UI-Release startete anschließend auch mit auf Windows-Systemverzeichnisse reduziertem `PATH`. Die Release-EXE importiert keine MinGW-DLL mehr.

Die neue dynamische OpenCL-Prüfung wurde im Release ausgeführt. Sie fand die Plattformgeräte `gfx90c`, `gfx1034`, `AMD Radeon(TM) Graphics`, `AMD Radeon(TM) RX 6500M` und `Microsoft Basic Render Driver`; mindestens ein GPU-Gerät ist verfügbar. `release\linguistische-emergenz-ng.exe --compute-test` führte auf der GPU einen Kernel für 1.024 Agenten-Belohnungswerte aus und verglich ihn mit der CPU-Referenz: `GPU/CPU reward parity passed`, `max_absolute_error=0`. Die Kandidatenreihung führt denselben Kernel für den mehrzieligen Nutzen abzüglich Bytekosten als GPU-Batch aus. Fehlt ein Gerät, schlägt der Kernel fehl oder weicht er numerisch ab, bleibt exakt diese CPU-Formel aktiv.

`--communication` lieferte für die gespeicherte Sprachversion eine real erzeugte Nachricht, eine unabhängig daraus dekodierte semantische Struktur und `Task success: yes`. `--evolution` gab Kandidaten-IDs, Elternbeziehungen sowie gemessene Kommunikations-, Semantik-, Kosten- und Aussprechbarkeitsmetriken aus. Beide Ausgaben stammen aus dem geladenen Forschungszustand.

Nach der Erweiterung des Semantikmodells wurde der vorhandene `LENG_STATE 1`-Forschungszustand migrationsfähig geladen, eine weitere Generation ausgeführt und als `LENG_STATE 2` gespeichert. Der Kommunikationstest rekonstruierte alle 14 Felder, einschließlich Modalität, Wahrscheinlichkeit, Emotion, sozialer Relation, Ziel und Aussagenebene, erfolgreich.

Die persistente Agentenpopulation wurde danach ergänzt und der Referenzlauf nach `LENG_STATE 3` fortgesetzt. `--population` meldete je Agent einen privaten Wortbestand sowie getrennte Erfolgs- und Fehlerzähler; nach Generation 10 besaß Agent 1 48 gelernte Token bei 949 erfolgreichen und 11 fehlgeschlagenen Interpretationen. Der Selbsttest prüft jetzt zusätzlich, dass dieser Agentenzustand einen Speicher-/Lade-Roundtrip unverändert übersteht.

In Generation 11 wurde die Robustheitsmetrik mit kontrollierter Token-Substitution ausgeführt. Der störungsfreie Test erzielte Kommunikations- und semantische Genauigkeit `1`; bei Ersetzung eines gültigen Akteur-Tokens betrug die feldweise Rekonstruktionsrate `0.928571` (13 von 14 Feldern). Der Selbsttest lehnt eine Robustheitsmetrik von 0 oder 1 für diesen definierten Störfall ab.

Der Exportpfad wurde mit dem gespeicherten Forschungsstand geprüft. Er erzeugte `language.txt`, `report.txt`, `communication.txt`, `population.txt`, `pareto_archive.csv` und `manifest.txt`; das Manifest weist `LENG_STATE 3`, Generation 11, 12 Kandidaten und 12 Agenten aus. Der Selbsttest prüft die beiden zentralen Exportdateien zusätzlich in einem isolierten temporären Verzeichnis.

Der Zustand wurde auf `LENG_STATE 5` erweitert, um den vollständigen Zustand des Zufallsgenerators zu speichern. Der Selbsttest lädt denselben Checkpoint in zwei frische Engine-Instanzen, führt jeweils genau eine weitere Generation aus und vergleicht Sprache sowie Population bytegleich. Dieser deterministische Fortsetzungstest besteht. Nach der Korrektur des Versionslesers wurde der Referenzlauf aus `LENG_STATE 4` geladen und bis Generation 13 als `LENG_STATE 5` fortgesetzt.

Der Linux-x64-Release wurde mit der KR-bundled Zig-C++-Toolchain erzeugt. Die Datei wurde als `ELF 64-bit LSB executable, x86-64` erkannt; `release-linux/linguistische-emergenz-ng --self-test` bestand in Ubuntu unter WSL mit Exit-Code 0. Damit ist der portable Core auf Windows x64 und Linux x64 gebaut und getestet. Die native Win32-Oberfläche ist erwartungsgemäß nicht Bestandteil des Linux-Artefakts.

Der portable Android-ARM64-Core wurde mit Android NDK 27.2.12479018 für `aarch64-linux-android24` als `release-android/arm64-v8a/libleng_core.so` gebaut. `llvm-readelf` meldet `ELF64` und `Machine: AArch64`. Kein Android-Gerät oder Emulator war für einen Lauf verfügbar; ein Android-Funktionstest wird deshalb nicht behauptet.

Die KR-Release-Pipeline erzeugt außerdem `release/leng_api.dll`. Ein direkter DLL-Smoketest über die in `leng/c_api.h` definierte C-ABI erzeugte einen Engine-Handle, führte mit einer kleinen Population eine Generation aus und las anschließend `Research generation: 1` sowie `Candidates: 4` aus dem Bericht. Die exportierten Symbole `leng_engine_create`, `leng_engine_run`, `leng_engine_report` und `leng_engine_last_error` wurden im DLL-Export geprüft.

Ein frischer End-to-End-Durchlauf erzeugte `research-e2e.leng`: Drei Generationen wurden aus einem leeren Zustand ausgeführt, eine echte Sender-/Empfänger-Kommunikation mit getrennten Lexika erfolgreich rekonstruiert, der Zustand um eine vierte Generation fortgesetzt und nach `research-e2e-export` exportiert. Das Manifest hält `LENG_STATE 6`, Generation 4, zwölf Kandidaten und zwölf Agenten fest; der Export enthält Sprache, Bericht, Kommunikation, Population und Pareto-Archiv. Der Lauf meldete auf zuvor nicht verwendeten Kompositionen Kommunikations- und semantische Genauigkeit `1`, bei kontrollierter Akteur-Token-Substitution eine feldweise Robustheit von `0.928571`.

Nach Integration des GPU-Batches wurden der KR-Windows-Release, der Selbsttest und der GPU/CPU-Paritätstest erneut erfolgreich ausgeführt. Der Linux-x64-Release wurde neu gebaut und sein Selbsttest in Ubuntu-WSL bestanden. Der Android-ARM64-Core wurde neu gebaut; `llvm-readelf` bestätigt weiterhin `ELF64` und `AArch64`. Ein Android-Lauf bleibt mangels Gerät oder Emulator ungeprüft.

Die Experimentverwaltung wurde um einen zustandsbasierten Vergleich erweitert. `--compare research-demo.leng research-e2e.leng` las beide gespeicherten Forschungsstände und meldete Generation 15 gegenüber 4, unveränderte Kommunikations- und Semantikwerte von `1`, identische Robustheit `0.928571` sowie eine Kostenabweichung von `7.65` Bytes pro Nachricht. Der Selbsttest vergleicht zusätzlich denselben Checkpoint gegen sich selbst und fordert dabei eine Generationsdifferenz von 0.

Der Kandidatenzustand wurde auf `LENG_STATE 7` erweitert. Er speichert nun die jeweilige Reihenfolge der vierzehn Grammatikfelder. Ein V6-Checkpoint (`research-e2e.leng`) wurde geladen, um eine Generation fortgesetzt und als V7 gespeichert; anschließend rekonstruierte der Kommunikationstest die vollständige erwartete semantische Struktur aus einer Nachricht in dieser gespeicherten Sprachversion erfolgreich.

Der portable Kern mit der V7-Grammatikrepräsentation wurde zusätzlich unter Linux x64 neu gebaut und in Ubuntu-WSL per Selbsttest ausgeführt. Der Android-ARM64-Core wurde neu erzeugt; die Artefaktprüfung meldet weiterhin `ELF64` und `AArch64`, jedoch keinen Geräte- oder Emulatorlauf.

Der Android-Blocker wurde aufgelöst: `leng-device-smoke` wurde für `aarch64-linux-android24` gebaut, über ADB auf dem verbundenen RMX3853 (`arm64-v8a`, API 36) ausgeführt und meldete `ANDROID_NATIVE_SMOKE_PASSED`. Der native Lauf führte eine Generation mit unabhängigen Agenten aus, prüfte eine erfolgreiche Kommunikationsrekonstruktion und verifizierte Speichern/Laden auf dem Gerät.

Das Humanlabor wurde auf `LENG_STATE 8` erweitert. Es alterniert zwischen einer Produktions-Transferaufgabe und einer Interpretations-Transferaufgabe für eine zuvor ungesehene Aussage. Der Selbsttest löst beide Aufgabentypen mit ihren erwarteten Antworten, speichert die getrennten Produktions-/Interpretationszähler und prüft die Ausgabe von Fehlerquote, Wiedererkennungsrate, Produktionsrate und mittlerer Antwortzeit. Diese Ausführung ist ein Funktionstest, keine Humanstudie; der gespeicherte Referenzlauf enthält weiterhin keine Versuchspersonendaten.

Die Tri-Core-Referenz wurde nochmals bis zu ihren konkreten OpenCL-Exporten geprüft: `CC_OpenCL.c` enthält Prototyp-Updates, SubQG-/Pheromonfeld-Schritte und eine VQE/SPSA-Simulation. Keiner dieser Pfade ist eine belegte Sprachverständigungsmetrik oder Quantenhardware-Beschleunigung. Daher wird keine irreführende Tri-Core- oder Quantenoption ausgewiesen; die vorhandene Pareto-Mehrzielselektion bleibt die reproduzierbar vergleichbare Optimierung des Sprachsystems.

Das Kommunikationslabor misst jetzt zusätzlich Byteumfang, Kodier-/Dekodierzeit und private Lexikon-Nutzlast. Im exportierten Referenzlauf wurde eine tatsächlich erzeugte 100-Byte-Nachricht erfolgreich rekonstruiert; die momentane Einzelmessung lag bei 32 µs Kodierung, 35 µs Dekodierung und 1.086 Byte Empfängerlexikon-Nutzlast. Diese Werte sind ausdrücklich lauf- und hardwareabhängig.

Die vollständige Regression nach den Kommunikationsänderungen besteht: Der KR-Windows-Release und der Linux-x64-Release bestanden jeweils den Selbsttest. Der Android-Core wurde erneut mit dem vorhandenen NDK gebaut; `llvm-readelf` bestätigt `ELF64` und `AArch64`. Die WSL-Ausgabe enthielt lediglich einen geerbten, nicht auflösbaren Alt-PATH-Eintrag und keinen Testfehler.

Der dokumentierte CMake-Entwicklerbuild wurde mit MSYS2-MinGW (`GNU 16.1.0`) in `build-current` neu konfiguriert. Er baute Kern, Konsole, Win32-Oberfläche und die C-ABI-Bibliothek (`msys-leng_api.dll`); `ctest` bestand den vollständigen Selbsttest mit 1/1 erfolgreichen Tests.

Grammatikkomplexität ist nun ein aktives Minimierungsziel. Sie wird aus den Inversionen der persistierten Feldreihenfolge berechnet und in Pareto-Dominanz sowie GPU-/CPU-identischer Kandidatenreihung einbezogen. Nach einem weiteren Referenzschritt zeigte Kandidat 66 bei Generation 7 Kommunikations- und Semantikwert `1`, Robustheit `0.928571` und die regulärste mögliche Grammatikkomplexität `1`. Der KR-Selbsttest und der GPU/CPU-Paritätstest bestanden danach erneut.

Das Semantikmodell wurde auf `LENG_STATE 9` um die strukturierte Abstraktionsebene `literal`/`metaphoric` erweitert. Ein V8-Checkpoint wurde geladen, fortgesetzt und als V9 gespeichert; die Nachricht von Kandidat 74 enthielt ein eigenes Abstraktionstoken und der Empfänger rekonstruierte alle 15 Felder korrekt. Der Selbsttest sowie der GPU/CPU-Paritätstest bestanden nach der Migration.

Der autonome Lexikonkern wurde mit `LENG_STATE 10` ergänzt. Er erzeugt sieben lokale Grundkonzepte und die zwei aus Beziehungen abgeleiteten, kompositionalen Einträge `forest-fire` und `forest-fire-danger`. Der KR-Release-Selbsttest prüfte Form-Rekonstruktion durch Agenten (je `6/6`), Speichern/Laden des Konzeptgraphen, deterministische Fortsetzung und den Export `evolutionary_lexicon.txt`. Ein separater Exportlauf meldete `state_format=LENG_STATE 10` und `concepts=9`. Das ist ein technischer Kern für offene Lexikonentwicklung, kein Nachweis eines vollständigen oder extern wissensgespeisten Lexikons.

`LENG_STATE 11` ergänzt eine verpflichtende Quellen- und Provenienzangabe für importierte Konzepte. Der KR-Selbsttest importiert eine TSV-Fixture mit den Konzepten `rain` und `melody`, prüft deren `Wikidata`-/CC0-Provenienz, den 6/6-Agententest sowie die Persistenz über Speichern/Laden. Ein separater CLI-Ende-zu-Ende-Lauf mit `concept-import-template.tsv` meldete `Imported 2 concepts`, danach 11 Konzepte und beide gespeicherten Wikidata-CC0-Herkünfte. Der TSV-Import ruft keine externen Dienste automatisch ab; er ist ein kontrollierter lokaler Aufnahmepfad.

Die Beziehungsauswertung des Imports wurde anschließend geprüft: Die Relation `can-affect:forest` der Wikidata-CC0-Beispielzeile für `rain` erzeugte den zwölften Eintrag `rain-forest` mit Wortbildungsweg `relation-compound`, persistierter auslösender Relation und 6/6 erfolgreicher Form-Rekonstruktion. Der KR-Selbsttest sowie der separate Importlauf bestanden danach erneut.

Der externe Abrufpfad wurde mit der öffentlichen Wikidata-API geprüft. `fetch_wikidata_concepts.ps1 -Qid Q7925,Q638` schrieb die realen Entitäten `rain` und `music` als UTF-8-TSV mit englischen Beschreibungen, CC0, kanonischen Wikidata-URLs und UTC-Abrufzeit. Die Engine importierte anschließend beide Zeilen, meldete 11 Konzepte und zeigte für jeden Eintrag den 6/6-Agentenstatus. Eine zuvor verwendete falsche Beispiel-ID (`Q80279`) wurde dabei erkannt und durch `Q7925` ersetzt.

Der Abruf liest zusätzlich die begrenzten Wikidata-Kanten `P279`, `P31`, `P361`, `P527`, `P828` und `P1542` und löst deren Ziel-Labels in separaten, begrenzten API-Anfragen auf. Der Live-Ende-zu-Ende-Test rief zuerst `flood` (`Q8068`) und dann `rain` (`Q7925`) ab. Die Kante `causes:flood` traf dadurch beim Import auf ein bekanntes Zielkonzept; die Engine erzeugte als zwölften Eintrag `rain-flood`, markierte ihn als `relation-compound`, bewahrte die auslösende Kante in der Provenienz und erreichte 6/6 Agentenrekonstruktionen. Der KR-Selbsttest bestand zuvor mit derselben Wortbildungslogik.

Die Importauswertung wurde danach auf zwei Phasen umgestellt: Alle Grundkonzepte werden vor jeder Beziehungsauswertung aufgenommen. Der Selbsttest stellt `rain` bewusst vor `flood` in die TSV-Fixture und fordert dennoch `rain-flood`; der KR-Selbsttest bestand. Damit hängt die Entdeckung einer Wortfamilie nicht mehr davon ab, ob das Zielkonzept in derselben Datei vor oder nach dem Quellkonzept steht.

Die Win32-Oberfläche erhielt für den TSV-Import einen nativen Dateiauswahldialog. Ein vorhandener Pfad im Eingabefeld bleibt ein direkter Import; bei leerem Feld öffnet **Import TSV…** den Filter für `.tsv`-Dateien. Der KR-Release sowie der vollständige Selbsttest bestanden nach dem Link gegen `comdlg32` erneut.

Nach den Erweiterungen des Konzeptlexikons wurde der Android-ARM64-Kern erneut mit dem NDK gebaut und auf dem per ADB erreichbaren Gerät `4c90bfcc` ausgeführt. `leng-device-smoke` meldete `ANDROID_NATIVE_SMOKE_PASSED`; die temporäre Binärdatei und der Gerätezustand wurden danach aus `/data/local/tmp` entfernt. Der direkte `adb`-Name war in dieser PowerShell-Sitzung nicht im `PATH`; der vorhandene SDK-Pfad `C:\Users\ralfk\AppData\Local\Android\Sdk\platform-tools\adb.exe` wurde daher explizit verwendet.

Ein 13-Entitäten-Alltagsabruf von Wikidata prüfte anschließend die Zusammenführung externer und interner Konzeptwurzeln. Überlappende Konzepte wie `tree`, `forest`, `fire` und `water` wurden nicht abgewiesen und ihre bestehenden Formen nicht ersetzt; ihre Einträge erhielten die kombinierte Quelle `seeded-human-knowledge+Wikidata` und zusätzliche CC0-Provenienz. Aus den angereicherten Kanten entstanden unter anderem die 6/6-validierten Formen `tree-forest` und `food-water`. Der Testzustand und die zeitabhängige Abrufdatei wurden danach entfernt.

Das Konzeptkommunikationslabor wurde als vierter Endnachweis ergänzt. Es übermittelte auf einem frischen Forschungszustand eine zuvor nicht als Gesamtphrase trainierte Nachricht für `forest-fire danger`. Der Empfänger zerlegte die Komposition in bekannte Formen, rekonstruierte exakt `forest-fire danger` und meldete `Task success: yes`; interne Konzept-IDs wurden nicht übertragen. Der KR-Selbsttest fordert diesen Nachweis sowie Exportinhalt (`rain-flood` und CC0-Provenienz) nun verbindlich.
