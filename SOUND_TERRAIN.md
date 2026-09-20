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

Reported account allowance after the audio milestone: 67% remaining; no separate
Astra allowance exposed. The full gameplay and package gates are still pending.
