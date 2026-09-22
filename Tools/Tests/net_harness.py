"""Opt-in loopback test harness. This transport does not verify EOS or WAN."""
import json, math, pathlib, subprocess, time
ROOT=pathlib.Path(__file__).resolve().parents[2]
BUILD=2026092201

def wait_for(predicate,seconds=10):
    deadline=time.monotonic()+seconds
    while time.monotonic()<deadline:
        if predicate():return True
        time.sleep(.1)
    return False

def distance(a,b):return math.hypot(a['x']-b['x'],a['y']-b['y'])

class NetworkTest:
    def __init__(self,output,lag=0,loss=0,executable=None):
        self.out=ROOT/output;self.out.mkdir(parents=True,exist_ok=True)
        self.peers=[];self.rows=[];self.lag=lag;self.loss=loss;self.executable=executable
        self.bridge_root=(pathlib.Path(executable).resolve().parents[2] if executable else ROOT)/'Saved/Automation'
    def check(self,name,passed,**evidence):
        row=dict(test=name,status='PASS' if passed else 'FAIL',**evidence);self.rows.append(row);print(row,flush=True)
        (self.out/'results.json').write_text(json.dumps(dict(transport='loopback development sockets',eos='NOT VERIFIED',wan='NOT VERIFIED',instances=len(self.peers),oneWayLagMs=self.lag,lossPercent=self.loss,tests=self.rows),indent=2))
        if not passed:raise AssertionError(name)
    def close(self):
        for peer in self.peers:
            if peer.proc.poll() is None:peer.proc.terminate()
        for peer in self.peers:
            if peer.proc.poll() is None:peer.proc.wait(timeout=20)

class Peer:
    def __init__(self,test,name,url=None):
        self.test=test;self.name=name;self.path=test.bridge_root/name;self.path.mkdir(parents=True,exist_ok=True)
        (self.path/'telemetry.json').unlink(missing_ok=True)
        (self.path/'command.json').write_text('{"seq":0,"cmd":"noop"}')
        if url is None:url=f'127.0.0.1:7788?DinoBuild={BUILD}'
        cmd=([str(test.executable)] if test.executable else [r'C:\Unreal Engine\UE_5.8\Engine\Binaries\Win64\UnrealEditor.exe',str(ROOT/'DinosaurBattle.uproject')])
        cmd += [url,'-game','-nullrhi','-nosound','-unattended','-nosplash','-DinoDevBridge','-DinoBridge='+name,
                '-port=7788','-multihome=127.0.0.1','-abslog='+str(test.out/(name+'.log')),
                '-PktLag='+str(test.lag),'-PktLoss='+str(test.loss)]
        self.proc=subprocess.Popen(cmd,creationflags=subprocess.CREATE_NO_WINDOW);test.peers.append(self)
        deadline=time.monotonic()+90
        while time.monotonic()<deadline:
            if self.proc.poll() is not None:raise RuntimeError(name+' exited during startup')
            if (self.path/'telemetry.json').exists():return
            time.sleep(.2)
        raise TimeoutError(name+' did not initialize')
    def state(self):
        deadline=time.monotonic()+5
        while time.monotonic()<deadline:
            try:return json.loads((self.path/'telemetry.json').read_text(encoding='utf-8-sig'))
            except (ValueError,OSError):time.sleep(.02)
        raise TimeoutError(self.name+' telemetry unavailable')
    def command(self,cmd,**kwargs):
        seq=self.state()['seq']+1
        (self.path/'command.json').write_text(json.dumps(dict(seq=seq,cmd=cmd,**kwargs)))
        if not wait_for(lambda:self.state()['seq']>=seq):raise TimeoutError(self.name+' command '+cmd)
        return self.state()
    def lobby(self,action,value=0):return self.command('lobby',action=action,value=value)
    def key(self,key,event='down'):return self.command('key',key=key,event=event)
    def tap(self,key):self.key(key);self.key(key,'up')
    def hold(self,key,seconds):
        self.key(key);time.sleep(seconds);self.key(key,'up');time.sleep(.3);return self.state()
    def actor(self,id):return next(a for a in self.state()['networkActors'] if a['id']==id)
    def player(self,id):return next(p for p in self.state()['players'] if p['id']==id)
    def quit(self):
        self.command('menu',open=True);self.key('F10');self.proc.wait(timeout=20)

def host_url(capacity=10,teams=False,bots=False):
    return f'/Game/Maps/LostValley?listen?bUseIPSockets?OnlineLobby=1?Capacity={capacity}?Teams={int(teams)}?Bots={int(bots)}?DinoBuild={BUILD}'
