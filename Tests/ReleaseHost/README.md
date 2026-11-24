# Release verification host

This small project is a test fixture, not part of the distributed plugin. It installs the packaged plugin into its own Plugins folder. It does not reference the demo project's assets or configuration.

Run `Tools/Verify-CharacterSelectRelease.ps1` from PowerShell. By default it chooses a new output directory under the repository's Saved folder. Use `-RunRoot <absolute-path-under-Saved>` to keep a known evidence location. Stages are Package, Host, Fixture, Cook and Run; All executes them in that order. Package and Host refuse existing output directories. After a failed later stage, inspect its logs before retrying that stage with the same RunRoot.

The fixture commandlet creates seven editable setup assets, two placeholder fighters and two maps. The launcher stores selections and travels to the destination map; its receiver reads the same GameInstance snapshot. The cooked executable exercises CPU selection, keyboard/gamepad, and two-gamepad session assignments. PASS requires matching selection IDs and character IDs after travel, loadable character assets, the expected CPU flag, and successful consumption of the pending result.

These modes inject actions directly into the join session. They test game flow and cooking, not OS device discovery, physical buttons, controller disconnects or input-processor routing. Do not report them as hardware tests.

For a rendered test, run the packaged executable with `-SelectionSmoke=Mixed -SelectionScreenshot -ForceRes -windowed -ResX=1280 -ResY=720`. Omit NullRHI. It requests a screenshot before final confirmation, then completes the same travel test. For manual input testing, launch without SelectionSmoke; the selection screen waits for physical input.

The host uses engine placeholder textures; it intentionally contains no demo character artwork or demo animation tracks.

The editor fixture module loads at Default so commandlet discovery succeeds. Its options are strongly referenced during Blueprint compilation. The runner explicitly waits for packaged GUI processes, checks their exit codes and PASS markers, and stops only its own test process if it exceeds 90 seconds. For offscreen screenshot verification, add `-RenderOffscreen` and confirm the saved PNG dimensions; `-ForceRes` prevents desktop-size clamping.
