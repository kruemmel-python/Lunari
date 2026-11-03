# Native API

`include/leng/engine.hpp` ist die öffentliche C++20-API. Für sprachagnostische lokale
Forschungssteuerung liegt zusätzlich die stabile C-ABI `include/leng/c_api.h` vor.

- `ResearchEngine::run()` führt die konfigurierte Anzahl Generationen aus.
- `save(path)` und `load(path)` sichern bzw. laden einen versionierten Zustand.
- `report()` liefert gemessene Kennzahlen; `export_language()` liefert die aktuelle sprachliche Realisation.
- `self_test(error)` ist ein nichtinteraktiver Integritätscheck für CI.

`leng_api.dll` erzeugt einen undurchsichtigen `leng_engine`-Handle. `run`, `load`, `save`,
`export_results` und `report` arbeiten ausschließlich auf diesem Handle; Fehler werden
über `leng_engine_last_error` geliefert. Zeichenketten aus `report` sind bis zum
nächsten Aufruf auf demselben Handle gültig.
