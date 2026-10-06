import time,json
from net_harness import NetworkTest,Peer
t=NetworkTest('Tests/Results/roster05/contact-probe',rendered=True);t.render_size=(1280,720)
try:
    h=Peer(t,'ContactProbe','/Game/Maps/LostValley');h.command('ai',paused=True);h.command('sandbox',enabled=True);h.command('species',value=4);h.command('face',yaw=0)
    h.command('testAI',id=1,species=0,x=-440,y=-280,yaw=180,health=1,enabled=False)
    time.sleep(.5);h.key('RightMouseButton');time.sleep(1.4);h.key('RightMouseButton','up')
    rows=[];start=time.monotonic()
    while time.monotonic()-start<1.5:
        s=h.state();rows.append({k:s[k] for k in ['attackElapsed','animation','attackContacts','hits','damageDealt']});time.sleep(.04)
    (t.out/'trajectory.json').write_text(json.dumps(rows,indent=2));print(json.dumps(rows[::3]),flush=True)
finally:t.close()
