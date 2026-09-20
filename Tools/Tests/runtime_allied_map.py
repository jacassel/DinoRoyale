"""Live packaged checks for persistent allied map positions and enemy concealment."""
import json
import time
import runtime_core as t

rows = []

def check(name, passed):
    rows.append(dict(test=name, passed=bool(passed)))
    (t.OUT / 'allied-map.json').write_text(json.dumps(rows, indent=2))
    print(('PASS ' if passed else 'FAIL ') + name, flush=True)

def actor(i):
    return next(a for a in t.state()['ai'] if a['id'] == i)

def place(i, x, y, species=1):
    t.command('testAI', id=i, species=species, x=x, y=y,
              health=1, enabled=False, yaw=0)

def current(a):
    return a['mapVisible'] and abs(a['markerX']-a['x']) < 1 and abs(a['markerY']-a['y']) < 1

t.command('menu', open=False)
t.command('species', value=0)
t.command('match', teams=True)
t.command('ai', paused=True)
t.command('sandbox', enabled=True)
t.command('teleport', x=0, y=0)
t.command('face', yaw=0)
for i in range(1, 5):
    place(i, -10000, i * 1500, 2 if i == 4 else 1)
    a = actor(i)
    check(f'Unseen quiet teammate {i} has current marker', not a['inSight'] and current(a))
time.sleep(6.5)
check('All four allied markers survive noise expiry', all(current(actor(i)) for i in range(1, 5)))
place(1, -16000, 6000)
check('Unseen ally marker follows changed position', current(actor(1)) and actor(1)['markerX'] < -15000)
place(4, 1800, 0, 2)
t.command('sightBlocker', enabled=True)
check('Occluded ally stays visible', not actor(4)['inSight'] and current(actor(4)))
t.key('M'); t.key('M', 'up')
t.command('screenshot')
check('Expanded map opens with all allied markers', t.state()['mapOpen'] and all(current(actor(i)) for i in range(1, 5)))
t.key('M'); t.key('M', 'up')
check('Minimap retains all allied markers', not t.state()['mapOpen'] and all(current(actor(i)) for i in range(1, 5)))
place(5, -10000, -6000, 0)
check('Quiet unseen enemy remains hidden', not actor(5)['inSight'] and not actor(5)['mapVisible'])
t.command('aiAbility', id=5, action='quick')
check('Enemy attack still reveals marker', actor(5)['mapVisible'])
time.sleep(6.5)
check('Enemy noise marker still expires', not actor(5)['mapVisible'])
t.command('scoreHit', attacker=5, victim=1, value=100000)
check('Dead ally has no living marker', not actor(1)['mapVisible'])
t.command('resetCombatant', id=1)
check('Respawned ally immediately returns', current(actor(1)))
t.command('damage', value=100000)
check('Allies stay visible during player respawn wait', t.state()['dead'] and all(current(actor(i)) for i in range(1, 5)))
t.command('respawn')
t.command('sightBlocker', enabled=False)
t.command('match', teams=False)
t.command('ai', paused=True)
t.command('teleport', x=0, y=0)
t.command('face', yaw=0)
place(1, -10000, 6000)
check('Switching to solo removes allied visibility', not actor(1)['mapVisible'])
t.command('species', value=1)
place(1, -10000, 6000)
check('Solo same-species raptor remains hidden', not actor(1)['mapVisible'])
t.command('menu', open=True)
print(f'RESULT {sum(r["passed"] for r in rows)}/{len(rows)}', flush=True)
raise SystemExit(any(not r['passed'] for r in rows))
