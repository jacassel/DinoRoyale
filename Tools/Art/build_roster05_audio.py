"""Distinct 0.5 creature designs using the project's already-credited CC0 recordings.

Anky: dry paired grunts, armored rattles and club cracks.
Brachi: layered resonant low calls, long air releases and heavy ground transients.
Pachy: short nasal barks, higher breath and compact headbutt knocks.
No original dinosaur assets are rewritten.
"""
from pathlib import Path
import numpy as np
from scipy import signal
import soundfile as sf
import hashlib,json
ROOT=Path(__file__).resolve().parents[2];OUT=ROOT/'Assets/Audio/Designed/Roster05';OUT.mkdir(parents=True,exist_ok=True)
SRC=ROOT/'Assets/Audio/Source/80-cc0-creture-sfx-2';RATE=44100
events=['Step','Quick','Heavy','Impact','Death','Charge','Hurt','SprintBreath','InjuredBreath']
cache={}
def recording(file,n,rate,offset):
 p=SRC/file
 if p not in cache:
  x,sr=sf.read(p);x=x.mean(axis=1) if x.ndim>1 else x
  x=signal.resample_poly(x,RATE,sr);cache[p]=x/max(.001,np.max(np.abs(x)))
 x=cache[p];start=min(int(offset*RATE),max(0,len(x)-int(n*rate)))
 return np.interp(np.arange(n)*rate+start,np.arange(len(x)),x,left=0,right=0)
rows=[]
for sp,(name,base,grit,pitch,lengths) in enumerate([
 ('Anky','grunt_07.ogg','grunt_09.ogg',.72,[.34,.63,1.24,.40,2.4,1.02,.69,.91,1.18]),
 ('Brachi','roar_06.ogg','breath_02.ogg',.48,[.55,1.03,1.80,.58,3.4,1.42,1.15,1.48,1.62]),
 ('Pachy','roar_05.ogg','die_03.ogg',1.43,[.23,.39,.87,.27,1.74,.67,.47,.57,.79])]):
 for kind,event in enumerate(events):
  for variant in range(3):
   rng=np.random.default_rng(51000+sp*100+kind*7+variant);duration=lengths[kind];n=int(RATE*duration);t=np.arange(n)/RATE;u=t/duration
   p=pitch*(.96+.04*variant);white=rng.normal(size=n)
   air=signal.sosfilt(signal.butter(3,[120 if sp==1 else 300,3200 if sp!=2 else 5800],btype='bandpass',fs=RATE,output='sos'),white)
   if kind in [0,3]:
    x=.54*recording('stomp_01.ogg',n,[.73,.43,1.45][sp],variant*.04)
    x+=.17*np.sin(2*np.pi*([68,34,135][sp]*t+12*t*t))*np.exp(-t*[20,10,30][sp])
    x+=air*(.11 if kind==3 else .05)*np.exp(-t*28)
    if kind==3 and sp==0:x+=.2*np.sin(2*np.pi*370*t)*np.exp(-t*43)
   elif kind>=7:
    x=.22*air*np.sin(np.pi*u)**1.6+.30*recording('breath_02.ogg',n,p*.8,variant*.09)
    if kind==8:x+=.21*recording(grit,n,p,variant*.08)*(1-u)
   else:
    x=.56*recording(base,n,p,variant*.10)+.25*recording(grit,n,p*1.21,variant*.07)
    if sp==0:x*=.67+.33*np.sin(2*np.pi*(7+variant)*t)**2
    elif sp==1:
     x+=.24*recording('roar_06.ogg',n,p*.71,(variant+1)*.11)
     delay=int(.07*RATE);x[delay:]+=.13*x[:-delay]
    else:x*=.60+.40*np.sin(np.pi*np.minimum(1,u*2))**2
    x+=.025*air*np.sin(np.pi*u)
    if kind==5:x*=.3+.7*u
    if kind==4:x*=np.exp(-1.9*u)
   x=signal.sosfilt(signal.butter(2,[37,6200 if sp==2 else 4300],btype='bandpass',fs=RATE,output='sos'),x)
   envelope=np.minimum(1,t/(.007 if kind in [0,3] else .03))*np.minimum(1,(duration-t)/.10);x*=envelope
   rms=np.sqrt(np.mean(x*x));x*=min((.14 if kind in [1,2,4,5,6] else .09)/max(rms,1e-8),.68/max(np.max(np.abs(x)),1e-8));x[0]=x[-1]=0
   file=OUT/f'{name}_{event}_{variant}.wav';sf.write(file,x,RATE,subtype='PCM_16')
   rows.append(dict(file=file.name,seconds=n/RATE,peak=float(abs(x).max()),rms=float(np.sqrt(np.mean(x*x))),sha256=hashlib.sha256(file.read_bytes()).hexdigest()))
(OUT/'manifest.json').write_text(json.dumps(rows,indent=2));print('ROSTER05_AUDIO_CREATED',len(rows))
