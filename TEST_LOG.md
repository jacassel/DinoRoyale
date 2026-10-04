# Test log

## Dino Royale 0.3 Alpha Test - in progress, October 4, 2026

- Preserved the prior QA2 standalone release and independent recovery; matching runtime SHA256 starts `76D74F5A0B8C3077`. Baseline source checkpoint: `alpha03-baseline-tested` (`05053ca`).
- Baseline rendered package: core 90/90; ecology 42/42. Uncapped 1600x900, live AI, isolated GPU: plains 65.584 FPS, forest 105.435 FPS, pond 80.170 FPS. The concurrent-editor measurement is retained and excluded from comparison.
- Grounding diagnostics found remote mesh smoothing cached zero offsets instead of species capsule offsets; remote carcasses inherited the smoothed zero. Corrected caching and authoritative carcass transforms. Controls/grounding checkpoint: `b45177d`.
- Pivot/grounding network checks: 64/64 after correcting the fixture to pause newly created raptor followers. Six new left/right clips preserve the existing rigs and actions in separate Blender copies.
- Pond eight-direction crossings for all three species: 96/96 before movement changes, 96/96 with 75ms per-peer delay after fixing a lobby-readiness race in the test.
- Forced movement-mode correction reproduced a stale swimming flag: Rex walking on pond bottom at Z=-266.6 while swimming=true. Movement-step synchronization recovers all three species (3/3) to surface swimming. This is a reproduced state inconsistency, not proof of the exact network event in the owner's earlier playtest.
- Custom team/map network checks: 13/13 on the current build. Two humans versus five bots, empty slots, host authority, replicated map variant, identical 36 edible plants, scores, follower exclusions, return to Standard, normal 5v5 and replacement bots on departure passed.
- Foliage audit: existing HISM components already avoided per-tree actors. Added real mesh LODs (canopy 97,200 to 1,267 vertices), tighter culling, reduced grass density and distant shadows. Performance measurements pending; no improvement percentage claimed yet.
- Post-fix delayed pond crossings: 96/96. Hunger/prey/threat/chase tests: 28/28 across all four personalities. Broad source regression is running; no package or performance claim yet.
- Current editor build passes. Broader regression, visual playtesting, packaged release, final performance comparison and GitHub push remain unfinished.

## EOS resolved URL repair / QA2 - September 26, 2026

- QA1 evidence: physical screenshots and local logs show successful authentication,
  hosting, discovery, compatibility and JoinSession/address resolution. The game
  rejects the resolved `[EOS:PUID]` URL before ClientTravel because it checks a raw
  `EOS:` prefix. Installed UE 5.8.2 source confirms the bracketed representation.
- Fixed the guard using FURL and FInternetAddrEOS, retaining the original resolved
  URL, named GameSession/GamePort source and compatibility option. Added redacted
  lifecycle/session/driver/PostLogin diagnostics; no portal or gameplay changes.
- Editor build and full Windows BuildCookRun succeeded. Final diagnostic-only
  rebuild/package also succeeded. Runtime EXE and cooked files match staging;
  preserved QA1 and the preexisting user input file match their baseline hashes.
- Editor and initial package automation: two passing suites (17 compatibility +
  15 EOS URL assertions). Final package: `automation-final-pass/index.json`, two
  passing suites, zero failures. Negative fixtures retain meaningful version and
  EOS transport checks.
- Packaged loopback: lobby 14/14, fundamentals 24/24, bots 17/17, matches 48/48,
  ecology 25/25: **128/128**. Rendered offline core: **90/90**. These are separate
  from the real EOS account test and do not establish WAN gameplay.
- Live EOS with two owner-authenticated accounts on one PC: bracketed resolution
  accepted, NetDriverEOS passthrough=0, guest ClientTravel, host accepted PreLogin
  2026092201==2026092201, remote PostLogin slot=1 players=2, guest NetMode=3.
  Both telemetry snapshots and game windows showed the same two-player match.
- The attached EOS combat/rehost test began after the instances left. Its failed
  initial precondition is preserved in live-eos-results.json; see
  live-eos-summary.json for the earlier successful connection evidence. Live EOS
  combat, invitation joining, repeated sessions, physical two-PC and different
  networks remain unverified. Owner requested stop; live pursuit ended.
- The first live bound-address diagnostic used FSocketEOS::GetAddress through a
  base reference and printed an invalid derived EOS ID despite working travel.
  Final code reads the driver's LocalAddr. Only logging changed after the broad
  regressions; the full gameplay suites were not repeated for this adjustment.
- Retained unsuccessful runs: initial parser assertions assumed SDK FromString
  validated arbitrary string identity; SDK documentation corrected that test
  assumption. First fundamentals score assertion sampled stale host telemetry;
  waiting for published host score fixed the fixture. Final automation invoked
  with -NoEOS crashed because the address fixture needs the EOS SDK DLL loaded;
  rerunning the established command without -NoEOS passed. Initial compile/API
  corrections remain in ignored build logs.
- Final normal packaged launch without configured EOS or development bridge:
  engine initialized and LostValley entered according to log. Computer Use was
  stopped with Escape; no new visual/input smoke pass is claimed. Only tracked
  smoke-launch processes were stopped. See final-smoke.json.
- Fresh distribution is DinoRoyale-20260926-QA2. OnlineServices.ini is excluded;
  copy the existing per-install file as described in FRIEND_QUICKSTART.md. Final
  manifest, ZIP/recovery verification and checksum are in checkpoint.json and
  release-manifest.json under Tests/Results/eos-qa-20260926.
- Detailed results and Steam/Epic remaining work: MULTIPLAYER_QA_REPORT.md and
  RELEASE_READINESS.md. No purchase, account/security change or service rename.

## EOS timeout repair — September 22, 2026

