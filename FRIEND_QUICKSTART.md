# Dino Royale — Version 0.6 quickstart

Use **DinoRoyale-0.6** on BOTH PCs. Compatibility is **2026100706**;
Older clients cannot join 0.6 matches. Keep the previous working installation.

1. Extract the complete 0.6 ZIP into a NEW folder.
2. Copy your existing configured `OnlineServices.ini` into
   `Windows/DinosaurBattle/OnlineServices.ini` on each PC. Do not edit its values.
   Configured credentials are deliberately absent from the 0.6 ZIP;
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

Open F5 / Match Setup in the host setup or lobby. Select 5, 10 (default), or
15 team points, allowed species, and independent additional Team A/B bots.
For two humans versus five bots, put both humans on A and set A bots to 0,
B bots to 5. Capacity stays ten and unequal teams are preserved. Invalid requests
remain visible and block Start. Guests cannot change host rules.

Performance keeps the same terrain, water, rocks, edible plants and match
rules while removing decorative trees and grass. The host's choice applies to
everyone. Raptors and Pachys have one leader and two followers; only the leader awards a scoring kill. Eighteen edible trees remain in Performance mode for Brachiosaurus.
FFA ends at five points (kill + two-assist bonuses); Team Battle at the selected 5/10/15 points (kill + bonuses for every three assists pooled across the team). P opens live FFA standings, and the leader appears at the top of the HUD. Respawn takes ten seconds;
the old carcass remains food.

During your next two-PC playtest, check mutual pivots, grounded remote dinosaurs,
swimming and carcass placement on both screens. Try both maps and an uneven team
match. The owner confirms successful multiplayer across different networks on the prior release; 0.6 local
loopback tests do not verify a fresh internet session. See BUILD_INFO.txt,
MULTIPLAYER_QA_REPORT.md and KNOWN_ISSUES.md for evidence and limitations.
