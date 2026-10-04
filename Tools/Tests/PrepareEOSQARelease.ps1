param([string]$ReleaseDirectory='Dist/Releases/DinoRoyale-20260926-QA2/Windows')
$ErrorActionPreference='Stop'
$projectRoot=(Resolve-Path (Join-Path $PSScriptRoot '../..')).Path
$release=(Resolve-Path (Join-Path $projectRoot $ReleaseDirectory)).Path
$releaseName=Split-Path (Split-Path $release -Parent) -Leaf
if(!$release.StartsWith((Join-Path $projectRoot 'Dist\Releases\'),[StringComparison]::OrdinalIgnoreCase)){throw 'Expected a release under this project Dist/Releases'}
$header=[IO.File]::ReadAllText((Join-Path $projectRoot 'Source/DinosaurBattle/DinoCompatibility.h'))
$compat=[regex]::Match($header,'Build\s*=\s*(\d+)').Groups[1].Value
if(!$compat){throw 'Compatibility constant missing'}
$engine=[IO.File]::ReadAllText((Join-Path $projectRoot 'Config/DefaultEngine.ini'))
if($engine -notmatch "(?m)^BuildIdOverride=$compat\s*$"){throw 'OSS build override differs from source'}
foreach($name in @('DinosaurBattle.exe','DinosaurBattle/Binaries/Win64/DinosaurBattle.exe','Engine/Binaries/Win64/EOSSDK-Win64-Shipping.dll')){
    if(!(Test-Path -LiteralPath (Join-Path $release $name))){throw "Missing required package file: $name"}
}
foreach($name in @('README.md','FRIEND_QUICKSTART.md','EOS_SETUP.md','MULTIPLAYER_QA_REPORT.md','RELEASE_READINESS.md','KNOWN_ISSUES.md')){
    Copy-Item -LiteralPath (Join-Path $projectRoot $name) -Destination $release
}
Copy-Item -LiteralPath (Join-Path $projectRoot 'OnlineServices.example.ini') -Destination (Join-Path $release 'DinosaurBattle/OnlineServices.example.ini')
Copy-Item -LiteralPath (Join-Path $projectRoot 'Assets/Audio/CREDITS.md') -Destination (Join-Path $release 'AUDIO_CREDITS.md')
Copy-Item -LiteralPath (Join-Path $PSScriptRoot 'CollectQALogs.ps1') -Destination $release
@'
@echo off
cd /d "%~dp0"
start "Dino Royale" "%~dp0DinosaurBattle.exe" -windowed -ResX=1600 -ResY=900
'@ | Set-Content -LiteralPath (Join-Path $release 'Play Dino Royale.bat') -Encoding ascii
@'
@echo off
powershell -NoProfile -ExecutionPolicy Bypass -File "%~dp0CollectQALogs.ps1" -PackageRoot "%~dp0."
if errorlevel 1 pause
'@ | Set-Content -LiteralPath (Join-Path $release 'Collect QA Logs.bat') -Encoding ascii
@"
Dino Royale - Version 0.3 Alpha Test
Release: $releaseName
Game compatibility build: $compat
Authoritative source: DinoCompatibility::Build / DINO_BUILD (EOS Int64)
Unreal OSS BuildUniqueId override: $compat
Unreal network checksum protection: ENABLED (no override or bypass)
Build type: Windows Development, complete BuildCookRun build/cook/stage/archive

Use this WHOLE fresh Windows folder on both PCs, not an overlay of an older install.
Both PCs must use this 0.3 release; older 0.2 clients have a different compatibility ID.

See MULTIPLAYER_QA_REPORT.md for completed tests and explicit limitations.
The owner reports the prior release worked in real multiplayer. This update has
local multi-process regression evidence; a fresh physical/WAN playtest remains separate.
Configured OnlineServices.ini is NOT included in this distribution.
Copy your existing file to Windows/DinosaurBattle/OnlineServices.ini on BOTH PCs.
Do not change its values. Sign in separately on each PC.
Internal executable/project/EOS identity remain DinosaurBattle; player name is Dino Royale.
Epic organization/product names were not changed. No purchases were made.
"@ | Set-Content -LiteralPath (Join-Path $release 'BUILD_INFO.txt') -Encoding utf8
Write-Output "Prepared $releaseName documents and launchers; compatibility $compat."
