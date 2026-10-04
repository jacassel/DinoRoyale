# Dino Royale 0.3 Alpha Test - development checkpoint, October 4, 2026

The owner has authorized this major update and resumed development. The historical stop note below applies only to the September QA2 session.

Current branch: `codex/dino-royale-alpha-0.3`. Current subsystem checkpoint: `alpha03-systems-tested` (controls, pivot clips, grounding, map variants, AI slots, foliage LODs, swim correction and prey priorities). Broad regression and release work continue from this checkpoint; preserve subsequent working changes. See the newest TEST_LOG entry and `Tests/Results/alpha03`.

Latest verified source checkpoint: `alpha03-navigation-settings-tested`, following
`alpha03-regression-tested` (`5710de5`, full source 478/478). First package also passes
478/478 plus broad multiplayer checks. Subsequent stress/UI testing fixed unreachable
retreat choices, eight disconnected navigation-grid cells, and custom settings saving.
Targeted routes (including all 42 landmark pairs per map) and six UI checks pass.
RC2 lacks the last navigation/settings changes; RC3 is the next candidate. Preserve
all candidates and the prior known-good QA2. Native play is blocked by a Windows
firewall dialog requiring user action; no security setting has been changed.

Keep the working prior package at `Dist/Releases/DinoRoyale-20260926-QA2-ForCairnFriends/Windows` and its independent QA2 recovery. The old launcher still points to a missing unsuffixed QA2 folder and must be updated only after the 0.3 package passes. The first 0.3 package exists at `Dist/Releases/DinoRoyale-0.3-Alpha-Test/Windows`; release verification is ongoing. No GitHub push exists yet.

Build: `powershell -ExecutionPolicy Bypass -File Tools/Build.ps1`.
Development runtime: `powershell -ExecutionPolicy Bypass -File Tools/Tests/StartTestGame.ps1`.
New compatibility identifier: `2026100303`; both peers must use 0.3. EOS transport and service identity remain unchanged. The owner reports prior real multiplayer works; this update's loopback tests do not establish a fresh WAN result.

Source regression 478/478, prey priorities 28/28, requested team configurations 25/25 and source multiplayer suites pass. Delayed stationary pivot replay and an offline pause cooldown bypass are fixed. Pending: package regression, broader grounding/carcass cases, measured Standard/Performance comparisons, visual native playtest, final recovery/ZIP, final docs/launcher, and private `DinoRoyale` GitHub push. Computer Use's Chrome URL policy stopped that browser attempt; do not bypass it. Native game QA is separate. Account remaining 90%; no Astra-specific allowance exposed. One agent; no purchases.

---

# Dino Royale QA2 handoff - September 26, 2026

**Live goal pursuit stopped at the owner's request. Do not resume live QA without
a new request.** Both Epic accounts signed in. Earlier in the same session, real
EOS discovery, JoinSession, ClientTravel, remote PostLogin and a shared match were
observed on one PC. Physical two-PC/WAN gameplay acceptance is still pending.

Send the whole `Dist/Releases/DinoRoyale-20260926-QA2.zip` to the brother's PC and
extract it into a fresh folder on BOTH computers. Launch `Windows/Play Dino Royale.bat`.
The workspace `LaunchGame.bat` now prefers QA2. The ZIP deliberately excludes the
configured `OnlineServices.ini`: copy each computer's existing file into the new
`Windows/DinosaurBattle/OnlineServices.ini`, without changing its values.
On this PC the original remains at
`Dist/Releases/DinoRoyale-20260924-QA1/Windows/DinosaurBattle/OnlineServices.ini`.
The temporary QA2 test copy is archived in `Dist/TestRuns/QA2-local-test-config`.
Do not copy Saved, authentication caches, the repository or a loose executable.

Read `FRIEND_QUICKSTART.md` for the exact physical test; `MULTIPLAYER_QA_REPORT.md`
for root causes, evidence and limitations; and `RELEASE_READINESS.md` for the
remaining college Epic test and optional Steam plan, with official references.
Both reports are included in the package. No purchases or Epic portal/name changes
were made. Player-facing branding is Dino Royale; stable internal paths remain
DinosaurBattle.

QA1 fixed EOS Int64 compatibility metadata being misread as Int32/zero. QA2 fixes
the separate raw EOS prefix guard rejecting Unreal 5.8's bracketed `[EOS:PUID]`
travel URL. It uses FURL/FInternetAddrEOS and the successfully joined named session.
Compatibility stays **2026092201**; all existing version checks remain. Different
release labels and hashes distinguish QA1 and QA2 despite their shared number.

