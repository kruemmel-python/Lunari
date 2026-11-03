# Lunari – Referenzgrammatik und Lexikon

Stand: 3. November 2026 · Ausgabe 1.0

## Geltung und Leseschlüssel

Dieses Dokument beschreibt zwei Schichten, die absichtlich getrennt bleiben.

| Kennzeichnung | Bedeutung |
| --- | --- |
| **Bestätigt** | Von Ralf durch Lehrbuch, Gespräch oder Screenshot bestätigt. |
| **Vorgeschlagen** | Neu-Lunari-v1-Regel oder -Form; nutzbar zum Testen, aber noch nicht als bestätigtes Alt-Lunari ausgegeben. |
| **Platzhalter** | Semantische Testvariable, keine Lunari-Vokabel. |

Alt-Lunari-Texte bleiben unverändert gültig. Neu-Lunari v1 ist ein analytisches,
versioniertes Register und ersetzt diese Texte nicht rückwirkend.

## 1. Lautung und Schrift

### 1.1 Inventar

| Gruppe | Buchstaben | IPA / Hinweis |
| --- | --- | --- |
| Vokale | `a e i o u` | [a e i o u] |
| Sonoranten | `m n l r` | [m n l ɾ~r] |
| Plosive | `p t k b d g` | wie im Deutschen, ohne Aspiration als Regel |
| Frikative | `f s h v` | [f s h v] |

**Vorgeschlagen:** Die Schreibung ist grundsätzlich lautnah. Groß- und
Kleinschreibung ändern keine Bedeutung; Satzanfang und Eigenname werden groß
geschrieben.

### 1.2 Silben und Betonung

Alt-Lunari verwendet belegbar vor allem offene V- und CV-Silben. Neu-Lunari
v1 erweitert dies kontrolliert:

| Ebene | Zulässig |
| --- | --- |
| Silbe | `V`, `CV`, `CVC` |
| Koda | `m n l r s v d` |
| reguläre Übergänge Koda–Anlaut | `mp mt mk mv; ns nt nk nv; lk lv; rt rk rn; lm lv; sm st sk; vr; dr dv` |
| nicht zulässig | beliebige Dreifachcluster, unanalysierbare Konsonantenfolgen |

Die Hauptbetonung liegt auf der vorletzten Silbe. Aufeinanderfolgende Vokale
werden getrennt gesprochen. Der Akut in `vaské` wäre ausschließlich eine
markierte Betonungsabweichung; die maschinenlesbare Standardschreibung lautet
vorerst `vaske`.

Beispiele: `mem-kur`, `gav-red`, `mer`, `ren`, `sen`.

## 2. Grundprinzipien von Neu-Lunari v1

- **analytisch:** keine Fälle, kein grammatisches Geschlecht und keine
  Konjugation nach Person oder Zahl;
- **SVO:** Subjekt – Verb – Objekt;
- **unveränderliche Wörter:** Zahl, Zeit, Modalität und Relation werden durch
  eigene Wörter ausgedrückt;
- **Nominalgruppe:** Bestimmtheit – Menge – Nomen – Eigenschaft;
- **Verbgruppe:** Zeit – Aspekt – Modalität – Negation – Passiv – Verb;
- **Frage:** feste Fragepartikel am Satzende, keine Verbvoranstellung.

Die aktuelle Engine unterstützt den dokumentierten, strukturierten
SVO-Teilumfang. Inhaltsrahmen `ki … ku`, Relativrahmen `ril … rul`,
Zitatrahmen `qi … qu` und Satzverbindungen werden als verschachtelte
Strukturen erzeugt und analysiert.

## 3. Satzbau

### 3.1 Aussage

```text
SUBJEKT  ZEIT  ASPEKT  MODALITÄT  NEGATION  PASSIV  VERB  OBJEKT  RELATION
```

Nicht benötigte Teile entfallen. Beispiel mit Testlexemen:

```text
mer ren sen nus tar lun nel dom.
ich Vergangenheit abgeschlossen nicht sehen Sonne Ort Haus.
„Ich habe die Sonne zu Hause nicht gesehen.“
```

`tar`, `lun` und `dom` sind hier nur phonologisch zulässige Testlexeme, keine
bestätigten Bedeutungen.

### 3.2 Nominalgruppe

