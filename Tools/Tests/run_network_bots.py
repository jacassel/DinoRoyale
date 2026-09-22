"""Stage F: capacity, replacement and cleanup with actual joining processes."""
import time
from net_harness import NetworkTest,Peer,host_url,wait_for
t=NetworkTest('Tests/Results/multiplayer/stage-f-02')
def main(peer):return [a for a in peer.state()['networkActors'] if a['id']<100]
def bots(peer):return [a for a in main(peer) if a['bot']]
try:
    host=Peer(t,'BotHost',host_url(6,bots=True));client=Peer(t,'BotClient');time.sleep(.5)
    t.check('six slots filled by two humans and four bots',wait_for(lambda:len(main(client))==6 and len(bots(client))==4))
    t.check('all participant IDs unique',len({a['id'] for a in main(host)})==6)
    t.check('one controller for each server bot',len([a for a in host.state()['ai'] if a['id']<100])==4)
    initial_scores=host.state()['team0Kills']+host.state()['team1Kills']
    third=Peer(t,'BotThird');time.sleep(.5)
    t.check('joining human replaces one filler bot',wait_for(lambda:len(main(third))==6 and len(bots(third))==3))
    t.check('replacement creates no kill or corpse',host.state()['team0Kills']+host.state()['team1Kills']==initial_scores and all(s['kills']==0 for s in host.state()['scoreboard']) and len(host.state()['corpses'])==0)
    t.check('replacement removes old AI controller',len([a for a in host.state()['ai'] if a['id']<100])==3)
    client.lobby(2,1);third.lobby(2,1);host.lobby(6)
    t.check('mixed humans and bots can start',wait_for(lambda:not third.state()['lobby']) and len(main(host))==6)
    time.sleep(2)
    t.check('server bots use active AI',any(a['speed']>1 for a in host.state()['ai'] if a['id']<100))
    host.command('ai',paused=True)
    third.quit()
    t.check('match disconnect fills empty human slot',wait_for(lambda:len(host.state()['players'])==2 and len(bots(host))==4 and len(main(host))==6))
    t.check('disconnect keeps score unchanged',all(s['kills']==0 for s in host.state()['scoreboard']))
    host.lobby(7);host.lobby(5,0)
    t.check('disabling bots leaves only connected humans',wait_for(lambda:len(main(client))==2 and not bots(client)))
    t.check('disabling bots removes their controllers',len([a for a in host.state()['ai'] if a['id']<100])==0)
    host.lobby(4,10);host.lobby(3,1);host.lobby(5,1)
    t.check('ten slots supports two humans and eight bots',wait_for(lambda:len(main(client))==10 and len(bots(client))==8))
    t.check('team bot filling balances five versus five',sum(a['team']==0 for a in main(host))==5 and sum(a['team']==1 for a in main(host))==5)
    client.lobby(1,0)
    t.check('bots rebalance around human team choices',wait_for(lambda:sum(a['team']==0 for a in main(host))==5 and sum(a['team']==1 for a in main(host))==5))
    host.lobby(4,2)
    t.check('reducing capacity removes excess bots',wait_for(lambda:len(main(client))==2 and not bots(client)))
    host.lobby(5,0);client.quit()
    t.check('bots off leaves departed slot vacant',wait_for(lambda:len(main(host))==1))
    host.quit()
finally:t.close()