Verification: full editor/package rebuild; **32 assertions / two suites PASS**
on the final package; **128/128 packaged loopback** multiplayer checks and
**90/90 rendered offline** checks before the final diagnostic-only adjustment.
Live EOS logs show host PreLogin acceptance and remote PostLogin players=2,
plus guest NetMode=3. EOS combat/rehost/invite acceptance was not completed.
The final normal launch reached the game world according to logs; Computer Use
was stopped with Escape, so no new visual/input pass is claimed for that launch.
Tracked smoke processes were cleaned up without further UI input.

Source checkpoint: `eos-qa2-ready-for-two-pc-20260926`.
Pre-change source: `eos-qa2-before-20260926` at 0acde06.
Independent playable recovery: `Dist/Checkpoints/DinoRoyale-20260926-QA2/Windows`.
Final evidence: `Tests/Results/eos-qa-20260926/checkpoint.json`,
`release-manifest.json`, `package-integrity.json`, `live-eos-summary.json`.
The checkpoint verifies files in both the ZIP and recovery copy; Saved data is
archived under ignored Dist/TestRuns. Keep the package backups: Dist is not in Git.
QA1, its ZIP, its independent recovery and original Dist/Windows remain preserved.

Preexisting staged `Config/DefaultInput.ini` remains byte-identical and outside
the agent commit. Single agent; no delegation or usage-reset redemption.
Final checkpoint allowance: **94% remaining**. No Astra-specific allowance is
exposed; the reported account allowance is used.

---

# Historical QA1 handoff - September 24, 2026

The false version rejection is repaired and the replacement is locally tested.
**The goal is still open until the owner visually verifies both two-PC join paths.**
No further gameplay scope should be added before that test.

Launch `C:\Users\joel1\Documents\DinosaurBattle Prototype\LaunchGame.bat`.
It prefers the fresh QA1 release. Send the WHOLE folder:
`C:\Users\joel1\Documents\DinosaurBattle Prototype\Dist\Releases\DinoRoyale-20260924-QA1\Windows`
Or send `Dist/Releases/DinoRoyale-20260924-QA1.zip` and extract it on both PCs.
Do not merge into old installs. Launch `Play Dino Royale.bat` in the release.

Compatibility remains **2026092201**; release label **QA1** distinguishes it from
old packages displaying the same number. EOS returns custom integers as Int64;
the previous Int32 getter turned the correct value into zero. Shared typed
reading now fixes both invite and browser gates; missing/different IDs remain
rejected. OSS BuildUniqueId is explicitly aligned; engine network checks remain.
Invite Friends uses the implemented EOS ShowFriendsUI instead of the stub.
No Epic organization/product names or settings changed. Current UI/docs use Dino
Royale; internal executable/project/EOS artifact paths remain DinosaurBattle.

Verified: full editor/package build, 17 compatibility assertions in editor and
package, 14/14 packaged loopback lobby tests, 90/90 rendered offline core checks.
Normal 1600x900 launch, native selection/map/menu/multiplayer navigation and F10
exit passed without a development bridge. Original interrupted/incorrect-fixture
runs are retained and explained in TEST_LOG.md and MULTIPLAYER_QA_REPORT.md.

Live replacement-package EOS checks are pending manual Epic sign-in. The sign-in
screen appeared; it timed out awaiting user input. The Computer Use skill forbids
agent interaction with authentication dialogs. Previous physical screenshots
confirm the earlier build's sign-in/hosting/invite delivery, not successful joining.
The original public Refresh timeout has no guest log and remains unresolved;
new diagnostics distinguish timeout, callback and metadata rejection.

Retest: both accounts sign in; host FFA / 2 slots / bots OFF / Public. Test an
Epic invite, verify **2 of 2** on both screens, guest dinosaur/Ready and Start Match.
Recreate and repeat through Refresh -> select -> Join Selected. Preserve BOTH
`Windows/DinosaurBattle/Saved/Logs` folders on failure; Collect QA Logs.bat helps.
Then test different internet connections. Broader college playtesting waits for
the owner's visual acceptance plus Epic branding and account-access approval.

