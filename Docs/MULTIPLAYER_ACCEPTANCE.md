# Multiplayer acceptance record

Every status requires runtime evidence. Local loopback is not EOS or WAN evidence.

| Requirement | Status | Evidence / limitation |
|---|---|---|
| Host can create an internet session. | NOT VERIFIED | EOS product/account configuration and user’s second-PC test pending |
| Remote player can discover it. | NOT VERIFIED | EOS product/account configuration and user’s second-PC test pending |
| Remote player can join it. | NOT VERIFIED | EOS product/account configuration and user’s second-PC test pending |
| Players can be on different networks. | NOT VERIFIED | EOS product/account configuration and user’s second-PC test pending |
| No manual port forwarding required. | NOT VERIFIED | EOS product/account configuration and user’s second-PC test pending |
| Host also plays normally. | PASS (local) | stage-b-04; latency-30 / latency-76 / latency-150-loss2 |
| Client movement works. | PASS (local) | stage-b-04; latency-30 / latency-76 / latency-150-loss2 |
| Client combat works. | PASS (local) | stage-b-04; latency-30 / latency-76 / latency-150-loss2 |
| Server authoritative damage works. | PASS (local) | stage-b-04; latency-30 / latency-76 / latency-150-loss2 |
| Respawning works. | PASS (local) | stage-b-04; latency-30 / latency-76 / latency-150-loss2 |
| Scoring works. | PASS (local) | stage-b-04; latency-30 / latency-76 / latency-150-loss2 |
| 2 humans. | PASS (local) | stage-b-04; latency-30 / latency-76 / latency-150-loss2 |
| 4 humans. | PASS (local) | stage-h-02 |
| Fewer than 10 players with bots OFF. | PASS (local) | stage-c-02 / stage-f-02 |
| Humans + bots. | PASS (local) | stage-c-02 / stage-f-02 |
| 10 networked human/player instances. | PASS (local) | scale-01: host plus nine real packaged processes; headless loopback |
| Five-kill victory condition. | PASS (local) | stage-de-01 |
| 1v1. | PASS (local) | stage-de-01 |
| 2v2. | PASS (local) | stage-h-02 |
| Odd configuration such as 2v1. | PASS (local) | stage-de-01 |
| 5v5. | PASS (local) | scale-01: host plus nine real packaged processes; headless loopback |
| Team bot filling. | PASS (local) | stage-c-02 / stage-f-02 |
| Ten-team-kill victory condition. | PASS (local) | stage-de-01 |
| Human raptor leader. | PASS (local) | stage-g-01 / latency-packs-loss2-02 |
| Two followers replicated. | PASS (local) | stage-g-01 / latency-packs-loss2-02 |
| Followers remain allied. | PASS (local) | stage-g-01 / latency-packs-loss2-02 |
| Leader-only scoring. | PASS (local) | stage-g-01 / latency-packs-loss2-02 |
| Multiple simultaneous packs. | PASS (local) | stage-g-01 / latency-packs-loss2-02 |
| Death/respawn correctly recreates pack. | PASS (local) | stage-g-01 / latency-packs-loss2-02 |
| Disconnect correctly destroys/transfers pack. | PASS (local) | stage-g-01 / latency-packs-loss2-02 |
| 30 ms simulation. | PASS (local) | latency-30: 15 ms outgoing delay per peer; measured ping 86.9 ms |
| ~75 ms simulation. | PASS (local) | latency-76: 38 ms outgoing delay per peer; measured ping 145.3 ms |
| ~150 ms simulation. | PASS (local) | latency-150-loss2: 75 ms outgoing delay per peer, 2% loss; measured ping 243.6 ms |
| Modest packet loss. | PASS (local) | latency-150-loss2: 75 ms outgoing delay per peer, 2% loss; measured ping 243.6 ms |
| No duplicate damage. | PASS (local) | stage-b-04; latency-30 / latency-76 / latency-150-loss2 |
| No duplicate scoring. | PASS (local) | stage-b-04; latency-30 / latency-76 / latency-150-loss2 |
| No duplicate bots. | PASS (local) | stage-c-02 / stage-f-02 |
| No persistent ghost players. | PASS (local) | scale-01: host plus nine real packaged processes; headless loopback |
| Full lobby rejects additional players. | PASS (local) | stage-h-02 |
| Human replaces bot. | PASS (local) | stage-c-02 / stage-f-02 |
| Client disconnect handled. | PASS (local) | stage-c-02 / stage-f-02 |
| Host disconnect handled. | PASS (local) | stage-h-02 |
| Rematch works. | PASS (local) | stage-de-01 |
| Return to lobby works. | PASS (local) | stage-de-01 |
| Repeated matches do not accumulate actors/state. | PASS (local) | stage-de-01 |
| Packaged host works. | PASS (local) | scale-01: host plus nine real packaged processes; headless loopback |
| Packaged client works. | PASS (local) | scale-01: host plus nine real packaged processes; headless loopback |
| External network test completed if physically possible. | NOT VERIFIED | EOS product/account configuration and user’s second-PC test pending |

PASS (local) means separate live game processes controlled by the test harness,
not multiple people or a live EOS service test. Evidence paths are under
`Tests/Results/multiplayer`. Nominal latency rows describe **added** round-trip
delay; game scheduling and transport add baseline latency. Exact total pings
of 30/75/150 ms were not established. The measured pings above are recorded
samples, not complete distributions. All three profiles passed 24 checks.
The 75 ms per-peer + 2% loss pack suite passed another 21 checks.
