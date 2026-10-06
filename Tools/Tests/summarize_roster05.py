"""Count explicitly selected acceptance runs without double-counting retries/wrappers."""
import pathlib,json
ROOT=pathlib.Path(__file__).resolve().parents[2];base=ROOT/'Tests/Results/roster05'
paths={
 'Original controls':'core-original-three/runtime_core/core-live.json',
 'Original match rules':'extended-regression/runtime_matches/match-rules.json',
 'Original swimming':'extended-regression/runtime_swimming/swimming.json',
 'Original ecology':'extended-regression/runtime_ecology/ecology-visibility.json',
 'Original settings':'extended-regression/runtime_settings/settings-water-blood.json',
 'Original combat':'candidate1-combat-regression/runtime_combat_polish/combat-polish.json',
 'Original integration':'candidate1-combat-regression/runtime_integration/integration-live.json',
 'Original safe respawn':'candidate1-combat-regression/runtime_safe_respawn/safe-respawn.json',
 'Full mixed matches':'candidate1-combat-regression/runtime_rounds/played-rounds.json',
 'Anky packaged controls':'candidate3-smoke-4/results.json',
 'Brachi packaged controls':'candidate3-smoke-5/results.json',
 'Pachy packaged controls':'candidate3-smoke-6/results.json',
 'New combos':'candidate3-retest-combos/results.json',
 'New rules':'candidate5-roster-rules/results.json',
 'New survival':'candidate3-retest-survival/results.json',
 'Selection and HUD':'candidate3-ui/results.json',
 'Packaged audio':'candidate3-audio-focused-mix/results.json',
 'Tree regrowth':'candidate1-tree-regrowth/results.json',
 'New roster network':'candidate3-network/results.json',
 'Pachy packs with loss':'candidate3-pachy-packs/results.json',
 'Raptor packs with loss':'candidate3-raptor-packs-ready-sync/results.json',
 'Network fundamentals with loss':'candidate3-fundamentals/results.json',
 'Network combat':'candidate1-existing-network/combat/results.json',
 'Network lobby':'candidate1-existing-network/lobby/results.json',
 'Network bots':'candidate1-existing-network/bots/results.json',
 'Network assist match rules':'candidate2-existing-network/matches/results.json',
 'Network ecology':'candidate2-existing-network/ecology/results.json',
 'Network custom teams and late join':'candidate1-team-configs/results.json',
 'Network silent timeouts':'candidate3-failure-edges/results.json',
 'Mixed roster endurance':'candidate6-soak/results.json',
}
rows=[]
expected=[90,24,40,42,28,69,41,3,2,15,15,15,20,52,48,13,30,4,32,21,22,24,39,20,17,52,25,25,13,4]
for (name,relative),count in zip(paths.items(),expected):
 path=base/relative
 if not path.exists():rows.append(dict(suite=name,path=relative,missing=True));continue
 data=json.loads(path.read_text());tests=data['tests'] if isinstance(data,dict) else data
 if not isinstance(tests,list):raise ValueError(relative)
 passed=sum(r.get('status')=='PASS' or r.get('passed') is True for r in tests)
 rows.append(dict(suite=name,path=relative,total=len(tests),expected=count,passed=passed,allPassed=passed==len(tests)==count))
result=dict(total=sum(r.get('total',0) for r in rows),passed=sum(r.get('passed',0) for r in rows),complete=all(r.get('allPassed',False) for r in rows),suites=rows)
(base/'acceptance-summary.json').write_text(json.dumps(result,indent=2));print(json.dumps(result,indent=2))
