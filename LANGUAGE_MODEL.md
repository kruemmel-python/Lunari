# Sprach- und Bedeutungsmodell

Das Bedeutungsmodell trennt Identität, Realisierung und Bedeutung:

| Ebene | Repräsentation | Beispiel |
| --- | --- | --- |
| Bedeutung | atomare Felder einer `Meaning` | `action:observe` |
| Tokenidentität | stabiler atomarer Schlüssel | `action:observe` |
| Realisierung | evolvierbares, aussprechbares Wort | `lunmalnem` |

## Neu-Lunari: kompakte, eigene Klangidentität

Neu-Lunari ersetzt die frühere gleichförmige offene-CV-Strategie durch einen
eigenständigen, kompakteren Rhythmus. Sein Kerninventar sind die Vokale
`a e i o u`, die Anlaute `m n l r p t k b d g f s h v` und die bewusst kleine
Koda-Menge `m n l r s v d`. Zulässige Silben sind `V`, `CV` und `CVC`.

Eine Koda vor dem nächsten Silbenanlaut ist nur in einer kleinen festen Menge
erlaubt (zum Beispiel `mk`, `nt`, `rk`, `st`, `lv`, `vr`). Dadurch werden Formen
wie `memkur` und `gavred` möglich, ohne dass ungeordnete
Konsonantenhäufungen entstehen. Neue
Funktionsformen dürfen eine oder zwei Silben haben; neue Inhaltswurzeln nutzen
meist drei kompakte Silben mit mindestens einer möglichen Koda. Wortenden auf
`m`, `n`, `l`, `r` oder `s` sind daher normal, offene Endungen bleiben aber
eine Option.

Das ist keine Nachahmung einer bestehenden Sprachfamilie, sondern eine
gestaltete lunari-eigene Phonotaktik. Bestehende Alt-Lunari-Formen bleiben als
älterer Sprachstand lesbar; neue, automatisch erzeugte Formen folgen
`morphology-v4-neu-lunari-compact-cvc`.

### Übergangswortschatz

Für die Gesprächssprache kann die Verdichtung sofort sichtbar werden. Die
folgenden Formen sind als Neu-Lunari-Register festgelegt; die Altformen bleiben
als verständliche, feierlichere oder historische Varianten erhalten.

| Bedeutung | Alt-Lunari | Neu-Lunari |
| --- | --- | --- |
| ich | `memekumo` | `mer` (kurz auch `me`) |
| du | `gavogedu` | `gav` |
| gut | `vipakevo` | `vaské` |

`memkur` und `gavred` sind zugleich Musterwörter für die neue Phonotaktik:
`mem-kur` nutzt den erlaubten Übergang `mk`, `gav-red` den Übergang `vr` und
einen geschlossenen Wortauslaut. Die Akut-Schreibung in `vaské` ist eine
optionale Lesemarkierung für Betonung; die interne maschinenlesbare Form bleibt
vorerst ASCII-kompatibel.

Die Metrik bewertet Struktur, Übergänge, Länge und akustische Trennbarkeit. Das
ist eine rechnerische Schätzung und keine Messung menschlicher Erlernbarkeit.

Der Aufgabenraum enthält Aussagen, Fragen und Aufforderungen sowie Akteure, Ereignisse, Objekte, räumlich/zeitliche und kausale/konditionale Relationen, Negation und Quantitäten. Zusätzlich sind Modalität (`certain`, `possible`, `required`), Wahrscheinlichkeit, Emotion, soziale Beziehung, Kommunikationsziel, Aussagenebene (`direct`, `reported`) und Abstraktion (`literal`, `metaphoric`) eigenständige strukturierte Felder. Diese Werte werden niemals aus Anzeigetext abgeleitet, sondern tokenisiert, gesendet und beim Empfänger rekonstruiert. Die Metaphernebene markiert eine strukturell prüfbare Aussageart; sie behauptet keine allgemeine automatische Metapherninterpretation.

`LENG_STATE 2` erweiterte den Zustand zunächst um diese Atome. Die aktuelle Version `LENG_STATE 11` enthält Population, RNG-Snapshot, getrennte Produktions-/Interpretationsmetriken des Humanlabors, für jeden Kandidaten eine explizite Reihenfolge der fünfzehn Grammatikfelder und einen offenen Konzeptgraphen mit Quelle und Provenienz. Beim Laden älterer Zustände werden die Abstraktionstoken und die erweiterte Standardreihenfolge ergänzt; beim nächsten Speichern wird außerdem der deterministische Startgraph ergänzt. Bisherige Token bleiben erhalten.

Der Konzeptgraph erweitert den festen Satzrahmen um Wörterbuchwissen. Einträge haben Herkunft, semantische Merkmale, Beziehungen und Wortfamilien. Zusammensetzungen verwenden die Formen ihrer bekannten Bestandteile; Agenten rekonstruieren die Bedeutung aus ihren Form-zu-Label-Zuordnungen. `seeded-human-knowledge` bezeichnet lokal kuratierte Startbegriffe, während `simulated-experience` aus Beziehungen erzeugte Komposita bezeichnet. Zusätzlich kann der kontrollierte TSV-Fluss frei lizenzierte Wikidata-Labels, Beschreibungen und ausgewählte Beziehungen aufnehmen; Quelle, CC0-Herkunft, URL und Abrufzeit bleiben am Eintrag erhalten. Ein automatischer Wiktionary-Import ist nicht implementiert.
