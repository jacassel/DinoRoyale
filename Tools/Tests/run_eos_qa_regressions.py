"""Run existing focused gameplay suites on a specific package, without changing fixtures.

These use explicit loopback development transport; they cannot establish EOS success.
"""
import argparse, pathlib, runpy
import net_harness

p=argparse.ArgumentParser()
p.add_argument('--executable',required=True)
p.add_argument('--output',default='Tests/Results/eos-qa-20260926')
p.add_argument('--suite',choices=['bots','matches','ecology'],required=True)
a=p.parse_args()
original_init=net_harness.NetworkTest.__init__
def packaged_init(self,output,**kwargs):
    original_init(self,f'{a.output}/{a.suite}',executable=a.executable,**kwargs)
net_harness.NetworkTest.__init__=packaged_init
runpy.run_path(str(pathlib.Path(__file__).with_name(f'run_network_{a.suite}.py')),run_name='__main__')