- Editor build and Win64 packaged build succeeded.
- `packaged-failure-edges-02`: **13/13**. A guest's silent timeout after
  20.110 seconds preserves the listen server and a healthy, controllable guest;
  bot/pack replacement has no duplicate actors or kill credit. A replacement
  guest joins. Silent host loss returns both guests after 20.578 seconds.
  Ending-match and unreachable-host rejection show the expected messages.
- `packaged-survival-edges-04`: **16/16**. Server starvation damage/hunger,
  suppressed stamina regeneration, starvation death and respawn, finite carcass
  consumption, real heavy interruption and knockback in both directions, and
  unmodified plant regrowth after 120.203 seconds agree across two processes.
- Both suites use the rebuilt packaged executable and explicit local development
  sockets. They do not validate EOS or WAN.
- All 54 non-Saved package files matched SHA-256 across candidate, recovery and
  promoted copies. Evidence: `timeout-fix-package-{manifest,checkpoint}.json`.
- Normal `LaunchGame.bat` reached the 1600x900 menu without the development
  bridge. Screenshot: `timeout-fix-launch/menu.png`. The firewall prompt remained
  unchanged; no native inputs were attempted behind it on this run. The game was
  closed by process cleanup; mapped gameplay controls passed in the suites above.
- Earlier failure evidence remains. The first timeout test found the repaired
  game bug; survival trials exposed a missing telemetry-field assertion, a
  telemetry timeout during concurrent packaging, and a transient Windows file
  sharing conflict. The final serialized run passed after harness corrections.
- User input baseline hash still matches. Account allowance: 44% remaining;
  no Astra-specific allowance exposed. Live EOS/internet play remains unverified.

The broader results below were obtained before this narrow handler repair and
were not all repeated; the new packaged suites and normal launch cover the change.

## EOS multiplayer — September 22, 2026

Final promoted LaunchGame.bat check: 1600x900 normal launch, native Rex selection
with 1, F4 Multiplayer and F10 quit passed; no development bridge was enabled.
Both launch processes exited. A Windows Firewall dialog limited native mouse
verification; no OS permissions were changed. Checkpoint account allowance:
47% remaining, no Astra-specific window exposed.

Evidence is under `Tests/Results/multiplayer`. EOS authentication/discovery/
invites/relay and different-network connectivity remain NOT VERIFIED.

| Suite | Passed | Scope |
|---|---:|---|
| stage-b-04 | 23/23 | Two-process authoritative movement/combat/death/respawn |
| stage-c-02 | 20/20 | Lobby permissions, ready gating, settings, return |
| stage-de-01 | 48/48 | FFA/team goals, assists, rematches, 2v1 |
| stage-f-02 | 17/17 | Human/bot replacement, up to eight main bots |
| stage-g-01 | 21/21 | Private raptor packs, scoring and cleanup |
| stage-h-02 | 13/13 | Four players, 2v2, full/version rejection, host exit |
| ecology-01 | 25/25 | Visibility, feeding, plants and all-species water movement |
| combat-01 | 39/39 | All-species combos, charge, injury, exhaustion, cooldowns |
| scale-01 | 18/18 | Ten packaged processes, 20 followers, FFA/5v5, bounded soak |
| latency-30 / latency-76 / latency-150-loss2 | 72/72 | Three packaged delay profiles; last with 2% loss |
| latency-packs-loss2-02 | 21/21 | Pack replication/cleanup at highest delay and loss |
| packaged-cosmetics | 3/3 | Different local blood preferences with replicated hits |
| packaged-four | 13/13 | Rebuilt package, four players and failure paths |
| packaged-rendered-final | 6/6 | Two rendered peers, movement/combat and frozen-lobby prey |
| packaged-ui-final | 6/6 | Final UI-only rebuild: repeated rendered smoke and native settings check |
| packaged-offline | 183/183 | Serialized package regression, including 42 navigation routes |

Native screenshots checked host and guest; ESC opens multiplayer menu. The
final build corrects scoreboard/lobby column alignment, 0.2 version/help text,
and own-pack counts. The last rebuild changes only the local-settings caption
and hides offline shortcuts during online settings; final UI/launch is checked
separately. Windows Firewall permission was not granted. Source and
package compile results, hash manifests and exact recovery paths are recorded
in HANDOFF.md. The existing staged input file remains unchanged.

Latency settings add nominal 30/76/150 ms round trip. Sampled total pings were
86.9/145.3/243.6 ms; exact total 30/75/150 ms was not established. The ten-process
run was headless, approximately 60 FPS, with minimum 13.64 GiB free RAM and
about 241–244 kB/s host outgoing traffic. It does not establish WAN, ten-window
rendering or long-session stability.

Failed iterations and fixes are documented in Docs/MULTIPLAYER.md. In particular,
offline trial 01 had a retained dummy, trial 02 overlapped native multi-window
inspection, and immediate score assertions raced GameState publication. Older
settings/match test exit codes now propagate assertion failures. Only the
serialized final all-pass results above establish the offline checkpoint.


## Sound and terrain â€” September 20, 2026

Evidence: `Tests/Results/sound-terrain`; scope/provenance: `SOUND_TERRAIN.md` and
`Assets/Audio/CREDITS.md`. Source checkpoint `c2ba3a1`; previous source tagged
`sound-terrain-before-20260920`. Original playable remains intact during validation.

- 81 designed sound files: finite 44.1kHz PCM, clean boundaries, peak <=0.72.
- 54/54 source audio checks cover all three playable species: loaded assets,
  footsteps/cadence, quick misses versus impacts, charge-up/heavy, hurt/injured
  breathing, healing, one-shot death, respawn, pause and rendered output levels.
- Recorded output peaks: Rex 0.219, raptor 0.313, Triceratops 0.255. Two initial
  pack-fight mixes were unclipped, at peaks 0.327 / 0.243. Direct listening is not
  available in this agent session; these are measured playback checks.
