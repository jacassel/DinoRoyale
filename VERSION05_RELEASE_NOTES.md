# Dino Royale — Version 0.5

Six playable dinosaurs, expanded food choices and scoring that rewards teamwork.

- **Ankylosaurus:** armored counterattacker with animated tail-club strikes, a shoulder shove and a committed heavy swing.
- **Brachiosaurus:** a slow, large area defender with forefoot stomps, a tail sweep, a two-foot heavy attack and a camera suited to its size.
- **Pachycephalosaurus:** a mobile headbutt fighter with a momentum charge; each pack has one scoring leader and two followers.
- Original Blender models, rigs, textured materials, sixteen animation clips per new species, actual-model portraits and **81 unique new sound clips**.
- **Brachiosaurus eats tree foliage.** Eighteen edible trees remain in both Standard and Performance mode and are highlighted for Brachiosaurus players. Consumed foliage regrows after two minutes; the trunk remains. Ankylosaurus and Pachy share Triceratops's shrubs.
- **P opens live FFA standings.** The current leader appears in black text at the top of the HUD.
- **FFA:** five points wins; each kill adds one and every two assists add one. **Teams:** ten points wins; every three assists pooled across teammates add one. Actual kills, deaths and assists remain separate.
- Existing Raptor packs, ten-second respawns, persistent session carcasses, swimming, hunger, injury, blood settings, map reveal, custom teams and multiplayer are preserved.

| New species | Health | Quick damage | Fully charged damage | Walking speed | Armor: damage received |
|---|---:|---:|---:|---:|---:|
| Ankylosaurus | 1,800 | 145 | 551 | 650 cm/s | 60% |
| Brachiosaurus | 3,100 | 230 | 713 | 580 cm/s | 90% |
| Pachycephalosaurus | 950 | 116 | 429.2 | 1,250 cm/s | 100% |

Damage is before the target's armor/brace. Third quick strikes add 12%; injured or exhausted attacks are weaker/slower. New attack contact follows the animated club, feet, tail or skull. AI aims those attacks and respects combo resets and large-body clearance. The Raptor's existing reduced damage remains unchanged.

Extract the entire Windows ZIP into a fresh folder and run **Windows/Play Dino Royale.bat**. Choose a dinosaur with **1–6** or click its portrait. WASD moves, mouse looks, Shift sprints, Q/E pivots, Ctrl braces, LMB attacks, hold/release RMB charges, hold F eats, M opens the map, and P opens FFA standings. Escape opens the menu; F10 there quits.

All players must update together: compatibility **2026100505**. The owner's actual supplied `OnlineServices.ini` is installed unchanged in the local project and playable package. The public ZIP excludes configured credentials: copy each PC's existing file into **Windows/DinosaurBattle/OnlineServices.ini**. The owner confirms successful multiplayer across different networks on the prior release; this update preserves that implementation and adds local separate-process regression testing.

See **VERSION05_TEST_REPORT.md** for performed tests and **KNOWN_ISSUES.md** for limits. This remains a development prototype: bounded AI testing does not establish competitive human balance, and procedural animation/contact and sound realism still benefit from human feedback.
