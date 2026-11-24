# CharacterSelection
A Character Selection UI template in unreal engine for fighting game

## Character Selection Studio
Open **Tools > Character Selection > Character Selection Studio**. This is the existing custom Slate window, expanded into an asset-backed authoring hub.

1. In **Setup**, select the Screen Widget Blueprint, Layout Data Asset, and Character Registry actually used by your game. The checklist shows missing targets. Use **Create new layout**, **Create new registry**, or **Create new character** to create a correctly typed data asset with a name/location dialog. Newly created assets become the selected target; they do not overwrite existing assets or change runtime references. Add new characters to the registry on the Roster page. Optionally choose a Character Asset and a Component Widget Blueprint (such as the preview or slot widget).
2. Choose a section in the sidebar. If multiple targets exist, use the target dropdown; for example, Player1Preview and Player2Preview can have different animation styles.
3. Edit searchable properties. **Edits apply immediately to the selected asset in memory and support Ctrl+Z. There is no Apply Configuration step.** Native property reset arrows restore defaults.
4. Run **Validate setup** for actionable feedback, then **Save selected assets**. This saves dirty selected packages and character packages referenced by the selected registry, not every dirty asset in the project. A cancelled or failed save is reported.
5. Restart Play to verify the result. Editing is disabled during Play. **Refresh targets** re-resolves current templates and defaults; it does not discard unsaved changes. Targets also refresh after Blueprint compilation and Undo/Redo.

| Section | Settings |
| --- | --- |
| Setup | Remembered asset pickers and workflow guidance |
| Grid layout | Placement, dimensions, spacing, padding, reversal, incomplete rows, slot widget class |
| Navigation | Supported navigation mode, edge behavior, locked-slot inclusion |
| Players & joining | Display names, starting character, CPU fallback, late human joining |
| Controls | Keyboard/gamepad action bindings, join policy, stick threshold and repeat timing |
| Join prompts | Banner font, color, background, margins, padding, top/bottom placement, wording and pulse |
| Preview animations | Per-instance/default animation styles, animation names and speed |
| Roster | Ordered character references |
| Character data | IDs, names, portraits, availability, gameplay classes and tags |
| Visual widgets | Native properties of widget-tree templates and their layout slots; open Designer for hierarchy/timelines |

The optional Component Blueprint lets you edit the preview or slot asset itself. Without it, Visual Widgets targets the screen's tree. Editing a component's class defaults may not override values explicitly set on an instance in the screen: choose the actual player-preview instance when you want an instance change.

**Open asset / Designer** opens the selected target's owning Blueprint or data asset. UMG remains the editor for animation tracks, widget hierarchy and Blueprint logic. The Studio exposes the supported configuration surface; it does not implement arbitrary gameplay systems or turn the project into a distributable plugin yet.

### Asset and layout ownership
Selecting an asset does not change the game's runtime references. Your game must still instantiate the same Screen Blueprint and pass the selected registry/layout to it. Saved contents live in `.uasset` files; remembered picker paths live in this project's editor config and are not runtime references.

Grid appearance, including `GridLayout.Placement`, is edited directly on the grid widget template. **Sync this grid's dimensions to Layout Data** is an explicit, undoable copy of columns, rows, slot size, spacing and padding to the layout data asset. Placement/reversal/centering remain on the widget. The runtime grid supplies its actual arrangement to navigation. Hidden/empty-slot metadata is not offered because those behaviors are not implemented.

### Validation
Checks include selected asset types, supported navigation, two player configurations, duplicate/reserved input bindings, stick thresholds, missing/duplicate roster IDs, missing portraits, grid dimensions, active animation styles and animation names. Validation cannot prove which assets your gameplay code loads or replace visual/device testing in Play.

## Navigation
Choose **Grid**, **Horizontal**, or **Vertical** on the Navigation page, click **Save selected assets**, and restart the character-select screen to use the new mode.

