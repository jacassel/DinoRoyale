# Test log

## Environment — 2026-09-18

- Confirmed Unreal Engine 5.8.2 at `C:/Unreal Engine/UE_5.8`.
- Confirmed Blender 5.2.2 LTS at `C:/Program Files/Blender Foundation/Blender 5.2`.
- Hardware: RTX 3060, 32 GiB RAM, 16 logical CPU threads.
- Initialized local Git in the previously empty project directory.
- C++ compiler and Windows SDK not present in conventional install locations; investigating toolchain setup.

No gameplay claims have been validated yet.

## First playable systems pass — 00:19–00:35

- Built and launched the real Unreal game with temporary character meshes.
- First live input suite: 84/90 assertions passed across T-Rex, Raptor and Triceratops.
- Found player respawning in place could leave movement blocked by the adjacent target. Fixed respawn relocation to a clear start.
- Camera pitch responded by 0.778 degrees to the injected test motion; the initial >1-degree assertion was too strict. Corrected the assertion to check actual nonzero camera response.
- Retest: **90/90 assertions passed**, including WASD in four directions, camera yaw/pitch, jump, Q immobilization and release, blocking attacks while braced, one quick hit per attack, charge feedback and stronger charged hits, damage, both injury movement/attack thresholds, critical charge rejection, delayed regeneration, recovered abilities, death, respawn and post-respawn movement.
- Measured first-pass quick / charged damage: Rex 180 / 468, Raptor 100 / 280, Triceratops 145 / 398.75.
- Tests ran in a rendered Unreal game using the real PlayerController input event pipeline and CharacterMovement physics; this is not a unit-test substitute implementation. Setup commands placed a target and applied injury.
- Inspected the visible game window with desktop computer-use tools. Temporary scene and HUD rendered.
- Results: Tests/Results/core-run01.json (initial), core-live.json (retest). Original Blender source and FBX assets generated for all species and prey. Art integration is a separate, not-yet-validated milestone.

## World/art integration — 00:35–00:57

- Original Blender source files, FBX skeletons and ten clips imported for T-Rex, raptor, triceratops and small prey.
- Built the seeded world, clearance-aware navigation, nine major AI, eighteen prey and regrowing food plants.
- First full-world launch exposed reversed procedural terrain triangles: ground was invisible from above and characters remained falling/sliding. Reversed terrain and creek winding, rebuilt and relaunched.
- Corrected scene screenshot shows terrain; runtime reports grounded stationary player. FPS cap explicitly applied on game start (initial console-config-only cap did not take effect in the editor game).
- Full-world control retest is running. AI is paused for isolated control assertions; separate autonomous AI observation is still required.
- Computer-use tool reported a physical Escape stop at 00:52; desktop automation was stopped. Development resumed under the active continuation using local build/runtime interfaces and game-generated screenshots.

## Final systems retest and visual fixes — 01:00–07:23

- Respawn clearance search now avoids nearby living capsules and world obstacles. Retest after this change: **90/90 core assertions passed** in the full world (Tests/Results/core-world02.json).
- Integration checks: **41/41 passed** (Tests/Results/integration01.json): stationary grounding, front shield reduction and rear bypass, brace/eat/charge/heavy/death animation state transitions, feeding heal and release, respawn animation recovery, all 30 directed region paths and exactly ten major combatants.
- Imported material-slot assignments were missing because the editor Python array yielded copied structs. Converted to a Python list before updating and saving; verified all slots have materials.
- Final visual inspection still showed unacceptably dark skin. Replaced the unreliable imported vertex-color dependency with editable species tint plus original surface textures; added a daylight ambient cubemap for readable shadows.
- Final build succeeded after these visual changes. Final world/AI runtime verification is ongoing; results will be recorded separately.
- The running test process remained open during the overnight interruption. Elapsed runtime alone is not evidence of an AI soak test because AI had been paused for controls; no such claim is made.

## AI and biome checks — 07:23–07:28

