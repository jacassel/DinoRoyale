"""A map derived from the same analytic terrain and region coordinates as the game."""
from pathlib import Path
import numpy as np
from PIL import Image,ImageDraw,ImageFilter
ROOT=Path(__file__).resolve().parents[2]
N=512;y,x=np.mgrid[:N,:N];X=(x/N-.5)*120000;Y=(.5-y/N)*120000
creek=-11500+2600*np.sin(X/12000)
H=240*np.sin(X/17000)*np.cos(Y/20000)+105*np.sin((X+Y)/9000)
H+=1900*np.exp(-((X-23500)/13500)**2-((Y-19500)/15500)**2)+550*np.exp(-((X+28000)/17000)**2-((Y-16000)/17000)**2)
H-=210*np.exp(-((Y-creek)/1250)**2)
edge=np.clip((np.maximum(abs(X),abs(Y))-51000)/8500,0,1);H+=edge**2*(2800+1000*np.sin(X/4700)*np.cos(Y/6600))
pond=np.sqrt(((X-6000)/5500)**2+((Y-9000)/3800)**2)
blend=np.clip((1.3-pond)/.3,0,1);blend=blend*blend*(3-2*blend);H=H*(1-blend)+(180-650*(1-pond**2))*blend
color=np.tile([.28,.36,.20],(N,N,1));forest=(X<-14000)&(Y>0);color[forest]=[.12,.25,.19]
ridge=(X>13000)&(Y>6000);color[ridge]=[.40,.30,.20]
grove=(Y<-15000)&(X<3000);color[grove]=[.30,.39,.21]
water=(abs(Y-creek)<650)|(pond<1);color[water]=[.17,.48,.51]
gradx,grady=np.gradient(H);shade=np.clip(.95+(gradx*.002-grady*.003),.60,1.20)
color*=shade[:,:,None];contour=(H.astype(int)%200)<12;color[contour]*=.83;color[edge>.8]*=.67
im=Image.fromarray((np.clip(color,0,1)*255).astype('uint8'));draw=ImageDraw.Draw(im)
for q in range(0,513,64):draw.line((q,0,q,512),fill=(60,76,53),width=1);draw.line((0,q,512,q),fill=(60,76,53),width=1)
draw.rectangle((12,12,500,500),outline=(175,164,115),width=2)
im.save(ROOT/'Assets/Export/UI/T_ValleyMap.png')
print('VALLEY_MAP_CREATED')
