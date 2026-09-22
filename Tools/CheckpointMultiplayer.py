"""Promote a verified multiplayer package with independent copies and SHA-256 checks.

This script never deletes packages. It refuses to overwrite checkpoint paths.
Run from the project root only after native launch/gameplay verification.
"""
import hashlib,json,pathlib,shutil
ROOT=pathlib.Path(__file__).resolve().parents[1]
DIST=(ROOT/'Dist').resolve()
CANDIDATE=DIST/'MultiplayerCandidate/Windows'
RECOVERY=DIST/'Checkpoints/2026-09-22-eos-multiplayer/Windows'
PREVIOUS=DIST/'Checkpoints/2026-09-22-before-eos-promotion/Windows'
ACTIVE=DIST/'Windows'
RESULTS=ROOT/'Tests/Results/multiplayer'

def checked(path):
    path=path.resolve()
    if not path.is_relative_to(DIST) or path==DIST:raise RuntimeError('Outside intended package tree')
    return path

def digest(path):
    with path.open('rb') as f:return hashlib.file_digest(f,'sha256').hexdigest()

def files(root):
    return sorted(p for p in root.rglob('*') if p.is_file() and 'Saved' not in p.relative_to(root).parts)

def manifest(root):
    return [dict(path=p.relative_to(root).as_posix(),size=p.stat().st_size,sha256=digest(p)) for p in files(root)]

for package in (CANDIDATE,RECOVERY,PREVIOUS,ACTIVE):checked(package)
if RECOVERY.exists() or PREVIOUS.exists():raise RuntimeError('Checkpoint already exists; inspect before continuing')
for name,expected_count in [('core-live.json',90),('integration-live.json',41),('settings-water-blood.json',28),('match-rules.json',24)]:
    rows=json.loads((RESULTS/'packaged-offline'/name).read_text())
    if len(rows)!=expected_count or not all(r['passed'] for r in rows):raise RuntimeError('Offline regression incomplete: '+name)
for folder,expected_count in [('packaged-four',13),('packaged-rendered-final',6),('packaged-ui-final',6)]:
    data=json.loads((RESULTS/folder/'results.json').read_text())
    if len(data['tests'])!=expected_count or not all(r['status']=='PASS' for r in data['tests']):raise RuntimeError('Package test incomplete: '+folder)
if not (RESULTS/'packaged-ui-final/inspection-done.txt').exists():raise RuntimeError('Final native inspection not recorded')

# Retain the original runtime exactly; Saved logs/settings may legitimately change.
baseline=json.loads((ROOT/'Tests/Results/multiplayer-baseline/package-manifest.json').read_text())
for row in baseline:
    if 'Saved' in pathlib.PurePosixPath(row['path'].replace('\\','/')).parts:continue
    old=ACTIVE/row['path']
    if not old.exists() or digest(old).lower()!=row['sha256'].lower():raise RuntimeError('Original runtime changed: '+row['path'])

expected=manifest(CANDIDATE)
if not expected or not (CANDIDATE/'DinosaurBattle.exe').is_file():raise RuntimeError('Candidate missing')
for row in expected:
    dst=RECOVERY/row['path'];dst.parent.mkdir(parents=True,exist_ok=True);shutil.copy2(CANDIDATE/row['path'],dst)
if manifest(RECOVERY)!=expected:raise RuntimeError('Recovery hash verification failed')

PREVIOUS.parent.mkdir(parents=True,exist_ok=True)
# Resolved absolute source and target have both been validated inside Dist above.
shutil.move(str(checked(ACTIVE)),str(checked(PREVIOUS)))
try:
    shutil.copytree(RECOVERY,ACTIVE)
    if manifest(ACTIVE)!=expected:raise RuntimeError('Promoted hash verification failed')
except Exception:
    failed=checked(DIST/'FailedEosPromotion')
    if ACTIVE.exists():
        if failed.exists():raise RuntimeError('Manual recovery required; prior package remains at '+str(PREVIOUS))
        shutil.move(str(checked(ACTIVE)),str(failed))
    shutil.move(str(checked(PREVIOUS)),str(checked(ACTIVE)))
    raise

# Preserve existing local graphics preferences, but distribute a clean recovery copy.
settings=PREVIOUS/'DinosaurBattle/Saved/Config/Windows/GameUserSettings.ini'
if settings.exists():
    dst=ACTIVE/'DinosaurBattle/Saved/Config/Windows/GameUserSettings.ini'
    dst.parent.mkdir(parents=True,exist_ok=True);shutil.copy2(settings,dst)
(RESULTS/'package-manifest.json').write_text(json.dumps(expected,indent=2))
(RESULTS/'package-checkpoint.json').write_text(json.dumps(dict(candidate=str(CANDIDATE),recovery=str(RECOVERY),promoted=str(ACTIVE),previous=str(PREVIOUS),verifiedFiles=len(expected),sha256Matched=True,eos='NOT VERIFIED',wan='NOT VERIFIED'),indent=2))
print('Verified and promoted',len(expected),'files; independent recovery:',RECOVERY)
