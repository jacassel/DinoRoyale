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
