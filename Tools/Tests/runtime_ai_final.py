import runtime_core as t,time,json,math
samples=[];start=time.monotonic()
t.command('species',value=1);t.command('ai',paused=False)
while time.monotonic()-start<90:
    s=t.state();samples.append(s)
    (t.OUT/'ai-final-retest.json').write_text(json.dumps(samples,indent=2))
    if len(samples)%15==0:print('AI '+str(len(samples))+' seconds '+str(sum(x['hits'] for x in s['ai'] if x['major']))+' hits',flush=True)
    time.sleep(1)
s=t.state();major=[x for x in s['ai'] if x['major']]
report={'durationSeconds':time.monotonic()-start,'majorAI':len(major),'majorCombatants':s['majorCount'],'meanFPS':s['meanFPS'],
'hitsBySpecies':{str(i):sum(x['hits'] for x in major if x['species']==i) for i in range(3)},
'allMoved':all(x['distance']>1000 for x in major),'maxStuckRecoveries':max(x['stuckRecoveries'] for x in major),
'failedPaths':sum(x['failedPaths'] for x in major),'fellUnderTerrain':any(x['z']<x['ground']-100 for q in samples for x in q['ai'] if x['major']),
'preyFled':any(x['state']=='Fleeing' for q in samples for x in q['ai'] if not x['major']),
'raptors':[x for x in major if x['species']==1]}
(t.OUT/'ai-final-summary.json').write_text(json.dumps(report,indent=2));print(json.dumps(report),flush=True)
t.command('screenshot')
