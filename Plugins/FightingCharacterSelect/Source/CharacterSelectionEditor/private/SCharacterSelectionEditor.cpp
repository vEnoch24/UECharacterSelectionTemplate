#include "SCharacterSelectionEditor.h"
#include "../CharacterSelectionEditor.h"
#include "Misc/App.h"
#include "Core/CharacterSelectTestController.h"
#include "SCharacterSelectLivePreview.h"
#include "Widgets/SWindow.h"
#include "Framework/Application/SlateApplication.h"
#include "CharacterSelectThemeOperations.h"
#include "Data/CharacterSelectTheme.h"
#include "WidgetBlueprint.h"
#include "Blueprint/WidgetTree.h"
#include "UI/CharacterSelectWidget.h"
#include "UI/CharacterSelectSlotWidget.h"
#include "Components/TextBlock.h"
#include "Components/NamedSlot.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Kismet2/KismetEditorUtilities.h"
#include "Components/RichTextBlock.h"
#include "Widgets/Input/SCheckBox.h"
#include "UI/CharacterSelectGridWidget.h"
#include "UI/CharacterSelectPreviewWidget.h"
#include "Data/CharacterSelectLayoutData.h"
#include "Data/CharacterSelectRegistry.h"
#include "Data/CharacterSelectData.h"
#include "Core/CharacterSelectNavigationProvider.h"
#include "Components/PanelSlot.h"
#include "Animation/WidgetAnimation.h"
#include "AssetRegistry/AssetData.h"
#include "AssetToolsModule.h"
#include "IAssetTools.h"
#include "Factories/DataAssetFactory.h"
#include "Editor.h"
#include "FileHelpers.h"
#include "Kismet2/BlueprintEditorUtils.h"
#include "Subsystems/AssetEditorSubsystem.h"
#include "ScopedTransaction.h"
#include "PropertyEditorModule.h"
#include "IDetailsView.h"
#include "PropertyEditorDelegates.h"
#include "PropertyCustomizationHelpers.h"
#include "Misc/ConfigCacheIni.h"
#include "Misc/PackageName.h"
#include "UObject/UnrealType.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SScrollBox.h"
#include "Widgets/Layout/SSplitter.h"
#include "Widgets/Layout/SWidgetSwitcher.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Input/SComboBox.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Layout/SWrapBox.h"
#include "Styling/AppStyle.h"
#include "Styling/CoreStyle.h"

#define LOCTEXT_NAMESPACE "CharacterSelectionHub"
namespace CharacterSelectionHub
{
	const TCHAR* Section = TEXT("CharacterSelectionEditor");
	const TCHAR* Keys[] = {TEXT("WidgetBlueprintPath"), TEXT("LayoutDataPath"), TEXT("RegistryPath"), TEXT("CharacterPath"), TEXT("ComponentBlueprintPath"), TEXT("ThemePath"), TEXT("LauncherPath")};
	const TCHAR* Pages[] = {TEXT("Setup"), TEXT("Grid layout"), TEXT("Navigation"), TEXT("Players & joining"), TEXT("Controls"), TEXT("Join prompts"), TEXT("Preview animation styles"), TEXT("Roster"), TEXT("Character data"), TEXT("All visual widgets"), TEXT("Themes & presets"), TEXT("Sounds & button hints"), TEXT("Match handoff"), TEXT("Slots"), TEXT("Character preview"), TEXT("Text & names"), TEXT("Slot animations"), TEXT("Portrait & text animations"), TEXT("2D presentation"), TEXT("3D presentation"), TEXT("Animation sequencing")};
	const TCHAR* Help[] = {
		TEXT("Select the assets your game actually uses. Selections are remembered for this project; choosing an asset here does not change your game's references."),
		TEXT("Edit the actual grid widget. Placement, size, padding, ordering and slot class are saved in its Blueprint. Runtime navigation follows this grid. Horizontal/Vertical modes use one row/column. Sync layout copies shared dimensions into the Layout Data Asset."),
		TEXT("Choose Grid, Horizontal or Vertical and edge behavior. Stop and Clamp keep focus at the boundary; Wrap moves to the opposite edge. Enable directional links and add a source Character ID. Each direction can use Automatic, Stop, or Go to Character (destination ID). Links are one-way and override geometry; missing/hidden/ambiguous targets stop. Radial layouts are not implemented."),
		TEXT("Configure player names and starting focus, automatic CPU selection, and late joining. Device ownership is exclusive: only one player may use the keyboard. Saved Human/CPU and Player Index fields are ignored by the device-join system."),
		TEXT("Assign keyboard and gamepad keys to actions. Each key should map to one action. Modifier keys and editor shortcuts are reserved. Tune left-stick dead zones and repeat timing. Update prompt help text if you change the bindings."),
		TEXT("Customize banner colors, font, padding, position, wording and pulse. To design your own banners, add Player1JoinText/Player2JoinText to the screen in UMG; those keep their Designer styling. Get Join Prompt can be overridden in Blueprint."),
		TEXT("Choose a player preview instance or a component Blueprint's defaults. Add named Animation Styles using exact UMG animation names. None uses the existing animation pair. Open Designer to author timelines; this panel never edits animation tracks."),
		TEXT("Add/remove character asset references and reorder the roster array. This array order is the current runtime order; Sort Priority is not automatically applied. Use Character Data to edit each entry."),
		TEXT("Edit identity, display name, subtitle, portraits, availability, gameplay class and tags. Choose an entry from your selected registry below, or use the Character Asset picker in Setup."),
		TEXT("Edit widget appearance and layout slots in the optional Component Blueprint, or the Screen Blueprint if no component is selected. Open Designer for visual positioning, hierarchy, animation tracks and Blueprint logic. Changes affect this exact template, not an unrelated runtime instance."),
		TEXT("Create or select a theme in Setup. Capture reads the selected Screen Blueprint and Layout Data into the theme. Apply copies enabled sections to those targets with Undo. Controls and players are opt-in. Matching preview names and animations are required; timelines, roster references and custom UMG styling are not copied."),
		TEXT("Assign short UI sounds to result events. Enable action hints to use your actual bindings in generated banners. Add key-to-image glyphs or leave them empty for readable labels. Player 2 can use alternate gamepad icons; hardware brand is not auto-detected. Custom UMG prompts use the Blueprint hint getters. Test audio and glyphs in Play."),
		TEXT("Choose a Launcher Blueprint in Setup. Enable Store Selection On Ready to retain the confirmed match across levels. Optionally enable Travel On Ready and assign a Match Level. Place a CharacterSelectMatchReceiver actor in the destination to read it. This does not spawn fighters or assign gameplay local players; include the destination map in packaging settings."),
        TEXT("Customize the slot's images, borders and player labels. The Studio finds the Slot Widget Class assigned to your grid. Select a widget below to edit its appearance, or its Layout slot to edit spacing. Shared slot Blueprints affect every slot that uses them. Use Open asset / Designer to author focus and selection timelines."),
        TEXT("Customize portrait images and preview layout. Connected preview Blueprints are discovered from the screen automatically. Edits here change the shared preview design; use Preview animation styles for each player instance. Character artwork comes from Character data at runtime."),
        TEXT("Choose a text widget to change its font, color, shadow and alignment, or choose its Layout slot for spacing. Text widgets are discovered in the screen and connected slot/preview Blueprints. Character names and subtitles come from Character data at runtime; generated join wording is edited on Join prompts. Timelines remain editable in the Designer."),
        TEXT("Choose a connected slot Blueprint below. Existing Blueprint animations preserves its current behavior. Choose a preset to enable configurable animations, then expand each state to adjust duration, easing, scale, movement, opacity and tint. Presets replace only these animation settings and support Undo. Shared selection means both players occupy the slot and at least one has selected. Locked appearance takes priority. Cancel Duration controls the last selection's return to focus or idle. Test transitions in Play; custom Blueprint logic can still animate inner widgets."),
        TEXT("Choose a player preview instance or a component Blueprint's defaults. Choose Fade, Slide, Staggered or Zoom, then expand Portrait, Name and Subtitle to customize each entrance and exit. Delay staggers elements within a phase. All character content swaps together after the longest exit; rapid navigation always uses the latest request. Disabled elements update without motion. Existing Blueprint animations restores the original style system without deleting its tracks. Save your edits. Open Live preview and enable Animation preview to test configured transitions; authored timelines require Play."),
        TEXT("Choose a player preview for defaults, or a character for overrides. Static artwork uses Large Portrait then Portrait unless Artwork is assigned. Sprite Frames plays ordered textures at the chosen frame rate; use uniform transparent canvas sizes. Offset and Scale are relative to the authored portrait. Mirror Player 2 flips only that player. Custom Widget requires a CharacterSelectPresentationWidget Blueprint and a Named Slot called CustomPresentationSlot in the preview Designer. New basic setups include it. Enable Animation preview to play sprite frames; custom Blueprint events require Play."),
        TEXT("Choose a character and set Preview Mode to Actor 3D, or enable Default 3D Preview on a player preview. Assign a dedicated cosmetic actor Blueprint or skeletal mesh, then tune camera location/target, actor transform, lighting and clips. Each player gets a separate scene. Select a player preview and click Add presentation host for Canvas-based portraits; otherwise add a Named Slot called CustomPresentationSlot in its Designer. The viewport background is opaque. Enable Animation preview in Live preview to test an assigned Skeletal Mesh and its clips. Actor Blueprint behavior requires Play."),
        TEXT("Start with a preset below, then choose entrance/exit order and confirmation timing. Priority: character override > player preview override > shared sequence asset > existing animations. Overrides replace the whole group; turn off Override to inherit again. Use sequencing off explicitly restores existing animation settings. Select a preview to create or assign a shared asset, then select that shared asset in the target list to edit it. Confirm can wait for entrance; cancelling discards a queued confirm. Slot focus/selection remains immediate on the Slot animations page. Save and test in Play.")
	};
	UObject* Defaults(UObject* Object)
	{
		auto* Blueprint = Cast<UWidgetBlueprint>(Object);
		return Blueprint && Blueprint->GeneratedClass ? Blueprint->GeneratedClass->GetDefaultObject() : nullptr;
	}
}

SCharacterSelectionEditor::~SCharacterSelectionEditor()
{
	if (auto Window = PreviewWindow.Pin()) Window->RequestDestroyWindow();
	if (GEditor) { GEditor->UnregisterForUndo(this); GEditor->OnBlueprintCompiled().Remove(CompileHandle); }
}

