# Neu-Lunari v1 – vorgeschlagene, testbare Grammatik

Status: **vorgeschlagen**. Diese Spezifikation ergänzt den bisherigen
reversiblen Textkanal, ersetzt ihn nicht. Bestätigte Alt-Lunari-Texte und ihre
Wortformen bleiben unverändert erhalten.

## Kern

- Versionskennung: `neu-lunari-v1-proposed`
- Grammatik: analytisch; keine grammatischen Geschlechter, Fälle oder
  Person-/Zahlkongruenz.
- Neutrale Reihenfolge: Subjekt – Zeit – Aspekt – Modalität – Negation –
  Passiv – Verb – Objekt – Relation.
- Nominalgruppe: Bestimmtheit – Menge – Nomen – Adjektiv.
- Fragen enden mit `ke`; Aufforderungen beginnen mit `va`; Verbote mit `vas`.
- Historische Formen werden nicht automatisch als neue Formen umgedeutet.

## Schreib- und Lautsystem

Die Standardschreibung verwendet `a e i o u` und `m n l r p t k b d g f s h v`.
Neu-Lunari erlaubt `V`, `CV` und `CVC`; zulässige Koda-Anlautsfolgen sind
begrenzt (etwa `mk`, `nt`, `rk`, `st`, `lv`, `vr`). Vokale werden getrennt
gesprochen. Betonung liegt regulär auf der vorletzten Silbe; `vaské` wäre nur
mit Akut eine ausdrücklich markierte Betonungsabweichung. Standardschreibung
bleibt ASCII; Großschreibung markiert Satzanfang und Eigennamen.

## Funktionswortvorschläge

| Funktion | Form | Status |
| --- | --- | --- |
| ich | `mer` | vorgeschlagen; Altform `memekumo` bleibt historisch |
| du | `gav` | vorgeschlagen |
| Vergangenheit | `ren` | vorgeschlagen |
| Zukunft | `sil` | vorgeschlagen |
| Verlauf | `lir` | vorgeschlagen |
| abgeschlossen | `sen` | vorgeschlagen |
| Negation | `nus` | vorgeschlagen |
| Frage | `ke` | vorgeschlagen |
| bestimmt / unbestimmt | `tel` / `sar` | vorgeschlagen |
| Passiv | `vel` | vorgeschlagen |
| Ort / Ziel / Herkunft | `nel` / `tal` / `sor` | vorgeschlagen |

Vor einer Aktivierung prüft `--lunari-collisions <state.leng>` alle reservierten
Funktionsformen gegen den geladenen Formbestand. Der Audit gegen
`german-wordlist-only.leng` (1.647.877 Formen) ergab für die ursprünglich
reservierten Kernformen keine Kollisionen. Neue v1-Formen bleiben vor einer
Aktivierung ebenfalls über denselben Audit prüfbar.

## Strukturierte Verwendung

Die CLI nimmt eine explizite Bedeutungsstruktur an. Das ist absichtlich kein
Versprechen freier Deutsch- oder Englischübersetzung.

```text
--lunari-generate "subject=I;verb=see;object=sun;time=past;aspect=completed;negation=predicate;relation=location;related=home"
=> mer ren sen nus see sun nel home.

--lunari-generate "subject=I;verb=tar;embedding=content;embedded={subject=you;verb=lun;time=future}"
=> mer tar ki gav sil lun ku.
```

Die Rückanalyse derselben Satzform rekonstruiert die markierten Felder. Sie
weist unvollständige und unbekannte Subjekte ausdrücklich zurück.

## Beispiele

Die folgenden ausführbaren Beispiele verwenden `tar`, `lun`, `dom` und
`vaske` als **phonologisch gültige, aber unbestätigte Testlexeme**. Ihre
deutschen Übersetzungen erläutern nur die jeweilige Struktur.

1. `mer tar lun.` — einfache SVO-Aussage.
2. `gav nus tar lun.` — Prädikatsverneinung.
3. `mer ren tar lun.` — Vergangenheit.
4. `mer sil tar lun.` — Zukunft.
5. `mer lir tar lun.` — Verlauf.
6. `mer sen tar lun.` — abgeschlossener Aspekt.
7. `mer tan tar lun.` — Gewohnheit.
8. `mer kan tar lun.` — Fähigkeit.
9. `mer ves tar lun.` — Möglichkeit.
10. `gav tar lun ke.` — Ja/Nein-Frage.
11. `man gav tar lun ke.` — Personenfrage in Argumentposition.
12. `va gav tar lun!` — Aufforderung.
13. `vas gav tar lun!` — Verbot.
14. `mer tar tel lun.` — bestimmtes Nomen.
15. `mer tar sar lun.` — unbestimmtes Nomen.
16. `mer tar lin lun.` — Plural.
17. `mer tar 3 lun.` — genaue Menge.
18. `mer tar mar 3 lun.` — ungefähre Menge.
19. `mer tar fal 3/4 mun dom lun.` — Bruch mit Maßeinheit.
20. `mer tar lun daz vaske.` — Eigenschaft mit Grad.
21. `mer tar lun mor vaske kom dom.` — Vergleich mit Maßstab.
22. `mer tar lun nel dom.` — Ort.
23. `mer tar lun tal dom.` — Zielort.
24. `mer tar lun sor dom.` — Herkunft.
25. `mer tar lun rem gav.` — Empfänger.
26. `mer tar lun ket dom.` — Instrument.
27. `mer tar lun val gav.` — Begleitung.
28. `mer tar lun tem dom.` — Thema.
29. `mer tar lun sur dom.` — Zweck.
30. `mer tar lun bel gav.` — Besitz.
31. `mer tar lun pel dom.` — Teil-Ganzes.
32. `mer tar lun mir dom.` — Inhalt.
33. `mer tar lun dam dom.` — Material.
34. `lun vel tar.` — Passiv.
35. `lun gul vaske.` — Zustand mit vorgeschlagener Kopula.
36. `nor mer mur tar lun.` — Satznegation mit Pflicht.
37. `mer tar lun ran gav tar dom.` — Satzverbindung.
38. `mer tar ki gav sil lun ku.` — Inhaltssatz.
39. `mer tar ril gav lun rul.` — Relativsatzrahmen.
40. `rav mer tar qi gav lun qu.` — berichtete Aussage mit Zitatrahmen.

Inhaltssätze (`ki … ku`), Relativsatzrahmen (`ril … rul`) und Zitate
(`qi … qu`) werden strukturell analysiert und können verschachtelt werden.
Die Bedeutung der offenen Testlexeme bleibt dabei ausdrücklich unbestätigt.
