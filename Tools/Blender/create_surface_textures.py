"""Generate original seamless micro-surface textures, not sourced art."""
from pathlib import Path
import numpy as np
from PIL import Image
ROOT=Path(__file__).resolve().parents[2]/'Assets/Export/Textures';ROOT.mkdir(parents=True,exist_ok=True)
rng=np.random.default_rng(341)
N=512
def cloud(size):
    a=rng.random((size,size))*255
    return np.asarray(Image.fromarray(a.astype('uint8')).resize((N,N),Image.Resampling.BICUBIC)).astype(float)/255

def save(name,height,strength):
    h=np.clip(height,0,1)
    Image.fromarray((h*255).astype('uint8')).save(ROOT/(name+'_Height.png'))
    dx=(np.roll(h,-1,1)-np.roll(h,1,1))*strength;dy=(np.roll(h,-1,0)-np.roll(h,1,0))*strength
    normal=np.stack([-dx,-dy,np.ones_like(h)],-1);normal/=np.linalg.norm(normal,axis=-1,keepdims=True)
    Image.fromarray(((normal*.5+.5)*255).astype('uint8')).save(ROOT/(name+'_Normal.png'))
soil=.35*cloud(8)+.24*cloud(32)+.18*cloud(128)+.23*cloud(256)
save('Ground',soil,2.3)
y,x=np.mgrid[:N,:N]/N
best=np.ones((N,N))*100
for row in range(-1,18):
    for col in range(-1,18):
        xx=(col+.5*(row%2))/16;yy=row/16
        d=((x-xx)*16)**2+((y-yy)*16)**2
        best=np.minimum(best,d)
scales=np.clip(1-np.sqrt(best)*1.75,0,1)**.35
save('Scales',scales*.72+cloud(128)*.10,1.1)
print('Original texture tiles generated')
