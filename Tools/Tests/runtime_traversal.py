"""Drive the real human pawn continuously between all six regions, for each species.
Only initial species setup teleports; route traversal uses W and camera-relative steering.
"""
import runtime_core as t,time,math,json,sys
selected=int(sys.argv[1]) if len(sys.argv)>1 else None
suffix='-retest-'+str(selected) if selected is not None else ''
rows=[];samples=[]
regions=[('Forest',-25000,17000),('Ridge',23000,19000),('Hunting',27000,-5000),('Creek',5000,-11500),('Grove',-15000,-23000),('Plains',0,0)]
regions=[(name,x*.5,y*.5) for name,x,y in regions]
def save():
    (t.OUT/('full-traversal'+suffix+'.json')).write_text(json.dumps(rows,indent=2));(t.OUT/('traversal-samples'+suffix+'.json')).write_text(json.dumps(samples,indent=2))
t.command('ai',paused=True);t.command('match',teams=False);t.command('ai',paused=True);t.command('sandbox',enabled=True);t.command('invulnerable',value=True);t.command('removeTarget')
for species,name in enumerate(['Trex','Raptor','Trike']):
    if selected is not None and species!=selected:continue
    t.command('species',value=species);time.sleep(.8)
    for region,x,y in regions:
        before=t.state();t.command('route',x=x,y=y);route=json.loads((t.BRIDGE/'route.json').read_text(encoding='utf-8-sig'))
        if not route['success']:rows.append(dict(species=name,region=region,passed=False,reason='No navigation route'));save();break
        start=time.monotonic();travel=0;last=before;progress=before;progress_time=start;sampletime=start;failure=None
        print('TRAVEL '+name+' -> '+region+' via '+str(len(route['points']))+' waypoints',flush=True)
        t.key('W')
        for point in route['points']:
            while True:
                s=t.state();distance=math.hypot(point['x']-s['x'],point['y']-s['y'])
                if distance<190:break
                now=time.monotonic();travel+=t.dist(last,s);last=s
                if t.dist(progress,s)>70:progress=s;progress_time=now
                if now-progress_time>4:failure='No movement progress for four seconds';break
                if s['z']<-1800:failure='Below terrain';break
                if now-start>180:failure='Route time exceeded 180 seconds';break
                yaw=math.degrees(math.atan2(point['y']-s['y'],point['x']-s['x']));t.command('camera',yaw=yaw)
                if now-sampletime>=1:
                    samples.append(dict(species=name,region=region,time=s['time'],x=s['x'],y=s['y'],z=s['z'],speed=s['speed'],falling=s['falling'],water=s['inWater'],keyW=s.get('keyW'),acceleration=s.get('acceleration'),floorActor=s.get('floorActor'),ignoreMove=s.get('ignoreMove')));sampletime=now;save()
                time.sleep(.08)
            if failure:break
        t.key('W','up');time.sleep(.4);end=t.state();goal=route['points'][-1];error=math.hypot(goal['x']-end['x'],goal['y']-end['y'])
        row=dict(species=name,region=region,passed=failure is None and error<450,reason=failure,seconds=round(time.monotonic()-start,2),travelMetres=round(travel/100,1),goalErrorCm=round(error,1),waypoints=len(route['points']))
        rows.append(row);save();print(('PASS ' if row['passed'] else 'FAIL ')+json.dumps(row),flush=True);t.command('screenshot')
        if failure:break
t.command('invulnerable',value=False);t.command('sandbox',enabled=False)
print('RESULT '+str(sum(x['passed'] for x in rows))+'/'+str(len(rows)),flush=True)
