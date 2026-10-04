# Dino Royale 0.3 multiplayer verification — October 4, 2026

The owner reports successful real multiplayer with the prior release. This update
retains the existing EOS session, relay and service identity. Compatibility moves
to **2026100303** because both peers need the new movement and lobby behavior.

The release gameplay executable passes **577/577** packaged multiplayer checks:
lobby 20, combat 39, bots 17, match rules 48, ecology 25, packs 21, Performance-map
matches 48, custom teams/late join 25, water 96, pivot 82, grounding/carcasses 66,
lobby/maps 13, survival edges 16, cosmetics 3, four-player sessions 13,
delayed fundamentals 24 and delayed packs 21. Compatibility and EOS resolved-URL
automation pass all 32 assertions. Source and packaged offline regression each
pass 478 checks across 14 suites.

Grounding, pivot and water use 75ms per-peer delay; fundamentals and packs also
use 2% packet loss. Grounding coverage includes all species, both observers,
movement/combat/water, flat ground, slopes, rocks, trees, pond edges, water,
host/client/AI deaths and independent respawn. Rendered two-process checks pass
7/7 and show both maps, a two-human-versus-five-bot roster, remote pivots and
replicated corpse food. Automated screenshots were visually inspected.

Evidence is under `Tests/Results/alpha03/rc3-network`, `rc3-regression`,
`rc3-automation` and `rc3-rendered`. The final RC4 package changes only the allowlist
for saving mouse sensitivity; its executable has the same SHA256 as RC3.
Fresh-process preferences pass 5/5; additional final-package UI/settings/core
checks pass 124/124. The requested native close-combat pass remains pending
owner handling of Windows' firewall prompt. See TEST_LOG.md for exact boundaries.

These tests use separate local game processes over explicit development sockets.
They verify local multiplayer behavior, not a new WAN/EOS connection. No firewall,
Epic portal or authentication settings have been changed by the agent.

The following report records the earlier EOS repair and its evidence; its release
number and historical stop instructions do not describe the active 0.3 update.

---

# Historical EOS join QA report - September 26, 2026

Release: **DinoRoyale-20260926-QA2**. Compatibility: **2026092201**.
**Live testing stopped at the owner's request. Physical EOS gameplay acceptance remains pending.**

## What was fixed across QA1 and QA2

The original false version rejection was a typed-metadata bug: EOS returned the
custom `DINO_BUILD` integer as Int64, while the game read Int32 and compared zero
with 2026092201. QA1 added a shared typed reader, aligned Unreal OSS BuildUniqueId
with 2026092201, and wired Invite Friends to the implemented ShowFriendsUI.
Missing or genuinely different versions remain rejected; Unreal's network
checksum checks remain enabled. The owner's subsequent physical QA1 test reached
discovery and successful JoinSession, exposing the separate travel-URL defect below.

## Root cause and first failing stage

QA1's `UDinoOnlineSession::OnJoin` used
`URL.StartsWith("EOS:")` on the raw resolved travel string. Installed Unreal
5.8.2 `FOnlineSessionEOS::GetConnectStringFromSessionInfo` returns `[EOS:PUID]`,
including brackets. `FURL` removes those brackets when extracting `Host`;
`UNetDriverEOS::InitConnect` tests that parsed host. The game incorrectly rejected
a valid resolved URL before calling ClientTravel. No failing EOS API precedes
this rejection in the supplied/local reproduction evidence.

The preserved QA1 log `DinosaurBattle.log` (September 27 UTC / September 26 local):
- 01:43:32: CreateSession callback success=1, address resolution=true.
- 01:43:33: network world hosting=1, actual class NetDriverEOS.
- 01:44:16: FindSessions success=1, one compatible result.
- 01:44:19: JoinSession result=0 (Success), GetResolvedConnectString=true,
  custom prefix check=false. Repeated twice with the same result.
- No guest ClientTravel/PostLogin followed those rejected attempts.

