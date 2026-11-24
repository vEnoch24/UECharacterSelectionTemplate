#if WITH_DEV_AUTOMATION_TESTS
#include "Misc/AutomationTest.h"
#include "SCharacterSelectionEditor.h"
#include "WidgetBlueprint.h"
#include "UI/CharacterSelectGridWidget.h"
#include "UI/CharacterSelectPreviewWidget.h"
#include "Data/CharacterSelectLayoutData.h"
#include "Data/CharacterSelectRegistry.h"
#include "Data/CharacterSelectData.h"
#include "Data/CharacterSelectTheme.h"
#include "Editor.h"
#include "Components/TextBlock.h"
#include "Components/Overlay.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/NamedSlot.h"
#include "Components/Image.h"
#include "Blueprint/WidgetTree.h"
#include "Blueprint/WidgetBlueprintGeneratedClass.h"
#include "Components/PanelSlot.h"
#include "UI/CharacterSelectSlotWidget.h"
#include "Types/CharacterSelectCustomization.h"
#include "UObject/UnrealType.h"
#include "Core/CharacterSelectTestController.h"
#include "Kismet2/KismetEditorUtilities.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCharacterSelectionStudioTest, "CharacterSelection.Editor.StudioTargets", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FCharacterSelectionStudioTest::RunTest(const FString& Parameters)
{
	auto* Blueprint = LoadObject<UWidgetBlueprint>(nullptr, TEXT("/Game/CharacterSelect/UI/Core/WBP_CharacterSelect.WBP_CharacterSelect"));
	if (!TestNotNull(TEXT("Screen Blueprint"), Blueprint)) return false;
	TSharedRef<SCharacterSelectionEditor> Studio = SNew(SCharacterSelectionEditor);
	Studio->Assets[0].Reset(Blueprint);
	Studio->Assets[1].Reset(NewObject<UCharacterSelectLayoutData>());
	auto* Registry = NewObject<UCharacterSelectRegistry>();
	Registry->Characters.Add(NewObject<UCharacterSelectData>());
	Studio->Assets[2].Reset(Registry);
	Studio->Assets[3].Reset(); Studio->Assets[4].Reset();
	Studio->Assets[5].Reset(NewObject<UCharacterSelectTheme>());
	auto* Launcher = FKismetEditorUtilities::CreateBlueprint(ACharacterSelectTestController::StaticClass(), GetTransientPackage(), MakeUniqueObjectName(GetTransientPackage(), UBlueprint::StaticClass(), TEXT("TestHandoffLauncher")), BPTYPE_Normal);
	Studio->Assets[6].Reset(Launcher);
	for (int32 Page = 1; Page <= 12; ++Page)
	{
		Studio->SelectPage(Page);
		TestTrue(*FString::Printf(TEXT("Page %d finds actual asset targets"), Page), Studio->CurrentTarget.IsValid() && Studio->CurrentTarget->Object.IsValid());
	}
	TestTrue(TEXT("Handoff target owns launcher Blueprint for saving and opening"), Studio->CurrentTarget->Blueprint.Get() == Launcher);
	for (int32 Page = 13; Page <= 15; ++Page)
    {
        Studio->SelectPage(Page);
        TestTrue(*FString::Printf(TEXT("Presentation page %d discovers connected designs"), Page), !Studio->Targets.IsEmpty());
        TSet<UObject*> Seen;
        for (const auto& Target : Studio->Targets)
        {
            TestTrue(TEXT("Discovered target has an owning Blueprint"), Target->Blueprint.IsValid());
            TestFalse(TEXT("Shared designs are not duplicated"), Seen.Contains(Target->Object.Get()));
            Seen.Add(Target->Object.Get());
            if (Page == 13) TestTrue(TEXT("Slot page edits slot designs"), Target->Blueprint->GeneratedClass->IsChildOf(UCharacterSelectSlotWidget::StaticClass()));
            if (Page == 14) TestTrue(TEXT("Preview page edits preview designs"), Target->Blueprint->GeneratedClass->IsChildOf(UCharacterSelectPreviewWidget::StaticClass()));
            if (Page == 15) TestTrue(TEXT("Text page edits text or its layout slot"), Target->Object->IsA<UTextBlock>() || Target->Object->IsA<UPanelSlot>());
        }
    }
    Studio->SelectPage(16);
    TestTrue(TEXT("Slot animations discovers connected class defaults"), !Studio->Targets.IsEmpty());
    for (const auto& Target : Studio->Targets)
        TestTrue(TEXT("Slot animation target is a slot class default"), Target->Object->IsA<UCharacterSelectSlotWidget>() && Target->Object->HasAnyFlags(RF_ClassDefaultObject));
    Studio->SelectPage(4);
    const auto* Threshold = FindFProperty<FProperty>(FCharacterSelectControlSettings::StaticStruct(), TEXT("EngageThreshold"));
    const auto* Confirm = FindFProperty<FProperty>(FCharacterSelectControlSettings::StaticStruct(), TEXT("Confirm"));
    TestTrue(TEXT("Stick tuning is advanced"), Threshold && Studio->IsAdvancedProperty(*Threshold));
    TestTrue(TEXT("Core bindings remain basic"), Confirm && !Studio->IsAdvancedProperty(*Confirm));
    Studio->SelectPage(-1);
    TestEqual(TEXT("Invalid page leaves navigation unchanged"), Studio->ActivePage, 4);
    Studio->SelectPage(17);
    TestTrue(TEXT("Independent animation page finds both players"), Studio->Targets.Num() >= 2);
    Studio->SelectPage(6);
	TestTrue(TEXT("Both preview instances can be edited independently"), Studio->Targets.Num() >= 2);
	Studio->SelectPage(1);
	auto* Grid = Cast<UCharacterSelectGridWidget>(Studio->CurrentTarget->Object.Get());
	if (!TestNotNull(TEXT("Grid template resolved"), Grid)) return false;
	Studio->SynchronizeLayout();
	auto* Layout = Cast<UCharacterSelectLayoutData>(Studio->Assets[1].Get());
	TestEqual(TEXT("Explicit sync uses actual grid columns"), Layout->Columns, Grid->GetGridLayout().Columns);
	Studio->Validate();
	TestFalse(TEXT("Validation supplies feedback"), Studio->Message.IsEmpty());
	// Missing selections have actionable instructions rather than a blank panel.
    Studio->Assets[0].Reset();
    Studio->SelectPage(13);
    TestTrue(TEXT("Missing slot has no stale target"), Studio->Targets.IsEmpty() && !Studio->CurrentTarget.IsValid());
    TestTrue(TEXT("Missing slot explains how to connect it"), Studio->GetTargetDescription().ToString().Contains(TEXT("Slot Widget Class")));
    // Track edits to connected components even after navigating to another page.
    Studio->CurrentTarget = MakeShared<SCharacterSelectionEditor::FTarget>();
    Studio->CurrentTarget->Object = Launcher->GeneratedClass->GetDefaultObject();
    Studio->CurrentTarget->Blueprint = Launcher;
    FPropertyChangedEvent Event(nullptr);
    Studio->OnPropertyChanged(Event);
    Studio->SelectPage(0);
    TestTrue(TEXT("Edited connected Blueprint retained for Save"), Studio->EditedAssets.Contains(Launcher));
    auto* SlotBP = Cast<UWidgetBlueprint>(FKismetEditorUtilities::CreateBlueprint(UUserWidget::StaticClass(), GetTransientPackage(), MakeUniqueObjectName(GetTransientPackage(), UBlueprint::StaticClass(), TEXT("TestSlotPreset")), BPTYPE_Normal, UWidgetBlueprint::StaticClass(), UWidgetBlueprintGeneratedClass::StaticClass()));
    auto* Root = SlotBP->WidgetTree->ConstructWidget<UOverlay>(UOverlay::StaticClass(), TEXT("Root"));
    SlotBP->WidgetTree->RootWidget = Root;
    for (const TCHAR* Name : {TEXT("Portrait"), TEXT("Player1FocusBorder"), TEXT("Player2FocusBorder"), TEXT("BothPlayersFocusBorder"), TEXT("Player1SelectedBorder"), TEXT("Player2SelectedBorder"), TEXT("LockOverlay")})
        Root->AddChild(SlotBP->WidgetTree->ConstructWidget<UImage>(UImage::StaticClass(), FName(Name)));
    SlotBP->ParentClass = UCharacterSelectSlotWidget::StaticClass();
    FKismetEditorUtilities::CompileBlueprint(SlotBP);
    Studio->Assets[4].Reset(SlotBP);
    Studio->SelectPage(16);
    if (!TestTrue(TEXT("Standalone slot animation target"), Studio->CurrentTarget.IsValid())) return false;
    Studio->ApplySlotAnimationPreset(static_cast<int32>(ECharacterSlotAnimationPreset::Pop));
    auto* SlotDefaults = Cast<UCharacterSelectSlotWidget>(SlotBP->GeneratedClass->GetDefaultObject());
    TestTrue(TEXT("Studio applies configured preset"), SlotDefaults->SlotAnimationSettings.Mode == ECharacterSlotAnimationMode::Configured);
    TestEqual(TEXT("Studio preset uses expected scale"), SlotDefaults->SlotAnimationSettings.Selected.Scale, 1.16f);
    GEditor->UndoTransaction();
    SlotDefaults = Cast<UCharacterSelectSlotWidget>(SlotBP->GeneratedClass->GetDefaultObject());
    TestTrue(TEXT("Preset is undoable"), SlotDefaults->SlotAnimationSettings.Mode == ECharacterSlotAnimationMode::Authored);
    GEditor->RedoTransaction();
    SlotDefaults = Cast<UCharacterSelectSlotWidget>(SlotBP->GeneratedClass->GetDefaultObject());
    TestTrue(TEXT("Preset is redoable"), SlotDefaults->SlotAnimationSettings.Mode == ECharacterSlotAnimationMode::Configured);
    Studio->ApplySlotAnimationPreset(-1);
    TestTrue(TEXT("Authored mode can be restored"), SlotDefaults->SlotAnimationSettings.Mode == ECharacterSlotAnimationMode::Authored);
    TestEqual(TEXT("Switching to authored retains custom values"), SlotDefaults->SlotAnimationSettings.Selected.Scale, 1.16f);
    auto* PreviewBP = Cast<UWidgetBlueprint>(FKismetEditorUtilities::CreateBlueprint(UUserWidget::StaticClass(), GetTransientPackage(), MakeUniqueObjectName(GetTransientPackage(), UBlueprint::StaticClass(), TEXT("TestPreviewPreset")), BPTYPE_Normal, UWidgetBlueprint::StaticClass(), UWidgetBlueprintGeneratedClass::StaticClass()));
    auto* PreviewRoot = PreviewBP->WidgetTree->ConstructWidget<UCanvasPanel>(UCanvasPanel::StaticClass(), TEXT("Root"));
    PreviewBP->WidgetTree->RootWidget = PreviewRoot;
    PreviewRoot->AddChild(PreviewBP->WidgetTree->ConstructWidget<UImage>(UImage::StaticClass(), TEXT("LargePortrait")));
    PreviewRoot->AddChild(PreviewBP->WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("CharacterName")));
    PreviewRoot->AddChild(PreviewBP->WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("Subtitle")));
    PreviewBP->ParentClass = UCharacterSelectPreviewWidget::StaticClass();
    FKismetEditorUtilities::CompileBlueprint(PreviewBP);
    Studio->Assets[4].Reset(PreviewBP);
    Studio->SelectPage(17);
    Studio->ApplyPreviewAnimationPreset(static_cast<int32>(ECharacterPreviewPreset::Staggered));
    auto* PreviewDefaults = Cast<UCharacterSelectPreviewWidget>(PreviewBP->GeneratedClass->GetDefaultObject());
    TestTrue(TEXT("Studio enables independent animations"), PreviewDefaults->PreviewAnimationSettings.Mode == ECharacterPreviewAnimationMode::Configured);
    TestEqual(TEXT("Studio preset adds subtitle stagger"), PreviewDefaults->PreviewAnimationSettings.Subtitle.Entrance.Delay, 0.14f);
    GEditor->UndoTransaction();
    PreviewDefaults = Cast<UCharacterSelectPreviewWidget>(PreviewBP->GeneratedClass->GetDefaultObject());
    TestTrue(TEXT("Preview preset supports Undo"), PreviewDefaults->PreviewAnimationSettings.Mode == ECharacterPreviewAnimationMode::Authored);
    GEditor->RedoTransaction();
    PreviewDefaults = Cast<UCharacterSelectPreviewWidget>(PreviewBP->GeneratedClass->GetDefaultObject());
    TestTrue(TEXT("Preview preset supports Redo"), PreviewDefaults->PreviewAnimationSettings.Mode == ECharacterPreviewAnimationMode::Configured);
    Studio->ApplyPreviewAnimationPreset(-1);
    TestEqual(TEXT("Restoring authored mode keeps configured settings"), PreviewDefaults->PreviewAnimationSettings.Subtitle.Entrance.Delay, 0.14f);
    Studio->SelectPage(19);
    auto* PortraitSlot = Cast<UCanvasPanelSlot>(PreviewBP->WidgetTree->FindWidget(TEXT("LargePortrait"))->Slot);
    PortraitSlot->SetPosition(FVector2D(31,47));
    PortraitSlot->SetSize(FVector2D(220,430));
    Studio->AddPresentationHost();
    auto* Host = Cast<UNamedSlot>(PreviewBP->WidgetTree->FindWidget(TEXT("CustomPresentationSlot")));
    if (!TestNotNull(TEXT("Studio creates the presentation host"), Host)) return false;
    auto* HostSlot = Cast<UCanvasPanelSlot>(Host->Slot);
    TestTrue(TEXT("Host matches artwork placement"), HostSlot && HostSlot->GetPosition().Equals(FVector2D(31,47)) && HostSlot->GetSize().Equals(FVector2D(220,430)));
    Studio->AddPresentationHost();
    TestTrue(TEXT("Repeated helper keeps the existing host"), PreviewBP->WidgetTree->FindWidget(TEXT("CustomPresentationSlot")) == Host);
    Studio->SelectPage(20);
    Studio->ApplySequencePreset(1);
    PreviewDefaults = Cast<UCharacterSelectPreviewWidget>(PreviewBP->GeneratedClass->GetDefaultObject());
    TestTrue(TEXT("Sequence preset enables player override"), PreviewDefaults->bOverrideSequence && PreviewDefaults->Sequence.bEnabled);
    TestTrue(TEXT("Portrait first preset applied"), PreviewDefaults->Sequence.EntranceOrder == ECharacterSequenceOrder::PortraitFirst);
    GEditor->UndoTransaction();
    PreviewDefaults = Cast<UCharacterSelectPreviewWidget>(PreviewBP->GeneratedClass->GetDefaultObject());
    TestFalse(TEXT("Sequence preset supports undo"), PreviewDefaults->bOverrideSequence);
    GEditor->RedoTransaction();
    PreviewDefaults = Cast<UCharacterSelectPreviewWidget>(PreviewBP->GeneratedClass->GetDefaultObject());
    TestTrue(TEXT("Sequence preset supports redo"), PreviewDefaults->bOverrideSequence);
    Studio->ApplySequencePreset(-1);
    TestFalse(TEXT("Inherit removes player override"), PreviewDefaults->bOverrideSequence);
    Studio->Assets[0].Reset(Blueprint);
    Studio->Assets[4].Reset();
    Studio->SelectPage(18);
    bool bCharacter2DTarget = false, bPreview2DTarget = false;
    for (const auto& Target : Studio->Targets)
    {
        bCharacter2DTarget |= Target->Object->IsA<UCharacterSelectData>();
        bPreview2DTarget |= Target->Object->IsA<UCharacterSelectPreviewWidget>();
    }
    TestTrue(TEXT("2D page exposes character overrides and player defaults"), bCharacter2DTarget && bPreview2DTarget);
    Studio->SelectPage(19);
    bool bCharacter3DTarget = false, bPreview3DTarget = false;
    for (const auto& Target : Studio->Targets)
    {
        bCharacter3DTarget |= Target->Object->IsA<UCharacterSelectData>();
        bPreview3DTarget |= Target->Object->IsA<UCharacterSelectPreviewWidget>();
    }
    TestTrue(TEXT("3D page exposes character overrides and player defaults"), bCharacter3DTarget && bPreview3DTarget);
    // Studio must flag a different setup from the launcher's actual references.
    Studio->Assets[0].Reset(Blueprint);
    Studio->Assets[6].Reset(Launcher);
    auto* LauncherCDO = Launcher->GeneratedClass->GetDefaultObject();
    FindFProperty<FObjectPropertyBase>(LauncherCDO->GetClass(), TEXT("CharacterSelectWidgetClass"))->SetObjectPropertyValue_InContainer(LauncherCDO, Blueprint->GeneratedClass);
    FindFProperty<FObjectPropertyBase>(LauncherCDO->GetClass(), TEXT("CharacterRegistry"))->SetObjectPropertyValue_InContainer(LauncherCDO, Studio->Assets[2].Get());
    FindFProperty<FObjectPropertyBase>(LauncherCDO->GetClass(), TEXT("CharacterLayout"))->SetObjectPropertyValue_InContainer(LauncherCDO, Studio->Assets[1].Get());
    TestTrue(TEXT("Matching launcher setup passes wiring check"), Studio->GetLauncherMismatch().IsEmpty());
    Studio->Assets[1].Reset(NewObject<UCharacterSelectLayoutData>());
    TestFalse(TEXT("Different layout is reported"), Studio->GetLauncherMismatch().IsEmpty());
    // No source assets are edited or saved by this test.
	return true;
}
#endif
