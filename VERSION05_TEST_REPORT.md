# Version 0.5 verification report

The selected regression set passes **845/845 checks across 30 suites**. Two
additional Unreal automation tests pass 32 compatibility/EOS-URL assertions.
Completed balance studies contain **93 AI bouts**, including the full two-repeat
solo/natural-pack matrix. Four rendered three-minute endurance scenarios pass
with all six species, 18 edible trees, no failed AI routes and no below-terrain
actors; persistent carcasses reached 61. This is bounded automated playtesting,
not an overnight human session or proof of competitive balance.

Editor compilation and the final Windows Development package succeeded with
Unreal 5.8.2. Final candidate: `DinoRoyale-0.5-Candidate7`.
Runtime SHA256: `8d4e912c2b7d488cc348e151e85adaef6ab0dd57251774f8e3df836fa671f1ef`.
The final package rebuilt the executable after the armor configuration change;
it is not byte-identical to Candidate 6. Candidate 7 repeats survival, Anky
controls, audio, combo, roster/scoring rules and multiplayer roster checks.
Earlier evidence covers unchanged systems and is identified in the summary.

The new roster uses original Blender source, rigged meshes, textured materials,
sixteen clips per new animal and 81 new sound assets. Existing species IDs remain
stable; the network compatibility identifier is 2026100505.

## Evidence and method

Tests launch real Unreal game processes. The opt-in local development bridge
arranges fixtures and sends mapped player inputs through the real controller;
movement, combat, health, hunger, AI, replication and scoring run in the game.
Rendered runs and saved screenshots cover appearance and controls. Headless AI
bouts are gameplay simulations, not rendering-performance measurements.

The selected acceptance runs are listed in
`Tests/Results/roster05/acceptance-summary.json`; failed attempts and corrected
fixtures remain in adjacent folders. Repeated checks are not counted again in
that summary. Earlier candidate tests remain relevant where the tested systems
were unchanged; final-candidate tests are identified by directory.

The complete AI matrix separates single animals from natural three-member packs.
It records the first leader death, actual health/stamina, contact counts, path
failures and recoveries. A 60-second unresolved fight is a timeout, not a win.
Trials use seeded personalities, but real engine scheduling and physics still
vary. These are bounded balance trials, not statistical proof of equal win rates.

## Final-candidate performance

Measured on this PC: Intel i7-11700, 32 GB RAM, NVIDIA RTX 3060, D3D12 SM5.
One rendered game process, 1920 x 1080, 100% screen percentage, uncapped for
measurement, audio enabled and the ordinary ten-combatant six-species roster.
Each scene warmed for four seconds and sampled for twenty. Other desktop apps
remained open. The normal shipped 60 FPS cap is unchanged.

| Scene | Standard FPS | Performance FPS | Standard sampled p95 frame ms | Performance sampled p95 frame ms |
|---|---:|---:|---:|---:|
| Plains | 131.4 | 148.4 | 8.37 | 7.68 |
| Forest | 128.1 | 162.0 | 8.76 | 7.67 |
| Pond | 143.4 | 158.3 | 8.00 | 7.91 |

All six scenes retained 18 edible trees and ten major combatants. Frame samples
are sparse telemetry, not a complete frame-time trace. CPU-thread/GPU timing
fields returned zero and are unavailable, not evidence of zero cost. These
short scenes do not predict minimum FPS on other PCs or very long sessions.
Evidence: `Tests/Results/roster05/candidate7-performance/performance-1080.json`.

## Repairs found through testing

- Ankylosaurus capsule height now agrees with its collision radius, removing the
  extra ground offset.
- New AI aims the active anatomical strike, accounts for large collision radii,
  and uses the actual combo-reset timer. This fixed Brachiosaurus waiting forever
  for a tail opportunity and reduced large-body approach stalls.
- Brachiosaurus's heavy attack uses both forefeet even after quick combo three.
- Ankylosaurus attempts a short, cooldown-limited spacing step when an attacker
  moves inside the club arc. An unbounded experimental step was rejected because
  fast enemies could keep it retreating. Its narrow club blind spot remains real.
- A focused armor trial increased Ankylosaurus survival in the sampled close
  fights. Final incoming-damage multiplier is 0.60 (was 0.78), giving 3,000
  effective health before brace while leaving speed, damage and real club
  reach unchanged. It still lost the resolved AI duels; this is a survivability
  adjustment, not a claim that competitive balance is solved.
- Every offline selection now retains all six species, with exactly one leader
  and two followers for each pack. The first soak caught a missing Rex when the
  human replaced its original roster slot.
- The network regression fixture now waits for server-observed Ready under
  latency and expects assist-based team victory. Combo tests respect intentional
  late-input buffering rather than mistaking a queued strike for a bypass.
- Windows normally mutes an unfocused game. Mixer recording uses a process-local
  test override; no shipped audio preference or Windows security setting changed.

## Multiplayer and online configuration

The owner confirms successful prior multiplayer across multiple different
networks. That working EOS implementation and service identity are preserved.
Current regression tests use separate packaged processes over development
loopback sockets, including injected 75 ms per-peer delay and 2% packet loss.
They are additional correctness evidence, not a new WAN/EOS session.

The owner's actual supplied OnlineServices.ini was copied unchanged to the
project and local packages. Hash equality is checked without printing values.
Configured credentials and Saved/authentication caches are excluded from the
public ZIP. Every PC must use the same Version 0.5 compatibility build.

The requested Windows Security network-permission dialog appeared during native
inspection. It was left for the owner to handle, as requested. The dialog limits
native foreground mouse/keyboard acceptance until dismissed; automated rendered
input/physics tests continue independently. No firewall settings were changed.

## Remaining limits

AI retreats, healing and positioning can leave bouts unresolved. Ankylosaurus's
slow turning and rear club make it especially vulnerable when rushed or flanked;
these trials do not establish competitive human balance. Animation uses explicit
clips without full foot placement/blending, so close-body intersections and
sliding remain. Sound events, waveforms and mixer levels were verified; the agent
could not directly listen to judge realism. See KNOWN_ISSUES.md for broader limits.
