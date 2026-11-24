# Fighting Character Select

Source plugin for Unreal Engine 5.8. Provides local two-player/CPU character selection, exclusive device joining, grid/linear navigation with optional directional links, preview animation hooks, sound/button hints, theme snapshots, match handoff, and a Slate authoring studio.

## Install

1. Copy this `FightingCharacterSelect` folder into `<YourProject>/Plugins/`. Exclude generated `Binaries`, `Intermediate` and `Saved` folders when sharing the source.
2. Enable **Fighting Character Select** in Unreal's Plugins window and restart. Compile for your engine/platform with its supported C++ toolchain. This is a source plugin; the demo project's binaries are not a portable binary distribution.
3. Open **Tools → Character Selection** to access Studio.

The plugin has no dependency on the demo project's Content, GameMode, GameInstance, default map, or input configuration. It adds no project-wide settings. Native module names remain `CharacterSelection` and `CharacterSelectionEditor` to preserve serialized Blueprint parent references. Do not install alongside an older project-local copy of those modules.

For an Unreal Automation Tool packaged ZIP, extract its contents into `<YourProject>/Plugins/FightingCharacterSelect/` and retain the supplied `Binaries` and `Intermediate` precompiled files. The instruction to exclude build folders above applies to sharing the editable source workspace, not to unpacking a binary release. Packaged binaries are specific to their advertised engine version and platform.

## Create your first screen

Choose **New setup** in Studio. Leave **Starter Screen** empty to generate a basic screen plus editable slot, grid and preview Widget Blueprints. Supply a unique setup name and a `/Game/...` destination. Review the seven output assets, create them, and use Studio’s Save selected assets to save all seven, including the generated components. Add character data assets to the generated roster (unique Character IDs, available status, thumbnail/large portrait and display text). Place exactly one generated launcher in a level, then Play.

The generated screen uses native controls and engine fonts, with no bundled character art or animation tracks. Edit the component Blueprints in UMG Designer for a custom look; use Studio for layout, prompts, controls, themes, feedback and handoff. Assign preview animations or named animation styles through the preview Blueprint and Studio. Existing authored animations are never rewritten by the wizard.

For an existing design, assign **Starter Screen** instead. This creates four assets: a screen copy, layout, roster, and launcher. The copied screen preserves its widget tree and animations; referenced slot/preview classes and art remain shared. Use Content Browser's **Migrate** for an existing project's screen and dependencies before selecting it in another project. Copying this plugin does not copy project artwork.

Created assets are unsaved and never overwrite existing packages. All seven basic destinations are checked before creation. Inspect partial results if creation fails; the wizard does not delete existing content.

## Studio presentation workflow

### Built in Studio Guide

The **Studio Guide** toolbar button opens a separate, resizable Slate guide with 15 grouped chapters, short walkthroughs with clear steps and brief tips, search, Previous/Next navigation and a remembered reading position. It opens automatically once per user/project when the editor first starts with the plugin enabled, without requiring Studio to be opened; later visits are manual. It does not modify selection assets. The guide remains open independently of Studio.

Each chapter reserves space for an illustration. Add PNG files and edit captions in `Resources/Guide/guide.json`, then use **Reload guide**. Content and new images need no C++ rebuild. See `Resources/Guide/AUTHORING.txt` for image paths, multiple images per chapter, content blocks and onboarding preferences. These resources are included in plugin packaging.

The sidebar groups Setup/Roster/Character data, Presentation, Animation, Input & flow, and Reuse & customize. Under Presentation, use **Slots**, **Character preview**, and **Text & names** to edit connected designs without manually locating each component. Slot discovery follows the grid's Slot Widget Class; preview discovery follows preview widgets in the selected screen. An optional Component Blueprint can also supply a design directly. Shared designs appear once and edits affect all screens using that Blueprint.

Choose a widget in **Choose what to edit** for its appearance, or a **Layout slot** entry for spacing and alignment. The asset path above the details identifies where edits go. Character portraits/names/subtitles are supplied by Character data at runtime; the Text page styles those widgets. Generated join banners use Join prompts. **All visual widgets** retains access to other custom screen/component widgets.

Basic mode is the default. **Advanced settings** reveals technical widget options, directional links and stick/repeat tuning; the preference is remembered per project and switching modes does not change asset values. Search filters the current target's visible properties. Unreal's reset arrows restore individual property defaults (not the last saved version); Ctrl+Z supports undo. Save selected assets also includes connected component Blueprints edited in this Studio session, even after changing pages.

