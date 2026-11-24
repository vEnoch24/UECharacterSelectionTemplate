# Release acceptance checklist

Baseline: Unreal Engine 5.8, Win64. Keep build logs, automation reports, packaged test logs and screenshots with each release candidate. A successful build is not a substitute for the checks below.

## Automated checks

- BuildPlugin produces editor, Development and Shipping plugin binaries for Win64.
- A separate host builds with only the packaged plugin installed.
- The setup wizard's generated assets can be saved, loaded in another process and cooked.
- The packaged selection map stores a ready selection, travels to the destination, and its receiver reads the same snapshot with loadable character assets.
- CPU, mixed-device and two-gamepad simulated session scenarios pass. These are injected session actions, not hardware tests.
- Demo-host regression tests preserve existing preview animation bindings and existing navigation behavior.

## Visual review

- Review the generated screen at 1280×720 and 1920×1080: banners, grid, names, portraits and focus indicators remain readable and within bounds.
- Review the user's authored screen separately; a neutral starter screenshot does not verify existing character art or animation timing.
- Check prompt flashing, character-change animations, rapid navigation, cancellation and repeated confirmation in Play and in a rendered packaged build.

## Physical input acceptance

Use a rendered packaged build, with no SelectionSmoke argument. Record controller models, connection type, OS and result for each case.

- Keyboard joins only one human slot; the same keyboard cannot join both.
- Keyboard + gamepad work in either join order.
- Two gamepads join separately and cannot control each other's slot.
- P1 alone confirms a fighter, chooses the CPU fighter and completes selection.
- P2 joins while P1 is choosing the CPU; ownership and prompts update correctly.
- Disconnect/reconnect before confirmation and after one player confirms; no stale ready state or unwanted travel occurs.
- Held directions repeat as configured; held confirm does not double-confirm.
- Cancel returns through selection states correctly.
- The consuming game's gameplay possession/input routing is verified independently of the menu snapshot.

## Distribution

- Include plugin source, descriptor and installation instructions; include binaries only for the advertised engine/platform combination.
- Exclude the demo Content, project Config, test host, local caches and logs from the plugin distribution.
- Choose a distribution license and verify rights for any additional assets before publishing.
- List untested engine versions/platforms and remaining manual checks explicitly. Do not mark a release as fully validated while required hardware or visual checks are pending.
