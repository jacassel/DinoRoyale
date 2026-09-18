# Additional user requests — 2026-09-18

1. Optional blood when attacking major dinosaurs or prey, controlled by a settings menu toggle.
2. Opening/loading character-selection screen to choose the dinosaur before play.
3. Water slows the character's movement.
4. Preserve and retest injury restrictions below 50% and 25% health, including interactions with water.
5. Improve dinosaur graphics as much as practical; the user removed the time limit.
6. Provide clear, verified instructions to open the game.

These extend the original specification; they do not replace its controls, AI, ecology, world, documentation or testing requirements.

## Match and scoring requests

7. Bottom-left kills / deaths / assists counter.
8. Optional 5 versus 5 dinosaur team-fight mode (one human + nine AI total).
9. Solo free-for-all goal: 5 major dinosaur kills.
10. Team-fight goal: 10 team kills.
11. Major dinosaur respawn delay: 10 seconds.

Confirmed: raptors stay allied as a pack in both solo and team modes. Only killing the pack leader awards a kill toward the match objective. Pack followers still fight, die and respawn. Pending follow-up: whether follower finishing blows credit their pack leader (recommended interpretation: the pack is one competitor).

## Water and animation feedback

12. Fix the T-Rex lower jaw clipping into its head during charged attacks (user observed in game).
13. Add a clearly visible swimming animation.
14. Add more water, such as a pond or second river. Implementation choice: Mirror Pond with shallow shores and a deep swimming area.

15. Improve AI combat quality: retaliate when attacked, pursue likely kills, use defensive brace, estimate favorable fights and choose escape when it improves survival. Validate decisions with behavior tests and actual matches.
