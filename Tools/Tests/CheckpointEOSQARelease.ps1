param([string]$ReleaseName='DinoRoyale-20260926-QA2',[string]$EvidenceDirectory='Tests/Results/eos-qa-20260926')
$ErrorActionPreference='Stop'
$projectRoot=(Resolve-Path (Join-Path $PSScriptRoot '../..')).Path
$release=(Resolve-Path (Join-Path $projectRoot ('Dist/Releases/'+$ReleaseName+'/Windows'))).Path
if(!$release.StartsWith((Join-Path $projectRoot 'Dist\Releases\'),[StringComparison]::OrdinalIgnoreCase)){throw 'Release must be within project Dist/Releases'}
if(Test-Path -LiteralPath (Join-Path $release 'DinosaurBattle/OnlineServices.ini')){throw 'Move the test-only configured OnlineServices.ini out before making a distributable zip'}
$running=Get-CimInstance Win32_Process -Filter "Name='DinosaurBattle.exe'" | Where-Object {$_.ExecutablePath -and $_.ExecutablePath.StartsWith($release,[StringComparison]::OrdinalIgnoreCase)}
if($running){throw 'Close the QA package before checkpointing'}
$saved=Join-Path $release 'DinosaurBattle/Saved'
if(Test-Path -LiteralPath $saved){
    $testArchive=Join-Path $projectRoot ('Dist/TestRuns/'+$ReleaseName+'-'+(Get-Date -Format 'yyyyMMdd-HHmmss'))
    New-Item -ItemType Directory -Path $testArchive | Out-Null
    $resolvedSaved=(Resolve-Path -LiteralPath $saved).Path
    $resolvedArchive=(Resolve-Path -LiteralPath $testArchive).Path
    if(!$resolvedSaved.StartsWith($release+'\',[StringComparison]::OrdinalIgnoreCase) -or !$resolvedArchive.StartsWith((Join-Path $projectRoot 'Dist\TestRuns\'),[StringComparison]::OrdinalIgnoreCase)){throw 'Unsafe Saved archive path'}
    Move-Item -LiteralPath $resolvedSaved -Destination (Join-Path $resolvedArchive 'Saved')
}
$manifest=@(Get-ChildItem -LiteralPath $release -Recurse -File | Where-Object {$_.Name -ne 'PACKAGE_SHA256.json'} | ForEach-Object {
    [pscustomobject]@{path=$_.FullName.Substring($release.Length+1);sha256=(Get-FileHash -LiteralPath $_.FullName -Algorithm SHA256).Hash;bytes=$_.Length}
})
$manifest | ConvertTo-Json | Set-Content -LiteralPath (Join-Path $release 'PACKAGE_SHA256.json')
$recovery=Join-Path $projectRoot ('Dist/Checkpoints/'+$ReleaseName+'/Windows')
if(Test-Path -LiteralPath $recovery){throw 'Recovery already exists; do not overwrite a checkpoint'}
New-Item -ItemType Directory -Path $recovery | Out-Null
Get-ChildItem -LiteralPath $release | Copy-Item -Destination $recovery -Recurse
foreach($row in $manifest){
    if((Get-FileHash -LiteralPath (Join-Path $recovery $row.path)).Hash -ne $row.sha256){throw "Recovery mismatch: $($row.path)"}
}
$zip=Join-Path $projectRoot ('Dist/Releases/'+$ReleaseName+'.zip')
if(Test-Path -LiteralPath $zip){throw 'Release zip already exists'}
Add-Type -AssemblyName System.IO.Compression.FileSystem
[IO.Compression.ZipFile]::CreateFromDirectory($release,$zip,[IO.Compression.CompressionLevel]::Optimal,$true)
$archive=[IO.Compression.ZipFile]::OpenRead($zip)
try {
    foreach($row in $manifest){
        $entry=$archive.GetEntry('Windows/'+$row.path.Replace('\','/'))
        if(!$entry){throw "Zip missing file: $($row.path)"}
        $stream=$entry.Open();$sha=[Security.Cryptography.SHA256]::Create()
        try {$hash=[BitConverter]::ToString($sha.ComputeHash($stream)).Replace('-','')} finally {$stream.Dispose();$sha.Dispose()}
        if($hash -ne $row.sha256){throw "Zip hash mismatch: $($row.path)"}
    }
} finally {$archive.Dispose()}
$evidence=Join-Path $projectRoot $EvidenceDirectory
Copy-Item -LiteralPath (Join-Path $release 'PACKAGE_SHA256.json') -Destination (Join-Path $evidence 'release-manifest.json')
[pscustomobject]@{release=$release;recovery=$recovery;filesVerified=$manifest.Count;zip=$zip;zipSHA256=(Get-FileHash -LiteralPath $zip).Hash;zipEveryRuntimeFileVerified=$true;distributionHasSaved=(Test-Path -LiteralPath $saved)} | ConvertTo-Json | Set-Content -LiteralPath (Join-Path $evidence 'checkpoint.json')
Write-Output "Release, independent recovery, and zip verified: $($manifest.Count) runtime/document files."
