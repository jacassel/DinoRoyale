"""Timed final verification: local traversal in each biome plus autonomous AI observation."""
import runtime_core as t
import time,json,math,pathlib
ROOT=t.ROOT;rows=[];snapshots=[]
regions=[('Plains',0,0),('Forest',-25000,17000),('Ridge',23000,19000),('Creek',5000,-11500),('Grove',-15000,-23000),('Hunting',27000,-5000)]
regions=[(name,x*.5,y*.5) for name,x,y in regions]
def save():
    (t.OUT/'final-world-checks.json').write_text(json.dumps(rows,indent=2))
    (t.OUT/'ai-observation.json').write_text(json.dumps(snapshots,indent=2))
def check(label,passed,**data):
    rows.append(dict(test=label,passed=bool(passed),**data));save();print(('PASS ' if passed else 'FAIL ')+label+' '+json.dumps(data),flush=True)
t.command('ai',paused=True);t.command('removeTarget');t.command('invulnerable',value=True)
for species,name in enumerate(['Trex','Raptor','Trike']):
    t.command('species',value=species)
    for region,x,y in regions:
        t.command('route',x=x,y=y);route=json.loads((t.BRIDGE/'route.json').read_text(encoding='utf-8-sig'))
        p=route['points'][-1] if route['points'] else {'x':x,'y':y}
        t.command('teleport',x=p['x'],y=p['y']);t.command('face',yaw=0);time.sleep(.5)
        a=t.state();b=t.hold('W',1.25)
        check(name+' local traversal '+region, t.dist(a,b)>300 and not b['falling'],distance=t.dist(a,b),grounded=not b['falling'])
        t.command('screenshot');time.sleep(.15)
# Observe autonomous AI without test control. A human raptor is the pack leader.
t.command('species',value=1);t.command('teleport',x=4500,y=4000);t.command('ai',paused=False)
start=time.monotonic()
while time.monotonic()-start<150:
    s=t.state();snapshots.append(s);save()
    if len(snapshots)%30==0:
        print('AI observation '+str(round(time.monotonic()-start))+' seconds',flush=True);t.command('screenshot')
    time.sleep(1)
s=t.state();major=[x for x in s['ai'] if x['major']]
check('nine AI and one human',len(major)==9 and s['majorCount']==10)
check('all major AI move',all(x['distance']>1000 for x in major),distance_by_id={x['id']:round(x['distance']) for x in major})
check('AI attacks connect',sum(x['hits'] for x in major)>10,hits_by_id={x['id']:x['hits'] for x in major})
check('no major fell under terrain',all(x['z']>x['ground']-100 for q in snapshots for x in q['ai'] if x['major']))
check('raptors recognize human leader',all(x['leader']==0 for x in major if x['species']==1 and not x['dead']))
check('prey flee',any(x['state']=='Fleeing' for q in snapshots for x in q['ai'] if not x['major']))
check('60 FPS target',s['meanFPS']>50,meanFPS=s['meanFPS'])
t.command('invulnerable',value=False);t.command('heal');t.command('screenshot')
print('DONE '+str(sum(x['passed'] for x in rows))+'/'+str(len(rows)),flush=True)
