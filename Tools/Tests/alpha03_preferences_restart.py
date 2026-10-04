"""Verify actual packaged preferences survive process exit and relaunch."""
import argparse
from net_harness import NetworkTest,Peer
p=argparse.ArgumentParser();p.add_argument('--executable',required=True);p.add_argument('--output',required=True);a=p.parse_args()
t=NetworkTest(a.output,executable=a.executable)
try:
    first=Peer(t,'PreferencesFirst','/Game/Maps/LostValley')
    before=first.state();first.command('menu',open=True);first.tap('F2')
    first.tap('N');first.tap('B');first.tap('Equals');expected=first.state()
    t.check('settings inputs change name tags, blood and sensitivity',expected['showNameTags']!=before['showNameTags'] and expected['bloodEnabled']!=before['bloodEnabled'] and expected['sensitivity']>before['sensitivity'])
    first.quit()
    second=Peer(t,'PreferencesRestart','/Game/Maps/LostValley');loaded=second.state()
    for key in ['showNameTags','bloodEnabled','sensitivity']:
        t.check(key+' survives a new packaged process',loaded[key]==expected[key],saved=expected[key],loaded=loaded[key])
    second.command('menu',open=True);second.tap('F2');second.tap('N');second.tap('B');second.tap('Hyphen')
    restored=second.state()
    t.check('original preferences restored',all(abs(float(restored[k])-float(before[k]))<.001 for k in ['showNameTags','bloodEnabled','sensitivity']))
    second.quit()
finally:t.close()