void SCharacterSelectionEditor::Construct(const FArguments& InArgs)
{
	LoadSelections();
	FDetailsViewArgs Args;
	Args.bAllowSearch = true;
	Args.bHideSelectionTip = true;
	Args.bLockable = false;
	Args.NameAreaSettings = FDetailsViewArgs::HideNameArea;
	Details = FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor").CreateDetailView(Args);
	Details->SetIsPropertyVisibleDelegate(FIsPropertyVisible::CreateSP(this, &SCharacterSelectionEditor::IsPropertyVisible));
	Details->SetIsPropertyEditingEnabledDelegate(FIsPropertyEditingEnabled::CreateLambda([] { return GEditor && !GEditor->PlayWorld; }));
	Details->OnFinishedChangingProperties().AddSP(this, &SCharacterSelectionEditor::OnPropertyChanged);
	if (GEditor)
	{
		GEditor->RegisterForUndo(this);
		CompileHandle = GEditor->OnBlueprintCompiled().AddSP(this, &SCharacterSelectionEditor::RebuildTargets);
	}
	TSharedRef<SVerticalBox> Sidebar = SNew(SVerticalBox);
	const int32 PageOrder[] = {0, 7, 8, 1, 13, 14, 18, 19, 15, 5, 16, 17, 20, 6, 2, 3, 4, 11, 12, 10, 9};
    for (const int32 Page : PageOrder)
    {
        const TCHAR* Group = Page == 0 ? TEXT("GET STARTED") : Page == 1 ? TEXT("PRESENTATION") : Page == 16 ? TEXT("ANIMATION") : Page == 2 ? TEXT("INPUT & FLOW") : Page == 10 ? TEXT("REUSE & CUSTOMIZE") : nullptr;
        if (Group) Sidebar->AddSlot().AutoHeight().Padding(8, 14, 0, 4)
            [SNew(STextBlock).Text(FText::FromString(Group)).Font(FCoreStyle::GetDefaultFontStyle("Bold", 10))];
		Sidebar->AddSlot().AutoHeight().Padding(0, 3)
		[
			SNew(SButton).HAlign(HAlign_Left).ContentPadding(FMargin(12, 9))
			.ButtonColorAndOpacity_Lambda([this, Page] { return ActivePage == Page ? FLinearColor(0.12f, 0.35f, 0.6f) : FLinearColor::White; })
			.Text(FText::FromString(CharacterSelectionHub::Pages[Page]))
			.ToolTipText(FText::FromString(CharacterSelectionHub::Help[Page]))
			.OnClicked_Lambda([this, Page] { SelectPage(Page); return FReply::Handled(); })
		];
	}
	ChildSlot
	[
		SNew(SBorder).Padding(16)
		[
			SNew(SVerticalBox)
			+ SVerticalBox::Slot().AutoHeight().Padding(0,0,0,6)
			[SNew(STextBlock).Text(LOCTEXT("Title", "Character Selection Studio")).Font(FCoreStyle::GetDefaultFontStyle("Bold", 22))]
			+ SVerticalBox::Slot().AutoHeight().Padding(0,0,0,10)
			[SNew(STextBlock).Text(LOCTEXT("Workflow", "Choose assets → customize → validate → save → test in Play. Property edits update assets immediately; Ctrl+Z undoes property edits. Use the wizard to create a connected setup.")).AutoWrapText(true)]
			+ SVerticalBox::Slot().AutoHeight().Padding(0,0,0,10)
			[
				SNew(SWrapBox).UseAllottedSize(true)
                + SWrapBox::Slot().Padding(0,0,8,0)
                [SNew(SButton).Text(LOCTEXT("StudioGuide", "Studio Guide"))
                    .ToolTipText(LOCTEXT("StudioGuideHelp", "Open the searchable step-by-step manual, with tutorials and illustration spaces."))
                    .OnClicked(this, &SCharacterSelectionEditor::OpenStudioGuide)]
                + SWrapBox::Slot().Padding(0,0,12,0)
                [SNew(SCheckBox)
                    .IsChecked_Lambda([this] { return bShowAdvancedSettings ? ECheckBoxState::Checked : ECheckBoxState::Unchecked; })
                    .OnCheckStateChanged_Lambda([this](ECheckBoxState State) { SetAdvancedSettings(State == ECheckBoxState::Checked); })
                    .ToolTipText(LOCTEXT("AdvancedHelp", "Basic mode hides technical widget options and advanced tuning. Enable this to show them. Switching modes never changes saved values."))
                    [SNew(STextBlock).Text(LOCTEXT("Advanced", "Advanced settings"))]]
                + SWrapBox::Slot().Padding(0,0,8,0)[SNew(SButton).Text(LOCTEXT("LivePreview", "Live layout preview")).IsEnabled_Lambda([] { return GEditor && !GEditor->PlayWorld; }).OnClicked(this, &SCharacterSelectionEditor::OpenLivePreview)]
				+ SWrapBox::Slot().Padding(0,0,8,0)[SNew(SButton).Text(LOCTEXT("Validate", "Validate setup")).OnClicked(this, &SCharacterSelectionEditor::Validate)]
				+ SWrapBox::Slot().Padding(0,0,8,0)[SNew(SButton).Text(LOCTEXT("Save", "Save selected assets")).IsEnabled_Lambda([] { return GEditor && !GEditor->PlayWorld; }).OnClicked(this, &SCharacterSelectionEditor::Save)]
				+ SWrapBox::Slot().Padding(0,0,8,0)[SNew(SButton).Text(LOCTEXT("Refresh", "Refresh targets")).ToolTipText(LOCTEXT("RefreshHelp", "Re-read current assets after compiling or editing them elsewhere. Does not discard unsaved changes.")).OnClicked_Lambda([this] { RebuildTargets(); return FReply::Handled(); })]
				+ SWrapBox::Slot().Padding(0,0,8,0)[SNew(SButton).Text(LOCTEXT("Open", "Open asset / Designer")).OnClicked(this, &SCharacterSelectionEditor::OpenTarget)]
			]
			+ SVerticalBox::Slot().AutoHeight().Padding(0,0,0,10)
			[SNew(SBox).MaxDesiredHeight(140)[SNew(SScrollBox) + SScrollBox::Slot()[SNew(STextBlock).Text_Lambda([this] { return GetStatus(); }).AutoWrapText(true)]]]
			+ SVerticalBox::Slot().FillHeight(1)
			[
				SNew(SSplitter)
				+ SSplitter::Slot().Value(0.22f)[SNew(SScrollBox) + SScrollBox::Slot()[Sidebar]]
				+ SSplitter::Slot().Value(0.78f)
				[
					SNew(SVerticalBox)
					+ SVerticalBox::Slot().AutoHeight().Padding(12,0,0,6)[SNew(STextBlock).Text_Lambda([this] { return FText::FromString(CharacterSelectionHub::Pages[ActivePage]); }).Font(FCoreStyle::GetDefaultFontStyle("Bold", 18))]
                        + SVerticalBox::Slot().AutoHeight().Padding(12,0,0,8)[SNew(STextBlock).Text_Lambda([this] { return FText::FromString(CharacterSelectionHub::Help[ActivePage]); }).AutoWrapText(true)]
					+ SVerticalBox::Slot().AutoHeight().Padding(12,0,0,8)[SAssignNew(TargetBar, SVerticalBox)]
                        + SVerticalBox::Slot().AutoHeight().Padding(12,0,0,8)
                        [SNew(STextBlock).Text(this, &SCharacterSelectionEditor::GetTargetDescription).AutoWrapText(true)
                            .Visibility_Lambda([this] { return ActivePage == 0 ? EVisibility::Collapsed : EVisibility::Visible; })]
					+ SVerticalBox::Slot().FillHeight(1).Padding(12,0)
					[
						SNew(SWidgetSwitcher).WidgetIndex_Lambda([this] { return ActivePage == 0 ? 0 : 1; })
						+ SWidgetSwitcher::Slot()[BuildSetup()]
						+ SWidgetSwitcher::Slot()[Details.ToSharedRef()]
					]
				]
			]
		]
	];
	RebuildTargets();
}

FReply SCharacterSelectionEditor::OpenStudioGuide()
{
    FModuleManager::LoadModuleChecked<FCharacterSelectionEditorModule>(TEXT("CharacterSelectionEditor")).OpenStudioGuide();
    return FReply::Handled();
}

