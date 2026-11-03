[CmdletBinding()]
param(
  [Parameter(Mandatory = $true)] [string]$Output,
  [Parameter(Mandatory = $true)] [string]$Checkpoint,
  [ValidateRange(1, 100000000)] [int]$Target = 100000,
  [ValidateRange(1, 50)] [int]$BatchSize = 50,
  [ValidateRange(100, 60000)] [int]$DelayMilliseconds = 5000,
  [ValidateRange(1, 20)] [int]$MaxConsecutiveErrors = 5,
  [ValidateRange(0, 1000000)] [int]$MaxRequests = 0,
  [string]$Language = 'en'
)

# Controlled primary-source intake.  At most one official wbgetentities
# request is issued per delay interval, checkpoints after each request, and
# never treats a missing label/description as a qualified concept.
$ErrorActionPreference = 'Stop'
if ($Language -notmatch '^[a-z]{2,12}(-[a-z0-9]+)?$') { throw 'Invalid language tag.' }
$out = [IO.Path]::GetFullPath($Output); $cp = [IO.Path]::GetFullPath($Checkpoint)
[IO.Directory]::CreateDirectory([IO.Path]::GetDirectoryName($out)) | Out-Null
$state = if (Test-Path -LiteralPath $cp) { Get-Content -Raw -LiteralPath $cp | ConvertFrom-Json } else { [pscustomobject]@{ nextQid = 1; accepted = 0; requests = 0 } }
if (-not (Test-Path -LiteralPath $out)) { [IO.File]::WriteAllText($out, "external_id`tlabel`tpart_of_speech`tsemantics`trelations`tsource`tprovenance`tsource_revision`tevidence_class`n", [Text.UTF8Encoding]::new($false)) }
$relations = @{ P279 = 'is-a'; P31 = 'is-a'; P361 = 'part-of'; P527 = 'has-part'; P828 = 'has-cause'; P1542 = 'causes' }
$failures = 0
$requestsThisRun = 0
Write-Output "collector-cycle-start requests=$($state.requests) max_requests=$MaxRequests"
try {
  while ($state.accepted -lt $Target -and ($MaxRequests -eq 0 -or $requestsThisRun -lt $MaxRequests)) {
    $ids = for ($i = 0; $i -lt $BatchSize; ++$i) { "Q$($state.nextQid + $i)" }
    $uri = "https://www.wikidata.org/w/api.php?action=wbgetentities&format=json&props=labels%7Cdescriptions%7Cclaims&languages=$Language&languagefallback=1&ids=$($ids -join '|')"
    try { $response = Invoke-RestMethod -Uri $uri -Headers @{ 'User-Agent' = 'Linguistische-Emergenz-NG/0.2-controlled-lexicon-research' } -TimeoutSec 45; $failures = 0; $state | Add-Member -NotePropertyName status -NotePropertyValue 'running' -Force; $state | Add-Member -NotePropertyName lastError -NotePropertyValue '' -Force } catch {
      ++$failures; $wait = [Math]::Min(60000, $DelayMilliseconds * [Math]::Pow(2, $failures - 1)); $state | Add-Member -NotePropertyName status -NotePropertyValue 'backoff' -Force; $state | Add-Member -NotePropertyName lastError -NotePropertyValue $_.Exception.Message -Force; $state | Add-Member -NotePropertyName lastBackoffMilliseconds -NotePropertyValue ([int]$wait) -Force; [IO.File]::WriteAllText($cp, ($state | ConvertTo-Json), [Text.UTF8Encoding]::new($false)); if ($failures -ge $MaxConsecutiveErrors) { throw "Wikidata request failed $failures consecutive times: $($_.Exception.Message)" }; Start-Sleep -Milliseconds $wait; continue }
    $retrieved = [DateTime]::UtcNow.ToString('o')
    $pendingLines = [Collections.Generic.List[string]]::new()
    foreach ($id in $ids) {
      if ($state.accepted -ge $Target) { break }
      $entity = $response.entities.$id; if ($null -eq $entity -or $entity.missing -ne $null) { continue }
      $label = ([string]$entity.labels.$Language.value -replace "`t|`r|`n", ' ').Trim(); $description = ([string]$entity.descriptions.$Language.value -replace "`t|`r|`n", ' ').Trim()
      if ([string]::IsNullOrWhiteSpace($label) -or [string]::IsNullOrWhiteSpace($description)) { continue }
      $edges = [Collections.Generic.List[string]]::new()
      foreach ($property in $relations.Keys) { foreach ($claim in @($entity.claims.$property)) { $targetId = $claim.mainsnak.datavalue.value.id; if ($targetId -is [string] -and $targetId -match '^Q[1-9][0-9]*$') { $edges.Add("$($relations[$property]):$targetId") } } }
      $provenance = "CC0;https://www.wikidata.org/wiki/$id;retrieved=$retrieved;language=$Language"
      $entityRevision = if ($null -ne $entity.lastrevid) { "wikidata-api;lastrevid=$($entity.lastrevid)" } else { 'wikidata-api' }
      $pendingLines.Add("$id`t$label`tnoun`t$description`t$(($edges | Select-Object -Unique) -join ';')`tWikidata`t$provenance`t$entityRevision`texternal-attested")
      ++$state.accepted
    }
    if ($pendingLines.Count -gt 0) { [IO.File]::AppendAllText($out, (($pendingLines -join "`n") + "`n"), [Text.UTF8Encoding]::new($false)) }
    $state.nextQid += $BatchSize; ++$state.requests; ++$requestsThisRun
    $state | Add-Member -NotePropertyName updatedUtc -NotePropertyValue ([DateTime]::UtcNow.ToString('o')) -Force; [IO.File]::WriteAllText($cp, ($state | ConvertTo-Json), [Text.UTF8Encoding]::new($false))
    Write-Output "accepted=$($state.accepted) next_qid=$($state.nextQid) requests=$($state.requests)"
    Start-Sleep -Milliseconds $DelayMilliseconds
  }
} finally { Write-Output "collector-cycle-end processed_requests=$requestsThisRun total_requests=$($state.requests) accepted=$($state.accepted)" }
