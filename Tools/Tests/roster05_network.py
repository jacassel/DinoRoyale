"""Three real processes: all roster IDs, new anatomical strikes, diets and assist scoring.

Loopback correctness regression; previous owner-confirmed WAN play is separate evidence.
"""
import argparse,time
from net_harness import NetworkTest,Peer,host_url,wait_for,distance
p=argparse.ArgumentParser();p.add_argument('--executable');p.add_argument('--output',default='Tests/Results/roster05/network');p.add_argument('--lag',type=int,default=0);p.add_argument('--loss',type=int,default=0);a=p.parse_args()
t=NetworkTest(a.output,executable=a.executable,lag=a.lag,loss=a.loss)
try:
 host=Peer(t,'RosterHost',host_url(6));client=Peer(t,'RosterGuest');third=Peer(t,'RosterThird')
 for species in [0,1,2,4,5,6]:
  client.lobby(0,species);t.check(f'lobby species {species} replicates to three peers',wait_for(lambda:host.actor(1)['species']==client.state()['species']==third.actor(1)['species']==species))
 host.lobby(0,5);client.lobby(0,6);third.lobby(0,4);client.lobby(2,1);third.lobby(2,1);host.lobby(6)
 t.check('all three new species start together',wait_for(lambda:not third.state()['lobby']) and {v['species'] for v in third.state()['networkActors'] if v['player']}=={4,5,6})
 host.command('ai',paused=True);host.command('sandbox',enabled=True)
 t.check('remote Pachy leader owns exactly two followers',wait_for(lambda:len([v for v in third.state()['networkActors'] if v['follower'] and v['pack']==1 and v['species']==6])==2))
 for species in [4,5,6]:
  host.command('testAI',id=1,species=species,x=0,y=0,yaw=0,health=1,enabled=False);client.command('face',yaw=0);time.sleep(.7)
  x,y=(-440,250) if species==4 else (320,120) if species==5 else (340,0)
  host.command('testAI',id=0,species=0,x=x,y=y,yaw=180,health=1,enabled=False);time.sleep(.5)
  hp=host.state()['health'];client.tap('LeftMouseButton');t.check(f'{species} client anatomical quick hits on server',wait_for(lambda:host.state()['health']<hp));t.check(f'{species} damaged health replicates to observer',wait_for(lambda:abs(third.actor(0)['health']-host.state()['health'])<.1));time.sleep(2)
  host.command('testAI',id=1,species=species,x=0,y=0,yaw=0,health=1,stamina=160,enabled=False);client.command('face',yaw=0)
  if species==5:x,y=335,0
  host.command('testAI',id=0,species=0,x=x,y=y,yaw=180,health=1,enabled=False);time.sleep(.6);hp=host.state()['health'];client.hold('RightMouseButton',2)
  t.check(f'{species} client heavy hits authoritatively',wait_for(lambda:host.state()['health']<hp));time.sleep(2)
  host.command('testAI',id=1,species=species,x=3000,y=4500,yaw=0,health=1,enabled=False);client.command('face',yaw=0);time.sleep(3)
  t.check(f'{species} swimming replicates',client.state()['swimming'] and third.actor(1)['swimming']);b=client.state();client.hold('W',.8);t.check(f'{species} remote swim moves and converges',distance(b,client.state())>100 and wait_for(lambda:distance(host.actor(1),client.state())<150))
  host.command('testAI',id=1,species=species,x=0,y=0,yaw=0,health=.24,stamina=160,enabled=False);time.sleep(.6);client.hold('RightMouseButton',.4);t.check(f'{species} critical health rejects remote charge',not client.state()['charging'])
 # Browse a replicated tree through remote input on the sparse map.
 host.lobby(7);host.lobby(9,1);host.lobby(0,0);client.lobby(0,5);third.lobby(0,4);client.lobby(2,1);third.lobby(2,1);host.lobby(6);host.command('ai',paused=True)
 tree=next(v for v in host.state()['plants'] if v['tree']);host.command('testAI',id=1,species=5,x=tree['x']-500,y=tree['y'],yaw=0,health=.7,hunger=10,enabled=False);client.command('face',yaw=0);time.sleep(3.5)
 t.check('performance mode keeps remote browse trees and species-specific outlines',client.state()['performanceMap'] and any(v['tree'] and v['outline'] for v in client.state()['plants']) and not any(v['tree'] and v['outline'] for v in third.state()['plants']))
 before=client.state()['hunger'];client.hold('F',2);t.check('remote Brachi restores hunger from tree',client.state()['hunger']>before+15)
 t.check('browse nutrition replicates to all peers',wait_for(lambda:any(v['tree'] and distance(v,tree)<1 and v['food']<tree['food']-10 for v in third.state()['plants'])))
 # Two other humans each finish one assisted kill, allowing the assister to win FFA.
 host.lobby(7);host.lobby(3,0);host.lobby(0,0);client.lobby(0,0);third.lobby(0,0);client.lobby(2,1);third.lobby(2,1);host.lobby(6);host.command('ai',paused=True);host.command('sandbox',enabled=False)
 def hit(attacker,victim,value=100000):host.command('scoreHit',attacker=attacker,victim=victim,value=value);time.sleep(.3)
 for _ in range(4):host.command('resetCombatant',id=2);hit(0,2)
 for killer,victim in [(1,2),(2,1)]:
  host.command('resetCombatant',id=killer);host.command('resetCombatant',id=victim);hit(0,victim,40);hit(killer,victim)
 t.check('assist-triggered FFA winner replicates',wait_for(lambda:third.state()['roundOver'] and third.state()['winner']==0 and client.state()['winner']==0))
 t.check('remote leaderboard orders combined points',third.state()['leaderboardIDs'][0]==0)
 host.lobby(7);host.lobby(3,1);host.lobby(1,0);client.lobby(1,0);third.lobby(1,1);client.lobby(2,1);third.lobby(2,1);host.lobby(6);host.command('ai',paused=True);host.command('sandbox',enabled=True)
 for attacker,killer in [(0,1),(1,0),(0,1)]:host.command('resetCombatant',id=2);hit(attacker,2,40);hit(killer,2)
 t.check('three pooled assists add team point on clients',wait_for(lambda:client.state()['team0Assists']==3 and third.state()['team0Score']==4 and third.state()['team0Kills']==3))
 third.quit();client.quit();host.quit()
finally:t.close()
