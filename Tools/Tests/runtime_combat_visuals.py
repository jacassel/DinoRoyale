"""Capture the actual rendered jaw, attacks, stamina HUD and blood toggles."""
import runtime_core as t,time,json,pathlib,shutil
out=t.ROOT/'Saved/CombatVisuals';out.mkdir(exist_ok=True);shots=[]
def wait(sec):
 start=t.state()['time']
 while t.state()['time']-start<sec:time.sleep(.015)
def shot(name):
 folder=t.ROOT/'Saved/Screenshots/Windows';before=set(folder.glob('*.png'));t.command('screenshot')
 for _ in range(100):
  new=set(folder.glob('*.png'))-before
  if new:
   src=max(new,key=lambda p:p.stat().st_mtime);time.sleep(.15);dest=out/(name+'.png')
   try:shutil.copyfile(src,dest)
   except OSError:time.sleep(.1);continue
   shots.append(dict(name=name,path=str(dest),animation=t.state()['animation']));return
  time.sleep(.03)
 raise RuntimeError('Screenshot missing')
def toggle_blood(enabled):
 t.command('menu',open=True)
 if not t.state()['settingsOpen']:t.key('F2');t.key('F2','up')
 if t.state()['bloodEnabled']!=enabled:t.key('B');t.key('B','up')
 t.command('menu',open=False)
t.command('menu',open=False);t.command('ai',paused=True);t.command('sandbox',enabled=True);t.command('removeTarget');toggle_blood(False)
for angle,label in [(90,'left'),(-90,'right'),(145,'front_oblique')]:
 t.command('species',value=0);t.command('face',yaw=0);t.command('camera',yaw=angle);wait(.5)
 t.key('RightMouseButton');wait(1.15);shot('rex_'+label+'_charge');t.key('RightMouseButton','up');wait(.15);shot('rex_'+label+'_heavy_early');wait(.1);shot('rex_'+label+'_heavy_late');wait(1.8)
 t.command('teleport',x=0,y=0);t.key('LeftMouseButton');t.key('LeftMouseButton','up');wait(.10);shot('rex_'+label+'_quick');wait(.8)
for i,name in enumerate(['rex','raptor','trike']):
 for enabled in [True,False]:
  toggle_blood(enabled);t.command('species',value=i);t.command('face',yaw=0);t.command('camera',yaw=65);t.command('target');wait(.5)
  t.key('LeftMouseButton');t.key('LeftMouseButton','up');wait(.05);shot(name+('_blood_on' if enabled else '_blood_off'));wait(1.5)
  t.command('target');t.command('stamina',value=100);t.key('RightMouseButton');wait(1.3);t.key('RightMouseButton','up');wait(.22);shot(name+('_heavy_blood_on' if enabled else '_heavy_blood_off'));wait(1.6)
 toggle_blood(False);t.command('removeTarget')
t.command('species',value=0);t.command('menu',open=True);(t.OUT/'combat-visual-captures.json').write_text(json.dumps(shots,indent=2));print('VISUAL_CAPTURE_COMPLETE',len(shots),flush=True)
