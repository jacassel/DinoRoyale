"""Actual audio event/lifetime checks through player input and runtime telemetry."""
import runtime_core as t,time,json,sys,wave,array,math,shutil
rows=[]
def wait(seconds):
    start=t.state()['time'];deadline=time.monotonic()+seconds*4+5
    while t.state()['time']-start<seconds and time.monotonic()<deadline:time.sleep(.025)
def check(name,ok,**data):
    rows.append(dict(test=name,passed=bool(ok),**data));print(rows[-1],flush=True)
    (t.OUT/'audio.json').write_text(json.dumps(rows,indent=2))
def press(key):t.key(key);t.key(key,'up')
t.command('menu',open=False);t.command('match',teams=False);t.command('ai',paused=True);t.command('sandbox',enabled=True)
for sp,name in enumerate(['Trex','Raptor','Trike']):
    t.command('species',value=sp);t.command('face',yaw=0);wait(.6)
    t.command('invulnerable',value=False)
    recording_started=time.time();t.command('audioRecord',start=True)
    check(name+' all clips loaded',t.state()['audioLoadedClips']==81)
    a=t.state();wait(.5);check(name+' idle quiet',t.state()['audioSteps']==a['audioSteps'])
    # Include enough complete strides to distinguish cadence from the starting
    # footstep phase and the heavy species' slower sprint acceleration.
    t.command('teleport',x=0,y=0);t.key('W');a=t.state();wait(3);b=t.state();t.key('W','up');walk=b['audioSteps']-a['audioSteps'];wait(.3)
    t.command('teleport',x=0,y=0);t.key('W');t.key('LeftShift');a=t.state();wait(3);b=t.state();t.key('W','up');t.key('LeftShift','up');run=b['audioSteps']-a['audioSteps']
    check(name+' movement cadence',walk>=1 and run>walk,walk=walk,run=run)
    check(name+' sprint exertion',b['audioSprintBreaths']>a['audioSprintBreaths'])
    wait(.4);press('SpaceBar');wait(.15);a=t.state();wait(.2);b=t.state();check(name+' airborne quiet',a['falling'] and b['audioSteps']==a['audioSteps']);wait(1.3)
    t.command('teleport',x=3000,y=4500);wait(2);a=t.state();t.key('W');wait(.7);t.key('W','up');b=t.state();check(name+' swimming has no footsteps',a['swimming'] and b['audioSteps']==a['audioSteps'])
    t.command('teleport',x=0,y=0);t.command('face',yaw=0);wait(.6);t.command('removeTarget');a=t.state();press('LeftMouseButton');wait(.4);b=t.state()
    check(name+' miss motion without impact',b['audioQuick']==a['audioQuick']+1 and b['audioImpacts']==a['audioImpacts']);wait(1)
    t.command('target');a=t.state();press('LeftMouseButton');wait(.4);b=t.state();check(name+' hit sound',b['audioImpacts']>a['audioImpacts']);wait(1.8)
    t.command('target');a=t.state();t.key('RightMouseButton');wait(1.4)
    check(name+' charge starts once',t.state()['audioCharges']==a['audioCharges']+1)
    t.key('RightMouseButton','up');wait(.45);b=t.state();check(name+' heavy sound',b['audioHeavy']==a['audioHeavy']+1 and b['audioVoices']<=3 and b['audioTotalVoices']<=24)
    wait(2);a=t.state();t.command('damage',value=a['maxHealth']*.65);wait(.3);b=t.state()
    check(name+' hurt reaction',b['audioHurts']==a['audioHurts']+1)
    wait(2.4);c=t.state();check(name+' injured breathing',c['audioInjuredBreaths']>b['audioInjuredBreaths'])
    t.command('heal');wait(1.2);a=t.state();wait(1);check(name+' healing stops injured breath',t.state()['audioInjuredBreaths']==a['audioInjuredBreaths'])
    wait(2);t.command('removeTarget');a=t.state();t.command('damage',value=100000);wait(.2);b=t.state();check(name+' death once',b['audioDeaths']==a['audioDeaths']+1 and b['dead'])
    t.command('menu',open=True);paused=t.state();time.sleep(.5);check(name+' menu pauses simulation',t.state()['time']==paused['time']);t.command('menu',open=False)
    wait(3.1);c=t.state();check(name+' death voice ends without looping',c['audioDeaths']==b['audioDeaths'] and c['audioVoices']==0)
    wait(8);c=t.state();check(name+' respawn silent',not c['dead'] and c['audioVoices']==0 and c['audioDeaths']==b['audioDeaths'])
    t.command('audioRecord',start=False)
    path=t.BRIDGE.parent/'AudioQA'/f'Species{sp}.wav'
    deadline=time.monotonic()+10
    while (not path.exists() or path.stat().st_mtime<recording_started) and time.monotonic()<deadline:time.sleep(.1)
    time.sleep(.3)
    shutil.copyfile(path,t.OUT/f'{name}-runtime.wav')
    with wave.open(str(path),'rb') as wav:
        pcm=array.array('h',wav.readframes(wav.getnframes()))
    peak=max(abs(x) for x in pcm)/32768;rms=math.sqrt(sum(x*x for x in pcm)/len(pcm))/32768
    check(name+' rendered audio nonzero and unclipped',.0001<rms<.5 and peak<.999,peak=peak,rms=rms)
t.command('menu',open=True)
sys.exit(any(not r['passed'] for r in rows))
