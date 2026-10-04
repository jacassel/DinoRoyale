# Dino Royale — Version 0.3 Alpha Test

The locally verified Windows package is published as a
[public Alpha Test release](https://github.com/jacassel/DinoRoyale/releases/tag/v0.3.0-alpha-test).
Use TEST_LOG.md for evidence and HANDOFF.md for the recoverable checkpoint.

## Changes

- Q/E turn the dinosaur independently of the camera; W returns to locomotion.
  Species have different pivot speeds and new left/right animation clips. Heavy
  attacks keep their turning commitment. Ctrl braces; F eats; Shift sprints.
- The player map marker is a black circle and direction arrow. Movement uses
  velocity; stationary direction uses facing. Enemy reveal rules remain intact.
- Standard foliage gains mesh LODs, shorter culling and shadow distances, and
  lower grass density. Performance removes cosmetic vegetation while retaining
  terrain, water, rocks, navigation, edible plants and the same game rules.
- Top-center scoring text is black. Head labels use player names or dinosaur/pack
  labels, respect sight and distance, and toggle through Escape → F2 → N.
  The custom settings section now saves correctly between launches.
- Hosts can enable individual AI slots and choose their teams, including uneven
  matches and empty slots. Normal 5v5 remains available. Followers do not consume
  scoring slots; FFA remains five kills, teams ten, and respawns ten seconds.
- Remote mesh smoothing now caches each species' actual capsule-to-mesh offset.
  Carcass transforms are server authoritative, grounded on land or floating in
  water, and independent of the respawning character.
- Swimming state is reconciled inside movement prediction/replay. Carnivores
  weigh hunger and major threats, bound prey chases and keep pack priorities.
  Retreats select reachable routes; grid endpoints avoid isolated clearance cells.
- Fixed historical pivot input lingering after network correction and an offline
  pause action that could clear a committed attack's cooldown.

## Verification

The release gameplay executable passes **478/478** offline gameplay checks and
**577/577** separate-process multiplayer checks, plus 32 online compatibility/URL
assertions. Coverage includes both maps, all species, uneven teams, four human
peers, bots, delayed/lossy movement, pivots, water, ecology, grounded corpses and
independent respawns. Rendered network checks pass 7/7. The final settings-only
package additionally passes 124 UI/settings/core checks and 5 restart checks.
Four-minute AI world runs on each map recorded no failed routes or stuck recovery.

At the same uncapped 1600×900 output settings on the RTX 3060 test PC, Standard
plains/forest/pond scenes rose from **66/105/80 to 185/177/187 FPS**. Across the
20-scenario live workload, Standard averaged 166 FPS and Performance 178 FPS.
See PERFORMANCE_REPORT.md for thread/GPU timings, native-1080p results, memory,
sampling limits and exact settings. The ordinary 60 FPS cap remains unchanged.

Computer Use checks covered Rex/Rex, Rex/Triceratops, Triceratops/Rex,
raptor/Rex and a raptor pack/Triceratops. Native mouse attacks landed in all five;
held pivots, movement and charged attacks used the development input bridge because
the desktop tool only taps keys. See NATIVE_PLAYTEST_REPORT.md for exact evidence,
fixture corrections and the distinction from continuous human keyboard play.
Public source is at https://github.com/jacassel/DinoRoyale; the version tag is
`v0.3.0-alpha-test`. The sealed ZIP's included notes predate publication and these
last checks; the current repository and release page supersede their pending status.

The owner reports working real multiplayer on the prior release. Local loopback
tests do not establish a fresh internet EOS session. All players need this version:
compatibility is **2026100303**. EOS architecture and service identity are retained.

## Recovery and launch

Double-click **LaunchGame.bat**, or **Play Dino Royale.bat** inside the complete
`Dist/Releases/DinoRoyale-0.3-Alpha-Test/Windows` folder. The distributable is
`Dist/Releases/DinoRoyale-0.3-Alpha-Test.zip`; independent recovery is under
`Dist/Checkpoints/DinoRoyale-0.3-Alpha-Test/Windows`. The package includes a SHA256
manifest. The prior working QA2 package and recovery remain preserved.
Configured OnlineServices.ini and Saved/authentication caches are excluded from
the ZIP. See FRIEND_QUICKSTART.md for controls and the two-PC setup.
