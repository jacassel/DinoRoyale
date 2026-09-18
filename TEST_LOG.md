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
