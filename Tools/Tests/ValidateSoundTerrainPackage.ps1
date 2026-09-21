# Build a separate candidate only after source validation; never overwrite Dist/Windows.
param([switch]$ResumeAfterAudio)
$ErrorActionPreference='Stop'
$projectRoot=(Resolve-Path (Join-Path $PSScriptRoot '..\..')).Path
Set-Location -LiteralPath $projectRoot
$evidence=Join-Path $projectRoot 'Tests\Results\sound-terrain'
$source=Get-Content -Raw -LiteralPath (Join-Path $evidence 'regression\summary.json') | ConvertFrom-Json
if($source.Count -ne 15 -or ($source | Where-Object {!$_.passed}).Count -or ($source | Measure-Object checks -Sum).Sum -ne 462){throw 'Full source regression has not passed'}
$candidate=Join-Path $projectRoot 'Dist\SoundTerrainCandidate\Windows'
$env:DINO_BRIDGE_DIR=if($ResumeAfterAudio){Join-Path $candidate 'DinosaurBattle\Saved\Automation'}else{Join-Path $projectRoot 'Saved\Automation'}
if($ResumeAfterAudio){
    foreach($gate in @(@('audio.json',54),@('audio-mix.json',2))){
        $rows=Get-Content -Raw -LiteralPath (Join-Path $evidence ('package-audio\'+$gate[0])) | ConvertFrom-Json
        if($rows.Count -ne $gate[1] -or ($rows | Where-Object {!$_.passed}).Count){throw 'Packaged audio must pass before resuming'}
    }
}
python -c "import sys;sys.path.insert(0,'Tools/Tests');import runtime_core as t;t.command('quit')"
if($LASTEXITCODE -ne 0){throw 'Current test game did not accept quit'}
Start-Sleep -Seconds 2
if(!$ResumeAfterAudio){
Write-Output 'BUILDING SEPARATE PACKAGE'
powershell -ExecutionPolicy Bypass -File Tools/Package.ps1 -Destination Dist/SoundTerrainCandidate *> (Join-Path $evidence 'package-build.log')
if($LASTEXITCODE -ne 0){throw 'BuildCookRun failed; inspect package-build.log'}
Copy-Item -LiteralPath 'Assets\Audio\CREDITS.md' -Destination (Join-Path $candidate 'AUDIO_CREDITS.md')
}
powershell -ExecutionPolicy Bypass -File Tools/Tests/StartPackagedTest.ps1 -PackageDirectory $candidate -Width 1600 -Height 900
if($LASTEXITCODE -ne 0){throw 'Candidate launch failed'}
$env:DINO_BRIDGE_DIR=Join-Path $candidate 'DinosaurBattle\Saved\Automation'
$env:DINO_TEST_WIDTH='1600';$env:DINO_TEST_HEIGHT='900'
foreach($suite in @(
    @('runtime_audio.py','package-audio',''),
    @('runtime_audio_mix.py','package-audio',''),
    @('run_visual_regression.py','package-regression','--package'),
    @('runtime_world_visuals.py','package-world',''),
    @('runtime_visual_performance.py','package-performance',''),
    @('runtime_world_soak.py','','0')
)){
    if($ResumeAfterAudio -and $suite[1] -eq 'package-audio'){continue}
    $env:DINO_RESULTS_DIR=if($suite[1]){Join-Path $evidence $suite[1]}else{$evidence}
    New-Item -ItemType Directory -Force -Path $env:DINO_RESULTS_DIR | Out-Null
    Write-Output ('PACKAGE CHECK '+$suite[0])
    $scriptFile=Join-Path $PSScriptRoot $suite[0]
    $log=Join-Path $env:DINO_RESULTS_DIR ($suite[0]+'.log')
    if($suite[2]){python $scriptFile $suite[2] *> $log}else{python $scriptFile *> $log}
    if($LASTEXITCODE -ne 0){Get-Content -LiteralPath $log -Tail 15;throw ('Packaged check failed: '+$suite[0])}
    if($suite[0] -eq 'runtime_audio_mix.py'){
        # duelSetup deliberately disables respawns and changes the population.
        # A fresh process restores all gameplay defaults before regression/world checks.
        python -c "import sys;sys.path.insert(0,'Tools/Tests');import runtime_core as t;t.command('quit')"
        if($LASTEXITCODE -ne 0){throw 'Audio test game did not accept quit'}
        Start-Sleep -Seconds 2
        powershell -ExecutionPolicy Bypass -File Tools/Tests/StartPackagedTest.ps1 -PackageDirectory $candidate -Width 1600 -Height 900
        if($LASTEXITCODE -ne 0){throw 'Fresh candidate launch failed'}
    }
}
python -c "import sys;sys.path.insert(0,'Tools/Tests');import runtime_core as t;t.command('quit')"
if($LASTEXITCODE -ne 0){throw 'Candidate did not accept quit'}
Write-Output 'CANDIDATE VALIDATED; REVIEW RESULTS BEFORE PROMOTION'
