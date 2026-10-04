# Dino Royale 0.3 Alpha Test — interactive close-combat checks

October 4, 2026. Tested the published RC4 runtime on Standard at 1600×900.
No game code or package changed during these checks.

## Method and limits

Computer Use operated the real game window: species selection, pause/quit,
native mouse attacks, and keyboard taps. The firewall dialog was absent; the
agent did not change security settings. Opponents were placed 3.18m away and
about 62 degrees left of the player using the opt-in development fixture.
Other bots were moved away. The player was invulnerable so deliberation between
tool calls did not interrupt the control checks; these are not balance results.

The desktop API provides key taps, not timed key-down/key-up. Q/W native taps
produced no measurable movement in this session, so **held pivots, W movement and
charged attacks used the game's mapped-input development bridge**. Native mouse
clicks initiated the quick attacks. Screenshots of each matchup were inspected
through Computer Use, alongside recorded telemetry. This is hybrid interactive
verification, not a claim of continuous human keyboard/mouse play or polished
animation feel. Human playtesting remains valuable.

## Results

| Matchup | Stationary left pivot | Camera yaw change during pivot | Native quick-hit damage | Turn during heavy's sampled 0.25s |
|---|---:|---:|---:|---:|
| Rex / Rex | 61.8° | 0° | 187 | 5.5° |
| Rex / Triceratops | 66.5° | 0° | 187 | 5.1° |
| Triceratops / Rex | 86.3° | 0° | 155 | 7.5° |
| Raptor / Rex | 66.5° | 0° | 66.6 | 0° |
| Raptor pack / Triceratops | 66.5° | 0° | 66.6 | 0° |

All held pivots kept the player at its original X/Y and retained the resulting
facing after release. The traces select the PivotLeft animation. Redirected native
quick attacks hit the adjacent enemy without a running circle. Moving the pointer
to click changed Rex/Rex camera yaw by about 0.9°; that occurred after the pivot
and is ordinary mouse look, not pivot-driven camera motion.

W returned control to locomotion and camera-relative orientation. Rex and raptor
settled to camera yaw in the recorded movement windows; Triceratops moved and
reduced its yaw gap from 76.9° to 14.9° in the short 0.45s window. An initial Rex
movement request overlapped heavy recovery and remained constrained; the recovered
request aligned normally. Heavy commitment is intentionally retained.

After setup, opponents were enabled for live combat. The final pack view showed
two nearby friendly followers. Both acquired the human leader and fought the
Triceratops; the encounter recorded a leader kill and the enemy respawning while
the two followers remained alive. See the live snapshots for cumulative counters;
they must not be treated as isolated match totals.

The first pack fixture incorrectly searched for network follower flags in an
offline round and left followers far away. Its `pack-trike-*` records are retained
as a rejected fixture, not the passing pack result. `pack-final-*` starts a normal
offline round and places its actual two raptors nearby. No pack code changed.

Visible limitations: body intersections at extremely close spacing, procedural
pose transitions and foot sliding remain. Pivot direction, fixed camera and
attacks were readable; the visuals still need polish. This bounded session does
not establish competitive balance, long-session stability or fresh EOS/WAN play.

Evidence: `Tests/Results/alpha03/native-acceptance`. Fixture helper:
`Tools/Tests/alpha03_interactive_fixture.py`. It requires the packaged runtime
started with `-DinoDevBridge -DinoBridge=NativeAcceptance`; ordinary launch uses
neither flag. The final game was restarted through LaunchGame.bat afterward.
