"""Additional live combat, feeding, animation and navigation checks."""
import runtime_core as t
import time,json,sys
from pathlib import Path
out=t.OUT/'integration-live.json'
results=[]
def check(name,ok,**data):
    r=dict(test=name,passed=bool(ok),**data);results.append(r);print(('PASS ' if ok else 'FAIL ')+name+' '+json.dumps(data),flush=True);out.write_text(json.dumps(results,indent=2))

t.command('ai',paused=True)
for species,name,mult in [(0,'Trex',.22),(1,'Raptor',.30),(2,'Trike',.10)]:
    t.command('species',value=species);t.command('face',yaw=0);t.command('target');time.sleep(.8)
    a=t.state();time.sleep(1);b=t.state();check(name+' stable ground',not b['falling'] and t.dist(a,b)<2,displacement=t.dist(a,b),z=b['z'])
    t.key('Q');time.sleep(.15);a=t.state();t.command('hitFromTarget',value=100,front=True);time.sleep(.1);b=t.state()
    check(name+' frontal shield reduction',abs((a['health']-b['health'])-100*mult)<1,damage=a['health']-b['health'])
    check(name+' brace animation',b['animation']=='Brace')
    a=b;t.command('hitFromTarget',value=100,front=False);time.sleep(.1);b=t.state();check(name+' rear bypasses shield',abs(a['health']-b['health']-100)<1)
    t.key('Q','up');time.sleep(.2);check(name+' brace release idle',t.state()['animation']=='Idle')
    t.command('removeTarget');t.command('species',value=species);t.command('food');t.command('damage',value=t.state()['maxHealth']*.6)
    time.sleep(.3);a=t.state();t.key('E');time.sleep(.4);b=t.state();check(name+' eat animation',b['eating'] and b['animation']=='Eat')
    time.sleep(1.3);b=t.state();check(name+' food healing',b['health']>a['health']+a['maxHealth']*.14,healed=b['health']-a['health'])
    t.key('E','up');time.sleep(.2);check(name+' release stops eating',not t.state()['eating'] and t.state()['animation']=='Idle')
    t.command('heal');t.key('RightMouseButton');time.sleep(.4);check(name+' charge animation',t.state()['animation']=='Charge')
    t.key('RightMouseButton','up');time.sleep(.15);check(name+' heavy animation',t.state()['animation']=='Heavy')
    time.sleep(2);check(name+' attack recovers animation',t.state()['animation']=='Idle')
    t.command('damage',value=100000);time.sleep(.3);check(name+' death animation',t.state()['dead'] and t.state()['animation']=='Death')
    time.sleep(6.3);check(name+' respawn animation reset',not t.state()['dead'] and t.state()['animation']=='Idle')
t.command('navAudit');data=json.loads((t.BRIDGE/'navigation-audit.json').read_text(encoding='utf-8-sig'))
(t.OUT/'navigation-audit.json').write_text(json.dumps(data,indent=2))
check('all 30 region routes',len(data['routes'])==30 and all(x['success'] for x in data['routes']),routes=len(data['routes']))
check('ten major combatants',t.state()['majorCount']==10,major=t.state()['majorCount'])
t.command('ai',paused=False)
print(f"RESULT {sum(r['passed'] for r in results)}/{len(results)}",flush=True)
sys.exit(any(not r['passed'] for r in results))
