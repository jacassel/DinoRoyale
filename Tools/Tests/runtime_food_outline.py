"""Verify edible-only species cue and capture its actual rendered silhouette."""
import runtime_core as t,time,json,shutil
out=t.ROOT/'Saved/QualityVisuals';out.mkdir(exist_ok=True)
rows=[]
def shot(name):
    folder=t.BRIDGE.parent/'Screenshots/Windows';before=set(folder.glob('*.png'))
    t.command('screenshot');deadline=time.monotonic()+10
    while time.monotonic()<deadline:
        files=set(folder.glob('*.png'))-before
        if files:
            src=max(files,key=lambda p:p.stat().st_mtime);time.sleep(.2)
            try:shutil.copyfile(src,out/(name+'.png'));return
            except OSError:pass
        time.sleep(.1)
    raise RuntimeError('No screenshot '+name)
t.command('menu',open=False);t.command('ai',paused=True);t.command('sandbox',enabled=True)
t.command('species',value=2);t.command('clearTestFood');t.command('teleport',x=20000,y=23000);t.command('face',yaw=0)
t.command('food');time.sleep(.6)
name=min(t.state()['plants'],key=lambda p:t.dist(p,t.state()))['name']
def plant():return next(p for p in t.state()['plants'] if p['name']==name)
rows.append(dict(test='Triceratops food outline enabled',passed=plant()['outline']))
t.command('camera',yaw=-40);time.sleep(.6);shot('triceratops-edible-outline')
for species in [0,1]:
    t.command('species',value=species);time.sleep(.4)
    rows.append(dict(test=f'Carnivore {species} food outline disabled',passed=all(not p['outline'] for p in t.state()['plants'])))
t.command('species',value=2);time.sleep(.4)
rows.append(dict(test='Return to Triceratops restores outline',passed=all(p['outline']==(p['food']>0) for p in t.state()['plants'])))
t.command('menu',open=True);t.command('sandbox',enabled=False)
(t.OUT/'food-outline.json').write_text(json.dumps(rows,indent=2));print(json.dumps(rows),flush=True)
assert all(r['passed'] for r in rows)
