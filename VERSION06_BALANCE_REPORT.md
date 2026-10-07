# Version 0.6 balance observations

## Final Albertosaurus values

| Value | Albertosaurus | Rex | Relative |
|---|---:|---:|---:|
| Health | 1,350 | 1,500 | 90% |
| Quick damage | 163 | 187 | 87.2% |
| Full heavy damage before defense | 578.65 | 673.2 | 86.0% |
| Run speed (cm/s) | 1,200 | 1,050 | 114.3% |
| Sprint speed (cm/s) | 1,740 | 1,449 | 120.1% |
| Acceleration (cm/s²) | 2,750 | 1,850 | 148.6% |
| Turn / pivot (degrees/s) | 205 / 145 | 130 / 95 | 157.7% / 152.6% |
| Stamina | 110 | 100 | 110% |
| Sprint drain/s | 13.5 | 15 | 90% |
| Quick / heavy stamina cost | 9 / 38 | 10 / 36 | Heavy commitment costs more |
| Quick duration / extra third-hit recovery | .53 / 1.08 s | .58 / .95 s | Responsive first bites; committed finisher |
| Heavy duration / added miss recovery | 1.30 / .55 s | 1.05 / .50 s | More punishable commitment |

Three uninterrupted full-strength quicks average roughly 190.5 damage/s for
Albertosaurus versus 216.9 for Rex, before defense, travel and input delays. Rex
also retains greater reach and width. Albertosaurus remains much tougher than
Raptor (520 health) and faster than the heavier herbivores; it eats meat and has
320 carcass food units. Brachi's combat/balance values were not changed.

## Playtest-adjust-playtest sequence

**77 real AI bouts** across four recorded studies:

| Study | Bouts | Evidence folder under Tests/Results/launch06 |
|---|---:|---|
| Initial Alberto matchup set, two repeats | 16 | balance-initial |
| Heavy commitment revision, three repeats including mirrors | 27 | balance-tuned |
| Rex comparison set, two repeats | 16 | rex-baseline |
| Final damage/health/cadence revision, two repeats including mirrors | 18 | balance-final |

The initial values were 1,380 health, 168 quick damage, 596.4 heavy damage,
.88 seconds extra combo recovery, 1.12 seconds heavy duration, .38 seconds drive
and 34 heavy stamina. They left sustained frontal damage too close to Rex while
adding substantial mobility. The first revision reduced heavy burst/drive and
increased its duration/cost. The final revision reduced health/quick damage,
increased third-hit recovery and heavy cost, and retained a roughly 86%-of-Rex
heavy payoff. Movement remained unchanged throughout.

Final outcomes exclude the two Alberto mirrors unless stated:

| Rival | Alberto wins | Alberto losses | Unresolved at 45 seconds |
|---|---:|---:|---:|
| Rex | 0 | 1 | 1 |
| Raptor solo / natural pack combined | 1 | 0 | 3 |
| Triceratops | 1 | 0 | 1 |
| Ankylosaurus | 2 | 0 | 0 |
| Brachiosaurus | 0 | 0 | 2 |
| Pachy solo / natural pack combined | 2 | 1 | 1 |
| Total non-mirror | 6 | 2 | 8 |

One mirror resolved and one timed out. The final Rex winner retained 1,337 health;
the other Rex bout remained unresolved. These observations support keeping Rex's
frontal advantage while retaining Alberto pursuit and repositioning. They do not
establish equal win rates. Anky remains vulnerable to fast aggressive predators;
its improved arc and genuine blind spots were preserved rather than inflating hits.

## Method and limits

Actual engine AI, collision, cooldowns, stamina, hunger, regeneration and movement
run continuously. Fixtures arrange two opponents or natural three-member packs,
remove unrelated fights, seed personalities, and stop at the first scoring leader
death or 45 simulated seconds. Timeouts are not wins; retreat and feeding can
restore health. No health resets occur during a bout. Every completed study passed
finite health/resource/grounding checks. Raw per-bout trajectories and outcomes
remain alongside each `matches.json`.

The simulation is real-time, not deterministic lockstep: frame timing can change
contacts even with the same random seed. The Rex comparison has its own matchup
seeds and is contextual, not a controlled causal estimate of a tuning change.
The final build still needs human competitive playtesting across real networks.
