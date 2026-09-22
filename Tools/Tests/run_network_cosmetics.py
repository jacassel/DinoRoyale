"""Different local blood preferences with real replicated hits."""
import argparse,time
from net_harness import NetworkTest,Peer,host_url,wait_for
p=argparse.ArgumentParser();p.add_argument('--executable',required=True);a=p.parse_args()
t=NetworkTest('Tests/Results/multiplayer/packaged-cosmetics',executable=a.executable)
def blood(peer,on):
    peer.command('menu',open=True);peer.tap('F2')
    if peer.state()['bloodEnabled']!=on:peer.tap('B')
    peer.command('menu',open=False)
try:
    host=Peer(t,'BloodHost',host_url(2));client=Peer(t,'BloodClient');client.lobby(2,1);host.lobby(6);host.command('ai',paused=True)
    blood(host,True);blood(client,False)
    t.check('players keep different local blood preferences',host.state()['bloodEnabled'] and not client.state()['bloodEnabled'])
    host.command('teleport',x=0,y=0);host.command('face',yaw=0)
    host.command('testAI',id=1,species=0,x=450,y=0,yaw=180,health=1,enabled=False);client.command('face',yaw=180);time.sleep(.6)
    before=host.state().get('bloodEmitted',0);client.tap('LeftMouseButton');time.sleep(.8)
    t.check('replicated hit renders blood only for opted-in host',host.state().get('bloodEmitted',0)>before and client.state().get('bloodEmitted',0)==0)
    blood(host,False);blood(client,True);time.sleep(.5)
    before=client.state().get('bloodEmitted',0);host_before=host.state().get('bloodEmitted',0);client.tap('LeftMouseButton');time.sleep(.8)
    t.check('replicated hit renders blood only for opted-in client',client.state().get('bloodEmitted',0)>before and host.state().get('bloodEmitted',0)==host_before)
    blood(client,False);client.quit();host.quit()
finally:t.close()
