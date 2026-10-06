# Version 0.5 development release

The owner confirms successful prior multiplayer across multiple networks. Version
0.5 preserves that implementation and expands the playable roster and scoring.
See VERSION05_RELEASE_NOTES.md, VERSION05_TEST_REPORT.md and TEST_LOG.md for the
current release. The historical college/Steam planning below does not override
the authorized Version 0.5 development and GitHub publication. No purchases or
Steam integration are part of this update.

---

# Dino Royale: Epic playtest and optional Steam readiness

**October 4, 2026 update:** official game branding is Dino Royale. Version 0.3
Alpha Test is in release verification, with compatibility **2026100303**. The
owner reports working multiplayer in the prior release and authorized this update
and a GitHub project named DinoRoyale. Use the current FRIEND_QUICKSTART and
TEST_LOG for 0.3. The report below is historical; the September stop and branding
approval notes do not prevent the authorized 0.3 work. No store release or spending
is authorized or performed.

---

Prepared September 26, 2026; official references checked during wrap-up.
This is a report of remaining work, not authorization to publish or spend.
Live QA pursuit stopped at the owner's request. No store submission, purchase,
account-access change, Epic organization rename or Epic product rename was made.

## Current checkpoint

QA2 repairs the second EOS blocker: the game rejected Unreal's valid bracketed
EOS travel URL. QA1 had already repaired the Int64/Int32 compatibility mismatch.
Compatibility remains 2026092201 with version protection enabled. Two distinct
Epic accounts completed a real EOS join and entered the same match on one PC.
This is stronger than sign-in alone, but does not establish physical two-PC play.
The package is Windows Development, not a production-certified release.

Use `FRIEND_QUICKSTART.md` to send the complete QA2 ZIP to your brother. Preserve
the previous installation and copy each PC's existing OnlineServices.ini into
the fresh Windows/DinosaurBattle directory. The ZIP excludes configured EOS
credentials and Saved/authentication caches. The two files for your brother to
read first are FRIEND_QUICKSTART.md and BUILD_INFO.txt, both inside the package.

## Gate 1: your physical two-PC acceptance

Before inviting college friends, visually verify all of the following on two
physical computers running the whole QA2 release:

- Different authorized Epic accounts; Public FFA, two slots, bots OFF.
- Public Refresh/select/Join, both names and 2/2 roster, dinosaur selection,
  guest Ready, host Start, both players present in the same match.
- Mutual movement, attacks and damage; death, about ten-second respawn and score
  observed on both screens. Finish a match and return to the lobby.
- Leave/recreate/rejoin, then separately accept an Epic social invite into a new
  lobby. Verify host departure produces a clear return to the menu.
- Repeat with one computer on another internet connection/hotspot. ForceRelays
  is configured, but different-network gameplay is still unverified.

Record release label, two PCs, networks, join route, time and result in
Docs/MULTIPLAYER_ACCEPTANCE.md. Collect QA Logs.bat on both computers if anything
fails. Keep raw logs private. Do not treat the shared compatibility number as
proof that both installations contain QA2: QA1 uses the same number.

## Gate 2: branding and authorized tester access

