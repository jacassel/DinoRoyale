# Sound and terrain checkpoint — September 20, 2026

Launch: double-click
`C:\Users\joel1\Documents\DinosaurBattle Prototype\LaunchGame.bat`.
The promoted standalone starts at 1600x900 and reached the selection menu. A
Windows Firewall prompt currently blocks the final native control check; the
user was asked to dismiss it with Cancel for this single-player prototype.
The agent did not act on the permission prompt or change security settings.
The fresh packaged mapped-input regression passed separately with the opt-in
development bridge. Native input/quit verification remains pending dismissal.

Source implementation: `c2ba3a1`; final source tag:
`sound-terrain-stable-20260920`. Playable: `Dist/Windows`.
Independent recovery: `Dist/Checkpoints/2026-09-20-sound-terrain/Windows`.
Previous playable: `Dist/Checkpoints/2026-09-20-before-sound-terrain/Windows`.
All 49 runtime files were SHA-256 verified across candidate, recovery and
promoted copies. The earlier visual-modernization recovery remains intact.
Dist is ignored by Git; source commits do not replace the package backup.

81 finite, varied CC0-based sound assets cover quick attacks, charge-up/heavy,
impacts, hurt, sprinting, ongoing injury and death for all three playable species.
Audio caps, attenuation and state-driven stops prevent uncontrolled overlap.
The user's video supplied cinematic direction; no video samples are shipped.
Credits: `Assets/Audio/CREDITS.md`, also included beside the packaged launcher.
Direct listening was unavailable to the agent; timing and output levels were
measured, but perceived realism/mix still need human listening. Preview reels
and packaged recordings: `Tests/Results/sound-terrain/review.html`.

502 trees and rolling hills; sampled interior high point increased from 9.61m
to 25.64m while preserving water basins. Trunks affect navigation, path shortcuts
check slopes, and a near-camera foliage mask repairs an observed leaf obstruction.
Combat values, controls and match rules are unchanged.

Verification: source gameplay 462/462; packaged gameplay 220/220; source and
packaged audio each 54/54; both packaged group mixes unclipped; 42/42 packaged
navigation routes. All eighteen source player traversals and three AI pond
crossings passed. A 240.52-second packaged world run recorded 179 hits, no failed
paths, falls, stuck recoveries or immediate respawn hits. First packaged core
attempt exposed audio-test fixture contamination; a fresh-process rerun passed
all assertions. Failed evidence and diagnosis are retained in `SOUND_TERRAIN.md`.

Warmed RTX 3060 packaged samples at 1600x900 averaged 59.49–60.00 FPS. One 169ms
pond hitch was sampled; this is not a full trace or a hitch-free guarantee.
Nine packaged map views reviewed. Evidence: `Tests/Results/sound-terrain`.

The preexisting staged `Config/DefaultInput.ini` is byte-identical to
`Tests/Results/sound-terrain/DefaultInput.before.ini` and excluded from source
commits. Final package checkpoint allowance: 63% remaining; no separate Astra
allowance exposed. No purchases, delegation or gameplay rebalancing.

Controls: 1/2/3 select Rex/raptor/Triceratops; WASD moves; mouse looks; Shift
sprints; Space jumps; Q braces; LMB attacks; hold/release RMB charges; hold E eats.
M opens/closes the map; point and R adds/removes a pin. Escape pauses; F10 in
the menu quits. See README for match/settings details.

## Previous visual modernization checkpoint — September 20, 2026

Double-click `C:\Users\joel1\Documents\DinosaurBattle Prototype\LaunchGame.bat`.
The launcher was verified on the promoted standalone at 1600x900, without a
development bridge or permission prompt. Native species selection, mouse look,
map, R pin, pause and F10 quit worked. Movement/sprint/jump inputs were exercised;
their quantitative behavior is covered by the regression suites. The game exited
normally and both launch processes closed.

Source checkpoint tag: `visual-modernization-stable-20260920`.
Playable: `Dist/Windows`.
Independent recovery: `Dist/Checkpoints/2026-09-20-visual-modernization/Windows`.
Previous playable: `Dist/Checkpoints/2026-09-20-before-visual-modernization/Windows`.
All 48 runtime files were SHA-256 verified across candidate, recovery and promoted
copies. Dist is ignored by Git; the recovery package is a separate local backup.

Blender sources: `Assets/Source/DinosaursModern` and `WorldModern`. All three
species have new skin color/normal/roughness maps, improved eyes and varied teeth;
carnivores have added tongues. Fuller conifers and curved grass complement new
soil, bark, stone, wet banks, layered water, restrained lighting and atmosphere.
Original sources, rigs, all 33 animation durations and gameplay rules are retained.
The preexisting staged `Config/DefaultInput.ini` remains byte-identical to
`Tests/Results/visual-modernization/DefaultInput.before.ini` and is excluded from
this source commit.

Verification: source 462/462 across 15 suites; actual package 220/220 across six
suites; blood/material checks 12/12; jaw probe 86/86 frames; 51 detail captures,
nine species-angle views and 27 packaged combat captures reviewed. A four-minute
packaged world run recorded 187 hits with no failed paths, falls, stuck recoveries
or immediate respawn hits. Evidence and rejected iterations are retained under
`Tests/Results/visual-modernization`; see `VISUAL_SPRINT.md` and `TEST_LOG.md`.

