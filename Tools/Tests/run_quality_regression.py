"""Sequential sprint regression; fail closed on stale/incomplete output."""
import os,sys,pathlib,subprocess,time,json
ROOT=pathlib.Path(__file__).resolve().parents[2]
OUT=ROOT/'Tests/Results/quality-sprint'
OUT.mkdir(exist_ok=True)
os.environ['DINO_RESULTS_DIR']=str(OUT)
import runtime_core as t
suites=[('runtime_sprint_rules.py','sprint-rules.json',36),('runtime_audio.py','audio.json',36),
('runtime_core.py','core-live.json',90),('runtime_combat_polish.py','combat-polish.json',69),
('runtime_combat_followup.py','combat-followup.json',17),('runtime_integration.py','integration-live.json',41),
('runtime_settings.py','settings-water-blood.json',28),('runtime_matches.py','match-rules.json',24),
('runtime_swimming.py','swimming.json',40),('runtime_ecology.py','ecology-visibility.json',42),
('runtime_ecology_edges.py','ecology-edges.json',5),('runtime_tactics.py','ai-tactics.json',11),
('runtime_traversal.py','full-traversal.json',18),('runtime_ai_swimming.py','ai-swimming.json',3)]
summary=[]
start=0
if len(sys.argv)>1:
    start=[x[0] for x in suites].index(sys.argv[1]);summary=json.loads((OUT/'summary.json').read_text())[:start]
    if len(summary)!=start or not all(r['passed'] for r in summary):raise SystemExit('Earlier suites must pass before resuming')
for script,result,count in suites[start:]:
    t.command('menu',open=False)
    for k in ['W','A','S','D','LeftControl','F','LeftShift','SpaceBar','LeftMouseButton','RightMouseButton']:t.key(k,'up')
    t.command('species',value=0);t.command('match',teams=False);t.command('ai',paused=True);t.command('sandbox',enabled=True);t.command('invulnerable',value=False);t.command('removeTarget');t.command('clearTestFood')
    begun=time.time();print('START',script,flush=True)
    with (OUT/(script+'.log')).open('w') as log:
        run=subprocess.run([sys.executable,str(ROOT/'Tools/Tests'/script)],stdout=log,stderr=subprocess.STDOUT,timeout=1200)
    p=OUT/result;rows=json.loads(p.read_text()) if p.exists() and p.stat().st_mtime>=begun else []
    passed=run.returncode==0 and len(rows)==count and all(r.get('passed') is True for r in rows)
    summary.append(dict(script=script,passed=passed,checks=len(rows),expected=count,seconds=round(time.time()-begun,1),exitCode=run.returncode))
    (OUT/'summary.json').write_text(json.dumps(summary,indent=2));print(summary[-1],flush=True)
    if not passed:break
t.command('ai',paused=True);t.command('menu',open=True)
sys.exit(not(len(summary)==len(suites) and all(r['passed'] for r in summary)))
