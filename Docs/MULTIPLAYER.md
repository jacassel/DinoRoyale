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

## Stages D/E/F checkpoint

FFA five-kill victory and team ten-kill victory, client kills/deaths/assists,
real host/client friendly-fire exclusion, a 2v1 team match, result movement
freeze and three repeated lobby/rematch cycles passed in stage-de-01.
Scoring thresholds use server-only damage fixtures; earlier Stage B covers real
input-driven enemy damage/death/respawn.

17/17 bot tests passed in stage-f-02: six slots with humans and bots, a third
joining human removes a bot/controller without kills/carcasses, active bot AI,
disconnect filling, bots OFF cleanup, two humans plus eight bots, balanced 5v5,
human team changes, capacity reduction and vacant slots when bots are disabled.
Stage-f-01 exposed a lobby pawn team value lagging its PlayerState selection;
reconciliation now updates both and the full suite passed again.
Three actual networked game processes are tested so far. Ten participant slots
with eight bots is not ten networked-player evidence. EOS/WAN remain unverified.

## Stages G/H checkpoint

21/21 pack checks passed in stage-g-01. Two human raptors with bots OFF each own
two followers. Distinct pack IDs, replicated movement, FFA allegiance,
leader-only scoring, follower kill credit, ten-second leader respawn, own-pack
cleanup, team allegiance, species changes, and replacing a raptor filler bot
were verified. Followers use separate actor IDs and consume no participant slots.

13/13 four-player checks passed in stage-h-02: four real processes control unique
participants, all peers observe movement, two concurrent packs, four-human FFA,
2v2, shared score/assist, full-capacity and incompatible-version rejection, and
host exit returns every guest to Multiplayer with "Host disconnected."
Stage-h-01 exposed two UE network failure callbacks overwriting the departure
reason; the cleanup path now preserves the first actionable failure.

25/25 ecology checks passed in ecology-01. A server-only sight blocker removes a
remote client's map marker; attack noise reveals the last position and expires
on the server clock. Client feeding changes server food/health/hunger/stamina,
movement cancels feeding, plants deplete on both peers, and all three species
swim, brace and jump from water using client input. Map markers are replicated
only to their owning player's controller, after server visibility filtering.

The EOS adapter explicitly disables lobby host migration, supplies matching
version buckets for create/search, and enables presence when joining EOS search
results. These are source/API checks, not live EOS service verification.
Account milestone: 54% remaining, reported account allowance (no Astra window).

39/39 additional combat checks passed in combat-01 using real client input for
all three species: three-hit combos, replicated damage, menu opening cannot
bypass committed attack recovery, charge initiation/lunge/pounce, 50%/25%
injury speed penalties, critical-health and low-stamina charge rejection, and
weak attacks while exhausted. Network lobbies/results freeze ongoing movement.

## Packaged scaling, latency and recovery checks

The Development Windows package passed ten actual networked processes (listen
host plus nine guests), with every main participant a raptor and twenty extra
followers. Both ten-player FFA and 5v5 ran for approximately 120 seconds each.
`scale-01` contains 18 passed checks and sampled resource/network data. No fatal
replication warnings, ghost players or orphan packs remained after departure.
The minimum available RAM was 13.64 GiB. Headless median simulation rate was
about 60 FPS; host 95th-percentile sampled frame time was 21.27 ms in FFA and
20.77 ms in teams. Host outgoing traffic was about 244/241 kB per second.
These are bounded headless loopback measurements, not rendering or WAN results.
The maximum independently verified filler-bot configuration is two humans plus
eight main bots (stage-f-02); followers and prey are additional AI actors.

Each packaged latency profile passed 24 assertions: 15, 38 and 75 ms outgoing
delay on **each** peer; the last also used 2% packet loss. Settings were read back
from both live net drivers. These add nominal 30/76/150 ms round-trip delay;
sampled total client pings were 86.9/145.3/243.6 ms including engine scheduling
and baseline overhead. Exact total 30/75/150 ms pings were not established.
All retained server-authoritative combat, single-count damage/death/scoring,
movement, ten-second respawn and disconnect cleanup. The pack suite additionally
passed 21 checks at 75 ms per peer and 2% loss (`latency-packs-loss2-02`).

