"""Uncapped rendered Standard/Performance comparison with live combatants."""
import argparse,json,statistics,time,math
import runtime_core as t
p=argparse.ArgumentParser();p.add_argument('--seconds',type=int,default=20);a=p.parse_args();rows=[]
def distribution(values):
    values=sorted(values)
    return dict(median=statistics.median(values),p95=values[int((len(values)-1)*.95)],p99=values[int((len(values)-1)*.99)],maximum=max(values))
t.command('console',value='t.MaxFPS 0');t.command('console',value='stat unit');t.command('menu',open=False)
for performance in [False,True]:
    t.command('mapVariant',performance=performance)
    for teams in [False,True]:
        for scene,x,y,moving in [('plains',0,0,False),('forest',-12500,8500,False),('pond',3000,6700,False),('close-combat',0,0,False),('traversal',-7000,-11000,True)]:
            t.command('species',value=0);t.command('match',teams=teams);t.command('sandbox',enabled=True);t.command('invulnerable',value=True)
            t.command('teleport',x=x,y=y);t.command('camera',yaw=130 if not moving else 0,pitch=-12);t.command('ai',paused=False)
            if scene=='close-combat':
                for ident in range(1,10):
                    angle=ident*math.tau/9
                    t.command('testAI',id=ident,species=(ident-1)//3,x=math.cos(angle)*1500,y=math.sin(angle)*1500,yaw=math.degrees(angle)+180,health=1,enabled=True)
            time.sleep(4)
            if moving:t.key('W');t.key('LeftShift')
            before=t.state();samples=[];start=time.monotonic();next_attack=0
            while time.monotonic()-start<a.seconds:
                s=t.state();samples.append(s)
                if scene=='close-combat' and time.monotonic()>next_attack:
                    t.key('LeftMouseButton');t.key('LeftMouseButton','up');next_attack=time.monotonic()+.9
                time.sleep(.045)
            after=t.state();t.key('W','up');t.key('LeftShift','up')
            row=dict(variant='Performance' if performance else 'Standard',mode='teams' if teams else 'ffa',scene=scene,
                seconds=after['frameSeconds']-before['frameSeconds'],fps=(after['frameCount']-before['frameCount'])/(after['frameSeconds']-before['frameSeconds']),
                sampledFrames=len(samples),timings={k:distribution([s[k] for s in samples]) for k in ['frameMs','gameThreadMs','renderThreadMs','gpuMs'] if k in after},
                hitchesOver33ms=sum(s['frameMs']>33.3 for s in samples),hitchesOver50ms=sum(s['frameMs']>50 for s in samples),
                counts={k:after.get(k) for k in ['actorCount','majorCount','treeInstances','grassInstances','fernInstances','navObstacles']},
                plants=len(after['plants']),memoryStartMB=before.get('memoryUsedMB'),memoryEndMB=after.get('memoryUsedMB'),
                movement=math.hypot(after['x']-before['x'],after['y']-before['y']),endHealth=after['health'])
            rows.append(row);(t.OUT/'performance-matrix.json').write_text(json.dumps(rows,indent=2));print(json.dumps(row),flush=True)
t.command('ai',paused=True);t.command('console',value='stat unit');t.command('console',value='t.MaxFPS 60');t.command('menu',open=True)
