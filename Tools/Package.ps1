param([string]$Destination='Dist')
$ErrorActionPreference='Stop'
$projectRoot=(Resolve-Path (Join-Path $PSScriptRoot '..')).Path
$outputPath=Join-Path $projectRoot $Destination
$env:UE_SDKS_ROOT=Join-Path $PSScriptRoot 'Toolchain\AutoSDK'
$env:VCToolsTelemetryOptOut='1'
& 'C:\Unreal Engine\UE_5.8\Engine\Build\BatchFiles\RunUAT.bat' BuildCookRun "-project=$projectRoot\DinosaurBattle.uproject" -noP4 -platform=Win64 -clientconfig=Development -nocompileeditor -build -cook -stage -pak -iostore -archive "-archivedirectory=$outputPath" '-ubtargs=-MaxParallelActions=2 -NoUBA -NoHotReloadFromIDE' -unattended -utf8output
exit $LASTEXITCODE
