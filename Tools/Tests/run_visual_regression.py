"""Bounded regression for visual asset changes; preserves earlier sprint evidence."""
import os,sys,pathlib,subprocess,time,json
ROOT=pathlib.Path(__file__).resolve().parents[2]
OUT=pathlib.Path(os.environ.get('DINO_RESULTS_DIR',str(ROOT/'Tests/Results/visual-modernization/regression')))
OUT.mkdir(parents=True,exist_ok=True);os.environ['DINO_RESULTS_DIR']=str(OUT)
os.environ.setdefault('DINO_TEST_WIDTH','1600');os.environ.setdefault('DINO_TEST_HEIGHT','900')
import runtime_core as t
suites=[('runtime_sprint_rules.py','sprint-rules.json',36),
('runtime_core.py','core-live.json',90),('runtime_combat_polish.py','combat-polish.json',69),
('runtime_combat_followup.py','combat-followup.json',17),('runtime_integration.py','integration-live.json',41),
('runtime_settings.py','settings-water-blood.json',28),('runtime_matches.py','match-rules.json',24),
('runtime_swimming.py','swimming.json',40),('runtime_ecology.py','ecology-visibility.json',42),
('runtime_ecology_edges.py','ecology-edges.json',5),('runtime_tactics.py','ai-tactics.json',11),
('runtime_traversal.py','full-traversal.json',18),('runtime_ai_swimming.py','ai-swimming.json',3),
('runtime_allied_map.py','allied-map.json',17),('runtime_map_results.py','map-results.json',21)]
if '--package' in sys.argv:
    suites=[s for s in suites if s[0] in ['runtime_core.py','runtime_settings.py','runtime_matches.py','runtime_swimming.py','runtime_allied_map.py','runtime_map_results.py']]
summary=[]
if '--resume' in sys.argv:
    prior=json.loads((OUT/'summary.json').read_text())
    for row,suite in zip(prior,suites):
        if not row['passed']:break
        assert row['script']==suite[0] and row['checks']==suite[2]
        summary.append(row)
    suites=suites[len(summary):]
expected_total=len(summary)+len(suites)
for script,result,count in suites:
    t.command('menu',open=False)
    for k in ['W','A','S','D','LeftControl','F','LeftShift','SpaceBar','LeftMouseButton','RightMouseButton']:t.key(k,'up')
    t.command('species',value=0);t.command('match',teams=False);t.command('ai',paused=True);t.command('sandbox',enabled=True)
    t.command('invulnerable',value=False);t.command('removeTarget');t.command('clearTestFood')
    begun=time.time();print('START',script,flush=True)
    with (OUT/(script+'.log')).open('w') as log:
        run=subprocess.run([sys.executable,str(ROOT/'Tools/Tests'/script)],stdout=log,stderr=subprocess.STDOUT,timeout=1200)
    p=OUT/result;rows=json.loads(p.read_text()) if p.exists() and p.stat().st_mtime>=begun else []
    passed=run.returncode==0 and len(rows)==count and all(r.get('passed') is True for r in rows)
    summary.append(dict(script=script,passed=passed,checks=len(rows),expected=count,seconds=round(time.time()-begun,1),exitCode=run.returncode))
    (OUT/'summary.json').write_text(json.dumps(summary,indent=2));print(summary[-1],flush=True)
    if not passed:break
t.command('ai',paused=True);t.command('menu',open=True)
sys.exit(not(len(summary)==expected_total and all(r['passed'] for r in summary)))
