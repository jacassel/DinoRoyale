"""All new combo contacts, rear/front counterplay, misses and heavy recovery."""
import argparse,time
from net_harness import NetworkTest,Peer,wait_for
p=argparse.ArgumentParser();p.add_argument('--executable');p.add_argument('--output',default='Tests/Results/roster05/combos');a=p.parse_args()
t=NetworkTest(a.output,executable=a.executable,rendered=True)
try:
 h=Peer(t,'RosterCombos','/Game/Maps/LostValley');h.command('match',teams=False);h.command('ai',paused=True);h.command('sandbox',enabled=True)
 for species,positions in [(4,[(-440,250),(-440,-250),(-400,280)]),(5,[(320,120),(-660,320),(320,-120)]),(6,[(290,0)]*3),(7,[(360,0)]*3)]:
  h.command('species',value=species);h.command('teleport',x=0,y=0);h.command('face',yaw=0);time.sleep(.6)
  for combo,(x,y) in enumerate(positions,1):
   h.command('testAI',id=1,species=0,x=x,y=y,yaw=0,health=1,enabled=False);h.command('stamina',value=160);before=h.actor(1)['health'];h.tap('LeftMouseButton')
   t.check(f'{species} combo {combo} makes anatomical contact',wait_for(lambda:h.actor(1)['health']<before,1) and h.state()['combo']==combo,health=h.actor(1)['health'])
   # A late click intentionally queues the next strike in the last 0.18 seconds.
   # Check rejection during the long third-strike recovery, away from that buffer.
   if combo==3:
    serial=h.state()['attackSerial'];h.tap('LeftMouseButton');t.check(f'{species} third-strike commitment cannot be bypassed',h.state()['attackSerial']==serial)
   wait_for(lambda:h.state()['recovery']<=0,4)
  if species==5:
   for ident,y in [(1,160),(2,-160)]:h.command('testAI',id=ident,species=0,x=340,y=y,yaw=180,health=1,enabled=False)
   h.command('stamina',value=160);h.hold('RightMouseButton',2)
   t.check('Brachi heavy after combo three contacts both forefeet sides',wait_for(lambda:h.actor(1)['health']<1500 and h.actor(2)['health']<1500,2))
   wait_for(lambda:h.state()['recovery']<=0,5);h.command('testAI',id=2,species=0,x=6000,y=0,yaw=0,health=1,enabled=False)
  h.command('testAI',id=1,species=0,x=5000,y=0,yaw=0,health=1,enabled=False);time.sleep(1.2);before=h.state()['hits'];h.tap('LeftMouseButton');time.sleep(.8);t.check(f'{species} distant quick attack misses',h.state()['hits']==before);wait_for(lambda:h.state()['recovery']<=0,4)
  h.command('stamina',value=160);h.hold('RightMouseButton',2);time.sleep(.8);t.check(f'{species} distant heavy misses and leaves recovery',h.state()['hits']==before and h.state()['recovery']>0);wait_for(lambda:h.state()['recovery']<=0,5)
  h.command('teleport',x=0,y=0);h.command('face',yaw=0);time.sleep(1.1)
  if species==4:
   h.command('testAI',id=1,species=0,x=330,y=0,yaw=180,health=1,enabled=False);before=h.actor(1)['health'];h.tap('LeftMouseButton');time.sleep(.7);t.check('Anky tail attack does not hit a frontal enemy',h.actor(1)['health']==before)
finally:t.close()
