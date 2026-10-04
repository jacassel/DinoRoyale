"""Two real UE processes over loopback. This is NOT an internet/EOS test.

Only server-side opt-in bridge commands arrange fixtures. All client attacks,
movement and abilities use mapped input; assertions compare both processes.
"""
import json, math, os, pathlib, subprocess, time, argparse

ROOT=pathlib.Path(__file__).resolve().parents[2]
parser=argparse.ArgumentParser()
parser.add_argument('--output',default='Tests/Results/multiplayer/stage-b')
parser.add_argument('--lag',type=int,default=0,help='One-way outgoing delay per process, milliseconds')
parser.add_argument('--loss',type=int,default=0)
parser.add_argument('--executable',help='Direct packaged DinosaurBattle/Binaries/Win64 executable')
args=parser.parse_args()
OUT=ROOT/args.output;OUT.mkdir(parents=True,exist_ok=True)
BRIDGE=(pathlib.Path(args.executable).resolve().parents[2] if args.executable else ROOT)/'Saved/Automation'
rows=[]
processes=[]

class Peer:
    def __init__(self,name,url):
        self.name=name;self.path=BRIDGE/name;self.path.mkdir(parents=True,exist_ok=True)
        (self.path/'telemetry.json').unlink(missing_ok=True)
        (self.path/'command.json').write_text('{"seq":0,"cmd":"noop"}')
        cmd=([str(args.executable)] if args.executable else [r'C:\Unreal Engine\UE_5.8\Engine\Binaries\Win64\UnrealEditor.exe',str(ROOT/'DinosaurBattle.uproject')])+[url,
             '-game','-nullrhi','-unattended','-nosplash','-nosound','-DinoDevBridge','-DinoBridge='+name,
             '-port=7787','-multihome=127.0.0.1','-abslog='+str(OUT/(name+'.log')),
             '-PktLag='+str(args.lag),'-PktLoss='+str(args.loss)]
        self.proc=subprocess.Popen(cmd,creationflags=subprocess.CREATE_NO_WINDOW);processes.append(self.proc)
        deadline=time.monotonic()+75
        while time.monotonic()<deadline:
            if self.proc.poll() is not None:raise RuntimeError(name+' exited during startup')
            if (self.path/'telemetry.json').exists():break
            time.sleep(.25)
        else:raise TimeoutError(name+' telemetry unavailable')
    def state(self):
        deadline=time.monotonic()+5
        while time.monotonic()<deadline:
            try:return json.loads((self.path/'telemetry.json').read_text(encoding='utf-8-sig'))
            except (ValueError,OSError):time.sleep(.02)
        raise TimeoutError(self.name+' telemetry read failed')
    def command(self,cmd,**kwargs):
        seq=self.state()['seq']+1
        (self.path/'command.json').write_text(json.dumps(dict(seq=seq,cmd=cmd,**kwargs)))
        deadline=time.monotonic()+10
        while time.monotonic()<deadline:
            s=self.state()
            if s['seq']>=seq:return s
            time.sleep(.03)
        raise TimeoutError(self.name+' did not acknowledge '+cmd)
    def key(self,key,event='down'):return self.command('key',key=key,event=event)
    def tap(self,key):self.key(key);self.key(key,'up')
    def hold(self,key,seconds):
        self.key(key);time.sleep(seconds);self.key(key,'up');time.sleep(.3);return self.state()
    def actor(self,id):return next(a for a in self.state()['networkActors'] if a['id']==id)

def check(name,passed,**evidence):
    rows.append(dict(test=name,status='PASS' if passed else 'FAIL',**evidence))
    print(rows[-1],flush=True)
    (OUT/'results.json').write_text(json.dumps(dict(transport='loopback development transport',wan='NOT VERIFIED',players=2,oneWayLagMs=args.lag,packetLossPercent=args.loss,tests=rows),indent=2))
    if not passed:raise AssertionError(name)

def wait_for(predicate,seconds=8):
    deadline=time.monotonic()+seconds
    while time.monotonic()<deadline:
        if predicate():return True
        time.sleep(.1)
    return False

def distance(a,b):return math.hypot(a['x']-b['x'],a['y']-b['y'])

