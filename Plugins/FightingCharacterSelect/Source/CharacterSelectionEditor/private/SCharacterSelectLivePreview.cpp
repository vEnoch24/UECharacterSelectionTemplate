#include "SCharacterSelectLivePreview.h"
#include "WidgetBlueprint.h"
#include "Blueprint/WidgetTree.h"
#include "UI/CharacterSelectWidget.h"
#include "UI/CharacterSelectGridWidget.h"
#include "UI/CharacterSelectPreviewWidget.h"
#include "UI/CharacterSelectSlotWidget.h"
#include "Widgets/Layout/SWrapBox.h"
#include "Widgets/Layout/SGridPanel.h"
#include "Data/CharacterSelectRegistry.h"
#include "Data/CharacterSelectLayoutData.h"
#include "Data/CharacterSelectData.h"
#include "Engine/Texture2D.h"
#include "Editor.h"
#include "UObject/UObjectGlobals.h"
#include "UObject/UnrealType.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Layout/SDPIScaler.h"
#include "Engine/UserInterfaceSettings.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SScaleBox.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Images/SImage.h"

namespace
{
    // UE's ForEachWidgetAndDescendants visits the contents of nested user widgets,
    // but skips their wrappers. Playback needs both the wrapper and its children.
    void VisitPreviewWidgets(UWidgetTree* Root, TFunctionRef<void(UWidget*)> Visitor)
    {
        TArray<UWidgetTree*> Pending; Pending.Add(Root);
        TSet<UWidgetTree*> SeenTrees;
        TSet<UWidget*> SeenWidgets;
        TArray<UWidget*> Widgets;
        while (!Pending.IsEmpty())
        {
            UWidgetTree* Tree = Pending.Pop();
            if (!Tree || SeenTrees.Contains(Tree)) continue;
            SeenTrees.Add(Tree);
            Tree->ForEachWidget([&](UWidget* Widget)
            {
                if (!Widget || SeenWidgets.Contains(Widget)) return;
                SeenWidgets.Add(Widget); Widgets.Add(Widget);
                if (auto* User = Cast<UUserWidget>(Widget)) Pending.Add(User->WidgetTree);
            });
        }
        for (UWidget* Widget : Widgets) Visitor(Widget);
    }
}

