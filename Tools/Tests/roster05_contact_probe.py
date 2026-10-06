import time,json,argparse
from net_harness import NetworkTest,Peer
p=argparse.ArgumentParser();p.add_argument('--species',type=int,default=4);p.add_argument('--quick',action='store_true');p.add_argument('--output',default='Tests/Results/roster05/contact-probe');a=p.parse_args()
t=NetworkTest(a.output,rendered=True);t.render_size=(1280,720)
try:
    h=Peer(t,'ContactProbe','/Game/Maps/LostValley');h.command('ai',paused=True);h.command('sandbox',enabled=True);h.command('species',value=a.species);h.command('face',yaw=0)
    h.command('testAI',id=1,species=0,x=320 if a.species==5 else -440,y=-120 if a.species==5 else 250,yaw=180,health=1,enabled=False)
    time.sleep(.5)
    if a.quick:h.tap('LeftMouseButton')
    else:h.key('RightMouseButton');time.sleep(1.8);h.key('RightMouseButton','up')
    rows=[];start=time.monotonic()
    while time.monotonic()-start<1.5:
        s=h.state();rows.append({k:s[k] for k in ['attackElapsed','animation','attackContacts','hits','damageDealt','x','y','z']});time.sleep(.04)
    (t.out/'trajectory.json').write_text(json.dumps(rows,indent=2));print(json.dumps(rows[::3]),flush=True)
finally:t.close()
