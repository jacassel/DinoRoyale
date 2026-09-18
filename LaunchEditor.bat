@echo off
cd /d "%~dp0"
set "UE_SDKS_ROOT=%~dp0Tools\Toolchain\AutoSDK"
start "Dinosaur Battle Editor" "C:\Unreal Engine\UE_5.8\Engine\Binaries\Win64\UnrealEditor.exe" "%~dp0DinosaurBattle.uproject"
