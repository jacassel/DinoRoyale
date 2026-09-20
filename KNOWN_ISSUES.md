# Known issues

- Combat balance is based on bounded AI trials, not competitive human playtesting. Retreats can produce unresolved encounters; first scoring deaths and subsequent counterkills are recorded separately.
- Animations switch explicit clips without full blending or foot placement. Original models remain procedural; joints, body overlap during close combat and sliding need further polish.
- Sounds are original synthesized prototype effects, rather than production creature recordings.
- Dinosaur-selection portraits retain the prior artwork; the new meshes and materials appear in gameplay.
- Packaged 1600x900 samples averaged 58.8–60 FPS on the RTX 3060, but one 195ms pond hitch was observed. A 45-second follow-up averaged 59.94 FPS without sampled frames over 50ms; occasional loading/streaming or system hitches remain possible. These bounded samples are not a full frame trace.
- Carcasses persist during the current game session, not across quitting/reloading. Frozen corpses are grounded approximations without ragdoll physics; reduced detail and slope intersections can be visible.
- Long-session corpse accumulation and hunger pacing still need human playtesting beyond the bounded world runs. Up to 67 persistent carcasses were observed over twelve simulated minutes at approximately 60 FPS; reduced LOD and distance culling limit their cost.
- Map visibility is sight/noise based; AI target acquisition still uses the existing tactical/proximity system and is not a complete stealth simulation.
- The visual modernization adds textured skin, differentiated surfaces, layered water and fuller foliage, but anatomy, fern silhouettes, large boulders and distant terrain still reveal their procedural origins. This is not photorealistic reference-level art. Water uses screen-space reflections, which can lose reflected objects near screen edges; it has no physical waves or wake simulation. The runtime-generated editor level appears sparse before Play.
- Private GitHub backup is unfinished: Chrome automation could not validate the current URL, and no noninteractive GitHub Git credential was available. Local Git checkpoints remain intact.

Final launch/package status and test results are recorded in TEST_LOG.md.
