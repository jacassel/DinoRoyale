# Internet multiplayer implementation

Status: Goal 2 is in progress. Internet multiplayer is NOT VERIFIED.
The full acceptance requirements are in `MULTIPLAYER_REQUIREMENTS.txt`.

## Baseline, 2026-09-21

Source baseline: `ea40961`, branch `codex/internet-multiplayer`.
Existing staged `Config/DefaultInput.ini` is preserved, with a byte copy in
`Tests/Results/multiplayer-baseline/DefaultInput.before.ini`.
The current playable `Dist/Windows` is retained. An independent complete copy is
`Dist/Checkpoints/2026-09-21-before-multiplayer/Windows`; all 53 files were
SHA-256 compared (including local saved files), see the baseline manifest.
No replacement package is promoted until launch/gameplay checks pass.

Account allowance at start: 62% remaining; no separate Astra window exposed.
One agent, no purchases, no router/security changes, no Sites networking.
The user has no EOS product or unique Steam App ID and no second-network PC
currently available. WAN acceptance must remain NOT VERIFIED until a real test.

## System inventory and conversion boundaries

| Existing system | Source | Networking responsibility |
|---|---|---|
| Runtime world, AI/food spawning | DinoGameMode, LostValleyWorld | Deterministic static world on every peer; living actors/food spawned only on server |
| Match mode, scores, victory, teams | DinoGameMode | Server rules; replicated GameState view for all clients |
| Player identity/lobby | No custom GameState/PlayerState/GameInstance exists | Add separate replicated player/match state and provider-independent online session subsystem |
| Input, selection, menu, settings, test bridge | DinoPlayerController | Local UI; validated intent RPCs; no client mutation of global rules; offline-only pause |
| Character, species, injury, death, respawn | DinosaurCharacter | Server authority, replicated life/species/identity; standard CharacterMovement |
| Combat, combo, charge, brace, knockback | CombatComponent | Server validates timing/resources and detects hits; replicate action state/cosmetic events |
| Stamina, hunger, health/regeneration | StaminaComponent, HungerComponent, HealthComponent | Server simulation; replicated resources |
| Eating, plants, regrowth, carcasses | FoodSystem | Server consumption/spawns/destruction; replicated quantities and presentation |
| AI personalities/navigation | DinosaurAIController, DinoTactics, LostValleyWorld | Server-only controllers and decisions |
| Raptor packs | DinoGameMode.GetPackLeader, DinosaurAIController | Existing species-wide FFA alliance must become explicit per-leader pack identity for online matches |
| Map visibility/labels | DinosaurCharacter, DinoHUD | Preserve sight/noise rules; authoritative per-viewer information |
| Animation, blood, audio | DinoAnimationComponent, DinoEffects, DinoAudioComponent | Replicated events/state; local blood and camera preferences |
| Respawn | DinosaurCharacter.Tick/ResetLife | Server-only ten-second lifecycle |
| Test automation | DinoPlayerController development bridge, Tools/Tests | Opt-in local files; isolate each process; never provide remote arbitrary test RPCs |

## Staged order

A. Network awareness and offline regression.
B. Listen host plus one client: spawn, movement, combat, damage, death, respawn.
C–G. Lobby/settings, FFA, teams, bot replacement, explicit raptor packs.
H–I. Four and ten connected instances within hardware limits.
J. EOS discovery/P2P, network emulation and genuine different-network test.
K. Packaged verification and promotion with a recovery copy.

EOS is the intended first internet provider through Unreal Online Subsystem.
Steam remains a future provider; no test App ID is production infrastructure.
Gameplay replication must not depend on EOS identities or SDK calls.
Local loopback tests are development evidence only, never WAN evidence.

Primary implementation references:
- https://dev.epicgames.com/documentation/unreal-engine/online-subsystem-eos-plugin-in-unreal-engine
- https://dev.epicgames.com/documentation/unreal-engine/online-subsystem-in-unreal-engine
- Installed UE 5.8 headers and plugin source are authoritative for compile-time APIs.

## Stage A checkpoint

Editor/game module builds successfully with UE 5.8.2. Rendered offline gameplay
regression: 183/183 assertions (core 90, animation/navigation 41, settings/water/
blood 28, match rules 24). Evidence: Tests/Results/multiplayer/stage-a-offline.
These are offline results only; online participant/session features are not yet verified.
Account milestone: 60% remaining. The original standalone package remains active.


## Stage B checkpoint

Two separate UE game processes (listen host plus client), loopback only:
23/23 checks passed in Tests/Results/multiplayer/stage-b-04/results.json.
Verified unique slots, both processes' terrain, client movement/sprint/jump/brace,
client quick/heavy damage on server, health replication, no repeated quick damage,
client death/carcass, score increment once, ten-second respawn with matching
position/health, disconnect cleanup and no unresolved movement-base warnings.
No EOS, WAN, lobby, bots, raptor packs, or packaged multiplayer verification yet.
Initial trials exposed missing procedural spawn positions, PostLogin order, and
unresolvable terrain base references; all were repaired and retested. Trial 03
also had a test teardown error (sending key-up after quitting), now corrected.
Stage B allows only two players until lobby/capacity work is implemented.
Account milestone: 58% remaining; no separate Astra allowance exposed.

## Stage C checkpoint

EOS session adapter and relay net driver compile against installed UE 5.8.2.
Sign-in, public/invite-only hosting, compatible-build discovery, joins, invites,
timeouts and departure paths are implemented but live EOS remains NOT VERIFIED:
the project has no EOS product credentials. Offline launch does not require EOS.
The loose OnlineServices.ini file is ignored by Git.

20/20 synchronized lobby checks passed with separate host/client game processes:
Tests/Results/multiplayer/stage-c-02/results.json. Tests cover human capacity,
host identity, server-frozen lobby, own species/team selection, automatic teams,
rejection of guest host actions, ready gating, shared start and closed menus,
post-start movement, host return to lobby, state reset and disconnect cleanup.
Trial 01 required the test to await PlayerState replication separately from
GameState; the server team assignment was already correct.
Capacity now accepts 2–10. Bot setting is synchronized; filling and packs are
subsequent stages. No new playable package has been promoted.
Account milestone: 56% remaining; no separate Astra allowance exposed.

