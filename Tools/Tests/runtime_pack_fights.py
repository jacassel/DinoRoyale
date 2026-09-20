"""Follow-up pack fights after the raptor survivability adjustment."""
import runtime_core as t,time,json,math,sys
reports=[]
def press(k):t.key(k);t.key(k,'up')
def release():
 for k in ['W','A','S','D','Q','LeftShift','RightMouseButton']:t.key(k,'up')
def place(i,sp,x,y=0,enabled=False,profile=3):t.command('testAI',id=i,species=sp,x=x,y=y,health=1,enabled=enabled,yaw=180,personality=profile)
for player,enemy,pack in [(1,0,True),(1,2,True),(0,1,False)]:
 t.command('menu',open=False);release();t.command('match',teams=False);t.command('ai',paused=True);t.command('sandbox',enabled=True);t.command('species',value=player);t.command('teleport',x=0,y=0);t.command('face',yaw=0)
 for i in range(1,10):place(i,(i-1)//3,-24000+i*400,-23000)
 place(1,enemy,1300,profile=3)
 if pack:
  place(2,1,-300,400,profile=0);place(3,1,-300,-400,profile=2)
 else:
  place(2,1,1700,550,profile=0);place(3,1,1700,-550,profile=2)
 # One setup point of damage identifies the aggressor; subsequent damage uses actual melee.
 t.command('scoreHit',attacker=0,victim=1,value=1)
 if pack:t.command('scoreHit',attacker=1,victim=0,value=1)
 for i in [1,2,3]:t.command('enableAI',id=i,enabled=True)
 start=t.state()['time'];base={a['id']:a for a in t.state()['ai']};snapshots=[];deadline=time.monotonic()+160
 while t.state()['time']-start<35 and time.monotonic()<deadline:
  s=t.state();ais=[a for a in s['ai'] if a['id'] in [1,2,3]];snapshots.append(s)
  enemies=[a for a in ais if not a['dead'] and (a['species']!=1 if pack else True)]
  if s['dead'] or not enemies:break
  e=min(enemies,key=lambda a:t.dist(s,a));d=t.dist(e,s);yaw=math.degrees(math.atan2(e['y']-s['y'],e['x']-s['x']));gap=abs((yaw-s['yaw']+180)%360-180);t.command('camera',yaw=yaw)
  reach=280 if player==1 else 620
  if s['charging']:
   if s['charge']>.95:t.key('RightMouseButton','up')
  elif s['recovery']<=0:
   if e['charging'] and d<900 and s['stamina']>20:t.key('W','up');t.key('Q')
   else:
    t.key('Q','up')
    if d>reach*.7 or gap>20:t.key('W');t.key('LeftShift','down' if d>1600 and s['stamina']>55 else 'up')
    else:
     t.key('W','up');t.key('LeftShift','up')
     if s['stamina']>40 and s['health']/s['maxHealth']>.25 and int(s['time']-start)%4<2:t.key('RightMouseButton')
     else:press('LeftMouseButton')
  time.sleep(.08)
 release();s=t.state();members=[a for a in s['ai'] if a['id'] in [1,2,3]]
 report=dict(player=player,enemy=enemy,humanLeadsPack=pack,seconds=s['time']-start,playerHP=s['health'],playerDead=s['dead'],ai=[dict(id=a['id'],species=a['species'],hp=a['health'],leader=a['leader'],hits=a['hits']-base[a['id']]['hits'],stuck=a['stuckRecoveries']-base[a['id']]['stuckRecoveries'],paths=a['failedPaths']-base[a['id']]['failedPaths']) for a in members],states=sorted({a['state'] for q in snapshots for a in q['ai'] if a['id'] in [1,2,3]}),staminaValid=all(0<=a['stamina']<=100.01 for q in snapshots for a in q['ai'] if a['id'] in [1,2,3]))
 reports.append(report);(t.OUT/'combat-pack-fights.json').write_text(json.dumps(reports,indent=2));print(json.dumps(report),flush=True)
t.command('ai',paused=True);t.command('menu',open=True)