Previous playable stays at `Dist/Windows`. Independent prior backup:
`Dist/Checkpoints/2026-09-24-before-eos-qa/Windows` (71 hashes verified).
New recovery: `Dist/Checkpoints/2026-09-24-eos-qa-fixed/Windows`.
Manifest/checkpoint evidence: `Tests/Results/eos-qa-20260924/checkpoint.json` and
`release-manifest.json`; the checkpoint script verifies every listed file in
both the recovery copy and ZIP. Saved data, logs and authentication caches are
moved to ignored Dist/TestRuns, not shipped. The EOS client configuration remains
in its required packaged location; no credentials were put in source control.
Source tag: `eos-qa-ready-for-two-pc-20260924`.

Preexisting staged Config/DefaultInput.ini is byte-identical and excluded from
agent commits. Final account allowance: 38%; no Astra-specific allowance exposed.
One agent, no delegation, no purchases or usage-reset redemption.

---
# EOS timeout repair checkpoint — September 22, 2026

Launch `C:\Users\joel1\Documents\DinosaurBattle Prototype\LaunchGame.bat`.
Current package: `Dist/Windows`. Independent recovery:
`Dist/Checkpoints/2026-09-22-eos-timeout-fix/Windows`.
Previous package: `Dist/Checkpoints/2026-09-22-before-eos-timeout-fix/Windows`.
Earlier EOS and standalone checkpoints remain intact. All 54 runtime files
(excluding Saved) match SHA-256 in candidate, recovery and active copies; see
`Tests/Results/multiplayer/timeout-fix-package-checkpoint.json` and its manifest.
Source tag: `eos-multiplayer-timeout-fixed-20260922`.

A real silent-client timeout exposed a bug: the host treated a single guest's
connection failure as a whole-session failure. The repaired handler leaves
per-client cleanup to Unreal, preserving the host and healthy guests. The
replacement passed 13/13 packaged failure checks, including silent host loss,
bot/pack replacement, reconnection, ending-match rejection and unreachable host.
It also passed 16/16 survival checks: starvation and respawn, finite food,
heavy-attack interruption/knockback in both directions and actual 120.203-second
plant regrowth replicated to the guest. Editor and packaged builds succeeded.
The test harness now retries a brief Windows command-file sharing conflict.

The promoted normal launcher reached the 1600x900 menu without a development
bridge. A Windows Firewall prompt prevented further native input verification;
no permission was changed. This smoke launch was closed by process cleanup.
Actual mapped gameplay inputs passed in the two new packaged suites. The
previous broader offline, ten-process and latency results below remain relevant;
they were not all repeated for this narrow timeout-handler change.

**Live EOS authentication, discovery, invitations, relay and different-network
gameplay remain NOT VERIFIED.** No EOS product configuration exists and the
developer portal is signed out. Follow `EOS_SETUP.md`, add the five values in
`Dist/Windows/DinosaurBattle/OnlineServices.ini`, then use the second PC on a
different connection with a second authorized Epic account. Send friends the
entire `Dist/Windows` folder. Offline play works without EOS setup.

User input file remains byte-identical to the preserved baseline and excluded
from this commit. Final account allowance: 44% remaining; no Astra-specific
window exposed. One agent; no purchases, deployment or security changes.

---

# Earlier EOS multiplayer checkpoint — September 22, 2026

Launch `C:\Users\joel1\Documents\DinosaurBattle Prototype\LaunchGame.bat`.
Playable package: `Dist/Windows`; send friends that entire Windows folder.
Independent recovery: `Dist/Checkpoints/2026-09-22-eos-multiplayer/Windows`.
Previous playable: `Dist/Checkpoints/2026-09-22-before-eos-promotion/Windows`.
The pre-multiplayer baseline and earlier checkpoints remain intact.
All 54 non-Saved files match SHA-256 across candidate, recovery and promoted
copies. Dist is ignored by Git: the independent package is the playable backup.
Source tag: `eos-multiplayer-local-tested-20260922` on `codex/internet-multiplayer`.

The normal LaunchGame.bat opened the promoted package at 1600x900 without a
development bridge. Native 1 selected Rex, F4 opened Multiplayer and F10 quit;
both game processes exited. A firewall prompt covered part of the view, so
mouse-click verification remains limited until the user handles that prompt.

EOS lobbies, Epic Account Portal sign-in and EOS P2P force-relay transport are
implemented. **Live EOS and different-network internet play are NOT VERIFIED.**
No EOS product is configured; the developer portal was signed out. Follow
EOS_SETUP.md and place the five-value OnlineServices.ini in
`Dist/Windows/DinosaurBattle`. The example file is included there. Use two Epic
accounts with access to the product, then perform the user's second-PC test.
This goal is not fully complete until live EOS hosting/discovery/join and
different-network gameplay pass. No port forwarding or IP entry is part of
the intended player flow. A Windows Firewall prompt was observed and was not
accepted; no OS security settings were changed.

