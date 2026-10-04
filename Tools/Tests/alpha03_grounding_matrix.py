"""Observe actual replicated meshes and persistent bodies across actions and terrain."""
import argparse,time,json,math
from net_harness import NetworkTest,Peer,host_url,wait_for
p=argparse.ArgumentParser();p.add_argument('--output',required=True);p.add_argument('--executable');p.add_argument('--lag',type=int,default=75);a=p.parse_args()
t=NetworkTest(a.output,executable=a.executable,lag=a.lag);evidence=[]
def place(id,sp,x,y=0,hp=1,yaw=0):
    h.command('testAI',id=id,species=sp,x=x,y=y,yaw=yaw,health=hp,enabled=False)
def mesh_samples(label,seconds=1):
    samples=[];end=time.monotonic()+seconds
    while time.monotonic()<end:
        for observer,peer,ids in [('host',h,[1,2]),('client',c,[0,2])]:
            for ident in ids:samples.append(dict(observer=observer,**peer.actor(ident)))
        time.sleep(.09)
    bad=[s for s in samples if abs(s['meshBaseZ']+s['halfHeight'])>.2 or abs(s['meshRelativeZ']+s['halfHeight'])>max(45,s['halfHeight']*.45)]
    t.check(label+' remote mesh follows capsule across both observers',not bad,samples=len(samples),bad=bad[:2])
    evidence.append(dict(action=label,samples=samples))
    (t.out/'mesh-samples.json').write_text(json.dumps(evidence,indent=2))
try:
    h=Peer(t,'GroundHost',host_url(3,bots=True));c=Peer(t,'GroundClient');c.lobby(2,1)
    assert wait_for(lambda:h.player(1)['ready']);h.lobby(6);assert wait_for(lambda:not c.state()['lobby']);h.command('sandbox',enabled=True);h.command('ai',paused=True)
    for sp in range(3):
        for id,x,y in [(0,0,0),(1,0,1400),(2,1800,1400)]:place(id,sp,x,y)
        h.command('ai',paused=True);h.command('face',yaw=0);c.command('face',yaw=0);time.sleep(1)
        mesh_samples(f'{sp} standing')
        for key,label in [('W','walking'),('Q','pivoting'),('LeftControl','bracing')]:
            for peer in [h,c]:peer.key(key)
            mesh_samples(f'{sp} {label}')
            for peer in [h,c]:peer.key(key,'up')
            time.sleep(.5)
        for peer in [h,c]:peer.key('W');peer.key('LeftShift')
        mesh_samples(f'{sp} sprinting')
        for peer in [h,c]:peer.key('W','up');peer.key('LeftShift','up');peer.tap('LeftMouseButton')
        mesh_samples(f'{sp} quick attack')
        time.sleep(1)
        for peer in [h,c]:peer.key('RightMouseButton')
        time.sleep(1.5)
        for peer in [h,c]:peer.key('RightMouseButton','up')
        mesh_samples(f'{sp} committed heavy attack')
        time.sleep(2)
        for id,x,y in [(0,3000,4100),(1,3000,4900),(2,4000,4500)]:place(id,sp,x,y)
        h.command('ai',paused=True);time.sleep(2);mesh_samples(f'{sp} swimming')
        h.command('face',yaw=270);c.command('face',yaw=90)
        h.key('W');c.key('W');time.sleep(4.5);mesh_samples(f'{sp} bank exit',1.5);h.key('W','up');c.key('W','up')
    # Actual seeded obstacles provide land fixtures outside their collision margins.
    fixtures=[('flat',0,0),('slope',11500,9500),('pond-edge',3000,6550),('water',3000,4500)]
    fixtures += [(r['kind'],r['x'],r['y']) for r in h.state()['obstacleFixtures']]
    bodies=[]
    for sp in range(3):
        for label,x,y in fixtures:
            place(1,sp,x,y);h.command('ai',paused=True);time.sleep(1.3)
            old={b['name'] for b in h.state()['corpses']};old_client={b['name'] for b in c.state()['corpses']};h.command('scoreHit',attacker=0,victim=1,value=99999)
            assert wait_for(lambda:any(b['name'] not in old_client for b in c.state()['corpses']))
            time.sleep(2.2)
            host=next(b for b in h.state()['corpses'] if b['name'] not in old)
            client=next(b for b in c.state()['corpses'] if b['name'] not in old_client and b['source']==host['source'])
            ground_ok=abs(host['bodyZ']-host['bodyGround'])<45 if label!='water' else 90<host['bodyZ']<185
            t.check(f'{sp} {label} body grounded or floating at water surface',ground_ok,bodyZ=host['bodyZ'],ground=host['bodyGround'])
            t.check(f'{sp} {label} authoritative body and food agree',abs(host['bodyZ']-client['bodyZ'])<2 and math.hypot(host['x']-client['x'],host['y']-client['y'])<2 and host['food']==client['food']==[360,120,480][sp] and client['frozen'])
            bodies.append(dict(species=sp,location=label,host=host,client=client))
            (t.out/'body-samples.json').write_text(json.dumps(bodies,indent=2))
    before=dict(client);assert wait_for(lambda:not c.state()['dead'],12)
    c.hold('W',.7);after=next(b for b in c.state()['corpses'] if b['name']==before['name'])
    t.check('ten-second respawn leaves old body independent',all(abs(before[k]-after[k])<.1 for k in ['x','y','z','bodyZ']) and after['food']==480)
    # Reverse the kill direction using an actual client attack against a low-health host.
    place(1,0,450,yaw=180);h.command('ai',paused=True);time.sleep(1)
    c.command('face',yaw=180)
    assert wait_for(lambda:abs(abs(h.actor(1)['yaw'])-180)<2 and abs(abs(c.state()['yaw'])-180)<2)
    place(0,0,0,hp=.005)
    old={b['name'] for b in c.state()['corpses']};c.tap('LeftMouseButton')
    t.check('real client attack kills host and publishes carcass',wait_for(lambda:h.state()['dead'] and any(b['source']==0 and b['name'] not in old for b in c.state()['corpses'])))
    place(2,2,1800);old={b['name'] for b in c.state()['corpses']};h.command('scoreHit',attacker=1,victim=2,value=99999)
    t.check('AI death publishes independent edible body online',wait_for(lambda:any(b['source']==2 and b['name'] not in old and b['food']==480 for b in c.state()['corpses'])))
    c.quit();h.quit()
finally:t.close()
