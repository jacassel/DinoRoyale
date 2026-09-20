"""Twelve live player-vs-AI combat encounters; no substituted combat/damage routines."""
import runtime_core as t,time,json,math,sys
rows=[]
def wait(sec):
 start=t.state()['time']
 while t.state()['time']-start<sec:time.sleep(.03)
def place(id,species,x,y=0,profile=3):
 return t.command('testAI',id=id,species=species,x=x,y=y,health=1,enabled=False,yaw=180,personality=profile)
def release():
 for k in ['W','S','A','D','LeftShift','Q','RightMouseButton','LeftMouseButton','SpaceBar']:t.key(k,'up')
def foe():return next(a for a in t.state()['ai'] if a['id']==1)
t.command('menu',open=False);t.command('sandbox',enabled=True);t.command('match',teams=False);t.command('invulnerable',value=False)
for species in range(3):
 for profile in range(4):
  release();t.command('ai',paused=True);t.command('removeTarget');t.command('species',value=species);t.command('teleport',x=0,y=0);t.command('face',yaw=0)
  for i in range(2,10):place(i,(i-1)//3,-24000+i*450,-23000)
  enemy=([0,1,2,0] if species==0 else [2,0,2,0] if species==1 else [1,0,1,0])[profile]
  place(1,enemy,1400,profile=profile);wait(.5);a=t.state();start=a['time'];base=foe();t.command('enableAI',id=1,enabled=True)
  samples=[];last_action='';heavy_started=None;deadline=time.monotonic()+180
  while t.state()['time']-start<24 and time.monotonic()<deadline:
   s=t.state();e=foe();samples.append(dict(time=s['time']-start,hp=s['health'],st=s['stamina'],combo=s['combo'],aihp=e['health'],aist=e['stamina'],aicombo=e['combo'],state=e['state'],hits=e['hits'],charging=e['charging'],sprint=e['sprinting'],exhausted=e['exhausted']))
   if s['dead'] or e['dead']:break
   yaw=math.degrees(math.atan2(e['y']-s['y'],e['x']-s['x']));t.command('camera',yaw=yaw);distance=t.dist(s,e)
   reach=[620,280,550][species]
   if s['charging']:
    if s['charge']>=.95:t.key('RightMouseButton','up')
   elif s['recovery']<=0:
    if e['charging'] and distance<900 and s['stamina']>20:
     t.key('W','up');t.key('LeftShift','up');t.key('Q');last_action='guard'
    else:
     t.key('Q','up')
     if distance>reach*.72:
      t.key('W');t.key('LeftShift','down' if distance>1400 and s['stamina']>60 else 'up')
     else:
      t.key('W','up');t.key('LeftShift','up')
      if s['stamina']>45 and not s['exhausted'] and s['health']/s['maxHealth']>.25 and (int(s['time']-start)%5<2 or e['exhausted']):t.key('RightMouseButton')
      else:t.key('LeftMouseButton');t.key('LeftMouseButton','up')
   time.sleep(.09)
  release();s=t.state();e=foe();t.command('enableAI',id=1,enabled=False)
  row=dict(player=species,enemy=enemy,profile=profile,seconds=round(s['time']-start,2),playerHP=s['health'],enemyHP=e['health'],playerDead=s['dead'],enemyDead=e['dead'],playerHits=s['hits']-a['hits'],aiHits=e['hits']-base['hits'],aiGuards=e['guards']-base['guards'],aiRetreats=e['retreats']-base['retreats'],aiStates=sorted({x['state'] for x in samples}),sprinting=any(x['sprint'] for x in samples),aiHeavy=any(x['charging'] for x in samples),staminaMinimum=min(x['aist'] for x in samples),stuck=e['stuckRecoveries']-base['stuckRecoveries'],failedPaths=e['failedPaths']-base['failedPaths'],rulesValid=all(0<=x['aist']<=100.01 and x['aicombo']<=3 and 0<=x['st']<=100.01 for x in samples),samples=samples)
  rows.append(row);(t.OUT/'combat-personalities.json').write_text(json.dumps(rows,indent=2));print(json.dumps({k:v for k,v in row.items() if k!='samples'}),flush=True)
t.command('ai',paused=True);t.command('menu',open=True);sys.exit(any(not x['rulesValid'] or x['failedPaths'] for x in rows))
