"""Copy a tested candidate into a fresh release, recovery and verified public ZIP.

Never overwrites a package. Configured online values stay local; no Saved caches,
logs or debug symbols enter the distribution. Run after gameplay acceptance.
"""
import argparse,pathlib,shutil,json,hashlib,zipfile,subprocess
ROOT=pathlib.Path(__file__).resolve().parents[1]
p=argparse.ArgumentParser();p.add_argument('--candidate',required=True);p.add_argument('--name',default='DinoRoyale-0.5');a=p.parse_args()
dist=(ROOT/'Dist').resolve();source=(ROOT/a.candidate).resolve();release=dist/'Releases'/a.name/'Windows';recovery=dist/'Checkpoints'/a.name/'Windows';archive=dist/'Releases'/(a.name+'.zip');evidence=ROOT/'Tests/Results/roster05/release'
for path in [source,release,recovery,archive]:
 if not path.resolve().is_relative_to(dist) or path.resolve()==dist:raise RuntimeError('Path outside Dist')
if any(x.exists() for x in [release,recovery,archive]):raise RuntimeError('Release/checkpoint already exists; preserve it')
if not (source/'DinosaurBattle.exe').exists():raise RuntimeError('Missing candidate bootstrap')
def digest(path):
 with path.open('rb') as f:return hashlib.file_digest(f,'sha256').hexdigest()
for src in source.rglob('*'):
 if not src.is_file():continue
 relative=src.relative_to(source)
 if 'Saved' in relative.parts or src.name.lower()=='onlineservices.ini' or src.suffix.lower() in ['.pdb','.log']:continue
 dst=release/relative;dst.parent.mkdir(parents=True,exist_ok=True);shutil.copy2(src,dst)
for name in ['README.md','FRIEND_QUICKSTART.md','EOS_SETUP.md','KNOWN_ISSUES.md','VERSION05_RELEASE_NOTES.md','VERSION05_TEST_REPORT.md','VERSION05_BALANCE_REPORT.md']:
 shutil.copy2(ROOT/name,release/name)
shutil.copy2(ROOT/'Assets/Audio/CREDITS.md',release/'AUDIO_CREDITS.md')
shutil.copy2(ROOT/'Tools/Tests/CollectQALogs.ps1',release/'CollectQALogs.ps1')
shutil.copy2(ROOT/'OnlineServices.example.ini',release/'DinosaurBattle/OnlineServices.example.ini')
(release/'Play Dino Royale.bat').write_text('@echo off\ncd /d "%~dp0"\nstart "Dino Royale" "%~dp0DinosaurBattle.exe" -windowed -ResX=1600 -ResY=900\n')
(release/'Collect QA Logs.bat').write_text('@echo off\npowershell -NoProfile -ExecutionPolicy Bypass -File "%~dp0CollectQALogs.ps1" -PackageRoot "%~dp0."\nif errorlevel 1 pause\n')
commit=subprocess.check_output(['git','rev-parse','HEAD'],cwd=ROOT,text=True).strip()
(release/'BUILD_INFO.txt').write_text(f'Dino Royale Version 0.5\nCompatibility: 2026100505\nSource commit: {commit}\nWindows Development / Unreal 5.8.2\nUse the entire fresh Windows folder on every PC.\nThe supplied configured OnlineServices.ini is installed locally; public ZIPs exclude credentials.\nCopy your existing configured file into Windows/DinosaurBattle before online play.\nThe owner confirms prior successful multiplayer across different networks.\nThis update has separate local multi-process regression evidence.\nSee VERSION05_TEST_REPORT.md for exact verification and limitations.\n')
manifest=[dict(path=f.relative_to(release).as_posix(),bytes=f.stat().st_size,sha256=digest(f)) for f in sorted(release.rglob('*')) if f.is_file()]
(release/'PACKAGE_SHA256.json').write_text(json.dumps(manifest,indent=2))
shutil.copytree(release,recovery)
for row in manifest:
 if digest(recovery/row['path'])!=row['sha256']:raise RuntimeError('Recovery mismatch: '+row['path'])
with zipfile.ZipFile(archive,'x',zipfile.ZIP_DEFLATED,compresslevel=6) as z:
 for f in sorted(release.rglob('*')):
  if f.is_file():z.write(f,'Windows/'+f.relative_to(release).as_posix())
with zipfile.ZipFile(archive) as z:
 for row in manifest:
  with z.open('Windows/'+row['path']) as f:
   if hashlib.file_digest(f,'sha256').hexdigest()!=row['sha256']:raise RuntimeError('ZIP mismatch')
 if any('/Saved/' in n or n.lower().endswith('/onlineservices.ini') for n in z.namelist()):raise RuntimeError('Private data in ZIP')
evidence.mkdir(parents=True,exist_ok=True)
shutil.copy2(release/'PACKAGE_SHA256.json',evidence/'release-manifest.json')
record=dict(candidate=str(source),release=str(release),recovery=str(recovery),sourceCommit=commit,filesVerified=len(manifest),zip=str(archive),zipBytes=archive.stat().st_size,zipSHA256=digest(archive),everyFileVerified=True,publicConfigurationIncluded=False)
(evidence/'checkpoint.json').write_text(json.dumps(record,indent=2))
# Install the actual owner-supplied config only after the credential-free ZIP is sealed.
for local in [release,recovery]:shutil.copy2(ROOT/'OnlineServices.ini',local/'DinosaurBattle/OnlineServices.ini')
record['localConfigurationHashMatches']=all(digest(local/'DinosaurBattle/OnlineServices.ini')==digest(ROOT/'OnlineServices.ini') for local in [release,recovery])
(evidence/'checkpoint.json').write_text(json.dumps(record,indent=2));print(json.dumps(record,indent=2))