TSharedRef<SWidget> SCharacterSelectionEditor::BuildSetup()
{
	const TCHAR* Labels[] = {TEXT("Screen Widget Blueprint"), TEXT("Layout Data Asset"), TEXT("Character Registry"), TEXT("Character Asset (optional)"), TEXT("Component Widget Blueprint (optional)"), TEXT("Theme / preset (optional)"), TEXT("Launcher Blueprint (optional)")};
	const TCHAR* Hints[] = {TEXT("The screen your game creates. A grid-only Blueprint can also be used for layout editing."), TEXT("The navigation/roster settings your controller reads."), TEXT("The ordered character list used at runtime."), TEXT("Edit a character directly, or choose a registry entry on Character Data."), TEXT("Preview, slot, or other widget to edit independently. Clear this to edit the screen's visuals."), TEXT("Reusable configuration snapshot. Capture and apply it on Themes & presets."), TEXT("Created by the setup wizard. Place one instance in a test level to launch the connected screen, roster and layout.")};
	UClass* Classes[] = {UWidgetBlueprint::StaticClass(), UCharacterSelectLayoutData::StaticClass(), UCharacterSelectRegistry::StaticClass(), UCharacterSelectData::StaticClass(), UWidgetBlueprint::StaticClass(), UCharacterSelectTheme::StaticClass(), UBlueprint::StaticClass()};
	TSharedRef<SVerticalBox> Box = SNew(SVerticalBox);
	Box->AddSlot().AutoHeight().Padding(4, 8)
	[SNew(SButton).Text(LOCTEXT("SetupWizard", "Create a new selection setup..."))
		.IsEnabled_Lambda([] { return GEditor && !GEditor->PlayWorld; })
		.OnClicked(this, &SCharacterSelectionEditor::OpenSetupWizard)];
	Box->AddSlot().AutoHeight().Padding(4, 8)
	[SNew(SBorder).Padding(12)[SNew(STextBlock).Text(this, &SCharacterSelectionEditor::GetSetupChecklist).AutoWrapText(true)]];
	for (int32 Index = 0; Index < 7; ++Index)
	{
		Box->AddSlot().AutoHeight().Padding(4,8)[SNew(STextBlock).Text(FText::FromString(Labels[Index])).Font(FCoreStyle::GetDefaultFontStyle("Bold", 12))];
		Box->AddSlot().AutoHeight().Padding(4,0,4,6)[SNew(STextBlock).Text(FText::FromString(Hints[Index])).AutoWrapText(true)];
		Box->AddSlot().AutoHeight().Padding(4,0,4,16)
		[SNew(SObjectPropertyEntryBox).AllowedClass(Classes[Index]).ObjectPath_Lambda([this, Index] { return Assets[Index].IsValid() ? Assets[Index]->GetPathName() : FString(); })
		.OnObjectChanged_Lambda([this, Index](const FAssetData& Data) { AssetChanged(Data, Index); }).AllowClear(true).DisplayUseSelected(true).DisplayBrowse(true)];
		if (Index == 6)
			Box->AddSlot().AutoHeight().Padding(4,0,4,12)
			[SNew(SButton).Text(LOCTEXT("UseLauncherAssets", "Use launcher's screen, roster & layout"))
				.ToolTipText(LOCTEXT("UseLauncherAssetsHelp", "Load the selected launcher's class defaults into Studio. Level-instance overrides must match these defaults. This changes Studio's selection only."))
				.IsEnabled_Lambda([this] { return Assets[6].IsValid() && GEditor && !GEditor->PlayWorld; })
				.OnClicked(this, &SCharacterSelectionEditor::UseLauncherAssets)];
		if ((Index >= 1 && Index <= 3) || Index == 5)
		{
			Box->AddSlot().AutoHeight().HAlign(HAlign_Left).Padding(4, 0, 4, 12)
			[SNew(SButton)
				.Text(FText::FromString(Index == 1 ? TEXT("Create new layout...") : Index == 2 ? TEXT("Create new registry...") : Index == 3 ? TEXT("Create new character...") : TEXT("Create new theme...")))
				.ToolTipText(LOCTEXT("CreateDataHelp", "Choose a folder and name for a new asset. It becomes this Studio's selected target; existing assets and runtime references are not replaced."))
				.IsEnabled_Lambda([] { return GEditor && !GEditor->PlayWorld; })
				.OnClicked_Lambda([this, Index] { return CreateDataAsset(Index); })];
		}
	}
	Box->AddSlot().AutoHeight().Padding(4,12)[SNew(STextBlock).Text(LOCTEXT("SetupNote", "Create data assets here or choose existing ones. The Studio does not replace runtime references, create gameplay players, or package a plugin. Use Open asset / Designer for animation timelines and Blueprint logic. Changes should be tested after restarting Play.")).AutoWrapText(true)];
	return SNew(SScrollBox) + SScrollBox::Slot()[Box];
}

FText SCharacterSelectionEditor::GetSetupChecklist() const
{
	const bool bScreen = Cast<UCharacterSelectWidget>(CharacterSelectionHub::Defaults(Assets[0].Get())) != nullptr;
	const bool bLayout = Cast<UCharacterSelectLayoutData>(Assets[1].Get()) != nullptr;
	const auto* Registry = Cast<UCharacterSelectRegistry>(Assets[2].Get());
	const bool bRegistry = Registry != nullptr;
	FString Checklist = TEXT("GETTING STARTED\n");
	Checklist += bScreen ? TEXT("[Ready] Screen Blueprint selected\n") : TEXT("[Needed] Use New setup to generate a basic screen, or select your existing screen\n");
	Checklist += bLayout ? TEXT("[Ready] Layout Data selected\n") : TEXT("[Needed] Create or select a Layout Data Asset\n");
	Checklist += bRegistry ? FString::Printf(TEXT("[Ready] Registry selected (%d character references)\n"), Registry->Characters.Num()) : TEXT("[Needed] Create or select a Character Registry\n");
	if (Registry && Registry->Characters.IsEmpty()) Checklist += TEXT("[Next] Create character assets, then add them on the Roster page.\n");
	else if (bScreen && bLayout && bRegistry) Checklist += TEXT("[Next] Customize the sidebar sections, validate the setup, then save.\n");
	const FString Mismatch = GetLauncherMismatch();
	if (!Mismatch.IsEmpty()) Checklist += TEXT("[Check] ") + Mismatch + TEXT("\n");
	Checklist += TEXT("Your game's screen creation must use these same assets. Validation checks their configuration, not your runtime wiring.");
	return FText::FromString(Checklist);
}

FReply SCharacterSelectionEditor::CreateDataAsset(int32 Index)
{
	if (!GEditor || GEditor->PlayWorld || (!(Index >= 1 && Index <= 3) && Index != 5)) return FReply::Handled();
	UClass* Classes[] = {UCharacterSelectLayoutData::StaticClass(), UCharacterSelectRegistry::StaticClass(), UCharacterSelectData::StaticClass(), UCharacterSelectTheme::StaticClass()};
	const TCHAR* Names[] = {TEXT("DA_CharacterSelectLayout"), TEXT("DA_CharacterRegistry"), TEXT("DA_Character"), TEXT("DA_CharacterSelectTheme")};
	const int32 TypeIndex = Index == 5 ? 3 : Index - 1;
	// Keep the factory alive while Unreal's modal asset dialog is open.
	TStrongObjectPtr<UDataAssetFactory> Factory(NewObject<UDataAssetFactory>());
	Factory->DataAssetClass = Classes[TypeIndex];
	UObject* Created = FModuleManager::LoadModuleChecked<FAssetToolsModule>("AssetTools").Get().CreateAssetWithDialog(
		Names[TypeIndex], TEXT("/Game/CharacterSelect/Data"), Classes[TypeIndex], Factory.Get(), NAME_None, false);
	if (!Created)
	{
		Message = LOCTEXT("CreateCancelled", "No asset created. Your selected targets are unchanged.");
		return FReply::Handled();
	}
	AssetChanged(FAssetData(Created), Index);
	Message = Index == 3
		? LOCTEXT("CreatedCharacter", "Character created and selected. Fill in its ID and portraits on Character Data, then add it to the registry on Roster. Save to persist changes.")
		: Index == 5
		? LOCTEXT("CreatedTheme", "Theme created and selected. Open Themes & presets and capture your selected screen configuration, then save.")
		: LOCTEXT("CreatedData", "Asset created and selected. Customize it in the sidebar and save. Assign it in your game's screen setup before testing.");
	return FReply::Handled();
}

void SCharacterSelectionEditor::LoadSelections()
{
	if (!GConfig) return;
	GConfig->GetBool(CharacterSelectionHub::Section, TEXT("ShowAdvancedSettings"), bShowAdvancedSettings, GEditorPerProjectIni);
	for (int32 Index = 0; Index < 7; ++Index)
	{
		FString Path;
		GConfig->GetString(CharacterSelectionHub::Section, CharacterSelectionHub::Keys[Index], Path, GEditorPerProjectIni);
		if (!Path.IsEmpty()) Assets[Index].Reset(LoadObject<UObject>(nullptr, *Path));
	}
}
void SCharacterSelectionEditor::SaveSelections() const
{
	if (!GConfig) return;
	for (int32 Index = 0; Index < 7; ++Index)
		GConfig->SetString(CharacterSelectionHub::Section, CharacterSelectionHub::Keys[Index], Assets[Index].IsValid() ? *Assets[Index]->GetPathName() : TEXT(""), GEditorPerProjectIni);
	GConfig->Flush(false, GEditorPerProjectIni);
}
void SCharacterSelectionEditor::AssetChanged(const FAssetData& Data, int32 Index)
{
	Assets[Index].Reset(Data.GetAsset());
	SaveSelections();
	Message = FText::GetEmpty();
	RebuildTargets();
}
void SCharacterSelectionEditor::SelectPage(int32 Page)
{
	if (Page < 0 || Page >= UE_ARRAY_COUNT(CharacterSelectionHub::Pages)) return;
	ActivePage = Page;
	Message = FText::GetEmpty();
	RebuildTargets();
}

