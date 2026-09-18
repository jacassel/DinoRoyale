"""Live player-input checks for stamina, sprint, three-hit commitment and heavy risk/reward."""
import runtime_core as t,time,json,sys
rows=[]
def check(n,ok,**d):
 rows.append(dict(test=n,passed=bool(ok),**d));(t.OUT/'combat-polish.json').write_text(json.dumps(rows,indent=2));print(('PASS ' if ok else 'FAIL ')+n+' '+json.dumps(d),flush=True)
def wait(sec):
 start=t.state()['time'];end=time.monotonic()+sec*4+10
 while t.state()['time']-start<sec and time.monotonic()<end:time.sleep(.025)
def press(k):t.key(k);t.key(k,'up')
def ready():
 end=time.monotonic()+8
 while t.state()['recovery']>0 and time.monotonic()<end:time.sleep(.025)
def reset(i):
 for k in ['W','S','A','D','Q','LeftShift','SpaceBar','RightMouseButton','E']:t.key(k,'up')
 t.command('menu',open=False);t.command('ai',paused=True);t.command('removeTarget');t.command('species',value=i);t.command('face',yaw=0);wait(.6)
t.command('sandbox',enabled=True)
for i,n in enumerate(['Trex','Raptor','Trike']):
 reset(i);a=t.state();t.key('W');wait(1);a=t.state();t.key('LeftShift');wait(1);b=t.state()
 check(n+' sprint accelerates and drains',b['sprinting'] and b['speed']>a['speed']*1.2 and b['stamina']<95,walk=a['speed'],sprint=b['speed'],stamina=b['stamina'])
 t.key('LeftShift','up');wait(.8);a=t.state();wait(1);b=t.state();check(n+' walking regenerates',b['stamina']>a['stamina']);t.key('W','up')
 t.command('stamina',value=50);wait(.7);a=t.state();wait(1);b=t.state();check(n+' idle regenerates quickly',b['stamina']-a['stamina']>=19)
 t.command('stamina',value=50);a=t.state();press('SpaceBar');wait(.15);b=t.state();check(n+' jump spends stamina',b['falling'] and b['stamina']<a['stamina']-10)
 a=b;press('SpaceBar');wait(.1);check(n+' airborne jump cannot spend again',t.state()['stamina']>=a['stamina']-.1);wait(1.5)
 t.command('stamina',value=0);t.key('W');t.key('LeftShift');press('SpaceBar');press('RightMouseButton');wait(.15);b=t.state()
 check(n+' exhaustion restricts sprint jump heavy',b['exhausted'] and not b['sprinting'] and not b['falling'] and not b['charging'])
 press('LeftMouseButton');wait(.16);check(n+' exhausted quick still works',t.state()['recovery']>0 and t.state()['weakAttack'])
 t.key('W','up');t.key('LeftShift','up');wait(3);check(n+' exhaustion reliably recovers',not t.state()['exhausted'] and t.state()['stamina']>25)
 reset(i);t.command('target');wait(.2);dur=[];damage=[]
 for hit in range(3):
  a=t.state();press('LeftMouseButton');wait(.2);b=t.state();dur.append(b['recovery']+.2);damage.append(a['targetHealth']-b['targetHealth']);check(n+f' combo strike {hit+1}',b['combo']==hit+1 and damage[-1]>0,damage=damage[-1]);
  if hit<2:ready()
 check(n+' third strike enforces recovery',dur[2]>dur[0]+.35,first=dur[0],third=dur[2]);serial=t.state()['attackSerial'];press('LeftMouseButton');wait(.1);check(n+' immediate fourth strike rejected',t.state()['attackSerial']==serial)
 t.key('Q');wait(.1);check(n+' brace cannot cancel commitment',not t.state()['brace'] and t.state()['recovery']>0);t.key('Q','up')
 a=t.state();t.command('menu',open=True);b=t.state();check(n+' pause cannot erase cooldown',b['recovery']>=max(0,a['recovery']-(b['time']-a['time']))-.06);t.command('menu',open=False);ready();wait(1.1)
 t.command('target');t.command('stamina',value=100);a=t.state();t.key('RightMouseButton');wait(1.4);b=t.state();check(n+' heavy has readable charge',b['charging'] and b['charge']>=.99);t.key('RightMouseButton','up');wait(.55);b=t.state()
 check(n+' heavy worthwhile and single hit',b['damageDealt']>damage[0]*2.6 and b['stamina']<80,quick=damage[0],heavy=b['damageDealt'],stamina=b['stamina']);hits=b['hits'];wait(.4);check(n+' heavy has no duplicate damage',t.state()['hits']==hits)
 ready();t.command('removeTarget');t.command('stamina',value=100);t.key('RightMouseButton');wait(1.4);t.key('RightMouseButton','up');wait(.6);b=t.state();check(n+' heavy miss punished',b['recovery']>max(0,b['attackDuration']-b['attackElapsed'])+.15,recovery=b['recovery']);ready()
 t.command('stamina',value=0);t.command('food');press('E');t.key('E');wait(.8);b=t.state();check(n+' food restores stamina at full health',b['stamina']>30,stamina=b['stamina']);t.key('E','up')
 t.command('damage',value=10);t.key('E');wait(.15);check(n+' damage prevents immediate combat feeding',not t.state()['eating']);t.key('E','up');wait(2.6);t.key('E');wait(.15);check(n+' food becomes available after safe interval',t.state()['eating'] or t.state()['stamina']>=99);t.key('E','up')
 reset(i);t.key('Q');t.command('stamina',value=3);wait(1.5);check(n+' brace drains and breaks at zero',not t.state()['brace'] and t.state()['exhausted']);t.key('Q','up')
t.command('ai',paused=True);t.command('screenshot');print('RESULT',sum(r['passed'] for r in rows),'/',len(rows),flush=True);sys.exit(any(not r['passed'] for r in rows))

