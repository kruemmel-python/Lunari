# Lunari-Textübersetzung

Die Schaltflächen der Benutzeroberfläche verwenden den strukturtreuen Lunari-Textkanal.

- Jeder bekannte deutsche Eintrag erscheint direkt als seine persistente
  Lunari-Form; es gibt keine technische Hülle vor jedem Wort.
- Groß-/Kleinschreibung, Leerraum, Zeilenumbrüche, Satzzeichen, Anführungszeichen und Sprecherwechsel bleiben erhalten.
- Nicht im Lexikon vorhandene UTF-8-Tokens werden beim Vorwärtsübersetzen als
  lokale Wortform aufgenommen und erhalten sofort eine neue offene Lunari-CV-
  Form. Herkunft und Status bleiben dabei als `translator-text-intake` und
  `meaning=unverified` dokumentiert; die Engine erfindet keine Bedeutung.
- Alle Lunari-Texttokens erfüllen die aktuelle offene V-/CV-Silbenphonotaktik.

Die Tests `tests/reference_nur_noch_fuenf_minuten.txt` und
`tests/independent_lunari_text.txt` werden durch
`scripts/test_lunari_text_roundtrip.ps1` vorwärts und rückwärts geprüft.

## Wissenschaftliche Grenze

Der lokale Bestand `words.full.list.build.txt` enthält Wortformen, aber keine
deutschen Bedeutungen, Wortarten, Übersetzungen oder Sinnunterscheidungen.
Der Textkanal garantiert daher bedeutungs- und strukturtreue Übertragung durch
eine reversible Lunari-Repräsentation. Er ist keine behauptete freie deutsche
zu englische Bedeutungsübersetzung. Für eine semantisch freie Übersetzung
werden zusätzlich überprüfbare deutsche Bedeutungsdaten benötigt; diese müssen
als getrennte Quelle mit Herkunft und Lizenz aufgenommen werden.

## Bedeutungs-Kanal

Zusätzlich existiert ein strikter **Bedeutung → Lunari**-Kanal. Er verwendet
nicht die deutschen Wortformen, sondern den evolvierten 15-Felder-
Bedeutungsrahmen (Sprechakt, Akteur, Handlung, Objekt, Relation, Negation,
Zeit, Menge und weitere pragmatische Felder). Eine vollständige Lunari-
Bedeutungsnachricht kann ohne Zugriff auf den deutschen Ausgangstext wieder
in diesen Bedeutungsrahmen dekodiert werden.

In der Oberfläche ist dieser Pfad über **Bedeutung → Lunari** erreichbar.
Der gegenwärtig überprüfte Kern akzeptiert deutsche oder englische Aussagen
mit einem bekannten Akteur, einer Handlung und einem Objekt, etwa:

`Der Mensch bewegt den Stein nicht gestern nahe.`

Der Rückweg zeigt alle dekodierten Bedeutungsfelder an. Bei Texten außerhalb
dieses überprüften Kerns lehnt der Bedeutungskanal die Analyse ab, statt eine
Bedeutung zu erfinden; hierfür bleibt der reversible Textkanal zuständig.
Kommandozeilenäquivalent:

```powershell
.\release\linguistische-emergenz-ng.exe --translate-meaning .\german-wordlist-only.leng constructed "Der Mensch bewegt den Stein nicht gestern nahe."
```

## Zwei unabhängige Instanzen

Der praktische Kommunikationsnachweis verwendet zwei getrennt gestartete
Konsoleninstanzen mit derselben unveränderten Lexikondatei. Nur der Sender
erhält die deutsche Geschichte; der Empfänger erhält ausschließlich den
erzeugten Lunari-Text. Der Test prüft danach sowohl die exakte
Rückgewinnung als auch die unveränderte SHA-256-Prüfsumme des gemeinsamen
Lexikons:

```powershell
.\scripts\test_lunari_two_instance_exchange.ps1
```

Für diesen eingefrorenen Austausch müssen die Wörter der Geschichte bereits
im persistenten Lexikon vorhanden sein. Neue Wörter müssen zunächst in einen
gemeinsamen, gespeicherten Lexikonstand aufgenommen werden, bevor ein anderer
Empfänger sie ohne deutsche Vorlage verstehen kann.
