[CmdletBinding()]
param(
  [Parameter(Mandatory = $true)] [string]$State,
  [Parameter(Mandatory = $true)] [ValidateRange(1, 100000)] [int]$ExpectedExternal
)

# Machine-checkable acceptance gate for a persisted research milestone.
$ErrorActionPreference = 'Stop'
$root = Split-Path -Parent $PSScriptRoot
$binary = Join-Path $root 'release\linguistische-emergenz-ng.exe'
$statePath = [IO.Path]::GetFullPath($State)
if (-not (Test-Path -LiteralPath $statePath)) { throw "Missing milestone state: $statePath" }
$status = & $binary --lexicon-status $statePath
if ($LASTEXITCODE -ne 0) { throw 'Could not inspect milestone state.' }
function Metric([string]$name) { $line = $status | Where-Object { $_ -like "$name=*" } | Select-Object -First 1; if ($null -eq $line) { throw "Missing status metric: $name" }; return [int64]($line.Substring($name.Length + 1)) }
$imported = Metric 'imported concepts'; $qualified = Metric 'qualified external concepts'; $realised = Metric 'automatically developed wordforms'; $validated = Metric 'agentically validated concepts'
if ($imported -lt $ExpectedExternal -or $qualified -lt $ExpectedExternal) { throw "Milestone insufficient: imported=$imported qualified=$qualified expected=$ExpectedExternal" }
if ($realised -lt $ExpectedExternal -or $validated -lt 1) { throw "Milestone lacks realisation or independent validation: realised=$realised validated=$validated" }
"MILESTONE VERIFIED`nstate=$statePath`nexpected_external=$ExpectedExternal`nimported=$imported`nqualified=$qualified`nwordforms=$realised`nagentically_validated=$validated"