`packaged-four` passed 13 checks on the rebuilt package, including full-lobby and
version rejection, four unique controllable players, 2v2 score/assist agreement
and host departure messages. `packaged-cosmetics` passed three checks with
different local blood preferences and actual replicated client hits.
`packaged-rendered` passed six checks with two rendered windows and real mapped
movement/combat. Native inspection saw both peers and the ESC multiplayer menu;
a Windows Firewall prompt occluded part of the windows and was not accepted.
Inspection found stale help/version labels and score headers that did not align
with their values; the final source corrects those and counts only one's own
followers in the online pack HUD.

Trial `latency-packs-loss2` included prey in a main-participant count; the
participant assertion now uses the main slot range. A separate repair updates
the replicated scoring flag even while the lobby is frozen, and the rendered
test verifies prey remain non-scoring there.

Offline trial 01 retained a collision dummy between species, obstructing one
Triceratops movement assertion. The core fixture now removes that dummy before
each species. Trial 02 ran while two additional rendered windows were inspected;
several immediate animation samples failed. Both trials also exposed immediate
score reads racing the 0.1-second GameState update. Score fixtures now wait for
publication, and both older settings/match suites return failure exit codes when
any assertion fails. Failed evidence is retained. Final serialized regression
results are recorded in TEST_LOG.md and HANDOFF.md.

EOS product configuration is still absent and the Epic Developer Portal is
signed out. A user-facing choice of setup instructions or signing in was left
pending while these tests continued. No accounts, credentials, purchases or
firewall permissions were created/changed. Live sign-in, discovery, invites,
relay traversal and different-network gameplay remain **NOT VERIFIED**.

Final serialized packaged regression passed **183/183**. The subsequent change
only corrected the online local-settings caption and hid offline shortcuts;
the final UI rebuild passed another six rendered smoke checks in
`packaged-ui-final`, and its settings/menu captions were inspected natively.
The promoted normal launcher passed native 1/F4/F10 checks. All 54 non-Saved
package files were hash-verified across candidate, recovery and promotion.
Source/editor and final package builds succeeded. Final allowance: 47% remaining.

## Silent timeout repair and survival edge checks

A subsequent acceptance audit tested lost traffic by suspending only the test's
own guest process, then resuming it. The first trial revealed that the global
network-failure callback sent the listen host back to the menu for an individual
guest ConnectionTimeout. The handler now ignores per-guest ConnectionTimeout and
ConnectionLost notifications on a server; Unreal's connection cleanup still
invokes Logout, removes that human and restores any configured bot/private pack.
Client-side and fatal server failures retain the existing menu/cleanup behavior.

`packaged-failure-edges-02` passed 13 checks on the new package. Its healthy third
process remains connected and accepts mapped movement after the guest timeout;
another guest subsequently joins. Suspending the host separately returns both
guests with Host disconnected. Ending-match rejection and unreachable-host
timeout also pass. No firewall or network settings were changed.

`packaged-survival-edges-04` passed 16 checks on the same package: server starvation,
resource restoration on respawn, finite food consumption, two-way real heavy
knockback and charge interruption, and plant regrowth after 120.203 seconds.
The regrowth test does not shorten the timer or replace the map. Earlier survival
trials are retained: trial 01 assumed a non-existent hunger field in the actor
snapshot; trial 02 hit a telemetry read timeout during concurrent packaging;
trial 03 hit a Windows command-file PermissionError after 13 passing checks.
The harness retries the same command sequence/payload for up to five seconds on
that transient sharing error. Final trial 04 ran alone and passed all checks.

Editor and packaged builds succeeded. Promotion preserved the old package,
created a separate recovery copy, and verified all 54 non-Saved files by SHA-256.
The normal launcher opened the new package at 1600x900; the unchanged firewall
prompt limits native verification. See HANDOFF.md for current paths/tag and
TEST_LOG.md for evidence. Final allowance: 44%. EOS product configuration remains
absent, so live EOS and different-network play remain NOT VERIFIED.

