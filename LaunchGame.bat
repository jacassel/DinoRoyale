@echo off
cd /d "%~dp0"
start "Dinosaur Battle" "C:\Unreal Engine\UE_5.8\Engine\Binaries\Win64\UnrealEditor.exe" "%~dp0DinosaurBattle.uproject" -game -windowed -ResX=1600 -ResY=900 -nosplash
