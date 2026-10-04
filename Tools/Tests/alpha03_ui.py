"""Real mapped settings/click paths plus rendered captures for visual inspection."""
import json,time,shutil,sys
import runtime_core as t
rows=[]
def check(name,ok,**data):
    rows.append(dict(test=name,passed=bool(ok),**data));(t.OUT/'ui.json').write_text(json.dumps(rows,indent=2));print(rows[-1],flush=True)
def tap(key):t.key(key);t.key(key,'up')
def shot(name):
    folder=t.BRIDGE.parent/'Screenshots/Windows';old=set(folder.glob('*.png'));t.command('screenshot');end=time.monotonic()+10
    while time.monotonic()<end:
        files=set(folder.glob('*.png'))-old
        if files:
            time.sleep(.2);shutil.copyfile(max(files,key=lambda p:p.stat().st_mtime),t.OUT/(name+'.png'));return
        time.sleep(.1)
    raise RuntimeError('No rendered screenshot for '+name)
t.command('menu',open=False);t.command('ai',paused=True);t.command('species',value=0);t.command('match',teams=False);t.command('sandbox',enabled=True)
t.command('teleport',x=0,y=0);t.command('camera',yaw=0,pitch=-10)
t.command('testAI',id=1,species=2,x=1300,y=450,yaw=180,health=1,enabled=False)
original=t.state()['showNameTags'];t.command('menu',open=True);tap('F2')
check('F2 opens name-tag setting',t.state()['settingsOpen']);tap('N');changed=t.state()['showNameTags']
check('N toggles local name tags',changed!=original)
configs=list((t.BRIDGE.parent/'Config').glob('*/Game.ini'))
value='True' if changed else 'False'
check('name-tag choice is written to local configuration',any('ShowNameTags='+value in p.read_text(encoding='utf-8-sig') for p in configs))
shot('settings-name-tags');tap('N');check('name-tag preference restored',t.state()['showNameTags']==original)
tap('F2');before=t.state()['round'];variant=t.state()['performanceMap'];t.command('mouse',x=550,y=710);tap('LeftMouseButton')
check('selection map row switches variant and resets round',t.state()['performanceMap']!=variant and t.state()['round']>before)
shot('selection-map');t.command('mouse',x=550,y=710);tap('LeftMouseButton');check('selection restores initial map',t.state()['performanceMap']==variant)
t.command('menu',open=False);t.command('teleport',x=0,y=0);t.command('face',yaw=0)
t.command('testAI',id=1,species=0,x=1500,y=300,yaw=180,health=1,enabled=False);time.sleep(1);shot('hud-names-and-score')
tap('M');shot('expanded-map-marker');tap('M')
t.command('sightBlocker',enabled=True);time.sleep(.3);shot('names-occluded');t.command('sightBlocker',enabled=False)
t.command('menu',open=True)
sys.exit(any(not r['passed'] for r in rows))
