# Epic Online Services setup

The game uses EOS lobbies and EOS P2P relay with an Unreal listen server. The host
also plays. Gameplay replication is independent of the online provider.

**Live EOS authentication, lobby discovery and internet connectivity are not yet
verified. This project did not have an EOS developer product when integration
started. Local multi-process tests do not prove EOS or different-network play.**

## Configure your product once

1. Sign in to the [Epic Developer Portal](https://dev.epicgames.com/portal/).
   Create/select your organization and an EOS product for Dinosaur Battle. Use
   the free EOS offering; no Epic Games Store distribution or paid hosting is
   required for this implementation.
2. In Product Settings, record the Product ID, Sandbox ID and Deployment ID for
   the deployment you want both PCs to use.
3. Create a client and an appropriate **Peer2Peer** client policy for authenticated
   players, including lobby and matchmaking operations. The host is a player
   client. Do not use a trusted server/admin policy for a distributed game.
4. Configure an Epic Account Services application for this product and link the
   client. Enable the permissions requested by Unreal: Basic Profile, Friends
   List and Presence. This game uses Account Portal sign-in plus EOS Connect.
   It does not require launching through the Epic Games Launcher.
5. Give both test accounts access to the selected product/sandbox. An unverified
   Epic Account Services application is restricted to the owning organization;
   complete Epic's brand review before offering sign-in to the public. Use
   separate Epic accounts for the two simultaneous players.
6. Copy `OnlineServices.example.ini` to `OnlineServices.ini` and replace all five
   placeholders using the client and deployment values from the portal. Keep
   the section name `[Dino.EOS]`. Do not paste these values into chat or commit
   the configured file. The distributed client must have only the permissions
   necessary for an authenticated player.
7. Put that configured file in the correct place and restart the game:
   - Source/editor game: next to `DinosaurBattle.uproject`.
   - Packaged game: `Windows/DinosaurBattle/OnlineServices.ini` (inside the
     DinosaurBattle subfolder, not next to the top-level launcher).
   Both PCs need the same product, deployment and game build. Editing this loose
   file does not require Unreal, a compiler, or repackaging.

If the portal's policy labels differ, use Epic's current client-policy guide to
grant lobby creation/search/join/update/invites and P2P to authenticated users.
The game does not implement purchases, title/player data storage, achievements,
voice chat or anti-cheat.

## Host and join

Launch `DinosaurBattle.exe`. In the selection menu choose **F4 Multiplayer**,
then **Sign in to Epic**, and complete the browser/overlay sign-in yourself.

Host: choose **Host Game**, mode, capacity 2–10, bots on/off and visibility.
Choose **Create Lobby**. Select your dinosaur and team. Guests mark Ready; the
host chooses Start Match. Smaller matches can start with empty slots.

Guest: choose **Join Game**, Refresh, select the matching host and Join Selected.
Public sessions appear in this browser. For invite-only lobbies, the host uses
Invite Friends and the guest accepts through Epic's social overlay.

ESC opens the multiplayer menu during play. The host can return everyone to the
lobby or start a rematch after results. Leaving the host ends the match for all
guests. There is no host migration. Local blood and mouse settings remain local.

## Your different-network playtest

Use the packaged build on both PCs with two authorized Epic accounts. Put the
second PC on a different router/internet connection (for example a phone
hotspot). Use the in-game browser or an invite. Do not use IP addresses, console
commands or router port forwarding. EOS transport is configured to force relay.

Test host + guest movement, jumping, sprinting, quick and charged attacks,
brace, feeding, death/ten-second respawn, scores, raptor followers, rematch,
leaving and rejoining. Try both FFA and Team Battle, bots on and off.

Record both build numbers, network arrangement, session visibility, actions and
result. Preserve both game logs from `DinosaurBattle/Saved/Logs` for diagnosis;
review logs for account identifiers or tokens before sharing them publicly.
Different-network connectivity must remain NOT VERIFIED until this test passes.

## Troubleshooting

- **EOS needs product configuration:** check the file path, section name and all
  five values; then restart.
- **Sign-in failed/cancelled:** check the EAS linked client, requested scopes,
  sandbox access and whether both accounts may use the application.
- **No compatible sessions:** both PCs must use the same game build/deployment,
  and the host must choose Public for browsing. Refresh after hosting finishes.
- **Different game version:** copy the same complete Windows package to both PCs.
- **Lobby is full:** raise capacity in the lobby or wait for a player to leave.
- **Host disconnected:** the listen server closed or lost connectivity. Return
  to Multiplayer and join a newly hosted session.
- **Online request timed out:** return to offline play or restart before retrying.

## Future Steam integration

`UDinoOnlineSession` owns authentication, session discovery, invites and joins.
It calls Unreal Online Subsystem interfaces. A legitimate Steamworks App ID,
Steam authentication/configuration, Steam lobby settings and SteamSockets net
driver can be integrated there without replacing dinosaur replication. Steam
packaging, account/ownership testing and release requirements remain future work.
No Spacewar App ID is included. This prototype has no host migration, anti-cheat,
dedicated service or production release certification.

## Primary references

- [Unreal EOS Online Subsystem](https://dev.epicgames.com/documentation/unreal-engine/online-subsystem-eos-plugin-in-unreal-engine)
- [EOS client policies](https://dev.epicgames.com/docs/epic-online-services/eos-fundamentals/client-and-client-policy/client-policy-guide)
- [Epic Account Services brand review](https://dev.epicgames.com/docs/epic-online-services/accounts-and-social/eos-epic-account-services/brand-review)
- [Epic account application verification](https://www.epicgames.com/help/en-US/c-Category_EpicAccount/c-EpicAccountServices/does-epic-games-verify-products-or-services-that-use-epic-accounts-a000084929)
- [EOS P2P](https://dev.epicgames.com/docs/epic-online-services/multiplayer/nat-p2p-interface)

Installed UE 5.8.2 plugin source supplies the exact API and current
`SocketSubsystemEOS.NetDriverEOS` class used here. The older `NetDriverEOSBase`
name and `bIsUsingP2PSockets` setting in some examples are deprecated in 5.8.
