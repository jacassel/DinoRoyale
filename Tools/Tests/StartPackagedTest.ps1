param([Parameter(Mandatory=$true)][string]$PackageDirectory,[string]$ExtraArguments='')
$ErrorActionPreference='Stop'
$packageRoot=(Resolve-Path -LiteralPath $PackageDirectory).Path
$gameExe=Join-Path $packageRoot 'DinosaurBattle.exe'
if(!(Test-Path -LiteralPath $gameExe)){throw 'Packaged executable missing'}
$bridgeDirectory=Join-Path $packageRoot 'DinosaurBattle\Saved\Automation'
New-Item -ItemType Directory -Force -Path $bridgeDirectory | Out-Null
$telemetryFile=Join-Path $bridgeDirectory 'telemetry.json'
if(Test-Path -LiteralPath $telemetryFile){Remove-Item -LiteralPath $telemetryFile}
Set-Content -LiteralPath (Join-Path $bridgeDirectory 'command.json') -Value '{"seq":0,"cmd":"noop"}'
$arguments='-windowed -ResX=1280 -ResY=720 -DinoDevBridge -DinoStartMenu -RenderOffscreen -ForceRes -unattended -ini:Engine:[Audio]:UnfocusedVolumeMultiplier=1.0'
if($ExtraArguments){$arguments+=' '+$ExtraArguments}
$gameProcess=Start-Process -FilePath $gameExe -ArgumentList $arguments -WindowStyle Hidden -PassThru
$deadline=(Get-Date).AddSeconds(60)
while(!(Test-Path -LiteralPath $telemetryFile) -and (Get-Date) -lt $deadline){Start-Sleep -Milliseconds 250}
if(!(Test-Path -LiteralPath $telemetryFile)){throw 'Packaged telemetry timed out; inspect before retrying'}
Write-Output ('Packaged launcher PID: '+$gameProcess.Id+'; bridge: '+$bridgeDirectory)
