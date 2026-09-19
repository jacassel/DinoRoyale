# Development and usage policy

Follow the user's September 19, 2026 usage policy for this project.

- Remain a single autonomous agent. Do not spawn agents or delegate work.
- Do not purchase credits, subscriptions, assets, services, or anything else.
- Check account usage at meaningful milestones: before starting work, after a
  substantial tested change, and before a final checkpoint. Do not poll constantly.
- Use the lowest available remaining percentage among applicable usage windows.
  If no Astra-specific allowance is exposed, disclose that and use the reported
  account allowance. Missing usage data is unknown, not unlimited.
- Above 60%: normal productive development; larger reversible work is acceptable.
- 60–35%: complete and test existing systems; avoid large speculative additions.
- 35–20%: stop expanding scope, finish the current feature, repair regressions,
  run targeted gameplay tests, and consolidate a stable checkpoint.
- 20–10%: no new gameplay features; core regression, important bug fixes,
  launch verification, and saving/committing a recoverable build take priority.
- Below 10%: only immediately necessary repairs; verify launch and controls,
  save/commit, and update concise status and exact launch instructions.
- At boundaries use the more conservative band. A functional tested game takes
  priority over another feature. Prefer small recoverable changes at milestones.
- Preserve existing working changes. Keep the known-good standalone build until
  its replacement has passed launch and gameplay checks. Dist is ignored by Git:
  a source commit alone does not back up the playable package.

See README.md for launch/controls, TEST_LOG.md for verification, KNOWN_ISSUES.md
for limitations, and HANDOFF.md for the latest recovery checkpoint.
