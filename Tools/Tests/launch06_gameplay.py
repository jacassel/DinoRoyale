"""Live mobility comparison, seventh-species ecology, pack scoring and setup guards."""
import argparse,time,json
from net_harness import NetworkTest,Peer,wait_for,distance
p=argparse.ArgumentParser();p.add_argument('--executable');p.add_argument('--output',default='Tests/Results/launch06/gameplay');a=p.parse_args()
t=NetworkTest(a.output,executable=a.executable)
try:
 h=Peer(t,'LaunchGameplay','/Game/Maps/LostValley')
 h.command('match',teams=False);h.command('menu',open=False);h.command('ai',paused=True);h.command('sandbox',enabled=True)
 measured={}
 for sp in [0,7]:
  h.command('species',value=sp);h.command('face',yaw=0);time.sleep(.7)
  b=h.state();h.key('W');time.sleep(.3);early=h.state();time.sleep(.9);s=h.state();h.key('W','up')
  h.command('species',value=sp);h.command('face',yaw=0);time.sleep(.7);h.key('W');h.key('LeftShift');time.sleep(1.2);sprint=h.state();h.key('W','up');h.key('LeftShift','up')
  h.command('species',value=sp);h.command('face',yaw=0);time.sleep(.7);h.key('E');time.sleep(.5);pivot=h.state();h.key('E','up')
  measured[sp]=dict(health=b['maxHealth'],speed=s['speed'],earlySpeed=early['speed'],sprint=sprint['speed'],pivot=abs(pivot['yaw']),stamina=b['maxStamina'],sprintStamina=sprint['stamina'])
 (t.out/'mobility.json').write_text(json.dumps(measured,indent=2))
 r,b=measured[0],measured[7]
 t.check('Alberto retains large predator health below Rex',.88<=b['health']/r['health']<=.94,measurements=measured)
 t.check('Alberto visibly faster running and sprinting',b['speed']>r['speed']*1.08 and b['sprint']>r['sprint']*1.12)
 t.check('Alberto accelerates and pivots faster',b['earlySpeed']>r['earlySpeed']*1.15 and b['pivot']>r['pivot']*1.25)
 t.check('Alberto has modestly better stamina economy',b['stamina']>r['stamina'] and b['stamina']-b['sprintStamina']<r['stamina']-r['sprintStamina'])
 h.command('species',value=7);h.command('face',yaw=0);h.command('clearTestFood');time.sleep(.5)
 h.command('damage',value=828);time.sleep(5.2)
 for hunger,hfactor,sfactor in [(95,1.5,1.25),(75,1,1),(65,.5,.65),(35,0,.4),(15,0,0),(9,0,0)]:
  h.command('hunger',value=hunger);h.command('stamina',value=30);b=h.state();time.sleep(1.1);s=h.state()
  t.check(f'Alberto hunger {hunger} uses shared regen thresholds',abs(s['healthRegenFactor']-hfactor)<.01 and abs(s['staminaRegenFactor']-sfactor)<.01 and (s['health']<b['health'] if hunger<10 else True))
 h.command('food',species=7);time.sleep(.5);corpse=next(c for c in h.state()['corpses'] if c['species']==7)
 t.check('Alberto death creates sized edible carcass',corpse['maxFood']==320)
 h.command('hunger',value=9);h.command('stamina',value=0);b=h.state();h.hold('F',2);s=h.state()
 t.check('starving exhausted Alberto eats meat and recovers',s['hunger']>b['hunger']+10 and s['health']>b['health'] and s['stamina']>20)
 t.check('eating and environmental food kill award no points',s['kills']==0)
 h.command('species',value=2);h.command('clearTestFood');h.command('face',yaw=0);h.command('food');h.command('species',value=7);h.command('face',yaw=0);h.command('hunger',value=20);time.sleep(.6);b=h.state();h.hold('F',1.5)
 t.check('Alberto rejects plants',h.state()['hunger']<=b['hunger'] and not any(v['outline'] for v in h.state()['plants']))
 h.command('clearTestFood');h.command('species',value=7);h.command('face',yaw=0);h.key('W');h.key('LeftShift');time.sleep(.5);s=h.state();h.key('W','up');h.key('LeftShift','up')
 t.check('Alberto sprint creates map noise',s['sprinting'] and s['revealUntil']>s['time'])
 h.tap('LeftMouseButton');s=h.state();t.check('Alberto attack creates map noise',s['revealUntil']>s['time'])
 # Independent packs may share a species; only actual followers credit a leader.
 for sp in [1,6]:
  h.command('species',value=sp);h.command('match',teams=False);h.command('ai',paused=True)
  followers=[v for v in h.state()['networkActors'] if v['follower'] and v['pack']==0]
  t.check(f'{sp} player owns exactly two non-scoring followers',len(followers)==2 and all(v['id'] not in h.state()['leaderboardIDs'] for v in followers))
  victim=next(v for v in h.state()['networkActors'] if not v['follower'] and v['id'] in range(1,10))['id']
  h.command('scoreHit',attacker=followers[0]['id'],victim=victim,value=100000);time.sleep(.3)
  t.check(f'{sp} follower kill credits human leader',h.state()['kills']==1)
  h.command('resetCombatant',id=victim);h.command('scoreHit',attacker=victim,victim=followers[1]['id'],value=100000);time.sleep(.3)
  t.check(f'{sp} follower death awards no kill',next(v for v in h.state()['scoreboard'] if v['id']==victim)['kills']==0)
 h.command('menu',open=True);h.lobby(3,1);h.lobby(15,0);h.lobby(16,0);h.tap('Escape')
 t.check('Escape cannot resume a team with no opponent',h.state()['menuOpen'] and bool(h.state()['setupWarning']))
 h.lobby(16,3);h.tap('Escape');h.tap('Enter')
 t.check('valid revised local setup starts fresh three-bot challenge',not h.state()['menuOpen'] and h.state()['kills']==0 and sum(v['team']==1 and not v['follower'] for v in h.state()['networkActors'])==3)
finally:t.close()
