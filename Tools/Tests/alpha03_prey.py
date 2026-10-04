"""Live hunger/priority/chase behavior across all four existing personalities."""
import json,time,sys
import runtime_core as t
rows=[]
def check(name,ok,**data):
    rows.append(dict(test=name,passed=bool(ok),**data))
    (t.OUT/'prey-priority.json').write_text(json.dumps(rows,indent=2))
    print(('PASS ' if ok else 'FAIL ')+name+' '+json.dumps(data),flush=True)
def ai():return next(x for x in t.state()['ai'] if x['id']==1)
def place(id,sp,x,y=0,profile=3,hunger=40,enabled=False):
    t.command('testAI',id=id,species=sp,x=x,y=y,yaw=0,health=1,hunger=hunger,personality=profile,enabled=enabled)
def reset(profile,hunger=40):
    t.command('ai',paused=True);t.command('species',value=0);t.command('teleport',x=-26000,y=-26000)
    for a in t.state()['ai']:
        if a['id']!=1:place(a['id'],a['species'],-24000+(a['id']%18)*200,-24000)
    place(1,0,0,profile=profile,hunger=hunger);place(100,3,1800)
def watch(sec):
    end=time.monotonic()+sec;s=[]
    while time.monotonic()<end:s.append(ai());time.sleep(.1)
    return s
t.command('menu',open=False);t.command('match',teams=False);t.command('sandbox',enabled=True);t.command('invulnerable',value=True)
for profile in range(4):
    reset(profile,100);t.command('enableAI',id=1,enabled=True)
    samples=watch(1.8);check(f'{profile} well-fed predator ignores small prey',all(x['target']!=100 for x in samples))
    reset(profile);t.command('enableAI',id=1,enabled=True)
    samples=watch(.7);check(f'{profile} hungry predator chooses nearby prey',any(x['target']==100 for x in samples))
    # Introduce a major threat while the animal is already hunting.
    t.command('teleport',x=ai()['x']+1600,y=ai()['y']+600)
    samples=watch(1.6);check(f'{profile} nearby major threat interrupts hunt',any(x['target']==0 for x in samples))
    reset(profile);t.command('enableAI',id=1,enabled=True);watch(.5)
    t.command('scoreHit',attacker=0,victim=1,value=20)
    t.command('teleport',x=ai()['x']+1100,y=ai()['y']+500)
    samples=watch(1);check(f'{profile} attack overrides small prey',any(x['target']==0 for x in samples) and ai()['retaliations']>0)
    reset(profile);before=ai()['abandonedPreyChases'];t.command('enableAI',id=1,enabled=True)
    # Keep a fleeing prey ahead of the hunter to exercise the real chase budget.
    # No target, AI clock, stamina or movement implementation is changed.
    samples=[];start=time.monotonic()
    while time.monotonic()-start<10:
        a=ai();samples.append(a)
        if a['abandonedPreyChases']>before:break
        place(100,3,a['x']+1800,a['y']);time.sleep(.15)
    check(f'{profile} unsuccessful prey chase is bounded',ai()['abandonedPreyChases']>before,seconds=round(time.monotonic()-start,2),travel=ai()['distance'])
    samples=watch(1.5);check(f'{profile} hunt cooldown prevents immediate reacquisition',all(x['target']!=100 for x in samples))
    reset(profile);place(1,0,24000,profile=profile);place(100,3,25300);t.command('enableAI',id=1,enabled=True)
    samples=watch(1.5);check(f'{profile} rejects prey beyond safe map interior',all(x['target']!=100 for x in samples))
t.command('ai',paused=True);t.command('invulnerable',value=False);t.command('sandbox',enabled=False)
sys.exit(any(not r['passed'] for r in rows))