```text
BESTIMMTHEIT  MENGE  NOMEN  ADJEKTIV
```

| Bedeutung | Struktur | Neu-Lunari-Form |
| --- | --- | --- |
| die / das … | bestimmt + Nomen | `tel Nomen` |
| ein / eine … | unbestimmt + Nomen | `sar Nomen` |
| mehrere … | plural + Nomen | `lin Nomen` |
| drei … | Zahl + Nomen | `3 Nomen` |
| ein kleines … | bestimmt + Nomen + Eigenschaft | `tel Nomen Adjektiv` |

### 3.3 Fragen und Sprechakte

| Funktion | Form | Satzmuster |
| --- | --- | --- |
| Aussage | unmarkiert | `mer tar lun.` |
| Ja/Nein-Frage | `ke` | `gav tar lun ke?` |
| Aufforderung | `va` | `va gav tar lun!` |
| Verbot | `vas` | `vas gav tar lun!` |

### 3.4 Negation und Geltungsbereich

| Bereich | Form | Bedeutung |
| --- | --- | --- |
| Prädikat | `nus` vor Verb | Handlung findet nicht statt |
| ganzer Satz | `nor` am Satzanfang | Aussage als Ganzes wird zurückgewiesen |
| Subjektfokus | `sam` vor dem Subjekt | Subjekt wird zurückgewiesen |
| Objektfokus | `som` vor dem Objekt | Objekt wird zurückgewiesen |
| Modalitätsfokus | `nam` vor dem Modalmarker | Modalität wird zurückgewiesen |

`mer nus tar lun.` bedeutet: „Ich sehe die Sonne nicht.“

### 3.5 Eigenschaften und Vergleich

Eigenschaften folgen dem Nomen. `daz`, `mor`, `les`, `sim` und `mas`
markieren sehr, mehr, weniger, gleich und höchst; bei einem benannten
Maßstab folgt `kom`.

```text
mer tar lun mor vaske kom dom.
ich sehe Sonne mehr gut Vergleich Haus.
```

Das Beispiel demonstriert die Struktur, nicht bestätigte Bedeutungen von
`tar`, `lun` oder `dom`. Strukturierte Eingabe:
`adjective=vaske;degree=more;compared_with=dom`.

### 3.6 Zeit, Aspekt und Modalität

| Kategorie | Form | Bedeutung |
| --- | --- | --- |
| Vergangenheit | `ren` | vor dem Sprechzeitpunkt |
| Zukunft | `sil` | nach dem Sprechzeitpunkt |
| Verlauf | `lir` | im Gang |
| Abschluss | `sen` | bis zum Bezugspunkt abgeschlossen |
| Gewohnheit | `tan` | regelmäßig |
| Fähigkeit | `kan` | können aufgrund Fähigkeit |
| Möglichkeit | `ves` | unter Umständen möglich |
| Erlaubnis | `dar` | dürfen |
| Pflicht | `mur` | müssen |
| Empfehlung | `res` | sollen / Empfehlung |
| Absicht | `vin` | beabsichtigen |
| Wunsch | `lum` | wünschen |

Alle Formen in dieser Tabelle sind **vorgeschlagen**.

### 3.6.1 Aussagenebene, Wahrscheinlichkeit und Haltung

`pur` und `sul` markieren Wahrscheinlichkeit beziehungsweise Unsicherheit;
`rav`, `hes` und `fik` markieren Bericht, Hypothese und Fiktion. Die
Rahmenmarker `fel`, `saf` und `gol` binden eine Emotion, soziale Beziehung
oder ein kommunikatives Ziel an den ganzen Satz, ohne eine Tatsachenbehauptung
zu erzeugen. Ihr Inhalt ist ein Lexem und wird nur dann als Bedeutung
bestätigt behandelt, wenn es im versionierten Lexikon bestätigt ist.

```text
rav pur fel vaske saf gav gol dom mer tar.
```

Strukturierte Eingabe: `level=report;probability=probable;emotion=vaske;
social=gav;goal=dom;subject=I;verb=tar`.

### 3.7 Relationen

