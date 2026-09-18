import runtime_core as t,time,json,math,sys
rows=[];t.command('match',teams=False);t.command('ai',paused=True);t.command('sandbox',enabled=True);t.command('teleport',x=-30000,y=-30000)
for ident,name in [(1,'Trex'),(4,'Raptor'),(7,'Trike')]:
    t.command('ai',paused=True);t.command('travelAI',id=ident,startX=-1800,startY=9000,x=13200,y=9000)
    start=time.monotonic();samples=[]
    while time.monotonic()-start<65:
        a=next(a for a in t.state()['ai'] if a['id']==ident);samples.append(a)
        if math.hypot(a['x']-13200,a['y']-9000)<450:break
        time.sleep(.2)
    r=dict(species=name,passed=any(s['swimming'] and s['animation']=='Swim' for s in samples) and math.hypot(a['x']-13200,a['y']-9000)<450 and not a['swimming'],seconds=round(time.monotonic()-start,2),stuck=a['stuckRecoveries'],failedPaths=a['failedPaths'],swimSamples=sum(s['swimming'] for s in samples),end=[a['x'],a['y']])
    rows.append(r);(t.OUT/'ai-swimming.json').write_text(json.dumps(rows,indent=2));print(('PASS ' if r['passed'] else 'FAIL ')+json.dumps(r),flush=True)
t.command('ai',paused=True);t.command('sandbox',enabled=False);print('RESULT '+str(sum(r['passed'] for r in rows))+'/'+str(len(rows)),flush=True);sys.exit(any(not r['passed'] for r in rows))
