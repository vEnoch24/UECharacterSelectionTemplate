#if WITH_DEV_AUTOMATION_TESTS
#include "Misc/AutomationTest.h"
#include "SCharacterSelectLivePreview.h"
#include "WidgetBlueprint.h"
#include "Blueprint/WidgetTree.h"
#include "UI/CharacterSelectGridWidget.h"
#include "UI/CharacterSelectSlotWidget.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "UObject/UnrealType.h"
#include "Engine/UserInterfaceSettings.h"
#include "Layout/ArrangedChildren.h"
#include "UI/CharacterSelectWidget.h"
#include "UI/CharacterSelectPreviewWidget.h"
#include "Engine/Texture2D.h"
#include "Editor.h"
#include "Widgets/Layout/SBox.h"

namespace
{
	bool FindLayoutGeometry(const TSharedRef<SWidget>& Root, const FGeometry& Geometry, const TSharedPtr<SWidget>& Target, FGeometry& Result)
	{
		if (Root == Target) { Result = Geometry; return true; }
		FArrangedChildren Children(EVisibility::All);
		Root->ArrangeChildren(Geometry, Children);
		for (int32 I = 0; I < Children.Num(); ++I)
			if (FindLayoutGeometry(Children[I].Widget, Children[I].Geometry, Target, Result)) return true;
		return false;
	}
}
#include "Data/CharacterSelectLayoutData.h"
#include "Data/CharacterSelectRegistry.h"
#include "Data/CharacterSelectData.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCharacterSelectLivePreviewTest, "CharacterSelection.Editor.LiveLayoutPreview", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FCharacterSelectLivePreviewTest::RunTest(const FString& Parameters)
{
	FCharacterSelectGridLayout Grid;
	Grid.Padding = FMargin(10,20,30,40);
	FVector2D Anchor, Offset;
	Grid.Placement = ECharacterSelectGridPlacement::BottomRight;
	Grid.GetPlacement(FVector2D(200,100), Anchor, Offset);
	TestTrue(TEXT("Bottom-right shared anchor"), Anchor == FVector2D(1,1));
	TestTrue(TEXT("Bottom-right shared offset"), Offset == FVector2D(-230,-140));
	Grid.Placement = ECharacterSelectGridPlacement::Center;
	Grid.GetPlacement(FVector2D(200,100), Anchor, Offset);
	TestTrue(TEXT("Center ignores edge padding as runtime does"), Anchor == FVector2D(0.5,0.5) && Offset == FVector2D(-100,-50));
	TStrongObjectPtr<UCharacterSelectRegistry> Registry(NewObject<UCharacterSelectRegistry>());
	TStrongObjectPtr<UCharacterSelectLayoutData> Layout(NewObject<UCharacterSelectLayoutData>());
	for (int32 I = 0; I < 3; ++I)
	{
		auto* Character = NewObject<UCharacterSelectData>(Registry.Get());
		Character->CharacterID = FName(*FString::Printf(TEXT("Preview%d"), I));
		Registry->Characters.Add(Character);
	}
	Registry->Characters[1]->Availability = ECharacterAvailability::Locked;
	Registry->Characters[2]->Availability = ECharacterAvailability::Hidden;
	auto* BP = LoadObject<UWidgetBlueprint>(nullptr, TEXT("/Game/CharacterSelect/UI/Core/WBP_CharacterSelect.WBP_CharacterSelect"));
	if (!TestNotNull(TEXT("Starter screen"), BP)) return false;
	const bool bDirty = BP->GetOutermost()->IsDirty();
	auto Preview = SNew(SCharacterSelectLivePreview);
	Preview->SetAssets(BP, Registry.Get(), Layout.Get());
	if (!TestTrue(TEXT("Preview controllers initialized"), Preview->Controllers[0].IsValid())) return false;
	if (!TestTrue(TEXT("Actual screen tree is rendered"), Preview->PreviewOwner.IsValid() && Preview->PreviewContent.IsValid() && Preview->PreviewGrid.IsValid())) return false;
	TestEqual(TEXT("Actual grid creates slots before first tick"), Preview->PreviewGrid->GetCharacterCount(), 2);
	// Inspect rendered Slate visibility, not just UWidget properties: designer
	// synchronization can make a collapsed indicator visible on the actual screen.
	auto CheckRenderedIndicators = [&]()
	{
		Preview->DrawRoster();
		int32 CheckedSlots = 0;
        Preview->PreviewGrid->WidgetTree->ForEachWidget([&](UWidget* Widget)
		{
			if (auto* Slot = Cast<UCharacterSelectSlotWidget>(Widget))
			{
				++CheckedSlots;
                // Stopped authored pose evaluators are retained, but do not play.
				const TCHAR* Names[] = {TEXT("Player1FocusBorder"), TEXT("Player2FocusBorder"), TEXT("BothPlayersFocusBorder"), TEXT("Player1SelectedBorder"), TEXT("Player2SelectedBorder"), TEXT("LockOverlay")};
				const bool Expected[] = {Slot->IsFocusedForPlayer(0), Slot->IsFocusedForPlayer(1), Slot->IsOccupiedByBothPlayers(), Slot->IsSelectedForPlayer(0), Slot->IsSelectedForPlayer(1), Slot->IsLocked()};
				for (int32 I = 0; I < UE_ARRAY_COUNT(Names); ++I)
					if (auto* Indicator = Slot->WidgetTree->FindWidget(Names[I]))
					{
						const auto Rendered = Indicator->GetCachedWidget();
						TestTrue(TEXT("Indicator has a rendered widget"), Rendered.IsValid());
						if (Rendered) TestEqual(FString::Printf(TEXT("%s rendered state on slot %d"), Names[I], Slot->GetRosterIndex()), Rendered->GetVisibility().IsVisible(), Expected[I]);
					}
			}
		});
        TestEqual(TEXT("Every rendered slot was checked"), CheckedSlots, Preview->PreviewGrid->GetCharacterCount());
	};
	CheckRenderedIndicators();
	for (int32 P = 0; P < 2; ++P)
	{
		Preview->Controllers[P]->FocusCharacterAtIndex(0);
		CheckRenderedIndicators();
		Preview->Controllers[P]->SelectCharacter();
		Preview->Controllers[P]->ConfirmSelection();
		CheckRenderedIndicators();
		Preview->Controllers[P]->CancelSelection();
		Preview->Controllers[P]->FocusCharacterAtIndex(1);
		CheckRenderedIndicators();
	}
	Preview->Controllers[0]->FocusCharacterAtIndex(0);
	Preview->Controllers[1]->FocusCharacterAtIndex(0);
	CheckRenderedIndicators();
	Preview->Controllers[0]->SelectCharacter(); Preview->Controllers[0]->ConfirmSelection();
	CheckRenderedIndicators();
	Preview->Controllers[1]->FocusCharacterAtIndex(1);
	CheckRenderedIndicators();
	Preview->Controllers[0]->CancelSelection();
	CheckRenderedIndicators();
	Preview->Refresh();
	CheckRenderedIndicators();
	const int32 AuthoredColumns = Preview->PreviewGrid->GridLayout.Columns;
	auto* Container = Cast<UCanvasPanel>(Preview->PreviewGrid->WidgetTree->FindWidget(TEXT("SlotContainer")));
	if (!TestNotNull(TEXT("Grid canvas binding"), Container) || !TestEqual(TEXT("Slots on real canvas"), Container->GetChildrenCount(), 2)) return false;
	auto* FirstSlot = Cast<UCanvasPanelSlot>(Container->GetChildAt(0)->Slot);
	FVector2D ExpectedAnchor, ExpectedOffset;
	Preview->Grid.GetPlacement(Preview->Grid.GetGridSize(2), ExpectedAnchor, ExpectedOffset);
	const FVector2D FirstCoordinates = Preview->Grid.GetSlotCoordinates(0, 2);
	TestEqual(TEXT("Immediate slot anchors"), FirstSlot->GetAnchors().Minimum, ExpectedAnchor);
	TestEqual(TEXT("Immediate slot offsets"), FirstSlot->GetPosition(), ExpectedOffset + FirstCoordinates * (Preview->Grid.SlotSize + Preview->Grid.SlotSpacing));
	TestEqual(TEXT("Immediate slot size"), FirstSlot->GetSize(), Preview->Grid.SlotSize);
	TestEqual(TEXT("Uses runtime roster filtering"), Preview->Controllers[0]->GetRosterEntries().Num(), 2);
	Preview->Controllers[0]->FocusCharacterAtIndex(1);
	Preview->Controllers[0]->SelectCharacter(); Preview->Controllers[0]->ConfirmSelection();
	TestFalse(TEXT("Locked character cannot confirm"), Preview->Controllers[0]->IsConfirmed());
	Preview->Controllers[0]->FocusCharacterAtIndex(0);
	Preview->Controllers[0]->SelectCharacter(); Preview->Controllers[0]->ConfirmSelection();
	TestTrue(TEXT("Available character confirms"), Preview->Controllers[0]->IsConfirmed());
	TestFalse(TEXT("Players have independent preview state"), Preview->Controllers[1]->IsConfirmed());
	Layout->NavigationMode = ECharacterSelectNavigationMode::Vertical;
	Preview->Refresh();
	TestEqual(TEXT("Vertical normalization"), Preview->Grid.Columns, 1);
	TestEqual(TEXT("Vertical mode preserves authored columns"), Preview->PreviewGrid->GridLayout.Columns, AuthoredColumns);
	Layout->NavigationMode = ECharacterSelectNavigationMode::Grid;
	Preview->Controllers[0]->Initialize(Registry.Get(), Layout.Get(), 0);
	Preview->PreviewGrid->RefreshLayout();
	TestEqual(TEXT("Switch back to grid restores authored columns on same widget"), Preview->PreviewGrid->GetEffectiveGridLayout().Columns, AuthoredColumns);
	Layout->NavigationMode = ECharacterSelectNavigationMode::Vertical;
	Preview->Refresh();
	TestFalse(TEXT("Refresh resets state"), Preview->Controllers[0]->IsConfirmed());
	Preview->Controllers[0]->FocusCharacterAtIndex(0);
	Preview->Move(ECharacterSelectDirection::Down);
	TestEqual(TEXT("Preview uses runtime navigation"), Preview->Controllers[0]->GetCurrentIndex(), 1);
	Layout->bUseDirectionalLinks = true;
	auto& Link = Layout->DirectionalLinks.Add(TEXT("Preview0")).Up;
	Link.Rule = ECharacterSelectLinkRule::Character; Link.TargetCharacterID = TEXT("Preview1");
	Preview->Refresh(); Preview->Controllers[0]->FocusCharacterAtIndex(0); Preview->Move(ECharacterSelectDirection::Up);
	TestEqual(TEXT("Preview honors directional overrides"), Preview->Controllers[0]->GetCurrentIndex(), 1);
	TestEqual(TEXT("Preview leaves screen unchanged"), BP->GetOutermost()->IsDirty(), bDirty);
	TestEqual(TEXT("Preview leaves layout dimensions unchanged"), Layout->Columns, 7);
	Preview->Resolution = FVector2D(1,1);
	Preview->DrawRoster();
	TestTrue(TEXT("Overflow is reported for a small canvas"), Preview->bClipped);
	Layout->bAllowLockedSlots = false;
	Preview->Refresh();
	TestEqual(TEXT("Locked-slot filtering refreshes"), Preview->Controllers[0]->GetRosterEntries().Num(), 1);
	TestFalse(TEXT("Preview reports unavailable link target"), Preview->LinkWarnings.IsEmpty());
	auto* Launcher = LoadObject<UBlueprint>(nullptr, TEXT("/Game/CharacterSelect/Generated/BP_DefaultSelection_Launcher.BP_DefaultSelection_Launcher"));
	if (Launcher && Launcher->GeneratedClass)
	{
		auto* CDO = Launcher->GeneratedClass->GetDefaultObject();
		auto Ref = [CDO](const TCHAR* Name) { auto* P = FindFProperty<FObjectPropertyBase>(CDO->GetClass(), Name); return P ? P->GetObjectPropertyValue_InContainer(CDO) : nullptr; };
		auto* ScreenClass = Cast<UClass>(Ref(TEXT("CharacterSelectWidgetClass")));
		auto* LauncherScreen = ScreenClass ? Cast<UWidgetBlueprint>(ScreenClass->ClassGeneratedBy) : nullptr;
		auto* LauncherRoster = Cast<UCharacterSelectRegistry>(Ref(TEXT("CharacterRegistry")));
		auto* LauncherLayout = Cast<UCharacterSelectLayoutData>(Ref(TEXT("CharacterLayout")));
		AddInfo(FString::Printf(TEXT("Default launcher: screen=%s roster=%s layout=%s"), *GetPathNameSafe(LauncherScreen), *GetPathNameSafe(LauncherRoster), *GetPathNameSafe(LauncherLayout)));
		Preview->Resolution = FVector2D(1920,1080);
		Preview->SetAssets(LauncherScreen, LauncherRoster, LauncherLayout);
		TestTrue(TEXT("Default launcher screen renders"), Preview->Problem.IsEmpty());
		if (Preview->PreviewGrid.IsValid())
		{
			TestEqual(TEXT("Launcher grid builds all visible slots immediately"), Preview->PreviewGrid->GetCharacterCount(), Preview->Controllers[0]->GetRosterEntries().Num());
			// Independently instantiate the compiled runtime class. Compare arranged
			// slot geometry to Studio's editable-tree rendering at multiple DPIs.
			TStrongObjectPtr<UUserWidget> Runtime(NewObject<UUserWidget>(GEditor->GetEditorWorldContext().World(), ScreenClass));
			Runtime->SetDesignerFlags(EWidgetDesignFlags::Designing);
			Runtime->Initialize();
			UCharacterSelectGridWidget* RuntimeGrid = nullptr;
			Runtime->WidgetTree->ForEachWidget([&](UWidget* W) { if (auto* G = Cast<UCharacterSelectGridWidget>(W)) RuntimeGrid = G; });
			if (TestNotNull(TEXT("Compiled launcher grid"), RuntimeGrid))
			{
				RuntimeGrid->InitializeGrid({Preview->Controllers[0].Get(), Preview->Controllers[1].Get()});
				RuntimeGrid->WidgetTree->ForEachWidget([](UWidget* W)
                {
                    if (auto* S = Cast<UCharacterSelectSlotWidget>(W)) S->SetStudioAnimationPreview(false);
                });
                auto RuntimeContent = Runtime->WidgetTree->RootWidget->TakeWidget();
                RuntimeGrid->WidgetTree->ForEachWidget([](UWidget* W)
                {
                    if (auto* S = Cast<UCharacterSelectSlotWidget>(W)) S->SyncStudioVisualState();
                });
				auto* RuntimeCanvas = Cast<UCanvasPanel>(RuntimeGrid->WidgetTree->FindWidget(TEXT("SlotContainer")));
				auto* PreviewCanvas = Cast<UCanvasPanel>(Preview->PreviewGrid->WidgetTree->FindWidget(TEXT("SlotContainer")));
				if (RuntimeCanvas && PreviewCanvas && RuntimeCanvas->GetChildrenCount() && PreviewCanvas->GetChildrenCount())
				{
					for (FVector2D Size : {FVector2D(1280,720), FVector2D(1920,1080), FVector2D(1024,768)})
					{
						Preview->Resolution = Size; Preview->DrawRoster();
						const float DPI = GetDefault<UUserInterfaceSettings>()->GetDPIScaleBasedOnSize(FIntPoint(Size.X,Size.Y));
						Preview->Stage->SlatePrepass(1.f); RuntimeContent->SlatePrepass(DPI);
						FGeometry StudioSlotGeometry, RuntimeSlotGeometry;
						const bool StudioFound = FindLayoutGeometry(Preview->Stage.ToSharedRef(), FGeometry::MakeRoot(Size, FSlateLayoutTransform(1.f)), PreviewCanvas->GetChildAt(0)->GetCachedWidget(), StudioSlotGeometry);
						const bool RuntimeFound = FindLayoutGeometry(RuntimeContent, FGeometry::MakeRoot(Size / DPI, FSlateLayoutTransform(DPI)), RuntimeCanvas->GetChildAt(0)->GetCachedWidget(), RuntimeSlotGeometry);
						TestTrue(TEXT("Actual slot geometry found in both hierarchies"), StudioFound && RuntimeFound);
						if (StudioFound && RuntimeFound)
						{
							TestTrue(TEXT("Studio and runtime pixel positions match"), StudioSlotGeometry.GetAbsolutePosition().Equals(RuntimeSlotGeometry.GetAbsolutePosition(), .1f));
							TestTrue(TEXT("Studio and runtime slot sizes match"), StudioSlotGeometry.GetLocalSize().Equals(RuntimeSlotGeometry.GetLocalSize(), .1f));
						}
					}
				}
			}

			if (auto* ParentSlot = Cast<UCanvasPanelSlot>(Preview->PreviewGrid->Slot))
				AddInfo(FString::Printf(TEXT("Launcher grid parent: anchors=%s to %s offsets=%s; columns=%d placement=%d"), *ParentSlot->GetAnchors().Minimum.ToString(), *ParentSlot->GetAnchors().Maximum.ToString(), *ParentSlot->GetPosition().ToString(), Preview->Grid.Columns, int32(Preview->Grid.Placement)));
		}
	}
    // Screen assets may initially hide either presentation until a player joins.
    // Studio simulates active players, so both wrappers must be revealed even
    // though gameplay join initialization is deliberately not run here.
    auto CheckPortraitWrappers = [&]()
    {
        for (const TCHAR* Name : {TEXT("Player1Preview"), TEXT("Player2Preview")})
        {
            auto* P = Cast<UCharacterSelectPreviewWidget>(Preview->PreviewOwner->WidgetTree->FindWidget(Name));
            if (!TestNotNull(TEXT("Player presentation wrapper exists"), P)) continue;
            for (ESlateVisibility Initial : {ESlateVisibility::Hidden, ESlateVisibility::Collapsed})
            {
                P->SetVisibility(Initial);
                Preview->DrawRoster();
                TestTrue(FString::Printf(TEXT("%s is visible after preview refresh"), Name), P->GetCachedWidget().IsValid() && P->GetCachedWidget()->GetVisibility().IsVisible());
                Preview->SyncVisibility();
                TestTrue(TEXT("Visibility synchronization preserves the active portrait"), P->GetVisibility() == ESlateVisibility::HitTestInvisible);
            }
        }
    };
    CheckPortraitWrappers();
    auto* AnimatedCharacter = Registry->Characters[0].Get();
    AnimatedCharacter->bOverrideSequence = true;
    AnimatedCharacter->Sequence = FCharacterSelectSequence::MakePreset(0);
    AnimatedCharacter->bOverridePresentation2D = true;
    AnimatedCharacter->Presentation2D.Mode = ECharacter2DPresentationMode::SpriteFrames;
    AnimatedCharacter->Presentation2D.FramesPerSecond = 10.f;
    AnimatedCharacter->Presentation2D.Frames = {NewObject<UTexture2D>(Registry.Get()), NewObject<UTexture2D>(Registry.Get())};
    Preview->bAnimate = true; Preview->bPlaying = false;
    Preview->SetAssets(BP, Registry.Get(), Layout.Get());
    CheckPortraitWrappers();
    auto* Portrait = Cast<UCharacterSelectPreviewWidget>(Preview->PreviewOwner->WidgetTree->FindWidget(TEXT("Player1Preview")));
    if (TestNotNull(TEXT("Animated preview portrait exists"), Portrait))
    {
        TestTrue(TEXT("Native animation keeps designer isolation"), Portrait->IsDesignTime());
        auto* AnimatedImage = Portrait->WidgetTree->FindWidget(TEXT("LargePortrait"));
        TestTrue(TEXT("Entrance applies its starting opacity"), AnimatedImage && AnimatedImage->GetRenderOpacity() < .01f);
        TestEqual(TEXT("Sequence starts entrance in Studio"), Portrait->GetStudioAnimationStatus(), FString(TEXT("Entrance")));
        auto* SlotCanvas = Cast<UCanvasPanel>(Preview->PreviewGrid->WidgetTree->FindWidget(TEXT("SlotContainer")));
        auto* AnimatedSlot = SlotCanvas && SlotCanvas->GetChildrenCount() ? Cast<UCharacterSelectSlotWidget>(SlotCanvas->GetChildAt(0)) : nullptr;
        if (AnimatedSlot)
        {
            AnimatedSlot->SlotAnimationSettings = FCharacterSlotAnimationSettings::MakePreset(ECharacterSlotAnimationPreset::Pop);
            AnimatedSlot->SetStudioAnimationPreview(true);
        }
        const FVector2D InitialSlotScale = AnimatedSlot ? AnimatedSlot->GetRenderTransform().Scale : FVector2D::ZeroVector;
        Preview->AdvancePlayback(.1f);
        TestEqual(TEXT("Pause holds playback clock"), Preview->PlaybackSeconds, 0.);
        Preview->AdvancePlayback(1.f/60.f, true);
        TestTrue(TEXT("Stepping advances nested slot animations"), AnimatedSlot && !AnimatedSlot->GetRenderTransform().Scale.Equals(InitialSlotScale, .000001));
        TestTrue(TEXT("Step advances one frame"), FMath::IsNearlyEqual(Preview->PlaybackSeconds, 1./60., .00001));
        Preview->bPlaying = true;
        Preview->PlaybackSpeed = .5f;
        const double Before = Preview->PlaybackSeconds;
        Preview->AdvancePlayback(.1f);
        TestTrue(TEXT("Playback speed scales time"), FMath::IsNearlyEqual(Preview->PlaybackSeconds - Before, .05, .00001));
        Preview->PlaybackSpeed = 1.f;
        for (int32 Frame=0; Frame<10; ++Frame) Preview->AdvancePlayback(.1f);
        TestEqual(TEXT("Studio sequence reaches ready"), Portrait->GetStudioAnimationStatus(), FString(TEXT("Ready")));
        TestTrue(TEXT("Stepping updates actual portrait opacity"), AnimatedImage && AnimatedImage->GetRenderOpacity() > .5f);
        Preview->Controllers[0]->SelectCharacter(); Preview->Controllers[0]->ConfirmSelection(); Preview->DrawRoster();
        Preview->AdvancePlayback(.1f);
        TestEqual(TEXT("Studio confirm reaches sequence"), Portrait->GetStudioAnimationStatus(), FString(TEXT("Confirmed")));
        Preview->Controllers[0]->CancelSelection(); Preview->DrawRoster();
        TestEqual(TEXT("Studio cancel returns to ready"), Portrait->GetStudioAnimationStatus(), FString(TEXT("Ready")));
        Preview->bCleared[0] = true; Preview->DrawRoster();
        for (int32 Frame=0; Frame<10; ++Frame) Preview->AdvancePlayback(.1f);
        TestNull(TEXT("Clear preview removes the character after exit"), Portrait->GetCharacter());
    }
    Preview->Refresh();
    TestEqual(TEXT("Reset clears playback clock"), Preview->PlaybackSeconds, 0.);
    TestEqual(TEXT("Playback leaves source screen dirty state unchanged"), BP->GetOutermost()->IsDirty(), bDirty);
	Preview->SetAssets(nullptr, nullptr, nullptr);
	TestFalse(TEXT("Missing assets clear stale controllers"), Preview->Controllers[0].IsValid());
	TestFalse(TEXT("Missing assets supply guidance"), Preview->Problem.IsEmpty());
	return true;
}
#endif
