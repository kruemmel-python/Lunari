# Quellenanalyse und Integrationsmatrix

| Quelle | Festgestellte relevante Funktion | Geplanter primärer Einsatz | Laufzeitabhängigkeit |
| --- | --- | --- | --- |
| CC_OpenCl_Enterprise | OpenCL-Geräte-/Speicher-/Kernel-ABI, Diagnose, CPU-nahe Algorithmen | dynamischer OpenCL-Adapter, GPU-Batch für Kandidatenreihung und CPU/GPU-Parität | keine |
| i18Token-Engine | C-ABI für Token, persistente Text-/Binärexporte; laut Header nicht threadsicher | unabhängige Tokenidentität/Realisation/Bedeutung, versioniertes Textformat und lokale C-ABI | keine |
| Tri-Core-Orchestrator ULTRA | GPU-Proto-Updates, SubQG/Myzel-Pheromonfeld und VQE/SPSA-Simulation (`CC_OpenCL.c`) | erneut geprüft: Die APIs optimieren feld-/energiebezogene Zustände, nicht Sprachverständigung. Sie werden daher nicht als Sprachoptimierer oder Quantenbeschleunigung fehlintegriert. Der vergleichbare, fachlich passende Mehrziel-/Pareto-Pfad ist stattdessen im Kern implementiert und über gespeicherte Zustände vergleichbar. | keine |
| Linguistische Emergenz | ZID/LPM/DWP, Pheromon-/Mood-Zyklus, Guardian und CPU-Fallback | semantischer Agenten-/Evolutionszyklus mit getrennten Agentenlexika und CPU-Fallback | keine |

Die Analyse ergab, dass keine der Quellen allein ein integriertes, portables Forschungssystem bereitstellt. Insbesondere darf die vorhandene VQE-Simulation nicht als Quantenbeschleunigung bezeichnet werden. Der aktuelle neue Kern übernimmt daher keine Binärartefakte oder Quellkopien; seine vollständige Laufzeit ist unabhängig von den Quellordnern.
