"""Requested roster, hunger and damage changes verified in the live engine."""
import runtime_core as t,time,json,sys
rows=[]
def wait(sec):
    start=t.state()['time']
    while t.state()['time']-start<sec:time.sleep(.025)
def check(name,ok,**data):
    rows.append(dict(test=name,passed=bool(ok),**data));print(rows[-1],flush=True)
    (t.OUT/'sprint-rules.json').write_text(json.dumps(rows,indent=2))
def press(k):t.key(k);t.key(k,'up')
t.command('menu',open=False)
for teams in [False,True]:
    for species in range(3):
        t.command('species',value=species);t.command('match',teams=teams);t.command('ai',paused=True);wait(.4);s=t.state()
        for team in ([0,1] if teams else [-1]):
            members=[a for a in s['ai'] if a['major'] and a['species']==1 and a['team']==team]
            human=species==1 and team==(-1 if not teams else 0)
            check(f'pack size teams={teams} species={species} team={team}',len(members)+human==3,count=len(members)+human)
            check(f'one scoring leader teams={teams} species={species} team={team}',sum(a['scoringTarget'] for a in members)+human==1)
        check(f'ten participants {teams} {species}',s['majorCount']==10)
t.command('match',teams=False);t.command('ai',paused=True);t.command('sandbox',enabled=True)
for sp,health,quick,heavy,drain in [(0,1500,187,673.2,.075),(1,520,66.6,193.14,.1125),(2,1650,135,445.5,.06)]:
    t.command('species',value=sp);t.command('face',yaw=0);wait(.6)
    check(f'health {sp}',t.state()['maxHealth']==health)
    a=t.state();wait(3);b=t.state();rate=(a['hunger']-b['hunger'])/(b['time']-a['time']);check(f'hunger drain {sp}',abs(rate-drain)<.003,rate=rate)
    t.command('target');wait(.3);a=t.state();press('LeftMouseButton');wait(.4);b=t.state();damage=a['targetHealth']-b['targetHealth'];check(f'quick damage {sp}',abs(damage-quick)<.1,damage=damage);wait(1.5)
    t.command('target');a=t.state();t.key('RightMouseButton');wait(1.5);t.key('RightMouseButton','up');wait(.5);b=t.state();damage=a['targetHealth']-b['targetHealth'];check(f'heavy damage {sp}',abs(damage-heavy)<.1,damage=damage);wait(2)
t.command('removeTarget');t.command('menu',open=True)
sys.exit(any(not r['passed'] for r in rows))
