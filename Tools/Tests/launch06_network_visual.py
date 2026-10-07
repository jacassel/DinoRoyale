"""Rendered guest views of host rules and replicated attack clips."""
import argparse,time,shutil
from net_harness import NetworkTest,Peer,host_url,wait_for
p=argparse.ArgumentParser();p.add_argument('--executable',required=True);p.add_argument('--output',required=True);a=p.parse_args()
t=NetworkTest(a.output,executable=a.executable,rendered=True);t.offscreen=True;t.render_size=(1600,900)
def shot(peer,name):
 folder=t.bridge_root.parent/'Screenshots/Windows';before=set(folder.glob('*.png'));peer.command('screenshot')
 assert wait_for(lambda:bool(set(folder.glob('*.png'))-before))
 time.sleep(.2);shutil.copyfile(max(set(folder.glob('*.png'))-before,key=lambda p:p.stat().st_mtime),t.out/(name+'.png'))
try:
 h=Peer(t,'VisualHost',host_url(10,teams=True,bots=True));g=Peer(t,'VisualGuest')
 h.lobby(1,0);g.lobby(1,0);h.lobby(15,1);h.lobby(16,4);h.lobby(13,15);h.lobby(14,5)
 t.check('rendered guest receives score bans and unequal setup',wait_for(lambda:g.state()['goal']==15 and g.state()['allowedSpeciesMask']==215 and g.state()['teamABots']==1 and g.state()['teamBBots']==4))
 shot(g,'guest-lobby-ban');h.tap('F5');g.tap('F5');shot(h,'host-setup');shot(g,'guest-readonly-setup')
 g.command('mouse',x=140,y=365);g.tap('LeftMouseButton');t.check('guest setup click cannot change host ban mask',h.state()['allowedSpeciesMask']==215)
 h.tap('F5');g.tap('F5');h.lobby(14,5);g.lobby(2,1)
 assert wait_for(lambda:h.player(1)['ready']);h.lobby(6);assert wait_for(lambda:not g.state()['lobby'])
 h.command('sandbox',enabled=True);h.command('ai',paused=True)
 for sp in [4,5,7]:
  h.command('testAI',id=0,species=sp,x=0,y=0,yaw=0,health=1,enabled=False)
  h.command('testAI',id=1,species=0,x=1300,y=400,yaw=180,health=1,enabled=False)
  g.command('camera',yaw=165,pitch=-10,distance=2100);h.command('face',yaw=0);time.sleep(.8)
  shot(g,f'{sp}-guest-idle');h.tap('LeftMouseButton')
  t.check(f'{sp} quick clip replicates to rendered guest',wait_for(lambda:g.actor(0)['animation'].startswith('Quick'),2))
  shot(g,f'{sp}-guest-quick');time.sleep(2)
  h.key('RightMouseButton');t.check(f'{sp} charge clip replicates',wait_for(lambda:g.actor(0)['animation']=='Charge',2));time.sleep(1.7);shot(g,f'{sp}-guest-charge');h.key('RightMouseButton','up')
  t.check(f'{sp} heavy clip replicates',wait_for(lambda:g.actor(0)['animation']=='Heavy',2));shot(g,f'{sp}-guest-heavy')
  t.check(f'{sp} replicated attack returns to locomotion',wait_for(lambda:g.actor(0)['animation'] in ['Idle','Walk','Run'],5))
 g.quit();h.quit()
finally:t.close()