- **Grid** uses the displayed widget's column count, reversal flags, and centered final-row positions. Horizontal movement stays within a row. Vertical movement chooses the nearest eligible row in the requested direction, then the closest displayed column; equal-distance ties use roster order.
- **Horizontal** arranges the roster in one row and accepts Left/Right only.
- **Vertical** arranges the roster in one column and accepts Up/Down only.
- **Wrap** moves to the opposite eligible edge. **Stop** and **Clamp** both retain the current slot at a discrete boundary. Non-navigable slots are skipped; an explicitly empty eligibility list prevents all movement.
- Reversed rows/columns change navigation along with the visuals. A centered incomplete row remains centered when mirrored. A configured row count cannot be smaller than the rows required by the roster.
- **Allow Locked Slots** includes characters whose availability is `Locked` for focus and preview, but confirmation is blocked. If disabled, those characters are omitted. Hidden, DLC, and ComingSoon characters are still omitted; empty slots and the hidden-slot flag are not implemented by this roster builder.
- Radial and Custom remain reserved enum values for asset compatibility but are hidden from mode selection. Older assets using those modes fall back to Grid with a runtime warning. Explicit directional links are available as per-direction overrides (see Milestone 5); radial layouts are deferred.

`UCharacterSelectNavigationProvider` now exposes Blueprint-native Initialize, GetNextIndex, and SetNavigableIndices hooks. A custom provider can be assigned through SetNavigationProvider; the automatic mode selector currently creates only the built-in providers. C++ subclasses should override the corresponding `_Implementation` methods.

Horizontal/vertical modes normalize the runtime widget to a row/column without changing the saved grid dimensions. They do not add scrolling or automatic scaling; choose slot sizes appropriate to the roster and viewport.

Developer verification: run the Unreal automation group `CharacterSelection.Navigation` for grid, linear navigation, and controller/locked-character behavior.

## Device join and controls
The selection screen starts with both sides waiting. The first keyboard key or controller button claims Player 1; the next **different** device claims Player 2. All keyboards are treated as one shared physical input source, so only one player can own keyboard input. Supported pairings are keyboard + controller (in either order), two controllers, or one device with a CPU opponent.

The join press is consumed. Press again to act. Keyboard controls are WASD or arrow keys, Enter/Space to confirm, and Escape/Backspace to cancel. Controllers use D-pad or left stick, the bottom face button (A/Cross) or Start to confirm, and the right face button (B/Circle) to cancel. Stick movement cannot join a player; a digital button is required to avoid accidental joins from stick drift. Confirm/cancel auto-repeat is ignored. Left-stick navigation has a 0.55 engagement threshold, 0.35 release threshold, a 0.35-second initial repeat delay, and 0.12-second repeat interval.

When Player 1 confirms and Player 2 has not joined, Player 1's device automatically controls the CPU slot. Cancel first undoes the CPU confirmation; cancel while choosing an unconfirmed CPU returns to Player 1's selection. A different device can still join Player 2 while the screen remains open: its join press clears the CPU's confirmation and gives that side to the human, preserving its current preview position. The old Tab toggle and separate second-player keyboard controls are replaced by device ownership.

The screen automatically adds flashing prompts at the top of the viewport. To style or position them in the Widget Blueprint, add optional TextBlocks named `Player1JoinText` and `Player2JoinText`. Each replaces that player's generated prompt, with text and pulse opacity driven by C++. Unjoined player previews and grid highlights are hidden until they join or the CPU slot becomes active. Existing PlayerConfigs keep their starting-character settings; Human/CPU status is determined by joining rather than the saved PlayerType value.

Gamepad ownership uses input-device IDs, not Slate user indices, so pads mapped to the same platform user can still join independently. Input interception is limited to the active, focused character-select screen and is removed when the screen closes. Controller disconnection reserves its slot and shows a reconnect prompt; reconnect the same controller and press a button to resume. It does not silently convert a disconnected human to CPU. Window/editor shortcuts such as Alt+Tab and Shift+F1 remain available.

Blueprint integration: `GetJoinSession` exposes `HasJoined`, `GetAssignment`, `IsCPUOpponent`, and `IsReady`. `GetPlayerSelectionController` exposes either player's selected character. `OnPlayersReady` fires when both selections become confirmed and the assigned devices are connected; use this event to continue to the next screen. This template does not automatically launch a match or create gameplay local players. Device assignments and chosen characters should be handed to your game/session system before removing the widget.

Run the `CharacterSelection.Controls` automation group for ownership, mixed inputs, CPU handoff, late join, reconnect, and stick repeat behavior. Physical-controller and visual checks should also be performed in Play mode.

