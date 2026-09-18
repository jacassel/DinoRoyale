param([string]$Target='DinosaurBattleEditor',[string]$Configuration='Development')
$ErrorActionPreference='Stop'
$projectRoot = Split-Path $PSScriptRoot -Parent
$env:UE_SDKS_ROOT = Join-Path $PSScriptRoot 'Toolchain\AutoSDK'
$env:VCToolsTelemetryOptOut='1'
& 'C:\Unreal Engine\UE_5.8\Engine\Build\BatchFiles\Build.bat' $Target Win64 $Configuration "-Project=$projectRoot\DinosaurBattle.uproject" -WaitMutex -NoHotReloadFromIDE -NoUBA -MaxParallelActions=4
exit $LASTEXITCODE
