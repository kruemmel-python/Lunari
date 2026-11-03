[CmdletBinding()]
param(
  [Parameter(Mandatory = $true)] [string]$Output,
  [ValidateRange(1, 1000000)] [int]$Count = 100000
)

# This fixture is intentionally marked agent-hypothesis.  It is only for RAM,
# index, persistence and search measurements; it cannot be reported as an
# external Wikidata import or a qualified research corpus.
$ErrorActionPreference = 'Stop'
$path = [IO.Path]::GetFullPath($Output)
[IO.Directory]::CreateDirectory([IO.Path]::GetDirectoryName($path)) | Out-Null
$writer = [IO.StreamWriter]::new($path, $false, [Text.UTF8Encoding]::new($false), 65536)
try {
  $writer.WriteLine("external_id`tlabel`tpart_of_speech`tsemantics`trelations`tsource`tprovenance`tsource_revision`tevidence_class")
  for ($i = 1; $i -le $Count; ++$i) {
    $id = "STRESS_$i"; $label = "scaling concept $i"; $relation = if ($i -gt 1) { "related-to:STRESS_$($i - 1)" } else { '' }
    $writer.WriteLine("$id`t$label`tnoun`tscaling measurement concept $i`t$relation`tScaleTest`tNON-EVIDENCE;generated-for-capacity-test`tscale-v1`tagent-hypothesis")
  }
} finally { $writer.Dispose() }
Write-Output "Created non-evidentiary $Count-row scaling fixture: $path"
