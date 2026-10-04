"""Reproduced grid endpoint regressions plus every landmark route on both maps."""
import json,sys
import runtime_core as t
rows=[]
t.command('menu',open=False);t.command('ai',paused=True)
for performance in [False,True]:
    t.command('mapVariant',performance=performance)
    for start,end in [((-23761.58,26668.22),(-27420.73,29287.50)),((-5422.2916,-8483.5582),(-18839.1522,-20469.9013))]:
        t.command('teleport',x=start[0],y=start[1]);t.command('route',x=end[0],y=end[1])
        path=json.loads((t.BRIDGE/'route.json').read_text(encoding='utf-8-sig'))
        rows.append(dict(performance=performance,start=start,end=end,passed=path['success'],path=path['points'],prunedCells=t.state()['disconnectedNavCells']))
    t.command('navAudit');audit=json.loads((t.BRIDGE/'navigation-audit.json').read_text(encoding='utf-8-sig'))
    rows.append(dict(performance=performance,passed=all(r['success'] for r in audit['routes']),landmarkRoutes=len(audit['routes'])))
t.command('mapVariant',performance=False);t.command('menu',open=True)
(t.OUT/'navigation.json').write_text(json.dumps(rows,indent=2));print(json.dumps(rows),flush=True);sys.exit(any(not r['passed'] for r in rows))
