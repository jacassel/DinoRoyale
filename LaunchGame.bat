@echo off
cd /d "%~dp0"
if exist "%~dp0Dist\Releases\DinoRoyale-0.6\Windows\DinosaurBattle.exe" (
    start "Dino Royale" "%~dp0Dist\Releases\DinoRoyale-0.6\Windows\DinosaurBattle.exe" -windowed -ResX=1600 -ResY=900
    exit /b
)
if exist "%~dp0Dist\Releases\DinoRoyale-0.5\Windows\DinosaurBattle.exe" (
    start "Dino Royale" "%~dp0Dist\Releases\DinoRoyale-0.5\Windows\DinosaurBattle.exe" -windowed -ResX=1600 -ResY=900
    exit /b
)
if exist "%~dp0Dist\Releases\DinoRoyale-0.3-Alpha-Test\Windows\DinosaurBattle.exe" (
    start "Dino Royale" "%~dp0Dist\Releases\DinoRoyale-0.3-Alpha-Test\Windows\DinosaurBattle.exe" -windowed -ResX=1600 -ResY=900
    exit /b
)
if exist "%~dp0Dist\Releases\DinoRoyale-20260926-QA2-ForCairnFriends\Windows\DinosaurBattle.exe" (
    start "Dino Royale" "%~dp0Dist\Releases\DinoRoyale-20260926-QA2-ForCairnFriends\Windows\DinosaurBattle.exe" -windowed -ResX=1600 -ResY=900
    exit /b
)
if exist "%~dp0Dist\Releases\DinoRoyale-20260926-QA2\Windows\DinosaurBattle.exe" (
    start "Dino Royale" "%~dp0Dist\Releases\DinoRoyale-20260926-QA2\Windows\DinosaurBattle.exe" -windowed -ResX=1600 -ResY=900
    exit /b
)
if exist "%~dp0Dist\Releases\DinoRoyale-20260924-QA1\Windows\DinosaurBattle.exe" (
    start "Dino Royale" "%~dp0Dist\Releases\DinoRoyale-20260924-QA1\Windows\DinosaurBattle.exe" -windowed -ResX=1600 -ResY=900
    exit /b
)
if exist "%~dp0Dist\Windows\DinosaurBattle.exe" (
    start "Dino Royale" "%~dp0Dist\Windows\DinosaurBattle.exe" -windowed -ResX=1600 -ResY=900
    exit /b
)
start "Dino Royale" "C:\Unreal Engine\UE_5.8\Engine\Binaries\Win64\UnrealEditor.exe" "%~dp0DinosaurBattle.uproject" -game -windowed -ResX=1600 -ResY=900 -nosplash