void SCharacterSelectLivePreview::Construct(const FArguments& Args)
{
	TSharedRef<SHorizontalBox> Controls = SNew(SHorizontalBox);
	auto Button = [&Controls](const TCHAR* Label, FOnClicked Click)
	{
		Controls->AddSlot().AutoWidth().VAlign(VAlign_Center).Padding(3)[SNew(SButton).Text(FText::FromString(Label)).OnClicked(Click)];
	};
	Button(TEXT("Player 1"), FOnClicked::CreateLambda([this] { ActivePlayer = 0; DrawRoster(); return FReply::Handled(); }));
	Button(TEXT("Player 2"), FOnClicked::CreateLambda([this] { ActivePlayer = 1; DrawRoster(); return FReply::Handled(); }));
    auto DPad = SNew(SGridPanel);
    auto DirectionButton = [this, DPad](const TCHAR* Label, int32 Column, int32 Row, ECharacterSelectDirection Direction)
    {
        DPad->AddSlot(Column, Row).Padding(2)
        [SNew(SBox).MinDesiredWidth(64)[SNew(SButton).HAlign(HAlign_Center)
            .Text(FText::FromString(Label)).OnClicked_Lambda([this, Direction] { return Move(Direction); })]];
    };
    DirectionButton(TEXT("Up"), 1, 0, ECharacterSelectDirection::Up);
    DirectionButton(TEXT("Left"), 0, 1, ECharacterSelectDirection::Left);
    DirectionButton(TEXT("Right"), 2, 1, ECharacterSelectDirection::Right);
    DirectionButton(TEXT("Down"), 1, 2, ECharacterSelectDirection::Down);
    DPad->AddSlot(1, 1).HAlign(HAlign_Center).VAlign(VAlign_Center)
        [SNew(STextBlock).Text_Lambda([this] { return FText::FromString(FString::Printf(TEXT("P%d"), ActivePlayer + 1)); })];
    Controls->AddSlot().AutoWidth().VAlign(VAlign_Center).Padding(12, 4)[DPad];
	Button(TEXT("Confirm"), FOnClicked::CreateLambda([this]
	{
		if (Controllers[ActivePlayer].IsValid()) { bCleared[ActivePlayer] = false; Controllers[ActivePlayer]->SelectCharacter(); Controllers[ActivePlayer]->ConfirmSelection(); DrawRoster(); }
		return FReply::Handled();
	}));
	Button(TEXT("Cancel"), FOnClicked::CreateLambda([this]
	{
		if (Controllers[ActivePlayer].IsValid()) { Controllers[ActivePlayer]->CancelSelection(); DrawRoster(); }
		return FReply::Handled();
	}));
	Button(TEXT("Reset"), FOnClicked::CreateLambda([this] { Refresh(); return FReply::Handled(); }));
    Button(TEXT("Clear preview"), FOnClicked::CreateLambda([this] { bCleared[ActivePlayer] = true; DrawRoster(); return FReply::Handled(); }));
    auto Playback = SNew(SWrapBox).UseAllottedSize(true);
    Playback->AddSlot().Padding(3)[SNew(SButton)
        .Text_Lambda([this] { return FText::FromString(bAnimate ? TEXT("Animation preview: ON") : TEXT("Animation preview: OFF")); })
        .OnClicked_Lambda([this] { bAnimate = !bAnimate; Refresh(); return FReply::Handled(); })];
    Playback->AddSlot().Padding(3)[SNew(SButton)
        .Text_Lambda([this] { return FText::FromString(bPlaying ? TEXT("Pause") : TEXT("Play")); })
        .IsEnabled_Lambda([this] { return bAnimate; })
        .OnClicked_Lambda([this] { bPlaying = !bPlaying; return FReply::Handled(); })];
    Playback->AddSlot().Padding(3)[SNew(SButton).Text(FText::FromString(TEXT("Step 1/60 s")))
        .IsEnabled_Lambda([this] { return bAnimate && !bPlaying; })
        .OnClicked_Lambda([this] { AdvancePlayback(1.f/60.f, true); return FReply::Handled(); })];
    for (float Speed : {.25f, .5f, 1.f, 2.f})
        Playback->AddSlot().Padding(3)[SNew(SButton).Text(FText::FromString(FString::Printf(TEXT("%gx"), Speed)))
            .OnClicked_Lambda([this, Speed] { PlaybackSpeed = Speed; return FReply::Handled(); })];
	TSharedRef<SHorizontalBox> Sizes = SNew(SHorizontalBox);
	for (FVector2D Size : {FVector2D(1280,720), FVector2D(1920,1080), FVector2D(1024,768)})
		Sizes->AddSlot().AutoWidth().Padding(3)[SNew(SButton)
		.Text(FText::FromString(FString::Printf(TEXT("%.0f x %.0f"), Size.X, Size.Y)))
		.OnClicked_Lambda([this, Size] { Resolution = Size; DrawRoster(); return FReply::Handled(); })];
	ChildSlot[SNew(SVerticalBox)
		+ SVerticalBox::Slot().AutoHeight().Padding(8)[SNew(STextBlock).AutoWrapText(true).Text(FText::FromString(TEXT("Enable Animation preview to test configured slot animations, portrait/text sequences, sprite frames and assigned 3D skeletal meshes. Choose a player, navigate, confirm or cancel. Pause and Step inspect motion; Reset replays from the initial focus. Authored slot animations show their settled poses; their motion, custom widget events and actor Blueprint behavior require Play. Asset changes rebuild this temporary preview; source assets are never changed by playback.")))]
		+ SVerticalBox::Slot().AutoHeight()[Sizes]
		+ SVerticalBox::Slot().AutoHeight()[Controls]
        + SVerticalBox::Slot().AutoHeight()[Playback]
		+ SVerticalBox::Slot().AutoHeight().Padding(8)[SNew(STextBlock).AutoWrapText(true).Text(this, &SCharacterSelectLivePreview::GetStatus)]
		+ SVerticalBox::Slot().FillHeight(1).Padding(8)[SNew(SScaleBox).Stretch(EStretch::ScaleToFit)
			[SAssignNew(Stage, SBox).WidthOverride(Resolution.X).HeightOverride(Resolution.Y)]]];
	PropertyHandle = FCoreUObjectDelegates::OnObjectPropertyChanged.AddLambda([this](UObject*, FPropertyChangedEvent&) { bPendingRefresh = true; });
}

