# Visual modernization — September 20, 2026

This is a graphics sprint. Gameplay rules, combat timings, AI, map layout,
collision, scoring, food, controls and menus are locked. The duplicated pasted
briefs describe the same request. The reference sheet repeats that brief; the
eight reference images guide natural coloration, anatomy and river appearance.
No reference pixels are used as game assets.

## Recovery and preexisting work

- Before-sprint source tag: `graphics-before-20260920` (`cd9ea08`).
- Existing staged `Config/DefaultInput.ini` changes belong to the user/editor;
  preserved separately in `Tests/Results/visual-modernization/DefaultInput.before.ini`.
- The previous `Dist/Windows` was preserved until the replacement passed; it is
  now retained at `Dist/Checkpoints/2026-09-20-before-visual-modernization/Windows`.
- Recovery: `Dist/Checkpoints/2026-09-20-map-results/Windows`. All 48 files
  verified against the prior package manifest with zero mismatches.
- Original Blender assets in `Assets/Source/DinosaursRefined` and `World` remain
  untouched. New native sources are in `DinosaursModern` and `WorldModern`.

## Observed causes and completed changes

Baseline: smooth pale animals, oversized black eye surrounds, identical teeth,
uniform yellow-green ground, very sparse tiered conifers, broad flat grass blades,
flat-looking water, weak surface texture differentiation.

First rendered dinosaur pass exposed overly regular stripes. The second bake
breaks and softens them; the third reduces eye surrounds and varies existing teeth.
Skeleton positions and animation durations are asserted unchanged during import.
New color, roughness and normal maps are baked locally from original procedural
materials. Texture channels are 2048 square, with three skeletal LODs retained.

Environment work: layered soil/stone/bark textures, wet-bank masks, depth-colored
water with two ripple layers and screen-space reflection, fuller canopy geometry,
slender curved grass and visual-only shoreline stones. Extra foliage uses an
independent random stream so obstacle/food placement remains unchanged.
Lighting uses fixed exposure, neutral sunlight, contact shadows, four cascades,
ambient occlusion, light atmospheric haze and restrained bloom/saturation.

## Completion gates (completed; evidence below)

1. Inspect front/side/rear and close views of all three species in the running game.
2. Inspect all species in plains, forest, ridge, river and pond, including shadows.
3. Inspect Rex idle/walk/run/quick/charge/heavy/turn/swim/eat/injury/death and the
   jaw through its attack frames. Inspect other species' action presentation.
4. Verify blood ON/OFF, clearing of particles, and retained creature materials.
5. Run core input/health/stamina/combo/injury/food/AI/swimming/match/map regression.
6. Measure warmed runtime performance at normal launch resolution and inspect logs
   for shader errors, rendering instability and crashes.
7. Package to a candidate folder, test that actual package, preserve a hashed
   recovery package, and only then promote it to `Dist/Windows`.
8. Verify normal launcher and native controls; update README/TEST_LOG/KNOWN_ISSUES/
   HANDOFF, commit the visual work, and provide the requested short report.

Evidence is under `Tests/Results/visual-modernization`. Import/build success alone
does not satisfy the visual gates. This file records the completed sprint,
including rejected passes, corrected test fixtures and remaining visual limits.

## Completed visual review

Reviewed all nine front/side/rear survey views and all 51 detailed runtime captures
at 1600x900. These cover all three species in bright plains, forest shadow,
river, pond, ridge and grove, plus walking, sprinting, turning, quick attacks,
charge, heavy, swimming, active feeding, damage and death. No inverted joints,
exploded vertices or upward jaw penetration appeared in these views. Original
clip transitions and foot sliding remain; no gameplay animation timing changed.

The Rex source jaw audit passed all 86 frames across Charge, Heavy and Quick.
The head-relative jaw probe never moved above its closed position. Maximum
opening was 22.50 / 61.49 / 43.70 Blender units respectively. This analytic check
complements rendered inspection; it is not an exhaustive mesh collision proof.

Visual iterations fixed excessive zebra-like coloration, buried irises, black eye
surrounds and uniform teeth. Unreal iteration fixed incompatible graph pin names
and FBX-added foliage slots that retained WorldGridMaterial. The final foliage
binding is applied to every slot. Original rigs, all 33 animation durations,
three skeletal LODs per species, map obstacles, feeding spots and water physics
are retained. The grass and bank stones use a separate random stream and do not
collide or affect navigation.

With the corrected foliage shader, three warmed 20-second live-AI scene samples
measured 59.97 / 60.00 / 59.96 FPS on the RTX 3060 at 1600x900, capped at 60.
Sampled p95 frame times were 16.667ms; maximum sampled frame time was 16.72ms.
These are bounded samples, not worst-case performance or a complete hitch trace.

