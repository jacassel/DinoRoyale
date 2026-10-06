"""Run actual rendered game controls/physics for one new animal before the next integration."""
import argparse,time,json,sys
from net_harness import NetworkTest,Peer,wait_for,distance
p=argparse.ArgumentParser();p.add_argument('--species',type=int,default=4);p.add_argument('--executable');p.add_argument('--output',default='Tests/Results/roster05/anky-first');a=p.parse_args()
t=NetworkTest(a.output,executable=a.executable,rendered=True);t.render_size=(1280,720)
try:
    h=Peer(t,'Roster05', '/Game/Maps/LostValley')
    h.command('ai',paused=True);h.command('sandbox',enabled=True);h.command('species',value=a.species);h.command('face',yaw=0)
    time.sleep(1);s=h.state();t.check('species loads with health and stamina',s['species']==a.species and s['health']==s['maxHealth'] and s['maxStamina']>0,species=s['species'],health=s['health'])
    for key,axis,sign in [('W','x',1),('S','x',-1),('A','y',-1),('D','y',1)]:
        h.command('face',yaw=0);b=h.state();e=h.hold(key,1);t.check(key+' mapped movement',(e[axis]-b[axis])*sign>100,travel=e[axis]-b[axis])
    h.command('face',yaw=0);b=h.state();h.key('W');h.key('LeftShift');time.sleep(1);e=h.state();h.key('W','up');h.key('LeftShift','up');t.check('sprint spends stamina and reveals',e['sprinting'] and e['stamina']<b['stamina'] and e['revealUntil']>b['revealUntil'])
    time.sleep(1);h.key('LeftControl');b=h.state();e=h.hold('W',.5);h.key('LeftControl','up');t.check('brace stays planted',e['brace'] and distance(b,e)<5)
    h.command('teleport',x=0,y=0);h.command('face',yaw=0);time.sleep(.7)
    for heavy in [False,True]:
        h.command('stamina',value=160)
        x,y=((-440,250) if a.species==4 else ((335,0) if heavy else (320,120)) if a.species==5 else (340,0))
        h.command('testAI',id=1,species=0,x=x,y=y,yaw=180,health=1,enabled=False)
        time.sleep(.4);before=h.actor(1)['health'];hits=h.state()['hits']
        if heavy:h.hold('RightMouseButton',1.8)
        else:h.tap('LeftMouseButton')
        time.sleep(1);e=h.state();after=h.actor(1)['health']
        t.check(('heavy' if heavy else 'quick')+' anatomical strike hits',after<before and e['hits']>hits,before=before,after=after,damage=e['damageDealt'])
        time.sleep(1.2);t.check('strike does not repeat after contact',h.actor(1)['health']>=after-.1)
    h.command('testAI',id=1,species=0,x=5000,y=0,yaw=180,health=1,enabled=False)
    h.command('hunger',value=35);h.command('food');time.sleep(.5);h.hold('F',2)
    t.check('herbivore restores hunger from plants',h.state()['hunger']>35,hunger=h.state()['hunger'])
    h.command('screenshot');time.sleep(.5)
    h.command('damage',value=99999);t.check('death produces carcass',h.state()['dead'] and any(x['species']==a.species for x in h.state()['corpses']))
    t.check('ten-second respawn',wait_for(lambda:not h.state()['dead'],12))
    t.check('carcass persists after respawn',any(x['species']==a.species for x in h.state()['corpses']))
finally:t.close()
