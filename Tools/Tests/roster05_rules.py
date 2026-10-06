"""Actual input/authority checks for 0.5 diets, packs, leaderboard and assist victory."""
import argparse,time
from net_harness import NetworkTest,Peer,wait_for
p=argparse.ArgumentParser();p.add_argument('--executable');p.add_argument('--output',default='Tests/Results/roster05/rules');a=p.parse_args()
t=NetworkTest(a.output,executable=a.executable,rendered=True);t.render_size=(1280,720)
try:
 h=Peer(t,'RosterRules','/Game/Maps/LostValley');h.command('ai',paused=True)
 def mode(teams=False,species=0):
  h.command('species',value=species);h.command('match',teams=teams);h.command('ai',paused=True);h.command('sandbox',enabled=False);time.sleep(.3)
 def score(i):return next(x for x in h.state()['scoreboard'] if x['id']==i)
 def hit(attacker,victim,amount=100000):
  h.command('scoreHit',attacker=attacker,victim=victim,value=amount);time.sleep(.3)
 def reset(i):h.command('resetCombatant',id=i)
 for teams in [False,True]:
  for species in [0,1,2,4,5,6]:
   mode(teams,species);s=h.state();roster=[v['species'] for v in s['ai'] if v['major']]+[species]
   t.check(f'all six species represented for player={species} teams={teams}',len(roster)==10 and set(roster)=={0,1,2,4,5,6} and roster.count(1)==3 and roster.count(6)==3)
 for teams in [False,True]:
  for species in [0,1,6]:
   mode(teams,species);s=h.state();allies=[x for x in s['ai'] if x['major'] and x['species']==species and (not teams or x['team']==0)]
   if species in [1,6]:
    t.check(f'{species} player pack has exactly two followers teams={teams}',len(allies)==2 and all(not x['scoringTarget'] for x in allies),members=[x['id'] for x in allies])
   for sp in [1,6]:
    group=[x for x in s['ai'] if x['major'] and x['species']==sp]
    if sp==species:continue
    t.check(f'{sp} AI pack one leader two followers teams={teams} player={species}',len(group)==3 and sum(x['scoringTarget'] for x in group)==1)
 mode(False,6);t.check('Pachy followers excluded from leaderboard',7 not in h.state()['leaderboardIDs'] and 8 not in h.state()['leaderboardIDs'] and 0 in h.state()['leaderboardIDs'])
 hit(7,1);t.check('Pachy follower kill credits player leader',score(0)['kills']==1 and score(7)['kills']==0)
 hit(1,8);t.check('Pachy follower death awards no opponent point',score(1)['kills']==0)
 mode();h.tap('P');t.check('P opens live leaderboard',h.state()['leaderboardOpen']);b=h.state();time.sleep(.5);t.check('leaderboard does not pause match',h.state()['time']>b['time']);h.tap('P');t.check('P closes leaderboard',not h.state()['leaderboardOpen'])
 for _ in range(4):reset(3);hit(0,3)
 t.check('four actual kills below FFA threshold',score(0)['points']==4 and not h.state()['roundOver'])
 for killer in [1,2]:reset(3);hit(0,3,40);hit(killer,3)
 t.check('two assists add one FFA point and can win',score(0)['kills']==4 and score(0)['assists']==2 and score(0)['points']==5 and h.state()['roundOver'] and h.state()['winner']==0)
 t.check('points leader sorted first',h.state()['leaderboardIDs'][0]==0);h.command('screenshot')
 mode(True)
 for _ in range(7):reset(5);hit(0,5)
 reset(5);hit(0,5,40);hit(4,5,40);hit(1,5)
 t.check('two pooled assists carry remainder without point',h.state()['team0Assists']==2 and h.state()['team0Score']==8 and not h.state()['roundOver'])
 reset(5);hit(0,5,40);hit(4,5)
 t.check('three assists pooled across players win at ten team points',h.state()['team0Kills']==9 and h.state()['team0Assists']==3 and h.state()['team0Score']==10 and h.state()['winnerTeam']==0 and h.state()['roundOver'])
 mode(True);h.tap('P');t.check('FFA leaderboard hidden in team mode',not h.state()['leaderboardOpen'])
 t.check('new round resets pooled assists',h.state()['team0Assists']==0 and h.state()['team0Score']==0)
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
