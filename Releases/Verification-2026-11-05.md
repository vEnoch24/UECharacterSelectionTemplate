# Release verification — tests run 5 November 2026

Report finalized 6 November 2026.

Candidate: `FightingCharacterSelect-0.7.0-UE5.8-Win64-Candidate.zip`.

SHA-256: `579c5002c2b8a89ea550a77134c87556304e6a0c574c9b5aec41d8d3c6f4380e`.

## Scope

Unreal Engine 5.8, Windows x64. The existing candidate was retained: every archive file matches the packaged output under `Saved/V1103Fixed/P`, and its source matches the current plugin source. The archive contains editor binaries and Development/Shipping runtime precompiled manifests. It contains no demo Content, host project, Saved files or local logs.

The independent host is under `Saved/V1103Fixed/H`. It installs the packaged plugin locally and uses only generated assets and engine placeholder textures. The reusable host source and verification script are under `Tests/ReleaseHost` and `Tools` respectively.

## Results

- Archive/source/package consistency: passed.
- Independent host editor build: passed.
- Fixture generation and saving: passed; seven setup assets, two characters and two maps.
- Win64 Development build, cook, package and archive: passed (BuildCookRun exit 0).
- CPU, mixed keyboard/gamepad and two-gamepad simulated session runs: passed, each exit 0 with a CHARSELECT_RELEASE_PASS marker after real cooked level travel. Checks include selection ID, character IDs, CPU flags, device snapshots, loadable character assets and consumption.
- Rendered packaged runs: passed at 1280×720 and 1920×1080. PNG dimensions were verified and screenshots inspected: banners, portraits, labels and focus rails remain in bounds. The first 1080p request was clamped by Unreal to 888×500; a repeat with ForceRes produced the verified 1920×1080 image.
- Final demo-host regression suite: passed; 20 succeeded, zero warnings, zero failures, zero skipped. The current demo editor build also succeeded. Tests include existing navigation and original preview-animation bindings. Results: `Saved/Automation/ReleaseVerification/index.json`; log: `Saved/V1103Fixed/DemoAutomation.log`.

The fixture module now loads at Default so Unreal can find its commandlet before engine initialization completes. Setup options are held with a strong UObject reference during Blueprint compilation. The small test editor module avoids precompiled headers to reduce memory requirements. These changes affect only the test host, not the plugin or existing demo animations.

Evidence is retained under `Saved/V1103Fixed`: `Fixture.log`, `CookConsole.txt`, `RunConsole.txt`, `CPU.log`, `Mixed.log`, `Pads.log`, `Rendered720.log` and `Rendered1080.log`.

Screenshots: [720p](VerificationImages/Starter-1280x720.png), [1080p](VerificationImages/Starter-1920x1080.png). These show engine placeholder textures, not the demo artwork or authored animation timing.

The PowerShell runner now waits explicitly for packaged GUI executables and verifies both exit status and the success marker. The original call-operator path returned early with an unset exit code; the actual first CPU run passed, and all three modes were rerun with correct process tracking.

## Pending manual acceptance

Physical keyboard/gamepad discovery, both join orders, separate controller ownership, held-button behavior and unplug/reconnect require real hardware input. Injected session actions do not verify those paths. The authored demo's animation timing also requires a visual Play review. Use the plugin's `RELEASE_CHECKLIST.md` and record actual device models/results before calling this release fully accepted.

To start the manual test from the repository root in PowerShell:

```powershell
& '.\Saved\V1103Fixed\G\Windows\SelectionReleaseHost\Binaries\Win64\SelectionReleaseHost.exe' -windowed -ForceRes -ResX=1280 -ResY=720 -log
```

Do not add `SelectionSmoke` for hardware testing. The destination is intentionally an empty test map: its receiver logs the match handoff; it does not spawn fighters. Restart the executable for each join-order/device scenario.

The candidate remains a beta; other engine versions/platforms and a Shipping end-to-end game run are not certified by these checks. No distribution license or public publication is implied.
