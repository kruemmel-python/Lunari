# Build

Der verbindlich geprüfte Windows-x64-Release wird mit `H:\KR-Compiler\KR-Compiler-Windows-x64-v4.3.1\toolchain\bin\g++.exe` gebaut:

```powershell
powershell -ExecutionPolicy Bypass -File .\scripts\build_kr_release.ps1
```

Der Vorgang erzeugt `release\linguistische-emergenz-ng.exe`, `release\linguistische-emergenz-ng-ui.exe` und `release\leng_api.dll`. Beide EXE-Dateien binden die GCC-, C++- und pthread-Laufzeit statisch ein. Ein isolierter GUI-Starttest mit einem auf Systemverzeichnisse beschränkten `PATH` war erfolgreich; der Importsatz enthält nur Windows-System-DLLs (`GDI32`, `KERNEL32`, `msvcrt`, `USER32`). `leng_api.dll` ist die dokumentierte native C-Schnittstelle für lokale Forschungssteuerungen.

Die Hardwareprüfung kann mit `release\linguistische-emergenz-ng.exe --compute-test` ausgeführt werden. Sie meldet explizit einen erfolgreichen GPU/CPU-Vergleich, einen CPU-Fallback-Skip oder einen Fehler.

Ein CMake-Entwicklerbuild mit MSYS2-MinGW bleibt für schnelle Tests verfügbar. Die konkreten Befehle stehen im README, seine Artefakte sind aber nicht als auslieferbarer Release vorgesehen.

Linux x64 wird mit der mitgelieferten Zig-C++-Toolchain gebaut:

```powershell
powershell -ExecutionPolicy Bypass -File .\scripts\build_kr_linux_release.ps1
```

Das erzeugt `release-linux\linguistische-emergenz-ng` als x86-64-ELF. Der Selbsttest wurde in der vorhandenen Ubuntu-WSL-Umgebung erfolgreich ausgeführt. Die Win32-Oberfläche ist naturgemäß kein Linux-Ziel; der portable Forschungskern, Persistenz, Export und CLI sind dort testbar.

Für Android ARM64 existiert ein nativer Core-Build mit dem vorhandenen Android NDK:

```powershell
powershell -ExecutionPolicy Bypass -File .\scripts\build_android_core.ps1
```

Er erzeugt `release-android\arm64-v8a\libleng_core.so` und den nativen Prüfling `leng-device-smoke` für `aarch64-linux-android24`. Der Prüfling wurde über ADB auf einem verbundenen ARM64-Gerät ausgeführt und prüfte Evolution, Agentenkommunikation sowie einen Speicher-/Lade-Roundtrip. Für eine Wiederholung: `adb push release-android/arm64-v8a/leng-device-smoke /data/local/tmp/` und anschließend `adb shell 'chmod 700 /data/local/tmp/leng-device-smoke && /data/local/tmp/leng-device-smoke'`.

Der KR Compiler ist als mögliche Build-Orchestrierung verfügbar; der aktuelle CMake-Referenzbuild verwendet noch keine `.kr`-Datei.
