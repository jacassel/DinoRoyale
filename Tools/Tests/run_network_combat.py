"""Real client inputs for combos, cooldowns, charge validation and injury."""
import time
from net_harness import NetworkTest,Peer,host_url,wait_for,distance
t=NetworkTest('Tests/Results/multiplayer/combat-01')
try:
    host=Peer(t,'FightHost',host_url(2));client=Peer(t,'FightClient');client.lobby(2,1);host.lobby(6);host.command('ai',paused=True)
    for species,reach in ((0,450),(1,210),(2,380)):
        host.command('respawn');host.command('teleport',x=0,y=0);host.command('face',yaw=0)
        host.command('testAI',id=1,species=species,x=reach,y=0,yaw=180,health=1,enabled=False);host.command('ai',paused=True)
        client.command('face',yaw=180);time.sleep(.6)
        for combo in range(1,4):
            hp=host.state()['health'];client.tap('LeftMouseButton')
            t.check(f'species {species} combo hit {combo} server damage',wait_for(lambda:host.state()['health']<hp) and wait_for(lambda:client.state()['combo']==combo))
            t.check(f'species {species} hit {combo} health agrees',wait_for(lambda:abs(client.actor(0)['health']-host.state()['health'])<.1))
            if combo==3:
                serial=client.state()['attackSerial']
                client.command('menu',open=True);client.command('menu',open=False);client.tap('LeftMouseButton');time.sleep(.08)
                t.check(f'species {species} menu does not bypass committed combo cooldown',client.state()['attackSerial']==serial and client.state()['recovery']>0,serial=client.state()['attackSerial'])
            wait_for(lambda:client.state()['recovery']<=0,4)
        host.command('testAI',id=1,species=species,x=4000,y=0,yaw=0,health=1,enabled=False);host.command('ai',paused=True);client.command('face',yaw=0);time.sleep(.6)
        client.key('RightMouseButton');t.check(f'species {species} client charge reaches server',wait_for(lambda:client.state()['charging'] and client.state()['charge']>.95))
        before=client.state();client.key('RightMouseButton','up');time.sleep(.25);after=client.state()
        t.check(f'species {species} charged lunge or pounce moves',distance(before,after)>30 or after['z']>before['z']+20,distance=distance(before,after),rise=after['z']-before['z'])
        wait_for(lambda:client.state()['recovery']<=0,4);time.sleep(.5)
        speeds=[]
        for health in (1,.49,.24):
            host.command('testAI',id=1,species=species,x=4000,y=0,yaw=0,health=health,enabled=False);host.command('ai',paused=True);time.sleep(.4)
            speeds.append(client.state()['maxSpeed'])
        t.check(f'species {species} injury penalties replicate',abs(speeds[1]/speeds[0]-.85)<.02 and abs(speeds[2]/speeds[0]-.7)<.02,speeds=speeds)
        client.key('RightMouseButton');time.sleep(.3);t.check(f'species {species} critical health blocks charge',not client.state()['charging']);client.key('RightMouseButton','up')
        host.command('testAI',id=1,species=species,x=4000,y=0,yaw=0,health=1,hunger=0,stamina=0,enabled=False);host.command('ai',paused=True);time.sleep(.5)
        client.key('RightMouseButton');time.sleep(.3);t.check(f'species {species} insufficient stamina blocks charge',not client.state()['charging']);client.key('RightMouseButton','up')
        client.tap('LeftMouseButton');t.check(f'species {species} exhausted quick attack remains weak',wait_for(lambda:client.state()['weakAttack']))
    client.quit();host.quit()
finally:t.close()
