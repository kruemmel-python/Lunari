param(
  [string]$LexiconPath = (Join-Path $PSScriptRoot '..\neu-lunari-german-v1.tsv'),
  [string]$OutputPath = (Join-Path $PSScriptRoot '..\docs\LUNARI_DUDEN.md')
)

$ErrorActionPreference = 'Stop'
$LexiconPath = [IO.Path]::GetFullPath($LexiconPath)
$OutputPath = [IO.Path]::GetFullPath($OutputPath)
if (-not (Test-Path -LiteralPath $LexiconPath)) { throw "V2-Lexikon nicht gefunden: $LexiconPath" }

$entryCount = 0
$entries = [System.Collections.Generic.List[string]]::new()
$reader = [IO.File]::OpenText($LexiconPath)
try {
  [void]$reader.ReadLine() # format marker
  [void]$reader.ReadLine() # columns
  while ($null -ne ($line = $reader.ReadLine())) {
    if ($line.IndexOf("`t") -lt 0) { continue }
    $entries.Add($line)
    ++$entryCount
  }
} finally { $reader.Dispose() }

# Learned words are appended by the translator.  A Duden must instead be
# alphabetically navigable.  This comparer keeps each initial in one section
# (A, Ä, B … O, Ö … U, Ü … Z) and then applies German collation inside it.
if (-not ('LunariDudenComparer' -as [type])) {
  Add-Type @'
using System;
using System.Collections.Generic;
using System.Globalization;
public sealed class LunariDudenComparer : IComparer<string> {
  private static readonly CompareInfo German = CultureInfo.GetCultureInfo("de-DE").CompareInfo;
  private static int Rank(char value) {
    const string alphabet = "AÄBCDEFGHIJKLMNOÖPQRSTUÜVWXYZ";
    var index = alphabet.IndexOf(char.ToUpperInvariant(value));
    return index < 0 ? alphabet.Length : index;
  }
  public int Compare(string left, string right) {
    string leftWord = left.Split('\t')[0], rightWord = right.Split('\t')[0];
    int initial = Rank(leftWord.Length == 0 ? '\uffff' : leftWord[0]).CompareTo(Rank(rightWord.Length == 0 ? '\uffff' : rightWord[0]));
    return initial != 0 ? initial : German.Compare(leftWord, rightWord, CompareOptions.IgnoreCase);
  }
}
'@
}
$entries.Sort([LunariDudenComparer]::new())

