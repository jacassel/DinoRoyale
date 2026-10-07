"""Full established movement/combat/death regression for the seventh species."""
import runtime_core as t
t.command('match',teams=False)
t.command('menu',open=False)
t.command('ai',paused=True)
t.command('sandbox',enabled=True)
t.run_species(7,'Albertosaurus')
raise SystemExit(any(not r['passed'] for r in t.results))
