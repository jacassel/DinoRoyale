"""Capture capsule/visual offsets on both real peers before or after grounding fixes."""
import argparse,json,time
from net_harness import NetworkTest,Peer,host_url,wait_for
p=argparse.ArgumentParser();p.add_argument('--output',required=True);a=p.parse_args()
t=NetworkTest(a.output);samples=[]
try:
    h=Peer(t,'GroundHost',host_url(3,bots=True));c=Peer(t,'GroundClient')
    c.lobby(2,1);h.lobby(6);h.command('ai',paused=True)
    for sp in range(3):
        h.command('testAI',id=1,species=sp,x=1400,y=0,yaw=180,health=1,enabled=False)
        h.command('testAI',id=2,species=sp,x=2100,y=0,yaw=180,health=1,enabled=False)
        c.command('face',yaw=180);c.hold('W',.5);time.sleep(1)
        samples.append(dict(species=sp,host=h.state()['networkActors'],client=c.state()['networkActors']))
    h.command('scoreHit',attacker=0,victim=1,value=99999);time.sleep(2)
    samples.append(dict(hostCorpses=h.state()['corpses'],clientCorpses=c.state()['corpses']))
    (t.out/'grounding.json').write_text(json.dumps(samples,indent=2))
    for row in samples[:3]:
        for view in ('host','client'):
            for d in row[view]:
                if d['id']<3:print(row['species'],view,d['id'],'base',d['meshBaseZ'],'relative',round(d['meshRelativeZ'],1),'expected',-d['halfHeight'],flush=True)
finally:t.close()