- 502 trees; 2,401 terrain samples show interior maximum 25.64m versus the prior
  9.61m, with maximum added relief 16.80m. Pond basin preserved to <0.001cm
  numerical tolerance at sampled points. Initial navigation audit: 42/42 routes.
- Nine landscape views inspected. Near-camera leaf obstruction was discovered
  and repaired with a material cutout; nine final views were captured, including
  a clear ridge-overlook comparison. Existing close camera collision with rocks
  and animals remains visible in some fixed test positions.
- Full source regression: **462/462, fifteen suites**. All eighteen continuous
  all-species regional traversals passed, plus all three AI pond crossings.
  Existing combat, food, visibility, resource, map and match checks also passed.
- Initial warmed 1600x900 source performance: 60.00 / 60.00 / 59.95 FPS across
  plains/forest/pond, max sampled frame 31.62ms. Packaged measurements follow.
- Standalone candidate BuildCookRun succeeded in 141.84 seconds. Packaged audio:
  **54/54**; recorded Rex/raptor/Triceratops peaks 0.226 / 0.281 / 0.230.
  Both group-fight mix checks passed (peaks 0.383 / 0.312; observed voice maxima
  10 / 9). Credits included beside the packaged launcher.
- Initial packaged core attempt: 84/90; six respawn checks failed because the
  preceding `duelSetup` audio fixture intentionally disabled respawning. The
  runner now starts a fresh candidate process after that fixture. Original
  evidence retained in `package-regression-first-attempt`; gameplay and test
  assertions were unchanged. Fresh-process rerun follows.
- Fresh-process package regression: **220/220, six suites**. All three natural
  respawns and subsequent movement passed. Packaged navigation audit: **42/42**.
  Nine packaged landscape views reviewed, plus full-size hill/ridge views.
- Packaged RTX 3060 at 1600x900, warmed 20-second live-AI samples: plains 59.98,
  forest 60.00 and pond 59.49 FPS. Maximum sampled frame times 16.71 / 16.68 /
  169.21ms; the pond hitch is retained as a known limitation.
- Four-minute packaged autonomous run passed: 240.52 seconds, 179 hits, no
  failed paths, no stuck recoveries, no below-terrain live actors and no immediate
  respawn hits. Up to 20 carcasses observed; maximum stationary travel sample
  count was one. Detailed samples are in `world-soak-0-samples.json`.

- Promoted the validated candidate to `Dist/Windows`. All **49 runtime files**
  SHA-256 verified against the independent sound-terrain recovery. Previous
  playable retained in `Dist/Checkpoints/2026-09-20-before-sound-terrain/Windows`.
- Normal `LaunchGame.bat` started the promoted standalone at 1600x900 without
  development/test arguments and reached the selection menu. Windows Firewall
  then displayed a permission prompt. Native input/quit check is pending user
  dismissal; the agent did not act on the prompt or alter security settings.
- Final package checkpoint allowance: 63% remaining; no Astra-specific allowance
  exposed. Preexisting staged input configuration retains its original SHA-256.

## Map and results update â€” 2026-09-20

- Standalone BuildCookRun succeeded. Packaging recovered automatically from a transient local Zen connection failure. An early launch attempt before archive completion found no executable; launch after successful archive passed.
- New packaged tests: allied visibility **17/17**, including all four teammates, followers, occlusion, distance, noise expiry, current coordinates, respawn, and enemy/solo concealment. Pins and results **21/21**, including actual M/R input, cursor-to-world placement, removal, outside-map rejection, eight-pin limit, restored movement, respawn persistence, round reset, human/AI five-kill wins, all ten KDA rows, paused results and hidden-card click protection.
- Existing packaged regressions: match rules **24/24**, settings/water/blood **28/28**. Total **90/90**. Reviewed four rendered screenshots: expanded map, minimap, human results and AI results.
- Evidence is in `Tests/Results/map-results-update`. Tested package promoted to `Dist/Windows`, with recovery at `Dist/Checkpoints/2026-09-20-map-results/Windows` and previous playable retained at `Dist/Checkpoints/2026-09-20-before-map-results/Windows`. SHA-256 compared all 48 runtime files across tested, recovery and promotion staging copies; Saved/test state excluded.
- No multiplayer added: this remains a local human-plus-AI prototype. Pins are personal round markers, not AI orders. Existing longer quality-sprint verification below was not rerun for this small update.

## Environment â€” 2026-09-18

- Confirmed Unreal Engine 5.8.2 at `C:/Unreal Engine/UE_5.8`.
- Confirmed Blender 5.2.2 LTS at `C:/Program Files/Blender Foundation/Blender 5.2`.
- Hardware: RTX 3060, 32 GiB RAM, 16 logical CPU threads.
- Initialized local Git in the previously empty project directory.
- C++ compiler and Windows SDK not present in conventional install locations; investigating toolchain setup.

No gameplay claims have been validated yet.

## First playable systems pass â€” 00:19â€“00:35

- Built and launched the real Unreal game with temporary character meshes.
- First live input suite: 84/90 assertions passed across T-Rex, Raptor and Triceratops.
- Found player respawning in place could leave movement blocked by the adjacent target. Fixed respawn relocation to a clear start.
- Camera pitch responded by 0.778 degrees to the injected test motion; the initial >1-degree assertion was too strict. Corrected the assertion to check actual nonzero camera response.
- Retest: **90/90 assertions passed**, including WASD in four directions, camera yaw/pitch, jump, Q immobilization and release, blocking attacks while braced, one quick hit per attack, charge feedback and stronger charged hits, damage, both injury movement/attack thresholds, critical charge rejection, delayed regeneration, recovered abilities, death, respawn and post-respawn movement.
- Measured first-pass quick / charged damage: Rex 180 / 468, Raptor 100 / 280, Triceratops 145 / 398.75.
- Tests ran in a rendered Unreal game using the real PlayerController input event pipeline and CharacterMovement physics; this is not a unit-test substitute implementation. Setup commands placed a target and applied injury.
- Inspected the visible game window with desktop computer-use tools. Temporary scene and HUD rendered.
- Results: Tests/Results/core-run01.json (initial), core-live.json (retest). Original Blender source and FBX assets generated for all species and prey. Art integration is a separate, not-yet-validated milestone.

