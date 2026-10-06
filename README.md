# Dino Royale — Version 0.5

[Download the Windows Alpha Test](https://github.com/jacassel/DinoRoyale/releases/tag/v0.5.0)
· [Public source repository](https://github.com/jacassel/DinoRoyale)

A six-species dinosaur combat prototype with offline free-for-all, team battles,
and working Epic Online Services multiplayer. The owner confirms multiplayer has
already been played successfully across different networks. Version 0.5 adds
Ankylosaurus, Brachiosaurus and Pachycephalosaurus, original rigged assets,
81 distinct new sound clips, tree browsing, Pachy packs, a live FFA leaderboard,
and assist-based scoring. The existing EOS provider and service identity are preserved.
All peers must use compatibility **2026100505**.

See [VERSION05_RELEASE_NOTES.md](VERSION05_RELEASE_NOTES.md), [TEST_LOG.md](TEST_LOG.md)
and [KNOWN_ISSUES.md](KNOWN_ISSUES.md) for this update's tests and limitations.
[EOS_SETUP.md](EOS_SETUP.md) explains online setup.

## Launch

Open `C:\Users\joel1\Documents\DinosaurBattle Prototype` in File Explorer and double-click **LaunchGame.bat**.
The launcher prefers the locally tested **Version 0.5** package at
`Dist\Releases\DinoRoyale-0.5\Windows`.
For another computer, extract `Dist\Releases\DinoRoyale-0.5.zip` into a fresh
folder and run **Windows\Play Dino Royale.bat**. Keep the entire folder together.
Copy each computer's existing configured `OnlineServices.ini` to its new
`Windows\DinosaurBattle` subfolder before online play; credentials are excluded
from the ZIP. Both players must use 0.5. The prior QA2 friends package, its independent
recovery, QA1 and `Dist\Windows` remain preserved. Version 0.5 verification is recorded in TEST_LOG.md. Local multi-process regression
checks complement the owner's existing successful different-network play; they
are not a new two-PC internet session.
Choose a dinosaur with **1-6** or its portrait. **Escape** opens the menu;
**F10** from the menu exits.
Double-click `LaunchEditor.bat` to open the project for editing.
The project file is `DinosaurBattle.uproject`; the startup level is `Content/Maps/LostValley.umap`.
The Unreal module, executable, internal paths and EOS artifact remain
`DinosaurBattle` to preserve existing cooked assets and service identity.
Player-facing branding is Dino Royale. Epic organization/product names are unchanged.

Engine: Unreal Engine **5.8.2**. Original assets: **Blender 5.2.2 LTS**.
Offline play needs no account. Multiplayer uses Epic account sign-in, EOS lobbies and EOS P2P relay.
No services, subscriptions, assets or hosting were purchased.

## Controls

| Input | Action |
|---|---|
| WASD | Camera-relative movement |
| Mouse | Look |
| Hold Left Shift | Sprint; consumes stamina |
| Space | Jump; surface surge while swimming |
| Hold Q / E | Pivot left / right without rotating the camera; WASD returns to locomotion |
| Hold Ctrl | Defensive brace; movement is disabled |
| LMB | Up to three quick strikes, then species-specific recovery |
| Hold / release RMB | Charge / execute heavy attack |
| Hold F near suitable food | Eat; release or move to stop |
| 1 / 2 / 3 / 4 / 5 / 6 | Select Rex / Raptor / Triceratops / Ankylosaurus / Brachiosaurus / Pachy and start a round |
| P | Toggle the live FFA leaderboard without pausing |
| Escape | Open menu; pauses offline, continues the world online |
| F4 in selection | Multiplayer: Epic sign-in, Host Game or Join Game |
| F3 in selection | Switch solo free-for-all / 5v5 team fight |
| F2 in selection | Settings: B toggles blood; N toggles name tags; +/- changes mouse sensitivity |
| Enter | Resume; start another round after results |
| M / H | Map / control help |
| R with map open | Pin cursor location; R near a pin removes it (up to eight pins) |
| F10 in the menu | Quit |

The game opens on dinosaur selection. Choose one of the six cards or press 1-6. FFA ends at **5 points**: each kill adds one, and every two assists add one. Team Battle ends at **10 team points**: each kill adds one, and every three assists pooled across teammates add one. Actual K/D/A remain separate. The current FFA leader appears in black text at the top; P shows the standings. The human and nine AI fill the match. Major dinosaurs respawn after **10 seconds**.

Online: the host plays and chooses 2–10 main participant slots, FFA or Team Battle,
bots ON/OFF and public/invite-only visibility. Guests select species/team in the
lobby and mark Ready; the host starts. Team Battle allows up to five per team,
including smaller and uneven matches. The host can enable/disable each AI slot
and choose Team A/B in its roster row; disabled slots remain empty. The 5v5 preset
restores the ordinary balanced roster. Bots yield to joining humans. Each human or bot Raptor or Pachy leader owns two extra followers; these do not
consume participant slots. Only leaders award kills. The host leaving ends the
match for everyone. Use the in-game lobby controls to rematch or return to lobby.
Choose **Sungrass Plains — Standard** or **Performance** on the selection screen or
in the host setup/lobby. Performance retains terrain, rocks, water, edible plants,
and match rules including **18 edible trees for Brachiosaurus**, while removing cosmetic trees, grass and ferns. The host's selection
applies to every peer. Changing the offline map starts a fresh round.

Name tags default ON and save locally. Open Escape, F2, then press N or click the
name-tag row. Tags use multiplayer display names or dinosaur/pack labels; they require
line of sight and remain within 60 meters. The toggle changes labels, not enemy reveal rules.

Send friends [FRIEND_QUICKSTART.md](FRIEND_QUICKSTART.md) with the complete Windows folder.

At round end, the game pauses on results showing scoring competitors' points, kills, deaths and assists, including when an AI wins solo play. Press Enter to start again. Map pins appear as gold diamonds on both maps, survive respawns, and clear for a new round. Open M, point at a location, then press R; adding a ninth pin replaces the oldest. The world continues while you use the map; close it with M to resume movement and mouse look.

Raptors and Pachycephalosaurs cooperate in three-member packs: one scoring leader and two AI followers, in both modes. Each pack has exactly one leader and two followers. Only the pack leader awards a kill when defeated. Followers' kills currently credit their leader (`SharePackKills=True` in the match configuration). Prey and pack followers do not add points to the kill goal. Kills, deaths and assists appear at the bottom left.

Blood is optional and off by default. Below 50% health movement and attacks slow; below 25% the slowdown increases and charged attacks are unavailable. Regeneration starts five seconds after damage; eating restores health faster. The creek slows walking, and Mirror Pond contains deeper swimming water.

## Current systems

- Shared character, health, stamina, injury, combat and feeding components.
- Four lightweight AI personalities: aggressive, defensive, skirmisher and balanced; all use the same stamina and cooldown rules.
- Configurable species values in `Config/DefaultGame.ini`, including charge, regeneration, camera and movement tuning.
- Original rigged dinosaur meshes and thirteen clips per original species and sixteen clips per new species, including swimming.
- Nine major AI dinosaurs plus eighteen smaller prey. Carnivores hunt; Raptors and Pachys share a leader; herbivores defend feeding areas; prey flee.
- Seeded terrain approximately 575 m across (half the previous travel scale), with plains, forest, ridge, creek, pond, hunting grounds and feeding groves.
- Clearance-aware grid navigation and local obstacle avoidance.
- Species-specific positional quick attacks, charge-up/heavy attacks, impacts, hurt reactions, sprint breaths, injured breaths and death sounds. Layered CC0 recordings have three variations per event, finite playback and overlap limits; see `Assets/Audio/CREDITS.md`.
- Runtime-generated world: the empty saved map is populated by GameMode when Play starts.

## Development

Run `powershell -ExecutionPolicy Bypass -File Tools/Build.ps1` to build the editor module using the local portable Microsoft toolchain. The toolchain is excluded from Git; its setup is recorded in SESSION.md. No registry/OS-wide compiler installation was made.

Original Blender automation is in `Tools/Blender`; native sources are in `Assets/Source`, FBX interchange files in `Assets/Export`, and imported Unreal assets in `Content/Dinosaurs` and `Content/World`.

The visual modernization sources are `Assets/Source/DinosaursModern` and `Assets/Source/WorldModern`. They retain the original rigs and animation timings, with baked 2048px dinosaur color/normal/roughness maps, revised eyes/mouths/teeth and fuller vegetation. Unreal adds layered soil/rock/bark, wet banks, depth-colored water with moving normals and screen-space reflections, contact shadows and restrained grading. Original source assets remain available. See `VISUAL_SPRINT.md` for the iteration and verification record.

The sound/terrain pass adds 81 native sound assets, 502 trees, clustered groves and
rolling hills reaching approximately 26m within the map interior. Existing water
basins and main travel corridors are retained. `Tools/Art/build_creature_audio.py`
rebuilds the WAV files; `Tools/Unreal/import_creature_audio.py` imports them.
See `SOUND_TERRAIN.md` for scope, source credits, measurements and the listening
limitation of the development session.

Live tests require launching with `-DinoDevBridge`. This opt-in local bridge accepts test setup and real input events through `Saved/Automation/command.json`, and writes runtime state to `telemetry.json`. It is disabled during a normal launch. Test scripts and recorded results are in `Tools/Tests` and `Tests/Results`.

See TEST_LOG.md for **actually performed** tests and KNOWN_ISSUES.md for current limitations.

## Combat polish balance

All values are editable in `Config/DefaultGame.ini`. Normal walking never costs stamina. Base stamina regeneration is 22/sec standing and 14/sec walking, modified by hunger; eating restores another 32/sec. Regeneration waits 0.4 seconds after spending; attacking/airborne regeneration is reduced. Jump costs 18. At zero stamina sprint, jump, heavy attacks and brace are disabled until at least 1.2 seconds and 25 stamina have recovered. A weaker, slower quick attack remains available.

| Species | Health | Quick damage | Full heavy | Quick interval | Third-strike extra recovery | Sprint speed / drain | Quick / heavy cost |
|---|---:|---:|---:|---:|---:|---|---|
| T-Rex | 1500 | 187 | 673.2 | 0.58s | 0.95s | 1449 cm/s / 15 per sec | 10 / 36 |
| Raptor | 520 | 66.6 | 193.14 | 0.31s | 0.50s | 2400 cm/s / 9 per sec | 7 / 26 |
| Triceratops | 1650 | 155 | 511.5 | 0.53s | 0.75s | 1317.5 cm/s / 13 per sec | 9 / 34 |
| Ankylosaurus | 1800 | 145 | 551 | 0.72s | 0.90s | 877.5 cm/s / 16 per sec | 12 / 40 |
| Brachiosaurus | 3100 | 230 | 713 | 0.95s | 0.90s | 783 cm/s / 24 per sec | 18 / 62 |
| Pachycephalosaurus | 950 | 116 | 429.2 | 0.46s | 0.90s | 1687.5 cm/s / 11 per sec | 8 / 32 |

Damage above is before armor and defense. Ankylosaurus takes 60% of incoming damage and Brachiosaurus 90%; frontal brace further reduces it to 10% / 17% respectively. Pachy brace is 25%. Brachiosaurus has a small traversal step instead of a giant jump. New attacks follow tail-club, foot/tail and skull bones with a single hit per attack; heavies have committed movement and punishable recovery.

The third quick strike gains 12% damage. Heavy attacks commit forward movement, restrict turning, knock unbraced opponents back and interrupt charging when the attacker is large enough; raptor pounces cannot repeatedly cancel a larger dinosaur's charge. A miss adds 0.50 / 0.25 / 0.45 seconds recovery respectively. T-Rex lunges, raptor pounces, and Triceratops drives forward with its horns. Sprint turning is particularly restricted for Triceratops. Eating is interrupted by damage and cannot restart for 2.5 seconds.

## Hunger, food and map visibility

Hunger starts full and declines gently: Rex 0.075, raptor 0.1125, Triceratops 0.06 points/second. Sprinting adds 0.035 for Rex/Triceratops and 0.0525 for raptors. Raptors consume 150% of the ordinary base rate, including the sprint surcharge. Without feeding, a walking raptor reaches 70% in about 4.4 minutes; severe starvation takes much longer.

| Hunger | Passive health regeneration | Passive stamina regeneration |
|---|---|---|
| 85-100% | 150% of base rate | 125% of base rate |
| 70-85% | Normal | Normal |
| 40-70% | 50% | 65% |
| Above 20%, below 40% | None | 40% |
| 20% or less | None | None |
| 10% or less | Lose 0.25% maximum health/sec | None |

Eating bypasses these passive restrictions: food restores 18 hunger/sec, 32 stamina/sec, and 12% maximum health/sec while available. A tiny prey carcass has 25 food units; raptor 120; Rex 360; Triceratops 480. Consumption is 30 / 18 / 24 units/sec for Rex / raptor / Triceratops. Carcasses persist **within the current session**, independently of respawn, until consumed. They are non-blocking and their animation freezes after collapse to reduce cost. Plants contain 120 units, disappear when depleted, and regrow after 120 seconds. Carnivores eat carcasses. Triceratops, Ankylosaurus and Pachycephalosaurus eat the same shrubs. Brachiosaurus eats tree foliage only. Eighteen browse trees remain on both maps; each holds 480 food units and regrows after 120 seconds. Depletion removes foliage while retaining the visible trunk and collision. Available food has a subtle green-gold outline for the species that can eat it; Brachiosaurus players highlight trees, while the other herbivores highlight shrubs. The cue respects visible surfaces, disappears on depletion, and returns on regrowth.

In team play, all living allies (including raptor followers) are always visible at their current positions on both the minimap and expanded map, including while you wait to respawn. Their markers return immediately when they respawn. Enemies, and other dinosaurs in solo play, appear only in line of sight or after they attack, charge or sprint. Noisy actions reveal a position for six seconds; ongoing sprint/charge keeps it updated. Once an animal goes quiet and out of sight, the marker holds its last revealed location until it expires. Health labels also respect line of sight. The player's black map circle stays visible; its arrow follows movement, or facing while stationary.


