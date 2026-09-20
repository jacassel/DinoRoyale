# Run only after the candidate package has passed its recorded gameplay checks.
$ErrorActionPreference='Stop'
$projectRoot=(Resolve-Path (Join-Path $PSScriptRoot '..\..')).Path
$evidence=Join-Path $projectRoot 'Tests\Results\visual-modernization'
foreach($gate in @(@('regression',462),@('package-regression',220))){
    $rows=Get-Content -Raw -LiteralPath (Join-Path $evidence ($gate[0]+'\summary.json')) | ConvertFrom-Json
    if(($rows | Where-Object { !$_.passed }).Count -or ($rows | Measure-Object checks -Sum).Sum -ne $gate[1]){throw ('Regression gate failed: '+$gate[0])}
}
$soak=Get-Content -Raw -LiteralPath (Join-Path $evidence 'world-soak.json') | ConvertFrom-Json
if(!$soak -or ($soak | Where-Object { !$_.passed }).Count){throw 'World observation gate failed'}
$candidate=Join-Path $projectRoot 'Dist\VisualCandidate\Windows'
$recovery=Join-Path $projectRoot 'Dist\Checkpoints\2026-09-20-visual-modernization\Windows'
$staging=Join-Path $projectRoot 'Dist\VisualPromotion\Windows'
$previous=Join-Path $projectRoot 'Dist\Checkpoints\2026-09-20-before-visual-modernization\Windows'
$playable=Join-Path $projectRoot 'Dist\Windows'
# Resolve every source/destination before recursive copy or directory move.
foreach($targetPath in @($candidate,$recovery,$staging,$previous,$playable)){
    $absolute=[IO.Path]::GetFullPath($targetPath)
    if(!$absolute.StartsWith(($projectRoot+'\Dist\'),[StringComparison]::OrdinalIgnoreCase)){throw ('Outside workspace Dist: '+$absolute)}
}
foreach($targetPath in @($recovery,$staging,$previous)){if(Test-Path -LiteralPath $targetPath){throw ('Destination already exists; inspect before retrying: '+$targetPath)}}
if(!(Test-Path -LiteralPath (Join-Path $candidate 'DinosaurBattle.exe'))){throw 'Candidate missing'}
$files=Get-ChildItem -LiteralPath $candidate -Recurse -File | Where-Object {$_.FullName -notmatch '\\Saved\\'}
$manifest=@()
foreach($file in $files){
    $relative=$file.FullName.Substring($candidate.Length+1)
    $sha=(Get-FileHash -LiteralPath $file.FullName -Algorithm SHA256).Hash
    foreach($destination in @($recovery,$staging)){
        $copy=Join-Path $destination $relative
        New-Item -ItemType Directory -Force -Path (Split-Path $copy) | Out-Null
        Copy-Item -LiteralPath $file.FullName -Destination $copy
        if((Get-FileHash -LiteralPath $copy -Algorithm SHA256).Hash -ne $sha){throw ('Copy hash mismatch: '+$copy)}
    }
    $manifest+=@{path=$relative;bytes=$file.Length;sha256=$sha}
}
New-Item -ItemType Directory -Force -Path (Split-Path $previous) | Out-Null
Move-Item -LiteralPath $playable -Destination $previous
Move-Item -LiteralPath $staging -Destination $playable
foreach($file in $manifest){if((Get-FileHash -LiteralPath (Join-Path $playable $file.path) -Algorithm SHA256).Hash -ne $file.sha256){throw ('Promoted hash mismatch: '+$file.path)}}
@{files=$manifest;count=$manifest.Count;candidate=$candidate;recovery=$recovery;previous=$previous;playable=$playable;verifiedAt=(Get-Date).ToString('o')} | ConvertTo-Json -Depth 5 | Set-Content -LiteralPath (Join-Path $evidence 'stable-package-manifest.json')
Write-Output ('Verified and promoted '+$manifest.Count+' runtime files; old playable retained at '+$previous)
