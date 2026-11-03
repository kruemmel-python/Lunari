$ErrorActionPreference = 'Stop'

$projectRoot = Split-Path -Parent $PSScriptRoot
$ndkRoot = 'C:\Users\ralfk\AppData\Local\Android\Sdk\ndk\27.2.12479018'
$clang = Join-Path $ndkRoot 'toolchains\llvm\prebuilt\windows-x86_64\bin\clang++.exe'
$output = Join-Path $projectRoot 'release-android\arm64-v8a'
$jniInclude = Join-Path $ndkRoot 'toolchains\llvm\prebuilt\windows-x86_64\sysroot\usr\include'
if (-not (Test-Path -LiteralPath $clang)) { throw "Android NDK C++ compiler not found: $clang" }
if (-not (Test-Path -LiteralPath (Join-Path $jniInclude 'jni.h'))) { throw "Android JNI headers not found: $jniInclude" }
New-Item -ItemType Directory -Force -Path $output | Out-Null

& $clang --target=aarch64-linux-android24 -std=c++20 -O2 -fPIC -shared -Wall -Wextra -Wpedantic -Werror '-I' (Join-Path $projectRoot 'include') '-I' $jniInclude (Join-Path $projectRoot 'src\engine.cpp') (Join-Path $projectRoot 'src\compute.cpp') (Join-Path $projectRoot 'src\android_jni.cpp') '-ldl' '-static-libstdc++' '-o' (Join-Path $output 'libleng_core.so')
if ($LASTEXITCODE -ne 0) { throw 'Android ARM64 core build failed.' }
& $clang --target=aarch64-linux-android24 -std=c++20 -O2 -Wall -Wextra -Wpedantic -Werror '-I' (Join-Path $projectRoot 'include') (Join-Path $projectRoot 'src\engine.cpp') (Join-Path $projectRoot 'src\compute.cpp') (Join-Path $projectRoot 'src\android_smoke.cpp') '-ldl' '-static-libstdc++' '-o' (Join-Path $output 'leng-device-smoke')
if ($LASTEXITCODE -ne 0) { throw 'Android ARM64 smoke build failed.' }
Write-Output "Android ARM64 core artifact created in $output"
