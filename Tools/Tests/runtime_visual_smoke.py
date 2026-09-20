"""Five focused captures for the unchanged jaw repair and compressed map."""
import runtime_core as t,time,json,shutil
out=t.ROOT/'Saved/QualityVisuals';out.mkdir(exist_ok=True);rows=[]
def wait(sec):
    start=t.state()['time']
    while t.state()['time']-start<sec:time.sleep(.02)
def shot(name):
    folder=t.BRIDGE.parent/'Screenshots/Windows';before=set(folder.glob('*.png'));t.command('screenshot')
    deadline=time.monotonic()+8
    while time.monotonic()<deadline:
        files=set(folder.glob('*.png'))-before
        if files:
            src=max(files,key=lambda p:p.stat().st_mtime);time.sleep(.1)
            try:shutil.copyfile(src,out/(name+'.png'));break
            except OSError:pass
        time.sleep(.05)
    else:raise RuntimeError('Missing screenshot '+name)
    rows.append(dict(name=name,path=str(out/(name+'.png')),animation=t.state()['animation']))
t.command('menu',open=False);t.command('ai',paused=True);t.command('sandbox',enabled=True);t.command('removeTarget')
for angle,name in [(90,'left'),(-135,'front')]:
    t.command('species',value=0);t.command('face',yaw=0);t.command('camera',yaw=angle);wait(.5)
    t.key('RightMouseButton');wait(1);shot('rex-'+name+'-charge');t.key('RightMouseButton','up');wait(.15);shot('rex-'+name+'-heavy');wait(2)
t.key('M');t.key('M','up');wait(.2);shot('compact-world-map');t.key('M');t.key('M','up')
t.command('menu',open=True);(t.OUT/'visual-smoke.json').write_text(json.dumps(rows,indent=2))