void SCharacterSelectionEditor::RebuildTargets()
{
	RefreshLivePreview();
	const FString Previous = CurrentTarget.IsValid() ? CurrentTarget->Label : FString();
	Details->SetObject(nullptr);
	Targets.Reset();
	CurrentTarget.Reset();
	auto Add = [this](UObject* Object, UBlueprint* Blueprint, const FString& Label)
	{
		if (!Object) return;
		auto Target = MakeShared<FTarget>(); Target->Object = Object; Target->Blueprint = Blueprint; Target->Label = Label;
		Targets.Add(Target);
	};
	auto* ScreenBP = Cast<UWidgetBlueprint>(Assets[0].Get());
	auto* ComponentBP = Cast<UWidgetBlueprint>(Assets[4].Get());
	if (ActivePage == 12)
	{
		auto* Launcher = Cast<UBlueprint>(Assets[6].Get());
		if (Launcher && Launcher->GeneratedClass) Add(Cast<ACharacterSelectTestController>(Launcher->GeneratedClass->GetDefaultObject()), Launcher, TEXT("Launcher match handoff"));
	}
	if (ActivePage == 10) Add(Assets[5].Get(), nullptr, TEXT("Theme snapshot and apply sections"));
	if (ActivePage == 2) Add(Assets[1].Get(), nullptr, TEXT("Navigation and locked-slot rules"));
	if ((ActivePage >= 3 && ActivePage <= 5) || ActivePage == 11)
		Add(Cast<UCharacterSelectWidget>(CharacterSelectionHub::Defaults(ScreenBP)), ScreenBP, TEXT("Screen defaults"));
	if (ActivePage == 7) Add(Assets[2].Get(), nullptr, TEXT("Roster order and character references"));
	if (ActivePage == 8 || ActivePage == 18 || ActivePage == 19 || ActivePage == 20)
	{
		Add(Assets[3].Get(), nullptr, TEXT("Selected character asset"));
		if (auto* Registry = Cast<UCharacterSelectRegistry>(Assets[2].Get()))
			for (auto* Character : Registry->GetAllCharacters()) if (Character) Add(Character, nullptr, Character->GetName() + TEXT(" — ") + Character->DisplayName.ToString());
	}
	if (ActivePage == 1 || ActivePage == 6 || ActivePage == 17 || ActivePage == 18 || ActivePage == 19 || ActivePage == 20 || ActivePage == 9)
	{
		TArray<UWidgetBlueprint*> Blueprints;
		if (ActivePage == 9) { if (ComponentBP || ScreenBP) Blueprints.Add(ComponentBP ? ComponentBP : ScreenBP); }
		else { if (ScreenBP) Blueprints.Add(ScreenBP); if (ComponentBP && ComponentBP != ScreenBP) Blueprints.Add(ComponentBP); }
		for (auto* Blueprint : Blueprints)
		{
			auto Accept = [this, Blueprint, &Add](UObject* Object, const FString& Name)
			{
				if ((ActivePage == 1 && Cast<UCharacterSelectGridWidget>(Object)) || ((ActivePage == 6 || ActivePage == 17 || ActivePage == 18 || ActivePage == 19 || ActivePage == 20) && Cast<UCharacterSelectPreviewWidget>(Object)) || ActivePage == 9)
					Add(Object, Blueprint, Blueprint->GetName() + TEXT(" / ") + Name);
			};
			if (ActivePage != 9) Accept(CharacterSelectionHub::Defaults(Blueprint), TEXT("Class defaults"));
			if (Blueprint->WidgetTree) Blueprint->WidgetTree->ForEachWidget([&Accept, &Add, Blueprint, this](UWidget* Widget)
			{
				Accept(Widget, Widget->GetName());
				if (ActivePage == 9 && Widget->Slot) Add(Widget->Slot, Blueprint, Blueprint->GetName() + TEXT(" / ") + Widget->GetName() + TEXT(" / Layout slot"));
			});
		}
	}
    if (ActivePage == 20)
    {
        TSet<UCharacterSelectSequenceAsset*> Seen;
        const auto PreviewTargets = Targets;
        for (const auto& Target : PreviewTargets)
            if (auto* Preview = Cast<UCharacterSelectPreviewWidget>(Target->Object.Get()))
                if (Preview->SharedSequence && !Seen.Contains(Preview->SharedSequence))
                { Seen.Add(Preview->SharedSequence); Add(Preview->SharedSequence, nullptr, TEXT("Shared sequence / ") + Preview->SharedSequence->GetName()); }
    }
    if (ActivePage >= 13 && ActivePage <= 16)
    {
        // Follow only the connected screen, preview and slot classes. A visited set
        // prevents duplicate targets and cycles in nested widget Blueprints.
        TArray<UWidgetBlueprint*> Pending;
        TSet<UWidgetBlueprint*> Visited;
        if (ComponentBP) Pending.Add(ComponentBP);
        if (ScreenBP) Pending.Add(ScreenBP);
        auto FollowClass = [&Pending](UClass* Class)
        {
            if (Class) if (auto* BP = Cast<UWidgetBlueprint>(Class->ClassGeneratedBy)) Pending.Add(BP);
        };
        auto FollowWidget = [&FollowClass](UObject* Object)
        {
            if (!Object) return;
            if (Object->IsA<UCharacterSelectPreviewWidget>() || Object->IsA<UCharacterSelectSlotWidget>()) FollowClass(Object->GetClass());
            if (auto* Grid = Cast<UCharacterSelectGridWidget>(Object))
                if (const auto* Property = FindFProperty<FClassProperty>(Grid->GetClass(), TEXT("SlotWidgetClass")))
                    FollowClass(Cast<UClass>(Property->GetObjectPropertyValue_InContainer(Grid)));
        };
        while (!Pending.IsEmpty())
        {
            UWidgetBlueprint* BP = Pending.Pop();
            if (!BP || Visited.Contains(BP)) continue;
            Visited.Add(BP);
            FollowWidget(CharacterSelectionHub::Defaults(BP));
            const bool bSlot = BP->GeneratedClass && BP->GeneratedClass->IsChildOf(UCharacterSelectSlotWidget::StaticClass());
            const bool bPreview = BP->GeneratedClass && BP->GeneratedClass->IsChildOf(UCharacterSelectPreviewWidget::StaticClass());
            if (ActivePage == 16 && bSlot) Add(CharacterSelectionHub::Defaults(BP), BP, BP->GetName() + TEXT(" / Slot animation defaults"));
            if (BP->WidgetTree) BP->WidgetTree->ForEachWidget([&](UWidget* Widget)
            {
                FollowWidget(Widget);
                const bool bText = Widget->IsA<UTextBlock>() || Widget->IsA<URichTextBlock>();
                if ((ActivePage == 13 && bSlot) || (ActivePage == 14 && bPreview) || (ActivePage == 15 && bText))
                {
                    Add(Widget, BP, BP->GetName() + TEXT(" / ") + Widget->GetName());
                    if (Widget->Slot) Add(Widget->Slot, BP, BP->GetName() + TEXT(" / ") + Widget->GetName() + TEXT(" / Layout slot"));
                }
            });
        }
    }

	TargetBar->ClearChildren();
	if (ActivePage != 0)
	{
		TargetBar->AddSlot().AutoHeight().Padding(0,0,0,4)[SNew(STextBlock).Text(LOCTEXT("EditingTarget", "Choose what to edit"))];
		TargetBar->AddSlot().AutoHeight()
		[
			SNew(SComboBox<TSharedPtr<FTarget>>).OptionsSource(&Targets)
			.OnGenerateWidget_Lambda([](TSharedPtr<FTarget> Target) { return SNew(STextBlock).Text(FText::FromString(Target->Label)); })
			.OnSelectionChanged_Lambda([this](TSharedPtr<FTarget> Target, ESelectInfo::Type) { SelectTarget(Target); })
			[SNew(STextBlock).Text_Lambda([this] { return CurrentTarget.IsValid() ? FText::FromString(CurrentTarget->Label) : LOCTEXT("NoTarget", "No matching target — choose assets in Setup"); })]
		];
        if (ActivePage == 18 || ActivePage == 19 || ActivePage == 20)
            TargetBar->AddSlot().AutoHeight().Padding(0,4)
            [SNew(SButton).Text(LOCTEXT("AddPresentationHost", "Add presentation host to preview Blueprint"))
                .ToolTipText(LOCTEXT("PresentationHostHelp", "Select a player preview target. Add an empty Named Slot using LargePortrait's Canvas layout. Existing artwork and animation tracks are preserved. Shared preview classes affect both players. Undo is supported; save afterwards."))
                .IsEnabled_Lambda([this] { return GEditor && !GEditor->PlayWorld && CurrentTarget.IsValid() && Cast<UCharacterSelectPreviewWidget>(CurrentTarget->Object.Get()); })
                .OnClicked(this, &SCharacterSelectionEditor::AddPresentationHost)];
        if (ActivePage == 20)
        {
            auto Buttons = SNew(SWrapBox).UseAllottedSize(true);
            const TCHAR* Labels[] = {TEXT("Inherit"), TEXT("Together"), TEXT("Portrait first"), TEXT("Text first"), TEXT("No motion")};
            for (int32 Index = 0; Index < UE_ARRAY_COUNT(Labels); ++Index)
                Buttons->AddSlot().Padding(0,4,8,4)[SNew(SButton).Text(FText::FromString(Labels[Index]))
                    .ToolTipText(LOCTEXT("SequencePresetTip", "Presets enable an override on the selected character/player, or edit the selected shared asset. Inherit removes a local override. Ctrl+Z undoes the change."))
                    .IsEnabled_Lambda([this] { return GEditor && !GEditor->PlayWorld && CurrentTarget.IsValid(); })
                    .OnClicked_Lambda([this, Index] { return ApplySequencePreset(Index - 1); })];
            Buttons->AddSlot().Padding(0,4,8,4)[SNew(SButton).Text(LOCTEXT("CreateSequence", "Create shared sequence..."))
                .IsEnabled_Lambda([this] { return GEditor && !GEditor->PlayWorld && CurrentTarget.IsValid() && Cast<UCharacterSelectPreviewWidget>(CurrentTarget->Object.Get()); })
                .OnClicked(this, &SCharacterSelectionEditor::CreateSequenceAsset)];
            TargetBar->AddSlot().AutoHeight()[Buttons];
        }
        if (ActivePage == 17)
        {
            auto Presets = SNew(SWrapBox).UseAllottedSize(true);
            const TCHAR* Labels[] = {TEXT("Existing Blueprint animations"), TEXT("Fade"), TEXT("Slide"), TEXT("Staggered"), TEXT("Zoom"), TEXT("No motion")};
            for (int32 Index = 0; Index < UE_ARRAY_COUNT(Labels); ++Index)
                Presets->AddSlot().Padding(0, 4, 8, 4)
                [SNew(SButton).Text(FText::FromString(Labels[Index]))
                    .ToolTipText(LOCTEXT("PreviewPresetHelp", "Update only this preview's portrait/text animation settings. Custom styles and UMG tracks are retained. Ctrl+Z undoes the change."))
                    .IsEnabled_Lambda([this] { return GEditor && !GEditor->PlayWorld && CurrentTarget.IsValid() && CurrentTarget->Object.IsValid(); })
                    .OnClicked_Lambda([this, Index] { return ApplyPreviewAnimationPreset(Index - 1); })];
            TargetBar->AddSlot().AutoHeight()[Presets];
        }

        if (ActivePage == 16)
        {
            auto Presets = SNew(SWrapBox).UseAllottedSize(true);
            const TCHAR* Labels[] = {TEXT("Existing Blueprint animations"), TEXT("Subtle"), TEXT("Pop"), TEXT("Lift"), TEXT("Fade"), TEXT("No motion")};
            for (int32 Index = 0; Index < UE_ARRAY_COUNT(Labels); ++Index)
                Presets->AddSlot().Padding(0, 4, 8, 4)
                [SNew(SButton).Text(FText::FromString(Labels[Index]))
                    .ToolTipText(Index == 0 ? LOCTEXT("AuthoredSlotHelp", "Switch back to the original Blueprint animation behavior. Your configurable values and authored tracks are retained.") : LOCTEXT("SlotPresetHelp", "Replace this slot Blueprint's animation settings with this preset. Then customize the states below. Ctrl+Z undoes the change."))
                    .IsEnabled_Lambda([this] { return GEditor && !GEditor->PlayWorld && CurrentTarget.IsValid() && CurrentTarget->Object.IsValid(); })
                    .OnClicked_Lambda([this, Index] { return ApplySlotAnimationPreset(Index - 1); })];
            TargetBar->AddSlot().AutoHeight()[Presets];
        }
        if (ActivePage == 10)
		{
            TargetBar->AddSlot().AutoHeight().Padding(0, 6)
                [SNew(STextBlock).AutoWrapText(true).Text(LOCTEXT("PresentationThemeGuide", "1. Create/select a Theme in Setup.  2. Capture this screen.  3. Choose destination screen + layout.  4. Review Apply sections, then Apply. Presentation copies each player's 2D/3D defaults, motion and resolved sequence; character overrides stay intact. Slot motion is opt-in because its Blueprint may be shared. Ctrl+Z restores applied settings."))];
			TargetBar->AddSlot().AutoHeight().Padding(0, 8)
			[SNew(SHorizontalBox)
				+ SHorizontalBox::Slot().AutoWidth().Padding(0,0,8,0)
				[SNew(SButton).Text(LOCTEXT("CaptureTheme", "Capture selected setup into theme"))
					.ToolTipText(LOCTEXT("CaptureThemeHelp", "Replaces this theme's snapshot with the current selected screen/layout configuration. Supports Undo. Save to persist."))
					.IsEnabled_Lambda([this] { return GEditor && !GEditor->PlayWorld && Assets[5].IsValid(); })
					.OnClicked_Lambda([this] { return TransferTheme(true); })]
				+ SHorizontalBox::Slot().AutoWidth()
				[SNew(SButton).Text(LOCTEXT("ApplyTheme", "Apply theme to selected setup"))
					.ToolTipText(LOCTEXT("ApplyThemeHelp", "Replaces enabled configuration sections on the selected screen/layout after compatibility checks. Supports Undo. Save to persist."))
					.IsEnabled_Lambda([this] { return GEditor && !GEditor->PlayWorld && Assets[5].IsValid(); })
					.OnClicked_Lambda([this] { return TransferTheme(false); })]];
		}
		if (ActivePage == 2) TargetBar->AddSlot().AutoHeight().Padding(0,6)
			[SNew(SBox).MaxDesiredHeight(100)[SNew(SScrollBox) + SScrollBox::Slot()
				[SNew(STextBlock).AutoWrapText(true).Text_Lambda([this]
				{
					FString Text = TEXT("Character IDs in the selected registry (use these for source and target):\n");
					if (auto* Registry = Cast<UCharacterSelectRegistry>(Assets[2].Get()))
					{
						for (auto* Character : Registry->GetAllCharacters()) if (Character)
							Text += Character->CharacterID.ToString() + TEXT(" — ") + Character->DisplayName.ToString() + TEXT("\n");
					}
					else Text += TEXT("Choose a registry in Setup.");
					return FText::FromString(Text);
				})]]];
		if (ActivePage == 1) TargetBar->AddSlot().AutoHeight().Padding(0,6)
			[SNew(SButton).Text(LOCTEXT("Sync", "Sync this grid's dimensions to Layout Data")).IsEnabled_Lambda([] { return GEditor && !GEditor->PlayWorld; }).OnClicked(this, &SCharacterSelectionEditor::SynchronizeLayout)];
	}
	for (const auto& Target : Targets) if (Target->Label == Previous) { SelectTarget(Target); return; }
	if (!Targets.IsEmpty()) SelectTarget(Targets[0]);
}

