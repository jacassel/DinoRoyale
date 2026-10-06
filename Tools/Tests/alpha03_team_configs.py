"""Requested team configurations, custom assignment, map travel and late joining."""
import argparse,time
from net_harness import NetworkTest,Peer,host_url,wait_for
p=argparse.ArgumentParser();p.add_argument('--output',required=True);p.add_argument('--executable');a=p.parse_args()
t=NetworkTest(a.output,executable=a.executable)
def count(peer):
    actors=[x for x in peer.state()['networkActors'] if x['scoring']]
    return [sum(x['team']==team for x in actors) for team in [0,1]]
def check_round(label,expected):
    t.check(label+' roster agrees on both peers',wait_for(lambda:count(h)==count(c)==expected),expected=expected)
    c.lobby(2,1);assert wait_for(lambda:h.player(1)['ready']);h.lobby(6)
    t.check(label+' starts with correct counts and ten-kill goal',wait_for(lambda:not c.state()['lobby'] and count(c)==expected and c.state()['goal']==10))
    h.lobby(7);assert wait_for(lambda:c.state()['lobby'])
try:
    h=Peer(t,'TeamHost',host_url(10,teams=True,bots=True));c=Peer(t,'TeamClient')
    for performance in [False,True]:
        h.lobby(9,int(performance));assert wait_for(lambda:c.state()['performanceMap']==performance)
        suffix='Performance' if performance else 'Standard'
        h.lobby(12);c.lobby(1,1);assert wait_for(lambda:h.player(1)['team']==1)
        check_round(suffix+' normal 5 vs 5',[5,5])
        h.lobby(5,0);h.lobby(4,2);check_round(suffix+' one vs one',[1,1])
        h.lobby(4,6);h.lobby(5,1);check_round(suffix+' three vs three',[3,3])
        h.lobby(12);c.lobby(1,0);assert wait_for(lambda:h.player(1)['team']==0)
        check_round(suffix+' two humans plus three AI vs five AI',[5,5])
        allied=[x['id'] for x in h.state()['networkActors'] if x['bot'] and x['scoring'] and x['team']==0]
        for id in allied:h.lobby(10,id)
        check_round(suffix+' two humans vs five AI',[2,5])
        enemy=next(x['id'] for x in h.state()['networkActors'] if x['bot'] and x['scoring'] and x['team']==1)
        h.lobby(11,enemy)
        t.check(suffix+' explicit AI team selection survives reconciliation',wait_for(lambda:count(c)==[3,4] and c.actor(enemy)['team']==0))
        h.lobby(11,enemy);assert wait_for(lambda:count(c)==[2,5])
    c.quit();h.lobby(12)
    t.check('one human plus four AI vs five AI',wait_for(lambda:count(h)==[5,5] and sum(x['bot'] and x['scoring'] for x in h.state()['networkActors'])==9))
    c=Peer(t,'TeamLateClient')
    t.check('late join receives Performance variant with shrubs and browse trees',wait_for(lambda:c.state()['performanceMap'] and len(c.state()['plants'])==54 and sum(p['tree'] for p in c.state()['plants'])==18 and c.state()['treeInstances']==0))
    t.check('late human replaces exactly one bot',wait_for(lambda:sum(x['bot'] and x['scoring'] for x in c.state()['networkActors'])==8 and len(c.state()['players'])==2))
    c.quit();h.quit()
finally:t.close()
