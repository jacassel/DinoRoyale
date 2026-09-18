"""Play two rendered matches with real player inputs while nine independent AI run."""
import runtime_core as t,time,math,json
reports=[]
for teams,species in [(False,0),(True,1)]:
    t.command('species',value=species);t.command('match',teams=teams);t.command('sandbox',enabled=False);t.command('invulnerable',value=False);t.command('removeTarget');t.command('ai',paused=False)
    start=time.monotonic();start_frames=t.state().get('frameCount',0);samples=[];last_sample=0;last_attack=0;charging=False;charge_started=0;walking=False
    while time.monotonic()-start<300:
        s=t.state();now=time.monotonic()
        if now-last_sample>1:
            samples.append(s);last_sample=now
            if len(samples)%30==0:print(('TEAM' if teams else 'SOLO')+' '+str(round(now-start))+'s score '+str(s['kills'])+' teams '+str(s['team0Kills'])+':'+str(s['team1Kills']),flush=True)
            (t.OUT/('team-match-samples.json' if teams else 'solo-match-samples.json')).write_text(json.dumps(samples,indent=2))
        if s['roundOver']:break
        enemies=[a for a in s['ai'] if a['major'] and not a['dead'] and (a['team']==1 if teams else not (species==1 and a['species']==1))]
        if s['dead'] or not enemies:
            if walking:t.key('W','up');walking=False
            if charging:t.key('RightMouseButton','up');charging=False
            time.sleep(.2);continue
        enemy=min(enemies,key=lambda a:t.dist(a,s)*(1 if a.get('scoringTarget') else 1.3))
        distance=t.dist(enemy,s);yaw=math.degrees(math.atan2(enemy['y']-s['y'],enemy['x']-s['x']));t.command('camera',yaw=yaw)
        gap=abs((yaw-s['yaw']+180)%360-180);reach=620 if species==0 else 280
        move=distance>reach*.65 or gap>15
        if move!=walking:t.key('W','down' if move else 'up');walking=move
        if charging and (now-charge_started>1.55 or s['health']/s['maxHealth']<.25):t.key('RightMouseButton','up');charging=False;last_attack=now
        if not charging and distance<reach*.92 and gap<25 and now-last_attack>1.0:
            if s['health']/s['maxHealth']>=.25 and int(now-start)%5<2:t.key('RightMouseButton');charging=True;charge_started=now
            else:t.key('LeftMouseButton');t.key('LeftMouseButton','up');last_attack=now
        time.sleep(.10)
    t.key('W','up');t.key('RightMouseButton','up');s=t.state();samples.append(s);t.command('screenshot')
    major=[a for a in s['ai'] if a['major']]
    report=dict(mode='team' if teams else 'solo',seconds=round(time.monotonic()-start,2),roundOver=s['roundOver'],winner=s['winner'],teamScore=[s['team0Kills'],s['team1Kills']],humanScore=[s['kills'],s['deaths'],s['assists']],majorAI=len(major),hitsBySpecies={str(i):sum(a['hits'] for a in major if a['species']==i) for i in range(3)},failedPaths=sum(a['failedPaths'] for a in major),maxStuckRecoveries=max(a['stuckRecoveries'] for a in major),fellBelowTerrain=any(a['z']<a['ground']-120 for q in samples for a in q['ai'] if a['major'] and not a['dead']),meanFPS=s['meanFPS'],states=sorted({a['state'] for q in samples for a in q['ai']}))
    report['observedFPS']=(s.get('frameCount',start_frames)-start_frames)/max(.1,time.monotonic()-start)
    report['guardsUsed']=sum(a.get('guards',0) for a in major);report['retreatDecisions']=sum(a.get('retreats',0) for a in major)
    reports.append(report);(t.OUT/'played-rounds.json').write_text(json.dumps(reports,indent=2));print(json.dumps(report),flush=True)
t.command('ai',paused=True)
