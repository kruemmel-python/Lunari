$ErrorActionPreference = 'Stop'

$projectRoot = Split-Path -Parent $PSScriptRoot
$krRoot = 'H:\KR-Compiler\KR-Compiler-Windows-x64-v4.3.1'
$zig = Join-Path $krRoot 'toolchain\zig\zig-x86_64-windows-0.17.0\zig.exe'
$release = Join-Path $projectRoot 'release-linux'
if (-not (Test-Path -LiteralPath $zig)) { throw "KR Zig toolchain not found: $zig" }
New-Item -ItemType Directory -Force -Path $release | Out-Null

& $zig c++ '-target' 'x86_64-linux-gnu' '-std=c++20' '-O2' '-Wall' '-Wextra' '-Wpedantic' '-Werror' '-I' (Join-Path $projectRoot 'include') (Join-Path $projectRoot 'src\engine.cpp') (Join-Path $projectRoot 'src\compute.cpp') (Join-Path $projectRoot 'src\main.cpp') '-ldl' '-o' (Join-Path $release 'linguistische-emergenz-ng')
if ($LASTEXITCODE -ne 0) { throw 'KR Linux release build failed.' }
Write-Output "KR Linux release artifact created in $release"
