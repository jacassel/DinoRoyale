"""Fresh-process map comparison at native 1920x1080 with live AI."""
import argparse
import json
import time
import statistics
import runtime_core as t

p = argparse.ArgumentParser()
p.add_argument('--performance', action='store_true')
a = p.parse_args()
t.command('mapVariant', performance=a.performance)
t.command('console', value='t.MaxFPS 0')
t.command('console', value='r.ScreenPercentage 100')
t.command('console', value='stat unit')
t.command('menu', open=False)
t.command('match', teams=True)
t.command('ai', paused=False)
t.command('sandbox', enabled=True)
t.command('invulnerable', value=True)
rows = []
for species, name, x, y in [(0, 'plains', 0, 0), (1, 'forest', -12500, 8500), (2, 'pond', 3000, 6700)]:
    t.command('species', value=species)
    t.command('teleport', x=x, y=y)
    t.command('camera', yaw=130, pitch=-12)
    time.sleep(4)
    before = t.state()
    samples = []
    start = time.monotonic()
    while time.monotonic() - start < 20:
        samples.append(t.state())
        time.sleep(.045)
    after = t.state()
    row = dict(variant='Performance' if a.performance else 'Standard', scene=name,
               fps=(after['frameCount']-before['frameCount'])/(after['frameSeconds']-before['frameSeconds']),
               medianMs={k:statistics.median(s[k] for s in samples)
                         for k in ['frameMs', 'gameThreadMs', 'renderThreadMs', 'gpuMs']},
               p95FrameMs=sorted(s['frameMs'] for s in samples)[int((len(samples)-1)*.95)],
               actors=after['actorCount'], combatants=after['majorCount'], plants=len(after['plants']))
    rows.append(row)
    (t.OUT/'performance-native-1080.json').write_text(json.dumps(rows, indent=2))
    print(json.dumps(row), flush=True)
t.command('screenshot')
time.sleep(1)
t.command('console', value='stat unit')
t.command('console', value='r.ScreenPercentage 0')
t.command('console', value='t.MaxFPS 60')
t.command('mapVariant', performance=False)
t.command('menu', open=True)
t.key('F10')
