param([int]$Width=1280,[int]$Height=720,[switch]$Offscreen,[switch]$StartMenu)
$projectRoot=(Resolve-Path (Join-Path $PSScriptRoot '..\..')).Path
$projectFile=Join-Path $projectRoot 'DinosaurBattle.uproject'
$bridgeDirectory=Join-Path $projectRoot 'Saved\Automation'
New-Item -ItemType Directory -Force -Path $bridgeDirectory | Out-Null
$telemetryFile=Join-Path $bridgeDirectory 'telemetry.json'
if(Test-Path -LiteralPath $telemetryFile){Remove-Item -LiteralPath $telemetryFile}
Set-Content -LiteralPath (Join-Path $bridgeDirectory 'command.json') -Value '{"seq":0,"cmd":"noop"}'
$arguments='"'+$projectFile+'" -game -windowed -ResX='+$Width+' -ResY='+$Height+' -WinX=20 -WinY=40 -nosplash -DinoDevBridge'
if($Offscreen){$arguments+=' -RenderOffscreen -unattended -ForceRes -ini:Engine:[Audio]:UnfocusedVolumeMultiplier=1.0'}
if($StartMenu){$arguments+=' -DinoStartMenu'}
$windowStyle=if($Offscreen){'Hidden'}else{'Normal'}
$gameProcess=Start-Process -FilePath 'C:\Unreal Engine\UE_5.8\Engine\Binaries\Win64\UnrealEditor.exe' -ArgumentList $arguments -WindowStyle $windowStyle -PassThru
$gameProcess.Id|Set-Content (Join-Path $projectRoot 'Saved\game.pid')
$deadline=(Get-Date).AddSeconds(60)
while(!(Test-Path -LiteralPath $telemetryFile) -and (Get-Date) -lt $deadline){Start-Sleep -Milliseconds 250;$gameProcess.Refresh();if($gameProcess.HasExited){throw 'Game exited before telemetry initialized'}}
if(!(Test-Path -LiteralPath $telemetryFile)){throw 'Game startup telemetry timed out; inspect the existing game process before retrying'}
Write-Output ('Game running: '+$gameProcess.Id)