Existing animation timelines remain in UMG Designer. Preview animation styles continue to select authored animation pairs in Existing Blueprint animations mode. Sprite and 3D presentation settings are available on their respective Presentation pages.

## Portrait and text animations

Open **Animation → Portrait & text animations** and choose a player preview instance from the screen, or a preview Component Blueprint's class defaults. **Fade**, **Slide**, **Staggered**, **Zoom**, and **No motion** populate editable settings for Portrait, Name and Subtitle. Each element has an enable switch plus independent Entrance and Exit duration, delay, easing, scale, translation and opacity. Presets support Undo/Redo and change only the selected target. Save and restart Play after editing defaults.

Entrance animates from the configured appearance to the element's Designer appearance; exit animates from the currently displayed appearance to the configured exit appearance. Delay is measured from the start of each phase. The character's portrait, name and subtitle swap together after the longest outgoing transition, then their independent entrances begin. Requests arriving during exit replace the pending character without restarting exit. A request during entrance starts an exit from the current appearance. Clear is handled as a character request, including during transitions. An initial character enters immediately without an empty outgoing phase.

Disabled elements update at the common content swap without motion. Zero duration applies a phase immediately after its delay; No motion disables all three elements. Scale and opacity multiply Designer values, translation adds to Designer position, and rotation, shear, fonts and colors are preserved. A Designer opacity of zero remains invisible. Keep Tick Frequency on Auto and leave room for translated/scaled artwork in parent clipping boundaries.

**Existing Blueprint animations** remains the default. Its button restores the original animation style system without deleting timelines, named styles, or configured element values. Configured mode bypasses automatic authored preview playback; custom Blueprint logic still runs, so avoid driving the same child transforms/opacity simultaneously. Enable Animation preview in the live preview to simulate configured transitions. Existing theme capture does not include the new independent animation settings; reuse them through screen/component Blueprints for now.

## Slot animations

Open **Animation → Slot animations**, choose the connected slot Blueprint, then choose **Subtle**, **Pop**, **Lift**, **Fade**, or **No motion**. These presets populate editable state settings and enable **Configurable state animations**. Expand Slot Animation Settings and a state to adjust duration, easing (Linear, Ease Out or Smooth Step), scale, translation, opacity and tint. Save selected assets and restart Play to use the new class defaults. Presets replace only the selected slot's animation settings and support Undo/Redo.

The default mode is **Existing Blueprint animations**, so existing setups retain their authored focus, selection and deselection behavior. The button of the same name switches back without erasing your configurable values or altering any animation tracks. Configurable mode replaces the slot's automatic authored focus/selection playback; Blueprint logic and unrelated custom animations still run. Avoid animating the slot widget's own render transform/opacity/tint concurrently from custom code.

State priority is **Locked → Shared selection → Selected → Shared focus → Focused → Idle**. A player occupies a slot through either focus or confirmed selection. Shared selection applies when both players occupy the slot and at least one has selected. Cancelling one selection keeps the other player's selected appearance; cancelling the final selection uses Cancel Duration to return to the remaining focus/idle state. Unfocus uses Idle's duration, and unlocking returns to the current player state. Rapid changes interpolate from the currently displayed appearance without queuing old transitions.

Scale multiplies the Designer scale; translation adds to Designer translation. Opacity and tint multiply Designer values. Rotation and shear are preserved. These are render effects and do not change grid spacing: leave room for larger scales, and check parent clipping in Play. Keep the slot's Tick Frequency on Auto for timed transitions. No motion uses immediate neutral states while player markers continue updating. Each state can also use a zero duration for an immediate change.

Settings are shared by all runtime slots using that slot Blueprint. Theme capture currently covers the existing screen/layout configuration, not these slot settings; share or duplicate the slot Blueprint to reuse them. Enable Animation preview in the live preview to simulate configured slot transitions.

## Modules and integration

* `CharacterSelection` — runtime UMG widgets, data assets, navigation, local device session and GameInstance match subsystem. C++ consumers add this module to their Build.cs dependencies.
* `CharacterSelectionEditor` — editor-only Studio, asset creation, validation and previews. Game targets must not depend on it.

