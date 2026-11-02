param(
  [string]$State = (Join-Path $PSScriptRoot '..\german-wordlist-only.leng'),
  [string]$Story = (Join-Path $PSScriptRoot '..\tests\reference_nur_noch_fuenf_minuten.txt')
)

$ErrorActionPreference = 'Stop'
$root = (Resolve-Path (Join-Path $PSScriptRoot '..')).Path
$binary = Join-Path $root 'release\linguistische-emergenz-ng.exe'
if (-not (Test-Path -LiteralPath $binary)) { throw 'Build the KR release before running this exchange test.' }
if (-not (Test-Path -LiteralPath $State)) { throw "State not found: $State" }
if (-not (Test-Path -LiteralPath $Story)) { throw "Story not found: $Story" }

function Get-Block([string[]]$Lines, [string]$Header, [string]$Footer) {
  $content = [string]::Join("`n", $Lines)
  $start = $content.IndexOf($Header, [System.StringComparison]::Ordinal)
  if ($start -lt 0) { throw "Missing report header: $Header" }
  $start += $Header.Length
  $end = $content.IndexOf($Footer, $start, [System.StringComparison]::Ordinal)
  if ($end -lt 0) { throw "Missing report footer: $Footer" }
  return $content.Substring($start, $end - $start)
}

# Both programs independently load the same frozen state. The sender is the
# only process receiving the German story. The receiver command below receives
# only `$lunari`, never `$source` or `$Story`.
$source = [System.IO.File]::ReadAllText((Resolve-Path $Story), [System.Text.UTF8Encoding]::new($false))
$before = (Get-FileHash -LiteralPath $State -Algorithm SHA256).Hash

$sender = & $binary --translate-text $State constructed $source
$lunari = Get-Block $sender "Lunari text:`n" "`n-- end Lunari text --"
if ($sender -notcontains 'newly learned tokens=0') {
  throw 'Frozen-lexicon exchange requires all sender words to already be present in the shared state.'
}

$receiver = & $binary --translate-text $State de $lunari
$received = Get-Block $receiver "Deutsch text:`n" "`n-- end translated text --"
$after = (Get-FileHash -LiteralPath $State -Algorithm SHA256).Hash
if ($before -ne $after) { throw 'The supposedly frozen shared lexicon changed during the exchange.' }
if ($received -cne $source) { throw 'Receiver did not reconstruct the sender story exactly from Lunari text.' }

Write-Output "PASS two-instance Lunari exchange: $(Split-Path $Story -Leaf)"
Write-Output "sender input: German story only"
Write-Output "receiver input: Lunari text only"
Write-Output "shared state SHA256: $before"
Write-Output "characters: source=$($source.Length), lunari=$($lunari.Length), recovered=$($received.Length)"