$outDir = Split-Path -Parent $OutputPath
New-Item -ItemType Directory -Force -Path $outDir | Out-Null
$utf8 = [Text.UTF8Encoding]::new($false)
$writer = [IO.StreamWriter]::new($OutputPath, $false, $utf8)
try {
  $writer.WriteLine('# Lunari-Duden — Deutsch → Neu-Lunari')
  $writer.WriteLine()
  $writer.WriteLine(('Stand: Neu-Lunari v2-Lexikon. Dieses Nachschlagewerk enthält **{0:N0}** gespeicherte deutsche Wortformen und ihre stabilen Neu-Lunari-Entsprechungen.' -f $entryCount))
  $writer.WriteLine()
  $writer.WriteLine('> Hinweis: Die Tabelle ist sehr groß. Für maschinelle Suche, Import und Updates bleibt `neu-lunari-german-v1.tsv` die kanonische Datenquelle; diese Datei ist ihre lesbare Markdown-Ausgabe.')
  $writer.WriteLine()
  $writer.WriteLine('## Benutzung')
  $writer.WriteLine()
  $writer.WriteLine('- Die linke Spalte enthält die deutsche Eingabeform, die rechte die dazu gespeicherte Lunari-Form.')
  $writer.WriteLine('- Neue deutsche Wörter werden beim Übersetzen deterministisch gelernt und im V2-Lexikon abgelegt. Nach einem erneuten Lauf dieses Skripts erscheinen sie auch hier.')
  $writer.WriteLine('- Groß-/Kleinschreibung ist für die Zuordnung nicht bedeutungsunterscheidend; die Darstellung folgt dem Datensatz.')
  $writer.WriteLine('- Die Wortformen sind keine Verschlüsselung: neue Inhaltswörter haben vier offene CV-Silben. Ihre Bedeutung ergibt sich aus ihrem gespeicherten Wörterbucheintrag.')
  $writer.WriteLine()
  $writer.WriteLine('## Aussprache und Wortbau')
  $writer.WriteLine()
  $writer.WriteLine('- Vokale: `a e i o u`.')
  $writer.WriteLine('- Silben: regulär `V`, `CV` oder `CVC`; die automatisch gelernten Inhaltswörter sind vier `CV`-Silben lang.')
  $writer.WriteLine('- Betonung: regelmäßig auf der vorletzten Silbe. Beispiel: `vusupuhe` = **vu-su-PU-he**, `rolopegi` = **ro-lo-PE-gi**.')
  $writer.WriteLine('- Zulässige Anlaute: `m n l r p t k b d g f s v h`. Die Grammatikpartikel sind kurze, unveränderliche Wörter.')
  $writer.WriteLine('- Produktive Ableitung: `-ar` Person/Handelnde, `-em` Werkzeug, `-im` Abstraktum, `-ul` Fach-/Sachbereich. Zusammensetzungen verbinden zwei gültige Formen; bei Vokal+Vokal fügt die Implementierung `n` ein.')
  $writer.WriteLine()
  $writer.WriteLine('## Grammatik')
  $writer.WriteLine()
  $writer.WriteLine('Neu-Lunari verwendet grundsätzlich **SVO**: Subjekt – (Zeit/Aspekt/Modalität/Negation) – Verb – Objekt. Lunari-Wörter werden nicht flektiert; grammatische Kategorien stehen als eigene Partikel. Fragen erhalten zusätzlich `ke` am Satzende.')
  $writer.WriteLine()
  $writer.WriteLine('Der UI-Satzübersetzer verwendet für regelmäßige deutsche Verben die **im V2-Lexikon vorhandene Infinitivform als Lunari-Lemma**. Die Worttabelle führt daneben auch deutsche Eingabeformen wie `lerne`, `lernst` oder `lernte`, weil sie als vollständiger Datenbestand gespeichert bleiben. Für Grammatik-Sätze ist jedoch das Lemma verbindlich: `lernen` → `ginukifu`.')
  $writer.WriteLine()
  $writer.WriteLine('### Grundpartikel')
  $writer.WriteLine()
  $writer.WriteLine('Personalpronomen und Inhaltswörter stammen im Benutzerübersetzer immer aus dem V2-Wörterverzeichnis. Beispielsweise gelten `ich` → `rasupemo` und `du` → `femuvefu`; die folgenden Formen sind ausschließlich grammatische Partikel.')
  $writer.WriteLine()
  $writer.WriteLine('| Funktion | Lunari |')
  $writer.WriteLine('| --- | --- |')
  @(
    @('Vergangenheit','ren'), @('Zukunft','sil'), @('Verbnegation','nus'), @('Frage am Satzende','ke'), @('Können/Fähigkeit','kan')
  ) | ForEach-Object { $writer.WriteLine(('| {0} | `{1}` |' -f $_[0], $_[1])) }
  $writer.WriteLine()
  $writer.WriteLine('### Satzmuster')
  $writer.WriteLine()
  $writer.WriteLine('```text')
  $writer.WriteLine('Aussage:  Subjekt [Zeit] [Aspekt] [Modalität] [nus] [vel] Verb [Objekt].')
  $writer.WriteLine('Frage:    [Fragewort] Subjekt [Zeit] [Aspekt] [Modalität] Verb [Objekt] ke?')
  $writer.WriteLine('Aufforderung: va Subjekt Verb [Objekt]!')
  $writer.WriteLine('Verbot:       vas Subjekt Verb [Objekt]!')
  $writer.WriteLine('```')
  $writer.WriteLine()
  $writer.WriteLine('### Verbindliche UI-Beispiele')
  $writer.WriteLine()
  $writer.WriteLine('| Deutsch | Neu-Lunari |')
  $writer.WriteLine('| --- | --- |')
  @(
    @('Ich lerne.','`rasupemo ginukifu.`'), @('Ich lernte.','`rasupemo ren ginukifu.`'), @('Ich werde lernen.','`rasupemo sil ginukifu.`'), @('Ich lerne nicht.','`rasupemo nus ginukifu.`'), @('Lernst du?','`femuvefu ginukifu ke?`')
  ) | ForEach-Object { $writer.WriteLine(('| {0} | {1} |' -f $_[0], $_[1])) }
  $writer.WriteLine()
  $writer.WriteLine('Die Rückübersetzung erkennt diese Partikel und stellt die entsprechende deutsche reguläre Verbform wieder her. Wort-für-Wort-Text ohne erkennbares Satzmuster bleibt weiterhin exakt in der gespeicherten V2-Reihenfolge.')
  $writer.WriteLine()
  $writer.WriteLine('## Vollständiges Wörterverzeichnis')
  $writer.WriteLine()

  $currentInitial = ''
  foreach ($line in $entries) {
      $parts = $line -split "`t", 2
      if ($parts.Count -ne 2) { continue }
      $german = $parts[0].Replace('|', '\|').Replace('`', '\`')
      $lunari = $parts[1].Replace('|', '\|').Replace('`', '\`')
      $initial = if ($german.Length -gt 0) { $german.Substring(0, 1).ToUpperInvariant() } else { '#' }
      if ($initial -ne $currentInitial) {
        if ($currentInitial -ne '') { $writer.WriteLine() }
        $currentInitial = $initial
        $writer.WriteLine("### $initial")
        $writer.WriteLine()
        $writer.WriteLine('| Deutsch | Neu-Lunari |')
        $writer.WriteLine('| --- | --- |')
      }
      $writer.WriteLine(('| `{0}` | `{1}` |' -f $german, $lunari))
  }
} finally { $writer.Dispose() }

Write-Output "Lunari-Duden erstellt: $OutputPath ($entryCount Einträge)"