void SCharacterSelectionEditor::SelectTarget(TSharedPtr<FTarget> Target)
{
	CurrentTarget = Target;
	Details->SetObject(Target.IsValid() ? Target->Object.Get() : nullptr, true);
}

FReply SCharacterSelectionEditor::ApplySequencePreset(int32 Preset)
{
    if (!GEditor || GEditor->PlayWorld || !CurrentTarget.IsValid() || Preset < -1 || Preset > 3) return FReply::Handled();
    auto* Object = CurrentTarget->Object.Get();
    if (!Object) return FReply::Handled();
    const FScopedTransaction Transaction(LOCTEXT("SequenceTransaction", "Change animation sequence"));
    if (auto* BP = CurrentTarget->Blueprint.Get()) BP->Modify();
    Object->SetFlags(RF_Transactional); Object->Modify();
    if (auto* Preview = Cast<UCharacterSelectPreviewWidget>(Object))
    { Preview->bOverrideSequence = Preset >= 0; if (Preset >= 0) Preview->Sequence = FCharacterSelectSequence::MakePreset(Preset); }
    else if (auto* Data = Cast<UCharacterSelectData>(Object))
    { Data->bOverrideSequence = Preset >= 0; if (Preset >= 0) Data->Sequence = FCharacterSelectSequence::MakePreset(Preset); }
    else if (auto* Shared = Cast<UCharacterSelectSequenceAsset>(Object))
    { if (Preset >= 0) Shared->Sequence = FCharacterSelectSequence::MakePreset(Preset); else Shared->Sequence.bEnabled = false; }
    Object->PostEditChange();
    FPropertyChangedEvent Event(nullptr); OnPropertyChanged(Event); Details->ForceRefresh();
    return FReply::Handled();
}
FReply SCharacterSelectionEditor::CreateSequenceAsset()
{
    if (!GEditor || GEditor->PlayWorld || !CurrentTarget.IsValid()) return FReply::Handled();
    auto* Preview = Cast<UCharacterSelectPreviewWidget>(CurrentTarget->Object.Get());
    if (!Preview) return FReply::Handled();
    TStrongObjectPtr<UDataAssetFactory> Factory(NewObject<UDataAssetFactory>());
    Factory->DataAssetClass = UCharacterSelectSequenceAsset::StaticClass();
    auto* Asset = Cast<UCharacterSelectSequenceAsset>(FModuleManager::LoadModuleChecked<FAssetToolsModule>("AssetTools").Get().CreateAssetWithDialog(TEXT("DA_SelectionSequence"), TEXT("/Game/CharacterSelect/Data"), UCharacterSelectSequenceAsset::StaticClass(), Factory.Get(), NAME_None, false));
    if (!Asset) return FReply::Handled();
    const FScopedTransaction Transaction(LOCTEXT("AssignSequenceTransaction", "Assign shared sequence"));
    if (auto* BP = CurrentTarget->Blueprint.Get()) BP->Modify();
    Preview->Modify(); Preview->SharedSequence = Asset;
    Asset->Sequence = FCharacterSelectSequence::MakePreset(0);
    EditedAssets.AddUnique(Asset);
    FPropertyChangedEvent Event(nullptr); OnPropertyChanged(Event);
    RebuildTargets();
    Message = LOCTEXT("SequenceCreated", "Shared asset created and assigned. Select Shared sequence in the target list to edit it. Assign the same asset to the other player; turn off local overrides to inherit it. Save selected assets.");
    return FReply::Handled();
}

FReply SCharacterSelectionEditor::ApplyPreviewAnimationPreset(int32 Preset)
{
    if (!GEditor || GEditor->PlayWorld || ActivePage != 17 || !CurrentTarget.IsValid() || Preset < -1 || Preset > static_cast<int32>(ECharacterPreviewPreset::NoMotion)) return FReply::Handled();
    auto* Preview = Cast<UCharacterSelectPreviewWidget>(CurrentTarget->Object.Get());
    auto* Blueprint = CurrentTarget->Blueprint.Get();
    if (!Preview || !Blueprint) return FReply::Handled();
    const FScopedTransaction Transaction(LOCTEXT("PreviewPresetTransaction", "Change portrait and text animation preset"));
    Blueprint->Modify();
    Preview->SetFlags(RF_Transactional);
    Preview->Modify();
    FProperty* Property = FindFProperty<FProperty>(UCharacterSelectPreviewWidget::StaticClass(), GET_MEMBER_NAME_CHECKED(UCharacterSelectPreviewWidget, PreviewAnimationSettings));
    Preview->PreEditChange(Property);
    if (Preset < 0) Preview->PreviewAnimationSettings.Mode = ECharacterPreviewAnimationMode::Authored;
    else Preview->PreviewAnimationSettings = FCharacterPreviewAnimationSettings::MakePreset(static_cast<ECharacterPreviewPreset>(Preset));
    FPropertyChangedEvent Event(Property, EPropertyChangeType::ValueSet);
    Preview->PostEditChangeProperty(Event);
    OnPropertyChanged(Event);
    Details->ForceRefresh();
    Message = LOCTEXT("PreviewPresetApplied", "Preview animation settings updated. Customize Portrait, Name and Subtitle below, save, then restart Play. Ctrl+Z undoes this change.");
    return FReply::Handled();
}

FReply SCharacterSelectionEditor::ApplySlotAnimationPreset(int32 Preset)
{
    if (!GEditor || GEditor->PlayWorld || ActivePage != 16 || !CurrentTarget.IsValid() || Preset < -1 || Preset > static_cast<int32>(ECharacterSlotAnimationPreset::NoMotion)) return FReply::Handled();
    auto* Slot = Cast<UCharacterSelectSlotWidget>(CurrentTarget->Object.Get());
    auto* Blueprint = CurrentTarget->Blueprint.Get();
    if (!Slot || !Blueprint) return FReply::Handled();
    const FScopedTransaction Transaction(LOCTEXT("SlotPresetTransaction", "Change slot animation preset"));
    Blueprint->Modify();
    Slot->SetFlags(RF_Transactional);
    Slot->Modify();
    FProperty* Property = FindFProperty<FProperty>(UCharacterSelectSlotWidget::StaticClass(), GET_MEMBER_NAME_CHECKED(UCharacterSelectSlotWidget, SlotAnimationSettings));
    Slot->PreEditChange(Property);
    if (Preset < 0) Slot->SlotAnimationSettings.Mode = ECharacterSlotAnimationMode::Authored;
    else Slot->SlotAnimationSettings = FCharacterSlotAnimationSettings::MakePreset(static_cast<ECharacterSlotAnimationPreset>(Preset));
    FPropertyChangedEvent Event(Property, EPropertyChangeType::ValueSet);
    Slot->PostEditChangeProperty(Event);
    OnPropertyChanged(Event);
    Details->ForceRefresh();
    Message = LOCTEXT("SlotPresetApplied", "Slot animation settings updated. Customize the states below, then save. Restart Play to use the new defaults. Ctrl+Z undoes this change.");
    return FReply::Handled();
}

void SCharacterSelectionEditor::SetAdvancedSettings(bool bEnabled)
{
    bShowAdvancedSettings = bEnabled;
    if (GConfig)
    {
        GConfig->SetBool(CharacterSelectionHub::Section, TEXT("ShowAdvancedSettings"), bEnabled, GEditorPerProjectIni);
        GConfig->Flush(false, GEditorPerProjectIni);
    }
    Details->ForceRefresh();
}