## Customizing join prompts and preview animations
The generated join banners live in `UCharacterSelectWidget::CreateJoinPrompts`, their wording comes from `GetJoinPrompt`, and their pulse comes from `GetPromptOpacity` in `Plugins/FightingCharacterSelect/Source/CharacterSelection/private/UI/CharacterSelectWidget.cpp`.

For visual editing, open `WBP_CharacterSelect` and add TextBlocks named exactly **Player1JoinText** and **Player2JoinText**. Position and style them in Designer; each replaces its generated banner. C++ supplies their text and opacity. Change **Join Prompt Min Opacity** and **Join Prompt Pulse Speed** in Class Defaults (speed 0 disables pulsing). Override **Get Join Prompt** in the Blueprint to customize wording; use PlayerIndex 0/1 and GetJoinSession to distinguish waiting, joined, CPU, and disconnected states. Return parent text when no custom wording is needed.

The older FocusedCharacterImage/FocusedCharacterName widgets are hidden when Player1Preview exists, preventing duplicate art/name display. Player1Preview and Player2Preview keep their existing animations.

To add a preview animation style:
1. Open `WBP_CharacterSelectPreview` (or a child Widget Blueprint). Keep the existing **CharacterChangeOutAnimation** and **CharacterChangeInAnimation** tracks untouched.
2. In UMG's Animations panel, author another pair, for example **SlideOut** and **SlideIn**, targeting the portrait/name/subtitle widgets as desired.
3. In Class Defaults, expand **Character Select > Preview > Animation > Animation Styles**. Add a map entry named **Slide**, set Out Animation Name to **SlideOut**, In Animation Name to **SlideIn**, and Playback Speed to **1**. Use exact UMG animation names. None skips a phase; leaving both None makes an instant style.
4. Set **Animation Style** to **Slide** on the preview widget defaults or on an individual Player1Preview/Player2Preview instance. Alternatively call **Set Animation Style** before **Set Character** in Blueprint. A style change takes effect on the next transition.
5. Set Animation Style back to **None** to use the untouched original animation pair at its original speed. A missing style falls back to the original pair; an invalid animation name logs a warning and skips that phase.

Transitions play the outgoing animation, update content to the latest requested character, then play the incoming animation. Rapid navigation updates the pending character without stacking multiple outgoing transitions. Clear Preview uses the same outgoing transition and no longer gets stuck waiting for an animation it never started.

## Customization notes and plugin roadmap
- Controls are configured on the screen defaults. One key should belong to only one action. Modifier keys, mouse/analog keys, F11, console, and editor/window shortcuts are reserved. Left-stick settings are independent of digital key repeat. Update the editable keyboard/gamepad help text and join hints to match your bindings/join policy.
- Disabling automatic CPU selection requires a second human. Disabling late joining prevents takeover after CPU selection begins. Existing exclusive keyboard/device ownership and reconnect protection remain in effect.
- Generated banner appearance settings do not override custom UMG prompt styling. Blueprint Get Join Prompt overrides also take precedence over the default wording settings.
- Character registry array order determines the displayed roster. Tags, gameplay class and Sort Priority remain data for integrating projects; they do not add gameplay/spawning/filtering by themselves.

Milestones 1–7 and the automated release-verification work are implemented below. Full release acceptance still requires the manual device and animation-timing checks listed under Milestone 8.

Regression groups: `CharacterSelection.Customization` covers remapping, stick tuning and join rules; `CharacterSelection.Editor` checks Studio asset-target discovery and layout synchronization. Existing control, navigation and preview groups remain available.

## Milestone 1: reusable themes and presets
1. In Studio Setup, choose your full Screen Widget Blueprint and Layout Data Asset, then **Create new theme** (or select an existing theme).
2. Open **Themes & presets** and click **Capture selected setup into theme**. Capture replaces the selected theme snapshot, supports Undo, and does not modify the screen.
3. Edit the snapshot if desired. Layout, generated prompt styling/text/pulse, and per-preview named animation styles apply by default. Enable **Apply Controls** and/or **Apply Players** only when you also want to replace bindings or player/join rules.
4. Select a destination Screen Blueprint and Layout Data in Setup, keeping the theme selected. Click **Apply theme to selected setup**, then validate and save. Apply is one undoable operation. Checks run before any target is modified; incompatible animation names or missing preview widgets reject the operation.
5. Save the theme to reuse it later. Duplicate it in the Content Browser to make variants. A theme is a configuration snapshot: changing it does not automatically change screens that previously used it. Reapply explicitly.

