# Known issues

- Combat balance is based on bounded AI trials, not competitive human playtesting. Retreats can produce unresolved encounters; first scoring deaths and subsequent counterkills are recorded separately.
- Animations switch explicit clips without full blending or foot placement. Original models remain procedural; joints, body overlap during close combat and sliding need further polish.
- Sounds are original synthesized prototype effects, rather than production creature recordings.
- Carcasses persist during the current game session, not across quitting/reloading. Frozen corpses are grounded approximations without ragdoll physics; reduced detail and slope intersections can be visible.
- Long-session corpse accumulation and hunger pacing still need human playtesting beyond the bounded world runs. Up to 67 persistent carcasses were observed over twelve simulated minutes at approximately 60 FPS; reduced LOD and distance culling limit their cost.
- Map visibility is sight/noise based; AI target acquisition still uses the existing tactical/proximity system and is not a complete stealth simulation.
- Terrain/materials and vegetation retain their prototype appearance. The runtime-generated editor level appears sparse before Play.
- Private GitHub backup is unfinished: Chrome automation could not validate the current URL, and no noninteractive GitHub Git credential was available. Local Git checkpoints remain intact.

Final launch/package status and test results are recorded in TEST_LOG.md.
