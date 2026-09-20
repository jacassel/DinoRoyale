"""Measure the existing plant's depletion and 120-second regrowth in game time."""
import json
import sys
import time
import runtime_core as t

t.command('menu', open=False)
t.command('ai', paused=True)
t.command('sandbox', enabled=True)
t.command('clearTestFood')
t.command('species', value=2)
t.command('teleport', x=20000, y=23000)
t.command('food')
t.command('hunger', value=0)
t.command('stamina', value=0)
time.sleep(.4)
plant_name = min(t.state()['plants'], key=lambda p: t.dist(p, t.state()))['name']

def plant():
    return next(p for p in t.state()['plants'] if p['name'] == plant_name)

try:
    assert plant()['outline'], 'Triceratops edible plant has no outline'
    t.key('E')
    deadline = time.monotonic() + 15
    while plant()['food'] > 0 and time.monotonic() < deadline:
        time.sleep(.05)
    t.key('E', 'up')
    depleted = t.state()['time']
    initial = plant()
    assert initial['hidden'] and initial['food'] == 0 and not initial['outline'], initial
    deadline = time.monotonic() + 160
    before = None
    while t.state()['time'] - depleted < 123 and time.monotonic() < deadline:
        elapsed = t.state()['time'] - depleted
        current = plant()
        if 118 <= elapsed < 119:
            before = dict(current)
        if current['food'] > 0:
            break
        time.sleep(.05)
    elapsed = t.state()['time'] - depleted
    current = plant()
    passed = (before is not None and before['hidden'] and before['food'] == 0
              and 119 <= elapsed <= 121 and current['food'] == 120 and not current['hidden'] and current['outline'])
    result = dict(passed=passed, elapsedGameSeconds=elapsed, before=before, plant=current)
    (t.OUT / 'plant-regrowth.json').write_text(json.dumps(result, indent=2))
    print(json.dumps(result), flush=True)
finally:
    t.key('E', 'up')
    t.command('sandbox', enabled=False)
    t.command('menu', open=True)
sys.exit(not passed)