## World/art integration â€” 00:35â€“00:57

- Original Blender source files, FBX skeletons and ten clips imported for T-Rex, raptor, triceratops and small prey.
- Built the seeded world, clearance-aware navigation, nine major AI, eighteen prey and regrowing food plants.
- First full-world launch exposed reversed procedural terrain triangles: ground was invisible from above and characters remained falling/sliding. Reversed terrain and creek winding, rebuilt and relaunched.
- Corrected scene screenshot shows terrain; runtime reports grounded stationary player. FPS cap explicitly applied on game start (initial console-config-only cap did not take effect in the editor game).
- Full-world control retest is running. AI is paused for isolated control assertions; separate autonomous AI observation is still required.
- Computer-use tool reported a physical Escape stop at 00:52; desktop automation was stopped. Development resumed under the active continuation using local build/runtime interfaces and game-generated screenshots.

## Final systems retest and visual fixes â€” 01:00â€“07:23

- Respawn clearance search now avoids nearby living capsules and world obstacles. Retest after this change: **90/90 core assertions passed** in the full world (Tests/Results/core-world02.json).
- Integration checks: **41/41 passed** (Tests/Results/integration01.json): stationary grounding, front shield reduction and rear bypass, brace/eat/charge/heavy/death animation state transitions, feeding heal and release, respawn animation recovery, all 30 directed region paths and exactly ten major combatants.
- Imported material-slot assignments were missing because the editor Python array yielded copied structs. Converted to a Python list before updating and saving; verified all slots have materials.
- Final visual inspection still showed unacceptably dark skin. Replaced the unreliable imported vertex-color dependency with editable species tint plus original surface textures; added a daylight ambient cubemap for readable shadows.
- Final build succeeded after these visual changes. Final world/AI runtime verification is ongoing; results will be recorded separately.
- The running test process remained open during the overnight interruption. Elapsed runtime alone is not evidence of an AI soak test because AI had been paused for controls; no such claim is made.

## AI and biome checks â€” 07:23â€“07:28

- Local physical traversal: **18/18 passed**, covering all six major regions with each human-playable species. Tests teleported only for setup, then drove the real character with W through each local area. This is not a complete route traversal.
- Initial autonomous observation found predators over-prioritizing faster prey, resulting in prolonged pursuit without hits, and raptors leaving the human leader too far behind.
- Fixed prey speed (950 cm/s), preference for major combatants, pursuit distance limits, pack regrouping beyond 50 m, and closer pursuit waypoint completion.
- Rebuilt and launched. **90.8-second AI retest**: nine major AI + one human, all AI moved >10 m, 22 T-Rex / 23 raptor / 18 triceratops hits, zero failed paths, zero stuck-recovery events, no major dinosaur below terrain, prey fleeing observed. Raptor leader IDs were 0 (human); all three were in With pack state at the end.
- Mean reported FPS **59.59** at 1280x720 with 10 major combatants, 18 prey and the generated environment. This is not a measured 1080p benchmark.
- Evidence: Tests/Results/final-world-checks.json, ai-observation-before-fix.json, ai-final-retest.json and ai-final-summary.json.
- User removed the 07:30 deadline at approximately 07:26 and authorized continued work. Full route traversal and further polish remain in progress.

## Settings, optional blood and water â€” 07:34â€“07:45

- Opening species-selection and pause/settings UI implemented; Escape releases the mouse. Number keys select species and resume; F2 opens settings; B toggles optional blood; +/- adjusts saved mouse sensitivity. M opens the derived region map.
- Fixed pause-state telemetry by enabling full PlayerController ticks when paused. World simulation remains paused while the menu and UI input work.
- An editor-only import commandlet hit a rooted material-expression assertion while replacing an already referenced graph. Recovered from the saved assets and authored a separate M_WorldSurface material; import retry succeeded. No operating-system crash occurred.
- Added bounded instanced blood particles with no collision or gameplay authority; disabled by default and saved as a user setting. Turning the option off clears existing particles.
- Added shallow creek wading: walking speed factor 0.65, composed with health and charge modifiers.
- **28/28 live checks passed** (Tests/Results/settings-water-blood.json): menu/pause input, species selection, sensitivity, blood off/on/clear, water slowing each species, 50% and 25% injury composition, charge rejection at critical health, recovery, leaving water, and map toggling.
- Visual inspections of the rendered opening and settings screens saved under Saved/Screenshots/Windows. Text sizing remains a polish task.

## Match scoring and extended traversal â€” 07:45â€“08:15

- **24/24 live match checks passed** (`Tests/Results/match-rules.json`): K/D/A attribution, assists, five-kill solo victory, ten-kill team victory, exactly five major combatants per team, friendly melee rejection, follower/prey exclusions, stable leader scoring, shared pack credit and ten-second respawn. Scoring setup called real ReceiveHit; these were controlled rule tests, not complete human-played matches.
- Raptor and Triceratops each completed continuous six-region circuits of about 1.75 km using real W movement and camera-relative steering, without inter-region teleports. T-Rex traversal was interrupted twice. The diagnostic retry recorded W becoming released and simulation time pausing, rather than a collision while input remained held. Automated testing is being moved to a rendered offscreen instance to isolate it from desktop focus/input changes. No T-Rex circuit pass is claimed yet.
- User reported the T-Rex lower jaw entering its head during charged attack. Reversed jaw opening rotation in quick, charge, heavy, eating and death clips. Regenerated all clips from native Blender sources, with new surface-paddling clips. **15/15 Blender jaw-direction checks passed**; these measure jaw-tip opening direction and do not replace visual collision inspection.
- Added Mirror Pond, depth-based swimming and surface-surge controls. C++ build and animation import succeeded. Live swimming regression is in progress.

