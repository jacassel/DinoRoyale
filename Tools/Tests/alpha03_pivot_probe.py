"""Record predicted and authoritative yaw around repeated key edges under latency."""
import argparse,time,json
from net_harness import NetworkTest,Peer,host_url,wait_for
p=argparse.ArgumentParser();p.add_argument('--output',required=True);p.add_argument('--executable');a=p.parse_args()
t=NetworkTest(a.output,executable=a.executable,lag=75);rows=[]
def sample(label):
    local=c.state();server=h.state();ca=next(x for x in local['networkActors'] if x['id']==1);ha=next(x for x in server['networkActors'] if x['id']==1)
    row=dict(phase=label,wall=time.monotonic(),clientTime=local['time'],serverTime=server['time'],client={k:ca[k] for k in ['species','yaw','pivot','speed','animation']},host={k:ha[k] for k in ['species','yaw','pivot','speed','animation']})
    rows.append(row);(t.out/'probe.json').write_text(json.dumps(rows,indent=2));return row
def observe(label,seconds):
    end=time.monotonic()+seconds
    while time.monotonic()<end:sample(label);time.sleep(.025)
try:
    h=Peer(t,'ProbeHost',host_url(2));c=Peer(t,'ProbeClient');c.lobby(2,1);assert wait_for(lambda:h.player(1)['ready']);h.lobby(6);assert wait_for(lambda:not c.state()['lobby']);h.command('ai',paused=True)
    for sp in range(3):
        h.command('testAI',id=1,species=sp,x=1000,y=0,yaw=0,health=1,enabled=False);h.command('ai',paused=True);c.command('face',yaw=0);observe(str(sp)+' settle',2)
        for trial in range(3):
            for key in ['Q','E']:
                sample(f'{sp}/{trial}/{key} before');c.key(key);observe(f'{sp}/{trial}/{key} held',.6)
                at_release=sample(f'{sp}/{trial}/{key} release');c.key(key,'up');observe(f'{sp}/{trial}/{key} released',1.5)
                last=sample('steady');delta=(last['host']['yaw']-last['client']['yaw']+180)%360-180
                print(dict(species=sp,trial=trial,key=key,release=at_release['client']['yaw'],client=last['client'],host=last['host'],error=delta),flush=True)
    c.quit();h.quit()
finally:t.close()
