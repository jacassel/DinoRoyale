"""Select accepted launch runs explicitly; exclude retries and wrapper results."""
import json
from pathlib import Path

BASE = Path(__file__).resolve().parents[2] / 'Tests/Results/launch06'
RUNS = [
    ('Original controls', 'candidate1-core/runtime_core/core-live.json', 90),
    ('Map and results', 'candidate1-core/runtime_map_results/map-results.json', 21),
    ('Safe respawn', 'candidate1-core/runtime_safe_respawn/safe-respawn.json', 3),
    ('Settings', 'candidate1-core/runtime_settings/settings-water-blood.json', 28),
    ('Original swimming', 'core-second/runtime_swimming/swimming.json', 40),
    ('Original ecology', 'core-second/runtime_ecology/ecology-visibility.json', 42),
    ('Scoring and diets', 'scoring-diets-first/results.json', 25),
    ('Configuration and multiplayer rules', 'candidate1-rules/results.json', 90),
    ('Seven-species selection', 'candidate1-ui/results.json', 15),
    ('Albertosaurus final controls', 'candidate2-alberto-core/launch06_alberto_core/core-live.json', 30),
    ('Final combos and club contacts', 'candidate2-combos/results.json', 26),
    ('Final mobility, ecology and pack scoring', 'candidate2-gameplay/results.json', 24),
    ('Final survival, blood and water', 'candidate2-survival-isolated/results.json', 64),
    ('Three-process delayed/lossy gameplay', 'candidate2-network-ready/results.json', 39),
    ('Rendered guest setup and attacks', 'candidate2-network-visual/results.json', 14),
    ('Albertosaurus mixer and sound events', 'candidate2-audio/results.json', 10),
    ('Art materials and trajectories', 'art-motion-second/results.json', 18),
    ('Final Brachi and Alberto camera motion', 'candidate2-motion-casefold/results.json', 8),
    ('Restart and independent bot-count UI', 'candidate2-setup-onscreen/results.json', 4),
    ('Final mixed-roster endurance', 'candidate2-soak/results.json', 4),
]
rows = []
for name, relative, expected in RUNS:
    path = BASE / relative
    if not path.exists():
        rows.append(dict(suite=name, path=relative, missing=True))
        continue
    data = json.loads(path.read_text(encoding='utf-8'))
    tests = data['tests'] if isinstance(data, dict) else data
    passed = sum(x.get('status') == 'PASS' or x.get('passed') is True for x in tests)
    rows.append(dict(suite=name, path=relative, total=len(tests), expected=expected,
                     passed=passed, allPassed=passed == len(tests) == expected))
result = dict(total=sum(x.get('total', 0) for x in rows),
              passed=sum(x.get('passed', 0) for x in rows),
              complete=all(x.get('allPassed', False) for x in rows), suites=rows)
(BASE / 'acceptance-summary.json').write_text(json.dumps(result, indent=2))
print(json.dumps(result, indent=2))
raise SystemExit(0 if result['complete'] else 1)
