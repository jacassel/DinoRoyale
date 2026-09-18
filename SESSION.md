# Overnight development session

- Work began 2026-09-18 00:01 America/New_York (04:01 UTC).
- User follow-up requests return at 07:30 in the upcoming morning. Working deadline:
  **2026-09-18 07:30 America/New_York / 11:30 UTC**. Finish saving, smoke testing and committing before then.
- Work remains local. No purchases, paid credits, subscriptions, public uploads, credentials or unrelated OS changes.
- User explicitly authorized consuming **two existing free Codex full-reset credits** when included usage is nearly exhausted. Usage tool confirmed two available at 00:31. None consumed yet.
- Use the reset tool only once a five-hour or weekly window has <=10% remaining. Record each redeemed credit here; do not buy credits or change billing.
- Keep concurrent builds modest (four compiler actions), cap game rendering at 60 FPS, and avoid excessive background workload.

## Working tools

- Unreal: `C:/Unreal Engine/UE_5.8`, version 5.8.2.
- Blender: `C:/Program Files/Blender Foundation/Blender 5.2/blender.exe`, version 5.2.2.
- C++: project-local portable Microsoft toolchain 14.44.35229, Windows SDK 10.0.22621.0, .NET 4.6.2 SDK headers extracted under Tools/Toolchain. Nothing purchased.
- `Tools/Build.ps1` sets UE_SDKS_ROOT only for its process. Editor integration with a Visual Studio IDE is absent, but compilation works.
- `-DinoDevBridge` enables the local test bridge. Disabled without the explicit flag. Tests inject real Unreal input events and read runtime telemetry.