## Pond and isolated traversal results â€” 08:15â€“08:27

- **40/40 swimming checks passed** (`swimming.json`): all three species float, paddle, slow down, stop under Q, apply 50%/25% injury modifiers, reject critical charge, recover from heavy attacks/surface surges, swim to shore and resume walking. All 42 directed paths among seven landmarks are reachable. Test setup placed the pawn in deep water; shore entry and AI swimming receive separate coverage.
- **18/18 continuous traversal legs passed** (`full-traversal.json` and `traversal-samples.json`), covering the six-region circuit for each species. T-Rex's previous interrupted-input stalls did not recur in the rendered offscreen instance. No per-region teleports were used. A background compilation overlapped part of this test, so elapsed traversal time is not a clean performance benchmark.
- Standalone Windows game target compiled successfully. Cooking, packaging and launching that output are still required.
- Refined asset import initially encountered Windows file locks while the old game was running. Stopped only that importer and deferred the retry until the test game closed; existing recoverable assets and source files were retained.

## Refined art and tactical AI â€” 08:28â€“08:41

- Imported four refined, continuous Blender skins with original baked UV textures. Each mesh now has three runtime LODs; successful reduction/import counts are recorded in `refined-import.json`. Native refined sources remain alongside original sources.
- Authored selection portraits from those meshes and replaced the raster-scaled HUD font with a distance-field font. Found the selector overlapped at a viewport clamped to 888x500; adjusted minimum UI scale and added ForceRes to offscreen benchmark launches. Visual retest pending.
- Visually inspected rendered T-Rex charging and heavy attack after the jaw fix: lower jaw opens below the skull. Inspected visible swimming posture and translucent pond water; screenshots 81â€“83 under Saved/Screenshots/Windows.
- Tactical AI estimates fight/escape confidence from current health, attack rate, movement, and nearby support. Guards have a bounded hold time and cooldown; retreat has hysteresis; pack raptors can flank frontal guards.
- Initial tactical checks were 9/11 during first-load shader warmup; isolated pursuit diagnostics showed the raptor properly pursued and killed its weakened target. Warm retest **11/11 passed** (`ai-tactics.json`), including retaliation damage, pursuit hits, increased retreat separation, actual frontal guard damage reduction, counterattack, desperate defense and supported flanking. Timing assertions now use simulation time to avoid shader-load false failures.

Checkpoint before combat polish: all four final regression scripts passed. Solo round ended at 221.53s; team round at 251.89s (2:10). Nine major AI remained active, no failed paths or below-terrain falls; team maximum stuck recoveries 3. Standalone BuildCookRun completed successfully; packaged launch still pending.


## Combat polish pass 1 â€” September 18

- Protected the starting build with commit 1715ab6 and tag pre-combat-polish-20260918. Prior standalone archive and original Blender assets retained.
- Implemented the shared stamina component, species sprint tuning, three-hit quick sequences, heavy drive/knockback/miss recovery, safe feeding delay, and four AI personalities.
- First live resource/combo run: 66/69. Three assertions used fixed timing inappropriate to fast raptor recovery or allowed regeneration before starting the low-stamina brace. Adjusted assertions to game time and actual remaining recovery, and set low stamina after bracing. Retest: 69/69.
- Reviewed exhaustion under repeated weak attacks and removed additional stamina spending while exhausted so attacks cannot continually postpone regeneration.
- Twelve live player-input encounters covered all three player species against all four AI profiles. All resource bounds held; no path failures or stuck recoveries. Profiles used guards, sprinting, charging, retreats and repositioning. Isolated raptors sometimes sensibly disengaged/hunted prey instead of accepting a much larger opponent; these cases do not prove a direct duel.
- Regression after the initial gameplay change: 90/90 core, 41/41 integration, 28/28 settings/water/blood, 24/24 match rules. Eating regression now waits the intentional 2.5-second safe-feeding window.
- Raptor frontal survivability was too unforgiving: raised health from 520 to 600, preserving vulnerability but allowing survival of one 561-damage full Rex bite. Follow-up tests running.
- Found persistent material overrides during species changes. Unreal SetSkinnedAssetAndUpdate preserves overrides; clear them before loading the new species to prevent old skin/slot assignments and nested dynamic instances. Original assets untouched.
- Read-only Blender jaw audit sampled all 86 frames of the retained Rex Charge/Heavy/Quick clips. Every frame keeps jaw opening downward relative to the skull; maximum openings at the measured tip were 22.50/61.49/43.70 cm. The earlier reversed-axis repair remains present. Multi-angle rendered verification follows.

Combat follow-up passed 17/17: correct species materials, held-sprint exhaustion/recovery, repeated exhausted attacks recovering, blood preserving materials, and raptor surviving full Rex heavy with 39 HP. Removed inherited F2 unlit/F3 lit and Shift debug bindings after screenshot exposed lighting switching during settings. Repeated 24 rendered capture sequence after fix; fixed test screenshot-copy retry for asynchronous file writes. Pack fights used shared abilities and no failed paths/stuck recoveries. Standalone combat package completed successfully; final packaged launch remains to be verified.


## Hunger, finite food and map visibility

