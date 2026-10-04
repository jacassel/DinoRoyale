"""Live hunger thresholds, finite ecology and sight/noise map tests."""
import runtime_core as t,time,json,sys
rows=[]
def wait(sec):
 start=t.state()['time'];deadline=time.monotonic()+sec*5+15
 while t.state()['time']-start<sec and time.monotonic()<deadline:time.sleep(.025)
def press(k):t.key(k);t.key(k,'up')
def check(n,ok,**v):
 rows.append(dict(test=n,passed=bool(ok),**v));(t.OUT/'ecology-visibility.json').write_text(json.dumps(rows,indent=2));print(('PASS ' if ok else 'FAIL ')+n+' '+json.dumps(v),flush=True)
def ai(i=1):return next(a for a in t.state()['ai'] if a['id']==i)
def place(i,sp,x,y=0,enabled=False):t.command('testAI',id=i,species=sp,x=x,y=y,health=1,enabled=enabled,yaw=180,personality=3)
def setup(i):
 for k in ['W','LeftControl','F','LeftShift','RightMouseButton']:t.key(k,'up')
 t.command('menu',open=False);t.command('ai',paused=True);t.command('clearTestFood');t.command('species',value=i);t.command('face',yaw=0);wait(.7)
t.command('menu',open=False);t.command('sandbox',enabled=True);t.command('match',teams=False);t.command('invulnerable',value=False)
for i,n in enumerate(['Trex','Raptor','Trike']):
 setup(i);a=t.state();wait(2);b=t.state();check(n+' hunger gentle normal drain',0<a['hunger']-b['hunger']<.3,loss=a['hunger']-b['hunger'])
 t.command('damage',value=b['maxHealth']*.6);wait(5.2)
 for hunger,hfactor,sfactor in [(95,1.5,1.25),(75,1,1),(65,.5,.65),(35,0,.4),(15,0,0),(9,0,0)]:
  t.command('hunger',value=hunger);t.command('stamina',value=30);a=t.state();wait(1.1);b=t.state();dt=b['time']-a['time'];hp=(b['health']-a['health'])/b['maxHealth']/dt;st=(b['stamina']-a['stamina'])/dt
  check(n+f' hunger {hunger} regen thresholds',abs(b['healthRegenFactor']-hfactor)<.01 and abs(b['staminaRegenFactor']-sfactor)<.01 and (abs(hp-.02*hfactor)<.003 if hunger>10 else hp<-.002) and abs(st-22*sfactor)<1.5,hpRate=hp,staminaRate=st)
 # Finite small food can still rescue an exhausted starving animal.
 t.command('hunger',value=9);t.command('stamina',value=0);t.command('food');a=t.state();t.key('F');wait(1.8);b=t.state();t.key('F','up')
 check(n+' eating rescues hunger health stamina',b['hunger']>a['hunger']+10 and b['health']>a['health'] and b['stamina']>20,hunger=b['hunger'],healthGain=b['health']-a['health'],stamina=b['stamina'])
 check(n+' starvation does not block feeding',b['hunger']>20)
# Independent carcass size and lifetime.
setup(0)
for sp in [3,1,0,2]:t.command('food',species=sp)
food={c['species']:c['maxFood'] for c in t.state()['corpses']};check('carcass food scales with size',food.get(3)==25 and food.get(1)==120 and food.get(0)==360 and food.get(2)==480,food=food)
place(1,0,3000);t.command('scoreHit',attacker=0,victim=1,value=99999);wait(1.9)
c=next(c for c in t.state()['corpses'] if c['source']==1);check('carcass settles to frozen pose',c['frozen']);wait(9)
check('corpse survives its dinosaur respawn',not ai()['dead'] and any(x['name']==c['name'] and x['food']==360 for x in t.state()['corpses']))
setup(0);t.command('food',species=3);c=t.state()['corpses'][0];t.command('hunger',value=0);t.command('stamina',value=0);t.command('damage',value=t.state()['maxHealth']*.6);wait(2.7);t.key('F');wait(1.6);t.key('F','up')
check('tiny carcass consumed exactly and removed',not any(x['name']==c['name'] for x in t.state()['corpses']) and t.state()['hunger']<18,hunger=t.state()['hunger'])
setup(2);t.command('food');plant=min(t.state()['plants'],key=lambda x:t.dist(x,t.state()));t.command('hunger',value=0);t.command('stamina',value=0);t.command('damage',value=t.state()['maxHealth']*.75);wait(2.7);t.key('F');wait(5.6);t.key('F','up');p=next(x for x in t.state()['plants'] if x['name']==plant['name']);check('eaten plant disappears and cannot supply more food',p['hidden'] and p['food']==0,food=p['food']);t.command('screenshot')
# Deterministic map visibility tests with real visibility-channel occlusion.
setup(0)
for i in range(2,10):place(i,(i-1)//3,-24000+i*400,-23000)
place(1,0,1600);wait(.5);check('quiet dinosaur visible in line of sight',ai()['inSight'] and ai()['mapVisible'])
t.command('camera',yaw=180);wait(.4);check('quiet dinosaur outside view hidden on map',not ai()['inSight'] and not ai()['mapVisible'])
t.command('aiAbility',id=1,action='quick');wait(.2);check('AI attack reveals an unseen position',ai()['mapVisible'] and not ai()['inSight'] and abs(ai()['markerX']-1600)<1)
wait(6.2);check('noise marker expires after six seconds',not ai()['mapVisible'])
t.command('camera',yaw=0);t.command('sightBlocker',enabled=True);wait(.3);check('terrain obstacle blocks sight and marker',not ai()['inSight'] and not ai()['mapVisible'])
t.command('aiAbility',id=1,action='charge');wait(.2);check('charge windup reveals through occlusion',ai()['mapVisible'] and not ai()['inSight']);t.command('aiAbility',id=1,action='release');wait(1.7);t.command('sightBlocker',enabled=False)
place(1,0,3500);t.command('species',value=1);t.command('damage',value=t.state()['maxHealth']*.8);t.command('enableAI',id=1,enabled=True);samples=[];start=t.state()['time']
while t.state()['time']-start<1.5:samples.append(ai());time.sleep(.04)
check('AI sprint reveals its location',any(a['sprinting'] and a['revealUntil']>t.state()['time'] for a in samples));t.command('ai',paused=True)
setup(0);t.key('W');t.key('LeftShift');wait(.8);check('human sprint reveals location',t.state()['sprinting'] and t.state()['revealUntil']>t.state()['time']);t.key('W','up');t.key('LeftShift','up');wait(6.5);check('human noise expires after stopping',t.state()['revealUntil']<t.state()['time'])
# A hungry herbivore seeks available vegetation without a fabricated eat command.
setup(2);t.command('food');place(1,2,1300);t.command('hunger',id=1,value=25);t.command('teleport',x=22000,y=0);t.command('enableAI',id=1,enabled=True);samples=[];start=t.state()['time']
while t.state()['time']-start<6:samples.append(ai());time.sleep(.1)
check('hungry AI seeks and actually consumes food',any(a['state']=='Feeding' for a in samples) and ai()['hunger']>45,hunger=ai()['hunger'],states=sorted({a['state'] for a in samples}))
t.command('ai',paused=True);t.command('species',value=0);t.command('menu',open=True);print('RESULT',sum(r['passed'] for r in rows),'/',len(rows),flush=True);sys.exit(any(not r['passed'] for r in rows))