SCharacterSelectLivePreview::~SCharacterSelectLivePreview()
{
    ReleasePresentations();
	FCoreUObjectDelegates::OnObjectPropertyChanged.Remove(PropertyHandle);
}

void SCharacterSelectLivePreview::SetAssets(UWidgetBlueprint* Screen, UCharacterSelectRegistry* Registry, UCharacterSelectLayoutData* Layout)
{
	ScreenAsset = Screen; RegistryAsset = Registry; LayoutAsset = Layout; Refresh();
}

void SCharacterSelectLivePreview::Tick(const FGeometry& Geometry, double Time, float Delta)
{
	SCompoundWidget::Tick(Geometry, Time, Delta);
	SetEnabled(!GEditor || !GEditor->PlayWorld);
    if (GEditor && GEditor->PlayWorld)
    {
        if (!bSuspendedForPIE) ReleasePresentations();
        bSuspendedForPIE = true;
        return;
    }
    if (bSuspendedForPIE) { bSuspendedForPIE = false; bPendingRefresh = true; }
    if (bPendingRefresh) Refresh();
    AdvancePlayback(Delta);
}

void SCharacterSelectLivePreview::Refresh()
{
	bPendingRefresh = false;
    ReleasePresentations();
    PlaybackSeconds = 0.; bCleared[0] = bCleared[1] = false;
	Stage->SetContent(SNullWidget::NullWidget);
	PreviewContent.Reset(); PreviewGrid.Reset(); PreviewOwner.Reset();
	Controllers[0].Reset(); Controllers[1].Reset(); Problem.Reset();
	LinkWarnings.Reset();
	if (LayoutAsset.IsValid()) LinkWarnings = FString::Join(LayoutAsset->GetDirectionalLinkIssues(RegistryAsset.Get()),TEXT("\n"));
	auto* BP = ScreenAsset.Get();
	UCharacterSelectGridWidget* Template = nullptr;
	int32 Count = 0;
	if (BP && BP->WidgetTree) BP->WidgetTree->ForEachWidget([&](UWidget* W)
	{
		if (auto* G = Cast<UCharacterSelectGridWidget>(W)) { Template = G; ++Count; }
	});
	if (Count != 1 || !RegistryAsset.IsValid() || !LayoutAsset.IsValid())
		Problem = TEXT("Select a screen with exactly one grid, a registry and a layout in Studio Setup.");
	else
	{
		Grid = Template->GetGridLayout();
		if (Grid.Columns < 1 || Grid.Rows < 0 || Grid.SlotSize.X <= 0 || Grid.SlotSize.Y <= 0 || Grid.SlotSpacing.X < 0 || Grid.SlotSpacing.Y < 0)
			Problem = TEXT("Correct the grid dimensions to preview this setup.");
		else
		{
			auto* Defaults = BP->GeneratedClass ? Cast<UCharacterSelectWidget>(BP->GeneratedClass->GetDefaultObject()) : nullptr;
			for (int32 P = 0; P < 2; ++P)
			{
				Controllers[P].Reset(NewObject<UCharacterSelectionController>());
				Controllers[P]->Initialize(RegistryAsset.Get(), LayoutAsset.Get(), P,
					Defaults && Defaults->PlayerConfigs.IsValidIndex(P) ? Defaults->PlayerConfigs[P].InitialFocusIndex : INDEX_NONE);
			}
			const int32 N = Controllers[0]->GetRosterEntries().Num();
			if (BP->GeneratedClass && GEditor && GEditor->GetEditorWorldContext().World())
			{
				// Render a private designer instance; never initialize the gameplay screen
				// or install its input processor. Copy the editable tree so unsaved layout
				// changes appear without requiring a Blueprint compile.
				PreviewOwner.Reset(NewObject<UUserWidget>(GEditor->GetEditorWorldContext().World(), BP->GeneratedClass, NAME_None, RF_Transient));
				PreviewOwner->SetDesignerFlags(EWidgetDesignFlags::Designing);
				PreviewOwner->DuplicateAndInitializeFromWidgetTree(BP->WidgetTree, {});
				PreviewOwner->Initialize();
				PreviewOwner->WidgetTree->ForEachWidget([this](UWidget* W)
				{
					if (auto* G = Cast<UCharacterSelectGridWidget>(W)) PreviewGrid = G;
				});
				if (PreviewGrid.IsValid() && PreviewOwner->WidgetTree->RootWidget)
				{
					PreviewGrid->InitializeGrid({Controllers[0].Get(), Controllers[1].Get()});
                    VisitPreviewWidgets(PreviewOwner->WidgetTree, [this](UWidget* W)
                    {
                        if (auto* P = Cast<UCharacterSelectPreviewWidget>(W)) P->SetStudioAnimationPreview(bAnimate);
                        if (auto* S = Cast<UCharacterSelectSlotWidget>(W)) S->SetStudioAnimationPreview(bAnimate);
                    });
					Grid = PreviewGrid->GetEffectiveGridLayout();
					PreviewContent = PreviewOwner->WidgetTree->RootWidget->TakeWidget();
				}
				else Problem = TEXT("The screen could not create its grid. Compile the screen and grid Blueprints.");
			}
			else Problem = TEXT("Compile the screen Blueprint before previewing its layout.");
			if (!N) Problem = TEXT("No visible characters. Add available characters to the selected roster.");
		}
	}
	DrawRoster();
}

