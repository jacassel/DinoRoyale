"""Reproduce an unreachable preferred retreat endpoint at the north-west rim."""
import json,time,sys
import runtime_core as t
t.command('menu',open=False);t.command('match',teams=False);t.command('species',value=0)
t.command('sandbox',enabled=True);t.command('ai',paused=True);t.command('teleport',x=0,y=0)
for ident in range(1,10):
    t.command('testAI',id=ident,species=0,x=ident*800,y=-20000,yaw=0,health=1,enabled=False)
t.command('testAI',id=1,species=1,x=-23761.58,y=26668.22,yaw=145,health=.35,personality=1,enabled=False)
t.command('testAI',id=2,species=0,x=-21000,y=24600,yaw=145,health=1,enabled=False)
t.command('ai',paused=True);t.command('scoreHit',attacker=2,victim=1,value=1)
def subject():return next(a for a in t.state()['ai'] if a['id']==1)
before=subject();t.command('enableAI',id=1,enabled=True);samples=[];end=time.monotonic()+10
while time.monotonic()<end:samples.append(subject());time.sleep(.15)
after=subject();t.command('ai',paused=True);t.command('menu',open=True)
row=dict(passed=t.dist(before,after)>600 and after['failedPaths']==before['failedPaths'],
         distance=t.dist(before,after),failedPaths=after['failedPaths']-before['failedPaths'],samples=samples)
(t.OUT/'retreat-edge.json').write_text(json.dumps(row,indent=2))
print({k:v for k,v in row.items() if k!='samples'},flush=True);sys.exit(not row['passed'])
