"""Observe nine autonomous combatants with the final resources and real lighting."""
import runtime_core as t,time,json,sys
reports=[]
for teams in [False,True]:
 t.command('menu',open=False);t.command('clearTestFood');t.command('match',teams=teams);t.command('species',value=1);t.command('sandbox',enabled=True);t.command('removeTarget');t.command('invulnerable',value=False);t.command('ai',paused=False)
 # Exercise food decisions without changing the world or granting abilities.
 t.command('hunger',id=1,value=35);t.command('hunger',id=8,value=35)
 a=t.state();start=time.monotonic();samples=[]
 while time.monotonic()-start<90:
  samples.append(t.state());time.sleep(.75)
 s=t.state();maj=[x for x in s['ai'] if x['major']];before={x['id']:x for x in a['ai'] if x['major']}
 r=dict(mode='team' if teams else 'solo',seconds=time.monotonic()-start,majorAI=len(maj),majorCount=s['majorCount'],observedFPS=(s['frameCount']-a['frameCount'])/(time.monotonic()-start),hits=sum(x['hits']-before[x['id']]['hits'] for x in maj),hitsBySpecies={sp:sum(x['hits']-before[x['id']]['hits'] for x in maj if x['species']==sp) for sp in range(3)},states=sorted({x['state'] for q in samples for x in q['ai'] if x['major']}),failedPaths=sum(x['failedPaths']-before[x['id']]['failedPaths'] for x in maj),maxStuckRecoveries=max(x['stuckRecoveries']-before[x['id']]['stuckRecoveries'] for x in maj),allMoved=all(x['distance']-before[x['id']]['distance']>500 for x in maj),fellBelowTerrain=any(x['z']<x['ground']-120 for q in samples for x in q['ai'] if x['major'] and not x['dead']),resourceBounds=all(0<=x['hunger']<=100.01 and 0<=x['stamina']<=100.01 and x['combo']<=3 for q in samples for x in q['ai']),corpses=len(s['corpses']),revealedSamples=sum(x['mapVisible'] for q in samples for x in q['ai'] if x['major']),hungryDinosFinal={x['id']:x['hunger'] for x in maj if x['id'] in [1,8]})
 r['passed']=r['majorAI']==9 and r['majorCount']==10 and r['hits']>10 and not r['failedPaths'] and not r['fellBelowTerrain'] and r['resourceBounds']
 reports.append(r);(t.OUT/'final-combat-world.json').write_text(json.dumps(reports,indent=2));print(json.dumps(r),flush=True);t.command('screenshot')
t.command('ai',paused=True);t.command('sandbox',enabled=False);t.command('menu',open=True);sys.exit(any(not r['passed'] for r in reports))
