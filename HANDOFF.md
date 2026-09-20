# Playable checkpoint — September 19, 2026

Double-click `C:\Users\joel1\Documents\DinosaurBattle Prototype\LaunchGame.bat`.
It opens the standalone game in `Dist\Windows`, verified in a normal window.
Press 1 / 2 / 3 to choose a dinosaur; WASD moves, mouse looks, Shift sprints,
Space jumps, Q braces, LMB attacks, hold/release RMB charges, E eats.
Escape pauses; F2 opens settings, B toggles blood; F3 changes match mode in
selection; F10 quits from the menu.

The inherited terrain material repair and cooked sky/ambient dependency are
present in the successfully packaged build. No gameplay balance changed this
session: raptor quick damage remains 74 and full pounce damage 214.6.

Fresh packaged verification passed: core 90/90; combat 69/69; combat follow-up
17/17; integration 41/41; settings/blood/wading 28/28; match rules 24/24;
swimming 40/40; ecology/map visibility 42/42; ecology edges 5/5; AI tactics
11/11; twelve personality encounters and three pack fights. See
`Tests/Results/packaged-milestone`. These are live Unreal simulation/input tests.
Some AI encounters include disengagement and hunting, not sustained duels.

One swimming assertion initially failed because its position baseline preceded
input processing. Three diagnostic replays measured zero drift after brace
became active. The test now waits for brace activation without relaxing the
movement tolerance; all 40 swimming checks passed on rerun. Original failure
and diagnostic evidence are retained. Respawn tests wait for actual simulation
completion. Visual capture now locates screenshots in the selected game's Saved
directory. The sequential packaged runner stops on failure and supports resume.

Recovery copy: `Dist\Checkpoints\2026-09-19-stable\Windows\DinosaurBattle.exe`.
Keep its entire Windows folder together. All 47 copied runtime files were
SHA-256 compared with the playable package; this copy also launched and accepted
species selection. The manifest is `Tests/Results/checkpoint-package-manifest.json`.
Dist is local and ignored by Git; no cloud backup was created.

Final multi-angle jaw/blood captures, precisely timed plant regrowth, AI pond
crossings, and complete-match verification are still running at this checkpoint.
Do not infer those final results from the earlier regression passes.

Known limitations: prototype visuals and animation transitions/foot contact,
close-body overlap, no audio, session-only carcasses, and no extended ecology
soak guarantee. Combat balance still needs human feedback. See KNOWN_ISSUES.md.

Usage at stabilization entry: 16% five-hour / 40% weekly remaining. AGENTS.md
records the user's thresholds, single-agent rule, and prohibition on purchases.


## Active quality sprint (source candidate, not promoted)

The original launcher/package above remains protected. Current source changes halve map dimensions, rebalance the three-member pack, and add generated audio. Requested-rules and recorded-audio suites both pass 36/36; full regression is running through `Tools/Tests/run_quality_regression.py`, with fresh results in `Tests/Results/quality-sprint`. Balance calibration details are in TEST_LOG.md. Finish the requested held-out balance, traversal, world, regression, visual and packaged checks before promoting a new package or declaring completion.
