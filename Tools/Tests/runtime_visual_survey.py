"""Repeatable rendered comparisons, using the running opt-in development bridge."""
import runtime_core as t
import time, shutil, json, sys

label=sys.argv[1] if len(sys.argv)>1 else 'current'
out=t.ROOT/'Tests/Results/visual-modernization'/label
out.mkdir(parents=True,exist_ok=True)
rows=[]

def shot(name):
    folder=t.BRIDGE.parent/'Screenshots/Windows'
    before=set(folder.glob('*.png'))
    t.command('screenshot')
    deadline=time.monotonic()+12
    while time.monotonic()<deadline:
        new=set(folder.glob('*.png'))-before
        if new:
            src=max(new,key=lambda p:p.stat().st_mtime)
            time.sleep(.25)
            shutil.copyfile(src,out/(name+'.png'))
            rows.append(dict(name=name,state=t.state()))
            print(name,flush=True)
            return
        time.sleep(.05)
    raise RuntimeError('Missing capture '+name)

t.command('menu',open=False)
t.command('ai',paused=True)
t.command('sandbox',enabled=True)
t.command('invulnerable',value=True)
t.command('removeTarget')
for i,species in enumerate(['rex','raptor','trike']):
    t.command('species',value=i)
    t.command('face',yaw=0)
    for yaw,view in [(90,'side'),(145,'front'),(0,'rear')]:
        t.command('camera',yaw=yaw)
        time.sleep(.65)
        shot(species+'-'+view)
for name,x,y in [('pond',3000,6600),('river',2500,-4900),('forest',-12500,8500),('ridge',11500,9500)]:
    t.command('teleport',x=x,y=y)
    t.command('camera',yaw=-90)
    time.sleep(1)
    shot(name)
t.command('menu',open=True)
(out/'captures.json').write_text(json.dumps(rows,indent=2))