Call `InitializeCharacterSelect` with a registry/layout when hosting the screen yourself. The generated launcher already does this. Optional match handoff stores both selections; the consuming game supplies fighter spawning, possession, gameplay input routing and map cooking. The plugin does not implement networking or persistent save games.

The original demo assets remain under the host project's `Content/CharacterSelect`; they are not part of this plugin. Redistribution/licensing of those assets is a separate decision. No distribution license has been selected for this source yet.

## Verification

Slot shared indicators include both focus and confirmed selection: a selected player continues to occupy the slot after their cursor moves. `BothPlayersFocusBorder` stays visible while both players occupy the slot. When this optional widget and the selection borders are siblings in an Overlay, the runtime instance places it above both selection borders without changing their panel-slot settings or the Blueprint asset. For custom panel hierarchies, place the shared indicator above the selection artwork in the Designer.

The shared Selected/Deselected animations run when the first player selects and the last player cancels. Focus/unfocus transitions run when the slot becomes occupied/empty; cancelling the final selection restores any remaining focus after deselection completes. Authored animation tracks are retained. `CharacterSelection.Editor.SlotSharedState` checks the demo slot's combined states, layer order, and cancellation transitions.

`CharacterSelection.Plugin.BasicStarter` creates and compiles a complete setup without demo assets, initializes its screen/components and checks collision protection. Core navigation/session tests also use in-memory data. The full `CharacterSelection.` suite includes integration tests loading the original demo Blueprints, including their animation bindings; run that full suite in the demo host.

Supported/verified engine baseline: UE 5.8 on Windows. Other engine versions and platforms require their own build and device checks. Release packaging, visual review of the basic starter, cooked map travel and physical controller testing are separate from headless automation.

See [release acceptance checklist](RELEASE_CHECKLIST.md) for the release gates. The repository's `Tools/Verify-CharacterSelectRelease.ps1` and `Tests/ReleaseHost` provide repeatable package, installation, fixture, cook and simulated travel checks.

### Matching Studio layout to Play

In **Setup**, select the launcher Blueprint you placed in your level and click **Use launcher's screen, roster & layout**. This selects its class-default assets for editing, and clears the independent component target. **Validate** and the setup checklist report mismatched launcher references. Check the placed actor's Details for instance overrides as well; those can differ from Blueprint defaults. Save the edited screen before testing Play.

**Grid layout** edits `GridLayout` on the grid embedded in the selected screen. Placement is relative to that grid's **SlotContainer**, not necessarily the whole viewport. The screen Designer controls the grid's containing panel, anchors, size and offsets. Horizontal/vertical navigation resolves its dimensions without overwriting the authored grid settings. Slots are positioned immediately when built; they do not need a first tick or valid cached geometry.

The live preview renders a transient copy of the actual screen widget hierarchy and real slot widgets, using project DPI scaling at the selected pixel resolution. Compare against the same Play viewport resolution. Use its navigation buttons; it does not run gameplay initialization, join input, Blueprint construction events or animation timelines. Layout changes made by gameplay Blueprint code still need a Play test.

`BP_DefaultSelection_Launcher` is optional. Use one launcher configured for your desired setup, or create the screen from your own PlayerController/HUD/GameMode and call `InitializeCharacterSelect` with the matching registry and layout. Avoid launching the same screen from both paths or placing multiple launchers in the level.

## Presentation milestone 4: 2D characters

Open **Studio > Presentation > 2D presentation**. The target picker lists the selected character, roster characters, and player preview instances. Choose a preview for shared defaults; choose a character and enable **Override Presentation 2D** to replace those defaults for that character. Character overrides replace the entire presentation settings group. Existing characters continue using their Large Portrait, falling back to Portrait, with identity framing.

- **Static Artwork:** optionally assign Artwork; otherwise keep the existing character portrait fields.
- **Sprite Frames:** add transparent texture assets to Frames in playback order, then choose Frames Per Second and Loop. Use frames with matching canvas sizes to avoid visual jumps. Playback restarts when the displayed character changes; non-looping clips hold their final frame. Empty/missing frames safely fall back to static artwork. This is a texture-frame animation, not a Paper2D Flipbook or sprite-sheet importer.
- **Framing:** Offset uses logical UMG units; Scale multiplies the authored scale. Facing can keep the original orientation, mirror both previews, or mirror only Player 2. Name/subtitle text is not mirrored. Framing composes with portrait transitions; rapid selection changes swap the presentation with the character's name and subtitle.
- **Custom Widget:** create a Widget Blueprint derived from `CharacterSelectPresentationWidget`, select it as Widget Class, and implement **On Presentation Changed** using its character and player-index parameters. It replaces the artwork region; the outer preview still owns the name and subtitle. Custom widgets own their internal animation behavior; portrait-image animation tracks are not retargeted to their contents.

