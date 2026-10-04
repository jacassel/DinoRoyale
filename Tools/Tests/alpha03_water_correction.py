"""Exercise water state after a movement-mode correction; not a teleport recovery."""
import argparse,time,json
from net_harness import NetworkTest,Peer,host_url,wait_for
p=argparse.ArgumentParser();p.add_argument('--output',required=True);a=p.parse_args()
t=NetworkTest(a.output);evidence=[]
try:
    h=Peer(t,'CorrectionHost',host_url(2));c=Peer(t,'CorrectionClient');c.lobby(2,1)
    assert wait_for(lambda:h.player(1)['ready']);h.lobby(6);assert wait_for(lambda:not c.state()['lobby']);h.command('ai',paused=True)
    for sp in range(3):
        h.command('testAI',id=1,species=sp,x=3000,y=4500,yaw=0,health=1,enabled=False)
        assert wait_for(lambda:c.state()['swimming']);time.sleep(2)
        before=h.actor(1);h.command('waterCorrection',id=1);time.sleep(2)
        after=h.actor(1);evidence.append(dict(species=sp,before=before,after=after,client=c.actor(1)))
        (t.out/'evidence.json').write_text(json.dumps(evidence,indent=2))
        t.check(f'{sp} swim mode recovers after correction',after['swimming'] and after['movementMode']==6 and after['z']>75,swimming=after['swimming'],mode=after['movementMode'],z=after['z'])
finally:t.close()