bool SCharacterSelectionEditor::IsAdvancedProperty(const FProperty& Property) const
{
    if (Property.HasAnyPropertyFlags(CPF_AdvancedDisplay)) return true;
    const FName Name = Property.GetFName();
    if (ActivePage == 2 && (Name == TEXT("bUseDirectionalLinks") || Name == TEXT("DirectionalLinks"))) return true;
    if (ActivePage == 4 && (Name == TEXT("EngageThreshold") || Name == TEXT("ReleaseThreshold") || Name == TEXT("InitialRepeatDelay") || Name == TEXT("RepeatInterval"))) return true;
    if (ActivePage == 9 || ActivePage >= 13)
    {
        const FString Category = Property.GetMetaData(TEXT("Category"));
        return Category.StartsWith(TEXT("Accessibility")) || Category.StartsWith(TEXT("Navigation")) ||
            Category.StartsWith(TEXT("Performance")) || Category.StartsWith(TEXT("Clipping")) ||
            Category.StartsWith(TEXT("Localization")) || Category.StartsWith(TEXT("Developer"));
    }
    return false;
}

FText SCharacterSelectionEditor::GetTargetDescription() const
{
    if (!CurrentTarget.IsValid() || !CurrentTarget->Object.IsValid())
    {
        if (ActivePage == 13 || ActivePage == 16) return LOCTEXT("MissingSlots", "No slot design found. Select your Screen in Setup, then assign its Slot Widget Class on Grid layout. You can also choose a slot Blueprint as the optional Component in Setup.");
        if (ActivePage == 14 || ActivePage == 17) return LOCTEXT("MissingPreview", "No preview design found. Select a Screen containing character preview widgets, or choose a preview Blueprint as the optional Component in Setup.");
        if (ActivePage == 15) return LOCTEXT("MissingText", "No text widgets found in the connected designs. Choose a Screen or Component Blueprint in Setup; add Text widgets in its Designer.");
        return LOCTEXT("MissingTargetHelp", "Choose the assets for this section on Setup. Its description above explains which asset is needed.");
    }
    if (ActivePage == 20)
    {
        FString Source = TEXT("Shared defaults: used by every preview referencing this asset.");
        if (auto* Preview = Cast<UCharacterSelectPreviewWidget>(CurrentTarget->Object.Get()))
            Source = Preview->GetSequenceSource(Cast<UCharacterSelectData>(Assets[3].Get())).ToString() + TEXT(" (for the character selected in Setup). Character overrides take priority over this preview.");
        else if (auto* Data = Cast<UCharacterSelectData>(CurrentTarget->Object.Get()))
            Source = Data->bOverrideSequence ? TEXT("Character override: replaces player and shared settings.") : TEXT("Inheriting: uses each player's preview override or shared asset.");
        return FText::FromString(CurrentTarget->Object->GetPathName() + TEXT("\n") + Source + TEXT("\nFlow: Exit > swap character, portrait and text > Entrance > Ready > Confirm. Cancel drops a waiting Confirm. Save, then restart Play."));
    }
    UObject* Owner = CurrentTarget->Blueprint.IsValid() ? static_cast<UObject*>(CurrentTarget->Blueprint.Get()) : CurrentTarget->Object.Get();
    return FText::FromString(FString::Printf(TEXT("Editing: %s\n%s Search filters this target's visible properties. Use the reset arrow beside a changed property to restore its Unreal default; Ctrl+Z undoes edits. Resetting is not a return to the last saved version."),
        *Owner->GetPathName(), bShowAdvancedSettings ? TEXT("Advanced settings shown.") : TEXT("Basic settings shown. Enable Advanced settings for additional options.")));
}

bool SCharacterSelectionEditor::IsPropertyVisible(const FPropertyAndParent& Property) const
{
	const FProperty* Root = Property.ParentProperties.IsEmpty() ? &Property.Property : Property.ParentProperties.Last();
	if (!bShowAdvancedSettings)
    {
        if (IsAdvancedProperty(Property.Property) || IsAdvancedProperty(*Root)) return false;
        for (const FProperty* Parent : Property.ParentProperties) if (IsAdvancedProperty(*Parent)) return false;
    }
    const FName Name = Root->GetFName();
    if (ActivePage == 16) return Name == GET_MEMBER_NAME_CHECKED(UCharacterSelectSlotWidget, SlotAnimationSettings);
    if (ActivePage == 20 && Property.Property.GetFName() == TEXT("Mode")) return false;
    if (ActivePage == 20) return Name == TEXT("SharedSequence") || Name == TEXT("bOverrideSequence") || Name == TEXT("Sequence");
    if (ActivePage == 19) return Name == TEXT("bDefault3DPreview") || Name == TEXT("DefaultPresentation3D") || Name == TEXT("PreviewMode") || Name == TEXT("bOverridePresentation3D") || Name == TEXT("Presentation3D");
    if (ActivePage == 18) return Name == TEXT("DefaultPresentation2D") || Name == TEXT("bOverridePresentation2D") || Name == TEXT("Presentation2D") || Name == TEXT("LargePortrait") || Name == TEXT("Portrait");
    if (ActivePage == 17) return Name == GET_MEMBER_NAME_CHECKED(UCharacterSelectPreviewWidget, PreviewAnimationSettings);
	if (ActivePage == 12) return Name == TEXT("bStoreSelectionOnReady") || Name == TEXT("bTravelOnReady") || Name == TEXT("MatchLevel") || Name == TEXT("LastHandoffError");
	if (ActivePage == 11) return Name == TEXT("SoundSettings") || Name == TEXT("HintSettings");
	if (ActivePage == 1) return Name == TEXT("GridLayout") || Name == TEXT("SlotWidgetClass");
	if (ActivePage == 2) return Name == TEXT("NavigationMode") || Name == TEXT("HorizontalEdgeBehavior") || Name == TEXT("VerticalEdgeBehavior") || Name == TEXT("bAllowLockedSlots") || Name == TEXT("bUseDirectionalLinks") || Name == TEXT("DirectionalLinks");
	if (ActivePage == 3)
	{
		const FName Leaf = Property.Property.GetFName();
		if (Leaf == TEXT("PlayerIndex") || Leaf == TEXT("PlayerType")) return false;
		return Name == TEXT("PlayerConfigs") || Name == TEXT("bAutoCPUOpponent") || Name == TEXT("bAllowLatePlayerJoin");
	}
	if (ActivePage == 4) return Name == TEXT("ControlSettings");
	if (ActivePage == 5) return Name == TEXT("PromptAppearance") || Name == TEXT("PromptText") || Name == TEXT("JoinPromptMinOpacity") || Name == TEXT("JoinPromptPulseSpeed");
	if (ActivePage == 6) return Name == TEXT("AnimationStyles") || Name == TEXT("AnimationStyle");
	return true;
}

void SCharacterSelectionEditor::OnPropertyChanged(const FPropertyChangedEvent& Event)
{
	if (!CurrentTarget.IsValid() || !CurrentTarget->Object.IsValid()) return;
	if (auto* Blueprint = CurrentTarget->Blueprint.Get())
    {
        FBlueprintEditorUtils::MarkBlueprintAsModified(Blueprint);
        EditedAssets.AddUnique(Blueprint);
    }
    else EditedAssets.AddUnique(CurrentTarget->Object.Get());
	CurrentTarget->Object->MarkPackageDirty();
	RefreshLivePreview();
	Message = LOCTEXT("Changed", "Updated the selected asset. Save selected assets to persist it; Ctrl+Z undoes the edit.");
}
void SCharacterSelectionEditor::PostUndo(bool bSuccess)
{
	if (bSuccess) { RebuildTargets(); Message = LOCTEXT("Undo", "Undo/redo applied. Save if you want to keep the current values."); }
}