For a custom widget in an existing preview Blueprint, add a **Named Slot** named exactly `CustomPresentationSlot`, place it over the artwork region, and compile. Leave it empty; runtime supplies its content. New basic setups already include this slot. Missing hosts or invalid classes fall back to normal artwork, and Studio Validate explains missing setup. Switching characters or clearing the preview removes the old custom content. Existing Blueprint animation tracks are never rewritten.

Save the edited character/preview assets, then restart Play to test sprite playback and custom widget events. Studio's layout preview shows static framing and the first sprite frame; it does not tick sprite animation or execute custom Blueprint presentation events. Enable Animation preview for native playback; custom Blueprint events still require Play. Tests cover source fallback, sprite timing/looping, independent facing, relative framing, rapid transitions, clearing and custom-widget lifetime.

## Presentation milestone 5: 3D characters

Open **Studio > Presentation > 3D presentation**. Choose a character, set **Preview Mode** to **Actor 3D**, and enable **Override Presentation 3D** to provide its settings. Alternatively, enable **Default 3D Preview** on a player preview and configure its **Default Presentation 3D**. Characters in **Inherit** mode follow that player preview; **Artwork 2D** explicitly retains the 2D path. Overrides replace the whole settings group.

1. Assign a **Skeletal Mesh**, or an **Actor Class** for a dedicated cosmetic preview actor Blueprint. If both are supplied, the mesh replaces the actor's first skeletal mesh component. Use a preview actor rather than the gameplay pawn: construction, BeginPlay and ticking run in an isolated preview world, without the gameplay level's services.
2. Adjust **Actor Transform**, **Camera Location** and **Camera Target** in Unreal world units. **Player 2 Yaw Offset** turns the second player's actor independently. Camera position controls framing; this version uses a perspective viewport with no exposed FOV control.
3. Set **Key Light Intensity**, **Sky Light Intensity** and **Background Color**. The viewport has an opaque rectangular background; it is not a transparent composited render target. Each player has a separate scene and actor.
4. Optionally assign **Idle Animation** (looping) and **Selection Animation** (one shot, then idle), plus **Animation Rate**. Clips must use the displayed mesh's skeleton. These overrides address the first skeletal mesh component. With no idle override, cancellation or selection completion restores the actor's original animation mode. Actor Blueprints can supply more elaborate behavior themselves.
5. Select the preview target and click **Add presentation host to preview Blueprint** if needed. It creates an empty `CustomPresentationSlot` Named Slot over `LargePortrait`, copying its Canvas layout. Existing hosts are retained. For non-Canvas portrait hierarchies, add the Named Slot manually over the artwork in UMG Designer. Save selected assets.

The viewport changes at the same content-swap point as the name and subtitle. Its transform and opacity follow the existing portrait transitions; authored animation tracks are not rewritten. Clearing or replacing the character destroys the old preview actor. Missing hosts or invalid scene settings fall back to artwork. Studio **Validate** checks source, camera, lighting, animation compatibility and host setup.

Restart Play to inspect framing, lighting and skeletal animation with your assets. Studio's layout-only mode uses fallback artwork. Animation preview supports explicitly assigned skeletal meshes; see milestone 7 below. Automated checks cover scene isolation, camera/actor settings, selection state, cleanup, 2D fallback, transition timing and the host helper; headless checks do not verify rendered lighting or animation poses. This implementation uses Unreal's experimental UMG Viewport widget; target-platform rendering and packaged behavior require release testing.

## Presentation milestone 6: animation sequencing and overrides

Open **Studio > Animation > Animation sequencing**. Select a player preview or character in **Choose what to edit**, then choose **Together**, **Portrait first**, **Text first**, or **No motion**. Presets enable an override on that target. Expand the settings to adjust Entrance Order, Exit Order, Group Gap, and the portrait/name/subtitle transitions. Portrait first introduces the portrait before the text and removes the text first on exit; Text first reverses this. Together starts both groups together, respecting each element's delay. No motion swaps immediately. Presets support Undo/Redo.

