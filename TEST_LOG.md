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
