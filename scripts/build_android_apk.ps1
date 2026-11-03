$ErrorActionPreference = 'Stop'
$root = Split-Path -Parent $PSScriptRoot
$sdk = 'C:\Users\ralfk\AppData\Local\Android\Sdk'
$buildTools = Join-Path $sdk 'build-tools\36.0.0'
$platform = Join-Path $sdk 'platforms\android-35\android.jar'
$javac = 'C:\Program Files\Java\jdk-22\bin\javac.exe'
$out = Join-Path $root 'release-android\apk'
$work = Join-Path $out 'work'
Remove-Item -LiteralPath $work -Recurse -Force -ErrorAction SilentlyContinue
New-Item -ItemType Directory -Force -Path "$work\assets", "$work\lib\arm64-v8a", "$work\classes", $out | Out-Null
New-Item -ItemType Directory -Force -Path "$work\dex" | Out-Null
Copy-Item (Join-Path $root 'neu-lunari-german-v1.tsv') "$work\assets\neu-lunari-german-v1.tsv"
Copy-Item (Join-Path $root 'release-android\arm64-v8a\libleng_core.so') "$work\lib\arm64-v8a\libleng_core.so"
& $javac -cp $platform -d "$work\classes" (Join-Path $root 'android-app\src\org\leng\research\MainActivity.java')
if ($LASTEXITCODE -ne 0) { throw 'Java compilation failed.' }
& (Join-Path $buildTools 'd8.bat') --lib $platform --output "$work\dex" "$work\classes\org\leng\research\MainActivity.class"
if ($LASTEXITCODE -ne 0) { throw 'D8 failed.' }
& (Join-Path $buildTools 'aapt2.exe') link -I $platform --manifest (Join-Path $root 'android-app\AndroidManifest.xml') --min-sdk-version 24 --target-sdk-version 35 -o "$out\leng-unsigned.apk"
if ($LASTEXITCODE -ne 0) { throw 'AAPT2 link failed.' }
Copy-Item "$work\dex\classes.dex" "$work\classes.dex"
Push-Location $work
try { & 'C:\Program Files\Java\jdk-22\bin\jar.exe' uf "$out\leng-unsigned.apk" 'classes.dex' 'assets/neu-lunari-german-v1.tsv' 'lib/arm64-v8a/libleng_core.so' }
finally { Pop-Location }
if ($LASTEXITCODE -ne 0) { throw 'AAPT2 add failed.' }
$keystore = "$out\debug.keystore"
if (-not (Test-Path $keystore)) { & 'C:\Program Files\Java\jdk-22\bin\keytool.exe' -genkeypair -keystore $keystore -storepass android -keypass android -alias leng -keyalg RSA -keysize 2048 -validity 10000 -dname 'CN=Linguistische Emergenz NG'; if ($LASTEXITCODE -ne 0) { throw 'Keystore creation failed.' } }
& (Join-Path $buildTools 'zipalign.exe') -f 4 "$out\leng-unsigned.apk" "$out\leng-aligned.apk"
& (Join-Path $buildTools 'apksigner.bat') sign --ks $keystore --ks-pass pass:android --key-pass pass:android --out "$out\lunari-android-arm64.apk" "$out\leng-aligned.apk"
if ($LASTEXITCODE -ne 0) { throw 'APK signing failed.' }
& (Join-Path $buildTools 'apksigner.bat') verify --verbose "$out\lunari-android-arm64.apk"
