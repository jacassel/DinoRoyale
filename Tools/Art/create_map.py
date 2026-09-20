"""A map derived from the same analytic terrain and region coordinates as the game."""
from pathlib import Path
import numpy as np
from PIL import Image,ImageDraw,ImageFilter
ROOT=Path(__file__).resolve().parents[2]
N=512;y,x=np.mgrid[:N,:N];X=(x/N-.5)*60000;Y=(.5-y/N)*60000
creek=-5750+1300*np.sin(X/6000)
H=120*np.sin(X/8500)*np.cos(Y/10000)+52.5*np.sin((X+Y)/4500)
H+=950*np.exp(-((X-11750)/6750)**2-((Y-9750)/7750)**2)+275*np.exp(-((X+14000)/8500)**2-((Y-8000)/8500)**2)
H-=210*np.exp(-((Y-creek)/1250)**2)
edge=np.clip((np.maximum(abs(X),abs(Y))-25500)/4250,0,1);H+=edge**2*(1400+500*np.sin(X/2350)*np.cos(Y/3300))
pond=np.sqrt(((X-3000)/2750)**2+((Y-4500)/1900)**2)
blend=np.clip((1.3-pond)/.3,0,1);blend=blend*blend*(3-2*blend);H=H*(1-blend)+(180-650*(1-pond**2))*blend
color=np.tile([.28,.36,.20],(N,N,1));forest=(X<-7000)&(Y>0);color[forest]=[.12,.25,.19]
ridge=(X>6500)&(Y>3000);color[ridge]=[.40,.30,.20]
grove=(Y<-7500)&(X<1500);color[grove]=[.30,.39,.21]
water=(abs(Y-creek)<650)|(pond<1);color[water]=[.17,.48,.51]
gradx,grady=np.gradient(H);shade=np.clip(.95+(gradx*.002-grady*.003),.60,1.20)
color*=shade[:,:,None];contour=(H.astype(int)%200)<12;color[contour]*=.83;color[edge>.8]*=.67
im=Image.fromarray((np.clip(color,0,1)*255).astype('uint8'));draw=ImageDraw.Draw(im)
for q in range(0,513,64):draw.line((q,0,q,512),fill=(60,76,53),width=1);draw.line((0,q,512,q),fill=(60,76,53),width=1)
draw.rectangle((12,12,500,500),outline=(175,164,115),width=2)
im.save(ROOT/'Assets/Export/UI/T_ValleyMap.png')
print('VALLEY_MAP_CREATED')