The priority is **character override > player preview override > shared sequence asset > existing animations**. Each override replaces the whole sequence. **Inherit** removes a character/player override while retaining its values. An enabled override with **Use sequencing** switched off explicitly uses the existing preview animation settings. On a shared asset, Inherit disables sequencing because that asset has no parent. Existing timelines and styles are retained.

For common defaults, select a player preview and click **Create shared sequence...**, choose a destination, then select the new **Shared sequence / ...** target to edit it. Assign that same asset in the other player's Shared Sequence picker, and turn off local overrides to inherit. Reopen this page after assigning an existing shared asset to refresh its target list. The page explains the current source; for preview targets it evaluates the character selected in Setup. Save selected assets and restart Play.

The flow is **Exit > atomic content swap > Entrance > Ready > Confirm** when Wait For Entrance is enabled. The outgoing character supplies its exit settings and the incoming character supplies its entrance settings. Group Gap is the pause between groups, in addition to individual element delays. Requests during exit replace the pending character without stacking transitions. A new request during entrance exits from the current appearance. Clear removes the queued character and confirmation.

**Wait For Entrance** delays confirmation playback until the incoming presentation settles. **Confirm Delay** adds a pause after it is eligible; cancellation discards a waiting confirm. Repeated confirmed-state updates do not replay confirmation. This controls presentation only: controller selection, slot indicators, match readiness and level travel remain immediate. If your game travels immediately on readiness, its launcher can leave before a cosmetic sequence finishes; coordinate that flow in your game Blueprint.

- **Static 2D artwork:** portrait/name/subtitle transitions use the selected order.
- **Sprite frames:** optional Restart Sprite On Confirm/Cancel restarts the existing frame animation. This does not assign separate confirmation frame sets.
- **Custom 2D widgets:** `On Sequence Event` provides Exit, Entrance, Ready, Confirm and Cancel events. The outer preview offers the same event for custom screen behavior. Events are emitted only with sequencing enabled. Portrait transforms/opacity also drive the custom region in this mode; avoid animating those same outer properties from two places.
- **3D:** confirmation timing starts the configured selection clip; cancel returns to idle. Viewport transitions follow the portrait group. Meshes, clips, camera and lighting remain on the 3D presentation page.

Slot focus and shared-selection state continue updating immediately; customize their visuals on **Slot animations**. Sequencing does not delay input, rewrite authored UMG timelines, or synchronize match handoff. When moving between sequenced and unsequenced characters, the configured transition path finishes the current swap; subsequent unsequenced changes resume the existing animation mode. Studio's layout-only mode shows settled content. Enable Animation preview for native timing; use Play for custom Blueprint events.

Automated coverage includes inheritance priority, explicit opt-out, grouping, delayed and cancelled confirmation, repeated confirmation, rapid clearing, 3D dispatch, and Studio presets/Undo/Redo. Rendering and authored custom Blueprint event behavior still require a Play check with your assets.

## Presentation milestone 7: interactive Studio preview

Open Studio's **Live preview**, then turn **Animation preview: ON**. It starts from the roster's initial focus. Choose Player 1 or Player 2 and use the D-pad, Confirm and Cancel buttons to simulate selection. The centre of the D-pad shows the active player. With Animation preview OFF, configured slots show their settled states. Authored slot animations show their completed poses, including their Keep/Restore State settings, in either preview mode. **Clear preview** removes that player's presentation; navigation or confirmation shows it again. **Reset** rebuilds the temporary screen and replays its starting presentation.

Use **Pause** to hold native animation time and **Step 1/60 s** to advance one frame. Step is available while paused and always advances 1/60 second, regardless of the selected playback speed. Choose **0.25x, 0.5x, 1x or 2x** for continuous playback. The status area shows the clock, speed and each player's phase or a specific fallback explanation. The clock caps large editor stalls rather than skipping an entire sequence. Selection buttons still work while paused; time-based motion waits for playback or stepping.

Supported preview paths:

- Configured slot state transitions, including shared focus and selection.
- Configured portrait/name/subtitle animations and Milestone 6 sequencing, including delayed confirmation and cancellation.
- 2D static artwork and sprite frames, with framing and facing settings.
- 3D previews with an explicitly assigned **Skeletal Mesh**, camera, lighting and idle/selection clips. The preview uses the native skeletal-mesh actor, even when an Actor Class is assigned, so it does not run that actor Blueprint. A presentation host is still required. Mesh animation is advanced by the Studio clock.

