# Dino Royale — Version 0.3 Alpha Test quickstart

Use **DinoRoyale-0.3-Alpha-Test** on BOTH PCs. Compatibility is **2026100303**;
0.2 clients cannot join 0.3 matches. Keep the previous working installation.

1. Extract the complete 0.3 ZIP into a NEW folder.
2. Copy your existing configured `OnlineServices.ini` into
   `Windows/DinosaurBattle/OnlineServices.ini` on each PC. Do not edit its values.
   Configured credentials are deliberately absent from the 0.3 ZIP;
   `OnlineServices.example.ini` remains available as a reference.
3. Run `Windows/Play Dino Royale.bat` (or `DinosaurBattle.exe`). Press **F4**,
   then sign in with different authorized Epic accounts on the two PCs.
4. PC A: Host Game -> PUBLIC -> Free-for-All -> 2 slots -> bots OFF -> choose
   Standard or Performance -> Create Lobby.
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

**Controls:** WASD move, mouse look, Q/E pivot, Ctrl brace, Shift sprint,
Space jump, LMB quick combo, hold/release RMB heavy, hold F eat, M map.
Pivot turns the dinosaur independently of the camera; W resumes movement.
Escape -> F2 -> N toggles name tags and saves the preference on this PC.
Escape opens the menu; F10 there quits.

The host can change individual AI rows: enabled/disabled and Team A/B.
For two humans versus five AI, use ten slots, put both humans on Team A,
disable the three allied AI and keep five opposing AI. The remaining slots
stay empty. The 5v5 preset restores the balanced roster.

Performance keeps the same terrain, water, rocks, edible plants and match
rules while removing decorative trees and grass. The host's choice applies to
everyone. Raptors remain packs; only the leader counts toward the kill goal.
FFA ends at five kills and Team Battle at ten. Respawn takes ten seconds;
the old carcass remains food.

During your next two-PC playtest, check mutual pivots, grounded remote dinosaurs,
swimming and carcass placement on both screens. Try both maps and an uneven team
match. The owner reports the prior release worked in real multiplayer; 0.3 local
loopback tests do not verify a fresh internet session. See BUILD_INFO.txt,
MULTIPLAYER_QA_REPORT.md and KNOWN_ISSUES.md for evidence and limitations.
