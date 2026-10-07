"""Seeded launch-roster AI matches using actual movement, combat and resource systems.

One-on-one comparisons and natural three-member packs are separate experiments.
Time-outs are reported, not counted as wins. Health is never reset during a match.
"""
import argparse,itertools,time,json,collections,math
from net_harness import NetworkTest,Peer
p=argparse.ArgumentParser();p.add_argument('--rounds',type=int,default=2);p.add_argument('--seconds',type=int,default=60);p.add_argument('--kind',choices=['solo','packs','all'],default='all');p.add_argument('--output',default='Tests/Results/roster05/balance-initial');p.add_argument('--executable');p.add_argument('--rendered',action='store_true');p.add_argument('--resume',action='store_true');p.add_argument('--focus',type=int);p.add_argument('--anky-armor',type=float);a=p.parse_args()
t=NetworkTest(a.output,executable=a.executable,rendered=a.rendered);t.render_size=(1280,720)
if a.anky_armor is not None:t.extra_args=[f'-ini:Game:[Dino.Anky]:ArmorMultiplier={a.anky_armor}']
species=[0,1,2,4,5,6,7];jobs=[]
for x,y in itertools.combinations(species,2):
 if a.focus is not None and a.focus not in [x,y]:continue
 if a.kind in ['solo','all']:jobs.append((x,y,False,False))
 if a.kind in ['packs','all'] and (x in [1,6] or y in [1,6]):jobs.append((x,y,x in [1,6],y in [1,6]))
if a.focus==7 and a.kind in ['solo','all']:jobs.append((7,7,False,False))
# Exercise large-body contact and combo-reset regressions first, then the remaining matrix.
jobs.sort(key=lambda job: (0 if job[:2] in [(2,5),(5,6),(4,5),(0,4),(2,4)] else 1,job))
path=t.out/'matches.json';rows=json.loads(path.read_text()) if a.resume and path.exists() else []
try:
 h=Peer(t,'RosterBalance','/Game/Maps/LostValley');h.command('menu',open=False)
 for trial in range(a.rounds):
  for x,y,px,py in jobs:
   key=f'{x}-{y}-{px}-{py}-{trial}'
   if any(r['key']==key for r in rows):continue
   seed=5500+trial*101+x*17+y;h.command('duelSetup',seed=seed,angle=(trial*137+x*23+y*11)%360,opponent=x,challenger=y,opponentPack=px,pack=py)
   s=h.state();start=s['time'];ids=[1,2]+([5,6] if px else [])+([3,4] if py else []);base={v['id']:v for v in s['ai'] if v['id'] in ids};samples=[];deadline=time.monotonic()+a.seconds*2+20
   winner=None
   while s['time']-start<a.seconds and time.monotonic()<deadline:
    actors={v['id']:v for v in s['ai'] if v['id'] in ids}
    samples.append(dict(time=s['time']-start,actors=[{k:v[k] for k in ['id','species','health','maxHealth','stamina','x','y','z','ground','state','hits','attacks','dead','stuckRecoveries','failedPaths','target','leader','fightConfidence']} for v in actors.values()]))
    if actors[1]['dead'] or actors[2]['dead']:
     winner='trade' if actors[1]['dead'] and actors[2]['dead'] else str(y if actors[1]['dead'] else x);break
    time.sleep(.45);s=h.state()
   row=dict(key=key,seed=seed,opponent=x,challenger=y,opponentPack=px,challengerPack=py,trial=trial,winner=winner or 'timeout',seconds=round(s['time']-start,2),valid=all(v['health']>=0 and v['stamina']>=0 and math.isfinite(v['x']) and v['z']>v['ground']-350 for snap in samples for v in snap['actors']),survivors=[dict(id=i,species=v['species'],health=round(v['health'],1),hits=v['hits']-base[i]['hits'],attacks=v['attacks']-base[i]['attacks'],stuck=v['stuckRecoveries']-base[i]['stuckRecoveries'],failedPaths=v['failedPaths']-base[i]['failedPaths']) for i,v in actors.items()],states=sorted({v['state'] for q in samples for v in q['actors']}))
   rows.append(row);path.write_text(json.dumps(rows,indent=2));(t.out/(key+'.json')).write_text(json.dumps(samples,separators=(',',':')));print(json.dumps(row),flush=True)
 t.check('all seeded bouts maintain valid health resources and grounding',all(r['valid'] for r in rows),matches=len(rows))
 print('OUTCOMES',dict(collections.Counter(r['winner'] for r in rows)),flush=True)
finally:t.close()
