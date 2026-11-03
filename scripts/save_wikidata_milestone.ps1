[CmdletBinding()]
param(
  [Parameter(Mandatory = $true)] [string]$Source,
  [Parameter(Mandatory = $true)] [string]$State,
  [Parameter(Mandatory = $true)] [ValidateRange(1, 100000)] [int]$Count
)

# A milestone is deliberately rebuilt from a prefix of the genuine source,
# never copied from a larger graph.  Its state therefore proves the requested
# source cardinality independently of later intake.
$ErrorActionPreference = 'Stop'
$root = Split-Path -Parent $PSScriptRoot
$sourcePath = [IO.Path]::GetFullPath($Source); $statePath = [IO.Path]::GetFullPath($State)
$binary = Join-Path $root 'release\linguistische-emergenz-ng.exe'
if (-not (Test-Path -LiteralPath $sourcePath)) { throw "Source missing: $sourcePath" }
if ((Get-Content -LiteralPath $sourcePath | Measure-Object -Line).Lines -lt ($Count + 1)) { throw "Source has fewer than $Count data rows." }
$prefix = [IO.Path]::ChangeExtension($statePath, '.tsv')
$reader = [IO.StreamReader]::new($sourcePath); $writer = [IO.StreamWriter]::new($prefix, $false, [Text.UTF8Encoding]::new($false))
try { for ($i = 0; $i -le $Count; ++$i) { $writer.WriteLine($reader.ReadLine()) } } finally { $writer.Dispose(); $reader.Dispose() }
& $binary --run $statePath 1 | Out-Null; if ($LASTEXITCODE -ne 0) { throw 'Could not create milestone research state.' }
& $binary --import-concepts $statePath $prefix; if ($LASTEXITCODE -ne 0) { throw 'Could not import milestone source.' }
& $binary --lexicon-status $statePath
