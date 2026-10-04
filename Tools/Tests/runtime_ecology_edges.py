"""Ecology edge cases and deferred plant regrowth verification."""
import runtime_core as t,time,json,sys
rows=[]
def wait(sec):
 a=t.state()['time']
 while t.state()['time']-a<sec:time.sleep(.025)
def check(n,ok,**v):
 rows.append(dict(test=n,passed=bool(ok),**v));(t.OUT/'ecology-edges.json').write_text(json.dumps(rows,indent=2));print(('PASS ' if ok else 'FAIL ')+n+' '+json.dumps(v),flush=True)
t.command('menu',open=False);t.command('ai',paused=True);t.command('sandbox',enabled=True);t.command('species',value=2);t.command('clearTestFood');t.command('teleport',x=20000,y=23000);t.command('food');t.command('hunger',value=0);t.command('stamina',value=0);t.command('damage',value=t.state()['maxHealth']*.75);wait(2.7)
p=min(t.state()['plants'],key=lambda p:t.dist(p,t.state()));t.key('F');wait(5.6);t.key('F','up');p=next(x for x in t.state()['plants'] if x['name']==p['name']);check('depleted remote plant hidden',p['hidden'] and p['food']==0);(t.OUT/'regrowth-pending.json').write_text(json.dumps(dict(name=p['name'],time=t.state()['time'])))
t.command('species',value=0);t.command('hunger',value=0);t.command('damage',value=t.state()['health']-1);wait(.6);check('starvation causes real death',t.state()['dead']);check('starvation leaves edible carcass',any(c['source']==0 for c in t.state()['corpses']));wait(10.1);s=t.state();check('starvation respawn restores resources and control',not s['dead'] and s['hunger']>99 and s['stamina']==100 and s['health']==s['maxHealth'])
# An off-camera opponent holding a windup remains noisy beyond the initial six-second window.
t.command('testAI',id=1,species=0,x=2500,y=0,health=1,enabled=False,yaw=180,personality=3);t.command('camera',yaw=180);t.command('aiAbility',id=1,action='charge');wait(7.2);a=next(a for a in t.state()['ai'] if a['id']==1);check('held charge keeps its unseen map reveal active',a['charging'] and a['mapVisible'] and not a['inSight']);t.command('aiAbility',id=1,action='release');wait(2)
t.command('species',value=0);t.command('ai',paused=True);print('RESULT',sum(x['passed'] for x in rows),'/',len(rows),flush=True);sys.exit(any(not x['passed'] for x in rows))