Capture requires a screen derived from CharacterSelectWidget with exactly one grid in its widget tree, plus a Layout Data Asset. Preview settings match destination widget names (normally Player1Preview/Player2Preview). The optional Component Blueprint picker is not used for theme transfer. Existing destination animation tracks must already contain the referenced animation names; disable Apply Preview Animations to reuse a theme across incompatible preview Blueprints.

Themes do not copy roster/character references, widget classes, widget hierarchies, arbitrary custom UMG appearance, or animation tracks. Custom prompt TextBlocks retain Designer styling. This milestone adds reusable configuration assets, not a complete visual skin exporter or a runtime theme switcher. Studio's existing Save selected assets includes the selected theme.

## Milestone 2: guided setup wizard

Open Studio **Setup → Create a new selection setup**. The wizard starts with the selected screen, layout and registry (or the bundled screen if no screen is selected).

1. Choose a full starter Screen Widget Blueprint. Optionally select a layout to copy its navigation rules and a registry to copy its character references. Clear the registry for an empty roster.
2. Choose a Content folder such as `/Game/MyFighter/UI` and a setup name such as `ArcadeSelect`. Click **Review setup** to inspect the exact output paths and sharing rules. Back edits these choices; Cancel creates nothing.
3. Click **Create setup**. The wizard duplicates `WBP_ArcadeSelect`, creates `DA_ArcadeSelect_Layout` and `DA_ArcadeSelect_Roster`, and generates `BP_ArcadeSelect_Launcher` with all three references assigned. Grid dimensions come from the screen; optional starter layout navigation rules are retained. Existing asset names are rejected, including unsaved assets.
4. The generated assets become Studio's selections and are highlighted in the Content Browser. **Save selected assets** includes the launcher. Add characters on Roster if starting empty, customize, validate and save.
5. Place **one** generated launcher in a test level and press Play. Remove or disable any other selection launcher to avoid overlapping screens. The wizard does not change the current map, GameMode, or an existing launcher's references.

The launcher derives from the existing `CharacterSelectTestController` actor and uses the existing initialization flow. For integration into a custom menu, create the generated screen and call `InitializeCharacterSelect` with the generated registry/layout instead. Do not launch it twice.

The copied screen has its own widget tree and retains animation settings. Preview/slot Blueprint classes, character data and artwork remain shared references; duplicate those separately if they also need independent editing. The wizard does not produce a standalone distributable plugin or migrate dependencies. Themes can be applied afterward through Themes & presets.

Creation leaves assets unsaved and is not an Undo operation; delete unwanted results through Content Browser. Validation occurs before creation. If duplication or compilation fails later, the wizard reports the failure and highlights any partial results for inspection or deletion; it does not silently remove user assets or overwrite destinations.

## Milestone 3: live layout and navigation preview

Select the Screen Blueprint, Registry and Layout in Studio Setup, then click **Live layout preview** in the toolbar. Keep the preview window beside Studio while editing. Property changes, asset selection, Blueprint compilation, Undo/Redo and theme application refresh the preview and reset its selection state. **Reset** also refreshes it manually. Closing Studio closes its preview.

- Choose **1280 x 720**, **1920 x 1080**, or **1024 x 768** to inspect placement and clipping. These are logical UMG canvas dimensions; the window scales the canvas to fit without changing the saved layout. An overflow message flags slots outside the canvas.
- Switch **Player 1 / Player 2**, click a slot, or use **Left/Right/Up/Down**. **Confirm** and **Cancel** exercise the same selection controller used at runtime, including locked characters. The two simulated players have independent state.
- The preview uses the actual roster builder, navigation rules, grid placement calculations, reversed ordering and incomplete-row coordinates. Horizontal and vertical layouts normalize the preview without changing the saved grid. Source assets are never modified by preview interactions.
- This is a schematic grid-container preview: portraits and player labels use simple editor widgets. It assumes the grid container fills the canvas. It does not render the entire Screen Blueprint, custom slot styling, nested container geometry, DPI scaling, join banners, animation timelines, or Blueprint events. Check those in Designer/Play.
- No game world, viewport overlay, device-join session, or global input processor is created. Preview controls are disabled during Play. Use Play for physical keyboard/controller ownership, CPU handoff and late joining.

