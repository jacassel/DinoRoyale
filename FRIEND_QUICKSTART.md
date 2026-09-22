# Dinosaur Battle multiplayer playtest

Copy the entire **Windows** folder. Keep all its subfolders together and run
**DinosaurBattle.exe** in that folder. Unreal Editor is not needed.

The host must first configure the EOS product using **EOS_SETUP.md**. Both copies
need the same build and a configured **DinosaurBattle/OnlineServices.ini**, and
each player needs a separate Epic account authorized for that product.
The integration is built; live Epic sign-in and internet play still need testing.

1. Open **F4 Multiplayer** from dinosaur selection and **Sign in to Epic**.
2. Host: **Host Game**, choose FFA or Team Battle, 2–10 slots, bots and visibility,
   then **Create Lobby**. Public makes the match discoverable in the browser.
3. Guest: **Join Game → Refresh**, select the host, then **Join Selected**.
   For invite-only matches, use the host's **Invite Friends** and Epic overlay.
4. Choose dinosaurs and teams. Guests mark **Ready**; the host chooses **Start Match**.

WASD moves; mouse looks; Shift sprints; Space jumps; hold Q braces; left click
quick-attacks; hold/release right click charges; hold E near suitable food eats.
M opens the map. ESC opens the menu without pausing an online match.
First to five kills wins FFA; first team to ten wins Team Battle. Respawn is ten
seconds. A raptor gets two followers without using extra player slots.

The host can return everyone to the lobby or rematch. Leaving the host ends the
match. Blood and mouse settings affect only your own PC.

For the internet test, put the second PC on a different connection, such as a
phone hotspot. Use the browser/invite flow, with no IP addresses or router port
forwarding. Test fighting, death/respawn, scoring, followers, rematching and
rejoining. Record failures and preserve **DinosaurBattle/Saved/Logs**.

Offline play is immediately available: select a dinosaur with 1, 2 or 3.
