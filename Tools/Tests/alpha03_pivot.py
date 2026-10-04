"""Mapped controls and predicted pivot/grounding checks with two actual game processes."""
import argparse,time,json
from net_harness import NetworkTest,Peer,host_url,wait_for,distance
p=argparse.ArgumentParser();p.add_argument('--output',default='Tests/Results/alpha03/pivot');p.add_argument('--executable');p.add_argument('--lag',type=int,default=0);a=p.parse_args()
t=NetworkTest(a.output,executable=a.executable,lag=a.lag)
def delta(a,b):return (b-a+180)%360-180
def settle():time.sleep(.6+a.lag/500)
try:
    h=Peer(t,'PivotHost',host_url(3,bots=True));c=Peer(t,'PivotClient')
    c.lobby(2,1);assert wait_for(lambda:h.player(1)['ready']);h.lobby(6);assert wait_for(lambda:not c.state()['lobby']);h.command('ai',paused=True);h.command('sandbox',enabled=True)
    for sp in range(3):
        h.command('testAI',id=1,species=sp,x=1000,y=0,yaw=0,health=1,enabled=False)
        h.command('testAI',id=2,species=sp,x=3500,y=0,yaw=180,health=1,enabled=False)
        c.command('face',yaw=0);settle();h.command('ai',paused=True);settle()
        for k,sign in [('Q',-1),('E',1)]:
            before=c.state();c.key(k);time.sleep(.5);during=c.state();c.key(k,'up');settle();after=c.state()
            turn=delta(before['yaw'],during['yaw'])
            t.check(f'{sp} {k} turns in place',turn*sign>20 and distance(before,after)<5,turn=turn,travel=distance(before,after))
            t.check(f'{sp} {k} leaves camera independent',abs(delta(before['cameraYaw'],after['cameraYaw']))<.1)
            t.check(f'{sp} {k} retains released facing',abs(delta(during['yaw'],after['yaw']))<25)
            t.check(f'{sp} {k} uses replicated stepping clip',during['animation']==('PivotLeft' if k=='Q' else 'PivotRight'),animation=during['animation'])
            t.check(f'{sp} {k} server agrees with client yaw',abs(delta(h.actor(1)['yaw'],after['yaw']))<8,host=h.actor(1)['yaw'],client=after['yaw'])
        c.key('Q');time.sleep(.3);c.key('W');time.sleep(.65);moving=c.state();c.key('W','up');c.key('Q','up')
        t.check(f'{sp} W exits held pivot',moving['pivot']==0 and moving['speed']>100 and abs(delta(moving['cameraYaw'],moving['yaw']))<15)
        settle();c.key('LeftControl');settle();before=c.state();c.key('Q');c.hold('W',.3);after=c.state();c.key('Q','up');c.key('LeftControl','up')
        t.check(f'{sp} Ctrl brace blocks movement and pivot',after['brace'] and distance(before,after)<5 and abs(delta(before['yaw'],after['yaw']))<2)
        settle();c.key('RightMouseButton');time.sleep(.6);c.key('RightMouseButton','up');time.sleep(.12);before=c.state();c.key('E');time.sleep(.28);after=c.state();c.key('E','up')
        t.check(f'{sp} heavy pivot remains committed',abs(delta(before['yaw'],after['yaw']))<20,turn=delta(before['yaw'],after['yaw']))
        time.sleep(2)
        # Both observing directions, and remotely observed AI, after smoothing settles.
        for view,peer,ids in [('host',h,[1,2]),('client',c,[0,2])]:
            for ident in ids:
                d=peer.actor(ident)
                t.check(f'{sp} {view} observes grounded actor {ident}',abs(d['meshBaseZ']+d['halfHeight'])<.1 and abs(d['meshRelativeZ']+d['halfHeight'])<6,base=d['meshBaseZ'],relative=d['meshRelativeZ'],half=d['halfHeight'])
        h.command('scoreHit',attacker=0,victim=1,value=99999);time.sleep(2)
        for view,peer in [('host',h),('client',c)]:
            corpse=next(x for x in peer.state()['corpses'] if x['source']==1 and x['species']==sp)
            t.check(f'{sp} {view} corpse uses anatomical offset',abs(corpse['bodyRelativeZ']+h.actor(1)['halfHeight'])<.1 and abs(corpse['bodyZ']-corpse['ground'])<6,corpse=corpse)
        t.check(f'{sp} ten-second respawn returns',wait_for(lambda:not c.state()['dead'],12))
        t.check(f'{sp} carcass survives respawn',any(x['source']==1 and x['species']==sp for x in c.state()['corpses']))
    # Camera inputs remain mapped after pivoting.
    before=c.state();c.key('MouseX','axis');settle();t.check('mouse still controls camera',abs(delta(before['cameraYaw'],c.state()['cameraYaw']))>.1)
finally:t.close()