FText SCharacterSelectionEditor::GetStatus() const
{
	int32 Dirty = 0; TSet<UPackage*> Packages;
	for (const auto& Asset : Assets) if (Asset.IsValid()) Packages.Add(Asset->GetOutermost());
	for (const auto& Asset : CreatedSetupAssets) if (Asset.IsValid()) Packages.Add(Asset->GetOutermost());
	for (const auto& Asset : EditedAssets) if (Asset.IsValid()) Packages.Add(Asset->GetOutermost());
	if (auto* Registry = Cast<UCharacterSelectRegistry>(Assets[2].Get()))
		for (auto* Character : Registry->GetAllCharacters()) if (Character) Packages.Add(Character->GetOutermost());
	for (auto* Package : Packages) if (Package->IsDirty()) ++Dirty;
	const FString Prefix = GEditor && GEditor->PlayWorld ? TEXT("Play is running: editing is disabled. ") : FString();
	return FText::FromString(Prefix + FString::Printf(TEXT("%d selected package(s) have unsaved changes. "), Dirty) + Message.ToString());
}
FReply SCharacterSelectionEditor::OpenTarget()
{
	UObject* Object = ActivePage == 12 ? Assets[6].Get() : CurrentTarget.IsValid() ? (CurrentTarget->Blueprint.IsValid() ? static_cast<UObject*>(CurrentTarget->Blueprint.Get()) : CurrentTarget->Object.Get()) : Assets[0].Get();
	if (GEditor && Object) GEditor->GetEditorSubsystem<UAssetEditorSubsystem>()->OpenEditorForAsset(Object);
	return FReply::Handled();
}
FReply SCharacterSelectionEditor::Save()
{
	if (!GEditor || GEditor->PlayWorld) return FReply::Handled();
	TArray<UPackage*> Packages;
	auto Add = [&Packages](UObject* Object) { if (Object && Object->GetOutermost()->IsDirty()) Packages.AddUnique(Object->GetOutermost()); };
	for (const auto& Asset : Assets) Add(Asset.Get());
	for (const auto& Asset : CreatedSetupAssets) Add(Asset.Get());
	for (const auto& Asset : EditedAssets) Add(Asset.Get());
	if (auto* Registry = Cast<UCharacterSelectRegistry>(Assets[2].Get())) for (auto* Character : Registry->GetAllCharacters()) Add(Character);
	if (Packages.IsEmpty()) { Message = LOCTEXT("NothingToSave", "No selected assets need saving."); return FReply::Handled(); }
	const auto Result = FEditorFileUtils::PromptForCheckoutAndSave(Packages, true, true);
	Message = Result == FEditorFileUtils::PR_Success ? LOCTEXT("Saved", "Selected assets saved. Restart Play to test the result.") : LOCTEXT("NotSaved", "Save was cancelled, declined, or failed. Check unsaved changes and Unreal's messages.");
	return FReply::Handled();
}
FReply SCharacterSelectionEditor::TransferTheme(bool bCapture)
{
	if (!GEditor || GEditor->PlayWorld) return FReply::Handled();
	auto* Theme = Cast<UCharacterSelectTheme>(Assets[5].Get());
	auto* Blueprint = Cast<UWidgetBlueprint>(Assets[0].Get());
	FCharacterSelectThemeTargets TargetsToUse;
	TargetsToUse.Screen = Cast<UCharacterSelectWidget>(CharacterSelectionHub::Defaults(Blueprint));
	TargetsToUse.Layout = Cast<UCharacterSelectLayoutData>(Assets[1].Get());
	int32 GridCount = 0;
	if (Blueprint && Blueprint->WidgetTree)
	{
		Blueprint->WidgetTree->ForEachWidget([&TargetsToUse, &GridCount](UWidget* Widget)
		{
			if (auto* Grid = Cast<UCharacterSelectGridWidget>(Widget)) { TargetsToUse.Grid = Grid; ++GridCount; }
			if (auto* Preview = Cast<UCharacterSelectPreviewWidget>(Widget)) TargetsToUse.Previews.Add(Preview);
		});
	}
	if (GridCount != 1) TargetsToUse.Grid = nullptr;
    UWidgetBlueprint* SlotBP = nullptr;
    if (TargetsToUse.Grid)
        if (const auto* Property = FindFProperty<FClassProperty>(TargetsToUse.Grid->GetClass(), TEXT("SlotWidgetClass")))
            if (auto* Class = Cast<UClass>(Property->GetObjectPropertyValue_InContainer(TargetsToUse.Grid)))
            {
                SlotBP = Cast<UWidgetBlueprint>(Class->ClassGeneratedBy);
                if (SlotBP) TargetsToUse.Slot = Cast<UCharacterSelectSlotWidget>(Class->GetDefaultObject());
            }

	const FScopedTransaction Transaction(bCapture ? LOCTEXT("CaptureThemeTx", "Capture Character Selection Theme") : LOCTEXT("ApplyThemeTx", "Apply Character Selection Theme"));
	FString Error;
	const bool bSuccess = bCapture ? CharacterSelectThemeOperations::Capture(Theme, TargetsToUse, Error) : CharacterSelectThemeOperations::Apply(Theme, TargetsToUse, Error);
	if (!bSuccess) { Message = FText::FromString(Error); return FReply::Handled(); }
	if (!bCapture && Blueprint)
	{
		// The target objects recorded their original property values in this transaction.
		Blueprint->Modify();
		FBlueprintEditorUtils::MarkBlueprintAsModified(Blueprint);
		Blueprint->MarkPackageDirty();
	}
    if (!bCapture && Theme->bApplySlotMotion && SlotBP)
    {
        SlotBP->Modify();
        FBlueprintEditorUtils::MarkBlueprintAsModified(SlotBP);
        EditedAssets.AddUnique(SlotBP);
    }
	Details->ForceRefresh();
	RefreshLivePreview();
	Message = bCapture
		? LOCTEXT("ThemeCaptured", "Captured layout, prompts, per-player presentation, resolved sequencing and slot motion. Review Apply sections: slot motion changes a shared Blueprint and is opt-in, as are controls and players. Art references stay shared; character overrides and UMG tracks are not copied. Save selected assets to persist.")
		: LOCTEXT("ThemeApplied", "Enabled theme sections applied to the selected screen/layout. Ctrl+Z undoes the operation. Validate and Save selected assets, then restart Play.");
	return FReply::Handled();
}

FReply SCharacterSelectionEditor::SynchronizeLayout()
{
	auto* Grid = CurrentTarget.IsValid() ? Cast<UCharacterSelectGridWidget>(CurrentTarget->Object.Get()) : nullptr;
	auto* Layout = Cast<UCharacterSelectLayoutData>(Assets[1].Get());
	if (!Grid || !Layout || !GEditor || GEditor->PlayWorld) { Message = LOCTEXT("SyncTargets", "Select a grid target and Layout Data Asset first."); return FReply::Handled(); }
	const FScopedTransaction Transaction(LOCTEXT("SyncTransaction", "Sync Character Selection Layout"));
	Layout->Modify();
	const auto& Value = Grid->GetGridLayout();
	Layout->Columns = Value.Columns; Layout->Rows = Value.Rows; Layout->SlotSize = Value.SlotSize; Layout->SlotSpacing = Value.SlotSpacing; Layout->GridPadding = Value.Padding;
	Layout->MarkPackageDirty();
	RefreshLivePreview();
	Message = LOCTEXT("Synced", "Copied grid dimensions to the Layout Data Asset. Placement stays on the widget. Save to persist.");
	return FReply::Handled();
}

namespace
{
	UObject* LauncherReference(UBlueprint* BP, const TCHAR* Name)
	{
		auto* CDO = BP && BP->GeneratedClass ? Cast<ACharacterSelectTestController>(BP->GeneratedClass->GetDefaultObject()) : nullptr;
		auto* Property = CDO ? FindFProperty<FObjectPropertyBase>(CDO->GetClass(), Name) : nullptr;
		return Property ? Property->GetObjectPropertyValue_InContainer(CDO) : nullptr;
	}
}

FString SCharacterSelectionEditor::GetLauncherMismatch() const
{
	auto* Launcher = Cast<UBlueprint>(Assets[6].Get());
	if (!Launcher) return FString();
	auto* Screen = Cast<UWidgetBlueprint>(Assets[0].Get());
	if (LauncherReference(Launcher, TEXT("CharacterSelectWidgetClass")) != (Screen ? Screen->GeneratedClass : nullptr)
		|| LauncherReference(Launcher, TEXT("CharacterLayout")) != Assets[1].Get()
		|| LauncherReference(Launcher, TEXT("CharacterRegistry")) != Assets[2].Get())
		return TEXT("Studio and launcher use different assets. In Setup, click Use launcher's screen, roster & layout, or update the launcher's defaults to your intended setup. Check placed actor overrides too.");
	return FString();
}

FReply SCharacterSelectionEditor::AddPresentationHost()
{
    if (!GEditor || GEditor->PlayWorld || !CurrentTarget.IsValid()) return FReply::Handled();
    auto* Preview = Cast<UCharacterSelectPreviewWidget>(CurrentTarget->Object.Get());
    auto* BP = Preview ? Cast<UWidgetBlueprint>(Preview->GetClass()->ClassGeneratedBy) : nullptr;
    if (!BP || !BP->WidgetTree) return FReply::Handled();
    if (BP->WidgetTree->FindWidget(TEXT("CustomPresentationSlot")))
    { Message = LOCTEXT("HostExists", "CustomPresentationSlot already exists. Open its Designer to adjust its layout."); return FReply::Handled(); }
    auto* Portrait = BP->WidgetTree->FindWidget(TEXT("LargePortrait"));
    auto* Canvas = Portrait ? Cast<UCanvasPanel>(Portrait->GetParent()) : nullptr;
    auto* Layout = Portrait ? Cast<UCanvasPanelSlot>(Portrait->Slot) : nullptr;
    if (!Canvas || !Layout)
    { Message = LOCTEXT("HostManual", "This portrait uses a custom panel hierarchy. In its preview Blueprint Designer, add a Named Slot called CustomPresentationSlot over the portrait region. Keep LargePortrait for fallback artwork and transitions."); return FReply::Handled(); }
    const FScopedTransaction Transaction(LOCTEXT("AddHostTransaction", "Add character presentation host"));
    BP->Modify(); BP->WidgetTree->Modify(); Canvas->Modify();
    auto* Host = BP->WidgetTree->ConstructWidget<UNamedSlot>(UNamedSlot::StaticClass(), TEXT("CustomPresentationSlot"));
    Host->SetFlags(RF_Transactional);
    BP->OnVariableAdded(Host->GetFName());
    auto* HostSlot = Canvas->AddChildToCanvas(Host);
    HostSlot->SetFlags(RF_Transactional);
    HostSlot->SetLayout(Layout->GetLayout());
    HostSlot->SetZOrder(Layout->GetZOrder());
    Host->SetVisibility(ESlateVisibility::Collapsed);
    FBlueprintEditorUtils::MarkBlueprintAsStructurallyModified(BP);
    EditedAssets.AddUnique(BP);
    FKismetEditorUtilities::CompileBlueprint(BP);
    RebuildTargets();
    Message = LOCTEXT("HostAdded", "Presentation host added. Save the preview Blueprint, then restart Play. Existing portrait artwork and animation bindings were retained.");
    return FReply::Handled();
}

FReply SCharacterSelectionEditor::UseLauncherAssets()
{
	if (!GEditor || GEditor->PlayWorld) return FReply::Handled();
	auto* Launcher = Cast<UBlueprint>(Assets[6].Get());
	auto* ScreenClass = Cast<UClass>(LauncherReference(Launcher, TEXT("CharacterSelectWidgetClass")));
	auto* Screen = ScreenClass ? Cast<UWidgetBlueprint>(ScreenClass->ClassGeneratedBy) : nullptr;
	auto* Layout = Cast<UCharacterSelectLayoutData>(LauncherReference(Launcher, TEXT("CharacterLayout")));
	auto* Registry = Cast<UCharacterSelectRegistry>(LauncherReference(Launcher, TEXT("CharacterRegistry")));
	if (!Screen || !Layout || !Registry)
	{
		Message = LOCTEXT("IncompleteLauncher", "Choose a compiled CharacterSelectTestController Blueprint with a screen Widget Blueprint, layout and registry assigned in its defaults.");
		return FReply::Handled();
	}
	Assets[0].Reset(Screen); Assets[1].Reset(Layout); Assets[2].Reset(Registry);
	Assets[4].Reset(); // Edit the launcher's embedded grid, not a separate component.
	SaveSelections(); RebuildTargets(); RefreshLivePreview();
	Message = LOCTEXT("LauncherLoaded", "Studio now uses the launcher's screen, roster and layout. Customize Grid layout, then save. Check placed actor overrides if Play still differs.");
	return FReply::Handled();
}

