# Dino Royale multiplayer playtest - QA1

Copy the **whole Windows folder** from release **DinoRoyale-20260924-QA1** into a
new location on both PCs. Do not merge it into an old installation. Run
**Play Dino Royale.bat** (or **DinosaurBattle.exe**). Unreal Editor is not needed.
BUILD_INFO must show release **QA1** and compatibility **2026092201** on both PCs.
The old faulty package also displayed that compatibility number: use the release
label and manifest to distinguish it. Keep `DinosaurBattle/OnlineServices.ini`.
Each player needs a separate Epic account already authorized for this product.

1. On both PCs, open **F4 Multiplayer** and **Sign in to Epic**.
2. Host: **Host Game -> FFA -> 2 slots -> bots OFF -> Public -> Create Lobby**.
3. Invite path: host clicks **Invite Friends** (or **Shift+F3**), selects the
   other account and **Invite to game**. Guest accepts in the Epic overlay.
4. Visually confirm **2 of 2 players on both PCs**. Guest chooses a dinosaur and
   **Mark Ready**. Host clicks **Start Match**. Confirm both can move and fight.
5. Leave the session, recreate the public lobby, and test the second path:
   guest **Join Game -> Refresh -> select the host -> Join Selected**. Repeat step 4.

Report each path separately. Stop if a join fails and preserve logs on BOTH PCs:
`Windows/DinosaurBattle/Saved/Logs/DinosaurBattle.log` and timestamped backups.
Use **Collect QA Logs.bat** to copy these into a timestamped folder beside the
launcher and open it. Keep logs private; engine logs may include account IDs.
If Refresh times out, restart both games before the next attempt.

WASD moves; mouse looks; Shift sprints; Space jumps; hold Q braces; left click
quick-attacks; hold/release right click charges; hold E near suitable food eats.
M opens the map. ESC opens the menu without pausing an online match.
FFA ends at five kills; teams at ten. Respawn is ten seconds. Raptors have two
followers without consuming player slots. Host exit ends the match.

Offline: select a dinosaur with 1, 2 or 3. F10 in the menu quits.

The corrected package's physical two-PC result is **pending your verification**.
After both join paths pass locally, repeat on different internet connections
(for example one PC on a hotspot). No IP entry or router port forwarding is part
of the game flow. Broader friend testing waits for the owner's visual two-PC
acceptance and Epic branding/access approval.
