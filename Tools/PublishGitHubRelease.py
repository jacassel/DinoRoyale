"""Publish a sealed versioned asset using existing Git Credential Manager authentication.

Credentials are captured in memory only. A draft becomes public only after GitHub
reports the exact local ZIP size and SHA256. Existing assets are never replaced.
"""
import argparse,pathlib,subprocess,json,hashlib,requests
p=argparse.ArgumentParser();p.add_argument('--version',default='0.5');p.add_argument('--evidence',default='Tests/Results/roster05/release');a=p.parse_args()
ROOT=pathlib.Path(__file__).resolve().parents[1];REPO='jacassel/DinoRoyale';TAG='v'+a.version+'.0';prefix='VERSION'+a.version.replace('.','')
evidence=ROOT/a.evidence;record=json.loads((evidence/'checkpoint.json').read_text());archive=pathlib.Path(record['zip'])
with archive.open('rb') as f:sha=hashlib.file_digest(f,'sha256').hexdigest()
if sha!=record['zipSHA256']:raise RuntimeError('Sealed ZIP changed')
credentials=subprocess.run(['git','credential','fill'],input='protocol=https\nhost=github.com\n\n',capture_output=True,text=True,check=True,cwd=ROOT)
values=dict(line.split('=',1) for line in credentials.stdout.splitlines() if '=' in line)
token=values.get('password');credentials=None;values=None
if not token:raise RuntimeError('Existing GitHub credential unavailable')
session=requests.Session();session.headers.update({'Authorization':'Bearer '+token,'Accept':'application/vnd.github+json','X-GitHub-Api-Version':'2026-03-10'});token=None
api='https://api.github.com/repos/'+REPO
def request(method,url,**kwargs):
 r=session.request(method,url,timeout=kwargs.pop('timeout',90),**kwargs);r.raise_for_status();return r.json()
remote=request('GET',api+'/commits/'+record['sourceCommit'])
if remote['sha']!=record['sourceCommit']:raise RuntimeError('Source commit not on GitHub')
body=(ROOT/(prefix+'_RELEASE_NOTES.md')).read_text(encoding='utf-8')+f'\n\nWindows ZIP SHA256: `{sha}`\n'
releases=request('GET',api+'/releases',params={'per_page':100});existing=[r for r in releases if r['tag_name']==TAG]
release=existing[0] if existing else request('POST',api+'/releases',json={'tag_name':TAG,'target_commitish':record['sourceCommit'],'name':'Dino Royale - Version '+a.version,'body':body,'draft':True,'prerelease':True})
assets=request('GET',release['assets_url']);found=[r for r in assets if r['name']==archive.name]
if found:asset=found[0]
else:
 if not release['draft']:raise RuntimeError('Published release has no expected asset; inspect manually')
 print(f'Uploading verified {archive.stat().st_size:,}-byte Windows ZIP to draft release...',flush=True)
 with archive.open('rb') as f:asset=request('POST',release['upload_url'].split('{')[0],params={'name':archive.name},headers={'Content-Type':'application/zip','Content-Length':str(archive.stat().st_size)},data=f,timeout=1800)
if asset['size']!=archive.stat().st_size or asset.get('digest')!='sha256:'+sha or asset['state']!='uploaded':raise RuntimeError('GitHub asset verification failed; draft preserved')
if release['draft']:release=request('PATCH',release['url'],json={'draft':False,'prerelease':True,'body':body})
public=requests.get(api+'/releases/tags/'+TAG,headers={'Accept':'application/vnd.github+json'},timeout=60);public.raise_for_status();data=public.json()
public_asset=next(x for x in data['assets'] if x['name']==archive.name)
if data['draft'] or public_asset['digest']!='sha256:'+sha:raise RuntimeError('Anonymous publication check failed')
result=dict(repository='https://github.com/'+REPO,release=data['html_url'],asset=public_asset['browser_download_url'],tag=TAG,sourceCommit=record['sourceCommit'],bytes=public_asset['size'],sha256=sha,anonymousVerified=True)
(evidence/'publication.json').write_text(json.dumps(result,indent=2));print(json.dumps(result,indent=2))
