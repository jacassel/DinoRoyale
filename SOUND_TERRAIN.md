# Sound and terrain sprint — September 20, 2026

Scope: redesign sound for Rex, raptor and Triceratops quick attacks, charged
attacks, impacts, hurt reactions, sprinting, ongoing injury and death; increase
tree coverage and elevation variation. Combat values, controls and match rules
remain unchanged. One agent, no purchases.

Before source tag: `sound-terrain-before-20260920` (`7e0a6dc`). The previous playable
and its independent visual-modernization recovery remain intact during testing.
The preexisting staged input configuration is preserved in
`Tests/Results/sound-terrain/DefaultInput.before.ini` and excluded from this commit.

## Reference and audio work

The user's YouTube link resolves to [Jurassic Fight Club T Rex Sound Effects](https://www.youtube.com/watch?v=Bikdo8MCecY), by CretaceousTheHunted.
It supplies cinematic direction; no samples from the video are shipped. Browser
access and reference metadata were verified. Audio input is unavailable to this
agent, so no claim of direct listening or an auditory quality review is made.

81 finite sound assets: three variations per event for each of the three species.
Recorded CC0 creature layers replace the old runtime sine/noise effects. Deep
Rex growls, sharper raptor calls and throaty Triceratops vocals have differentiated
pitch/filtering. Footfalls combine recorded stomps, ground noise and low impacts.
Impacts include a restrained wet layer. Breathing is quieter than attacks.
See `Assets/Audio/CREDITS.md` for author/license/download provenance.

Accepted charge starts and nonlethal applied damage trigger their sounds at the
event. A lethal hit plays death once. Sprint and injury breathing follow the
existing game state; healing, stopping, attacks and canceled charge fade the
appropriate sound. Respawn/species changes stop voices. No endlessly looping
sound, runtime waveform synthesis or per-frame asset loading is used. Short
clips are imported with inline loading; caps are three voices per animal and
24 globally, with lower priority for footsteps and breathing.

All 81 source WAV files passed finite-sample, headroom and zero-boundary checks.
The first compile exposed a TObjectPtr extraction error; corrected with `.Get()`.
An initial import used a relative script path resolved under the engine folder;
the absolute-path import succeeded. These were development iterations, before
any replacement of the playable package.

## Terrain and trees

Broad hills and secondary folds lift the interior maximum from approximately
9.61m to 25.64m on a 2,401-point grid. Maximum added relief is 16.80m. Elevation
fades out around the original spawn, creek and pond; the sampled pond basin
matches the original to floating-point tolerance. 99.92% of sampled interior
terrain is below the navigation slope threshold; steep pond banks remain.

502 trees in total, with independent-seed groves, variable size and clustering.
Trunks participate in existing collision/obstacle navigation. Main cardinal
corridors, landmark clearings, water crossings and minimum obstacle clearance
are reserved. Existing rocks and original trees retain their horizontal positions.
Path shortcuts now check slopes along their length rather than only at endpoints.
Initial directed navigation audit: 42/42 routes found.

Runtime landscape captures, recorded audio, measurements and final gameplay /
package checks are retained under `Tests/Results/sound-terrain`.

## Tested milestones

Source audio: 54/54 checks. Each species has loaded all 81 clips, responds to
accepted events, has faster footfall cadence while sprinting, stops footsteps
airborne/in water, begins charge once, reacts to injury, stops injured breaths
on healing, plays death once and remains silent on respawn. Actual output WAVs
are nonzero and unclipped: Rex peak 0.219, raptor 0.313, Triceratops 0.255.
Initial mixed fights also passed unclipped-output checks (peaks 0.327 / 0.243).
These measurements do not establish perceived realism; preview reels and full
runtime recordings are available in `Tests/Results/sound-terrain/review.html`.

Initial warmed source performance with live AI at 1600x900: 60.00 FPS plains,
60.00 forest, 59.95 pond; highest sampled frame 31.62ms. Later packaged results
are the release measurement. Frames are sampled, not a complete profiler trace.

Increased trees exposed near-camera leaf polygons filling the view. The foliage
material now masks geometry within roughly 140cm of the camera plane. Final
ridge-overlook captures show the obstruction removed. Nearby rocks/other animals
can still push the collision-aware camera close to the player, as before.

Full source gameplay regression passed 462/462 across fifteen suites. All eighteen
continuous player traversals passed in 287.7 seconds combined; each species drove
through six regions. AI swimming passed all three species (39.9 seconds combined).
Allied visibility, map pins and results screens passed their existing checks.
No gameplay test fixture changes were needed for the new terrain.
The Triceratops pond crossing used one normal crowded-bank recovery and still
completed without a failed path; the other two species needed no recovery.

The separate candidate completed BuildCookRun successfully in 141.84 seconds.
Its 54/54 audio checks passed again with actual packaged output peaks of 0.226
(Rex), 0.281 (raptor) and 0.230 (Triceratops). Two packaged group-fight recordings
were unclipped at peaks 0.383 / 0.312, with maximum observed voice counts 10 / 9.
The audio credits are included beside the packaged launcher.

The first packaged gameplay attempt passed 84/90 core checks but failed the six
respawn assertions. Diagnosis: the preceding audio group-fight fixture used
`duelSetup`, which deliberately sets player/AI respawn delays to zero for a dead
spectator. That fixture state persisted into the core test. The package runner
now restarts the test process after group fights to restore the normal world
and respawn defaults. The first attempt is retained in
`package-regression-first-attempt`; no gameplay code or expected assertions were
changed for this test-isolation repair.

The fresh-process package rerun passed **220/220 across six suites**, including
all respawn assertions at the normal ten-second delay. The package also found
all 42 directed navigation-audit routes. Nine packaged landscape captures were
reviewed together, with full-resolution hill-approach and ridge-overlook checks.
Warmed 1600x900 packaged performance: plains 59.98 FPS, forest 60.00 FPS and pond
59.49 FPS; sampled maxima 16.71 / 16.68 / 169.21ms. The isolated pond hitch remains
a limitation; these measurements do not establish hitch-free play.

Four-minute packaged world observation passed: 179 hits in 240.52 seconds, zero
failed paths, stuck recoveries, below-terrain actors or immediate respawn hits.
Up to 20 carcasses were observed. This is a bounded run, not a long-session test.

Source implementation checkpoint: `c2ba3a1`. Reported account allowance after
source regression: 65% remaining; final package checkpoint: 63% remaining. No
separate Astra allowance was exposed.

The verified candidate was promoted to `Dist/Windows`; all 49 runtime files were
SHA-256 checked across candidate, recovery and promoted copies. Independent
recovery: `Dist/Checkpoints/2026-09-20-sound-terrain/Windows`. The previous playable
is retained at `Dist/Checkpoints/2026-09-20-before-sound-terrain/Windows`, alongside
the earlier visual-modernization recovery. Dist is ignored by Git.

`LaunchGame.bat` started the promoted standalone at 1600x900 without test flags
and reached the selection menu. A Windows Firewall permission prompt blocked
the native control check. User dismissal was requested; no permission action or
security-setting change was made by the agent. Native input/quit verification
remains pending until that prompt is dismissed. The packaged mapped-input
regression above passed separately with the opt-in development bridge.