Verification includes `CharacterSelection.Editor.LiveLayoutPreview` plus the existing navigation, controls, theme and setup-wizard tests.

## Milestone 4: selection sounds and device button hints

Select your Screen Blueprint, then open Studio **Sounds & button hints**. Settings are stored on that screen, support ordinary property Undo, and are copied by the setup wizard.

### Optional UI sounds
In **Sound Settings → Sounds**, add entries for **Joined**, **Reconnected**, **Moved**, **Confirmed**, **Cancelled**, and **Rejected**, and assign short non-looping Sound Wave or Sound Cue assets. Unassigned entries are silent. Volume zero mutes all events; volume and pitch are configurable. No audio assets are added automatically.

Feedback follows results from the device-join session: movement requires an actual focus change, blocked confirmation emits Rejected, CPU-back emits Cancelled for P1, and a consumed join/reconnect press does not also confirm. Boundaries, no-op cancels, ignored confirmation repeats and already-confirmed choices are silent. Disconnecting alone does not play a reconnect sound. Initialization and live layout preview are silent. Direct calls to a selection controller bypass this device-session feedback path.

The screen's **On Selection Feedback** Blueprint event supplies the result and affected player index (0/1) for custom visual responses. Default audio is played before that event. Leave a sound unassigned if your Blueprint handles its audio instead. This does not change character preview animations.

### Binding-aware hints and glyphs
Enable **Hint Settings → Show Action Hints** to add a wrapping row to generated join banners. For each action, the first matching keyboard or gamepad binding is shown. Reorder bindings on Controls to choose the displayed key. Missing device bindings show **Unbound**. These represent digital actions; left-stick navigation still works independently.

- Add entries to **Glyphs** keyed by the real input key (for example Enter or Gamepad Face Button Bottom), then choose an image in the brush. Missing images fall back to the key's readable label.
- Use **Key Labels** for preferred names such as Cross, A, or custom keyboard names; **Action Labels** changes Up/Down/Left/Right/Confirm/Back wording. **Icon Size** controls the icon dimensions.
- **Player 2 Gamepad Glyphs** optionally overrides controller artwork for the second device owner. Unassigned overrides use the main Glyphs map. Supply artwork appropriate for your target controllers: Xbox/PlayStation/Nintendo brands are not automatically detected.
- Each player uses their assigned device. The CPU side uses P1's bindings/artwork until a second human joins. Waiting and disconnected players hide the action row.
- Generated hints replace the static Keyboard Help/Gamepad Help line for joined players. With hints disabled, existing help text remains unchanged. Blueprint Get Join Prompt overrides remain authoritative.

Custom `Player1JoinText`/`Player2JoinText` banners retain their design and static text behavior. For custom UMG hint widgets, call **Get Action Hint Key**, **Get Action Hint Label**, and **Get Action Hint Brush** with player index and action. Invalid keys indicate no connected device owner; brushes without a resource should fall back to a text label. Refresh your custom hints when the join session's **On State Changed** fires.

Themes capture sounds and hints, but only apply them when **Apply Feedback** is enabled. Older themes leave existing feedback unchanged by default. Save and restart Play to check audio volume, artwork, wrapping and physical devices; the schematic live layout preview does not render or play this feedback.

Automation coverage: `CharacterSelection.Controls.FeedbackAndHints` verifies result events, silent no-ops, CPU handoff, late joining, reconnection, remapped hints, glyph fallback and device-specific overrides. Studio target and theme tests cover the new editing page and opt-in preset behavior.

## Milestone 5: per-character directional links

In Studio **Navigation**, enable **Use Directional Links**, then expand **Directional Links**. Add a map entry whose key is the source character's **Character ID** (from Character Data). Expand its Up/Down/Left/Right rules:

- **Automatic** uses the selected grid/linear/custom-provider navigation and edge behavior. A source with no map entry also uses Automatic.
- **Stop** keeps focus on the source, even when wrapping would normally move it.
- **Go to Character** accepts a destination **Character ID**, not its display name, asset name or roster index. The move goes directly to that eligible character regardless of distance or the normal navigation axis.

For example, set A's Right rule to Go to Character with target C to skip B. Add C's Left rule pointing to A separately if you want the return path. Links are one-way. Self-links stay in place.