- Local physical traversal: **18/18 passed**, covering all six major regions with each human-playable species. Tests teleported only for setup, then drove the real character with W through each local area. This is not a complete route traversal.
- Initial autonomous observation found predators over-prioritizing faster prey, resulting in prolonged pursuit without hits, and raptors leaving the human leader too far behind.
- Fixed prey speed (950 cm/s), preference for major combatants, pursuit distance limits, pack regrouping beyond 50 m, and closer pursuit waypoint completion.
- Rebuilt and launched. **90.8-second AI retest**: nine major AI + one human, all AI moved >10 m, 22 T-Rex / 23 raptor / 18 triceratops hits, zero failed paths, zero stuck-recovery events, no major dinosaur below terrain, prey fleeing observed. Raptor leader IDs were 0 (human); all three were in With pack state at the end.
- Mean reported FPS **59.59** at 1280x720 with 10 major combatants, 18 prey and the generated environment. This is not a measured 1080p benchmark.
- Evidence: Tests/Results/final-world-checks.json, ai-observation-before-fix.json, ai-final-retest.json and ai-final-summary.json.
- User removed the 07:30 deadline at approximately 07:26 and authorized continued work. Full route traversal and further polish remain in progress.

## Settings, optional blood and water — 07:34–07:45

- Opening species-selection and pause/settings UI implemented; Escape releases the mouse. Number keys select species and resume; F2 opens settings; B toggles optional blood; +/- adjusts saved mouse sensitivity. M opens the derived region map.
- Fixed pause-state telemetry by enabling full PlayerController ticks when paused. World simulation remains paused while the menu and UI input work.
- An editor-only import commandlet hit a rooted material-expression assertion while replacing an already referenced graph. Recovered from the saved assets and authored a separate M_WorldSurface material; import retry succeeded. No operating-system crash occurred.
- Added bounded instanced blood particles with no collision or gameplay authority; disabled by default and saved as a user setting. Turning the option off clears existing particles.
- Added shallow creek wading: walking speed factor 0.65, composed with health and charge modifiers.
- **28/28 live checks passed** (Tests/Results/settings-water-blood.json): menu/pause input, species selection, sensitivity, blood off/on/clear, water slowing each species, 50% and 25% injury composition, charge rejection at critical health, recovery, leaving water, and map toggling.
- Visual inspections of the rendered opening and settings screens saved under Saved/Screenshots/Windows. Text sizing remains a polish task.

## Match scoring and extended traversal — 07:45–08:15

- **24/24 live match checks passed** (`Tests/Results/match-rules.json`): K/D/A attribution, assists, five-kill solo victory, ten-kill team victory, exactly five major combatants per team, friendly melee rejection, follower/prey exclusions, stable leader scoring, shared pack credit and ten-second respawn. Scoring setup called real ReceiveHit; these were controlled rule tests, not complete human-played matches.
- Raptor and Triceratops each completed continuous six-region circuits of about 1.75 km using real W movement and camera-relative steering, without inter-region teleports. T-Rex traversal was interrupted twice. The diagnostic retry recorded W becoming released and simulation time pausing, rather than a collision while input remained held. Automated testing is being moved to a rendered offscreen instance to isolate it from desktop focus/input changes. No T-Rex circuit pass is claimed yet.
- User reported the T-Rex lower jaw entering its head during charged attack. Reversed jaw opening rotation in quick, charge, heavy, eating and death clips. Regenerated all clips from native Blender sources, with new surface-paddling clips. **15/15 Blender jaw-direction checks passed**; these measure jaw-tip opening direction and do not replace visual collision inspection.
- Added Mirror Pond, depth-based swimming and surface-surge controls. C++ build and animation import succeeded. Live swimming regression is in progress.

## Pond and isolated traversal results — 08:15–08:27