- Reduced raptor quick damage from 82 to 74 (full pounce 214.6) at the user's request.
- Added species-configurable hunger, gentle depletion, the requested 70/40/20/10 percent regeneration/starvation thresholds, and rapid direct restoration through food.
- Separated edible carcasses from respawning characters. Food budgets: prey 25, raptor 120, Rex 360, Triceratops 480. Corpses collapse, freeze, use reduced LOD/distance culling, and remain until consumed. Depleted plants disappear and regrow after 120 seconds.
- Implemented sight/noise map visibility and line-of-sight health labels. Noise reveals last-known positions for six seconds; sprinting and charging keep the reveal current.
- Live ecology/visibility test passed 42/42: measured actual health/stamina regeneration and starvation rates for all three species; verified direct food recovery, exact finite consumption, correct corpse sizes, frozen collapse, persistence beyond respawn, plant hiding, camera-view visibility, physical occlusion, AI/human noise and expiration, and hungry AI actually feeding.
- Five additional edge checks passed: hidden depleted remote plant, starvation death/carcass, full-resource ten-second respawn, and an offscreen held charge staying revealed beyond six seconds.
- Full previous-system regression and final packaged launch checks are in progress; only completed results are claimed above.


## Compact world / balance / audio sprint ï¿½ in progress

- Protected inherited runtime evidence in e71899b (`quality-sprint-input-20260919`); existing `Dist/Windows` and `Dist/Checkpoints/2026-09-19-stable/Windows` remain untouched.
- Halved horizontal geography, landmarks, spawns, navigation bounds and map projection. Kept corridor/animal sizes and swimming depth. Regenerated and imported the overview texture.
- Current calibration: Rex 1500 HP, 187 quick, 673.2 full heavy, 0.85s charge, 240cm sweep width; raptor 520 HP, 66.6 quick, 193.14 full pounce, 0.1125 hunger/sec (1.5x base), unchanged mobility/stamina economy. Normal packs have one leader and two followers in solo and each team.
- Fixed AI target thrashing under pack hits, AI abandoning its own charge to guard, and small pounces repeatedly interrupting larger animals. Nearby raptor support now contributes appropriately to tactical confidence.
- Calibration initially used complete pack elimination; that overstates what a Rex must accomplish under the actual leader-only scoring rules. Current trials record first scoring death AND a 20-second survivor follow-through. Six calibration trials: Rex scored first 4, pack 2; 2 Rex first-score wins were counterkilled. This is preliminary, not final balance validation.
- Added original synthesized finite positional footsteps, quick/heavy motion, successful-hit and species death feedback, with per-animal and shared overlap caps.
- Live requested-rules suite: **36/36**. Live audio suite: **36/36**, including actual Unreal master-output WAV captures. Species peaks 0.095/0.111/0.104; nonzero, unclipped output. Movement/sprint cadence, no airborne/swim steps, miss-vs-impact, death-once, voice termination, pause and respawn checks passed.
- Full regression, continuous traversal, held-out final balance, longer world observations and replacement packaging are still pending. Do not treat the current source checkpoint as the final playable package.


### Smaller-map regression repairs and checkpoint

- Preserved initial failures. Swimming initially passed 36/40: Rex/Triceratops grounded their buoyant capsules before the old exit threshold. Added radius-aware shore clearance and retained entry/exit hysteresis. Retest **40/40**.
- AI tactics initially passed 10/11: guard duration omitted the released heavy's windup, producing phase-dependent early guard expiration. Included injury-adjusted heavy windup in the guard timer; retest **11/11**, with frontal Rex heavy reduced from 673.2 to 67.32 damage.
- Continuous traversal **18/18**: all six-region circuits for all species via actual W movement, no inter-region teleports; approximately half the previous route length.
- AI pond crossing initially **2/3**. Triceratops exited water but a recovery nudge discarded its requested destination when earlier test animals blocked the bank. Preserve the requested travel destination independently of recovery waypoints. Unchanged crowded test retest **3/3**: Triceratops reached shore after one successful recovery, zero failed paths.
- All fourteen current regression suites pass, **460/460 checks**. Original failed results and guard diagnostic retained in `Tests/Results/quality-sprint`.
- Final held-out balance, extended packaged world observations, native launch and package promotion remain pending.

### Standalone observations and plant cue

- Added a visible-silhouette green-gold outline to edible plants for the human Triceratops only. Custom-depth stencil plus an occlusion-aware postprocess keeps ordinary decorative ferns unmarked. Actual packaged screenshot reviewed; all four species-switch checks passed. The 120-second depletion/regrowth test passed at 119.78 simulated seconds, including outline removal/restoration.
- The outlined standalone candidate passed 193 targeted checks: core 90, settings 28, audio 36, requested rules 36, AI swimming 3. This candidate used the earlier 1500/520 health tuning; final tuning receives separate checks below.
- Three four-minute autonomous world scenarios (solo, teams, low hunger) passed. Landed hits: 259/175/251; failed paths 0/0/0; terrain falls 0; no stuck recoveries. Longest stationary travel samples 3/1/1 seconds. Temporary pack separation resolved within 25/17/12 samples. FPS approximately 59.7-59.8. Low-hunger animals actively fed and ended above 75 hunger. Persistent corpses accumulated to 67 across these consecutive rounds without a performance collapse; indefinite-session accumulation remains a limitation.
- Two of 35 observed AI respawns took damage within two seconds (1.97s and 1.02s). Added a nearest-safe-position preference with 3000cm clearance from living major enemies; when surrounded, choose the clearest valid candidate instead of an invalid last candidate. Verification follows separately.
- Two real multi-animal combat recordings passed, peaks 0.0811 and 0.0805, with finite unclipped output. All three isolated species recordings and death/respawn voice tests already passed.
- First 24 varied trials (1500 HP Rex / 520 HP raptor): first scoring death favored Rex 14, pack 3, seven 90-second disengagement timeouts; four Rex first scores were counterkilled within 20 seconds. First score is not complete pack elimination, and counterkills are not erased from the report. Two telemetry-reader interruptions during packaging were preserved; only incomplete trials resumed, completed seeds were retained.
- Six 1400/600 health trials: four Rex first scores, one pack first score, one timeout; two counterkills. Six 1320/600 trials with original 1.05s Rex charge and 200cm sweep: four Rex first scores, one pack first score, one timeout; one counterkill. Restoring the longer windup gives opponents clearer counterplay while preserving 187/673.2 damage. Raptors retain baseline 600 health, unchanged speed/stamina, 66.6/193.14 damage and 1.5x base hunger. Final held-out seed range starts at 800.
- Triceratops vs earlier 1500 HP Rex: five Rex wins, one timeout. Revised 1320 HP Rex with restored windup: two Rex wins, one Triceratops win, one timeout. Triceratops vs earlier 520 HP pack: two Triceratops first scores, two pack first scores, two timeouts; follow-through outcomes retained in JSON. Final 600 HP pack sanity follows.

