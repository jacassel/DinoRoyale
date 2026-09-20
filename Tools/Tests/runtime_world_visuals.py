"""Capture the updated slopes/groves and export the actual world/nav audit."""
import runtime_core as t
import time, json, shutil

out=t.OUT/'landscape';out.mkdir(parents=True,exist_ok=True)
t.command('menu',open=False);t.command('species',value=0);t.command('ai',paused=True)
t.command('sandbox',enabled=True);t.command('invulnerable',value=True)
t.command('worldAudit');t.command('navAudit')
for name in ['world-audit.json','navigation-audit.json']:
    shutil.copyfile(t.BRIDGE/name,t.OUT/name)
rows=[]
for name,x,y,yaw,pitch,distance in [
    ('plains',0,0,90,-13,1200),('forest',-12500,8500,-90,-13,1400),
    ('ridge',11500,9500,-90,-13,1450),('river',2500,-4900,-90,-13,1400),
    ('pond',3000,6600,-90,-13,1400),('grove',-7500,-11500,125,-10,1450),
    ('hill-approach',10500,4500,55,-7,1550),('forest-overlook',-13500,14500,-45,-15,2000),
    ('ridge-overlook',13000,12500,-145,-12,2000)]:
    t.command('teleport',x=x,y=y);t.command('face',yaw=yaw);t.command('camera',yaw=yaw,pitch=pitch,distance=distance)
    time.sleep(.85);folder=t.BRIDGE.parent/'Screenshots/Windows';before=set(folder.glob('*.png'))
    t.command('screenshot');deadline=time.monotonic()+12
    while time.monotonic()<deadline:
        new=set(folder.glob('*.png'))-before
        if new:
            time.sleep(.2);src=max(new,key=lambda p:p.stat().st_mtime);shutil.copyfile(src,out/(name+'.png'));break
        time.sleep(.1)
    else:raise RuntimeError('Missing screenshot '+name)
    rows.append(dict(name=name,state=t.state()));print(name,flush=True)
(out/'captures.json').write_text(json.dumps(rows,indent=2))
t.command('menu',open=True)
