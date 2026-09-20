"""Record actual multi-animal combat audio without an invulnerable combat target."""
import runtime_core as t,time,json,wave,array,math,shutil,sys
rows=[]
isolated=t.BRIDGE.parent/'AudioQA/Species2.wav'
if isolated.exists():shutil.copyfile(isolated,isolated.with_name('Species2-isolated.wav'))
for opponent in [0,2]:
    t.command('menu',open=False);t.command('duelSetup',seed=313+opponent,angle=0,opponent=opponent,pack=True)
    # Dead spectator stays excluded from AI target selection but hears the nearby fight.
    t.command('teleport',x=0,y=-1200);t.command('face',yaw=90)
    started=time.time();t.command('audioRecord',start=True);begin=t.state()['time'];voice_samples=[]
    while t.state()['time']-begin<12:
        voice_samples.append(t.state()['audioTotalVoices']);time.sleep(.1)
    t.command('audioRecord',start=False)
    p=t.BRIDGE.parent/'AudioQA/Species2.wav';deadline=time.monotonic()+10
    while (not p.exists() or p.stat().st_mtime<started) and time.monotonic()<deadline:time.sleep(.1)
    time.sleep(.3)
    with wave.open(str(p),'rb') as w:pcm=array.array('h',w.readframes(w.getnframes()))
    peak=max(abs(x) for x in pcm)/32768;rms=math.sqrt(sum(x*x for x in pcm)/len(pcm))/32768
    shutil.copyfile(p,p.with_name(f'Combat-{opponent}.wav'))
    shutil.copyfile(p,t.OUT/f'Combat-{opponent}.wav')
    rows.append(dict(opponent=opponent,passed=.0001<rms<.5 and peak<.999 and max(voice_samples,default=0)<=24,peak=peak,rms=rms,maxVoices=max(voice_samples,default=0)));print(rows[-1],flush=True)
(t.OUT/'audio-mix.json').write_text(json.dumps(rows,indent=2));t.command('menu',open=True)
sys.exit(any(not r['passed'] for r in rows))