### Rejecting misleading first-score parity

The 1320/600, narrow-sweep validation was stopped after 16 completed trials, rather than wasting the remaining eight: fourteen ended with Rex dead during the follow-through, none with a surviving Rex scoring victory, two disengaged. First-score parity alone is insufficient. The entire result and explicit REJECTED.json are retained. Continue with the earlier 1500/520 durability and 240cm sweep while preserving the original 1.05s charge windup; validate both score order and follow-through survival. This is not a completed final checkpoint.

### Respawn safeguard verified

- All three species respawned at full health after 10.02-10.04 seconds, 3233cm from an enemy placed at their home point.
- The release candidate passed 36 requested-rule checks, 69 combat checks, 17 combat follow-ups, 90 core checks, and four plant-outline checks after the spawn change (219 total including the three respawn checks). These checks used the intermediate 1320/600 profile; final raw values will be checked again after packaging.
- A fourth four-minute world run passed: 195 landed hits, zero failed paths, terrain falls or stuck recoveries, approximately 59.6 FPS. All eight observed post-respawn first-hit delays exceeded 4.98 seconds; no immediate respawn hits. Temporary pack separation resolved within 20 samples.
- The 1500/520 profile with a longer 1.05s charge still lost all five resolved fights in six calibration trials, including three counterkills. The original shorter charge remains necessary under pack pressure. The earlier 1400/600, 0.85s charge, 240cm sweep profile is undergoing held-out validation because its six calibration fights yielded two surviving Rex victories, three pack kills, one disengagement.
- At 1400/600, four Triceratops/Rex checks yielded three Rex wins and one 90-second disengagement. The defensive Triceratops retained 1463/1650 health in the unresolved fight; one Rex victory ended at 362/1400 health. Rex remains favored in this small AI sample; this is not a claim of equal Triceratops win rates.


### Frozen tuning and interpretation of balance

After comparing the alternatives, retain the original 24-trial Rex/raptor profile: Rex 1500 HP, 187 quick, 673.2 full heavy, 0.85s charge and 240cm sweep; raptor 520 HP, 66.6 quick, 193.14 pounce, 1.5x base hunger and two followers. The lower-durability alternatives were worse under follower counterpressure and were stopped early with their evidence preserved.

Across the retained 24 trials, Rex defeated the leader and survived the bounded follow-through 10 times; the pack killed Rex 7 times, including four counterkills after losing its leader; seven encounters disengaged to the 90-second cap. That is 58.8% / 41.2% of resolved survival outcomes. First-score order alone was 14 Rex / 3 pack; only three encounters eliminated every raptor. These are scoring encounters with up to 20 seconds of follow-through, not a claim that Rex completely wiped the pack ten times. Median first-score time among resolved encounters was 4.58 seconds. All 24 retained trials respected resource/terrain bounds. See `balance-accepted-summary.json` and its referenced raw data.

One modest Triceratops adjustment maintains its offensive relevance: quick damage 135 -> 155, full charge 445.5 -> 511.5, so three unbraced full charges can defeat the tougher Rex before regeneration. Health, movement, brace and stamina values are unchanged. Four fresh direct fights produced three Rex wins and one Triceratops win. Four pack checks produced one pack first score, one Triceratops first score followed by a counterkill, and two disengagements. Defensive holding and flanking remain important; this is a sanity sample, not a 50/50 claim.

The earlier fourth world observation followed control tests that could leave a non-AI test target in the level. Its clean-path and respawn observations are retained, but interaction-density conclusions are superseded by a final clean run. The world runner now explicitly removes test targets and test food and disables invulnerability before observation. The initial three world runs began in a fresh game after plant-only tests and were not affected.

### Final packaged checkpoint

- Final BuildCookRun succeeded. Exact final stat/roster checks 36/36; combat 69/69; combat follow-up 17/17; safe respawn 3/3; edible-outline switching 4/4: **129/129** on the final package.
- Final clean four-minute world run: 197 hits, zero failed paths, zero terrain falls, zero stuck recoveries, longest stationary-travel sample 1 second, no immediate respawn hits, approximately 59.95 FPS. All eight observed respawn-to-first-hit delays exceeded seven seconds. The three earlier clean scenarios plus this run provide sixteen minutes of autonomous world observation.
- Native standalone launched and rendered dinosaur selection. A Windows Security network permission prompt blocked the extra desktop-input check. No security action was automated; the user was asked to cancel the prompt. This limitation is explicit in `stable-release/native-launch.json`; automated real-input controls passed.
- Promoted the tested package to `Dist/Windows`; the old playable moved intact to `Dist/Checkpoints/2026-09-20-before-quality/Windows`. New recovery package is `Dist/Checkpoints/2026-09-20-quality/Windows`. All 49 runtime files matched SHA-256 across the tested, staged/promoted and recovery copies. Test Saved directories were excluded from the clean copies.


