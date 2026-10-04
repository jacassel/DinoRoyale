"""Run an existing network suite against a named build/map without replacing its assertions."""
import argparse,pathlib,runpy,sys
import net_harness
p=argparse.ArgumentParser();p.add_argument('--suite',required=True);p.add_argument('--output',required=True)
p.add_argument('--executable');p.add_argument('--lag',type=int);p.add_argument('--loss',type=int);p.add_argument('--performance',action='store_true')
a,rest=p.parse_known_args();original=net_harness.NetworkTest.__init__;url=net_harness.host_url
def init(self,output,**kwargs):
    if a.executable:kwargs['executable']=a.executable
    if a.lag is not None:kwargs['lag']=a.lag
    if a.loss is not None:kwargs['loss']=a.loss
    original(self,a.output,**kwargs)
net_harness.NetworkTest.__init__=init
if a.performance:net_harness.host_url=lambda *args,**kwargs:url(*args,**kwargs)+'?PerformanceMap=1'
path=pathlib.Path(__file__).with_name('run_network_'+a.suite+'.py')
sys.argv=[str(path),*rest]
if a.suite in ['cosmetics','survival_edges','failure_edges','rendered','scale']:
    if not a.executable:raise SystemExit('This suite requires a packaged executable')
    sys.argv+=['--executable',a.executable]
runpy.run_path(str(path),run_name='__main__')
