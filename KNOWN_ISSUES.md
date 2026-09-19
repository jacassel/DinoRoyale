# Known issues

- Combat and pack balance remains provisional. Packs can overwhelm isolated animals; avoid treating a frontal one-versus-three fight as a fair duel.
- Animations switch explicit clips without full blending or foot placement. Original models remain procedural; joints, body overlap during close combat and sliding need further polish.
- No sound effects are implemented. Combat/resource feedback is visual.
- Carcasses persist during the current game session, not across quitting/reloading. Frozen corpses are grounded approximations without ragdoll physics; reduced detail and slope intersections can be visible.
- Long-session corpse accumulation and hunger pacing need more human playtesting. Corpse rendering uses reduced LOD and distance culling.
- Map visibility is sight/noise based; AI target acquisition still uses the existing tactical/proximity system and is not a complete stealth simulation.
- Terrain/materials and vegetation retain their prototype appearance. The runtime-generated editor level appears sparse before Play.
- Private GitHub backup is unfinished: Chrome automation could not validate the current URL, and no noninteractive GitHub Git credential was available. Local Git checkpoints remain intact.

Final launch/package status and test results are recorded in TEST_LOG.md.
