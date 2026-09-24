param([string]$PackageRoot=$PSScriptRoot)
$ErrorActionPreference='Stop'
$root=(Resolve-Path -LiteralPath $PackageRoot).Path
$destination=Join-Path $root ('QA-Logs-'+(Get-Date -Format 'yyyyMMdd-HHmmss'))
New-Item -ItemType Directory -Path $destination | Out-Null
$logs=Join-Path $root 'DinosaurBattle\Saved\Logs'
if(Test-Path -LiteralPath $logs){Get-ChildItem -LiteralPath $logs -Filter '*.log' -File | Copy-Item -Destination $destination}
foreach($name in @('BUILD_INFO.txt','PACKAGE_SHA256.json')){
    $file=Join-Path $root $name
    if(Test-Path -LiteralPath $file){Copy-Item -LiteralPath $file -Destination $destination}
}
'Dino Royale QA logs. Share privately with the developer. Engine logs may include account identifiers. No OnlineServices.ini or login caches were copied.' | Set-Content -LiteralPath (Join-Path $destination 'READ_ME.txt')
Invoke-Item -LiteralPath $destination
