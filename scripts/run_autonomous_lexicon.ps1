[CmdletBinding()]
param(
  [string]$State = 'wikidata-100k.leng',
  [string]$Source = 'wikidata-100k.tsv',
  [string]$CollectorCheckpoint = 'wikidata-100k.collector.json',
  [ValidateRange(1000, 100000)] [int]$Target = 100000,
  [ValidateRange(100, 10000)] [int]$ImportBatch = 1000,
  [ValidateRange(100, 60000)] [int]$ApiDelayMilliseconds = 5000,
  [ValidateRange(1, 1000)] [int]$RequestsPerCycle = 20
)

# Reproducible autonomous research driver.  Each ingestion batch is committed
# by the native executable before the next one starts.  Safe to relaunch after
# a shutdown: both collector and engine retain independent checkpoints.
$ErrorActionPreference = 'Stop'
$root = Split-Path -Parent $PSScriptRoot
$binary = Join-Path $root 'release\linguistische-emergenz-ng.exe'
if (-not (Test-Path -LiteralPath $binary)) { throw "Release binary missing: $binary. Run build_kr_release.ps1 first." }
$statePath = Join-Path $root $State; $sourcePath = Join-Path $root $Source; $checkpointPath = Join-Path $root $CollectorCheckpoint
if (-not (Test-Path -LiteralPath $statePath)) { & $binary --run $statePath 1; if ($LASTEXITCODE -ne 0) { throw 'Could not create fresh research state.' } }
do {
  & (Join-Path $PSScriptRoot 'collect_wikidata_range.ps1') -Output $sourcePath -Checkpoint $checkpointPath -Target $Target -DelayMilliseconds $ApiDelayMilliseconds -MaxRequests $RequestsPerCycle
  # `$LASTEXITCODE` is only defined reliably for native executables.  The
  # collector is a PowerShell script, so use the PowerShell success flag.
  if (-not $?) { throw 'Controlled Wikidata collection failed.' }
  # Each bounded native import persists before the next batch.  A restart can
  # therefore resume both source collection and graph integration safely.
  do {
    $result = & $binary --autonomous-step $statePath $sourcePath $ImportBatch
    if ($LASTEXITCODE -ne 0) { throw 'Native autonomous intake failed.' }
    $result | Write-Output
    $importComplete = $result -match 'complete=yes'
  } while (-not $importComplete)
  $collector = Get-Content -Raw -LiteralPath $checkpointPath | ConvertFrom-Json
  foreach ($milestone in @(1000, 10000, 100000)) {
    $milestoneState = Join-Path $root ("wikidata-milestone-$milestone.leng")
    if ([int]$collector.accepted -ge $milestone -and -not (Test-Path -LiteralPath $milestoneState)) {
      & (Join-Path $PSScriptRoot 'save_wikidata_milestone.ps1') -Source $sourcePath -State $milestoneState -Count $milestone
      if (-not $?) { throw "Could not create actual $milestone-concept milestone." }
      & (Join-Path $PSScriptRoot 'verify_lexicon_milestone.ps1') -State $milestoneState -ExpectedExternal $milestone
      if (-not $?) { throw "Could not verify actual $milestone-concept milestone." }
    }
  }
  $complete = [int]$collector.accepted -ge $Target
} while (-not $complete)
& $binary --lexicon-status $statePath
