[CmdletBinding()]
param(
  [Parameter(Mandatory = $true, Position = 0)]
  [ValidateNotNullOrEmpty()]
  [string[]]$Qid,
  [string]$Output = 'wikidata-concepts.tsv',
[string]$Language = 'en',
[string]$Revision = 'wikidata-api'
)

$ErrorActionPreference = 'Stop'

if ($Language -notmatch '^[a-z]{2,12}(-[a-z0-9]+)?$') { throw 'Language must be a simple BCP-47 language tag.' }
foreach ($id in $Qid) {
  if ($id -notmatch '^Q[1-9][0-9]*$') { throw "Invalid Wikidata entity id: $id" }
}

function ConvertTo-TsvCell([string]$Value) {
  if ($null -eq $Value) { return '' }
  return (($Value -replace "`t", ' ') -replace "`r?`n", ' ').Trim()
}

$ids = [string]::Join('|', $Qid)
$uri = "https://www.wikidata.org/w/api.php?action=wbgetentities&format=json&props=labels%7Cdescriptions%7Cclaims&languages=$Language&languagefallback=1&ids=$ids&origin=*"
$response = Invoke-RestMethod -Uri $uri -Method Get -Headers @{ 'User-Agent' = 'Linguistische-Emergenz-NG/0.1 (research lexicon importer)' }
$retrieved = [DateTime]::UtcNow.ToString('o')
$relationProperties = [ordered]@{
  P279  = 'is-a'       # subclass of
  P31   = 'is-a'       # instance of
  P361  = 'part-of'    # part of
  P527  = 'has-part'   # has part(s)
  P828  = 'has-cause'  # has cause
  P1542 = 'causes'     # has effect
}
$rows = [System.Collections.Generic.List[string]]::new()
$rows.Add("external_id`tlabel`tpart_of_speech`tsemantics`trelations`tsource`tprovenance`tsource_revision`tevidence_class")

foreach ($id in $Qid) {
  $entity = $response.entities.$id
  if ($null -eq $entity -or $entity.missing -ne $null) { throw "Wikidata did not return entity $id" }
  $label = ConvertTo-TsvCell $entity.labels.$Language.value
  if ([string]::IsNullOrWhiteSpace($label)) { throw "Wikidata entity $id has no $Language label" }
  $description = ConvertTo-TsvCell $entity.descriptions.$Language.value
  if ([string]::IsNullOrWhiteSpace($description)) { $description = "Wikidata entity $id" }
  $relations = [System.Collections.Generic.List[string]]::new()
  foreach ($property in $relationProperties.Keys) {
    foreach ($claim in @($entity.claims.$property)) {
      $targetId = $claim.mainsnak.datavalue.value.id
      if ($targetId -is [string] -and $targetId -match '^Q[1-9][0-9]*$') { $relations.Add("$($relationProperties[$property]):$targetId") }
    }
  }
  $provenance = "CC0;https://www.wikidata.org/wiki/$id;retrieved=$retrieved;language=$Language"
  $entityRevision = if ($null -ne $entity.lastrevid) { "$Revision;lastrevid=$($entity.lastrevid)" } else { $Revision }
  $rows.Add("$id`t$label`tnoun`t$description`t$([string]::Join(';', @($relations | Select-Object -Unique)))`tWikidata`t$provenance`t$entityRevision`texternal-attested")
}

$outputPath = [System.IO.Path]::GetFullPath($Output)
$directory = [System.IO.Path]::GetDirectoryName($outputPath)
if (-not [string]::IsNullOrEmpty($directory)) { [System.IO.Directory]::CreateDirectory($directory) | Out-Null }
$utf8NoBom = [System.Text.UTF8Encoding]::new($false)
[System.IO.File]::WriteAllText($outputPath, (($rows -join "`n") + "`n"), $utf8NoBom)
Write-Output "Fetched $($Qid.Count) Wikidata concepts to $outputPath"
