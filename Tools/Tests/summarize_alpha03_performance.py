"""Summarize measured Alpha 0.3 performance; never infer unmeasured results."""
import json
import pathlib
import statistics

root = pathlib.Path(__file__).resolve().parents[2]
evidence = root / 'Tests/Results/alpha03'
before = json.loads((evidence / 'baseline/performance.json').read_text())
after = json.loads((evidence / 'performance-after-standard/performance.json').read_text())
matrix = json.loads((evidence / 'performance-final/performance-matrix.json').read_text())
assert len(before) == len(after) == 3 and len(matrix) == 20
matched = []
for old, new in zip(before, after):
    assert (old['region'], old['species']) == (new['region'], new['species'])
    matched.append(dict(scene=old['region'], beforeFPS=old['fps'], afterFPS=new['fps'],
                        fpsIncreasePercent=(new['fps'] / old['fps'] - 1) * 100,
                        beforeP95Ms=old['p95Ms'], afterP95Ms=new['p95Ms']))
pairs = []
for standard in [r for r in matrix if r['variant'] == 'Standard']:
    performance = next(r for r in matrix if r['variant'] == 'Performance'
                       and r['scene'] == standard['scene'] and r['mode'] == standard['mode'])
    pairs.append(dict(mode=standard['mode'], scene=standard['scene'],
                      standardFPS=standard['fps'], performanceFPS=performance['fps'],
                      fpsIncreasePercent=(performance['fps'] / standard['fps'] - 1) * 100))
summary = dict(matchedBaseline=matched, mapComparison=pairs,
               averageFPSByVariant={v: statistics.mean(r['fps'] for r in matrix if r['variant'] == v)
                                    for v in ['Standard', 'Performance']},
               sampledFrames=sum(r['sampledFrames'] for r in matrix),
               sampledHitchesOver33ms=sum(r['hitchesOver33ms'] for r in matrix),
               sampledHitchesOver50ms=sum(r['hitchesOver50ms'] for r in matrix),
               memoryRangeMB=[min(r[k] for r in matrix for k in ['memoryStartMB', 'memoryEndMB']),
                              max(r[k] for r in matrix for k in ['memoryStartMB', 'memoryEndMB'])])
(evidence / 'performance-final/summary.json').write_text(json.dumps(summary, indent=2))

lines = [
    '# Dino Royale 0.3 Alpha Test — measured performance', '',
    'October 4, 2026. Windows 11, Intel Core i7-11700, 32 GB RAM, NVIDIA RTX 3060,',
    'Unreal 5.8.2 Windows Development package, DirectX 12 / SM5, 1600×900 output, VSync off.',
    'Both packages retain identical scalability settings (quality level 3 and engine-default',
    'resolution quality). Current stat-unit captures show 83.4% internal resolution',
    '(1336×751), with TAA. These are not native-100%-resolution measurements.',
    'Each measurement uses a four-second warmup and a twenty-second live gameplay',
    'window with ten major combatants, prey and pack followers. Rendering is active',
    'offscreen. No other game, editor, build or multiplayer test ran concurrently.',
    'The opt-in telemetry bridge was enabled. The ordinary 60 FPS cap is unchanged;',
    'tests explicitly removed it to measure headroom. No hardware settings changed.', '',
    '## Same scenes: preserved prior build versus 0.3 Standard', '',
    'The same script, resolution, species, locations, camera directions and live-AI',
    'setup were used. The invalid concurrent-editor baseline is excluded.', '',
    '| Scene | Prior FPS | 0.3 Standard FPS | FPS increase | Prior / new p95 frame ms |',
    '|---|---:|---:|---:|---:|',
]
for r in matched:
    lines.append(f"| {r['scene']} | {r['beforeFPS']:.1f} | {r['afterFPS']:.1f} | {r['fpsIncreasePercent']:+.1f}% | {r['beforeP95Ms']:.2f} / {r['afterP95Ms']:.2f} |")
