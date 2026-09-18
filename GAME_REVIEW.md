# Playable build review

The current Unreal build is playable. This is still a rough pre-alpha, not a finished game. The latest saved gameplay/art milestone is Git commit `acffc91`.

## Open and play

1. Open File Explorer and go to `C:\Users\joel1\Documents\DinosaurBattle Prototype`.
2. Double-click **LaunchGame.bat**. It uses the installed Unreal Engine 5.8.2; do not open Component Services or Computer Management.
3. Wait for the dinosaur selection screen, then click a dinosaur or press **1**, **2**, or **3**.
4. **F3** on that screen selects solo free-for-all or 5v5 team fight. **F2** opens settings.
5. **Escape** pauses and releases the mouse. **F10** in the menu exits.

Controls: WASD move, mouse look, Space jump/surface surge, hold Q to brace, LMB quick attack, hold/release RMB charged attack, hold E to eat, M map, H help. Blood is off by default; toggle it with B in settings.

## Built and working

- Three original Blender-created playable species, rigs, eleven animation clips each, baked skin textures and three mesh LODs.
- Shared movement, health, injury, combat, food and animation components with editable balance configuration.
- Health regeneration after five seconds; 50% and 25% injury restrictions; critical charge lockout and recovery.
- Nine major AI plus small prey; raptors cooperate as packs in both modes. Only pack leaders award kill points. Followers' kills currently credit the leader.
- Bottom-left K/D/A, first-to-five solo and first-to-ten team scoring, results/restart flow, ten-second major respawns.
- Approximately 1.15 km world: plains, forest, ridge, creek, pond, hunting grounds and feeding grove. Shallow water slows walking; deep water activates paddling and buoyancy.
- Selection/settings, optional blood, map and health feedback.

## Actual verification

- Earlier complete control/health/combat regression: 90/90 assertions across all three species.
- Match-rule checks: 24/24. Settings/blood/wading: 28/28.
- Continuous map traversal: 18/18 legs, all six original regions with each species, no inter-region teleports.
- Swimming: 40/40 checks. AI swimming across the pond: 3/3, with no failed paths or stuck recoveries.
- Tactical AI: 11/11 checks for retaliation damage, pursuit hits, retreat separation, actual blocking damage reduction, guard release/counterattack and supported flanking.
- Visually inspected corrected T-Rex jaw opening, swimming posture, refined assets and the 1600x900 selection screen.
- Complete played-match observation, final regression and standalone packaging are being completed; consult TEST_LOG.md and Tests/Results for their final status.

## Remaining work toward the full goal

1. Finish cooking/packaging and launch-test the standalone Windows build.
2. Finish full solo/team match verification and review long-session AI/pack behavior and balance.
3. Improve animation blending, foot contact and some remaining deformation; the art is original and recognizable but still visibly procedural.
4. Add sound and stronger combat/locomotion feedback.
5. Improve environment density/materials and benchmark the final packaged game at 1600x900/1080p.

All source is local. Native Blender files: Assets/Source/DinosaursRefined. Gameplay code: Source/DinosaurBattle. Balance: Config/DefaultGame.ini. Logs and limitations: TEST_LOG.md and KNOWN_ISSUES.md. No purchases or public uploads were made.
