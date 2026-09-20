# Dinosaur Battle — Pre-Alpha 0.1

A local single-player dinosaur combat prototype in active development.

## Launch

Open `C:\Users\joel1\Documents\DinosaurBattle Prototype` in File Explorer and double-click **LaunchGame.bat**.
It starts the standalone build in `Dist\Windows`; keep that entire folder together. If the packaged build is absent, the launcher falls back to the installed Unreal Engine 5.8.2.
Choose a dinosaur with **1 / 2 / 3**. **Escape** pauses; **F10** from the menu exits.
Double-click `LaunchEditor.bat` to open the project for editing.
The project file is `DinosaurBattle.uproject`; the startup level is `Content/Maps/LostValley.umap`.

Engine: Unreal Engine **5.8.2**. Original assets: **Blender 5.2.2 LTS**.
No commercial models, online gameplay, accounts, external services or paid APIs are used.

## Controls

| Input | Action |
|---|---|
| WASD | Camera-relative movement |
| Mouse | Look |
| Hold Left Shift | Sprint; consumes stamina |
| Space | Jump; surface surge while swimming |
| Hold Q | Defensive brace; movement is disabled |
| LMB | Up to three quick strikes, then species-specific recovery |
| Hold / release RMB | Charge / execute heavy attack |
| Hold E near suitable food | Eat; release or move to stop |
| 1 / 2 / 3 | Select T-Rex / Velociraptor / Triceratops and start a new round |
| Escape | Pause, release the mouse, open dinosaur selection |
| F3 in selection | Switch solo free-for-all / 5v5 team fight |
| F2 in selection | Settings: B toggles blood; +/- changes mouse sensitivity |
| Enter | Resume; start another round after results |
| M / H | Map / control help |
| F10 in the menu | Quit |

The game opens on dinosaur selection. Choose a card or press 1, 2 or 3 to begin. In solo mode the first competitor to **5 kills** wins; in team mode the first team to **10 kills** wins. The human and nine AI fill the match. Major dinosaurs respawn after **10 seconds**.

Raptors cooperate in three-member packs: one scoring leader and two AI followers, in both modes. Only the pack leader awards a kill when defeated. Followers' kills currently credit their leader (`SharePackKills=True` in the match configuration). Prey and pack followers do not add points to the kill goal. Kills, deaths and assists appear at the bottom left.

Blood is optional and off by default. Below 50% health movement and attacks slow; below 25% the slowdown increases and charged attacks are unavailable. Regeneration starts five seconds after damage; eating restores health faster. The creek slows walking, and Mirror Pond contains deeper swimming water.

## Current systems

- Shared character, health, stamina, injury, combat and feeding components.
- Four lightweight AI personalities: aggressive, defensive, skirmisher and balanced; all use the same stamina and cooldown rules.
- Configurable species values in `Config/DefaultGame.ini`, including charge, regeneration, camera and movement tuning.
- Original rigged dinosaur meshes and eleven animation clips per species, including swimming.
- Nine major AI dinosaurs plus eighteen smaller prey. Carnivores hunt; raptors share a leader; triceratops defend feeding areas; prey flee.
- Seeded terrain approximately 575 m across (half the previous travel scale), with plains, forest, ridge, creek, pond, hunting grounds and feeding groves.
- Clearance-aware grid navigation and local obstacle avoidance.
- Original positional footsteps, quick/heavy attack motion, successful impacts, and species death sounds, with finite playback and overlap limits.
- Runtime-generated world: the empty saved map is populated by GameMode when Play starts.

## Development

Run `powershell -ExecutionPolicy Bypass -File Tools/Build.ps1` to build the editor module using the local portable Microsoft toolchain. The toolchain is excluded from Git; its setup is recorded in SESSION.md. No registry/OS-wide compiler installation was made.

Original Blender automation is in `Tools/Blender`; native sources are in `Assets/Source`, FBX interchange files in `Assets/Export`, and imported Unreal assets in `Content/Dinosaurs` and `Content/World`.

Live tests require launching with `-DinoDevBridge`. This opt-in local bridge accepts test setup and real input events through `Saved/Automation/command.json`, and writes runtime state to `telemetry.json`. It is disabled during a normal launch. Test scripts and recorded results are in `Tools/Tests` and `Tests/Results`.

See TEST_LOG.md for **actually performed** tests and KNOWN_ISSUES.md for current limitations.

## Combat polish balance

All values are editable in `Config/DefaultGame.ini`. Normal walking never costs stamina. Base stamina regeneration is 22/sec standing and 14/sec walking, modified by hunger; eating restores another 32/sec. Regeneration waits 0.4 seconds after spending; attacking/airborne regeneration is reduced. Jump costs 18. At zero stamina sprint, jump, heavy attacks and brace are disabled until at least 1.2 seconds and 25 stamina have recovered. A weaker, slower quick attack remains available.

| Species | Health | Quick damage | Full heavy | Quick interval | Third-strike extra recovery | Sprint speed / drain | Quick / heavy cost |
|---|---:|---:|---:|---:|---:|---|---|
| T-Rex | 1500 | 187 | 673.2 | 0.58s | 0.95s | 1449 cm/s / 15 per sec | 10 / 36 |
| Raptor | 520 | 66.6 | 193.14 | 0.31s | 0.50s | 2400 cm/s / 9 per sec | 7 / 26 |
| Triceratops | 1650 | 155 | 511.5 | 0.53s | 0.75s | 1317.5 cm/s / 13 per sec | 9 / 34 |

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

Eating bypasses these passive restrictions: food restores 18 hunger/sec, 32 stamina/sec, and 12% maximum health/sec while available. A tiny prey carcass has 25 food units; raptor 120; Rex 360; Triceratops 480. Consumption is 30 / 18 / 24 units/sec for Rex / raptor / Triceratops. Carcasses persist **within the current session**, independently of respawn, until consumed. They are non-blocking and their animation freezes after collapse to reduce cost. Plants contain 120 units, disappear when depleted, and regrow after 120 seconds. Carnivores eat carcasses; Triceratops eats plants. Available edible plants have a subtle green-gold outline while playing Triceratops. The cue respects visible surfaces, disappears on depletion, and returns on regrowth.

The map shows other dinosaurs only in line of sight or after they attack, charge or sprint. Noisy actions reveal a position for six seconds; ongoing sprint/charge keeps it updated. Once an animal goes quiet and out of sight, the marker holds its last revealed location until it expires. Health labels also respect line of sight. The player's own map arrow stays visible.


