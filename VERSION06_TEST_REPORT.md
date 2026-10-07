# Version 0.6 verification

## Accepted evidence

**595/595 selected checks across 20 suites**. Exact paths, expected counts and
results are in `Tests/Results/launch06/acceptance-summary.json`; regenerate with
`python Tools/Tests/summarize_launch06.py`. Earlier failures, retries and wrapper
results remain recorded but are not counted again.

Windows Candidate 2 is the final gameplay build. Candidate 1 has the same compiled
gameplay code and art; Candidate 2 contains the final Alberto balance values.
Candidate 1 supplies original controls, map/results, settings, safe respawn,
90 match-rule checks and seven-card UI coverage. Candidate 2 repeats Alberto
controls, all four affected species' combos/survival, ecology/pack fixes, delayed
multiplayer, rendered guest animations, mixer checks, final camera motion,
restart/bot-count UI and endurance. Earlier editor runs retain original swimming,
ecology, scoring/diets and Anky/Pachy material/trajectory coverage. Final Brachi
camera motion supersedes its earlier animation measurements.

Checks exercise real Unreal movement, physics, damage, stamina, AI, replication
and mapped controls. The development bridge arranges fixtures and reads telemetry;
it is not a substitute implementation of gameplay. Rendered screenshots inspect
normal camera distance, materials, menus, attacks, swimming and blood settings.

- Original Rex/Raptor/Trike controls: 90; final Alberto controls: 30. Movement,
  mouse look, jump, sprint, stamina, quick/heavy/brace, eating, health, death and
  ten-second respawn are covered.
- Configuration: 90. Offline and multiple actual network processes cover goals
  5/10/15, thresholds/rematches, single/multiple/only-one bans, zero-roster denial,
  packs, restricted respawn, guests rejected by authority, travel, late joining,
  independent zero/unequal bot counts and capacity warnings without deleting humans.
- Final combat/survival/gameplay: 114. Three-hit commitment, visible club contacts,
  misses/recovery, Brachi feet, mobility differences, meat diet, starvation/regen,
  carcass food, brace/armor, exhaustion, injury, water, optional blood and pack scores.
- Three-process gameplay: 39, with **60 ms one-way injected lag and 2% loss**.
  Seven-species selection, attacks, critical movement restrictions, swimming,
  eating, scoring/assists and team/FFA results synchronize. Another 14 rendered
  checks inspect guest setup and Anky/Brachi/Alberto animation replication.
- Mixer/events: 10. Alberto uses the 162-clip existing bank with a raised Rex pitch.
  Recorded audio is nonzero and unclipped (peak 0.2705; RMS 0.0259). Direct auditory
  judgment was unavailable; realism and mix still require human listening.

## Iteration and fixes

The flat generic horn material ignored the baked armor atlas. Correct material
assignment, color/normal detail and restrained geometry define Anky's club and
Pachy's dome. Anky's third quick now strikes with its real club rather than the
shoulder. Wider braced animations and anatomical AI spacing preserve genuine
contact limits. Brachi retains its balance values; neck lean and forefoot weight
transfer make its existing strikes more visible from the rear camera.

Offline leaders/followers now share multiplayer's participant lifecycle. Fixes
include proper follower ownership, bans removing follower packs, no extra follower
death rows, actual bot counts when first editing after bots are off, assigning the
human a valid team after a saved Team-mode restart, and validating Escape/resume
instead of bypassing an invalid setup. A valid local setup change begins a new round.

**77 AI bouts** cover initial values, a heavy-commitment revision, Rex comparisons
and final damage/health/cadence tuning. Final non-mirror Alberto results were six
wins, two losses and eight timeouts. See `VERSION06_BALANCE_REPORT.md`; these are
bounded observations, not established competitive win rates.

Failures were diagnosed rather than hidden: one initial Trike respawn movement
check passed isolated terrain investigation and full/packaged retests. A packaged
bone name differed only in capitalization; the trajectory fixture now compares
case-insensitively. A restart UI window extended beyond the desktop, so its mouse
coordinates were clamped; placing it on-screen passed. One isolated blood test hit
a newly spawned pack follower shielding the target; the fixture now parks unrelated
followers. A delayed client Ready message correctly blocked starting; the network
fixture now waits for server Ready and all peers' round start. No gameplay bypass
was added for these fixture failures. The unity-build color-name collision was
fixed; editor build 8 and both Windows packages succeeded.

## Endurance and performance

Four **120-second** rendered mixed-roster scenarios cover Standard/Performance
maps and FFA/Teams, deliberately including all seven leaders for this stress
fixture. All passed with 18 edible trees, zero failed paths and no below-terrain
living actors. Persistent session carcasses reached 63. The largest movement stall
was 17 consecutive roughly 0.8-second samples in Standard Teams; it recovered within
the bounded run. Hit/death/carcass counters can be cumulative and must not be summed
as independent totals. This does not establish overnight stability.

Performance evidence: `candidate2-soak/performance-1080.json` under the evidence
root. One rendered game process on the RTX 3060 PC, 1920x1080, 100% resolution,
uncapped, four-second settling plus twenty seconds per scene. These samples run
after endurance with persistent corpses and live AI; roster/pack size can vary.
The normal shipped 60 FPS cap is unchanged. Thread/GPU counters report zero and
are unavailable, not zero-cost work. Sampled frame percentiles can miss spikes.

| Map | Plains FPS | Forest FPS | Pond FPS |
|---|---:|---:|---:|
| Standard | 119.4 | 124.3 | 130.4 |
| Performance | 121.1 | 131.0 | 142.8 |

Sampled 95th-percentile frames range from 8.20 to 9.95 ms. These are practical
bounded samples, not an identical-roster before/after performance benchmark.

## Scope and recovery

This is separate-process local development-socket verification, including injected
lag/loss, **not a fresh two-PC EOS/WAN test**. The owner's prior successful WAN
play is separate evidence. EOS identity/provider integration is preserved, and
compatibility is now **2026100706**. No firewall, security, portal or paid-service
changes were made. Native bootstrap acceptance and exact package/source hashes
are recorded in `Tests/Results/launch06/release` after sealing.

The normal Candidate 2 bootstrap, without development bridge flags, reached the
Version 0.6 menu. Windows Security's network-permission prompt covered the window;
no permission decision was automated. Foreground native input inspection remains
limited by that prompt, separately from the successful rendered mapped-input tests.

All 61 files in both the existing 0.5 release and independent playable recovery
match its published manifest; its complete source bundle verifies. The owner's
renamed 0.5 public-distribution ZIP is preserved and was not substituted for that
file-by-file verification. The original Trex Blender edit was checkpointed in
`719941e` and remains unchanged. Version 0.6 receives its own release, independent
playable recovery, source bundle and hash-verified ZIP; Dist remains Git-ignored.

Procedural anatomy, close body intersections, limited clip blending/foot placement,
Anky's vulnerability to agile predators, human animation feel and long-session
pacing remain limitations. No purchases, resets or subagents were used.