| Relation | Form | Muster |
| --- | --- | --- |
| Ort | `nel` | `Verb nel Ort` |
| Zielort | `tal` | `Verb tal Ziel` |
| Herkunft | `sor` | `Verb sor Quelle` |
| Empfänger | `rem` | `Verb Objekt rem Empfänger` |
| Instrument | `ket` | `Verb Objekt ket Werkzeug` |
| Begleitung | `val` | `Verb val Begleiter` |
| Thema | `tem` | `Verb tem Thema` |
| Zweck | `sur` | `Verb sur Zweck` |
| Besitz | `bel` | `Besitztes bel Besitzer` |
| Teil–Ganzes | `pel` | `Teil pel Ganzes` |
| Inhalt | `mir` | `Verb mir Inhalt` |
| Material | `dam` | `Nomen dam Material` |

### 3.8 Passiv, Zustand, Satzverbindung

`vel` markiert vorgeschlagenes Passiv vor dem Verb: `Objekt vel Verb`.
Ein Zustand nutzt eine Kopula plus Eigenschaft; die belegte Form `gukikehi`
hat die bestätigte Bedeutung „ist“. Ihre allgemeine Erweiterung zur Kopula ist
eine **vorgeschlagene** Regel, keine rückwirkende Neubedeutung.

Satzverbindungen folgen der normalen SVO-Stellung. Die vorgeschlagenen
Verbinder sind `ran` (und), `lor` (oder), `bas` (aber), `kos` (Ursache),
`kim` (Bedingung), `lus` (Einräumung), `suv` (Satz-Zweck) und `tim` (Zeitbezug).
Inhaltssätze stehen zwischen `ki … ku`, Relativsätze zwischen `ril … rul`,
Zitate zwischen `qi … qu`. Diese Formen sind v1-Vorschläge.

## 3.9 Zahlen und Maße

Neu-Lunari v1 verwendet Ziffern als eindeutige Grundzahlen; sie stehen vor
dem Nomen und lösen keine zusätzliche Pluralpartikel aus. Die v1-Partikeln
`mar`, `las`, `sav`, `fal` und `tor` markieren ungefähr, mindestens,
höchstens, Bruch und Ordnungszahl. Einheiten folgen dem Maßmarker `mun` und
werden nicht flektiert.

```text
mer tar fal 3/4 mun vek lun.
ich sehen Bruch 3/4 Maß vek Sonne.
```

Die strukturierte Eingabe ist beispielsweise
`quantity=fraction:3/4;measure=vek`; zulässig sind außerdem `exact:N`,
`about:N`, `atleast:N`, `atmost:N` und `ordinal:N`. Die Einheit `vek` im
Beispiel ist ein phonologisch zulässiges Testlexem, keine bestätigte Einheit.

## 4. Lexikon: bestätigtes Alt-Lunari

| Deutsch | Lunari | Wortart / Hinweis | Quelle |
| --- | --- | --- | --- |
| Hallo | `Segameka` | Gruß | Lehrbuch |
| Guten Morgen | `Rumudase Vuvudite` | Grußformel | Lehrbuch |
| Auf Wiedersehen | `Modinona Denasomi` | Verabschiedung | Lehrbuch |
| Bis später | `Nonupado figipaha` | Verabschiedung | Gespräch |
| ich | `memekumo` | Pronomen | Lehrbuch |
| du | `gavogedu` | Pronomen | Lehrbuch |
| sie | `runebedo` | Pronomen; genaue grammatische Reichweite offen | Gespräch |
| wir | `ramerini` | Pronomen | Lehrbuch |
| das | `negigivo` | Demonstrativ / Inhalt offen | Gespräch |
| jetzt | `pigodiku` | Adverb | Gespräch |
| dann | `Musodita` | Adverb / Konnektor | Gespräch |
| aber | `Kehisuko` | Konjunktion | Lehrbuch |
| nicht | `tavituda` | Negation | Lehrbuch |
| sehr | `daradusi` | Gradadverb | Gespräch |
| etwas | `dobevisu` | Quantor | Gespräch |
| müde | `ludivife` | Adjektiv | Lehrbuch |
| gut | `vipakevo` | Adjektiv | Gespräch |
| Kaffee | `Rodesivi` | Nomen | Gespräch |
| hilft | `mulamubu` | Verbform; Lexemumfang offen | Gespräch |
| freut | `kugudadi` | Verbform; Lexemumfang offen | Gespräch |
| zusammenarbeiten | `rukivime` | Verb | Gespräch |
| gespannt | `petetosu` | Adjektiv | Gespräch |
| möchtest | `kobubeba` | Verbform; Lexemumfang offen | Gespräch |
| erzählen | `tihodofa` | Verb | Lehrbuch |
| lernen | `bupovohi` | Verb | Lehrbuch |
| sprechen | `vonukulo` | Verb | Lehrbuch |
| schlafen | `vonirive` | Verb | Lehrbuch |
| Name | `Dovihosu` | Nomen | Lehrbuch |
| heißen | `beleraso` | Verb | Lehrbuch |
| heiße | `tibabeda` | Verbform | Gespräch |
| Bett | `Sedabiku` | Nomen | Lehrbuch |
| fünf | `ranafofa` | Zahlwort | Lehrbuch |
| Minuten | `Golinivo` | Nomen / Maßeinheit | Gespräch |