Run **Validate setup**, save the Layout Data Asset, then use **Live layout preview** to exercise each direction. The preview reports link problems and uses the same runtime resolver. Refresh/reinitialize after changing roster availability or ordering during gameplay.

IDs must be non-empty and unique across the entire selected registry, including currently excluded characters. Reordering the roster preserves links. Missing or ambiguous explicit destinations stop movement without an automatic fallback. Hidden/DLC/ComingSoon destinations stop; locked characters can be focused only when Allow Locked Slots is enabled and still cannot be confirmed. Confirmed selections cannot move. Invalid source/target IDs and excluded targets appear in Studio validation. Automatic and Stop do not require a target ID.

Links are stored per Layout Data Asset so screens sharing character assets can have different routes. The setup wizard copies them when copying a starter layout; validate them against the new roster. Theme application leaves directional links untouched because they depend on roster IDs. Links override navigation only: they do not move slots, add empty cells, draw connecting arrows, or implement radial/freeform visual layouts. Existing screen and preview animation tracks are unchanged.

Verification: `CharacterSelection.Navigation.DirectionalLinks` tests override/fallback behavior, one-way routes, reordering, hidden/locked/missing/ambiguous targets, self-links, linear modes and confirmation. Live-preview tests also cover link execution and validation feedback.

## Milestone 6: match/session handoff

The **Character Select Match Subsystem** is a Game Instance subsystem. It holds one pending selection snapshot across level travel within that running game instance. It is not a SaveGame, online session, or gameplay input router.

### Launcher example (no changes to existing behavior unless enabled)

1. Use the setup wizard to create a selection setup, or select an existing launcher derived from `CharacterSelectTestController` in Studio Setup.
2. Open **Match handoff** and enable **Store Selection On Ready**. Save the launcher Blueprint. When both selections are confirmed and human devices are connected, it stores the match. Cancelling, late joining or disconnecting while the menu remains active clears the pending snapshot when the session is no longer ready.
3. To travel automatically, enable **Travel On Ready** and assign **Match Level**. Validation checks the destination asset exists. Include that map in your project's packaged-map/cooking configuration. A missing destination leaves the menu open, stores the snapshot, and reports **Last Handoff Error**.
4. Place one generated launcher in the selection level. Place a `CharacterSelectMatchReceiver` actor (or a Blueprint child) in the destination level. On BeginPlay the receiver reads the saved result, exposes **Match Selection** and **Received Selection**, and logs the two choices. In a Blueprint child, implement **On Match Selection Received** to continue your game's flow.
5. Enable **Consume On Read** on the receiver if it is the sole consumer. Otherwise the example reads without clearing; call **Clear Selection** once your game has accepted it. Use **Selection ID** to identify a snapshot if multiple systems read it.

The example passes the data but does not spawn fighters, choose spawn points, create gameplay local players, possess pawns, or assign Enhanced Input contexts. Those decisions belong to the consuming game. The receiver can be tested directly in a destination map with no stored match: Received Selection stays false and no event fires.

### Custom Blueprint flow

At the screen's **On Players Ready** event:

1. Get Game Instance Subsystem → **CharacterSelectMatchSubsystem**.
2. Call **Store From Screen**, passing the character-selection screen. Check its return value and Error before proceeding.
3. On success, remove the screen and open your next level, or show your next menu.
4. At the destination, call **Get Selection** (a copy) or **Consume Selection** (a copy followed by clearing). Always check the return value; false resets the output to an empty result.

For manual flows that leave the selection menu open, clear/rebuild the snapshot on cancellation, disconnect, or late joining; the store deliberately retains a snapshot independently of the source widget/session. The opt-in launcher handles this invalidation automatically. Do not bind both your own travel handler and the launcher's automatic travel to the same ready event.

### Result contents and validation

Each result contains a new **Selection ID** and exactly two player records in P1/P2 order. Each record contains Player Index, Character ID, Character Asset (soft reference), Character Class (soft class reference), Is CPU, Device assignment, Costume Index, and Color Index. Costume/color reflect the current selection state; this milestone does not add costume selection UI.

Storage rejects incomplete sessions, disconnected human devices, unavailable selected characters, empty/duplicate selected Character IDs, and selections outside the supplied registry. Failed storage clears previous pending data. CPU records have no owned device even though P1 chose their character. Mirror matches are permitted. Missing gameplay classes do not prevent storing a UI selection; the receiving game must validate/load its required assets before spawning.

