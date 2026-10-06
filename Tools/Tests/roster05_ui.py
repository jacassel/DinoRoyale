"""Rendered selection/hotkey/leaderboard checks with saved screenshots for review."""
import argparse,time,shutil
from net_harness import NetworkTest,Peer
p=argparse.ArgumentParser();p.add_argument('--executable');p.add_argument('--width',type=int,default=1280);p.add_argument('--height',type=int,default=720);p.add_argument('--output',default='Tests/Results/roster05/ui');a=p.parse_args()
t=NetworkTest(a.output,executable=a.executable,rendered=True);t.render_size=(a.width,a.height)
try:
 h=Peer(t,'RosterUI','/Game/Maps/LostValley');h.command('ai',paused=True);h.command('match',teams=False);h.command('sandbox',enabled=True)
 def shot(name):
  folder=t.bridge_root.parent/'Screenshots/Windows';before=set(folder.glob('*.png'));h.command('screenshot');deadline=time.monotonic()+10
  while time.monotonic()<deadline:
   files=set(folder.glob('*.png'))-before
   if files:time.sleep(.3);shutil.copyfile(max(files,key=lambda p:p.stat().st_mtime),t.out/(name+'.png'));return
   time.sleep(.1)
  raise RuntimeError('Screenshot not written')
 for key,species in [('One',0),('Two',1),('Three',2),('Four',4),('Five',5),('Six',6)]:
  h.command('menu',open=True);h.tap(key);h.command('ai',paused=True);time.sleep(.5);t.check(key+' selects correct species and closes menu',h.state()['species']==species and not h.state()['menuOpen']);shot('species-'+str(species))
 h.command('menu',open=True);shot('selection-six')
 for slot,species in enumerate([0,1,2,4,5,6]):
  h.command('menu',open=True);h.command('mouse',x=a.width*(.085+slot%3*.285+.13),y=a.height*(.28+slot//3*.241+.11));h.tap('LeftMouseButton');h.command('ai',paused=True)
  t.check('card click '+str(species),h.state()['species']==species and not h.state()['menuOpen'])
 h.command('species',value=0);h.command('match',teams=False);h.command('ai',paused=True);h.command('sandbox',enabled=True)
 h.command('scoreHit',attacker=1,victim=3,value=100000);time.sleep(.3);h.tap('P');t.check('P shows live AI leader',h.state()['leaderboardOpen'] and h.state()['leaderboardIDs'][0]==1);shot('leaderboard-live')
 h.tap('P');shot('leader-black-hud');h.command('menu',open=True);h.tap('F2');shot('settings');h.tap('F2');h.tap('Enter');h.tap('M');shot('map');h.tap('M')
finally:t.close()
