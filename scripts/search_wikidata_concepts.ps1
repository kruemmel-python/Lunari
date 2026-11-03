[CmdletBinding()]
param(
  [Parameter(Mandatory = $true, Position = 0)]
  [ValidateNotNullOrEmpty()]
  [string[]]$Search,
  [ValidateRange(1, 20)]
  [int]$Limit = 5,
  [string]$Language = 'en'
)

$ErrorActionPreference = 'Stop'
if ($Language -notmatch '^[a-z]{2,12}(-[a-z0-9]+)?$') { throw 'Language must be a simple BCP-47 language tag.' }

foreach ($term in $Search) {
  if ([string]::IsNullOrWhiteSpace($term)) { throw 'Search terms may not be empty.' }
  $escaped = [uri]::EscapeDataString($term)
  $uri = "https://www.wikidata.org/w/api.php?action=wbsearchentities&search=$escaped&language=$Language&limit=$Limit&format=json&origin=*"
  $response = Invoke-RestMethod -Uri $uri -Method Get -Headers @{ 'User-Agent' = 'Linguistische-Emergenz-NG/0.1 (research lexicon discovery)' }
  foreach ($result in $response.search) {
    [pscustomobject]@{
      query       = $term
      id          = $result.id
      label       = $result.label
      description = $result.description
      url         = "https://www.wikidata.org/wiki/$($result.id)"
    }
  }
}