lines += ['', '## Current maps: FFA and teams', '',
          'Each map uses the same five scene definitions in both modes. The close-combat',
          'fixture places nine active AI around the player; traversal holds forward/sprint.',
          'AI outcomes vary between runs. These are scene measurements, not a controlled',
          'replay or a guarantee for another computer. This is one continuous session:',
          'persistent corpses accumulate, and Performance runs after Standard. Actor',
          'counts document that extra workload; the fresh-process 1080p comparison',
          'below separates map cost from this accumulated workload.', '',
          '| Mode | Scene | Standard FPS | Performance FPS | FPS increase |',
          '|---|---|---:|---:|---:|']
for r in pairs:
    lines.append(f"| {r['mode']} | {r['scene']} | {r['standardFPS']:.1f} | {r['performanceFPS']:.1f} | {r['fpsIncreasePercent']:+.1f}% |")
lines += ['', '## Timing and world measurements', '',
          '| Map | Mode / scene | Game / render / GPU median ms | Frame p95 ms | Actors | Memory end MB |',
          '|---|---|---:|---:|---:|---:|']
for r in matrix:
    t = r['timings']
    lines.append(f"| {r['variant']} | {r['mode']} / {r['scene']} | {t['gameThreadMs']['median']:.2f} / {t['renderThreadMs']['median']:.2f} / {t['gpuMs']['median']:.2f} | {t['frameMs']['p95']:.2f} | {r['counts']['actorCount']} | {r['memoryEndMB']:.0f} |")
lines += ['',
          'Standard retains 502 tree instances, 31,731 grass instances and 523 fern',
          'instances in existing HISM components. Performance has zero cosmetic tree,',
          'grass and fern instances. Both retain 36 edible plant locations, terrain,',
          'water and rocks. Standard retains tree collision and 556 navigation obstacles;',
          'Performance removes the tree obstacles along with the trees.', '',
          f"Across {summary['sampledFrames']:,} timing samples: {summary['sampledHitchesOver33ms']} exceeded 33.3 ms and {summary['sampledHitchesOver50ms']} exceeded 50 ms.",
          f"Observed process memory ranged from {summary['memoryRangeMB'][0]:.0f} to {summary['memoryRangeMB'][1]:.0f} MB.",
          'Memory includes generated assets, caches and persistent corpses. This bounded',
          'run does not establish leak-free long sessions. Samples arrive about every',
          '50 ms and can miss individual frame spikes; FPS uses actual frame-count/time',
          'deltas. GPU/game/render values are engine counters, not a full profiler trace.',
          'Draw-call counts are visible in post-window stat-unit screenshots only, not',
          'a timed series. Screenshot capture occurs outside the',
          'measurement windows. No claim is made about unmeasured weak-PC performance.', '',
          'Evidence: `Tests/Results/alpha03/baseline/performance.json`,',
          '`performance-after-standard/performance.json`, and',
          '`performance-final/performance-matrix.json` / `summary.json`.', '']
native = {}
for variant in ['standard', 'performance']:
    path = evidence / ('performance-1080-' + variant) / 'performance-native-1080.json'
    if path.exists():
        native[variant] = json.loads(path.read_text())
if len(native) == 2:
    assert all(len(v) == 3 for v in native.values())
    lines += ['## Fresh processes at native 1920×1080', '',
              'A separate process per map, 100% screen percentage, same three scene',
              'definitions and species, ten live major combatants in team mode. These',
              'results compare the current maps only, not the prior-build baseline.', '',
              '| Scene | Standard FPS | Performance FPS | FPS increase | Standard / Performance GPU median ms |',
              '|---|---:|---:|---:|---:|']
    for old, new in zip(native['standard'], native['performance']):
        assert old['scene'] == new['scene']
        lines.append(f"| {old['scene']} | {old['fps']:.1f} | {new['fps']:.1f} | {(new['fps']/old['fps']-1)*100:+.1f}% | {old['medianMs']['gpuMs']:.2f} / {new['medianMs']['gpuMs']:.2f} |")
    lines += ['', 'Evidence: `performance-1080-standard/performance-native-1080.json` and',
              '`performance-1080-performance/performance-native-1080.json`.', '']
(root / 'PERFORMANCE_REPORT.md').write_text('\n'.join(lines), encoding='utf-8')
print(json.dumps(summary, indent=2))