- **40/40 swimming checks passed** (`swimming.json`): all three species float, paddle, slow down, stop under Q, apply 50%/25% injury modifiers, reject critical charge, recover from heavy attacks/surface surges, swim to shore and resume walking. All 42 directed paths among seven landmarks are reachable. Test setup placed the pawn in deep water; shore entry and AI swimming receive separate coverage.
- **18/18 continuous traversal legs passed** (`full-traversal.json` and `traversal-samples.json`), covering the six-region circuit for each species. T-Rex's previous interrupted-input stalls did not recur in the rendered offscreen instance. No per-region teleports were used. A background compilation overlapped part of this test, so elapsed traversal time is not a clean performance benchmark.
- Standalone Windows game target compiled successfully. Cooking, packaging and launching that output are still required.
- Refined asset import initially encountered Windows file locks while the old game was running. Stopped only that importer and deferred the retry until the test game closed; existing recoverable assets and source files were retained.

## Refined art and tactical AI — 08:28–08:41

- Imported four refined, continuous Blender skins with original baked UV textures. Each mesh now has three runtime LODs; successful reduction/import counts are recorded in `refined-import.json`. Native refined sources remain alongside original sources.
- Authored selection portraits from those meshes and replaced the raster-scaled HUD font with a distance-field font. Found the selector overlapped at a viewport clamped to 888x500; adjusted minimum UI scale and added ForceRes to offscreen benchmark launches. Visual retest pending.
- Visually inspected rendered T-Rex charging and heavy attack after the jaw fix: lower jaw opens below the skull. Inspected visible swimming posture and translucent pond water; screenshots 81–83 under Saved/Screenshots/Windows.
- Tactical AI estimates fight/escape confidence from current health, attack rate, movement, and nearby support. Guards have a bounded hold time and cooldown; retreat has hysteresis; pack raptors can flank frontal guards.
- Initial tactical checks were 9/11 during first-load shader warmup; isolated pursuit diagnostics showed the raptor properly pursued and killed its weakened target. Warm retest **11/11 passed** (`ai-tactics.json`), including retaliation damage, pursuit hits, increased retreat separation, actual frontal guard damage reduction, counterattack, desperate defense and supported flanking. Timing assertions now use simulation time to avoid shader-load false failures.

Checkpoint before combat polish: all four final regression scripts passed. Solo round ended at 221.53s; team round at 251.89s (2:10). Nine major AI remained active, no failed paths or below-terrain falls; team maximum stuck recoveries 3. Standalone BuildCookRun completed successfully; packaged launch still pending.


## Combat polish pass 1 — September 18

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


## Compact world / balance / audio sprint � in progress

- Protected inherited runtime evidence in e71899b (`quality-sprint-input-20260919`); existing `Dist/Windows` and `Dist/Checkpoints/2026-09-19-stable/Windows` remain untouched.
- Halved horizontal geography, landmarks, spawns, navigation bounds and map projection. Kept corridor/animal sizes and swimming depth. Regenerated and imported the overview texture.
- Current calibration: Rex 1500 HP, 187 quick, 673.2 full heavy, 0.85s charge, 240cm sweep width; raptor 520 HP, 66.6 quick, 193.14 full pounce, 0.1125 hunger/sec (1.5x base), unchanged mobility/stamina economy. Normal packs have one leader and two followers in solo and each team.
- Fixed AI target thrashing under pack hits, AI abandoning its own charge to guard, and small pounces repeatedly interrupting larger animals. Nearby raptor support now contributes appropriately to tactical confidence.
- Calibration initially used complete pack elimination; that overstates what a Rex must accomplish under the actual leader-only scoring rules. Current trials record first scoring death AND a 20-second survivor follow-through. Six calibration trials: Rex scored first 4, pack 2; 2 Rex first-score wins were counterkilled. This is preliminary, not final balance validation.
- Added original synthesized finite positional footsteps, quick/heavy motion, successful-hit and species death feedback, with per-animal and shared overlap caps.
- Live requested-rules suite: **36/36**. Live audio suite: **36/36**, including actual Unreal master-output WAV captures. Species peaks 0.095/0.111/0.104; nonzero, unclipped output. Movement/sprint cadence, no airborne/swim steps, miss-vs-impact, death-once, voice termination, pause and respawn checks passed.
- Full regression, continuous traversal, held-out final balance, longer world observations and replacement packaging are still pending. Do not treat the current source checkpoint as the final playable package.
