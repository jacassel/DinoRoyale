# Creature audio sources

The game contains edited, layered CC0 recordings and locally generated foley.
All downloads were verified on the authors' pages on September 20, 2026.

- rubberduck, [80 CC0 creature SFX #2](https://opengameart.org/content/80-cc0-creture-sfx-2): creature vocals, breath, stomp and wet impact layers.
- Darsycho, [Monster snarls](https://opengameart.org/content/monster-snarls): snarls and textured vocal layers.
- trazzz123, [CC0 Deep Monster Roar](https://opengameart.org/content/cc0-deep-monster-roar): low Rex vocal layer.

License: [CC0 1.0](https://creativecommons.org/publicdomain/zero/1.0/).
Direct download URLs are retained in `Source/sources.json`. Sources are preserved
alongside the designed WAV files. Processing includes excerpting, resampling,
filtering, layering, fade envelopes, level normalization and three variations.
Rebuild with `python Tools/Art/build_creature_audio.py`.

Version 0.5 adds 81 separately designed Ankylosaurus, Brachiosaurus and
Pachycephalosaurus clips using the same credited CC0 library and original generated
foley. Rebuild these with `python Tools/Art/build_roster05_audio.py`. Their distinct
layering, envelopes and resonance are authored separately; the original three
sound banks are preserved. Three variants cover each of nine gameplay events.

User reference: [Jurassic Fight Club T Rex Sound Effects, CretaceousTheHunted](https://www.youtube.com/watch?v=Bikdo8MCecY).
The video is a creative reference only; no audio from it is included in the game
or source assets. These are cinematic creature designs, not scientifically
verified reconstructions of dinosaur calls. Direct listening is unavailable in
the agent session; output timing, lifetime, levels and spectra are measured,
and rendered game recordings are supplied for human review.
