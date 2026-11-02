$ErrorActionPreference = 'Stop'

$projectRoot = Split-Path -Parent $PSScriptRoot
$outputZip = Join-Path $projectRoot 'release\Lunari-Developer-Windows.zip'
$stateFile = Join-Path $projectRoot 'german-wordlist-only.leng'
$textbookDirectory = Join-Path $projectRoot 'Lehrbuch'

if (-not (Test-Path -LiteralPath $stateFile)) {
    throw "Required local German wordlist state is missing: $stateFile"
}
if (-not (Test-Path -LiteralPath $textbookDirectory -PathType Container)) {
    throw "Required textbook directory is missing: $textbookDirectory"
}
if (Test-Path -LiteralPath $outputZip) {
    throw "Refusing to overwrite existing package: $outputZip"
}

$stage = Join-Path ([System.IO.Path]::GetTempPath()) ("lunari-developer-" + [guid]::NewGuid().ToString('N'))
$packageRoot = Join-Path $stage 'Lunari-Developer'
New-Item -ItemType Directory -Force -Path $packageRoot | Out-Null

try {
    Copy-Item -LiteralPath (Join-Path $projectRoot 'include') -Destination $packageRoot -Recurse
    Copy-Item -LiteralPath (Join-Path $projectRoot 'tests') -Destination $packageRoot -Recurse
    Copy-Item -LiteralPath $textbookDirectory -Destination $packageRoot -Recurse

    $sourceDestination = Join-Path $packageRoot 'src'
    New-Item -ItemType Directory -Force -Path $sourceDestination | Out-Null
    foreach ($source in @('engine.cpp', 'compute.cpp', 'main.cpp', 'win32_ui.cpp', 'user_ui.cpp', 'c_api.cpp')) {
        Copy-Item -LiteralPath (Join-Path $projectRoot (Join-Path 'src' $source)) -Destination $sourceDestination
    }

    $files = @(
        'CMakeLists.txt',
        'README.md',
        'BUILD.md',
        'GUI_BENUTZERHANDBUCH.md',
        'LUNARI_TEXT_TRANSLATION.md',
        'DEVELOPER_PACKAGE_README.md',
        'german-wordlist-only.leng'
    )
    foreach ($file in $files) {
        Copy-Item -LiteralPath (Join-Path $projectRoot $file) -Destination $packageRoot
    }

    $scriptDestination = Join-Path $packageRoot 'scripts'
    New-Item -ItemType Directory -Force -Path $scriptDestination | Out-Null
    foreach ($script in @('build_kr_release.ps1', 'test_lunari_text_roundtrip.ps1', 'test_lunari_two_instance_exchange.ps1', 'create_developer_zip.ps1')) {
        Copy-Item -LiteralPath (Join-Path $PSScriptRoot $script) -Destination $scriptDestination
    }

    $manifest = @(
        'Lunari Developer Package',
        'Bundled state: german-wordlist-only.leng',
        'State classification: local German wordlist-derived forms; no Wikidata content',
        'Bundled teaching materials: Lehrbuch/',
        ('State SHA256: ' + (Get-FileHash -LiteralPath $stateFile -Algorithm SHA256).Hash),
        ('State bytes: ' + (Get-Item -LiteralPath $stateFile).Length),
        ('Packaged UTC: ' + [DateTime]::UtcNow.ToString('o'))
    )
    Set-Content -LiteralPath (Join-Path $packageRoot 'PACKAGE_MANIFEST.txt') -Value $manifest -Encoding utf8

    New-Item -ItemType Directory -Force -Path (Split-Path -Parent $outputZip) | Out-Null
    Compress-Archive -LiteralPath $packageRoot -DestinationPath $outputZip -CompressionLevel Optimal
    Write-Output "Developer package created: $outputZip"
} finally {
    if (Test-Path -LiteralPath $stage) {
        Remove-Item -LiteralPath $stage -Recurse -Force
    }
}
