"""Real offline and separate-process host/guest tests for launch match configuration."""
import argparse,time
from net_harness import NetworkTest,Peer,host_url,wait_for
p=argparse.ArgumentParser();p.add_argument('--executable');p.add_argument('--output',default='Tests/Results/launch06/rules-first');a=p.parse_args()
t=NetworkTest(a.output,executable=a.executable)
def leaders(peer):return [d for d in peer.state()['networkActors'] if d['species']!=3 and not d['follower']]
def bots(peer):return [d for d in leaders(peer) if not d['player']]
try:
 h=Peer(t,'LaunchOffline','/Game/Maps/LostValley');h.command('ai',paused=True);h.command('sandbox',enabled=True)
 t.check('offline varied allowed roster in ten main slots',len(leaders(h))==10 and len({d['species'] for d in leaders(h)})>=3 and all(d['species'] in [0,1,2,4,5,6,7] for d in leaders(h)),actors=leaders(h))
 for species in [0,1,2,4,5,6,7]:
  h.command('species',value=species);t.check(f'offline species {species} selects',h.state()['species']==species)
 h.lobby(14,7);h.command('species',value=7);t.check('offline human cannot select banned Alberto',h.state()['species']!=7)
 t.check('banned Alberto removed from AI roster',not any(d['species']==7 for d in leaders(h)))
 h.lobby(14,1);t.check('raptor ban removes leader and followers',not any(d['species']==1 for d in h.state()['networkActors']))
 for species in [0,2,4,5]:h.lobby(14,species)
 t.check('only one species allowed',h.state()['allowedSpeciesMask']==64 and all(d['species']==6 for d in leaders(h)))
 h.lobby(14,6);t.check('last allowed species cannot be banned',h.state()['allowedSpeciesMask']==64)
 h.command('damage',value=99999);t.check('respawn respects only allowed species',wait_for(lambda:not h.state()['dead'],12) and h.state()['species']==6)
 for species in [0,1,2,4,5,7]:h.lobby(14,species)
 h.lobby(3,1)
 for aa,bb in [(4,5),(0,3),(0,5),(1,5),(2,0),(0,0),(2,6),(9,9)]:
  h.lobby(15,aa);h.lobby(16,bb);s=h.state();valid=bb>0 and 1+aa+bb<=10
  t.check(f'offline {1+aa} vs {bb} validity',bool(s['setupWarning'])!=valid,warning=s['setupWarning'])
  if valid:t.check(f'offline requested bots {aa}/{bb}',sum(d['team']==0 for d in bots(h))==aa and sum(d['team']==1 for d in bots(h))==bb)
 h.quit()
 h=Peer(t,'LaunchHost',host_url(10,teams=True));g=Peer(t,'LaunchGuest')
 h.lobby(1,0);g.lobby(1,1);h.lobby(15,0);h.lobby(16,0)
 t.check('zero bots valid with one human on each team',wait_for(lambda:len(leaders(g))==2 and not g.state()['setupWarning']))
 for goal in [5,10,15]:
  h.lobby(13,goal);t.check(f'host score {goal} replicates',wait_for(lambda:g.state()['goal']==goal))
  g.lobby(13,5 if goal!=5 else 15);t.check('guest cannot change score limit',h.state()['goal']==goal)
  g.lobby(2,1);h.lobby(6);t.check('match starts',wait_for(lambda:not g.state()['lobby']))
  h.command('ai',paused=True);h.command('sandbox',enabled=False)
  for i in range(goal):
   h.command('resetCombatant',id=1);h.command('scoreHit',attacker=0,victim=1,value=100000)
   if i<goal-1:t.check(f'{goal}-point match not over at {i+1}',not h.state()['roundOver'])
  t.check(f'match ends at exactly {goal} for guest',wait_for(lambda:g.state()['roundOver'] and g.state()['team0Score']==goal))
  h.lobby(8);t.check('rematch preserves target',wait_for(lambda:not g.state()['roundOver']) and g.state()['goal']==goal)
  h.lobby(7)
 h.lobby(14,7);g.lobby(0,7);t.check('guest cannot choose banned Alberto',wait_for(lambda:g.state()['allowedSpeciesMask']==119) and g.state()['species']!=7)
 g.lobby(14,7);g.lobby(15,4);t.check('guest cannot change bans or bots',h.state()['allowedSpeciesMask']==119 and h.state()['teamABots']==0)
 h.lobby(15,1);h.lobby(16,5);g.lobby(1,0)
 t.check('two humans plus one bot vs five bots',wait_for(lambda:len(leaders(g))==8 and sum(d['team']==0 for d in leaders(g))==3 and sum(d['team']==1 for d in leaders(g))==5))
 for aa,bb in [(0,3),(3,0),(2,5),(0,0)]:
  g.lobby(1,1);h.lobby(15,aa);h.lobby(16,bb)
  t.check(f'online bots {aa}/{bb} replicate',wait_for(lambda:sum(d['team']==0 for d in bots(g))==aa and sum(d['team']==1 for d in bots(g))==bb))
 h.lobby(15,4);h.lobby(16,5);g.lobby(2,1);h.lobby(6)
 t.check('over-capacity setup warns and blocks start without deleting humans',bool(h.state()['setupWarning']) and h.state()['lobby'] and len(h.state()['players'])==2)
 h.lobby(16,4);g.lobby(2,1);h.lobby(6);t.check('fixed capacity starts ten combatants',wait_for(lambda:not g.state()['lobby'] and len(leaders(g))==10))
 g.quit();h.quit()
finally:t.close()
