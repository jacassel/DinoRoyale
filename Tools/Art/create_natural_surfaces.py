"""Deterministic, seamless original ground/rock/bark/water textures."""
from pathlib import Path
import numpy as np
from scipy.ndimage import gaussian_filter, zoom
from PIL import Image

ROOT=Path(__file__).resolve().parents[2]/'Assets/Export/NaturalSurfaces'
ROOT.mkdir(parents=True,exist_ok=True)
N=1024;rng=np.random.default_rng(208)
y,x=np.mgrid[:N,:N]/N

def cloud(sigma):
    a=gaussian_filter(rng.random((N,N)),sigma,mode='wrap')
    return (a-a.min())/(a.max()-a.min())

def save(name,height,color,strength=5,rough=None):
    h=np.clip(height,0,1)
    dx=(np.roll(h,-1,1)-np.roll(h,1,1))*strength
    dy=(np.roll(h,-1,0)-np.roll(h,1,0))*strength
    n=np.stack([-dx,dy,np.ones_like(h)],axis=-1);n/=np.linalg.norm(n,axis=-1,keepdims=True)
    Image.fromarray(np.uint8(np.clip(color,0,1)*255)).save(ROOT/(name+'_BaseColor.png'))
    Image.fromarray(np.uint8((n*.5+.5)*255)).save(ROOT/(name+'_Normal.png'))
    mask=np.stack([h,rough if rough is not None else .65+h*.3,cloud(2)],axis=-1)
    Image.fromarray(np.uint8(np.clip(mask,0,1)*255)).save(ROOT/(name+'_Masks.png'))

macro=cloud(50);grain=cloud(1);mid=cloud(8)
soil=.38*mid+.4*grain+.22*macro
col=np.array([.25,.215,.158])[None,None,:]*(.60+.60*soil[...,None])
flecks=np.clip((grain-.63)*6,0,1)
col=col*(1-flecks[...,None]*.18)+np.array([.22,.20,.15])*flecks[...,None]*.18
save('Soil',soil,col,4)
strata=.5+.5*np.sin(x*2*np.pi*7+np.sin(y*2*np.pi*4)*.8+mid*3)
rock=.33*mid+.32*grain+.35*strata
col=np.array([.30,.285,.255])[None,None,:]*(.62+.55*rock[...,None])
lichen=np.clip((macro-.58)*3,0,.6)[...,None]
col=col*(1-lichen)+np.array([.225,.237,.155])*lichen
save('Rock',rock,col,6)
bark=.5+.5*np.sin(x*2*np.pi*33+np.sin(y*2*np.pi*2)*.8+mid*3)
bark=np.clip(bark*.45+grain*.30+mid*.25,0,1)
save('Bark',bark,np.array([.22,.16,.105])*(.42+.72*bark[...,None]),5)
# Multiple incommensurate-looking (but exactly tiling) crossing ripples.
water=np.zeros((N,N))+.5
for kx,ky,amp,phase in [(5,8,.10,1),(13,-3,.052,2),(22,11,.018,.4),(3,-9,.085,0),(35,7,.015,3)]:
    water+=amp*np.sin((x*kx+y*ky)*2*np.pi+phase)
save('Ripples',water,np.ones((N,N,3))*.2,8)
print('NATURAL_SURFACES_CREATED',flush=True)
