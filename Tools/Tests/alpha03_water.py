"""Repeat network pond crossings from eight headings using real movement input."""
import argparse,time,math,json
from net_harness import NetworkTest,Peer,host_url,wait_for,distance
p=argparse.ArgumentParser();p.add_argument('--output',default='Tests/Results/alpha03/water');p.add_argument('--executable');p.add_argument('--lag',type=int,default=0);a=p.parse_args()
t=NetworkTest(a.output,executable=a.executable,lag=a.lag);samples=[]
try:
    h=Peer(t,'WaterHost',host_url(2));c=Peer(t,'WaterClient');c.lobby(2,1);h.lobby(6);h.command('ai',paused=True);h.command('sandbox',enabled=True)
    for sp in range(3):
        for angle in range(0,360,45):
            rad=math.radians(angle);dx,dy=math.cos(rad),math.sin(rad)
            h.command('testAI',id=1,species=sp,x=3000,y=4500,yaw=angle,health=1,enabled=False)
            h.command('ai',paused=True);c.command('face',yaw=angle)
            t.check(f'{sp} pond center swimming {angle}',wait_for(lambda:c.state()['swimming'],5))
            c.key('W');start=time.monotonic();visited=[]
            while time.monotonic()-start<17:
                s=c.state();visited.append({k:s[k] for k in ['time','x','y','z','swimming','falling','speed','animation']})
                if not s['swimming'] and not s['falling'] and math.hypot((s['x']-3000)/2750,(s['y']-4500)/1900)>1.08:break
                time.sleep(.12)
            c.key('W','up');s=c.state()
            samples.append(dict(species=sp,angle=angle,samples=visited));(t.out/'crossings.json').write_text(json.dumps(samples,indent=2))
            t.check(f'{sp} exits shoreline {angle}',not s['swimming'] and not s['falling'] and math.hypot((s['x']-3000)/2750,(s['y']-4500)/1900)>1.05,x=s['x'],y=s['y'],z=s['z'])
            c.command('camera',yaw=angle+180);c.key('W');begun=time.monotonic()
            while time.monotonic()-begun<12 and not c.state()['swimming']:time.sleep(.1)
            c.key('W','up');t.check(f'{sp} re-enters pond {angle}',c.state()['swimming'])
            c.key('Q');time.sleep(.3);s=c.state();c.key('Q','up');t.check(f'{sp} pivots while swimming {angle}',s['pivot']!=0 and s['animation']=='Swim')
finally:t.close()
