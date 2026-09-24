"""Run the existing offline core suite with its established isolated fixture."""
import json, subprocess, sys
import runtime_core as t

t.command('menu',open=False)
for key in ['W','A','S','D','Q','E','LeftShift','SpaceBar','LeftMouseButton','RightMouseButton']:
    t.key(key,'up')
t.command('species',value=0)
t.command('match',teams=False)
t.command('ai',paused=True)
t.command('sandbox',enabled=True)
t.command('invulnerable',value=False)
t.command('removeTarget')
t.command('clearTestFood')
run=subprocess.run([sys.executable,str(t.ROOT/'Tools/Tests/runtime_core.py')],timeout=600)
rows=json.loads((t.OUT/'core-live.json').read_text())
passed=run.returncode==0 and len(rows)==90 and all(row['passed'] for row in rows)
(t.OUT/'summary.json').write_text(json.dumps({'checks':len(rows),'passed':passed,'fixture':'Existing run_final_package isolation: paused AI, ignored win condition, no invulnerability'},indent=2))
sys.exit(not passed)
