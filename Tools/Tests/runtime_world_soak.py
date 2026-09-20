"""Three bounded autonomous world scenarios, reporting behavior and persistent stalls."""
import runtime_core as t,time,json,sys,collections,math
rows=[]
t.command('removeTarget');t.command('clearTestFood');t.command('invulnerable',value=False)
for run in ([int(sys.argv[1])] if len(sys.argv)>1 else range(3)):
    t.command('menu',open=False);t.command('species',value=run);t.command('match',teams=run==1);t.command('sandbox',enabled=True);t.command('ai',paused=False)
    if run==2:
        for i in range(10):t.command('hunger',id=i,value=38)
    s=t.state();start=s['time'];base={a['id']:a for a in s['ai']};last={};stalls=collections.Counter();maxstalls=collections.Counter();states=collections.Counter();samples=[];deadline=time.monotonic()+480
    respawn_times={};first_hit_delays=[];split_samples=collections.Counter();max_split=0
    while s['time']-start<240 and time.monotonic()<deadline:
        for a in s['ai']:
            if not a['major']:continue
            k=a['id'];states[a['state']]+=1
            leader=s if a['leader']==0 else next((v for v in s['ai'] if v['id']==a['leader']),None)
            if a['species']==1 and leader and a['leader']!=k and not a['dead'] and not leader['dead'] and t.dist(a,leader)>8000:split_samples[k]+=1
            else:split_samples[k]=0
            max_split=max(max_split,split_samples[k])
            if k in last and last[k]['dead'] and not a['dead']:respawn_times[k]=s['time']
            if k in respawn_times and a['health']<a['maxHealth']:
                first_hit_delays.append(s['time']-respawn_times.pop(k))
            if k in last and not a['dead'] and a['state'] in ['Pursuing','Regrouping','Roaming','Seeking food','Retreating'] and t.dist(a,last[k])<50:stalls[k]+=1
            else:stalls[k]=0
            maxstalls[k]=max(maxstalls[k],stalls[k]);last[k]=a
        samples.append(dict(time=s['time']-start,ai=s['ai'],corpses=len(s.get('corpses',[])),playerDead=s['dead'],deaths=s['deaths']))
        time.sleep(1);s=t.state()
    major=[a for a in s['ai'] if a['major']]
    row=dict(run=run,teams=run==1,seconds=s['time']-start,states=dict(states),hits=sum(a['hits']-base[a['id']]['hits'] for a in major),
        failedPaths=sum(a['failedPaths']-base[a['id']]['failedPaths'] for a in major),maxStallSamples=max(maxstalls.values(),default=0),
        belowTerrain=any(a['z']<a['ground']-150 for q in samples for a in q['ai'] if not a['dead']),
        maxCorpses=max(q['corpses'] for q in samples),meanFPS=s['meanFPS'],playerDeaths=s['deaths'],
        hungriest=min(a['hunger'] for a in major),stuckRecoveries=sum(a['stuckRecoveries']-base[a['id']]['stuckRecoveries'] for a in major),
        respawnFirstHitDelays=first_hit_delays,immediateRespawnHits=sum(v<2 for v in first_hit_delays),maxPackSplitSamples=max_split)
    row['passed']=row['seconds']>=239 and row['hits']>10 and not row['belowTerrain'] and row['maxStallSamples']<30 and row['failedPaths']==0
    rows.append(row);(t.OUT/'world-soak.json').write_text(json.dumps(rows,indent=2));(t.OUT/f'world-soak-{run}-samples.json').write_text(json.dumps(samples,indent=2));print(json.dumps(row),flush=True)
t.command('ai',paused=True);t.command('menu',open=True)
sys.exit(any(not r['passed'] for r in rows))
