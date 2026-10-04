"""Rendered two-process views; automated captures, not native play acceptance."""
import argparse,time,shutil,json
from net_harness import NetworkTest,Peer,host_url,wait_for,BUILD
p=argparse.ArgumentParser();p.add_argument('--executable',required=True);p.add_argument('--output',required=True);a=p.parse_args()
t=NetworkTest(a.output,executable=a.executable,rendered=True);t.offscreen=True;t.render_size=(1600,900)
def shot(peer,name):
    folder=peer.path.parent.parent/'Screenshots/Windows';old=set(folder.glob('*.png'));peer.command('screenshot');end=time.monotonic()+10
    while time.monotonic()<end:
        files=set(folder.glob('*.png'))-old
        if files:
            time.sleep(.3);shutil.copyfile(max(files,key=lambda p:p.stat().st_mtime),t.out/(name+'.png'));return
        time.sleep(.1)
    raise RuntimeError('Missing rendered view '+name)
def arrange():
    h.command('ai',paused=True)
    h.command('testAI',id=0,species=0,x=0,y=0,yaw=0,health=1,enabled=False)
    h.command('testAI',id=1,species=2,x=900,y=280,yaw=180,health=1,enabled=False)
    bots=[v for v in h.state()['networkActors'] if v['bot'] and v['scoring']]
    for i,v in enumerate(bots):h.command('testAI',id=v['id'],species=i%3,x=1800+i*500,y=900+i*200,yaw=180,health=1,enabled=False)
    h.command('ai',paused=True);time.sleep(1)
    h.command('face',yaw=0);c.command('face',yaw=180)
    h.command('camera',yaw=0,pitch=-10);c.command('camera',yaw=180,pitch=-10);time.sleep(.6)
try:
    h=Peer(t,'ViewHost',host_url(10,teams=True,bots=True)+'?Name=AlphaHost')
    c=Peer(t,'ViewGuest',f'127.0.0.1:7788?DinoBuild={BUILD}?Name=AlphaGuest')
    assert wait_for(lambda:len(c.state()['players'])==2)
    c.lobby(1,0);assert wait_for(lambda:h.player(1)['team']==0);h.lobby(12)
    for slot in [v['id'] for v in h.state()['networkActors'] if v['bot'] and v['scoring'] and v['team']==0]:h.lobby(10,slot)
    t.check('rendered unequal roster has two humans versus five bots',wait_for(lambda:sum(v['scoring'] for v in c.state()['networkActors'])==7))
    shot(h,'host-custom-roster');shot(c,'guest-custom-roster')
    for performance in [False,True]:
        if performance:h.lobby(7);assert wait_for(lambda:c.state()['lobby'])
        h.command('mapVariant',performance=performance);c.lobby(2,1);assert wait_for(lambda:h.player(1)['ready']);h.lobby(6)
        assert wait_for(lambda:not c.state()['lobby']);h.command('sandbox',enabled=True);arrange()
        label='Performance' if performance else 'Standard'
        t.check(label+' rendered map matches both peers',h.state()['performanceMap']==c.state()['performanceMap']==performance)
        shot(h,label+'-host');shot(c,label+'-guest')
        before=c.state();c.hold('Q',.4);time.sleep(.5);after=c.state()
        t.check(label+' rendered pivot keeps camera fixed',abs(before['cameraYaw']-after['cameraYaw'])<.01 and abs(before['yaw']-after['yaw'])>10)
        shot(h,label+'-remote-pivot')
        enemy=next(v['id'] for v in h.state()['networkActors'] if v['bot'] and v['scoring'] and v['team']==1)
        h.command('scoreHit',attacker=enemy,victim=1,value=99999);assert wait_for(lambda:c.state()['dead']);time.sleep(2)
        t.check(label+' rendered client death leaves replicated food',any(b['source']==1 and b['food']==480 for b in c.state()['corpses']))
        shot(h,label+'-carcass')
        (t.out/(label+'-state.json')).write_text(json.dumps(dict(host=h.state(),guest=c.state()),indent=2))
    c.quit();h.quit()
finally:t.close()
