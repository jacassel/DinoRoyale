"""Attach to two manually authenticated QA instances and verify real EOS travel.

No credentials, account IDs or peer addresses are read or built. Requires an
already connected EOS host/guest, using the opt-in development bridge. Leaves
both instances connected in a lobby for visual inspection; does not close them.
"""
import argparse,json,pathlib,time
from net_harness import Peer,wait_for,distance

p=argparse.ArgumentParser()
p.add_argument('--evidence',default='Tests/Results/eos-qa-20260926')
a=p.parse_args();out=pathlib.Path(a.evidence);rows=[]

class AttachedPeer(Peer):
    def __init__(self,handle,name):
        self.name=name;self.path=pathlib.Path(json.loads((out/handle).read_text(encoding='utf-8-sig'))['bridge'])

def check(name,passed,**evidence):
    row=dict(test=name,status='PASS' if passed else 'FAIL',**evidence);rows.append(row)
    print(row,flush=True)
    (out/'live-eos-results.json').write_text(json.dumps(dict(transport='LIVE EOS lobby and NetDriverEOS P2P; ForceRelays configured',accounts=2,physicalPCs=1,differentNetworkVerified=False,tests=rows),indent=2))
    if not passed:raise AssertionError(name)

host=AttachedPeer('live-handle.json','EOSHost');guest=AttachedPeer('live-guest-handle.json','EOSGuest')
check('live EOS host and guest reached replicated world',host.state()['netMode']==2 and guest.state()['netMode']==3 and len(guest.state()['players'])==2)
host.lobby(7)
check('return to shared EOS lobby',wait_for(lambda:host.state()['lobby'] and guest.state()['lobby']))
guest.lobby(0,1)
check('guest dinosaur selection replicates over EOS',wait_for(lambda:host.player(1)['species']==1))
guest.lobby(2,1)
check('guest ready replicates over EOS',wait_for(lambda:host.player(1)['ready']))
host.lobby(6)
check('host starts both EOS players',wait_for(lambda:not host.state()['lobby'] and not guest.state()['lobby']))
host.command('ai',paused=True)
before=guest.state();after=guest.hold('W',.7)
check('guest movement visible to EOS host',distance(before,after)>100 and wait_for(lambda:distance(host.actor(1),guest.state())<90),distance=distance(before,after))
before=host.state();after=host.hold('D',.6)
check('host movement visible to EOS guest',distance(before,after)>100 and wait_for(lambda:distance(guest.actor(0),host.state())<90),distance=distance(before,after))
check('guest raptor followers replicate over EOS',wait_for(lambda:len([d for d in guest.state()['networkActors'] if d['follower'] and d['pack']==1])==2))
host.command('teleport',x=0,y=0);host.command('face',yaw=0)
host.command('testAI',id=1,species=0,x=450,y=0,yaw=180,health=1,enabled=False)
guest.command('face',yaw=180);time.sleep(.7)
hp=host.state()['health'];guest.tap('LeftMouseButton')
check('guest attack damages authoritative host over EOS',wait_for(lambda:host.state()['health']<hp))
check('damage replicates to attacking EOS guest',wait_for(lambda:abs(guest.actor(0)['health']-host.state()['health'])<.1))
time.sleep(1)
host.command('testAI',id=1,species=0,x=450,y=0,yaw=180,health=.05,enabled=False)
host.command('face',yaw=0);time.sleep(.5);host.tap('LeftMouseButton')
check('host attack kills EOS guest',wait_for(lambda:guest.state()['dead']))
death=time.monotonic()
check('score replicates over EOS',wait_for(lambda:host.state()['kills']==1 and next(s for s in guest.state()['scoreboard'] if s['id']==0)['kills']==1))
time.sleep(5)
check('EOS guest remains dead during respawn delay',guest.state()['dead'])
check('EOS guest respawns after ten-second game timer',wait_for(lambda:not guest.state()['dead'],8),wallSeconds=time.monotonic()-death)
host.lobby(7)
check('EOS players return to lobby after combat',wait_for(lambda:guest.state()['lobby']))
guest.command('online',action='leave')
check('EOS guest leave cleans server roster',wait_for(lambda:guest.state()['netMode']==0 and len(host.state()['players'])==1,30))
host.command('online',action='leave')
check('EOS host can close the first lobby',wait_for(lambda:host.state()['netMode']==0 and not host.state()['onlineBusy'],30))
host.command('online',action='host',teams=False,capacity=2,bots=False,public=True)
check('EOS host can create a second lobby',wait_for(lambda:host.state()['netMode']==2 and host.state()['lobby'],45))
guest.command('online',action='search')
check('EOS discovery finds the recreated lobby',wait_for(lambda:not guest.state()['onlineBusy'] and guest.state()['onlineResults']==1,45))
guest.command('online',action='join',index=0)
check('EOS rejoin reaches two-player replicated lobby',wait_for(lambda:guest.state()['netMode']==3 and len(guest.state()['players'])==2 and len(host.state()['players'])==2,45))
print('Live EOS test finished; both windows remain in the recreated lobby.',flush=True)
