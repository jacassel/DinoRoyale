"""Bounded packaged-process scaling. Stops before memory pressure harms the PC."""
import argparse,json,time,statistics,pathlib,psutil
from net_harness import NetworkTest,Peer,host_url,wait_for,distance
p=argparse.ArgumentParser();p.add_argument('--executable',required=True);p.add_argument('--count',type=int,default=10);p.add_argument('--seconds',type=int,default=120);p.add_argument('--output',default='Tests/Results/multiplayer/scale-01');a=p.parse_args()
t=NetworkTest(a.output,executable=a.executable);peers=[];samples=[]
def snapshot(phase):
    states=[peer.state() for peer in peers];mem=psutil.virtual_memory()
    row=dict(phase=phase,wall=time.monotonic(),availableGB=mem.available/2**30,
             peers=[dict(id=s['combatantID'],frameMs=s['frameMs'],pingMs=s.get('pingMs'),players=len(s['players']),
                         main=sum(x['scoring'] for x in s['networkActors']),followers=sum(x['follower'] for x in s['networkActors']),
                         outBytes=s.get('netOutBytes'),inBytes=s.get('netInBytes'),lostOut=s.get('netOutPacketsLost'),lostIn=s.get('netInPacketsLost')) for s in states])
    samples.append(row)
    (t.out/'samples.json').write_text(json.dumps(samples,indent=2))
    if mem.available<2.5*2**30:raise RuntimeError('Available memory below 2.5 GB; stopped for machine stability')
    return states
def soak(phase):
    start=time.monotonic();last_motion=0
    while time.monotonic()-start<a.seconds:
        states=snapshot(phase)
        if any(len(s['players'])!=len(peers) for s in states):raise AssertionError('Player disappeared during soak')
        if time.monotonic()-last_motion>20:
            # Move each real client and request attacks; all decisions still run on server.
            for i,peer in enumerate(peers):
                if not peer.state()['dead']:
                    peer.command('face',yaw=(i*360/len(peers)+time.monotonic()*2)%360);peer.key('W');peer.tap('LeftMouseButton')
            time.sleep(.5)
            for peer in peers:peer.key('W','up')
            last_motion=time.monotonic()
        time.sleep(.5)
    values=[x for x in samples if x['phase']==phase];duration=values[-1]['wall']-values[0]['wall']
    summary=dict(instances=len(peers),durationSeconds=duration,minimumAvailableGB=min(s['availableGB'] for s in values),
                 peers=[dict(id=i,medianFPS=1000/statistics.median(s['peers'][i]['frameMs'] for s in values),
                             p95FrameMs=sorted(s['peers'][i]['frameMs'] for s in values)[int(.95*(len(values)-1))],
                             outgoingBytesPerSecond=(values[-1]['peers'][i]['outBytes']-values[0]['peers'][i]['outBytes'])/duration,
                             incomingBytesPerSecond=(values[-1]['peers'][i]['inBytes']-values[0]['peers'][i]['inBytes'])/duration) for i in range(len(peers))])
    (t.out/(phase+'-summary.json')).write_text(json.dumps(summary,indent=2))
    t.check(phase+' sustained network session',True,**summary)
try:
    host=Peer(t,'ScaleHost',host_url(10));peers.append(host)
    for i in range(1,a.count):
        available=psutil.virtual_memory().available
        if available<4*2**30:
            (t.out/'capacity-limit.json').write_text(json.dumps(dict(requested=a.count,connected=len(peers),status='NOT VERIFIED at requested count',reason='Less than 4 GB available before next process',availableGB=available/2**30),indent=2));break
        peer=Peer(t,'ScaleClient'+str(i));peers.append(peer)
        t.check(f'{i+1} actual connected game processes',wait_for(lambda:all(len(p.state()['players'])==len(peers) for p in peers)),availableGB=psutil.virtual_memory().available/2**30)
    t.check('all connected players have unique slots',{p.state()['combatantID'] for p in peers}==set(range(len(peers))))
    for peer in peers:peer.lobby(0,1)
    t.check('all raptor leaders own two followers',wait_for(lambda:all(len([x for x in p.state()['networkActors'] if x['follower']])==2*len(peers) for p in peers)))
    for peer in peers[1:]:peer.lobby(2,1)
    host.lobby(6);host.command('sandbox',enabled=True)
    t.check('raptor-heavy FFA starts at tested capacity',wait_for(lambda:all(not p.state()['lobby'] for p in peers)))
    soak('ffa')
    host.lobby(7);host.lobby(3,1)
    for i,peer in enumerate(peers):peer.lobby(1,i%2)
    if len(peers)==10:
        peers[1].lobby(1,0);t.check('team cannot exceed five human participants',wait_for(lambda:'full' in peers[1].state()['lobbyStatus']) and peers[1].player(1)['team']==1)
    for peer in peers[1:]:peer.lobby(2,1)
    host.lobby(6)
    t.check('balanced team round starts at tested capacity',wait_for(lambda:all(p.state()['teamMode'] and not p.state()['lobby'] for p in peers)))
    soak('teams')
    for peer in peers[1:]:peer.quit()
    t.check('all departed clients and their packs cleaned up',wait_for(lambda:len(host.state()['players'])==1 and len([x for x in host.state()['networkActors'] if x['scoring']])==1 and len([x for x in host.state()['networkActors'] if x['follower']])==2))
    host.quit()
    bad=[]
    for logfile in t.out.glob('*.log'):
        content=logfile.read_text(errors='replace')
        for term in ['could not resolve the new relative movement base','Reliable buffer overflow','Assertion failed','Fatal error','Out of memory']:
            if term in content:bad.append(dict(file=logfile.name,message=term))
    t.check('no fatal replication or memory warnings',not bad,warnings=bad)
finally:t.close()