## Visual modernization â€” September 20, 2026

Evidence: `Tests/Results/visual-modernization`; iteration narrative and fixture
corrections: `VISUAL_SPRINT.md`. Original working package was SHA-256 verified
before changes; original Blender sources and the staged input configuration were
preserved. No gameplay balance, AI, collision, water physics or animation timing
was changed.

- Reviewed nine final front/side/rear views, 51 action/environment details and 27
  packaged combat captures. Earlier rejected visual passes remain available.
- Blender jaw probe: all 86 frames across Charge/Heavy/Quick passed; rendered
  left/right/front-oblique jaw sequences also reviewed.
- Source regression: **462/462**, 15 suites, including 18 continuous routes and
  all three crowded AI pond crossings. The latter had one successful Triceratops
  recovery and zero failed paths.
- Standalone BuildCookRun passed; no reported compile errors, warnings or failed
  shader compilations. Actual packaged regression: **220/220**, six suites.
- Twelve packaged blood/material capture checks pass: ON has 6â€“20 particles,
  OFF has zero, and every creature slot retains its new species material.
- Corrected stale test material names, finite-guard/desperation setups (verified
  against the untouched old package), physics-settle timing and fixed-resolution
  mouse coordinates. These changes affect tests only. All initial failures and
  the archive-time telemetry interruption are retained with explanations.
- Completed packaged world observation, performance, native controls and promotion
  evidence are recorded below.


### Packaged world and performance

- Four-minute post-regression AI run passed: 187 hits, no failed paths, terrain
  falls, stuck recoveries or immediate respawn hits; maximum stationary travel
  sample one second, maximum 20 persistent corpses. Cumulative session mean
  58.44 FPS (includes preceding tests).
- Warmed 20-second counter-delta samples at 1600x900: plains 60.00, forest 58.81,
  pond 59.41 FPS. Sampled p95 frame time approximately 16.667ms. A single 194.8ms
  pond frame was sampled. Same-scene 45-second follow-up: 59.94 FPS, sampled max
  16.74ms, no sampled frames over 50ms. The original hitch is retained as a limit.
- Runtime log scan found no fatal errors, GPU crashes, failed material/shader
  compilations or texture-pool-over-budget messages. Measurements are capped at
  60 and frame-time sampling is about 22Hz, not an exhaustive profiler trace.

### Promoted package and native launcher

- All 48 runtime files were SHA-256 verified across the candidate, independent
  recovery and promoted `Dist/Windows`. Previous playable retained intact at
  `Dist/Checkpoints/2026-09-20-before-visual-modernization/Windows`; new recovery
  at `Dist/Checkpoints/2026-09-20-visual-modernization/Windows`.
- Normal `LaunchGame.bat` opened the promoted standalone at 1600x900 without a
  development bridge or permission prompt. Native species selection, mouse look,
  M map, R pin and Escape pause responded. Movement/sprint/jump inputs were also
  exercised; the automated core suite provides their quantitative checks.
- F10 quit normally. Runtime log ended `LogExit: Exiting.` at 21:45:08 UTC and
  both game/launcher processes closed. Old menu portraits are an art limitation.
- Source tag: `visual-modernization-stable-20260920`. Preexisting staged input
  configuration remains byte-identical and excluded from the visual commit.
  Final account allowance 70%; no separate Astra allowance exposed.

## Dino Royale EOS defect sprint - September 24, 2026

- Reviewed all six supplied physical-QA photos and the pasted QA brief. Host lobby,
  social presence, invitation delivery and identical BUILD_INFO were observed;
  the exact verbose rejection maps to the game custom pre-join version gate.
- Reproduced UE EOS Int64 metadata read through the former Int32 getter returning
  zero. Shared typed reader repairs browser/invite checks. Meaningful version
  protection remains. Explicit OSS override aligns EOS-created BuildUniqueId.
- Invite Friends now uses implemented ShowFriendsUI. Previous host log explicitly
  reported ShowInviteUI unimplemented. No portal settings or product names changed.
- Editor build and full Windows Development BuildCookRun passed. Unreal engine
  automation passed in editor and actual package, 17 assertions each. Packaged
  loopback lobby/protocol regression passed 14/14, including deliberate mismatches.
- Final rendered packaged offline core regression passed 90/90. First invocation
  omitted standard AI-isolation fixture (84/90); next attempt was interrupted when
  process cleanup mistakenly ended that test instead of the sign-in process. Both
  are preserved. Final correctly isolated run passed with no gameplay code edits.
- Normal LaunchGame.bat: 1600x900, no development bridge; native 1 selection, M map,
  Escape, F4 and F10 passed. Both processes exited. No firewall prompt observed.
- QA1 EOS sign-in opened Epic authentication and timed out awaiting manual input.
  New-package live hosting, discovery, overlay button and invite/connect callbacks
  remain unverified. Two physical PCs and different networks are NOT proven by
  the local tests. Original Refresh timeout needs the guest log; reader fix alone
  cannot explain a missing search callback.
- Fresh release: Dist/Releases/DinoRoyale-20260924-QA1/Windows. Existing EOS config
  copied byte-for-byte. Executable matches built binary; all five pak/iostore files
  match the same completed staging run. Distribution excludes Saved/login caches.
- Previous Dist/Windows untouched; all 71 files independently backed up and hashed
  in Dist/Checkpoints/2026-09-24-before-eos-qa/Windows. New recovery and ZIP are
  validated by Tools/Tests/CheckpointEOSQARelease.ps1; evidence under
  Tests/Results/eos-qa-20260924, including initial failures and native captures.
- User Config/DefaultInput.ini is byte-identical and remains excluded from commits.
  One agent; no purchases. Final checkpoint allowance 38%; no Astra-specific
  allowance exposed. Source tag: eos-qa-ready-for-two-pc-20260924.
