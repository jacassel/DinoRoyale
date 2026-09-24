# Known issues

- The owner's two-PC screenshots confirm Epic sign-in, lobby creation, social presence and invite delivery on the earlier release. QA1 repairs a reproduced Int64/Int32 compatibility-read defect. **The corrected package still needs manual sign-in and the owner's visual two-PC invite/browser join verification.** Internet gameplay and different-network relay connectivity remain unverified. See MULTIPLAYER_QA_REPORT.md.
- The original public Refresh timeout is unresolved: no guest timeout log was supplied. The corrected typed reader prevents compatible results being discarded, and new operation/callback diagnostics distinguish filtering from a provider timeout. Restart after a timed-out online operation.
- Live sign-in for QA1 reached Epic's authentication screen and timed out awaiting manual completion. Hosting/discovery/overlay-button operation on this replacement could not yet be verified. Earlier firewall-prompt limitations are historical; no security settings were changed.
- The largest test used ten real packaged game processes on one PC over explicit development loopback sockets. It was headless: its roughly 60 FPS simulation rate is not ten-window rendering performance or WAN evidence. Two-minute FFA and team runs do not establish long-session stability.
- Combat is server authoritative without historical hit rewind. Host advantage and delayed hit feedback are expected under latency; competitive balance needs real human internet playtesting. Injected delays and 2% loss passed the recorded correctness checks.
- There is no host migration: host departure returns guests to the menu. A timed-out online operation requires restarting before retrying online. Offline play remains available.
- The browser currently displays up to seven compatible results at once. Large public populations, account restrictions and Epic overlay behavior need live EOS validation. No production anti-cheat, dedicated hosting, voice chat or Steam integration is included.

- Combat balance is based on bounded AI trials, not competitive human playtesting. Retreats can produce unresolved encounters; first scoring deaths and subsequent counterkills are recorded separately.
- Animations switch explicit clips without full blending or foot placement. Original models remain procedural; joints, body overlap during close combat and sliding need further polish.
- Creature sounds are cinematic designs layered from credited CC0 recordings. The agent could verify timing, playback, levels and clipping but could not directly listen in this session; human listening remains necessary to judge realism and the final mix.
- Dinosaur-selection portraits retain the prior artwork; the new meshes and materials appear in gameplay.
- Latest sound/terrain packaged 1600x900 samples averaged 59.49–60.00 FPS on the RTX 3060. One 169ms pond hitch was sampled; occasional loading/streaming or system hitches remain possible (the preceding graphics checkpoint also recorded a 195ms hitch). These bounded samples are not a full frame trace.
- Carcasses persist during the current game session, not across quitting/reloading. Frozen corpses are grounded approximations without ragdoll physics; reduced detail and slope intersections can be visible.
- Long-session corpse accumulation and hunger pacing still need human playtesting beyond the bounded world runs. Up to 67 persistent carcasses were observed over twelve simulated minutes at approximately 60 FPS; reduced LOD and distance culling limit their cost.
- Map visibility is sight/noise based; AI target acquisition still uses the existing tactical/proximity system and is not a complete stealth simulation.
- The visual modernization adds textured skin, differentiated surfaces, layered water and fuller foliage, but anatomy, fern silhouettes, large boulders and distant terrain still reveal their procedural origins. This is not photorealistic reference-level art. Water uses screen-space reflections, which can lose reflected objects near screen edges; it has no physical waves or wake simulation. The runtime-generated editor level appears sparse before Play.
- Private GitHub backup is unfinished: Chrome automation could not validate the current URL, and no noninteractive GitHub Git credential was available. Local Git checkpoints remain intact.

Final launch/package status and test results are recorded in TEST_LOG.md.
