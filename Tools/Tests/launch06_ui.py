"""Mapped controls and actual game screenshots at the shipping menu aspect ratio."""
import argparse,time,shutil
from net_harness import NetworkTest,Peer
p=argparse.ArgumentParser();p.add_argument('--executable');p.add_argument('--width',type=int,default=1600);p.add_argument('--height',type=int,default=900);p.add_argument('--output',default='Tests/Results/launch06/ui-first');a=p.parse_args()
t=NetworkTest(a.output,executable=a.executable,rendered=True);t.render_size=(a.width,a.height)
try:
 h=Peer(t,'LaunchUI','/Game/Maps/LostValley');h.command('match',teams=False);h.command('ai',paused=True);h.command('sandbox',enabled=True)
 def shot(name):
  folder=t.bridge_root.parent/'Screenshots/Windows';before=set(folder.glob('*.png'));h.command('screenshot');deadline=time.monotonic()+10
  while time.monotonic()<deadline:
   files=set(folder.glob('*.png'))-before
   if files:time.sleep(.3);shutil.copyfile(max(files,key=lambda p:p.stat().st_mtime),t.out/(name+'.png'));return
   time.sleep(.1)
  raise RuntimeError('Screenshot not written')
 for key,species in [('One',0),('Two',1),('Three',2),('Four',4),('Five',5),('Six',6),('Seven',7)]:
  h.command('menu',open=True);h.tap(key);h.command('ai',paused=True);time.sleep(.5);t.check(key+' selects and starts',h.state()['species']==species and not h.state()['menuOpen']);shot('species-'+str(species))
 h.command('menu',open=True);shot('selection-seven');h.tap('F5');shot('setup-ffa')
 h.lobby(3,1);h.lobby(15,0);h.lobby(16,5);h.lobby(13,15);h.lobby(14,1);shot('setup-teams')
 h.tap('F5');shot('selection-banned');h.tap('Two');t.check('banned hotkey stays in selection',h.state()['menuOpen'] and h.state()['species']!=1)
 h.lobby(14,1);h.lobby(3,0)
 for slot,species in enumerate([0,1,2,4,5,6,7]):
  h.command('menu',open=True);h.command('mouse',x=a.width*(.063+slot%4*.223+.1025),y=a.height*(.28+slot//4*.241+.11));h.tap('LeftMouseButton');h.command('ai',paused=True)
  t.check('card click '+str(species),h.state()['species']==species and not h.state()['menuOpen'])
 h.command('species',value=7);h.command('menu',open=True);h.tap('F2');shot('settings');h.tap('F2');h.tap('Enter');h.tap('M');shot('map');h.tap('M')
finally:t.close()
