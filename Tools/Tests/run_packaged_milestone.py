"""Run existing suites sequentially against an already running packaged dev bridge.

Set DINO_BRIDGE_DIR to the packaged Saved/Automation directory. Never run another
bridge-driven test concurrently. Results are retained separately from editor runs.
"""
import json
import os
import shutil
import subprocess
import sys
import time
import runtime_core as t

if 'DINO_BRIDGE_DIR' not in os.environ:
    raise SystemExit('Set DINO_BRIDGE_DIR explicitly to the packaged game bridge.')

suites = [
    ('runtime_combat_polish.py', 'combat-polish.json', 69),
    ('runtime_combat_followup.py', 'combat-followup.json', 17),
    ('runtime_integration.py', 'integration-live.json', 41),
    ('runtime_settings.py', 'settings-water-blood.json', 28),
    ('runtime_matches.py', 'match-rules.json', 24),
    ('runtime_swimming.py', 'swimming.json', 40),
    ('runtime_ecology.py', 'ecology-visibility.json', 42),
    ('runtime_ecology_edges.py', 'ecology-edges.json', 5),
    ('runtime_tactics.py', 'ai-tactics.json', 11),
    ('runtime_personalities.py', 'combat-personalities.json', 12),
    ('runtime_pack_fights.py', 'combat-pack-fights.json', 3),
]
destination = t.OUT / 'packaged-milestone'
destination.mkdir(exist_ok=True)
summary = []
start_at = 0
if len(sys.argv) > 1:
    start_at = [suite[0] for suite in suites].index(sys.argv[1])
    summary = json.loads((destination / 'summary.json').read_text())[:start_at]
    if len(summary) != start_at or not all(row['passed'] for row in summary):
        raise SystemExit('Resume requires passing records for all earlier suites.')

def valid(row):
    if 'passed' in row:
        return row['passed'] is True
    if 'rulesValid' in row:
        return row['rulesValid'] and row['failedPaths'] == 0
    if 'staminaValid' in row:
        return row['staminaValid'] and all(a['paths'] == 0 for a in row['ai'])
    return False

try:
    for script, result_file, expected in suites[start_at:]:
        t.command('menu', open=False)
        for key in ['W', 'A', 'S', 'D', 'Q', 'E', 'LeftShift', 'SpaceBar',
                    'RightMouseButton', 'LeftMouseButton']:
            t.key(key, 'up')
        t.command('match', teams=False)
        t.command('ai', paused=True)
        t.command('sandbox', enabled=True)
        t.command('invulnerable', value=False)
        t.command('clearTestFood')
        started = time.time()
        print('START ' + script, flush=True)
        with (destination / (script + '.log')).open('w') as log:
            run = subprocess.run([sys.executable, str(t.ROOT / 'Tools/Tests' / script)],
                                 stdout=log, stderr=subprocess.STDOUT, timeout=900)
        result = t.OUT / result_file
        rows = json.loads(result.read_text()) if result.exists() and result.stat().st_mtime >= started else []
        passed = run.returncode == 0 and len(rows) == expected and all(valid(row) for row in rows)
        if rows:
            shutil.copyfile(result, destination / result_file)
        summary.append(dict(script=script, passed=passed, exitCode=run.returncode,
                            checks=len(rows), expected=expected,
                            seconds=round(time.time() - started, 1)))
        (destination / 'summary.json').write_text(json.dumps(summary, indent=2))
        print(json.dumps(summary[-1]), flush=True)
        if not passed:
            break
finally:
    for key in ['W', 'A', 'S', 'D', 'Q', 'E', 'LeftShift', 'SpaceBar',
                'RightMouseButton', 'LeftMouseButton']:
        try:
            t.key(key, 'up')
        except Exception:
            pass
    t.command('ai', paused=True)
    t.command('sandbox', enabled=False)
    t.command('menu', open=True)

sys.exit(not (len(summary) == len(suites) and all(row['passed'] for row in summary)))
