"""Real persisted team mode and count controls after a bots-off host setup."""
import argparse,time,shutil
from net_harness import NetworkTest,Peer,wait_for
p=argparse.ArgumentParser();p.add_argument('--executable',required=True);p.add_argument('--output',required=True);a=p.parse_args()
t=NetworkTest(a.output,executable=a.executable,rendered=True);t.render_size=(1600,900)
try:
 h=Peer(t,'SetupPersist','/Game/Maps/LostValley');original=h.state()['teamMode'];h.command('menu',open=True)
 if not original:h.tap('F3')
 h.quit();t.peers.clear();h=Peer(t,'SetupRestart','/Game/Maps/LostValley');s=h.state()
 t.check('persisted team mode puts offline human on a real team',s['teamMode'] and s['players'][0]['team']==0 and not s['setupWarning'])
 h.lobby(5,0);h.tap('F5')
 folder=t.bridge_root.parent/'Screenshots/Windows';before=set(folder.glob('*.png'));h.command('screenshot');assert wait_for(lambda:bool(set(folder.glob('*.png'))-before));time.sleep(.2);shutil.copyfile(max(set(folder.glob('*.png'))-before,key=lambda p:p.stat().st_mtime),t.out/'before-count-click.png')
 h.command('mouse',x=1336,y=540);h.tap('LeftMouseButton')
 t.check('first A plus starts from displayed zero bots',h.state()['teamABots']==1 and h.state()['teamBBots']==0)
 h.command('mouse',x=1336,y=621);h.tap('LeftMouseButton')
 t.check('B plus creates independent one-bot opponent',h.state()['teamABots']==1 and h.state()['teamBBots']==1 and not h.state()['setupWarning'])
 h.tap('F5');h.tap('Enter');t.check('revised setup resumes as two versus one',not h.state()['menuOpen'] and sum(v['team']==0 and not v['follower'] and v['species']!=3 for v in h.state()['networkActors'])==2)
 h.command('menu',open=True)
 if not original:h.tap('F3')
 h.quit()
finally:t.close()
