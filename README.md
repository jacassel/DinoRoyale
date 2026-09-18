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
| Space | Jump |
| Hold Q | Defensive brace; movement is disabled |
| LMB | Quick bite, slash or horn attack |
| Hold / release RMB | Charge / execute heavy attack |
| Hold E near suitable food | Eat; release or move to stop |
| 1 / 2 / 3 | Switch to T-Rex / Velociraptor / Triceratops |

Escape/menu and map UI are still in development. Switching species is a pre-alpha testing convenience that restores health.

## Current systems

- Shared character, health, injury, combat and feeding components.
- Configurable species values in `Config/DefaultGame.ini`, including charge, regeneration, camera and movement tuning.
- Original rigged dinosaur meshes and ten animation clips per species.
- Nine major AI dinosaurs plus eighteen smaller prey. Carnivores hunt; raptors share a leader; triceratops defend feeding areas; prey flee.
- Seeded terrain approximately 1.15 km across, with plains, forest, ridge, creek, hunting grounds and feeding groves.
- Clearance-aware grid navigation and local obstacle avoidance.
- Runtime-generated world: the empty saved map is populated by GameMode when Play starts.

## Development

Run `powershell -ExecutionPolicy Bypass -File Tools/Build.ps1` to build the editor module using the local portable Microsoft toolchain. The toolchain is excluded from Git; its setup is recorded in SESSION.md. No registry/OS-wide compiler installation was made.

Original Blender automation is in `Tools/Blender`; native sources are in `Assets/Source`, FBX interchange files in `Assets/Export`, and imported Unreal assets in `Content/Dinosaurs` and `Content/World`.

Live tests require launching with `-DinoDevBridge`. This opt-in local bridge accepts test setup and real input events through `Saved/Automation/command.json`, and writes runtime state to `telemetry.json`. It is disabled during a normal launch. Test scripts and recorded results are in `Tools/Tests` and `Tests/Results`.

See TEST_LOG.md for **actually performed** tests and KNOWN_ISSUES.md for current limitations.