See `Tests/Results/eos-qa-20260926/qa1-diagnostics-redacted.txt` for safe excerpts.
Screenshots additionally show the real remote lobby, both accounts and the exact
error. The log does not print the full original URL; its bracketed representation
is established by the installed resolver source, executed engine regression and
the subsequent live QA2 resolution diagnostics.

Installed engine sources:
- `OnlineSubsystemEOS/Private/OnlineSessionEOS.cpp`: resolver at lines 3253-3310;
  lobby host address created from owner ProductUserId, and copied on successful join.
- `SocketSubsystemEOS/Private/InternetAddrEOS.cpp`: current socket shape is
  `EOS:PUID`, without the obsolete socket/channel suffix.
- `SocketSubsystemEOS/Private/NetDriverEOS.cpp`: InitConnect uses FURL.Host;
  InitListen selects EOS unless unavailable or explicit development IP/LAN options.
- EOS SDK `eos_common.h` explicitly says FromString/IsValid do not authenticate
  arbitrary ID strings. URL validation therefore verifies transport shape only;
  the source remains the successfully joined named EOS session.

## Repair and diagnostics

- `DinoEOSAddress.h` uses Unreal FURL and FInternetAddrEOS to check a nonempty EOS
  host; IP fallback, missing endpoint and obsolete address forms remain rejected.
- OnJoin still waits for Success, resolves **GameSession / GamePort**, then passes
  the original resolved URL to ClientTravel, appending the existing DinoBuild option.
  No peer address is manually built, reformatted, logged or hard-coded.
- `[DINO_EOS]` diagnostics cover provider/identity validity, session info/settings,
  creation/search/join callbacks, named-session state, URL resolution/shape,
  actual NetDriver, passthrough state, bound EOS socket, travel and network failures,
  PreLogin, Login, PostLogin, PlayerState and departure.
- Existing lobby/presence/private/capacity settings, CreateSession -> OpenLevel
  `?listen` flow and relay policy remain. OSS StartSession is not used by this
  lobby architecture. Logs distinguish advertisement completion from listen readiness.
- No engine source patch, architecture change, portal modification, IP workaround,
  gameplay change, version bypass, credential change or purchase.

## Verification

Editor compatibility and resolved-URL regression: **2 suites PASS** (17 + 15
assertions). The new test reproduces QA1's false rejection with real FURL and EOS
address classes and verifies the preserved compatibility travel option. It does
not contact another account or establish an EOS connection.

The editor and Windows Development package rebuilt successfully. The final
package passed both automation suites again: **32 assertions, 2 suites, 0 failures**
(`automation-final-pass/index.json`).

Packaged regression results before the final diagnostic-only adjustment:

| Test | Result | Transport / scope |
|---|---:|---|
| Lobby | 14/14 | Development loopback |
| Movement, combat, damage, respawn, score fundamentals | 24/24 | Development loopback |
| Bots | 17/17 | Development loopback |
| Matches and teams | 48/48 | Development loopback |
| Ecology and packs | 25/25 | Development loopback |
| Offline core | 90/90 | Rendered packaged offline |

The multiplayer total is **128/128**, separate from the 90 offline checks. These
do not establish EOS/WAN gameplay. Evidence is under `Tests/Results/eos-qa-20260926`.

After the owner manually completed both Epic sign-ins, real EOS logs established:

- Host CreateSession succeeded and resolved `[EOS:<redacted>]`; the old prefix
  check would still reject it. NetDriverEOS entered listen mode, passthrough=0.
- Guest discovered the lobby, joined successfully, and requested ClientTravel
  at 02:17:21 UTC on September 27 (September 26 local).
- Host accepted matching compatibility IDs in PreLogin at 02:17:23, then logged
  a remote PostLogin at 02:17:24: valid controller/PlayerState, slot=1, players=2.
- Guest entered NetMode=3 with NetDriverEOS at 02:17:24. Host/guest telemetry
  showed the same two-player match, bots OFF; two gameplay windows were visible.
- This was **two Epic accounts on one PC through EOS**, with no IP fallback.
  It is not a completed physical two-PC or different-network test.