Device IDs describe local devices in this running process. They are not local-player indices, network identities, or persistent hardware IDs. A saved connected flag is a snapshot, not a continuing connection guarantee. Revalidate device connectivity and perform your game's player/device routing after travel; do not treat a gamepad device number as a Create Player controller index. Multiple game instances/PIE sessions have separate subsystem stores. Selections do not survive application restart or replicate to other machines.

### Verification

`CharacterSelection.Match.SnapshotLifecycle` covers readiness, CPU and mixed-device results, copied outputs, independent source-session lifetime, stale-data rejection, selected-ID/availability checks, consumption and null inputs. Studio tests cover the launcher editing target. These tests do not substitute for an actual packaged travel/device test: build with the selected destination map, select both sides, verify the receiver, then check keyboard + gamepad, two gamepads, CPU choice, cancellation and reconnection on the target platform.


## Milestone 7: reusable source plugin

Runtime and Studio modules now live under `Plugins/FightingCharacterSelect`. The project is a demo host with a small `CharacterSelectionDemo` primary game module. Native module/class names are unchanged, so existing Blueprint references, animation tracks, maps and artwork stay at their original paths. Game targets no longer include the editor module. The unused EnhancedInput build dependency has been removed; the existing device-join input processor remains unchanged.

See [plugin installation and setup guide](Plugins/FightingCharacterSelect/README.md). Copy only that plugin folder to another project. Do not copy the demo Content or Config merely to enable the plugin.

**New setup** now works without a starter: clear Starter Screen to generate seven assets (screen, grid, slot, preview, layout, roster, launcher). The four-asset copy workflow is still available for existing screens and preserves their animations. Basic generated widgets use engine resources and contain no demo art. Add your character assets to the roster and customize the component Blueprints before Play. Outputs are unsaved until you save them.

After updating the source layout, regenerate the project's IDE files so source links follow the plugin directories. Build the `CharacterSelectionEditor` target as before. The runtime module's `/Script/CharacterSelection` identity is preserved; no asset redirects or resaves are required solely for this move.

Verification for this milestone: UE 5.8 Win64 Development Editor and Game builds succeeded; all 20 demo-host automation tests passed without warnings; `CharacterSelection.Plugin.BasicStarter` also passed without warnings in an empty host with no demo Content or Config. IDE project files were regenerated. These checks do not certify a cooked distribution, actual map travel, controller hardware, or visual layout.

## Milestone 8: release verification

The final demo-host regression run passed all 20 tests with zero warnings or failures. The verification report was finalized on 6 November 2026; full manual acceptance remains pending.

The Win64 UE 5.8 candidate is in `Releases/FightingCharacterSelect-0.7.0-UE5.8-Win64-Candidate.zip`, with a SHA-256 sidecar and a dated [verification report](Releases/Verification-2026-11-05.md). Its files match the packaged output and plugin source at the time of release verification. Later presentation milestones are source updates and are not included in that candidate ZIP. Demo art, demo Config, test-host files and local caches are excluded.

The independent test host generated and saved a fresh setup, then built, cooked and packaged successfully. CPU, mixed keyboard/gamepad and two-gamepad simulated sessions passed real level travel and destination-receiver checks. Rendered packaged screenshots were inspected at verified 1280×720 and 1920×1080 dimensions; see the report for images and logs.

`Tools/Verify-CharacterSelectRelease.ps1` and `Tests/ReleaseHost` retain the repeatable workflow. The test fixture now loads its commandlet early enough, keeps its options alive during Blueprint compilation, and uses a smaller-memory editor build. The runner waits for packaged GUI processes and checks exit codes and success markers.

Full release acceptance remains pending physical keyboard/controller testing and review of the authored screen's animation timing. Injected session actions and static starter screenshots cannot certify these. The report contains the manual launch command and links to the acceptance checklist. The candidate remains beta until those checks are recorded.


## Presentation expansion (second milestone series)

Presentation milestones 1-8 add Studio organization, configurable slot/portrait/text animations, 2D and 3D presentations, sequencing, interactive preview, reusable presentation themes and guided 2D/3D starters. See the [plugin guide](Plugins/FightingCharacterSelect/README.md) for onboarding and theme application. These source updates are separate from the earlier 0.7.0 candidate ZIP; manual device and rendering acceptance is still required.
