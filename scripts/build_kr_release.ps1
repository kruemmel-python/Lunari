$ErrorActionPreference = 'Stop'

$projectRoot = Split-Path -Parent $PSScriptRoot
$krRoot = if ($env:KR_COMPILER_ROOT) {
    $env:KR_COMPILER_ROOT
} else {
    'H:\KR-Compiler\KR-Compiler-Windows-x64-v4.3.1'
}
$compiler = Join-Path $krRoot 'toolchain\bin\g++.exe'
$release = Join-Path $projectRoot 'release'
if (-not (Test-Path -LiteralPath $compiler)) { throw "KR C++ toolchain not found: $compiler" }
New-Item -ItemType Directory -Force -Path $release | Out-Null
$env:PATH = (Join-Path $krRoot 'toolchain\bin') + ';' + $env:PATH

$common = @('-std=c++20', '-O2', '-DNDEBUG', '-Wall', '-Wextra', '-Wpedantic', '-Werror', '-static', '-static-libgcc', '-static-libstdc++', '-I', (Join-Path $projectRoot 'include'), (Join-Path $projectRoot 'src\engine.cpp'), (Join-Path $projectRoot 'src\compute.cpp'), (Join-Path $projectRoot 'src\lunari_grammar.cpp'))
& $compiler @common (Join-Path $projectRoot 'src\main.cpp') '-o' (Join-Path $release 'linguistische-emergenz-ng.exe')
if ($LASTEXITCODE -ne 0) { throw 'KR console release build failed.' }

& $compiler @common '-mwindows' (Join-Path $projectRoot 'src\win32_ui.cpp') '-lgdi32' '-luser32' '-lcomctl32' '-lcomdlg32' '-o' (Join-Path $release 'linguistische-emergenz-ng-ui.exe')
if ($LASTEXITCODE -ne 0) { throw 'KR GUI release build failed.' }

& $compiler @common '-mwindows' (Join-Path $projectRoot 'src\user_ui.cpp') '-pthread' '-lgdi32' '-luser32' '-lcomctl32' '-o' (Join-Path $release 'linguistische-emergenz-ng-user.exe')
if ($LASTEXITCODE -ne 0) { throw 'KR user GUI release build failed.' }

& $compiler @common '-mwindows' (Join-Path $projectRoot 'src\developer_comment_ui.cpp') '-lgdi32' '-luser32' '-lshell32' '-lole32' '-o' (Join-Path $release 'lunari-developer-comments.exe')
if ($LASTEXITCODE -ne 0) { throw 'KR developer comments GUI release build failed.' }

& $compiler @common '-shared' (Join-Path $projectRoot 'src\c_api.cpp') '-DLENG_API_BUILD' '-o' (Join-Path $release 'leng_api.dll')
if ($LASTEXITCODE -ne 0) { throw 'KR C API release build failed.' }

Write-Output "KR release artifacts created in $release"
