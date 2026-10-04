import os,sys,pathlib,time,json,math
ROOT=pathlib.Path(__file__).resolve().parents[2]
os.environ['DINO_BRIDGE_DIR']=str(ROOT/'Dist/Releases/DinoRoyale-0.3-Alpha-Test/Windows/DinosaurBattle/Saved/Automation/NativeAcceptance')
OUT=ROOT/'Tests/Results/alpha03/native-acceptance';OUT.mkdir(parents=True,exist_ok=True)
os.environ['DINO_RESULTS_DIR']=str(OUT)
sys.path.insert(0,str(ROOT/'Tools/Tests'))
import runtime_core as t

def snapshot(label):
    s=t.state();(OUT/(label+'.json')).write_text(json.dumps(s,indent=2))
    actor=next((a for a in s['networkActors'] if a['id']==1),{})
    print(json.dumps({'label':label,**{k:s.get(k) for k in ['species','health','x','y','yaw','cameraYaw','animation','speed','pivot','charging','charge','recovery']},'opponent':{k:actor.get(k) for k in ['species','health','x','y','yaw']}}))
    return s

mode=sys.argv[1]
if mode=='setup':
    player=int(sys.argv[2]);opponent=int(sys.argv[3]);pack=bool(int(sys.argv[4]));label=sys.argv[5]
    for key in ['Q','E','W','A','S','D','LeftControl','RightMouseButton','LeftMouseButton','LeftShift']:t.key(key,'up')
    t.command('menu',open=False);t.command('ai',paused=True);t.command('sandbox',enabled=True)
    t.command('species',value=player);t.command('match',teams=False);t.command('ai',paused=True);t.command('invulnerable',value=True)
    t.command('teleport',x=0,y=0);t.command('face',yaw=0);t.command('camera',yaw=0,pitch=-15,distance=1000)
    for a in t.state()['networkActors']:
        if a['id']!=0:
            t.command('testAI',id=a['id'],species=a['species'],x=-24000+a['id']*350,y=-22000,yaw=0,health=1,enabled=False)
    t.command('testAI',id=1,species=opponent,x=150,y=-280,yaw=90,health=1,enabled=False)
    if pack:
        followers=[a for a in t.state()['networkActors'] if a['id']!=0 and a['species']==1 and a['id']<10]
        for i,a in enumerate(followers):
            t.command('testAI',id=a['id'],species=1,x=-180,y=-180+i*360,yaw=-45,health=1,enabled=False)
    time.sleep(.5);snapshot(label)
elif mode=='hold':
    key=sys.argv[2];duration=float(sys.argv[3]);label=sys.argv[4]
    rows=[];t.key(key);end=time.monotonic()+duration
    while time.monotonic()<end: rows.append(t.state());time.sleep(.03)
    t.key(key,'up');time.sleep(.15)
    (OUT/(label+'-trace.json')).write_text(json.dumps(rows,indent=2));snapshot(label)
elif mode=='snapshot':snapshot(sys.argv[2])
elif mode=='heavy':
    label=sys.argv[2];t.key('RightMouseButton');time.sleep(1);t.key('RightMouseButton','up');t.key('E');before=t.state();time.sleep(.25);after=t.state();t.key('E','up')
    turn=(after['yaw']-before['yaw']+180)%360-180
    report={'method':'Development bridge held buttons; native attack/pivot taps recorded separately','turnDegreesDuringFirstQuarterSecond':turn,'before':before,'after':after}
    (OUT/(label+'.json')).write_text(json.dumps(report,indent=2));print(json.dumps({'label':label,'heavyCommitted':abs(turn)<20,'turn':turn}))
elif mode=='live':
    t.command('enableAI',id=1,enabled=True)
    if 'pack' in sys.argv[2]:
        for a in t.state()['networkActors']:
            if a['id']!=0 and a['species']==1 and a['id']<10:t.command('enableAI',id=a['id'],enabled=True)
    snapshot(sys.argv[2])
elif mode=='pause':t.command('ai',paused=True);t.command('menu',open=True)
