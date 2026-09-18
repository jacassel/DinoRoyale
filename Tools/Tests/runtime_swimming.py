"""Rendered-engine swim entry/exit, buoyancy, input and animation checks."""
import runtime_core as t,time,json,sys
rows=[]
def check(name,ok,**values):
    row=dict(test=name,passed=bool(ok),**values);rows.append(row);print(('PASS ' if ok else 'FAIL ')+name+' '+json.dumps(values),flush=True)
    (t.OUT/'swimming.json').write_text(json.dumps(rows,indent=2))
t.command('match',teams=False);t.command('ai',paused=True);t.command('sandbox',enabled=True);t.command('removeTarget')
for i,name in enumerate(['Trex','Raptor','Trike']):
    t.command('species',value=i);t.command('teleport',x=6000,y=9000);t.command('face',yaw=0);time.sleep(3.5)
    s=t.state();check(name+' enters deep pond swimming',s['swimming'] and s['animation']=='Swim',animation=s['animation'],z=s['z'])
    start=s;time.sleep(1);end=t.state();check(name+' floats at surface',abs(end['z']-start['z'])<5 and -10<end['waterSurface']-end['z']<100,height=end['z'],surface=end['waterSurface'])
    a=t.state();b=t.hold('W',1);check(name+' swims forward',b['x']-a['x']>200 and b['swimming'],metres=(b['x']-a['x'])/100)
    check(name+' swimming is slower',s['swimSpeed']<s['maxSpeed'],swimSpeed=s['swimSpeed'],wadingSpeed=s['maxSpeed'])
    t.key('Q');a=t.state();t.key('W');time.sleep(.65);b=t.state();t.key('W','up');check(name+' brace holds position in water',b['brace'] and t.dist(a,b)<2,distance=t.dist(a,b));t.key('Q','up')
    t.command('damage',value=t.state()['maxHealth']*.6);time.sleep(.15);check(name+' injured swim slowdown',abs(t.state()['swimSpeed']/s['swimSpeed']-.85)<.01)
    t.command('damage',value=t.state()['maxHealth']*.22);time.sleep(.15);check(name+' critical swim slowdown',abs(t.state()['swimSpeed']/s['swimSpeed']-.7)<.01)
    t.key('RightMouseButton');time.sleep(.2);check(name+' critical charge blocked in water',not t.state()['charging']);t.key('RightMouseButton','up');t.command('heal')
    t.key('RightMouseButton');time.sleep(.5);t.key('RightMouseButton','up');time.sleep(2);check(name+' swimming recovers after heavy attack',t.state()['swimming'] and t.state()['animation']=='Swim')
    a=t.state();t.key('SpaceBar');time.sleep(.18);b=t.state();t.key('SpaceBar','up');check(name+' surface surge',b['z']>a['z']+25,rise=b['z']-a['z']);time.sleep(2);check(name+' returns to swimming after surge',t.state()['swimming'])
    t.command('camera',yaw=90);t.command('screenshot');time.sleep(.3);t.command('camera',yaw=0)
    t.key('W');start=time.monotonic()
    while time.monotonic()-start<20 and t.state()['x']<12100:time.sleep(.1)
    t.key('W','up');time.sleep(.8);s=t.state();check(name+' swims to shore and exits',s['x']>11800 and not s['swimming'] and not s['falling'],x=s['x'],z=s['z'],animation=s['animation'])
    a=s;b=t.hold('W',.6);check(name+' land controls restored',t.dist(a,b)>150 and not b['swimming'])
t.command('navAudit');audit=json.loads((t.BRIDGE/'navigation-audit.json').read_text(encoding='utf-8-sig'));check('all 42 pond-inclusive region paths',len(audit['routes'])==42 and all(r['success'] for r in audit['routes']))
t.command('sandbox',enabled=False)
print('RESULT '+str(sum(r['passed'] for r in rows))+'/'+str(len(rows)),flush=True)
sys.exit(any(not r['passed'] for r in rows))
