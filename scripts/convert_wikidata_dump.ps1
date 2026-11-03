[CmdletBinding()]
param(
  [Parameter(Mandatory = $true)] [string]$Input,
  [Parameter(Mandatory = $true)] [string]$Output,
  [ValidateRange(1, 1000000)] [int]$Limit = 100000,
  [string]$Language = 'en',
  [string]$Revision = 'wikidata-dump-local'
)

# Converts a user-supplied official Wikidata JSON or JSON.GZ dump in bounded
# streaming fashion.  It does not download anything.  The output is the
# canonical nine-column source accepted by the native engine.
$ErrorActionPreference = 'Stop'
if ($Language -notmatch '^[a-z]{2,12}(-[a-z0-9]+)?$') { throw 'Invalid language tag.' }
$inputPath = [IO.Path]::GetFullPath($Input)
if (-not (Test-Path -LiteralPath $inputPath)) { throw "Missing local dump: $inputPath" }
$outputPath = [IO.Path]::GetFullPath($Output)
[IO.Directory]::CreateDirectory([IO.Path]::GetDirectoryName($outputPath)) | Out-Null

function Cell([object]$v) { if ($null -eq $v) { return '' }; return (([string]$v -replace "`t", ' ') -replace "`r?`n", ' ').Trim() }
function Relation([object]$entity, [hashtable]$props) {
  $result = [Collections.Generic.List[string]]::new()
  foreach ($p in $props.Keys) {
    foreach ($claim in @($entity.claims.$p)) {
      $id = $claim.mainsnak.datavalue.value.id
      if ($id -is [string] -and $id -match '^Q[1-9][0-9]*$') { $result.Add("$($props[$p]):$id") }
    }
  }
  return (($result | Select-Object -Unique) -join ';')
}

$bytes = [IO.File]::OpenRead($inputPath)
try {
  if ($inputPath.EndsWith('.gz', [StringComparison]::OrdinalIgnoreCase)) { $stream = [IO.Compression.GzipStream]::new($bytes, [IO.Compression.CompressionMode]::Decompress) }
  else { $stream = $bytes }
  try {
    $reader = [IO.StreamReader]::new($stream, [Text.UTF8Encoding]::new($false), $true, 65536)
    $writer = [IO.StreamWriter]::new($outputPath, $false, [Text.UTF8Encoding]::new($false), 65536)
    try {
      $writer.WriteLine("external_id`tlabel`tpart_of_speech`tsemantics`trelations`tsource`tprovenance`tsource_revision`tevidence_class")
      $props = @{ P279 = 'is-a'; P31 = 'is-a'; P361 = 'part-of'; P527 = 'has-part'; P828 = 'has-cause'; P1542 = 'causes' }
      $count = 0
      while (-not $reader.EndOfStream -and $count -lt $Limit) {
        $line = $reader.ReadLine().Trim().TrimStart('[').TrimEnd(',').Trim()
        if ([string]::IsNullOrWhiteSpace($line) -or $line -eq ']') { continue }
        try { $entity = $line | ConvertFrom-Json -Depth 32 } catch { continue }
        if ($entity.type -ne 'item' -or $entity.id -notmatch '^Q[1-9][0-9]*$') { continue }
        $label = Cell $entity.labels.$Language.value
        $description = Cell $entity.descriptions.$Language.value
        if ([string]::IsNullOrWhiteSpace($label) -or [string]::IsNullOrWhiteSpace($description)) { continue }
        $relations = Relation $entity $props
        $provenance = "CC0;https://www.wikidata.org/wiki/$($entity.id);dump=$([IO.Path]::GetFileName($inputPath));language=$Language"
        $entityRevision = if ($null -ne $entity.lastrevid) { "$Revision;lastrevid=$($entity.lastrevid)" } else { $Revision }
        $writer.WriteLine("$($entity.id)`t$label`tnoun`t$description`t$relations`tWikidata`t$provenance`t$entityRevision`texternal-attested")
        ++$count
      }
      Write-Output "Converted $count externally-attested Wikidata concepts to $outputPath"
    } finally { $writer.Dispose() }
  } finally { if ($stream -ne $bytes) { $stream.Dispose() } }
} finally { $bytes.Dispose() }
