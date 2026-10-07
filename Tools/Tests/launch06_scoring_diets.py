"""Actual input/authority checks for 0.5 diets, packs, leaderboard and assist victory."""
import argparse,time
from net_harness import NetworkTest,Peer,wait_for
p=argparse.ArgumentParser();p.add_argument('--executable');p.add_argument('--output',default='Tests/Results/launch06/scoring-diets');a=p.parse_args()
t=NetworkTest(a.output,executable=a.executable,rendered=True);t.render_size=(1280,720)
try:
 h=Peer(t,'LaunchScoring','/Game/Maps/LostValley');h.command('ai',paused=True)
 def mode(teams=False,species=0):
  h.command('species',value=species);h.command('match',teams=teams);h.command('ai',paused=True);h.command('sandbox',enabled=False);time.sleep(.3)
 def score(i):return next(x for x in h.state()['scoreboard'] if x['id']==i)
 def hit(attacker,victim,amount=100000):
  h.command('scoreHit',attacker=attacker,victim=victim,value=amount);time.sleep(.3)
 def reset(i):h.command('resetCombatant',id=i)
 # Actual score events, including assist-triggered victories.
 mode(False,7);h.tap('P');t.check('P opens live FFA leaderboard',h.state()['leaderboardOpen']);h.tap('P')
 for _ in range(4):reset(3);hit(0,3)
 t.check('four kills below unchanged five-point FFA threshold',score(0)['points']==4 and not h.state()['roundOver'])
 for killer in [1,2]:reset(3);hit(0,3,40);hit(killer,3)
 t.check('two assists add one point and can win FFA',score(0)['kills']==4 and score(0)['assists']==2 and h.state()['winner']==0 and h.state()['roundOver'])
 t.check('combined-points leader sorted first',h.state()['leaderboardIDs'][0]==0)
 mode(True,7)
 leaders=[v for v in h.state()['networkActors'] if not v['follower'] and v['species']!=3]
 allies=[v['id'] for v in leaders if v['team']==0 and v['id']!=0];enemy=next(v['id'] for v in leaders if v['team']==1)
 t.check('default team battle is five versus five leaders',sum(v['team']==0 for v in leaders)==5 and sum(v['team']==1 for v in leaders)==5)
 h.command('target',team=0);time.sleep(.6);hp=h.state()['targetHealth'];h.tap('LeftMouseButton');time.sleep(.6)
 t.check('Alberto cannot melee a teammate',h.state()['targetHealth']==hp);h.command('removeTarget')
 for _ in range(7):reset(enemy);hit(0,enemy)
 reset(enemy);hit(0,enemy,40);hit(allies[0],enemy,40);hit(allies[1],enemy)
 t.check('two pooled assists preserve remainder below goal',h.state()['team0Assists']==2 and h.state()['team0Score']==8 and not h.state()['roundOver'])
 reset(enemy);hit(0,enemy,40);hit(allies[0],enemy)
 t.check('third pooled assist wins at ten team points',h.state()['team0Kills']==9 and h.state()['team0Assists']==3 and h.state()['team0Score']==10 and h.state()['winnerTeam']==0)
 mode(True,7);hit(0,100);t.check('environmental prey does not score',h.state()['team0Kills']==0)
 h.tap('P');t.check('FFA leaderboard hidden in teams',not h.state()['leaderboardOpen'])
 mode(False,5);h.command('sandbox',enabled=True)
 for performance in [False,True]:
  h.command('mapVariant',performance=performance);time.sleep(.5);s=h.state();trees=[x for x in s['plants'] if x['tree']];shrubs=[x for x in s['plants'] if not x['tree']]
  t.check(f'18 edible trees remain performance={performance}',len(trees)==18 and all(x['foliageVisible'] for x in trees))
  t.check(f'Brachi highlights trees only performance={performance}',all(x['outline'] for x in trees) and not any(x['outline'] for x in shrubs))
  h.command('teleport',x=trees[0]['x']-500,y=trees[0]['y']);h.command('face',yaw=0);h.command('hunger',value=30);time.sleep(1);h.hold('F',2)
  t.check(f'Brachi browses world tree performance={performance}',h.state()['hunger']>50,hunger=h.state()['hunger']);h.command('screenshot')
 for species in [2,4,6]:
  h.command('species',value=species);h.command('clearTestFood');h.command('food');h.command('hunger',value=30);time.sleep(.5);s=h.state()
  t.check(f'{species} highlights shrubs only',all(x['outline']==(not x['tree'] and x['food']>0) for x in s['plants']))
  h.hold('F',1.5);t.check(f'{species} eats shared shrubs',h.state()['hunger']>45)
 h.command('species',value=5);h.command('hunger',value=20);time.sleep(.6);before=h.state()['hunger'];h.hold('F',1)
 t.check('Brachi rejects nearby shrub',h.state()['hunger']<=before)
 h.command('clearTestFood');h.command('face',yaw=0);h.command('food');time.sleep(.5);tree=min((x for x in h.state()['plants'] if x['tree']),key=lambda x:x['x']**2+x['y']**2)
 for _ in range(8):h.command('hunger',value=1);h.hold('F',1.6)
 depleted=next(x for x in h.state()['plants'] if x['name']==tree['name'])
 t.check('consumed tree retains trunk but hides edible canopy',depleted['food']==0 and not depleted['hidden'] and not depleted['foliageVisible'] and not depleted['outline'],food=depleted['food'])
 h.command('species',value=6);h.command('hunger',value=20);time.sleep(.6);before=h.state()['hunger'];h.hold('F',1);t.check('Pachy rejects trees',h.state()['hunger']<=before)
 h.command('mapVariant',performance=False)
finally:t.close()
