"""Actual ten-second respawn avoids an enemy camping the home point."""
import runtime_core as t,time,json
rows=[]
t.command('menu',open=False);t.command('sandbox',enabled=True)
for sp in range(3):
    t.command('species',value=sp);t.command('match',teams=False);t.command('ai',paused=True)
    t.command('invulnerable',value=False);t.command('teleport',x=0,y=0)
    t.command('testAI',id=1,species=0,x=400,y=0,health=1,enabled=False,yaw=180)
    t.command('scoreHit',attacker=1,victim=0,value=10000)
    start=t.state()['time'];deadline=time.monotonic()+25
    while t.state()['dead'] and time.monotonic()<deadline:time.sleep(.05)
    s=t.state();enemy=next(a for a in s['ai'] if a['id']==1);distance=t.dist(s,enemy)
    rows.append(dict(species=sp,passed=not s['dead'] and distance>=3000 and s['health']==s['maxHealth'],
                     seconds=s['time']-start,enemyDistance=distance,health=s['health']))
    print(rows[-1],flush=True)
(t.OUT/'safe-respawn.json').write_text(json.dumps(rows,indent=2));t.command('menu',open=True)
assert all(r['passed'] for r in rows)
