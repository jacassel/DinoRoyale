# Dino Royale multiplayer QA report - September 24, 2026

Release: **DinoRoyale-20260924-QA1**. Authoritative compatibility: **2026092201**.
Physical two-PC acceptance is **PENDING**; do not open the broader playtest yet.

## Root cause

The rejection in the photos is the custom pre-join guard in
`Source/DinosaurBattle/DinoOnlineSession.cpp`, before JoinSession or Unreal's
network handshake. It read the advertised `DINO_BUILD` into `int32`.
The host advertised numeric **2026092201**. OSS EOS serializes that number as
Int64; `FOnlineSessionEOS::CopyLobbyAttributes` returns an Int64 variant.
Unreal's `FVariantData::GetValue(int32&)` only accepts an Int32 variant and
returns **0** otherwise. The key lookup can report success even when this typed
read returns zero. Thus the old comparison was **0 != 2026092201**.
The browser's result filter repeated the same faulty read.

The engine automation test reproduces this with the actual Unreal variants and
getter: **host wire=2026092201; old guest read=0; corrected read=2026092201**.
The original guest log/raw lobby payload was not supplied; the original remote
number cannot be recovered from BUILD_INFO alone. The screenshots identify the
failing branch; the installed engine source and executed regression establish
the defect. New diagnostics will capture the actual remote type/value on retest.

Secondary finding: OSS EOS CreateSession overwrites the supplied BuildUniqueId
with GetBuildUniqueId. The previous configuration did not override its engine
changelist-derived value. That is a separate compatibility field, not the source
of this exact error message. The corrected package logs OSSBuildUniqueId
**2026092201**, network checksum **1589990096**, engine-compatible changelist
**55116800**, project version **0.2.0**. No network-check bypass was introduced.

## Changes

- `DinoCompatibility.h`: one typed, range-checked compatibility reader shared by
  discovery and invites; advertise Int64 explicitly. Local Int32 is also accepted.
  Missing, malformed, overflowing and different values remain rejected.
- `DefaultEngine.ini`: supported OSS build-ID override set to 2026092201 so EOS's
  overwrite agrees with the explicit game protocol. Unreal's engine network
  checksum and server PreLogin version gate remain active.
- `DinoOnlineSession.cpp`: sanitized compatibility, operation/callback, invite,
  connect-string scheme and net-driver diagnostics; no credentials in new logs.
- Invite Friends calls **ShowFriendsUI**, which opens EOS's existing social panel.
  The old **ShowInviteUI** function is an unimplemented stub in this engine; the
  preserved host logs contain that explicit warning. Feedback now appears in lobby.
- Menus, project display name, current documentation and release launcher use
  **Dino Royale**. Internal Unreal/EOS artifact names and paths stay DinosaurBattle.
  No Epic organization/product rename, portal setting change or purchase was made.
- Added opt-in development-bridge online actions/telemetry and reproducible tests.

## Tests

- Editor build and full Windows BuildCookRun: PASS.
- Actual Unreal compatibility regression: PASS in editor AND packaged executable;
  checks same Int32/Int64, older/newer versions, absent/invalid metadata, overflow,
  advertised flags and the configured OSS override (17 assertions per run).
- Fresh packaged lobby regression: **14/14 PASS**. Older/missing travel IDs rejected;
  same-build two-player roster, selection, ready/start, movement, raptor followers,
  return to lobby and departure cleanup pass over explicit loopback test transport.
- Offline controls/combat regression: **90/90 PASS** on the fresh rendered package, using the existing isolated AI fixture.
- Normal LaunchGame.bat at 1600x900, without the development bridge: PASS. Native dinosaur selection, M map, Escape, F4 multiplayer and F10 quit passed; both launch processes exited. Branded views were saved.
- Live replacement-package EOS sign-in/hosting/search/overlay: awaiting manual
  Epic sign-in. The overlay sign-in screen rendered. Earlier physical screenshots
  establish these services for the previous package, not the new package.
- New-package two-PC invitation join, public-browser join, relay gameplay and
  different-network gameplay: NOT VERIFIED. Local processes are not that evidence.

Evidence: `Tests/Results/eos-qa-20260924`. Build/raw logs remain local and ignored
by Git under `Build/Logs` and the package's Saved/Logs. Initial compile failures
(test include path and a UE 5.8 automation flag rename) were corrected before the
successful build. The first offline invocation omitted AI isolation (84/90); the next was accidentally interrupted during process cleanup. Both are preserved; the final isolated run passes 90/90 with no gameplay changes.

## Fresh package and exact physical retest

Send this entire folder, and use it on BOTH PCs:
`C:\Users\joel1\Documents\DinosaurBattle Prototype\Dist\Releases\DinoRoyale-20260924-QA1\Windows`

1. Extract/copy into NEW folders. Check BUILD_INFO release QA1 and compatibility
   2026092201 on both. The old faulty build had the same compatibility number.
2. Run `Play Dino Royale.bat`; F4 Multiplayer; sign in with the two authorized
   Epic accounts. Keep `DinosaurBattle/OnlineServices.ini` in place.
3. Host FFA, 2 slots, bots OFF, Public. Create Lobby. Click Invite Friends or
   Shift+F3, select the second account and Invite to game. Guest accepts.
4. Visually verify **2 of 2 players on both PCs**, guest dinosaur selection,
   Mark Ready, then host Start Match. Move/fight on both PCs.
5. Leave and create another public lobby. Guest uses Join Game -> Refresh ->
   select host -> Join Selected. Repeat step 4. Record both paths separately.
6. If successful, repeat on separate internet connections. Broader college
   testing waits for the owner's visual acceptance and Epic branding/access approval.

If either path fails, preserve BOTH PCs' `Windows/DinosaurBattle/Saved/Logs`
folders (`DinosaurBattle.log` plus backups). `Collect QA Logs.bat` creates a
private timestamped copy beside the launcher, without copying OnlineServices.ini
or login caches. New log lines show local/remote ID, type/source, rejection
reason, callback progress and transport selection; engine logs may contain IDs.

## Remaining issues and recovery

The previous public Refresh **timeout is not explained by the typed read alone**:
the bad reader would filter returned results, not prevent a completion callback.
No guest timeout log was supplied. Operation/search-state diagnostics now separate
provider timeout, callback failure and compatibility filtering. A timed-out
operation still requires restart; no speculative backend/session redesign was made.

There is no host migration; host departure ends the match. Seven browser rows are
visible at once. Existing latency/host-advantage and long-session limits remain
in KNOWN_ISSUES.md. Live EOS verification remains gated by manual sign-in and the
physical retest. The desktop window/executable and Epic overlay product label may
still say DinosaurBattle; the Epic product name was deliberately not changed.

The previous playable `Dist/Windows` is untouched. Independent pre-sprint backup:
`Dist/Checkpoints/2026-09-24-before-eos-qa/Windows` (all 71 files SHA-256 verified).
The pre-sprint source tag is `eos-qa-before-20260924`.
Replacement recovery: `Dist/Checkpoints/2026-09-24-eos-qa-fixed/Windows`. Release and recovery are checked against PACKAGE_SHA256.json; the sibling DinoRoyale-20260924-QA1.zip contains the whole Windows folder. Source tag: `eos-qa-ready-for-two-pc-20260924`.
