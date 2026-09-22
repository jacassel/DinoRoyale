# Known issues

- EOS authentication, public discovery, invitations, relay connectivity and different-network play are **NOT VERIFIED**. The project has no configured EOS product and the developer portal is signed out. Follow EOS_SETUP.md; use two authorized Epic accounts for the real internet test.
- A Windows Firewall prompt occluded part of native verification windows. ESC/F2 keyboard checks worked, but a native mouse click was blocked by the prompt. It was not accepted; its permission choice still needs the user. Automated mapped-input gameplay tests passed separately.
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
