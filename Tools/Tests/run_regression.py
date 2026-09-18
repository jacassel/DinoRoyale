"""Sequential regression suite; run only when other bridge-driven tests have finished."""
import runtime_core as t,subprocess,sys,json
t.command('match',teams=False);t.command('ai',paused=True);t.command('sandbox',enabled=True);t.command('invulnerable',value=False)
summary=[]
for script in ['runtime_core.py','runtime_integration.py','runtime_settings.py','runtime_matches.py']:
    t.command('ai',paused=True)
    log=t.ROOT/('regression-'+script+'.log')
    with log.open('w') as out:r=subprocess.run([sys.executable,str(t.ROOT/'Tools/Tests'/script)],stdout=out,stderr=subprocess.STDOUT)
    summary.append(dict(script=script,exitCode=r.returncode));(t.OUT/'regression-summary.json').write_text(json.dumps(summary,indent=2));print(script+' exit '+str(r.returncode),flush=True)
t.command('sandbox',enabled=False);t.command('ai',paused=True);t.command('menu',open=True)
sys.exit(any(r['exitCode'] for r in summary))
