"""Close, multi-region and action captures of the actual running game."""
import runtime_core as t
import time,json,shutil,sys
out=t.OUT/'details';out.mkdir(parents=True,exist_ok=True);rows=[]

def wait(seconds):
    start=t.state()['time'];deadline=time.monotonic()+seconds*4+10
    while t.state()['time']-start<seconds:
        if time.monotonic()>deadline:raise RuntimeError('Game stopped advancing')
        time.sleep(.01)

def shot(name):
    folder=t.BRIDGE.parent/'Screenshots/Windows';before=set(folder.glob('*.png'))
    s=t.state();t.command('screenshot');deadline=time.monotonic()+10
    while time.monotonic()<deadline:
        new=set(folder.glob('*.png'))-before
        if new:
            file=max(new,key=lambda p:p.stat().st_mtime);time.sleep(.14)
            shutil.copyfile(file,out/(name+'.png'));rows.append(dict(name=name,state=s));print(name,flush=True);return
        time.sleep(.02)
    raise RuntimeError('Missing screenshot')

def setup(i):
    t.command('menu',open=False);t.command('species',value=i);t.command('ai',paused=True)
    t.command('sandbox',enabled=True);t.command('invulnerable',value=False);t.command('removeTarget');t.command('face',yaw=0)

for i,name in enumerate(['rex','raptor','trike']):
    setup(i)
    t.command('camera',yaw=130,pitch=-5,distance=[1050,570,950][i]);wait(.6);shot(name+'-close-front')
    t.command('camera',yaw=80,pitch=-7,distance=[1150,620,1050][i]);wait(.4);shot(name+'-close-side')
    for region,x,y in [('forest',-12500,8500),('river',2500,-5300),('pond',3000,6600),('ridge',11500,9500),('grove',-7500,-11500)]:
        t.command('teleport',x=x,y=y);t.command('camera',yaw=115,pitch=-12,distance=[1500,900,1400][i]);wait(.7);shot(name+'-'+region)
    t.command('teleport',x=3000,y=4500);wait(1.2);shot(name+'-swim')
    t.command('teleport',x=0,y=0);t.command('camera',yaw=90,pitch=-7,distance=[1250,650,1100][i]);wait(.8)
    t.key('W');wait(.28);shot(name+'-walk');t.key('LeftShift');wait(.5);shot(name+'-sprint');t.key('LeftShift','up');t.key('W','up');wait(.7)
    t.key('D');wait(.2);shot(name+'-turn');t.key('D','up');wait(.5)
    t.command('face',yaw=0);t.command('camera',yaw=90,pitch=-7,distance=[1250,650,1100][i]);wait(.4)
    t.key('LeftMouseButton');t.key('LeftMouseButton','up');wait(.13);shot(name+'-quick');wait(1)
    t.key('RightMouseButton');wait(1.2);shot(name+'-charge');t.key('RightMouseButton','up');wait(.15);shot(name+'-heavy');wait(1.8)
    t.command('hunger',value=60);t.command('food');wait(.6);t.key('F');wait(.4)
    if not t.state()['eating']:raise RuntimeError(name+' did not enter eating animation')
    shot(name+'-eat');t.key('F','up')
    t.command('damage',value=t.state()['maxHealth']*.2);wait(.05);shot(name+'-injured')
    t.command('damage',value=100000);wait(.8);shot(name+'-death')
t.command('species',value=0);t.command('menu',open=True)
(out/'captures.json').write_text(json.dumps(rows,indent=2))
