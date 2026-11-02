#requires -Version 5.1
<#
Lunari Lehrbuch: alle deutschen Lerntexte in einem Durchlauf kodieren.
Original .leng bleibt unveraendert; die CLI arbeitet nur mit einer temporaeren Kopie.
Ausgabe: JSON / CSV / TXT fuer den Rueckimport in das Schulbuch.
#>
[CmdletBinding()]
param(
  [string]$Projektordner = 'H:\Linguistische-Emergenz-System\Linguistische-Emergenz-NG',
  [string]$Ausgabeordner = '',
  [int]$MaxZeichenProAufruf = 4500
)
$ErrorActionPreference = 'Stop'
[Console]::OutputEncoding = New-Object System.Text.UTF8Encoding($false)
$OutputEncoding = [Console]::OutputEncoding
if (-not $Ausgabeordner) { $Ausgabeordner = Join-Path $PSScriptRoot 'Lunari_Lehrbuch_Ergebnisse' }
New-Item -ItemType Directory -Force -Path $Ausgabeordner | Out-Null
$exe = Join-Path $Projektordner 'release\linguistische-emergenz-ng.exe'
$original = Join-Path $Projektordner 'german-wordlist-only.leng'
if (-not (Test-Path -LiteralPath $exe)) { throw "CLI nicht gefunden: $exe" }
if (-not (Test-Path -LiteralPath $original)) { throw "Lexikon nicht gefunden: $original" }
if ($MaxZeichenProAufruf -lt 1000 -or $MaxZeichenProAufruf -gt 12000) { throw 'MaxZeichenProAufruf muss zwischen 1000 und 12000 liegen.' }
$originalHash = (Get-FileHash -LiteralPath $original -Algorithm SHA256).Hash
$compressed = [Convert]::FromBase64String('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')
$inputMemory = New-Object System.IO.MemoryStream(,$compressed)
$zipStream = New-Object System.IO.Compression.GzipStream($inputMemory,[System.IO.Compression.CompressionMode]::Decompress)
$reader = New-Object System.IO.StreamReader($zipStream,[System.Text.Encoding]::UTF8)
try { $auftraege = @((ConvertFrom-Json -InputObject $reader.ReadToEnd())) } finally { $reader.Dispose(); $zipStream.Dispose(); $inputMemory.Dispose() }
$delimiter = "`n<><><>`n"
$chunks = New-Object System.Collections.ArrayList
$current = New-Object System.Collections.ArrayList
$length = 0
foreach ($request in $auftraege) {
  $size = ([string]$request.deutsch).Length + $delimiter.Length
  if ($size -gt $MaxZeichenProAufruf) { throw "Eintrag $($request.id) ist groesser als die Batchgrenze." }
  if ($current.Count -gt 0 -and ($length + $size) -gt $MaxZeichenProAufruf) {
    [void]$chunks.Add(@($current.ToArray())); $current.Clear(); $length = 0
  }
  [void]$current.Add($request); $length += $size
}
if ($current.Count -gt 0) { [void]$chunks.Add(@($current.ToArray())) }
$workDir = Join-Path ([System.IO.Path]::GetTempPath()) ("lunari-lehrbuch-"+[Guid]::NewGuid().ToString('N'))
New-Item -ItemType Directory -Force -Path $workDir | Out-Null
$workingLexicon = Join-Path $workDir 'arbeitslexikon.leng'
Copy-Item -LiteralPath $original -Destination $workingLexicon
$results = New-Object System.Collections.ArrayList
$newTokensTotal = 0
try {
  for ($i=0; $i -lt $chunks.Count; $i++) {
    $batch = @($chunks[$i]); $joined = ($batch | ForEach-Object { [string]$_.deutsch }) -join $delimiter
    Write-Host ("Batch {0}/{1}: {2} Eintraege" -f ($i+1),$chunks.Count,$batch.Count)
    $lines = & $exe '--translate-text' $workingLexicon 'constructed' $joined 2>&1
    if ($LASTEXITCODE -ne 0) { throw "CLI-Fehler in Batch $($i+1): $($lines -join [Environment]::NewLine)" }
    $output = $lines -join "`n"
    $prefix = "Lunari text:`n"
    $suffix = "`n-- end Lunari text --"
    $from = $output.IndexOf($prefix,[StringComparison]::Ordinal)
    if ($from -lt 0) { throw "Kein Lunari-Text in Batch $($i+1): $output" }
    $from += $prefix.Length
    $to = $output.IndexOf($suffix,$from,[StringComparison]::Ordinal)
    if ($to -lt 0) { throw "Keine Ende-Markierung in Batch $($i+1)" }
    $encoded = $output.Substring($from,$to-$from)
    $translated = @($encoded.Split([string[]]@($delimiter),[StringSplitOptions]::None))
    if ($translated.Count -ne $batch.Count) { throw "Batch $($i+1): $($translated.Count) statt $($batch.Count) Eintraege; keine Zuordnung vorgenommen." }
    $match = [regex]::Match($output,'newly learned tokens=(\d+)')
    $newInBatch = if ($match.Success) { [int]$match.Groups[1].Value } else { throw "Lernstatus fehlt in Batch $($i+1)" }
    $newTokensTotal += $newInBatch
    for ($j=0; $j -lt $batch.Count; $j++) {
      [void]$results.Add([pscustomobject]@{
        id = [string]$batch[$j].id
        lektion = [string]$batch[$j].lektion
        bereich = [string]$batch[$j].bereich
        deutsch = [string]$batch[$j].deutsch
        lunari = [string]$translated[$j]
        batch = $i+1
        batch_neue_woerter = $newInBatch
      })
    }
  }
  $finalHash = (Get-FileHash -LiteralPath $original -Algorithm SHA256).Hash
  if ($finalHash -ne $originalHash) { throw 'ACHTUNG: SHA-256 des Originallexikons hat sich geaendert!' }
  $json = Join-Path $Ausgabeordner 'Lunari_Lehrbuch_Uebersetzungen.json'
  $csv = Join-Path $Ausgabeordner 'Lunari_Lehrbuch_Uebersetzungen.csv'
  $txt = Join-Path $Ausgabeordner 'Lunari_Lehrbuch_Uebersetzungen.txt'
  $results.ToArray() | ConvertTo-Json -Depth 6 | Set-Content -LiteralPath $json -Encoding UTF8
  $results.ToArray() | Export-Csv -LiteralPath $csv -Encoding UTF8 -NoTypeInformation -Delimiter ';'
  $text = New-Object System.Collections.Generic.List[string]
  foreach($r in $results) {
    $text.Add("[$($r.id)] $($r.lektion) / $($r.bereich)")
    $text.Add("DE: $($r.deutsch)")
    $text.Add("LU: $($r.lunari)")
    $text.Add('')
  }
  [System.IO.File]::WriteAllLines($txt,$text.ToArray(),(New-Object System.Text.UTF8Encoding($true)))
  if ($newTokensTotal -gt 0) {
    $expandedState = Join-Path $Ausgabeordner 'Lunari_Lehrbuch_Arbeitslexikon_mit_neuen_Woertern.leng'
    Copy-Item -LiteralPath $workingLexicon -Destination $expandedState -Force
    Write-Warning "Neue Woerter wurden nur der Arbeitskopie hinzugefuegt. Fuer diese Uebersetzungen ist die mitgelieferte Arbeitslexikon-Datei erforderlich."
  }
  $summary = @(
    "Eintraege=$($results.Count)",
    "Batches=$($chunks.Count)",
    "Neue_Wortformen_in_Arbeitskopie=$newTokensTotal",
    "Original_Lexikon_SHA256=$originalHash",
    "Original_unveraendert=JA",
    "Erweitertes_Arbeitslexikon_gespeichert=$(if ($newTokensTotal -gt 0) { 'JA' } else { 'NEIN' })",
    "Hinweis: Bei neuen Wortformen ist die Arbeitskopie vom eingefrorenen Original abgewichen."
  )
  $summary | Set-Content -LiteralPath (Join-Path $Ausgabeordner 'Lunari_Lehrbuch_Protokoll.txt') -Encoding UTF8
  Write-Host "FERTIG: $($results.Count) Uebersetzungen, $($chunks.Count) CLI-Aufrufe, neue Wortformen: $newTokensTotal"
  Write-Host "Ergebnisse: $Ausgabeordner"
  Write-Host "Originallexikon unveraendert: $originalHash"
} finally {
  if (Test-Path -LiteralPath $workDir) { Remove-Item -LiteralPath $workDir -Recurse -Force }
}