F4 Multiplayer opens Host Game / Join Game. Host chooses FFA or Team Battle,
2–10 main slots, bots and visibility. Guests select species/team and Ready;
host starts. FFA ends at five kills; teams at ten; respawn is ten seconds.
Bots yield to human joins, teams cap at five, and each raptor leader gets two
private followers without consuming slots. ESC does not pause online.
Host exit ends the match; guests return to Multiplayer. No host migration.
FRIEND_QUICKSTART.md contains the shareable instructions.

Verified: ten real packaged processes over local development sockets, two
120-second headless FFA/5v5 runs with twenty followers; up to eight main bots
with two humans; real client combat, damage/scoring/respawn, pack cleanup,
visibility/food/water and lobby/host-loss behavior. Headless simulation was
about 60 FPS, with minimum 13.64 GiB free RAM; this is not rendered or WAN
performance. Three profiles injected 15/38/75 ms outgoing delay per peer;
the last used 2% packet loss. Total sampled pings were 86.9/145.3/243.6 ms.
Each passed 24 checks; the worst-profile pack suite passed 21 more.

Packaged offline regression: **183/183** (core 90, integration/navigation
41, settings/water/blood 28, match rules 24). The subsequent rebuild only changes
the local-settings caption and hides offline shortcuts while online; it receives
a separate final UI/launch check. Final network/visual verification
is recorded in TEST_LOG.md and Tests/Results/multiplayer. Tests are automated
game processes, not ten human playtesters. Failed trials remain in evidence;
fixture cleanup, GameState publication timing and old test exit codes were fixed.

Preserved user input: Config/DefaultInput.ini remains byte-identical to
Tests/Results/multiplayer-baseline/DefaultInput.before.ini and excluded from
agent commits. Final account checkpoint: 47% remaining; no Astra-specific
allowance is exposed. One agent; no purchases or deployment. See KNOWN_ISSUES.md for
latency/host advantage, art/audio limitations, bounded soak coverage and Steam
work remaining. EOS_SETUP.md describes the provider boundary for future Steam.

---

# Sound and terrain checkpoint â€” September 20, 2026

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

Warmed RTX 3060 packaged samples at 1600x900 averaged 59.49â€“60.00 FPS. One 169ms
pond hitch was sampled; this is not a full trace or a hitch-free guarantee.
Nine packaged map views reviewed. Evidence: `Tests/Results/sound-terrain`.

The preexisting staged `Config/DefaultInput.ini` is byte-identical to
`Tests/Results/sound-terrain/DefaultInput.before.ini` and excluded from source
commits. Final package checkpoint allowance: 63% remaining; no separate Astra
allowance exposed. No purchases, delegation or gameplay rebalancing.

Controls: 1/2/3 select Rex/raptor/Triceratops; WASD moves; mouse looks; Shift
sprints; Space jumps; Q/E pivot; Ctrl braces; LMB attacks; hold/release RMB charges; hold F eats.
M opens/closes the map; point and R adds/removes a pin. Escape pauses; F10 in
the menu quits. See README for match/settings details.

## Previous visual modernization checkpoint â€” September 20, 2026

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

RTX 3060 at 1600x900: warmed packaged scenes averaged 58.81â€“60.00 FPS at the 60 FPS
cap. One 194.8ms pond hitch was sampled; a 45-second follow-up averaged 59.94 FPS
without sampled frames above 50ms. These are bounded samples, not a full trace.
Anatomy, fern/rock silhouettes, clip transitions/sliding and distant repetition
remain procedural. Menu portraits retain the prior artwork. Water reflections
have screen-edge limitations. This is an improvement, not reference-level realism.

Controls: 1/2/3 select Rex/raptor/Triceratops; WASD moves; mouse looks; Shift
sprints; Space jumps; Q/E pivot; Ctrl braces; LMB attacks; hold/release RMB charges; hold F eats.
M toggles the map; point and R adds/removes a pin. Escape pauses/releases mouse;
F10 in the menu quits. See README for match and settings details.

Final reported account allowance: 70% remaining; no separate Astra allowance
exposed. No purchases, delegation or gameplay rebalancing in this sprint.

## Previous map and results update â€” September 20, 2026

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
Shift sprints; Space jumps; Q/E pivot; Ctrl braces; LMB attacks; hold/release RMB charges;
hold F eats. Escape pauses and releases the mouse; F10 in the menu quits.

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
