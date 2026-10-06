"""Run existing independent gameplay regression suites against an owned game process."""
import argparse,subprocess,sys,os
from net_harness import NetworkTest,Peer,ROOT
p=argparse.ArgumentParser();p.add_argument('--executable');p.add_argument('--output',default='Tests/Results/roster05/regression');p.add_argument('--scripts',nargs='+',default=['runtime_core.py','runtime_matches.py','runtime_swimming.py','runtime_ecology.py']);a=p.parse_args()
t=NetworkTest(a.output,executable=a.executable,rendered=True);t.render_size=(1280,720)
try:
 h=Peer(t,'RosterRegression','/Game/Maps/LostValley');h.command('menu',open=False);h.command('ai',paused=True)
 for script in a.scripts:
  out=t.out/script.removesuffix('.py');out.mkdir(parents=True,exist_ok=True)
  env=dict(os.environ,DINO_BRIDGE_DIR=str(h.path),DINO_RESULTS_DIR=str(out))
  with (out/'console.log').open('w') as f:r=subprocess.run([sys.executable,str(ROOT/'Tools/Tests'/script)],env=env,stdout=f,stderr=subprocess.STDOUT)
  t.check(script+' exits successfully',r.returncode==0)
finally:t.close()
