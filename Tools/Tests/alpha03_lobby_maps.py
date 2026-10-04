"""Authoritative unequal bot rosters and replicated map variants in two game processes."""
import argparse,time
from net_harness import NetworkTest,Peer,host_url,wait_for
p=argparse.ArgumentParser();p.add_argument('--output',default='Tests/Results/alpha03/lobby-maps');p.add_argument('--executable');a=p.parse_args()
t=NetworkTest(a.output,executable=a.executable)
try:
    h=Peer(t,'SetupHost',host_url(10,teams=True,bots=True));c=Peer(t,'SetupClient');assert wait_for(lambda:len(c.state()['players'])==2)
    c.lobby(1,0);assert wait_for(lambda:h.player(1)['team']==0)
    h.lobby(12);time.sleep(.5)
    t.check('5v5 preset with both humans on Team A',sum(x['team']==0 for x in h.state()['networkActors'] if x['scoring'])==5 and sum(x['team']==1 for x in h.state()['networkActors'] if x['scoring'])==5)
    # Disable 3 allied AI: exactly two humans versus five AI; three empty slots.
    allied_slots=[x['id'] for x in h.state()['networkActors'] if x['bot'] and x['scoring'] and x['team']==0]
    victim=next(x['id'] for x in h.state()['networkActors'] if x['bot'] and x['scoring'] and x['team']==1)
    for slot in allied_slots:h.lobby(10,slot)
    assert wait_for(lambda:len([x for x in c.state()['networkActors'] if x['scoring']])==7)
    s=h.state();t.check('two humans vs five AI with three empty slots',sum(x['bot'] and x['scoring'] for x in s['networkActors'])==5 and all(x['team']==1 for x in s['networkActors'] if x['bot'] and x['scoring']))
    old=list(s['botSlots']);c.lobby(10,5);time.sleep(.3);t.check('guest cannot edit host AI slots',h.state()['botSlots']==old)
    plants=sorted((p['x'],p['y']) for p in c.state()['plants'])
    h.lobby(9,1);t.check('performance variant reaches both peers',wait_for(lambda:c.state()['performanceMap'] and h.state()['performanceMap']))
    t.check('performance has no decorative foliage',all(c.state()[k]==0 for k in ['treeInstances','grassInstances','fernInstances']))
    t.check('performance retains 36 feeding plants',len(c.state()['plants'])==36 and sorted((p['x'],p['y']) for p in c.state()['plants'])==plants)
    c.lobby(9,0);time.sleep(.3);t.check('guest cannot change map',h.state()['performanceMap'])
    c.lobby(2,1);assert wait_for(lambda:h.player(1)['ready']);h.lobby(6);assert wait_for(lambda:not c.state()['lobby'])
    t.check('unequal team goal stays ten',c.state()['goal']==10)
    h.command('scoreHit',attacker=0,victim=victim,value=99999)
    t.check('unequal team kill scores replicate',wait_for(lambda:c.state()['team0Kills']==1))
    t.check('raptor followers stay outside slot scoring',all(not x['scoring'] for x in c.state()['networkActors'] if x['follower']))
    h.lobby(7);assert wait_for(lambda:c.state()['lobby']);h.lobby(9,0)
    t.check('standard restores trees and navigation obstacles on both peers',wait_for(lambda:not c.state()['performanceMap'] and c.state()['treeInstances']>400 and c.state()['navObstacles']>400))
    h.lobby(12);time.sleep(.4);t.check('preset recovers all ten participants',sum(x['scoring'] for x in h.state()['networkActors'])==10)
    # One human plus 4 AI versus 5 AI after guest departs.
    c.quit();t.check('departed human slot fills for normal 5v5',wait_for(lambda:sum(x['bot'] and x['scoring'] for x in h.state()['networkActors'])==9))
    h.quit()
finally:t.close()
