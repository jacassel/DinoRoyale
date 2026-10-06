"""Wait for the real 120-second browse-tree regrowth timer, without accelerating time."""
import argparse,time
from net_harness import NetworkTest,Peer,wait_for,distance
p=argparse.ArgumentParser();p.add_argument('--executable');p.add_argument('--output',default='Tests/Results/roster05/tree-regrowth');a=p.parse_args()
t=NetworkTest(a.output,executable=a.executable)
try:
 h=Peer(t,'TreeRegrowth','/Game/Maps/LostValley');h.command('ai',paused=True);h.command('species',value=5);h.command('mapVariant',performance=True)
 tree=next(x for x in h.state()['plants'] if x['tree']);h.command('teleport',x=tree['x']-500,y=tree['y']);h.command('face',yaw=0);time.sleep(1)
 def tree_now():return next(x for x in h.state()['plants'] if x['name']==tree['name'])
 for _ in range(8):h.command('hunger',value=1);h.hold('F',1.6)
 depleted=h.state()['time'];t.check('real browse tree fully consumed',tree_now()['food']==0 and not tree_now()['foliageVisible'] and not tree_now()['hidden'])
 deadline=time.monotonic()+145
 while h.state()['time']-depleted<100 and time.monotonic()<deadline:time.sleep(.5)
 t.check('tree remains depleted before two-minute regrowth',tree_now()['food']==0 and not tree_now()['outline'])
 t.check('tree regrows leaves nutrition and outline after timer',wait_for(lambda:tree_now()['food']>0,28) and tree_now()['foliageVisible'] and tree_now()['outline'],seconds=h.state()['time']-depleted)
 h.command('hunger',value=20);before=tree_now()['food'];h.hold('F',1);t.check('regrown tree can be browsed again',h.state()['hunger']>30 and tree_now()['food']<before)
finally:t.close()