The attached live movement/combat/rehost script started after the game instances
had left the session. Its retained `live-eos-results.json` has a failed initial
precondition; it does not overturn the earlier successful connection logs and
does not prove EOS gameplay. Both instances were stopped when wrap-up was requested.
Do not count live EOS attacks, damage, respawn, scoring, invite joining or rehosting
as passed. Host departure returned the guest to the menu; no host migration exists.

The first live diagnostics incorrectly printed `boundEOSAddressValid=0` because
FSocketEOS::GetAddress copied through the FInternetAddr base type. The final
build reads the driver's actual LocalAddr and labels it `localEOSAddressValid`.
This final change only corrects logging; it does not alter transport or gameplay.
The full 128+90 gameplay set was not rerun for that diagnostic-only change.
The final normal offline launch reached engine startup without a development
bridge; Computer Use was stopped with Escape, so no final visual/input pass is
claimed for that launch. The earlier rendered offline checks remain the gameplay evidence.

The original diagnostic run's two failed negative assertions are retained: SDK
handle validity is not string authentication. The test and guard were corrected
using the SDK documentation. Initial compile/API-name corrections are retained in
local build logs. The first fundamentals score check read stale host telemetry;
waiting for score replication fixed that test race, and all 24 checks then passed.
One final automation invocation incorrectly used `-NoEOS`, preventing the SDK DLL
from loading for the EOS address fixture. It crashed in the fixture; repeating
the previously passing command without that flag passed both suites. No sign-in
or live session is needed for the parser test. No failures are hidden as successful
gameplay evidence.

## Fresh build and manual retest

Fresh package:
`C:\Users\joel1\Documents\DinosaurBattle Prototype\Dist\Releases\DinoRoyale-20260926-QA2\Windows`

Sibling ZIP: `DinoRoyale-20260926-QA2.zip`. Copy the WHOLE new package to each PC.
Configured credentials are not included. Copy the existing OnlineServices.ini to
`Windows/DinosaurBattle/OnlineServices.ini` on both PCs; do not change its values.
On this PC the unchanged configured original remains in
`Dist/Releases/DinoRoyale-20260924-QA1/Windows/DinosaurBattle/OnlineServices.ini`.
The temporary QA2 test copy was moved to ignored `Dist/TestRuns/QA2-local-test-config`.
Each PC needs only its existing configuration file, not the other player's login
cache. Do not copy Saved, logs, auth caches, source, Unreal Engine, or a loose EXE.

Follow FRIEND_QUICKSTART.md: separate Epic accounts; PUBLIC FFA / 2 slots / bots
OFF; guest Refresh -> select -> Join. Verify 2/2, Ready, Start, mutual movement,
attacks, damage, death, ten-second respawn and score. Test re-host/rejoin and Epic
invite joining separately, then a different network/hotspot.

On failure use Collect QA Logs.bat on BOTH PCs. Raw logs are in
`Windows/DinosaurBattle/Saved/Logs`; share privately. No configured ini or auth
cache is included by the log collector. A `[DINO_EOS] ClientTravel requested`
line alone is NOT successful network travel; guest network-world entry plus
server PostLogin/roster/gameplay are the next required evidence.

QA1, its ZIP and independent recovery stay untouched. Pre-change source tag:
`eos-qa2-before-20260926`. Broader playtesting remains gated by physical acceptance
and branding/access approval. Different-network EOS relay is **NOT VERIFIED**.

Final source checkpoint tag: `eos-qa2-ready-for-two-pc-20260926`.
Independent playable recovery: `Dist/Checkpoints/DinoRoyale-20260926-QA2/Windows`.
`checkpoint.json` records the ZIP checksum and verification; `release-manifest.json`
lists individual files. `package-integrity.json` verifies runtime/cooked files
against final staging, the compiled EXE, preserved QA1 and the unchanged user input
file. Dist is ignored by Git, so keep the ZIP/recovery as well as the source tag.
The preexisting staged `Config/DefaultInput.ini` is excluded from the agent commit.

For the staged Epic college test and optional Steam launch, read
`RELEASE_READINESS.md`. No purchases, account changes, portal edits or organization/
product renames were made. Live testing is stopped, not declared fully accepted.