The owner's condition remains **branding approved plus visually accepted two-PC
lobby before broader testing**. The current Epic portal review status was not
rechecked during wrap-up; two successful accounts do not establish access for
everyone else. Epic distinguishes verification of the application brand from an
endorsement of the game. [Epic account application verification](https://www.epicgames.com/help/c-202300000001645/c-202300000001752/a202300000014396?lang=en-US).

Finish the application's brand-review requirements using Dino Royale artwork,
clear product/publisher information, a public privacy policy and a support contact.
The policy must accurately explain actual account-data handling. Epic staff's
published guidance calls for a public policy, privacy contact and description of
data handling. Check the current portal's exact requirements and rejection notes
before resubmitting; approval is not established by this report.
[Epic staff privacy-policy guidance](https://forums.unrealengine.com/t/what-is-the-required-information-and-wording-for-our-privacy-policy-with-eos-login/832603),
[Brand review reference](https://dev.epicgames.com/docs/epic-online-services/accounts-and-social/eos-epic-account-services/brand-review).

Then verify the intended testers' access to the exact product, sandbox, deployment
and EAS application. Keep the working IDs and application/client linkage intact.
For restricted sandboxes, investigate Player Groups/tester access in the current
portal instead of adding friends as organization administrators. Epic's published
support guidance discusses player groups for non-organization sandbox testers;
it also discusses access keys when distributing builds through the Epic launcher.
That launcher entitlement flow is separate from this directly shared Windows ZIP.
Do not assume a Player Group automatically satisfies every EAS brand/access gate.
[Epic sandbox testing guidance](https://forums.unrealengine.com/t/how-to-get-access-to-the-dev-sandbox/1270116).

Prove one newly authorized college account can sign in, discover and join before
adding the full group. Review the existing game-client policy for the services
actually used; never include developer/admin credentials in a public package.
The temporary manual configuration-copy procedure is acceptable for these two
existing installations, but a broader release needs a reviewed, reproducible
client configuration so each new tester can install without editing secrets.
This is proposed work; no credentials were changed or disclosed in this sprint.

An EOS-based college test does not itself require publishing an Epic Games Store
listing. Keep the first group on a controlled build and existing EOS integration.
Unreal's EOS documentation explicitly describes configuration when a title is
not launching on the Epic Games Store.
[Unreal EOS configuration](https://dev.epicgames.com/documentation/unreal-engine/online-subsystem-eos-plugin-in-unreal-engine).

## Gate 3: controlled college playtest

Start with 2-4 people on different home networks, then expand toward the supported
ten-player configuration. Existing ten-process loopback tests are not ten-human
internet or rendering-performance evidence. Test FFA, teams, odd team sizes,
bots on/off, full lobbies, late joins, leaving/rejoining and repeated matches.
Run a 30-60 minute session to expose stale sessions, ghost players, accumulating
actors, performance degradation and account/overlay interruptions.

Keep one host/guest log pair for each failure and a short defect record: build,
time, steps, expected/actual behavior, severity and reproduction rate. Prioritize
failure to launch/sign in/join, crashes, lost controls and incorrect damage/score
before adding gameplay. Keep a known-good ZIP and rollback instructions. Give
every tester the same immutable release and manifest; increment compatibility
when a future protocol/content change is genuinely incompatible.

Before wider public distribution, produce and retest a Shipping candidate, audit
development commands/bridges and exclude test tools, auth caches and private logs.
Verify prerequisites on a clean Windows PC without Unreal installed, install and
update behavior, minimum-spec performance, resolution/input settings and clear
failure messages. Preserve private debug symbols separately for crash diagnosis.
Remaining product limitations include no host migration, production anti-cheat,
dedicated hosting or Steam integration, and unfinished art/audio/balance polish.
These are scope decisions, not all mandatory features for a small friends test.

## Optional Steam path

The lowest-change technical path is to distribute Dino Royale through Steam while
retaining EOS for multiplayer and the existing Epic account sign-in. Clearly
disclose the account/internet requirements in store/setup text. Steam distribution
does not require replacing EOS with SteamSockets. A later Steam-native sign-in,
Steam friends/invites or account-linking experience is separate integration work;
evaluate Steam identity plus EOS Connect/EOS Plus with a legitimate App ID, and
regression-test Steam-to-Steam and Steam-to-Epic play. Epic documents EOS Plus as
an optional combination of EOS and a base platform such as Steam, with limited
Beta feature coverage. [Unreal EOS/EOS Plus](https://dev.epicgames.com/documentation/unreal-engine/online-subsystem-eos-plugin-in-unreal-engine).

If you decide to proceed, the remaining work is:

1. **Owner onboarding and budget decision.** Complete Steamworks legal identity,
   banking and tax onboarding. Steam currently lists a **US$100 per-product Steam
   Direct fee**. For the first releases, plan for the stated 30-day period after
   payment and at least two weeks of public Coming Soon visibility. These are
   planning requirements only; nothing was purchased or submitted.
   [Steam Direct](https://partner.steamgames.com/steamdirect).
2. **A Steam installable candidate.** Configure the real App ID, Windows depot,
   launch executable and prerequisites; upload through SteamPipe to a private
   test branch. Test clean install, update, uninstall, launch from Steam, overlay
   interaction, EOS first-time consent/sign-in and two-computer multiplayer.
   Do not ship the whole development repository or rely on Unreal being installed.
   [Steam build uploading documentation](https://partner.steamgames.com/doc/sdk/uploading).
3. **Store materials and accurate disclosures.** Prepare Dino Royale capsules,
   logo, gameplay screenshots/trailer, description, content survey, system
   requirements, support/privacy links and asset-license credits. Audit rights
   and applicable Unreal licensing before commercial release. Promise only
   working, tested features; controller support, Deck support, achievements and
   cloud saves are additional decisions, not features established here.
4. **Valve review and release planning.** Submit the store page and near-final
   build for review. Valve's detailed guidance says reviews typically take 3-5
   business days and asks developers to allow at least 7 business days. Leave
   room to fix review feedback and obey the separate release timing gates.
   [Steam review process](https://partner.steamgames.com/doc/store/review_process).
5. **Optional Steam Playtest before sale.** Steam Playtest uses a child App ID
   with controlled admission and a playable/not-playable switch. It supports
   signups or direct Playtest keys and has a simplified review. The feature is
   free to developers and testers; this does not remove the parent product's
   Steam Direct onboarding requirement. It is a sensible later distribution
   option once the current physical EOS acceptance gates pass.
   [Steam Playtest](https://partner.steamgames.com/doc/features/playtest).

No Steam launch date is justified yet. The immediate next milestone is the
brother's QA2 physical test, followed by brand/access approval and a small college
group. Store preparation can follow without redesigning the working multiplayer.
