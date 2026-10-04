# Dino Royale 0.3 Alpha Test — measured performance

October 4, 2026. Windows 11, Intel Core i7-11700, 32 GB RAM, NVIDIA RTX 3060,
Unreal 5.8.2 Windows Development package, DirectX 12 / SM5, 1600×900 output, VSync off.
Both packages retain identical scalability settings (quality level 3 and engine-default
resolution quality). Current stat-unit captures show 83.4% internal resolution
(1336×751), with TAA. These are not native-100%-resolution measurements.
Each measurement uses a four-second warmup and a twenty-second live gameplay
window with ten major combatants, prey and pack followers. Rendering is active
offscreen. No other game, editor, build or multiplayer test ran concurrently.
The opt-in telemetry bridge was enabled. The ordinary 60 FPS cap is unchanged;
tests explicitly removed it to measure headroom. No hardware settings changed.

## Same scenes: preserved prior build versus 0.3 Standard

The same script, resolution, species, locations, camera directions and live-AI
setup were used. The invalid concurrent-editor baseline is excluded.

| Scene | Prior FPS | 0.3 Standard FPS | FPS increase | Prior / new p95 frame ms |
|---|---:|---:|---:|---:|
| plains | 65.6 | 185.0 | +182.1% | 16.11 / 6.46 |
| forest | 105.4 | 176.9 | +67.8% | 10.84 / 6.30 |
| pond | 80.2 | 187.5 | +133.8% | 13.77 / 6.23 |

## Current maps: FFA and teams

Each map uses the same five scene definitions in both modes. The close-combat
fixture places nine active AI around the player; traversal holds forward/sprint.
AI outcomes vary between runs. These are scene measurements, not a controlled
replay or a guarantee for another computer. This is one continuous session:
persistent corpses accumulate, and Performance runs after Standard. Actor
counts document that extra workload; the fresh-process 1080p comparison
below separates map cost from this accumulated workload.

| Mode | Scene | Standard FPS | Performance FPS | FPS increase |
|---|---|---:|---:|---:|
| ffa | plains | 169.9 | 177.0 | +4.2% |
| ffa | forest | 165.2 | 186.2 | +12.7% |
| ffa | pond | 178.2 | 187.9 | +5.4% |
| ffa | close-combat | 162.1 | 164.6 | +1.5% |
| ffa | traversal | 166.0 | 182.9 | +10.2% |
| teams | plains | 161.1 | 167.2 | +3.8% |
| teams | forest | 164.5 | 185.9 | +13.0% |
| teams | pond | 172.8 | 185.6 | +7.4% |
| teams | close-combat | 152.9 | 160.3 | +4.8% |
| teams | traversal | 162.7 | 180.5 | +11.0% |

## Timing and world measurements

| Map | Mode / scene | Game / render / GPU median ms | Frame p95 ms | Actors | Memory end MB |
|---|---|---:|---:|---:|---:|
| Standard | ffa / plains | 5.20 / 5.19 / 4.85 | 6.75 | 119 | 1173 |
| Standard | ffa / forest | 4.56 / 5.87 / 5.31 | 6.70 | 120 | 1167 |
| Standard | ffa / pond | 4.82 / 4.41 / 4.37 | 6.94 | 122 | 1172 |
| Standard | ffa / close-combat | 5.22 / 5.64 / 5.17 | 6.62 | 126 | 1182 |
| Standard | ffa / traversal | 5.11 / 5.30 / 5.03 | 6.86 | 130 | 1174 |
| Standard | teams / plains | 5.20 / 5.69 / 5.26 | 6.92 | 133 | 1184 |
| Standard | teams / forest | 5.10 / 5.78 / 5.22 | 6.75 | 134 | 1174 |
| Standard | teams / pond | 4.90 / 4.39 / 4.40 | 7.16 | 137 | 1180 |
| Standard | teams / close-combat | 5.25 / 6.04 / 5.44 | 7.11 | 141 | 1188 |
| Standard | teams / traversal | 5.35 / 5.40 / 5.11 | 6.81 | 143 | 1179 |
| Performance | ffa / plains | 4.74 / 4.26 / 4.20 | 6.27 | 151 | 1175 |
| Performance | ffa / forest | 4.41 / 3.42 / 3.26 | 5.79 | 151 | 1168 |
| Performance | ffa / pond | 4.36 / 3.37 / 3.31 | 5.88 | 156 | 1173 |
| Performance | ffa / close-combat | 5.17 / 4.74 / 4.51 | 6.81 | 160 | 1103 |
| Performance | ffa / traversal | 4.50 / 3.41 / 3.22 | 5.97 | 162 | 1101 |
| Performance | teams / plains | 5.08 / 4.65 / 4.54 | 6.81 | 164 | 1117 |
| Performance | teams / forest | 4.39 / 3.45 / 3.19 | 5.73 | 167 | 1109 |
| Performance | teams / pond | 4.35 / 3.45 / 3.44 | 5.81 | 172 | 1112 |
| Performance | teams / close-combat | 5.17 / 5.24 / 4.80 | 6.82 | 173 | 1119 |
| Performance | teams / traversal | 4.49 / 3.43 / 3.20 | 5.89 | 174 | 1112 |

Standard retains 502 tree instances, 31,731 grass instances and 523 fern
instances in existing HISM components. Performance has zero cosmetic tree,
grass and fern instances. Both retain 36 edible plant locations, terrain,
water and rocks. Standard retains tree collision and 556 navigation obstacles;
Performance removes the tree obstacles along with the trees.

Across 7,523 timing samples: 2 exceeded 33.3 ms and 2 exceeded 50 ms.
Observed process memory ranged from 1101 to 1188 MB.
Memory includes generated assets, caches and persistent corpses. This bounded
run does not establish leak-free long sessions. Samples arrive about every
50 ms and can miss individual frame spikes; FPS uses actual frame-count/time
deltas. GPU/game/render values are engine counters, not a full profiler trace.
Draw-call counts are visible in post-window stat-unit screenshots only, not
a timed series. Screenshot capture occurs outside the
measurement windows. No claim is made about unmeasured weak-PC performance.

Evidence: `Tests/Results/alpha03/baseline/performance.json`,
`performance-after-standard/performance.json`, and
`performance-final/performance-matrix.json` / `summary.json`.

## Fresh processes at native 1920×1080

A separate process per map, 100% screen percentage, same three scene
definitions and species, ten live major combatants in team mode. These
results compare the current maps only, not the prior-build baseline.

| Scene | Standard FPS | Performance FPS | FPS increase | Standard / Performance GPU median ms |
|---|---:|---:|---:|---:|
| plains | 148.3 | 169.3 | +14.1% | 6.05 / 4.84 |
| forest | 147.2 | 184.2 | +25.1% | 6.16 / 4.23 |
| pond | 158.1 | 177.2 | +12.1% | 5.58 / 4.47 |

Evidence: `performance-1080-standard/performance-native-1080.json` and
`performance-1080-performance/performance-native-1080.json`.
