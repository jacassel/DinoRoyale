"""Measure warmed real frame counts and sampled frame timing, with live AI at 1600x900."""
import runtime_core as t,time,json,statistics
rows=[]
t.command('menu',open=False);t.command('match',teams=True);t.command('ai',paused=False)
t.command('sandbox',enabled=True);t.command('invulnerable',value=True)
for i,name,x,y in [(0,'plains',0,0),(1,'forest',-12500,8500),(2,'pond',3000,6700)]:
    t.command('species',value=i);t.command('teleport',x=x,y=y);t.command('camera',yaw=130,pitch=-12)
    time.sleep(4);a=t.state();samples=[];start=time.monotonic()
    while time.monotonic()-start<20:
        s=t.state();samples.append(s['frameMs']);time.sleep(.045)
    b=t.state();ordered=sorted(samples)
    rows.append(dict(region=name,species=i,seconds=b['frameSeconds']-a['frameSeconds'],
        fps=(b['frameCount']-a['frameCount'])/(b['frameSeconds']-a['frameSeconds']),
        sampledFrames=len(samples),medianMs=statistics.median(samples),p95Ms=ordered[int(len(ordered)*.95)],
        p99Ms=ordered[int(len(ordered)*.99)],maxMs=max(samples)))
    (t.OUT/'performance.json').write_text(json.dumps(rows,indent=2));print(rows[-1],flush=True)
t.command('ai',paused=True);t.command('menu',open=True)
