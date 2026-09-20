"""Focused follow-up checks for exhausting actions, materials and raptor survival."""
import runtime_core as t,time,json,sys
rows=[]
def wait(s):
 start=t.state()['time']
 while t.state()['time']-start<s:time.sleep(.02)
def press(k):t.key(k);t.key(k,'up')
def check(n,ok,**v):
 rows.append(dict(test=n,passed=bool(ok),**v));(t.OUT/'combat-followup.json').write_text(json.dumps(rows,indent=2));print(('PASS ' if ok else 'FAIL ')+n+' '+json.dumps(v),flush=True)
t.command('menu',open=False);t.command('ai',paused=True);t.command('sandbox',enabled=True)
for i,n in enumerate(['Trex','Raptor','Trike']):
 t.command('species',value=i);wait(.6);check(n+' correct skin after species switch','M_'+n+'_BakedSkin' in t.state()['materials'],materials=t.state()['materials'])
 t.key('W');t.key('LeftShift');t.command('stamina',value=2);samples=[];start=t.state()['time']
 while t.state()['time']-start<4:samples.append(t.state());time.sleep(.04)
 t.key('W','up');t.key('LeftShift','up');check(n+' held sprint exhausts then automatically resumes',any(s['exhausted'] and not s['sprinting'] for s in samples) and any(s['sprinting'] and s['stamina']>10 for s in samples[15:]))
 t.command('species',value=i);wait(.5);t.command('stamina',value=0);start=t.state()['time'];recovered=False
 while t.state()['time']-start<7:
  s=t.state();recovered|=not s['exhausted'];
  if s['recovery']<=0:press('LeftMouseButton')
  time.sleep(.06)
 check(n+' repeated weak attacks cannot permanently lock stamina',recovered,stamina=t.state()['stamina']);wait(2)
 t.command('menu',open=True);baseline=t.state()['materials'];press('F2');press('B');check(n+' blood toggle preserves materials',baseline==t.state()['materials']);press('B');check(n+' disabling clears all particles',t.state()['bloodParticles']==0);t.command('menu',open=False)
t.command('species',value=1);check('raptor fragility adjustment loaded',t.state()['maxHealth']==520)
t.command('scoreHit',attacker=1,victim=0,value=673.2);wait(.15);check('unbraced raptor dies to full rex heavy',t.state()['dead'] and t.state()['health']==0,remaining=t.state()['health'])
t.command('species',value=0);t.command('ai',paused=True);print('RESULT',sum(r['passed'] for r in rows),'/',len(rows),flush=True);sys.exit(any(not r['passed'] for r in rows))
