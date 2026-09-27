# Dino Royale QA2 two-PC quickstart

Use **DinoRoyale-20260926-QA2** on BOTH PCs. Compatibility stays **2026092201**;
that number alone does not distinguish the old broken packages.

1. Extract the whole QA2 ZIP into a NEW folder. Keep QA1 as a backup.
2. Copy your existing configured `OnlineServices.ini` into
   `Windows/DinosaurBattle/OnlineServices.ini` on each PC. Do not edit its values.
   Configured credentials are deliberately absent from the QA2 ZIP;
   `OnlineServices.example.ini` remains available as a reference.
3. Run `Windows/Play Dino Royale.bat` (or `DinosaurBattle.exe`). Press **F4**,
   then sign in with different authorized Epic accounts on the two PCs.
4. PC A: Host Game -> PUBLIC -> Free-for-All -> 2 slots -> bots OFF -> Create Lobby.
5. PC B: Join Game -> Refresh -> select PC A's lobby -> Join Selected.
6. Verify **both names / 2 of 2 players**. Guest chooses a dinosaur and Mark Ready;
   host starts. Both players should spawn and see each other move.
7. Verify attacks, damage, death, approximately ten-second respawn and score on
   both screens. Return to the lobby, leave, and recreate/rejoin it once.
8. Recreate a lobby and test an Epic invite separately: Invite Friends or Shift+F3
   -> Invite to game -> guest accepts. Repeat the roster/Ready/Start check.
9. Once the above works, connect the second PC through a hotspot/different internet
   connection and repeat. Do not change router, firewall or security settings.

If joining fails, run `Collect QA Logs.bat` on BOTH PCs immediately after the
failure. Logs are in `Windows/DinosaurBattle/Saved/Logs`. Keep the timestamped
collections private: existing engine logs may include account identifiers.
The new `[DINO_EOS]` lines report API results, redacted URL shape, actual driver,
listen/socket state, ClientTravel, PreLogin, Login, PostLogin and cleanup.

The QA1 defect was a custom check rejecting UE 5.8's `[EOS:...]` resolved URL.
QA2 keeps EOS P2P relay and version checks; no direct-IP workaround was added.
Real internet gameplay is not verified by local loopback tests. Broader college
playtesting waits for the owner's visual two-PC acceptance and branding/access approval.
