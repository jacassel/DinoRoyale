"""Rendered mixed-roster endurance and isolated 1080p map performance samples."""
import argparse,time,json,math,statistics,shutil,sys
from net_harness import NetworkTest,Peer
p=argparse.ArgumentParser();p.add_argument('--executable',required=True);p.add_argument('--seconds',type=int,default=180);p.add_argument('--output',default='Tests/Results/roster05/final-soak');p.add_argument('--soak-only',action='store_true');p.add_argument('--performance-only',action='store_true');a=p.parse_args()
t=NetworkTest(a.output,executable=a.executable,rendered=True);t.render_size=(1920,1080);t.audio=True
reports=[]
try:
 h=Peer(t,'RosterSoak','/Game/Maps/LostValley');h.command('menu',open=False)
 def shot(name):
  folder=t.bridge_root.parent/'Screenshots/Windows';before=set(folder.glob('*.png'));h.command('screenshot');end=time.monotonic()+10
  while time.monotonic()<end:
   files=set(folder.glob('*.png'))-before
   if files:time.sleep(.3);shutil.copyfile(max(files,key=lambda p:p.stat().st_mtime),t.out/(name+'.png'));return
   time.sleep(.1)
  raise RuntimeError('Screenshot not written')
 for performance,teams,species in ([] if a.performance_only else [(False,False,4),(True,False,5),(False,True,6),(True,True,0)]):
  h.command('menu',open=False);h.command('mapVariant',performance=performance);h.command('species',value=species);h.command('match',teams=teams);h.command('sandbox',enabled=True);h.command('invulnerable',value=False);h.command('ai',paused=False)
  start=h.state();last=start;stalls={};maxstalls={};samples=[];wall=time.monotonic();next_attack=0
  while last['time']-start['time']<a.seconds and time.monotonic()-wall<a.seconds*2:
   s=h.state();samples.append(s)
   for d in s['ai']:
    if not d['major']:continue
    old=next((v for v in last['ai'] if v['id']==d['id']),d);ident=d['id']
    moving=d['state'] in ['Pursuing','Roaming','Seeking food','Retreating','Regrouping']
    stalled=not d['dead'] and moving and math.hypot(d['x']-old['x'],d['y']-old['y'])<20
    stalls[ident]=stalls.get(ident,0)+1 if stalled else 0;maxstalls[ident]=max(maxstalls.get(ident,0),stalls[ident])
   if not s['dead'] and time.monotonic()>next_attack:h.tap('LeftMouseButton');next_attack=time.monotonic()+1.8
   last=s;time.sleep(.8)
  row=dict(performance=performance,teams=teams,playerSpecies=species,seconds=last['time']-start['time'],samples=len(samples),species=sorted({d['species'] for s in samples for d in s['ai'] if d['major']}|{species}),hits=sum(d['hits'] for d in last['ai'] if d['major']),maxStallSamples=max(maxstalls.values()),playerDeaths=last['deaths'],corpses=len(last['corpses']),maxCorpses=max(len(s['corpses']) for s in samples),failedPaths=sum(d['failedPaths'] for d in last['ai'] if d['major']),belowTerrain=any(d['z']<d['ground']-150 for s in samples for d in s['ai'] if d['major'] and not d['dead']),trees=sum(d['tree'] for d in last['plants']))
  reports.append(row);(t.out/'soak.json').write_text(json.dumps(reports,indent=2));(t.out/f'soak-{int(performance)}-{int(teams)}.json').write_text(json.dumps(samples,separators=(',',':')))
  t.check(f'mixed roster soak performance={performance} teams={teams}',row['seconds']>=a.seconds-.5 and row['species']==[0,1,2,4,5,6] and row['hits']>10 and not row['belowTerrain'] and row['failedPaths']==0 and row['maxStallSamples']<40 and row['trees']==18,**row);shot(f'soak-{int(performance)}-{int(teams)}')
 if a.soak_only:sys.exit(0)
 # Measure with the ordinary live six-species AI roster and no other game processes.
 perf=[];h.command('console',value='t.MaxFPS 0');h.command('console',value='r.ScreenPercentage 100')
 for performance in [False,True]:
  h.command('mapVariant',performance=performance);h.command('species',value=5);h.command('match',teams=True);h.command('sandbox',enabled=True);h.command('invulnerable',value=True);h.command('ai',paused=False)
  for scene,x,y in [('plains',0,0),('forest',-12500,8500),('pond',3000,6700)]:
   h.command('teleport',x=x,y=y);h.command('camera',yaw=130,pitch=-12);time.sleep(4);b=h.state();samples=[];end=time.monotonic()+20
   while time.monotonic()<end:samples.append(h.state());time.sleep(.08)
   e=h.state();r=dict(performance=performance,scene=scene,width=1920,height=1080,fps=(e['frameCount']-b['frameCount'])/(e['frameSeconds']-b['frameSeconds']),p95FrameMs=sorted(s['frameMs'] for s in samples)[int((len(samples)-1)*.95)],medianMs={k:statistics.median(s[k] for s in samples) for k in ['frameMs','gameThreadMs','renderThreadMs','gpuMs']},major=e['majorCount'],trees=sum(v['tree'] for v in e['plants']))
   perf.append(r);(t.out/'performance-1080.json').write_text(json.dumps(perf,indent=2));print(r,flush=True);shot(f'{scene}-{int(performance)}')
 h.command('console',value='t.MaxFPS 60');h.command('console',value='r.ScreenPercentage 0')
finally:t.close()
