"""Live Unreal input/physics integration tests. Start the game with -DinoDevBridge.

The bridge sends FInputKeyEventArgs through the real PlayerController input mappings.
It never substitutes test movement/combat implementations. Setup commands place a dummy
and apply injury; assertions inspect the continuously simulated game state.
"""
import json, pathlib, time, math, sys

ROOT=pathlib.Path(__file__).resolve().parents[2]
BRIDGE=ROOT/'Saved/Automation'
OUT=ROOT/'Tests/Results'
OUT.mkdir(parents=True,exist_ok=True)
results=[]
sequence=0

def state(timeout=10):
    start=time.monotonic()
    while time.monotonic()-start<timeout:
        try:return json.loads((BRIDGE/'telemetry.json').read_text(encoding='utf-8-sig'))
        except (OSError,ValueError):time.sleep(.03)
    raise RuntimeError('Game telemetry did not respond')

def command(cmd,**values):
    global sequence
    sequence=max(sequence,int(state().get('seq',0)))+1
    path=BRIDGE/'command.json'
    payload=json.dumps(dict(seq=sequence,cmd=cmd,**values))
    for attempt in range(30):
        try:path.write_text(payload);break
        except OSError:time.sleep(.03)
    start=time.monotonic()
    while time.monotonic()-start<10:
        s=state()
        if s['seq']>=sequence:return s
        time.sleep(.025)
    raise RuntimeError('Game did not acknowledge '+cmd)

def key(k,event='down',value=1):return command('key',key=k,event=event,value=value)
def hold(k,duration):
    key(k);time.sleep(duration);key(k,'up');time.sleep(.15);return state()
def check(name,passed,**data):
    row=dict(test=name,passed=bool(passed),**data);results.append(row)
    print(('PASS' if passed else 'FAIL')+' '+name+' '+json.dumps(data),flush=True)
    (OUT/'core-live.json').write_text(json.dumps(results,indent=2))
def dist(a,b):return math.hypot(a['x']-b['x'],a['y']-b['y'])

def run_species(i,name):
    command('species',value=i);command('face',yaw=0);time.sleep(.8)
    base=state();normal_speed=base['maxSpeed']
    check(name+' selection',base['species']==i and base['health']==base['maxHealth'])
    for k,axis,sign in [('W','x',1),('S','x',-1),('D','y',1),('A','y',-1)]:
        command('face',yaw=0);a=state();b=hold(k,.8)
        check(name+' '+k+' movement',(b[axis]-a[axis])*sign>120,displacement=round(b[axis]-a[axis],1))
    a=state();key('MouseX','axis',12);time.sleep(.15);b=state()
    check(name+' camera yaw',abs(b['cameraYaw']-a['cameraYaw'])>1,delta=b['cameraYaw']-a['cameraYaw'])
    a=state();key('MouseY','axis',5);time.sleep(.15);b=state()
    check(name+' camera pitch',abs(b['cameraPitch']-a['cameraPitch'])>.1,delta=b['cameraPitch']-a['cameraPitch'])
    a=state();key('SpaceBar');time.sleep(.22);b=state();key('SpaceBar','up')
    check(name+' jump',b['falling'] and b['z']>a['z']+25,rise=round(b['z']-a['z'],1))
    time.sleep(1.3)
    key('Q');a=state();key('W');time.sleep(.7);b=state();key('W','up')
    check(name+' brace blocks movement',b['brace'] and dist(a,b)<2,distance=round(dist(a,b),3))
    key('LeftMouseButton');key('LeftMouseButton','up');time.sleep(.2)
    check(name+' brace blocks attack',state()['recovery']==0)
    key('Q','up');a=state();b=hold('W',.5)
    check(name+' release restores movement',not b['brace'] and dist(a,b)>80)
    command('face',yaw=0);command('target');time.sleep(.6)
    a=state();key('LeftMouseButton');key('LeftMouseButton','up');time.sleep(.35);b=state()
    quick=a['targetHealth']-b['targetHealth'];normal_duration=b['attackDuration']
    check(name+' quick hit',quick>0,damage=quick)
    time.sleep(1);c=state()
    check(name+' no duplicate hits',abs(c['targetHealth']-b['targetHealth'])<.01)
    command('target');time.sleep(.3);a=state();key('RightMouseButton');time.sleep(2);b=state()
    check(name+' charge feedback',b['charging'] and b['charge']>.95)
    key('RightMouseButton','up');time.sleep(.45);b=state();charged=a['targetHealth']-b['targetHealth']
    check(name+' charged hit stronger',charged>quick,quick=quick,charged=charged)
    time.sleep(1.6);check(name+' combat state recovers',not state()['charging'] and state()['recovery']==0)
    command('damage',value=state()['maxHealth']*.60);time.sleep(.15);a=state()
    check(name+' damage lowers health',.38<a['health']/a['maxHealth']<.42)
    check(name+' injured movement',abs(a['maxSpeed']/normal_speed-.85)<.01,ratio=a['maxSpeed']/normal_speed)
    key('LeftMouseButton');key('LeftMouseButton','up');time.sleep(.15);a=state()
    check(name+' injured attack slowdown',a['attackDuration']>normal_duration*1.09,ratio=a['attackDuration']/normal_duration)
    time.sleep(1.2);command('damage',value=state()['maxHealth']*.22);time.sleep(.15);a=state()
    check(name+' critical movement',abs(a['maxSpeed']/normal_speed-.70)<.01,ratio=a['maxSpeed']/normal_speed)
    key('RightMouseButton');time.sleep(.2);check(name+' critical charge blocked',not state()['charging']);key('RightMouseButton','up')
    key('LeftMouseButton');key('LeftMouseButton','up');time.sleep(.12);a=state()
    check(name+' critical attack slowdown',a['attackDuration']>normal_duration*1.30,ratio=a['attackDuration']/normal_duration)
    time.sleep(1.3);a=state();time.sleep(1.5);b=state()
    check(name+' regen waits',abs(b['health']-a['health'])<.1,delta=b['health']-a['health'])
    time.sleep(5);c=state()
    check(name+' passive regen',c['health']>b['health']+c['maxHealth']*.04,delta=c['health']-b['health'])
    command('heal');time.sleep(.2);a=state()
    check(name+' recovered speed',abs(a['maxSpeed']-normal_speed)<1)
    key('RightMouseButton');time.sleep(.2);check(name+' recovered charge',state()['charging']);key('RightMouseButton','up');time.sleep(1.8)
    command('damage',value=100000);time.sleep(.15);check(name+' death',state()['dead'] and state()['health']==0)
    a=state();hold('W',.5);check(name+' dead movement blocked',dist(a,state())<2)
    time.sleep(6);a=state();check(name+' respawn',not a['dead'] and a['health']==a['maxHealth'])
    b=hold('W',.5);check(name+' respawn movement',dist(a,b)>80)
    command('screenshot');time.sleep(.3)

if __name__=='__main__':
    try:
        for i,n in enumerate(['Trex','Raptor','Trike']):run_species(i,n)
    finally:
        for k in ['W','A','S','D','Q','SpaceBar','RightMouseButton','LeftMouseButton']:
            try:key(k,'up')
            except Exception:pass
    failed=sum(not r['passed'] for r in results)
    print(f'RESULT: {len(results)-failed}/{len(results)} passed',flush=True)
    sys.exit(bool(failed))
