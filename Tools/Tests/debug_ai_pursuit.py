import runtime_core as t,time,json
t.command('species',value=0);t.command('match',teams=False);t.command('sandbox',enabled=True);t.command('ai',paused=True);t.command('invulnerable',value=False);t.command('teleport',x=0,y=0);t.command('face',yaw=0)
for i in range(2,10):t.command('testAI',id=i,species=(i-1)//3,x=-24000+i*450,y=-23000,health=1,enabled=False,yaw=180)
t.command('testAI',id=1,species=1,x=1600,y=0,health=1,enabled=False,yaw=180);t.command('damage',value=t.state()['maxHealth']*.82);time.sleep(.6);t.command('enableAI',id=1,enabled=True)
samples=[]
for _ in range(45):
    s=t.state();a=next(a for a in s['ai'] if a['id']==1);samples.append(dict(player={k:s[k] for k in ['x','y','health','dead']},ai=a));print({k:a[k] for k in ['x','y','speed','target','state','decision','leader']},flush=True);time.sleep(.12)
(t.OUT/'pursuit-diagnostic.json').write_text(json.dumps(samples,indent=2));t.command('ai',paused=True)