FReply SCharacterSelectionEditor::Validate()
{
	TArray<FString> Issues;
	const FString Mismatch = GetLauncherMismatch();
	if (!Mismatch.IsEmpty()) Issues.Add(Mismatch);
	if (auto* LauncherBP = Cast<UBlueprint>(Assets[6].Get()))
	{
		auto* Launcher = LauncherBP->GeneratedClass ? Cast<ACharacterSelectTestController>(LauncherBP->GeneratedClass->GetDefaultObject()) : nullptr;
		if (!Launcher) Issues.Add(TEXT("Launcher must derive from CharacterSelectTestController."));
		else if (Launcher->bTravelOnReady)
		{
			if (!Launcher->bStoreSelectionOnReady) Issues.Add(TEXT("Automatic travel requires Store Selection On Ready."));
			if (Launcher->MatchLevel.IsNull() || !FPackageName::DoesPackageExist(Launcher->MatchLevel.ToSoftObjectPath().GetLongPackageName()))
				Issues.Add(TEXT("Assign an existing Match Level on Match handoff; include that map in packaged builds."));
		}
	}
	auto* Screen = Cast<UCharacterSelectWidget>(CharacterSelectionHub::Defaults(Assets[0].Get()));
	auto* Layout = Cast<UCharacterSelectLayoutData>(Assets[1].Get());
	auto* Registry = Cast<UCharacterSelectRegistry>(Assets[2].Get());
	if (!Screen) Issues.Add(TEXT("Select a Screen Blueprint derived from CharacterSelectWidget for player/control/prompt settings."));
	if (!Layout) Issues.Add(TEXT("Select the runtime Layout Data Asset."));
	if (!Registry) Issues.Add(TEXT("Select the runtime Character Registry."));
	if (Layout) Issues.Append(Layout->GetDirectionalLinkIssues(Registry));
	if (Layout && !UCharacterSelectNavigationProvider::IsModeSupported(Layout->NavigationMode)) Issues.Add(TEXT("Navigation mode is unsupported; choose Grid, Horizontal, or Vertical."));
	if (Screen)
	{
		if (Screen->SoundSettings.Volume < 0 || Screen->SoundSettings.Volume > 2 || Screen->SoundSettings.Pitch < 0.1f || Screen->SoundSettings.Pitch > 4 || Screen->HintSettings.IconSize < 8 || Screen->HintSettings.IconSize > 128)
			Issues.Add(TEXT("Feedback requires volume 0–2, pitch 0.1–4 and icon size 8–128. Check Sounds & button hints."));
		if (Screen->PlayerConfigs.Num() != 2) Issues.Add(TEXT("Exactly two player configurations are required."));
		const auto& Controls = Screen->ControlSettings;
		if (Controls.ReleaseThreshold > Controls.EngageThreshold) Issues.Add(TEXT("Stick release threshold should not exceed engagement threshold."));
		TSet<FKey> Keys;
		for (const auto* Binding : {&Controls.Up, &Controls.Down, &Controls.Left, &Controls.Right, &Controls.Confirm, &Controls.Cancel})
		{
			if (Binding->IsEmpty()) Issues.Add(TEXT("One or more input actions have no keys. Check Controls."));
			for (const auto& Key : *Binding)
			{
				if (Keys.Contains(Key)) Issues.Add(TEXT("Duplicate action binding: ") + Key.ToString());
				if (!Key.IsValid() || Key.IsAnalog() || Key.IsMouseButton() || Key.IsModifierKey() || Key == EKeys::F11 || Key == EKeys::Tilde) Issues.Add(TEXT("Unsupported/reserved binding: ") + Key.ToString());
				Keys.Add(Key);
			}
		}
	}
	if (Registry)
	{
		TSet<FName> IDs;
		if (Registry->Characters.IsEmpty()) Issues.Add(TEXT("The registry has no characters."));
		for (auto* Character : Registry->GetAllCharacters())
		{
			if (!Character) { Issues.Add(TEXT("The registry contains an empty character reference.")); continue; }
			if (Character->CharacterID.IsNone() || IDs.Contains(Character->CharacterID)) Issues.Add(TEXT("Missing/duplicate Character ID: ") + Character->GetName());
			IDs.Add(Character->CharacterID);
			if (!Character->Portrait) Issues.Add(TEXT("Missing slot portrait: ") + Character->GetName());
		}
	}
	for (const auto& Asset : {Assets[0].Get(), Assets[4].Get()})
	{
		auto* Blueprint = Cast<UWidgetBlueprint>(Asset);
		if (!Blueprint) continue;
		auto Check = [&Issues](UObject* Object)
		{
			if (auto* Grid = Cast<UCharacterSelectGridWidget>(Object))
			{
				const auto& L = Grid->GetGridLayout();
				if (L.Columns < 1 || L.SlotSize.X <= 0 || L.SlotSize.Y <= 0 || L.SlotSpacing.X < 0 || L.SlotSpacing.Y < 0) Issues.Add(TEXT("Invalid grid dimensions on ") + Grid->GetName());
			}
			if (auto* Preview = Cast<UCharacterSelectPreviewWidget>(Object))
			{
				if (!Preview->AnimationStyle.IsNone() && !Preview->AnimationStyles.Contains(Preview->AnimationStyle)) Issues.Add(TEXT("Unknown active animation style on ") + Preview->GetName());
				for (const auto& Pair : Preview->AnimationStyles)
				{
					if (Pair.Value.PlaybackSpeed <= 0) Issues.Add(TEXT("Animation speed must be positive: ") + Pair.Key.ToString());
					for (FName Name : {Pair.Value.OutAnimationName, Pair.Value.InAnimationName})
					{
						const auto* Property = FindFProperty<FObjectPropertyBase>(Preview->GetClass(), Name);
						if (!Name.IsNone() && (!Property || !Property->PropertyClass->IsChildOf(UWidgetAnimation::StaticClass()))) Issues.Add(TEXT("Missing preview animation: ") + Name.ToString());
					}
				}
			}
		};
		Check(CharacterSelectionHub::Defaults(Blueprint));
		if (Blueprint->WidgetTree) Blueprint->WidgetTree->ForEachWidget([&Check](UWidget* Widget) { Check(Widget); });
	}
    TArray<UCharacterSelectData*> Characters;
    if (Registry) Characters = Registry->GetAllCharacters();
    if (auto* Selected = Cast<UCharacterSelectData>(Assets[3].Get())) Characters.AddUnique(Selected);
    for (auto* Character : Characters) if (Character && Character->bOverridePresentation2D)
        for (const FString& Issue : Character->Presentation2D.GetIssues()) Issues.Add(Character->GetName() + TEXT(": ") + Issue);
    for (auto* Character : Characters) if (Character && Character->bOverridePresentation3D)
        for (const FString& Issue : Character->Presentation3D.GetIssues()) Issues.Add(Character->GetName() + TEXT(": ") + Issue);
    auto CheckPreview2D = [&](UCharacterSelectPreviewWidget* Preview)
    {
        if (!Preview) return;
        for (const FString& Issue : Preview->DefaultPresentation2D.GetIssues()) Issues.Add(Preview->GetName() + TEXT(": ") + Issue);
        if (Preview->bDefault3DPreview) for (const FString& Issue : Preview->DefaultPresentation3D.GetIssues()) Issues.Add(Preview->GetName() + TEXT(": ") + Issue);
        bool bNeeds3DHost = Preview->bDefault3DPreview;
        for (auto* Character : Characters) if (Character)
        {
            const bool b3D = Character->PreviewMode == ECharacterPreviewMode::Actor3D || (Character->PreviewMode == ECharacterPreviewMode::Inherit && Preview->bDefault3DPreview);
            bNeeds3DHost |= b3D;
            if (b3D && !Character->bOverridePresentation3D) for (const FString& Issue : Preview->DefaultPresentation3D.GetIssues()) Issues.Add(Character->GetName() + TEXT(" / ") + Preview->GetName() + TEXT(": ") + Issue);
        }
        bool bNeedsHost = bNeeds3DHost || Preview->DefaultPresentation2D.Mode == ECharacter2DPresentationMode::CustomWidget;
        for (auto* Character : Characters) if (Character && Character->bOverridePresentation2D && Character->Presentation2D.Mode == ECharacter2DPresentationMode::CustomWidget) bNeedsHost = true;
        auto* PreviewBP = Cast<UWidgetBlueprint>(Preview->GetClass()->ClassGeneratedBy);
        if (bNeedsHost && (!PreviewBP || !PreviewBP->WidgetTree || !Cast<UNamedSlot>(PreviewBP->WidgetTree->FindWidget(TEXT("CustomPresentationSlot")))))
            Issues.Add(Preview->GetName() + TEXT(": add a Named Slot named CustomPresentationSlot to its preview Blueprint for custom or 3D widgets. Artwork is used as fallback until connected."));
    };
    if (auto* ScreenBP = Cast<UWidgetBlueprint>(Assets[0].Get())) if (ScreenBP->WidgetTree)
        ScreenBP->WidgetTree->ForEachWidget([&](UWidget* W) { CheckPreview2D(Cast<UCharacterSelectPreviewWidget>(W)); });
    CheckPreview2D(Cast<UCharacterSelectPreviewWidget>(CharacterSelectionHub::Defaults(Assets[4].Get())));
	FString Report = Issues.IsEmpty() ? TEXT("Checks passed. ") : FString::Printf(TEXT("Review %d item(s):\n"), Issues.Num());
	for (const auto& Issue : Issues) Report += TEXT("• ") + Issue + TEXT("\n");
	Report += TEXT("Verify your game uses these exact assets. Check layout, input hints and animation playback in Play mode.");
	Message = FText::FromString(Report);
	return FReply::Handled();
}
FReply SCharacterSelectionEditor::OpenLivePreview()
{
	if (auto Window = PreviewWindow.Pin()) { Window->BringToFront(); RefreshLivePreview(); return FReply::Handled(); }
	LivePreview = SNew(SCharacterSelectLivePreview);
	auto Window = SNew(SWindow).Title(LOCTEXT("PreviewTitle", "Character Selection — Layout & animation preview")).ClientSize(FVector2D(1000, 760))[LivePreview.ToSharedRef()];
	Window->SetOnWindowClosed(FOnWindowClosed::CreateSP(this, &SCharacterSelectionEditor::OnPreviewClosed));
	PreviewWindow = Window;
	FSlateApplication::Get().AddWindow(Window);
	RefreshLivePreview();
	return FReply::Handled();
}

void SCharacterSelectionEditor::OnPreviewClosed(const TSharedRef<SWindow>& Window)
{
	LivePreview.Reset(); PreviewWindow.Reset();
}

void SCharacterSelectionEditor::RefreshLivePreview()
{
	if (LivePreview) LivePreview->SetAssets(Cast<UWidgetBlueprint>(Assets[0].Get()), Cast<UCharacterSelectRegistry>(Assets[2].Get()), Cast<UCharacterSelectLayoutData>(Assets[1].Get()));
}
#undef LOCTEXT_NAMESPACE
