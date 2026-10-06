# Version 0.5 AI balance trials

These are actual engine AI fights, capped at 60 simulated seconds. Timeouts are not wins. Packs contain three animals; solo comparisons contain one per side. These small seeded samples do not establish competitive human balance.

| Study | Completed bouts | Result counts |
|---|---:|---|
| Initial roster | 24 | timeout: 13, Raptor: 3, T-Rex: 1, Pachycephalosaurus: 4, Triceratops: 2, Ankylosaurus: 1 |
| Contact and combo corrections | 48 | timeout: 24, Raptor: 6, T-Rex: 4, Pachycephalosaurus: 10, Triceratops: 4 |
| Bounded Anky positioning | 14 | T-Rex: 2, Triceratops: 2, timeout: 4, Raptor: 2, Pachycephalosaurus: 4 |
| Final Anky armor 0.60 | 7 | T-Rex: 1, Triceratops: 1, timeout: 2, Raptor: 1, Pachycephalosaurus: 2 |

The 48-bout matrix covers all 15 single-animal pairs plus nine natural-pack pairings, twice. All completed bouts retained valid health/stamina and grounding. A telemetry read timeout during concurrent cooking interrupted that matrix; saved bouts were retained and the remaining cases resumed against the equivalent packaged runtime. No crash was established by that timeout.

Ankylosaurus remained weak in the autonomous duels. The final armor change raises effective unbraced health from about 2,308 to 3,000 while preserving its speed, damage and club blind spot. In the same-seed sample, survival against Rex increased from 13.59 to 19.07 seconds, against Triceratops from 15.38 to 21.75 seconds, and against a Pachy pack from 5.42 to 9.62 seconds. These are illustrative runs with variable physics scheduling, not statistically significant estimates. It still lost those bouts; do not interpret this as solved tournament balance.

Packs can overwhelm isolated large animals; the scoring leader remains the vulnerable win target. Small solo animals often retreat instead of accepting an unfavorable fight. Brachiosaurus can hold space and sustain fights, but mobile attackers and defensive horn fighters remain effective counters.

| Matchup | Formation | Trial 1 | Trial 2 |
|---|---|---|---|
| T-Rex / Raptor | 1 vs 1 | timeout (60.42s) | timeout (60.31s) |
| T-Rex / Raptor | 1 vs 3 | Raptor (10.86s) | T-Rex (1.8s) |
| T-Rex / Triceratops | 1 vs 1 | timeout (60.44s) | T-Rex (15.91s) |
| T-Rex / Ankylosaurus | 1 vs 1 | T-Rex (13.28s) | T-Rex (10.58s) |
| T-Rex / Brachiosaurus | 1 vs 1 | timeout (60.45s) | timeout (60.3s) |
| T-Rex / Pachycephalosaurus | 1 vs 1 | timeout (60.23s) | timeout (60.29s) |
| T-Rex / Pachycephalosaurus | 1 vs 3 | Pachycephalosaurus (4.75s) | Pachycephalosaurus (10.91s) |
| Raptor / Triceratops | 1 vs 1 | timeout (60.38s) | timeout (60.22s) |
| Raptor / Triceratops | 3 vs 1 | Raptor (11.87s) | Triceratops (42.59s) |
| Raptor / Ankylosaurus | 1 vs 1 | timeout (60.22s) | timeout (60.02s) |
| Raptor / Ankylosaurus | 3 vs 1 | Raptor (15.87s) | Raptor (10.07s) |
| Raptor / Brachiosaurus | 1 vs 1 | timeout (60.34s) | timeout (60.47s) |
| Raptor / Brachiosaurus | 3 vs 1 | timeout (60.24s) | timeout (60.45s) |
| Raptor / Pachycephalosaurus | 1 vs 1 | timeout (60.33s) | timeout (60.22s) |
| Raptor / Pachycephalosaurus | 3 vs 3 | Raptor (44.59s) | Raptor (9.08s) |
| Triceratops / Ankylosaurus | 1 vs 1 | Triceratops (13.22s) | Triceratops (13.28s) |
| Triceratops / Brachiosaurus | 1 vs 1 | timeout (60.31s) | Triceratops (34.85s) |
| Triceratops / Pachycephalosaurus | 1 vs 1 | timeout (60.44s) | timeout (60.32s) |
| Triceratops / Pachycephalosaurus | 1 vs 3 | Pachycephalosaurus (4.52s) | Pachycephalosaurus (11.35s) |
| Ankylosaurus / Brachiosaurus | 1 vs 1 | timeout (60.14s) | timeout (60.04s) |
| Ankylosaurus / Pachycephalosaurus | 1 vs 1 | Pachycephalosaurus (18.2s) | Pachycephalosaurus (19.08s) |
| Ankylosaurus / Pachycephalosaurus | 1 vs 3 | Pachycephalosaurus (8.65s) | Pachycephalosaurus (7.24s) |
| Brachiosaurus / Pachycephalosaurus | 1 vs 1 | timeout (60.0s) | timeout (60.21s) |
| Brachiosaurus / Pachycephalosaurus | 1 vs 3 | Pachycephalosaurus (20.05s) | Pachycephalosaurus (19.04s) |