RTX 3060 at 1600x900: warmed packaged scenes averaged 58.81–60.00 FPS at the 60 FPS
cap. One 194.8ms pond hitch was sampled; a 45-second follow-up averaged 59.94 FPS
without sampled frames above 50ms. These are bounded samples, not a full trace.
Anatomy, fern/rock silhouettes, clip transitions/sliding and distant repetition
remain procedural. Menu portraits retain the prior artwork. Water reflections
have screen-edge limitations. This is an improvement, not reference-level realism.

Controls: 1/2/3 select Rex/raptor/Triceratops; WASD moves; mouse looks; Shift
sprints; Space jumps; Q braces; LMB attacks; hold/release RMB charges; hold E eats.
M toggles the map; point and R adds/removes a pin. Escape pauses/releases mouse;
F10 in the menu quits. See README for match and settings details.

Final reported account allowance: 70% remaining; no separate Astra allowance
exposed. No purchases, delegation or gameplay rebalancing in this sprint.

## Previous map and results update — September 20, 2026

Latest update: all living team allies have current-position markers on both maps,
including followers and while the human waits to respawn. Enemy/solo concealment
is unchanged. Open M, point, and press R to add/remove a gold pin (maximum eight;
ninth replaces oldest). Pins survive respawn and clear on a new round. R no
longer forces death. M closes the map and restores movement/mouse look.

Solo results at five kills show all ten competitors' kills/deaths/assists for
human or AI victories. Results pause the game; Enter starts a clean round.
Hidden dinosaur-selection cards can no longer be activated through results.

Standalone build succeeded; 90/90 packaged checks passed: allies 17, map pins
and solo results 21, existing match rules 24, settings/water/blood 28. Four
rendered screenshots were reviewed. Evidence: `Tests/Results/map-results-update`.
Playable: `Dist/Windows`; recovery: `Dist/Checkpoints/2026-09-20-map-results/Windows`.
Prior playable retained at `Dist/Checkpoints/2026-09-20-before-map-results/Windows`.
All 48 runtime files were SHA-256 verified across candidate/recovery/promotion
staging. Source tag: `map-results-stable-20260920`. Pins are local, round-only
markers; AI teammates do not follow pins. Account allowance at final checkpoint:
83% remaining; no separate Astra allowance exposed. No purchases or delegation.

## Previous quality checkpoint

Double-click `C:\Users\joel1\Documents\DinosaurBattle Prototype\LaunchGame.bat`.
It starts the verified standalone package in `Dist\Windows` at 1600x900.
Choose 1 / 2 / 3 for Rex / raptor / Triceratops. WASD moves; mouse looks;
Shift sprints; Space jumps; Q braces; LMB attacks; hold/release RMB charges;
hold E eats. Escape pauses and releases the mouse; F10 in the menu quits.

Triceratops edible plants now have a subtle green-gold silhouette glow.
Decorative plants and carnivore views are unmarked; depleted plants lose the
cue and regain it on their 120-second regrowth. Geometry occludes the cue.

Final values: Rex 1500 health, 187 quick, 673.2 heavy; raptor 520 health,
66.6 quick, 193.14 pounce, 1.5x base hunger, one leader plus two followers.
Triceratops retains 1650 health, with 155 quick / 511.5 heavy to maintain
its offensive relevance. Mobility and stamina costs remain species-specific.
Practical map travel span is half the previous scale, roughly 575m across.
Movement, attack-motion, impact and once-only death sounds are implemented.

Retained 24-trial Rex/pack validation: 10 Rex leader-kills surviving the
20-second follow-through, 7 pack kills of Rex (including 4 counterkills),
7 disengagements at 90 seconds. Resolved survival outcomes: 58.8% / 41.2%.
First scores alone were 14 Rex / 3 pack; only 3 trials wiped every raptor.
Do not confuse a surviving scoring victory with complete pack elimination.
Triceratops sanity: 1 win / 3 losses against Rex; pack trials had one first
score each and two disengagements, with a pack counterkill after the Trike score.
Human competitive balance is not proven by these bounded AI samples.

Verification: 460-check full regression; all-species six-region traversal;
42 directed navigation routes; swimming and crowded AI pond exits; five jaw/map
visual captures; real audio-output recordings; three initial clean four-minute
world scenarios and one final clean four-minute run. The final package passed
129 targeted checks. The final world run recorded 197 hits, zero path failures,
terrain falls or stuck recoveries, approximately 60 FPS, and no immediate
respawn hits. All three targeted respawns were ten seconds and over 32m from
an enemy camping the home point. Earlier failed tests and rejected tunings are
retained and explained in TEST_LOG.md.

The native standalone rendered its selection menu. Windows then displayed a
firewall permission prompt. The extra desktop input check stopped there;
automated real-input control checks passed. Click Cancel if this network prompt
appears; this local single-player game does not require network permission.
No security permissions were changed by the agent.

Recovery package:
`Dist\Checkpoints\2026-09-20-quality\Windows\DinosaurBattle.exe`.
Keep the entire Windows directory together. All 49 runtime files were SHA-256
compared across the tested package, promoted playable and recovery copies.
Manifest: `Tests\Results\quality-sprint\stable-package-manifest.json`.
Previous playable retained at `Dist\Checkpoints\2026-09-20-before-quality\Windows`;
the September 19 stable recovery copy also remains intact.

Source tag: `quality-sprint-stable-20260920`. Dist is ignored by Git; the local
recovery folder is the playable-package backup. No cloud backup was created.
Known limits: procedural visuals/animation transitions and body overlap,
synthetic prototype audio, session-only carcasses, and indefinite-session corpse
accumulation/human balance still needing playtesting. See KNOWN_ISSUES.md.

The bounded sprint is complete. No further development or background tests
should run after the completion report.