try:
    host=Peer('NetHost','/Game/Maps/LostValley?listen?bUseIPSockets')
    host.command('ai',paused=True)
    client=Peer('NetClient','127.0.0.1:7787')
    time.sleep(2)
    check('net emulation settings active',host.state().get('emulatedLagMs')==args.lag and client.state().get('emulatedLagMs')==args.lag and host.state().get('emulatedLossPercent')==args.loss and client.state().get('emulatedLossPercent')==args.loss,oneWayMs=args.lag,lossPercent=args.loss,measuredClientPingMs=client.state().get('pingMs'))
    check('two unique possessed players',host.state()['combatantID']==0 and client.state()['combatantID']==1,hostMode=host.state()['netMode'],clientMode=client.state()['netMode'])
    check('replicated participant count',len([a for a in client.state()['networkActors'] if a['id']<100])==2)
    check('client terrain collision',client.state()['floorActor'].startswith('LostValleyWorld'),floor=client.state()['floorActor'])
    a=client.state();client.command('face',yaw=0);b=client.hold('W',1)
    check('client movement',distance(a,b)>200,displacement=distance(a,b))
    check('host sees client movement',wait_for(lambda:distance(host.actor(1),client.state())<90),error=distance(host.actor(1),client.state()))
    before=client.state();client.key('LeftShift');client.key('W');time.sleep(1);sprint=client.state();client.key('W','up');client.key('LeftShift','up')
    check('server validates client sprint',sprint['sprinting'] and sprint['stamina']<before['stamina'],speed=sprint['speed'],stamina=sprint['stamina'])
    time.sleep(.8);a=client.state();client.key('SpaceBar');time.sleep(.3+args.lag/1000);b=client.state();client.key('SpaceBar','up')
    check('client jump',b['z']>a['z']+25,rise=b['z']-a['z'])
    time.sleep(1.5);client.key('LeftControl');time.sleep(.35+args.lag/500);a=client.state();b=client.hold('W',.5)
    check('replicated brace stops movement',b['brace'] and distance(a,b)<15,distance=distance(a,b));client.key('LeftControl','up')
    # Host-authoritative fixture: two Rexes 450 cm apart, facing each other.
    host.command('teleport',x=0,y=0);host.command('face',yaw=0)
    host.command('testAI',id=1,species=0,x=450,y=0,yaw=180,health=1,enabled=False)
    client.command('face',yaw=180);time.sleep(1)
    before=host.state()['health'];client.tap('LeftMouseButton');time.sleep(.7+args.lag/500)
    after=host.state()['health']
    check('client attack damages authoritative host',after<before,damage=before-after)
    check('damage replicated to attacking client',abs(client.actor(0)['health']-after)<.1)
    time.sleep(.7);check('no repeated quick damage',abs(host.state()['health']-after)<.1)
    host.command('testAI',id=1,species=0,x=450,y=0,yaw=180,health=1,enabled=False);time.sleep(.5)
    client.key('RightMouseButton');time.sleep(2.2+args.lag/500)
    check('charge initiation replicated',client.state()['charging'] and client.state()['charge']>.95)
    before=host.state()['health'];client.key('RightMouseButton','up');time.sleep(.7+args.lag/500)
    check('charged client hit',host.state()['health']<before,damage=before-host.state()['health'])
    time.sleep(1.6)
    host.command('testAI',id=1,species=0,x=450,y=0,yaw=180,health=.05,enabled=False);host.command('teleport',x=0,y=0);host.command('face',yaw=0);time.sleep(.7)
    host.tap('LeftMouseButton')
    check('host attack kills client',wait_for(lambda:client.state()['dead']))
    check('death creates one replicated carcass',wait_for(lambda:len(client.state()['corpses'])==len(host.state()['corpses']) and len(client.state()['corpses'])>0),count=len(client.state()['corpses']))
    # Peer telemetry files update independently. Seeing death on the client
    # does not imply the host's next score snapshot has been written yet.
    check('server score increments once',wait_for(lambda:host.state()['kills']==1),kills=host.state()['kills'])
    time.sleep(5);check('client waits through respawn delay',client.state()['dead'])
    check('server respawns client',wait_for(lambda:not client.state()['dead'],seconds=8))
    check('respawn health replicated',abs(client.state()['health']-client.state()['maxHealth'])<.1)
    check('respawn position replicated',wait_for(lambda:distance(host.actor(1),client.state())<90),error=distance(host.actor(1),client.state()))
    check('death counted once',host.state()['kills']==1)
    client.command('menu',open=True);client.key('F10');client.proc.wait(timeout=15)
    check('disconnect removes player actor',wait_for(lambda:not any(a['id']==1 for a in host.state()['networkActors'])))
    client_log=(OUT/'NetClient.log').read_text(errors='replace')
    check('movement base resolves on client','could not resolve the new relative movement base' not in client_log)
    host.command('menu',open=True);host.key('F10');host.proc.wait(timeout=15)
finally:
    for proc in processes:
        if proc.poll() is None:proc.terminate();proc.wait(timeout=15)
