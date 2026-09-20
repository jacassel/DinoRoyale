"""Behavioral AI assertions in the live engine; setup alters health/positions only."""
import runtime_core as t,time,json,math,sys
rows=[]
def check(n,ok,**data):
    rows.append(dict(test=n,passed=bool(ok),**data));(t.OUT/'ai-tactics.json').write_text(json.dumps(rows,indent=2));print(('PASS ' if ok else 'FAIL ')+n+' '+json.dumps(data),flush=True)
def ai(i=1):return next(a for a in t.state()['ai'] if a['id']==i)
def place(i,species,x,y=0,health=1,enabled=False,yaw=180):return t.command('testAI',id=i,species=species,x=x,y=y,health=health,enabled=enabled,yaw=yaw)
def reset(player,foe,x,health=1):
    for k in ['W','Q','RightMouseButton','LeftMouseButton']:t.key(k,'up')
    t.command('species',value=player);t.command('match',teams=False);t.command('sandbox',enabled=True);t.command('ai',paused=True);t.command('invulnerable',value=False);t.command('teleport',x=0,y=0);t.command('face',yaw=0)
    for i in range(2,10):place(i,(i-1)//3,-24000+i*450,-23000)
    place(1,foe,x,health=health);time.sleep(.5)
def enable():t.command('enableAI',id=1,enabled=True)
def observe(seconds):
    samples=[];start=t.state()['time'];deadline=time.monotonic()+seconds*8+15
    while t.state()['time']-start<seconds and time.monotonic()<deadline:samples.append(ai());time.sleep(.08)
    return samples

reset(2,0,480);before=ai();t.command('scoreHit',attacker=0,victim=1,value=20);enable();samples=observe(3)
check('AI retaliates against attacker',any(s['target']==0 and 'Retaliate' in s['decision'] for s in samples) and ai()['retaliations']>before['retaliations'])
check('retaliation deals real melee damage',t.state()['health']<t.state()['maxHealth'],playerHealth=t.state()['health'],aiHits=ai()['hits']-before['hits'])

reset(0,1,1600);t.command('damage',value=t.state()['maxHealth']*.82);a=ai();enable();samples=observe(1.2);b=ai()
check('healthy raptor pursues weakened prey dinosaur',b['x']<a['x']-300 and any(s['state'] in ['Pursuing','Attacking','Charging'] for s in samples),travel=a['x']-b['x'],fight=b['fightConfidence'])
samples+=observe(2);check('pursuit results in attacks',ai()['hits']>a['hits'] or t.state()['dead'],hits=ai()['hits']-a['hits'])

reset(0,1,1000,health=.12);a=ai();enable();samples=observe(1.8);b=ai()
check('critical raptor chooses escape from healthy rex',any(s['state']=='Retreating' and s['escapeConfidence']>s['fightConfidence'] for s in samples),fight=b['fightConfidence'],escape=b['escapeConfidence'])
check('retreat physically increases separation',t.dist(b,t.state())>t.dist(a,dict(x=0,y=0))+500,distance=t.dist(b,t.state()))

# Present a fully charged threat, then release during the observed guard window.
# Holding another half-second after recognition can deliberately outlast the finite guard.
reset(0,2,550);t.command('invulnerable',value=True);a=ai();t.key('RightMouseButton');time.sleep(.9);enable();samples=[];guard_start=t.state()['time'];b=ai()
while b['state']!='Bracing' and t.state()['time']-guard_start<.6:
 samples+=observe(.04);b=ai()
check('triceratops recognizes and braces against charge',b['state']=='Bracing' and b['guards']>a['guards'])
t.key('RightMouseButton','up');time.sleep(.45);c=ai();damage=b['health']-c['health'];check('AI brace actually reduces frontal damage',0<damage<80,damage=damage)
samples=observe(3);check('AI releases guard and counterattacks',any(s['state'] in ['Attacking','Charging','Pursuing'] for s in samples) and ai()['hits']>c['hits'],states=sorted({s['state'] for s in samples}))

# With the retained 1650 HP / 155 damage balance, 18% health still favors fighting
# a lone raptor. Six percent sets up the intended desperate, poor-escape condition.
reset(1,2,300,health=.06);enable();samples=observe(1.2);b=ai();check('slow injured trike guards when escape is poor',any(s['state']=='Bracing' and s['fightConfidence']<.35 and s['escapeConfidence']<.38 for s in samples) and all(s['state']!='Retreating' for s in samples),fight=b['fightConfidence'],escape=b['escapeConfidence'])

reset(2,1,500);place(4,1,1000,550);place(5,1,1000,-550);t.key('Q');enable();samples=observe(1.5);b=ai()
check('supported raptor flanks a frontal guard',any(s['state']=='Flanking' for s in samples) and abs(b['y'])>100,y=b['y'],states=sorted({s['state'] for s in samples}));t.key('Q','up')
t.command('ai',paused=True);t.command('invulnerable',value=False);t.command('sandbox',enabled=False);t.command('match',teams=False)
print('RESULT '+str(sum(r['passed'] for r in rows))+'/'+str(len(rows)),flush=True);sys.exit(any(not r['passed'] for r in rows))