Remaining art limits: procedural skull/limb/fern/rock silhouettes, simple clip
transitions, some sliding/body overlap, repetitive terrain at distance and
screen-space reflection limitations. The reference images remain a higher art
quality target; this pass improves the existing game within practical limits.


## Regression fixture corrections

The material follow-up initially reported 14/17 because it asserted the old
`M_*_BakedSkin` names. It now requires every slot to contain the appropriate new
`M_*_Modern` material and passed 17/17.

The AI tactics test initially reported 9/11. Running the unchanged test against
the untouched pre-sprint standalone reproduced the same two failures and damage
values. The guard test held the threat beyond the AI's finite guard window; it
now presents a fully charged threat and releases during the observed brace. The
injured Triceratops fixture used 18% health, which still produces 0.616 fight
confidence under the retained balance. It now uses 6% and explicitly requires
fight confidence below 0.35 and escape confidence below 0.38 while bracing.
Original failures and old-package comparison results are preserved. No gameplay
AI, combat or balance code was changed to satisfy these tests.


The corrected tactics fixture passed 11/11 on both the prior standalone and the
visual build, including 67.32 damage from a braced Rex heavy. The first 0.12-second
fixed observation was too early for an AI thinking tick; the final fixture waits
(up to 0.6 seconds) for the actual Bracing state before releasing.

All 18 continuous player routes passed. During overlapping package archiving,
the AI swimming suite recorded two successful crossings then a telemetry read
timeout; the game process remained responsive and its log contained no crash.
The interrupted results are retained. With build work finished, the isolated
rerun passed all three crossings, with zero failed paths. Triceratops used one
normal crowded-bank recovery before reaching shore.

Standalone BuildCookRun succeeded in 328.86 seconds with zero reported compiler
warnings/errors or failed shader compilations. The candidate stayed separate
from the existing playable until packaged regression passed.


The allied-map occluder check sampled the same frame as wall creation: a directed
trace recorded `inSight=true` at 0 seconds and `false` by 0.052 seconds, with the
ally marker current throughout. Added a 0.2-second physics-settle wait; 17/17 pass.
The map-pins fixture assumed 1280x720 coordinates while this sprint tests at
1600x900. Cursor positions now use the configured viewport dimensions, with
half-pixel world-coordinate tolerance. All 21 map/result assertions pass.

The complete source regression now passes **462/462 across 15 suites**, including
all-species controls/combat/resources/animation, ecology, tactics, player and AI
swimming, 18 continuous regional traversals, allied visibility, pins and solo
results. No production gameplay change was needed for the fixture corrections.
Source evidence is in `regression/summary.json`; interrupted and original failed
attempts remain beside it. Account allowance at this milestone: 72% remaining;
no separate Astra allowance exposed. No purchases or delegation.


Packaged regression passed 220/220 on the actual new executable. All 27 packaged
combat captures were reviewed, including 15 Rex jaw-angle/attack-phase views.
The separate twelve blood/material assertions passed for all three species.

A 240.65-second packaged AI observation passed after regression/capture work:
187 landed hits, zero failed paths, terrain falls, stuck recoveries or immediate
respawn hits; longest stationary travel observation one second; up to 20 corpses.
The session's cumulative mean was 58.44 FPS. This includes preceding test work
and is separate from the warmed per-scene counter-delta samples.

## Final promotion and native launch

Packaged warmed 20-second scenes averaged 60.00 / 58.81 / 59.41 FPS (plains,
forest, pond) at 1600x900 on the RTX 3060. One 194.8ms pond frame was sampled.
A 45-second pond follow-up averaged 59.94 FPS with no sampled frames above 50ms.
The initial hitch remains a documented limitation; sampling is not a full trace.

All 48 runtime files match by SHA-256 between the tested candidate, new recovery
and promoted `Dist/Windows`. The previous playable is retained independently.
`stable-package-manifest.json` records the paths, sizes and hashes. Normal
`LaunchGame.bat` launched the promoted executable without the development bridge
or a permission prompt. Native selection, camera, map, R pin, pause and F10 quit
were verified; movement/sprint/jump inputs were exercised. The normal log closed
with `LogExit: Exiting.` and both processes ended. Menu portraits still use their
prior artwork; all runtime dinosaurs use the new assets.

Source checkpoint: `visual-modernization-stable-20260920`. README, TEST_LOG,
KNOWN_ISSUES and HANDOFF document the completed result. The user/editor's staged
input configuration remains unchanged and outside this source commit. Final
account allowance: 70% remaining; no separate Astra allowance exposed.
