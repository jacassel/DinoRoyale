"""Silent packet loss/timeout and ending-match rejection on the shipped package.

Only this test's own game processes are suspended; no firewall/network settings
are modified. Loopback timeouts do not establish EOS or WAN connectivity.
"""
import argparse,time,psutil
from net_harness import NetworkTest,Peer,host_url,wait_for,distance
p=argparse.ArgumentParser();p.add_argument('--executable',required=True);p.add_argument('--output',default='Tests/Results/multiplayer/packaged-failure-edges');a=p.parse_args()
t=NetworkTest(a.output,executable=a.executable)
suspended=[]
def silence(peer):
    process=psutil.Process(peer.proc.pid);process.suspend();suspended.append(process);return process
def resume(process):
    process.resume();suspended.remove(process)
try:
    host=Peer(t,'TimeoutHost',host_url(4,bots=True));client=Peer(t,'TimeoutClient')
    healthy=Peer(t,'HealthyGuest')
    client.lobby(0,1);client.lobby(2,1);healthy.lobby(2,1);host.lobby(6);host.command('ai',paused=True)
    t.check('raptor client and private followers connected',wait_for(lambda:len(host.state()['players'])==3 and len([d for d in host.state()['networkActors'] if d['follower'] and d['pack']==1])==2))
    paused=silence(client);started=time.monotonic()
    t.check('silent client times out while listen host remains active',wait_for(lambda:host.state()['netMode']==2 and len(host.state()['players'])==2,40),elapsedSeconds=time.monotonic()-started)
    t.check('timed-out human replaced by exactly one bot and pack',wait_for(lambda:len([d for d in host.state()['networkActors'] if d['id']==1 and d['bot']])==1 and len([d for d in host.state()['networkActors'] if d['follower'] and d['pack']==1])==2))
    t.check('timeout replacement does not award kills',all(s['kills']==0 for s in host.state()['scoreboard']))
    before=healthy.state();after=healthy.hold('W',.5)
    t.check('healthy guest remains connected and controllable',after['netMode']==3 and distance(before,after)>80 and wait_for(lambda:distance(host.actor(2),healthy.state())<90))
    resume(paused)
    t.check('resumed stale client returns to menu',wait_for(lambda:client.state()['netMode']==0 and client.state()['menuOpen'],40))
    client.quit()
    guest=Peer(t,'TimeoutGuest');t.check('new guest can replace timeout filler',wait_for(lambda:len(host.state()['players'])==3 and not host.actor(1)['bot']))
    paused=silence(host);started=time.monotonic()
    t.check('silent host loss returns both guests with clear reason',wait_for(lambda:all(p.state()['netMode']==0 and p.state()['onlineStatus']=='Host disconnected.' for p in (guest,healthy)),40),elapsedSeconds=time.monotonic()-started)
    resume(paused);guest.quit();healthy.quit()
    t.check('host removes timed-out guest after resuming',wait_for(lambda:len(host.state()['players'])==1,40))
    host.lobby(7);host.lobby(5,0)
    late=Peer(t,'EndingPlayer');late.lobby(2,1);host.lobby(6);host.command('ai',paused=True)
    for kills in range(1,6):
        host.command('resetCombatant',id=1);host.command('scoreHit',attacker=0,victim=1,value=100000)
        if not wait_for(lambda:host.state()['kills']==kills):raise AssertionError('fixture score publication')
    t.check('ending round visible to both players',wait_for(lambda:late.state()['roundOver'] and host.state()['roundOver']))
    rejected=Peer(t,'EndingRejected')
    t.check('ending match rejects new guest with clear reason',wait_for(lambda:rejected.state()['netMode']==0 and rejected.state()['onlineStatus']=='Match is ending.'))
    t.check('rejected ending join creates no extra participant',len(host.state()['players'])==2)
    rejected.quit();late.quit();host.quit()
    missing=Peer(t,'MissingHost','127.0.0.1:7799?DinoBuild=2026092201')
    t.check('unreachable host returns clear failure and menu',wait_for(lambda:missing.state()['netMode']==0 and missing.state()['onlineStatus']=='Could not connect to host.' and missing.state()['menuOpen'],45))
    missing.quit()
finally:
    for process in suspended:
        try:process.resume()
        except psutil.NoSuchProcess:pass
    t.close()
