"""Real input map pins and solo results, including an AI winner."""
import json
import time
import shutil
import runtime_core as t

rows=[]
def check(name, passed):
    rows.append(dict(test=name, passed=bool(passed)))
    (t.OUT/'map-results.json').write_text(json.dumps(rows, indent=2))
    print(('PASS ' if passed else 'FAIL ')+name, flush=True)
def press(key):
    t.key(key);t.key(key, 'up')
def capture(name):
    folder=t.BRIDGE.parent/'Screenshots/Windows'
    before=set(folder.glob('*.png'))
    t.command('screenshot')
    for _ in range(100):
        files=set(folder.glob('*.png'))-before
        if files:
            time.sleep(.5)
            shutil.copy2(max(files, key=lambda p:p.stat().st_mtime), t.OUT/(name+'.png'))
            return
        time.sleep(.1)
    raise RuntimeError('Screenshot timed out')

t.command('menu',open=False);t.command('species',value=0)
t.command('match',teams=False);t.command('ai',paused=True)
t.command('sandbox',enabled=False)
press('R')
check('R outside map no longer kills player',not t.state()['dead'] and not t.state()['mapPins'])
press('M');t.command('mouse',x=640,y=360);press('R')
s=t.state()
check('R pins map cursor at world center',len(s['mapPins'])==1 and abs(s['mapPins'][0]['x'])<1 and abs(s['mapPins'][0]['y'])<1)
t.command('mouse',x=766,y=234);press('R')
s=t.state();check('Map cursor converts to world coordinates',len(s['mapPins'])==2 and abs(s['mapPins'][1]['x']-15000)<1 and abs(s['mapPins'][1]['y']-15000)<1)
t.command('mouse',x=20,y=20);press('R')
check('Outside-map R ignored',len(t.state()['mapPins'])==2)
t.command('mouse',x=640,y=360);press('R')
check('R on existing pin removes it',len(t.state()['mapPins'])==1)
for x,y in [(430,160),(490,160),(550,160),(610,160),(670,160),(730,160),(790,160),(850,160)]:
    t.command('mouse',x=x,y=y);press('R')
check('Pins capped at eight with oldest replaced',len(t.state()['mapPins'])==8 and all(abs(p['y']-15000)>1 for p in t.state()['mapPins']))
capture('map-pins')
press('M');check('Closing map restores movement input',not t.state()['ignoreMove'] and not t.state()['mapOpen'])
a=t.state();t.hold('W',.4);check('Movement works after closing map',t.dist(a,t.state())>50)
capture('minimap-pins')
t.command('damage',value=100000);t.command('respawn')
check('Pins survive respawn',len(t.state()['mapPins'])==8)
t.command('match',teams=False);t.command('ai',paused=True)
check('New round clears pins',not t.state()['mapPins'])
# Populate an assist and a death before the human reaches five kills.
t.command('scoreHit',attacker=0,victim=1,value=50)
t.command('scoreHit',attacker=2,victim=1,value=100000)
for n in range(5):
    t.command('resetCombatant',id=1);t.command('scoreHit',attacker=0,victim=1,value=100000)
    check(f'Solo human kill {n+1} ends only at five',t.state()['roundOver']==(n==4))
s=t.state();check('Human win opens results with all ten KDA rows',s['menuOpen'] and s['winner']==0 and len(s['scoreboard'])==10 and s['kills']==5 and s['assists']==1)
check('Defeated AI deaths included',next(r for r in s['scoreboard'] if r['id']==1)['deaths']==6)
capture('solo-human-results')
t.command('mouse',x=640,y=400);press('LeftMouseButton')
check('Results clicks cannot activate hidden selection cards',t.state()['roundOver'] and t.state()['menuOpen'])
stamp=t.state()['time'];time.sleep(.4);check('Results pause the match',abs(t.state()['time']-stamp)<.05)
press('Enter');check('Enter starts a clean round',not t.state()['roundOver'] and not t.state()['menuOpen'] and all(r['kills']==0 and r['deaths']==0 and r['assists']==0 for r in t.state()['scoreboard']))
t.command('ai',paused=True)
for _ in range(5):
    t.command('resetCombatant',id=0);t.command('scoreHit',attacker=1,victim=0,value=100000)
s=t.state();check('AI reaching five also opens all-player solo results',s['roundOver'] and s['menuOpen'] and s['winner']==1 and len(s['scoreboard'])==10 and s['deaths']==5)
capture('solo-ai-results')
print(f'RESULT {sum(r["passed"] for r in rows)}/{len(rows)}',flush=True)
raise SystemExit(any(not r['passed'] for r in rows))