FReply SCharacterSelectLivePreview::Move(ECharacterSelectDirection Direction)
{
	if (Controllers[ActivePlayer].IsValid()) { bCleared[ActivePlayer] = false; Controllers[ActivePlayer]->MoveSelection(Direction); DrawRoster(); }
	return FReply::Handled();
}

FText SCharacterSelectLivePreview::GetStatus() const
{
	FString Text = FString::Printf(TEXT("Canvas %.0f x %.0f | Controlling Player %d\n"), Resolution.X, Resolution.Y, ActivePlayer+1);
    if (bAnimate) Text += FString::Printf(TEXT("%s | %.2f s | %gx (Step uses 1/60 s)\n"), bPlaying ? TEXT("Playing") : TEXT("Paused"), PlaybackSeconds, PlaybackSpeed);
	if (!LinkWarnings.IsEmpty()) Text += LinkWarnings + TEXT("\n");
	if (!Problem.IsEmpty()) return FText::FromString(Text + Problem);
	for (int32 P = 0; P < 2; ++P) if (Controllers[P].IsValid())
	{
		auto* Character = Controllers[P]->GetFocusedCharacter();
		Text += FString::Printf(TEXT("P%d: %s%s   "), P+1, Character ? *Character->DisplayName.ToString() : TEXT("No focus"), Controllers[P]->IsConfirmed() ? TEXT(" (confirmed)") : TEXT(""));
	}
    if (bAnimate && PreviewOwner.IsValid())
        for (int32 P = 0; P < 2; ++P)
            if (auto* Portrait = Cast<UCharacterSelectPreviewWidget>(PreviewOwner->WidgetTree->FindWidget(P == 0 ? TEXT("Player1Preview") : TEXT("Player2Preview"))))
                Text += FString::Printf(TEXT("\nP%d: %s"), P+1, *Portrait->GetStudioAnimationStatus());
	if (bClipped) Text += TEXT("\nThe unscaled grid is larger than the viewport; check the preview for clipping. Adjust slot size, spacing, placement or roster size.");
	return FText::FromString(Text);
}

