"""Design finite, varied creature/foley layers from the credited CC0 source recordings.

Requires numpy, scipy and soundfile. No part of the YouTube reference is sampled.
The mixes are cinematic creature designs, not recordings of extinct animals.
"""
from pathlib import Path
import json, hashlib
import numpy as np
import soundfile as sf
from scipy import signal

ROOT = Path(__file__).resolve().parents[2]
SRC = ROOT / 'Assets/Audio/Source'
OUT = ROOT / 'Assets/Audio/Designed'
OUT.mkdir(parents=True, exist_ok=True)
RATE = 44100
PACK = SRC / '80-cc0-creture-sfx-2'
SNARL = SRC / 'monster-snarls'
DEEP = SRC / 'cc0-deep-monster-roar/monster_roar.wav'
EVENTS = ['Step', 'Quick', 'Heavy', 'Impact', 'Death', 'Charge', 'Hurt', 'SprintBreath', 'InjuredBreath']
LENGTHS = [.28, .52, 1.18, .31, 2.65, .85, .68, .82, 1.12]
cache = {}

def read(p):
    if p not in cache:
        x, sr = sf.read(p)
        if x.ndim == 2: x = x.mean(axis=1)
        x = signal.resample_poly(x, RATE, sr)
        x = signal.sosfilt(signal.butter(2, 32, 'highpass', fs=RATE, output='sos'), x)
        cache[p] = x / max(.01, np.max(np.abs(x)))
    return cache[p]

def layer(path, n, rate=1., variant=0):
    x = read(path)
    # Choose an energetic phrase, varying the start without hard-cutting the ends.
    needed = max(32, int(n*rate))
    if len(x) > needed:
        offsets = np.linspace(0, len(x)-needed, 17).astype(int)
        energy = [np.mean(x[k:k+needed]**2) for k in offsets]
        order = np.argsort(energy)[::-1]
        start = offsets[order[variant % min(4,len(order))]]
        x = x[start:start+needed]
    x = np.interp(np.arange(n)*rate, np.arange(len(x)), x, left=0, right=0)
    return x

def noise(rng, n, low, high):
    x = rng.normal(size=n)
    x = signal.sosfilt(signal.butter(3, [low,high], 'bandpass', fs=RATE, output='sos'),x)
    return x / max(.001,np.sqrt(np.mean(x*x)))

rows=[]
for sp, name in enumerate(['Trex','Raptor','Trike']):
    pitch = [.83,1.24,.97][sp]
    primary = [DEEP,PACK/'roar_05.ogg',PACK/'roar_06.ogg'][sp]
    grit = [SNARL/'monster-snarl-attack.ogg',SNARL/'monster-snarls-2_0.ogg',PACK/'grunt_07.ogg'][sp]
    for kind, event in enumerate(EVENTS):
        for v in range(3):
            rng=np.random.default_rng(4400+sp*100+kind*7+v)
            length=LENGTHS[kind]*([1.,.79,1.04][sp] if kind not in [0,3] else 1)
            n=int(length*RATE); t=np.arange(n)/RATE; u=t/length
            rate=pitch*[.94,1.,1.07][v]
            x=np.zeros(n)
            if kind in [0,3]:
                body=layer(PACK/'stomp_01.ogg',n,[.64,1.22,.78][sp],v)
                soil=noise(rng,n,350,5300)*np.exp(-t*(28 if kind==0 else 20))
                mass=np.sin(2*np.pi*([43,94,55][sp])*t)*np.exp(-t*24)
                x=.65*body+.10*soil+.16*mass
                if kind==3:
                    x+=.20*layer(PACK/f'slime_0{v+1}.ogg',n,.9,v)
            elif kind in [7,8]:
                # Breath has no periodic electronic oscillator: air and recorded glottal grit.
                air=noise(rng,n,180 if sp!=1 else 450,2600 if sp!=1 else 5100)
                cycle=np.sin(np.pi*u)**1.6
                x=.14*air*cycle+.17*layer(PACK/'breath_02.ogg',n,rate*.65,v)
                if kind==8: x+=.16*layer(grit,n,rate*.75,v)*(1-u)
            else:
                src=primary if kind in [2,4] else grit
                if kind==6: src=PACK/(['die_02.ogg','die_03.ogg','grunt_09.ogg'][sp])
                x=.70*layer(src,n,rate*(.81 if kind==4 else 1),v)
                x+=.27*layer(grit if src!=grit else primary,n,rate*1.03,(v+1)%3)
                x+=.035*noise(rng,n,220,3400)*np.sin(np.pi*u)
                if kind==5: x*=.3+.7*np.sin(np.pi*u/2)
                if kind==4:
                    x*=np.exp(-u*1.5)
                    x+=.14*noise(rng,n,130,2000)*np.sin(np.pi*u)**2*(u**2)
            # Each exported clip has gentle, zero-valued boundaries and mix headroom.
            x=signal.sosfilt(signal.butter(2, 6500 if sp==1 else 4700, 'lowpass',fs=RATE,output='sos'),x)
            attack=.008 if kind in [0,3] else .025
            release=.06 if kind in [0,3] else .16
            env=np.minimum(1,t/attack)*np.minimum(1,(length-t)/release)
            x*=np.maximum(0,env)
            rms=np.sqrt(np.mean(x*x)); target=.16 if kind in [1,2,4,5,6] else .10
            x*=min(target/max(rms,1e-8),.72/max(np.max(np.abs(x)),1e-8))
            x[0]=x[-1]=0
            path=OUT/f'{name}_{event}_{v}.wav';sf.write(path,x,RATE,subtype='PCM_16')
            rows.append(dict(file=path.name,species=sp,event=event,variant=v,seconds=n/RATE,
                peak=float(np.max(np.abs(x))),rms=float(np.sqrt(np.mean(x*x))),
                sha256=hashlib.sha256(path.read_bytes()).hexdigest()))
(OUT/'manifest.json').write_text(json.dumps(rows,indent=2))
print(f'Created {len(rows)} clips; max peak {max(r["peak"] for r in rows):.3f}')
