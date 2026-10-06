"""Rendered audio mixer/event checks for all three unique 0.5 sound banks."""
import argparse,time,wave,array,math,shutil
from net_harness import NetworkTest,Peer,wait_for
p=argparse.ArgumentParser();p.add_argument('--executable');p.add_argument('--output',default='Tests/Results/roster05/audio');a=p.parse_args()
t=NetworkTest(a.output,executable=a.executable,rendered=True);t.audio=True;t.render_size=(1280,720)
try:
 h=Peer(t,'RosterAudio','/Game/Maps/LostValley');h.command('match',teams=False);h.command('ai',paused=True);h.command('sandbox',enabled=True)
 for species,name in [(4,'Anky'),(5,'Brachi'),(6,'Pachy')]:
  h.command('species',value=species);h.command('face',yaw=0);time.sleep(.8);stamp=time.time();h.command('audioRecord',start=True)
  t.check(name+' six complete sound banks loaded',h.state()['audioLoadedClips']==162)
  b=h.state();h.hold('W',2.5);s=h.state();t.check(name+' footsteps',s['audioSteps']>b['audioSteps'])
  b=s;h.key('W');h.hold('LeftShift',3);h.key('W','up');s=h.state();t.check(name+' sprint breath',s['audioSprintBreaths']>b['audioSprintBreaths'])
  h.command('teleport',x=0,y=0);h.command('face',yaw=0);time.sleep(.5)
  x,y=(-440,250) if species==4 else (320,120) if species==5 else (340,0)
  h.command('testAI',id=1,species=0,x=x,y=y,yaw=180,health=1,enabled=False);b=h.state();h.tap('LeftMouseButton');time.sleep(1.1);s=h.state()
  t.check(name+' unique quick and contact sounds play',s['audioQuick']>b['audioQuick'] and s['audioImpacts']>b['audioImpacts'])
  time.sleep(1);h.command('stamina',value=160);b=h.state();h.hold('RightMouseButton',1.8);time.sleep(1);s=h.state();t.check(name+' charge and heavy play once',s['audioCharges']==b['audioCharges']+1 and s['audioHeavy']==b['audioHeavy']+1)
  time.sleep(2);h.command('damage',value=h.state()['maxHealth']*.67/(.60 if species==4 else .9 if species==5 else 1));b=h.state();time.sleep(4);s=h.state();t.check(name+' hurt and injured breathing',s['audioHurts']>0 and s['audioInjuredBreaths']>b['audioInjuredBreaths'])
  b=s;h.command('damage',value=100000);time.sleep(.3);t.check(name+' death voice fires once',h.state()['audioDeaths']==b['audioDeaths']+1);time.sleep(4.2);t.check(name+' death voice ends',h.state()['audioVoices']==0)
  h.command('audioRecord',start=False);path=t.bridge_root.parent/'AudioQA'/f'Species{species}.wav';t.check(name+' mixer recording written',wait_for(lambda:path.exists() and path.stat().st_mtime>=stamp,10));time.sleep(.3)
  shutil.copyfile(path,t.out/f'{name}-runtime.wav')
  with wave.open(str(path),'rb') as f:pcm=array.array('h',f.readframes(f.getnframes()))
  peak=max(abs(x) for x in pcm)/32768;rms=math.sqrt(sum(x*x for x in pcm)/len(pcm))/32768
  t.check(name+' recorded mix audible and unclipped',.0001<rms<.5 and peak<.999,peak=peak,rms=rms)
finally:t.close()