void SCharacterSelectLivePreview::DrawRoster()
{
	if (!Stage) return;
	bClipped = false;
	Stage->SetWidthOverride(Resolution.X); Stage->SetHeightOverride(Resolution.Y);
	if (PreviewOwner.IsValid() && PreviewOwner->WidgetTree)
	{
		for (int32 P = 0; P < 2; ++P)
			if (auto* Portrait = Cast<UCharacterSelectPreviewWidget>(PreviewOwner->WidgetTree->FindWidget(P == 0 ? TEXT("Player1Preview") : TEXT("Player2Preview"))))
				if (Controllers[P].IsValid())
                {
                    // Play reveals these wrappers when a player joins. Studio
                    // simulates two active players without creating a join session.
                    // Keep the wrapper visible during Clear's exit animation too.
                    Portrait->SetVisibility(ESlateVisibility::HitTestInvisible);
                    Portrait->SetPresentationPlayerIndex(P);
                    Portrait->SetCharacter(bCleared[P] ? nullptr : Controllers[P]->GetFocusedCharacter());
                    Portrait->SetPreviewSelected(!bCleared[P] && Controllers[P]->IsConfirmed());
                }
		if (PreviewOwner->WidgetTree->FindWidget(TEXT("Player1Preview")))
			for (const TCHAR* Name : {TEXT("FocusedCharacterName"), TEXT("FocusedCharacterImage")})
				if (auto* Legacy = PreviewOwner->WidgetTree->FindWidget(Name)) Legacy->SetVisibility(ESlateVisibility::Collapsed);
	}
    SyncVisibility();
	const float DPIScale = GetDefault<UUserInterfaceSettings>()->GetDPIScaleBasedOnSize(FIntPoint(Resolution.X, Resolution.Y));
	Stage->SetContent(SNew(SBorder).Padding(0).BorderBackgroundColor(FLinearColor(0.035f,0.035f,0.045f))
		.Clipping(EWidgetClipping::ClipToBounds)
		[SNew(SDPIScaler).DPIScale(DPIScale)[PreviewContent.IsValid() ? PreviewContent.ToSharedRef() : SNullWidget::NullWidget]]);
	if (!PreviewGrid.IsValid() || !Controllers[0].IsValid()) return;
	// A conservative early warning; the real hierarchy below handles nested panels,
	// anchors, padding and ScaleBoxes. A grid larger than the viewport needs review.
	const FVector2D GridSize = Grid.GetGridSize(Controllers[0]->GetRosterEntries().Num());
	const FVector2D LogicalSize = Resolution / DPIScale;
	bClipped = GridSize.X > LogicalSize.X || GridSize.Y > LogicalSize.Y;
}

void SCharacterSelectLivePreview::SyncVisibility()
{
	// UMG designer synchronization replaces serialized visibility with
	// GetVisibilityInDesigner (normally Visible). Keep design mode for isolation,
	// but display the real state of every nested widget on the rendered surface.
	if (PreviewOwner.IsValid() && PreviewOwner->WidgetTree)
	{
        VisitPreviewWidgets(PreviewOwner->WidgetTree, [](UWidget* Widget)
        {
            if (auto* Slot = Cast<UCharacterSelectSlotWidget>(Widget)) Slot->SyncStudioVisualState();
        });
        VisitPreviewWidgets(PreviewOwner->WidgetTree, [](UWidget* Widget)
		{
			if (const auto SlateWidget = Widget->GetCachedWidget())
				{
                    static const FProperty* VisibilityProperty = FindFProperty<FProperty>(UWidget::StaticClass(), TEXT("Visibility"));
                    if (VisibilityProperty)
                        SlateWidget->SetVisibility(UWidget::ConvertSerializedVisibilityToRuntime(*VisibilityProperty->ContainerPtrToValuePtr<ESlateVisibility>(Widget)));
                }
		});
    }
}
void SCharacterSelectLivePreview::ReleasePresentations()
{
    if (PreviewOwner.IsValid() && PreviewOwner->WidgetTree)
        VisitPreviewWidgets(PreviewOwner->WidgetTree, [](UWidget* W)
        {
            if (auto* P = Cast<UCharacterSelectPreviewWidget>(W)) P->ReleaseStudioPresentation();
        });
}
void SCharacterSelectLivePreview::AdvancePlayback(float DeltaTime, bool bSingleStep)
{
    if (!bAnimate || (!bPlaying && !bSingleStep) || (GEditor && GEditor->PlayWorld) || !PreviewOwner.IsValid() || !FMath::IsFinite(DeltaTime)) return;
    const float Delta = bSingleStep ? 1.f/60.f : FMath::Clamp(DeltaTime, 0.f, .1f) * PlaybackSpeed;
    PlaybackSeconds += Delta;
    VisitPreviewWidgets(PreviewOwner->WidgetTree, [Delta](UWidget* W)
    {
        if (auto* P = Cast<UCharacterSelectPreviewWidget>(W)) P->AdvanceStudioAnimation(Delta);
        if (auto* S = Cast<UCharacterSelectSlotWidget>(W)) S->AdvanceStudioAnimation(Delta);
    });
    SyncVisibility();
}
