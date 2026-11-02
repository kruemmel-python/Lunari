param(
  [string]$State = (Join-Path $PSScriptRoot '..\german-wordlist-only.leng'),
  [string[]]$Texts = @(
    (Join-Path $PSScriptRoot '..\tests\reference_nur_noch_fuenf_minuten.txt'),
    (Join-Path $PSScriptRoot '..\tests\independent_lunari_text.txt')
  )
)

$ErrorActionPreference = 'Stop'
$root = (Resolve-Path (Join-Path $PSScriptRoot '..')).Path
$binary = Join-Path $root 'release\linguistische-emergenz-ng.exe'
if (-not (Test-Path -LiteralPath $binary)) { throw 'Build the KR release before running this regression.' }
if (-not (Test-Path -LiteralPath $State)) { throw "State not found: $State" }

function Get-Block([string[]]$Lines, [string]$Header, [string]$Footer) {
  $content = [string]::Join("`n", $Lines)
  $start = $content.IndexOf($Header, [System.StringComparison]::Ordinal)
  if ($start -lt 0) { throw "Missing report header: $Header" }
  $start += $Header.Length
  $end = $content.IndexOf($Footer, $start, [System.StringComparison]::Ordinal)
  if ($end -lt 0) { throw "Missing report footer: $Footer" }
  return $content.Substring($start, $end - $start)
}

foreach ($path in $Texts) {
  $source = [System.IO.File]::ReadAllText((Resolve-Path $path), [System.Text.UTF8Encoding]::new($false))
  $forward = & $binary --translate-text $State constructed $source
  $lunari = Get-Block $forward "Lunari text:`n" "`n-- end Lunari text --"
  foreach ($token in [regex]::Matches($lunari, '[a-z]+')) {
    if ($token.Value -notmatch '^(?:[mnlrptkbdgfshv]?[aeiou])+$') { throw "Invalid Lunari phonotactics in text token: $($token.Value)" }
  }
  $backward = & $binary --translate-text $State de $lunari
  $recovered = Get-Block $backward "Deutsch text:`n" "`n-- end translated text --"
  if ($recovered -cne $source) { throw "Round-trip mismatch: $path" }
  Write-Output "PASS $(Split-Path $path -Leaf): characters=$($source.Length), lunari-characters=$($lunari.Length)"
}

$phonotactics = & $binary --phonotactics $State
if (-not ($phonotactics -contains 'open-syllable audit: passed')) { throw 'Persistent lexicon violates Lunari phonotactics.' }
Write-Output 'PASS Lunari phonotactic audit'

# This is deliberately separate from text round-tripping. The decode process
# receives only the Lunari semantic frame generated above, never the German
# source sentence.
$semanticForward = & $binary --translate-meaning $State constructed 'Der Mensch bewegt den Stein nicht gestern nahe.'
$semanticFrame = Get-Block $semanticForward "Lunari frame:`n" "`n-- end Lunari frame --"
if ([string]::IsNullOrWhiteSpace($semanticFrame)) { throw 'Semantic channel produced no Lunari frame.' }
$semanticBackward = & $binary --translate-meaning $State en $semanticFrame
$semanticReport = [string]::Join("`n", $semanticBackward)
if (-not $semanticReport.Contains('Meaning: statement; human / move / stone') -or
    -not $semanticReport.Contains('Negation: yes') -or
    -not $semanticReport.Contains('Time: past') -or
    -not $semanticReport.Contains('Decoded independently from Lunari semantic-field forms; no German source text was consulted.')) {
  throw 'Independent Lunari semantic-frame regression failed.'
}
Write-Output 'PASS independent Lunari semantic-frame decoding'
