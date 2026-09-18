# Dinosaur Battle — Pre-Alpha 0.1

A local single-player dinosaur combat prototype in active development.

## Launch

Double-click `LaunchGame.bat` to play with the installed Unreal Engine 5.8.2.
Double-click `LaunchEditor.bat` to open the project for editing.
The project file is `DinosaurBattle.uproject`; the startup level is `Content/Maps/LostValley.umap`.

Engine: Unreal Engine **5.8.2**. Original assets: **Blender 5.2.2 LTS**.
No commercial models, online gameplay, accounts, external services or paid APIs are used.

## Controls

| Input | Action |
|---|---|
| WASD | Camera-relative movement |
| Mouse | Look |
| Space | Jump; surface surge while swimming |
| Hold Q | Defensive brace; movement is disabled |
| LMB | Quick bite, slash or horn attack |
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

Raptors cooperate as packs in both modes. Only the pack leader awards a kill when defeated. Followers' kills currently credit their leader (`SharePackKills=True` in the match configuration). Prey and pack followers do not add points to the kill goal. Kills, deaths and assists appear at the bottom left.

Blood is optional and off by default. Below 50% health movement and attacks slow; below 25% the slowdown increases and charged attacks are unavailable. Regeneration starts five seconds after damage; eating restores health faster. The creek slows walking, and Mirror Pond contains deeper swimming water.

## Current systems

- Shared character, health, injury, combat and feeding components.
- Configurable species values in `Config/DefaultGame.ini`, including charge, regeneration, camera and movement tuning.
- Original rigged dinosaur meshes and eleven animation clips per species, including swimming.
- Nine major AI dinosaurs plus eighteen smaller prey. Carnivores hunt; raptors share a leader; triceratops defend feeding areas; prey flee.
- Seeded terrain approximately 1.15 km across, with plains, forest, ridge, creek, pond, hunting grounds and feeding groves.
- Clearance-aware grid navigation and local obstacle avoidance.
- Runtime-generated world: the empty saved map is populated by GameMode when Play starts.

## Development

Run `powershell -ExecutionPolicy Bypass -File Tools/Build.ps1` to build the editor module using the local portable Microsoft toolchain. The toolchain is excluded from Git; its setup is recorded in SESSION.md. No registry/OS-wide compiler installation was made.

Original Blender automation is in `Tools/Blender`; native sources are in `Assets/Source`, FBX interchange files in `Assets/Export`, and imported Unreal assets in `Content/Dinosaurs` and `Content/World`.

Live tests require launching with `-DinoDevBridge`. This opt-in local bridge accepts test setup and real input events through `Saved/Automation/command.json`, and writes runtime state to `telemetry.json`. It is disabled during a normal launch. Test scripts and recorded results are in `Tools/Tests` and `Tests/Results`.

See TEST_LOG.md for **actually performed** tests and KNOWN_ISSUES.md for current limitations.
