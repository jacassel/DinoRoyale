"""Lay out existing runtime captures for visual inspection; no game operations."""
from PIL import Image,ImageDraw
from pathlib import Path
root=Path(__file__).resolve().parents[2]/'Tests/Results/visual-modernization'
out=root/'review';out.mkdir(exist_ok=True)
groups=[['close-front','close-side','forest','river','pond','ridge'],
        ['grove','swim','walk','sprint','turn','quick'],
        ['charge','heavy','eat','injured','death']]
for species in ['rex','raptor','trike']:
    for i,names in enumerate(groups):
        canvas=Image.new('RGB',(1600,3*475),(18,22,25));draw=ImageDraw.Draw(canvas)
        for j,name in enumerate(names):
            im=Image.open(root/'details'/f'{species}-{name}.png').convert('RGB');im.thumbnail((800,450))
            x=(j%2)*800;y=(j//2)*475
            canvas.paste(im,(x,y));draw.text((x+12,y+452),f'{species} / {name}',fill='white')
        canvas.save(out/f'{species}-{i+1}.jpg',quality=90)
