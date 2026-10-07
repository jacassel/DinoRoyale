"""Actual game attack trajectories and ordinary-camera frames, with slow-motion inspection."""
import argparse,time,json,shutil,math
from net_harness import NetworkTest,Peer,wait_for
p=argparse.ArgumentParser();p.add_argument('--executable');p.add_argument('--output',default='Tests/Results/launch06/art-motion-first');a=p.parse_args()
t=NetworkTest(a.output,executable=a.executable,rendered=True);t.render_size=(1600,900)
try:
 h=Peer(t,'LaunchMotion','/Game/Maps/LostValley');h.command('match',teams=False);h.command('ai',paused=True);h.command('sandbox',enabled=True)
 def shot(name):
  folder=t.bridge_root.parent/'Screenshots/Windows';before=set(folder.glob('*.png'));h.command('screenshot')
  if not wait_for(lambda:bool(set(folder.glob('*.png'))-before),10):raise RuntimeError('No screenshot')
  time.sleep(.15);shutil.copyfile(max(set(folder.glob('*.png'))-before,key=lambda p:p.stat().st_mtime),t.out/(name+'.png'))
 for sp in [4,5,6,7]:
  h.command('species',value=sp);h.command('face',yaw=0);h.command('camera',yaw=0,pitch=-13);time.sleep(1)
  shot(f'{sp}-idle-normal');h.command('camera',yaw=115,pitch=-8);shot(f'{sp}-idle-side');h.command('camera',yaw=0,pitch=-13)
  t.check(f'{sp} textured material loaded',any('Launch06' in m for m in h.state()['materials']),materials=h.state()['materials'])
  for heavy in [False,True]:
   h.command('species',value=sp);h.command('face',yaw=0);h.command('stamina',value=160)
   h.command('timeScale',value=.2);time.sleep(.3)
   if heavy:h.key('RightMouseButton');time.sleep(8);shot(f'{sp}-charge');h.key('RightMouseButton','up')
   else:h.tap('LeftMouseButton')
   duration={4:1.5 if heavy else .72,5:1.9 if heavy else .95,6:1.15 if heavy else .46,7:1.30 if heavy else .53}[sp]
   rows=[];captured=set();deadline=time.monotonic()+duration*5+6
   while time.monotonic()<deadline:
    s=h.state();rows.append({k:s[k] for k in ['time','attackElapsed','animation','attackContacts','hits','x','y','z','recovery']})
    phase=s['attackElapsed']/duration
    for key,target in [('windup',.15),('strike',.36),('follow',.6)]:
     if phase>=target and key not in captured:shot(f'{sp}-'+('heavy' if heavy else 'quick')+'-'+key);captured.add(key)
    if phase>=1:break
    time.sleep(.045)
   (t.out/(f'{sp}-'+('heavy' if heavy else 'quick')+'.json')).write_text(json.dumps(rows,indent=2))
   if sp==4:
    coords=[p for s in rows for p in s['attackContacts'] if p['bone']=='club_tip']
    span=max(p['y'] for p in coords)-min(p['y'] for p in coords)
    # A larger curved sweep brings the club forward beside the hips; lateral span
    # alone is not monotonic once the chain rotates beyond ninety degrees.
    forward=max(p['x'] for p in coords)
    t.check(('heavy' if heavy else 'quick')+' club has broad lateral and forward sweep',span>600 and forward>(-180 if heavy else -350),spanCm=span,forwardX=forward)
    t.check('club stays out of torso',all(not(-190<p['x']<140 and abs(p['y'])<150) for p in coords))
   if sp==5:
    coords=[p for s in rows for p in s['attackContacts'] if p['bone']=='head']
    span=max(p['x'] for p in coords)-min(p['x'] for p in coords)
    t.check(('heavy' if heavy else 'quick')+' neck visibly shifts',span>60,headTravelCm=span)
   h.command('timeScale',value=1);t.check(f'{sp} attack returns to idle',wait_for(lambda:h.state()['animation']=='Idle',6))
 h.quit()
finally:t.close()
