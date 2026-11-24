# Presentation milestone 8 verification — 20 November 2026

Source implementation: presentation theme snapshots, guided 2D/3D starters, onboarding documentation and runtime compatibility checks. This is the second milestone series; the earlier 0.7.0 release candidate is unchanged.

## Automated results

| Check | Result | Evidence |
| --- | --- | --- |
| UE 5.8 Win64 Development Editor build | Passed | [Build log](../Saved/Logs/PresentationMilestone8EditorFinal.txt) |
| UE 5.8 Win64 Development Game build | Passed | [Build log](../Saved/Logs/PresentationMilestone8GameVerified.txt) |
| CharacterSelection automation suite | 27 passed, 0 failed, 0 skipped | [Report](../Saved/Automation/PresentationMilestone8ReleaseCheck/index.json), [log](../Saved/Logs/PresentationMilestone8ReleaseCheck.log) |

One passing test, LiveLayoutPreview, records existing grid diagnostic messages at warning severity. The final test run exited successfully. Tests ran headlessly with NullRHI, so these results do not certify rendered quality or physical devices.

Coverage includes native portrait/text motion and 2D framing snapshots, resolved shared-sequence defaults copied by value, optional slot motion, compatibility with older snapshots, failed preflight without partial changes, and Undo. New generated-starter tests create both 2D and 3D setups, check all seven generated assets and compiled Blueprint bindings, reject a missing 3D mesh, and capture/apply each presentation through a theme. The 3D fixture uses Unreal's SkeletalCube asset; it does not require project character art. Existing slot focus, shared selection, preview visibility, navigation, sequencing, joining and match-state tests also pass.

The standalone build exposed an existing editor-only SetDesignerFlags call in the 3D preview path. It is now guarded with WITH_EDITOR, and both targets build successfully.

## User workflow

See [Presentation milestone 8 in the plugin guide](../Plugins/FightingCharacterSelect/README.md#presentation-milestone-8-reusable-themes-and-guided-starters). Starter examples are generated on demand by the setup wizard; this work does not replace the project's existing screen, roster, art or Blueprint animations. Generated test assets were not saved into project Content.

## Manual release acceptance still required

- Generate and save a 2D setup; reopen it, assign your roster and artwork, then inspect both players at the supported resolutions.
- Generate and save a 3D setup with your mesh. Check camera framing, lighting, idle/selection animations and per-character mesh overrides. Check fallback artwork for missing or incompatible presentation assets.
- Capture a theme from one setup and apply it to another. Inspect both players, save/reopen, and confirm that character overrides still take precedence. Check shared slot Blueprint usage before enabling Apply Slot Motion.
- Exercise rapid swaps, shared focus, one-player cancellation, sequence confirmation delays, Clear, Reset and pause/step. Review authored UMG motion and custom Blueprint events in Play.
- Test one keyboard plus one gamepad, two gamepads, late join, disconnect/rejoin, CPU selection and match handoff using actual hardware.
- Cook/package the intended project and repeat the visual/input checks in the packaged build before distribution.

No new candidate ZIP was produced, and the previous candidate was not overwritten. This source verification does not claim a fresh clean-host install, cooked-package validation, GPU rendering inspection or physical-controller acceptance.