## 5. Lexikon: vorgeschlagenes Neu-Lunari

Diese Wörter sind technisch kollisionsfrei gegen 1.647.877 persistierte
Lunari-Formen geprüft, aber noch nicht als Nutzerwortschatz bestätigt.

| Bedeutung | Form | Status / Altform |
| --- | --- | --- |
| ich | `mer` | vorgeschlagen; Altform `memekumo` |
| du | `gav` | vorgeschlagen; Altform `gavogedu` |
| dritte Person / sie | `sel` | vorgeschlagen |
| wir inklusiv | `rin` | vorgeschlagen |
| wir exklusiv | `lan` | vorgeschlagen |
| gut | `vaske` | vorgeschlagen; stilistische Akutschreibung `vaské`; Altform `vipakevo` |
| Frage | `ke` | vorgeschlagen |
| Aufforderung | `va` | vorgeschlagen |
| Verbot | `vas` | vorgeschlagen |
| Negation | `nus` | vorgeschlagen |
| Satznegation | `nor` | vorgeschlagen |
| bestimmt / unbestimmt | `tel` / `sar` | vorgeschlagen |
| Plural | `lin` | vorgeschlagen |
| Passiv | `vel` | vorgeschlagen |

## 6. Bestätigte Beispielsätze

| Lunari | Bestätigte deutsche Bedeutung |
| --- | --- |
| `Lunodera tepapavo vipakevo! Memekumo fapihiso hukekegi dobevisu ludivife. Kehisuko bahosige Rodesivi mulamubu.` | Mir geht's gut! Ich bin noch etwas müde. Aber der Kaffee hilft. |
| `Negigivo kugudadi lilaboda. Memekumo redanufo sebevife daradusi vipakevo masorosa ramerini muripubo rukivime!` | Das freut mich. Ich finde es sehr gut, wie wir beide zusammenarbeiten! |
| `Memekumo fapihiso petetosu! Dosesaso kobubeba gavogedu lunodera tihodofa?` | Ich bin gespannt! Was möchtest du mir erzählen? |
| `Nonupado figipaha!` | Bis später! |

## 7. Technische Verwendung

```powershell
.\build-kr-path\linguistische-emergenz-ng.exe --lunari-generate 'subject=I;verb=tar;object=lun;time=past'
.\build-kr-path\linguistische-emergenz-ng.exe --lunari-analyse 'mer ren tar lun.'
.\build-kr-path\linguistische-emergenz-ng.exe --lunari-word memkur
.\build-kr-path\linguistische-emergenz-ng.exe --lunari-collisions .\german-wordlist-only.leng
```

Der strukturierte Grammatikkanal ist nicht der alte reversible Textkanal. Für
freie Übersetzung gilt weiterhin: nur unterstützte Bedeutungsstrukturen werden
erzeugt; unbekannte Wörter oder Konstruktionen werden diagnostiziert und nicht
erraten.

## 8. Offene Entscheidungen für die nächste Bestätigung

1. Soll `mer`, `gav` und `vaske` aktiv werden oder nur Vorschläge bleiben?
2. Wie heißen die fehlenden Fragewörter, Konjunktionen, Fokusnegation und
   Einbettungsmarker im bestätigten Lunari?
3. Soll `gukikehi` verbindlich zur allgemeinen Kopula erweitert werden?
4. Welche der Neu-Lunari-Formen erhalten eine deutsche und englische
   Bestätigung für den produktiven Lexikoneintrag?