**Authored UMG timelines, custom presentation-widget events, actor Blueprint logic, gameplay-driven layout, join input, sound and match travel still require Play.** Unsupported custom/actor-only presentations show fallback artwork with a status explanation. No gameplay screen initialization or device input processor is installed. Animation preview is opt-in and preserves the existing layout-only mode.

Property edits rebuild the preview and reset its clock and simulated selections. Resolution buttons retain the current state while changing the same project DPI layout. Opening Play suspends Studio playback and releases 3D actors; stopping Play rebuilds the preview. Closing or refreshing the preview also releases presentation actors. Preview controls never save or modify source assets; normal edits in Studio still use Save selected assets.

Headless automation checks isolation, pause/step/speed, phase progression, confirm/cancel, clearing, reset and existing layout geometry. Inspect final rendering, lighting and skeletal poses visually with your assets.

## Presentation milestone 8: reusable themes and guided starters

### Create an example you can customize

Open **Character Selection Studio > Setup > Setup wizard**. Leave **Starter Screen** empty to generate independent, editable widget Blueprints. Choose a Presentation option:

| Starter | Starting behavior | What to supply |
| --- | --- | --- |
| Basic - static artwork | Existing basic screen, no added motion | Character portrait and large-portrait textures |
| 2D - animated portraits and text | Lift slot motion, Fade portrait/text transitions, portrait-first entrance and text-first exit | Your character artwork; optionally ordered frames on 2D presentation |
| 3D - skeletal character previews | The same motion/sequence, with a 3D host for both players | Preview Mesh in the wizard; camera framing and optional idle/selection clips in Studio |

Choose a new folder/name, review the generated paths, and create the setup. The wizard creates seven assets and never overwrites existing ones. Add character Data Assets to its roster, save, and place one generated Launcher in a test level. A mesh in the 3D starter is a common default; use **Character data > 3D presentation** overrides to assign a different mesh and animations per fighter. Preview clips must match the mesh skeleton. Camera framing depends on your asset's scale and origin.

Supplying a Starter Screen copies that screen instead; the generated-presentation options are hidden and do not alter the copied screen. No third-party character art or meshes are bundled with these examples.

### Reuse your presentation across screens

1. In Setup, select the source screen and layout. Create or select a Theme.
2. On **Themes & presets**, choose **Capture selected setup into theme**.
3. Select the destination screen and layout in Setup, keeping the Theme selected.
4. Review **Apply sections**, then apply. Validate setup, inspect Live preview, and save selected assets.

**Apply Presentation** copies each named preview's native portrait/name/subtitle motion, 2D artwork/framing, 3D mesh/actor/camera/light/animation defaults, and resolved sequence defaults. Shared sequence settings are captured by value and become a local preview override on the destination; the original shared asset remains unchanged. Per-character overrides still take precedence and are not copied or erased.

**Apply Slot Motion** is a separate opt-in. It changes the grid's slot Blueprint defaults, which may also be used by other screens. Studio includes that Blueprint in Save selected assets. Use an independent slot Blueprint when you want different behavior on different screens. Authored UMG tracks are never copied or edited.

The older **Apply Preview Animations** section copies named authored animation styles; disable it if the destination does not have those named tracks. Art, meshes, clips and actor/widget classes remain shared references, so migrate their dependencies when transferring a theme to another project. Widget names must match (normally Player1Preview and Player2Preview), and custom/3D presentations require CustomPresentationSlot. Incompatible themes are rejected before any target settings change. Apply supports Ctrl+Z. Older snapshots preserve the newer presentation fields until recaptured.

### Validate before distributing

Use **Validate setup**, then check Live preview at each supported resolution. Test shared focus, one-player cancellation, repeated character swaps, Clear, and Pause/Step. Play is required for authored animation timing, Blueprint presentation events, actual keyboard/controller joining, CPU selection and match travel. Inspect skeletal poses and lighting with your own meshes; headless automation checks asset wiring and state logic, not final rendered quality.

The November 2026 presentation updates are source changes; the older 0.7.0 candidate ZIP does not include them. See the [presentation milestone 8 verification report](../../Releases/Presentation-Milestone8-Verification-2026-11-20.md) for build/test results and remaining manual acceptance.
