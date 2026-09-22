import runtime_core as t,time,json,sys
rows=[]
def check(n,ok,**data):
    rows.append(dict(test=n,passed=bool(ok),**data));(t.OUT/'settings-water-blood.json').write_text(json.dumps(rows,indent=2));print(('PASS ' if ok else 'FAIL ')+n+' '+json.dumps(data),flush=True)
def press(k):t.key(k);t.key(k,'up');time.sleep(.12)
def settings():
    if not t.state()['menuOpen']:t.command('menu',open=True)
    if not t.state()['settingsOpen']:press('F2')
def blood(enabled):
    settings()
    if t.state()['bloodEnabled']!=enabled:press('B')
    t.command('menu',open=False);time.sleep(.2)
t.command('ai',paused=True);settings();check('settings open during pause',t.state()['settingsOpen'])
a=t.state()['time'];time.sleep(.5);check('world paused',abs(t.state()['time']-a)<.01)
old=t.state()['sensitivity'];press('Equals');check('sensitivity setting changes',t.state()['sensitivity']>old);press('Hyphen')
press('Escape');check('escape returns to selection',t.state()['menuOpen'] and not t.state()['settingsOpen'])
press('Two');time.sleep(.8);check('number selects and resumes',t.state()['species']==1 and not t.state()['menuOpen'])
blood(False);t.command('species',value=0);t.command('face',yaw=0);t.command('target');time.sleep(.7)
a=t.state();press('LeftMouseButton');time.sleep(.2);b=t.state();check('blood off suppresses particles',b['targetHealth']<a['targetHealth'] and b['bloodEmitted']==a['bloodEmitted'])
time.sleep(.7);blood(True);t.command('target');time.sleep(.3);a=t.state();press('LeftMouseButton');time.sleep(.12);b=t.state();check('blood on emits on melee hit',b['bloodEmitted']>a['bloodEmitted'] and b['bloodParticles']>0,particles=b['bloodParticles']);t.command('screenshot')
blood(False);check('turning blood off clears particles',t.state()['bloodParticles']==0);t.command('removeTarget')
for i,name in enumerate(['Trex','Raptor','Trike']):
    t.command('species',value=i);time.sleep(.7);land=t.state()['maxSpeed']
    t.command('teleport',x=0,y=-5750);time.sleep(.8);s=t.state();check(name+' water slows',s['inWater'] and abs(s['maxSpeed']/land-.65)<.02,ratio=s['maxSpeed']/land)
    t.command('damage',value=s['maxHealth']*.6);time.sleep(.2);s=t.state();check(name+' water plus 50 percent injury',abs(s['maxSpeed']/land-.65*.85)<.02,ratio=s['maxSpeed']/land)
    t.command('damage',value=s['maxHealth']*.2);time.sleep(.2);s=t.state();check(name+' water plus critical injury',abs(s['maxSpeed']/land-.65*.70)<.02,ratio=s['maxSpeed']/land)
    press('RightMouseButton');check(name+' critical charge blocked in water',not t.state()['charging'])
    t.command('heal');time.sleep(.2);check(name+' healing restores water speed',abs(t.state()['maxSpeed']/land-.65)<.02)
    t.command('teleport',x=0,y=0);time.sleep(.8);check(name+' exiting restores land speed',not t.state()['inWater'] and abs(t.state()['maxSpeed']/land-1)<.02)
press('M');check('map opens',t.state()['mapOpen']);t.command('screenshot');press('M');check('map closes',not t.state()['mapOpen'])
print('RESULT '+str(sum(x['passed'] for x in rows))+'/'+str(len(rows)),flush=True)
sys.exit(any(not x['passed'] for x in rows))
