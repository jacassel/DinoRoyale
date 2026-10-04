"""Targeted standalone verification after the full 460-check source regression."""
import runtime_core as t,subprocess,sys,time,json
suites=[('runtime_core.py','core-live.json',90),('runtime_settings.py','settings-water-blood.json',28),
        ('runtime_audio.py','audio.json',36),('runtime_sprint_rules.py','sprint-rules.json',36),
        ('runtime_ai_swimming.py','ai-swimming.json',3)]
summary=[]
for script,result,count in suites:
    t.command('menu',open=False)
    for k in ['W','A','S','D','LeftControl','F','LeftShift','SpaceBar','LeftMouseButton','RightMouseButton']:t.key(k,'up')
    t.command('species',value=0);t.command('match',teams=False);t.command('ai',paused=True)
    t.command('sandbox',enabled=True);t.command('invulnerable',value=False);t.command('removeTarget');t.command('clearTestFood')
    begun=time.time()
    with (t.OUT/(script+'.log')).open('w') as log:
        run=subprocess.run([sys.executable,str(t.ROOT/'Tools/Tests'/script)],stdout=log,stderr=subprocess.STDOUT,timeout=600)
    path=t.OUT/result
    rows=json.loads(path.read_text()) if path.exists() and path.stat().st_mtime>=begun else []
    passed=run.returncode==0 and len(rows)==count and all(r.get('passed') for r in rows)
    summary.append(dict(script=script,passed=passed,checks=len(rows),seconds=round(time.time()-begun,1)))
    (t.OUT/'package-summary.json').write_text(json.dumps(summary,indent=2));print(summary[-1],flush=True)
    if not passed:break
t.command('ai',paused=True);t.command('menu',open=True)
sys.exit(not(len(summary)==len(suites) and all(r['passed'] for r in summary)))
