"""Bounded real-AI duels, no scripted attack choice or resource overrides."""
import runtime_core as t,time,json,sys,collections,math
n=int(sys.argv[1]) if len(sys.argv)>1 else 8
label=sys.argv[2] if len(sys.argv)>2 else 'initial'
opponent=int(sys.argv[3]) if len(sys.argv)>3 else 0
pack=not(len(sys.argv)>4 and sys.argv[4] in ['single','rex'])
challenger=0 if len(sys.argv)>4 and sys.argv[4]=='rex' else 1
seed_start=int(sys.argv[5]) if len(sys.argv)>5 else 103
rows=[]
for trial in range(n):
    t.command('menu',open=False)
    t.command('duelSetup',seed=seed_start+trial,angle=trial*137.5,opponent=opponent,pack=pack)
    if challenger==0:
        angle=trial*137.5
        t.command('testAI',id=2,species=0,x=650*math.cos(math.radians(angle)),y=650*math.sin(math.radians(angle)),health=1,enabled=True,yaw=angle+180,personality=(seed_start+trial+2)%4)
    s=t.state();start=s['time'];base={a['id']:a for a in s['ai']};samples=[];deadline=time.monotonic()+180
    first_score=None;score_time=None
    while s['time']-start<90 and time.monotonic()<deadline:
        a={a['id']:a for a in s['ai'] if a['id'] in range(1,5 if pack else 3)}
        samples.append({'time':s['time']-start,'ai':list(a.values())})
        if first_score is None and (a[1]['dead'] or a[2]['dead']):
            first_score='trade' if a[1]['dead'] and a[2]['dead'] else ('raptors' if challenger==1 else 'trex') if a[1]['dead'] else 'opponent'
            score_time=s['time']-start
        if a[1]['dead'] or all(v['dead'] for k,v in a.items() if k!=1) or (score_time is not None and s['time']-start-score_time>=20):break
        time.sleep(.35);s=t.state()
    winner=first_score or 'timeout'
    row=dict(trial=trial,seed=seed_start+trial,opponent=opponent,challenger=challenger,pack=pack,winner=winner,seconds=s['time']-start,scoreSeconds=score_time,
        winCondition='first scoring death (raptor leader or opposing dinosaur)',counterkill=winner=='opponent' and a[1]['dead'],packWiped=all(v['dead'] for k,v in a.items() if k!=1),
        survivors=[dict(id=k,health=v['health'],hits=v['hits']-base[k]['hits'],stuck=v['stuckRecoveries']-base[k]['stuckRecoveries'],paths=v['failedPaths']-base[k]['failedPaths']) for k,v in a.items()],
        states=sorted({v['state'] for q in samples for v in q['ai']}),
        valid=all(0<=v['stamina']<=100.01 and v['z']>=v['ground']-200 for q in samples for v in q['ai']))
    rows.append(row);(t.OUT/f'balance-{label}.json').write_text(json.dumps(rows,indent=2))
    (t.OUT/f'balance-{label}-last-samples.json').write_text(json.dumps(samples,indent=2))
    print(json.dumps(row),flush=True)
t.command('ai',paused=True);t.command('menu',open=True)
print(dict(collections.Counter(r['winner'] for r in rows)),flush=True)
