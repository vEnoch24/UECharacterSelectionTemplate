#if WITH_DEV_AUTOMATION_TESTS
#include "Misc/AutomationTest.h"
#include "UI/CharacterSelectSlotWidget.h"
#include "Blueprint/WidgetTree.h"
#include "Animation/WidgetAnimation.h"
#include "Components/PanelWidget.h"
#include "Components/Image.h"
#include "UObject/StrongObjectPtr.h"
#include "Misc/PackageName.h"
#include "Editor.h"
#include "Engine/World.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCharacterSlotStateTest, "CharacterSelection.Editor.SlotSharedState", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FCharacterSlotStateTest::RunTest(const FString& Parameters)
{
    // The project example is optional when this plugin is installed in another project.
    if (!FPackageName::DoesPackageExist(TEXT("/Game/CharacterSelect/UI/Slots/WBP_CharacterSlot")))
    { AddInfo(TEXT("Project example WBP_CharacterSlot is unavailable; skipping example integration test.")); return true; }
    UClass* Class = LoadClass<UCharacterSelectSlotWidget>(nullptr, TEXT("/Game/CharacterSelect/UI/Slots/WBP_CharacterSlot.WBP_CharacterSlot_C"));
    if (!TestNotNull(TEXT("Authored Blueprint loads"), Class)) return false;
    UWorld* World = GEditor->GetEditorWorldContext().World();
    if (!TestNotNull(TEXT("Editor test world"), World)) return false;
    TStrongObjectPtr<UCharacterSelectSlotWidget> Slot(NewObject<UCharacterSelectSlotWidget>(World, Class));
    // This test exercises authored clip arbitration, regardless of which preset
    // the developer has saved on the demo Blueprint. Change only the test instance.
    Slot->SlotAnimationSettings.Mode = ECharacterSlotAnimationMode::Authored;
    Slot->Initialize();
    // Transient test widgets have no owning player; initialize their runtime hooks.
    if (!Slot->OnAnimationFinishedPlaying().IsBoundToObject(Slot.Get())) Slot->NativeOnInitialized();
    if (!TestNotNull(TEXT("Combined focus binding"), Slot->BothPlayersFocusBorder.Get())) return false;
    UPanelWidget* Panel = Slot->BothPlayersFocusBorder->GetParent();
    const auto OriginalPanelSlot = Slot->BothPlayersFocusBorder->Slot;
    const auto SlateWidget = Slot->TakeWidget();
    TestTrue(TEXT("Combined indicator above P1 selection"), Panel->GetChildIndex(Slot->BothPlayersFocusBorder) > Panel->GetChildIndex(Slot->Player1SelectedBorder));
    TestTrue(TEXT("Combined indicator above P2 selection"), Panel->GetChildIndex(Slot->BothPlayersFocusBorder) > Panel->GetChildIndex(Slot->Player2SelectedBorder));
    TestTrue(TEXT("Portrait stays above indicator"), Panel->GetChildIndex(Slot->Portrait) > Panel->GetChildIndex(Slot->BothPlayersFocusBorder));
    TestTrue(TEXT("Lock stays above indicator"), Panel->GetChildIndex(Slot->LockOverlay) > Panel->GetChildIndex(Slot->BothPlayersFocusBorder));
    TestTrue(TEXT("Original layout slot retained"), Slot->BothPlayersFocusBorder->Slot == OriginalPanelSlot);
    auto Visible = [](UWidget* W) { return W->GetVisibility() != ESlateVisibility::Collapsed; };
    for (int32 Mask = 0; Mask < 16; ++Mask)
    {
        const bool F1 = !!(Mask & 1), F2 = !!(Mask & 2), S1 = !!(Mask & 4), S2 = !!(Mask & 8);
        Slot->SetFocusedForPlayer(0, F1); Slot->SetFocusedForPlayer(1, F2);
        Slot->SetSelectedForPlayer(0, S1); Slot->SetSelectedForPlayer(1, S2);
        TestEqual(FString::Printf(TEXT("State %d shared indicator"), Mask), Visible(Slot->BothPlayersFocusBorder), (F1 || S1) && (F2 || S2));
        TestEqual(TEXT("P1 focus retained"), Visible(Slot->Player1FocusBorder), F1);
        TestEqual(TEXT("P2 focus retained"), Visible(Slot->Player2FocusBorder), F2);
        TestEqual(TEXT("P1 selection retained"), Visible(Slot->Player1SelectedBorder), S1);
        TestEqual(TEXT("P2 selection retained"), Visible(Slot->Player2SelectedBorder), S2);
        TestEqual(TEXT("Raw focus query unchanged"), Slot->IsFocusedByBothPlayers(), F1 && F2);
    }
    for (int32 First = 0; First < 2; ++First)
    {
        for (int32 P = 0; P < 2; ++P) { Slot->SetSelectedForPlayer(P, false); Slot->SetFocusedForPlayer(P, false); }
        Slot->SetFocusedForPlayer(First, true);
        UWidgetAnimation* FocusClip = Slot->SlotTransitionAnimation;
        Slot->SetFocusedForPlayer(1 - First, true);
        TestTrue(TEXT("Second focus preserves active transition"), Slot->SlotTransitionAnimation == FocusClip);
        Slot->SetSelectedForPlayer(First, true);
        TestTrue(TEXT("First selection plays authored animation"), Slot->SlotTransitionAnimation == Slot->SelectedAnimation);
        TestTrue(TEXT("Selection keeps shared indicator"), Visible(Slot->BothPlayersFocusBorder));
        Slot->SetSelectedForPlayer(1 - First, true);
        Slot->SetSelectedForPlayer(First, false);
        TestTrue(TEXT("One cancellation does not deselect other player"), Slot->SlotTransitionAnimation == Slot->SelectedAnimation);
        Slot->SetFocusedForPlayer(First, false);
        TestFalse(TEXT("Departing player removes combined indicator"), Visible(Slot->BothPlayersFocusBorder));
        TestTrue(TEXT("Departure preserves remaining selection animation"), Slot->SlotTransitionAnimation == Slot->SelectedAnimation);
        Slot->SetSelectedForPlayer(1 - First, false);
        TestTrue(TEXT("Last cancellation plays authored deselection"), Slot->SlotTransitionAnimation == Slot->DeselectedAnimation);
        TestTrue(TEXT("Remaining focus scheduled for restoration"), Slot->bRestoreFocusAfterDeselection);
        // Exercise the real animation completion delegate, including queued stops.
        Slot->FlushAnimations();
        for (int32 Frame = 0; Frame < 180; ++Frame)
        {
            Slot->TickActionsAndAnimation(1.f / 60.f);
            Slot->FlushAnimations();
        }
        TestTrue(TEXT("Focus restored for remaining player"), Slot->SlotTransitionAnimation == (First == 0 ? Slot->Player2FocusAnimation : Slot->Player1FocusAnimation));
        Slot->SetSelectedForPlayer(First, true);
        Slot->SetSelectedForPlayer(First, false);
        Slot->SetSelectedForPlayer(First, true);
        Slot->RestoreFocusAfterDeselection();
        TestTrue(TEXT("Reselection cancels stale focus restoration"), Slot->SlotTransitionAnimation == Slot->SelectedAnimation);
    }
    const bool Before = Slot->IsOccupiedByBothPlayers();
    Slot->SetFocusedForPlayer(-1, true); Slot->SetSelectedForPlayer(2, true);
    TestEqual(TEXT("Invalid players ignored"), Slot->IsOccupiedByBothPlayers(), Before);
    TestFalse(TEXT("Invalid focus not stored"), Slot->IsFocusedForPlayer(-1));
    TestFalse(TEXT("Invalid selection not stored"), Slot->IsSelectedForPlayer(2));
    Slot->SetLocked(true); TestTrue(TEXT("Lock displayed"), Visible(Slot->LockOverlay));
    Slot->SetLocked(false); TestFalse(TEXT("Lock cleared"), Visible(Slot->LockOverlay));
    Slot->DeselectedAnimation = nullptr;
    Slot->SetSelectedForPlayer(0, false); Slot->SetSelectedForPlayer(1, false);
    TestFalse(TEXT("Missing deselection clip restores focus immediately"), Slot->bRestoreFocusAfterDeselection);
    Slot->BothPlayersFocusBorder = nullptr;
    Slot->SetFocusedForPlayer(0, true); Slot->SetFocusedForPlayer(1, true);
    TestTrue(TEXT("Optional combined widget does not affect state"), Slot->IsOccupiedByBothPlayers());
    // Compare actual authored endpoint properties against a settled runtime widget.
    TStrongObjectPtr<UCharacterSelectSlotWidget> Runtime(NewObject<UCharacterSelectSlotWidget>(World, Class));
    TStrongObjectPtr<UCharacterSelectSlotWidget> Studio(NewObject<UCharacterSelectSlotWidget>(World, Class));
    Studio->SetDesignerFlags(EWidgetDesignFlags::Designing);
    for (auto* W : {Runtime.Get(), Studio.Get()})
    {
        W->SlotAnimationSettings.Mode = ECharacterSlotAnimationMode::Authored;
        W->Initialize();
    }
    Runtime->NativeOnInitialized();
    Studio->SetStudioAnimationPreview(false);
    const auto RuntimeSlate = Runtime->TakeWidget();
    const auto StudioSlate = Studio->TakeWidget();
    auto ComparePoses = [&]()
    {
        Runtime->FlushAnimations();
        for (int32 Frame = 0; Frame < 180; ++Frame)
        {
            Runtime->TickActionsAndAnimation(1.f / 60.f);
            Runtime->FlushAnimations();
        }
        Studio->SyncStudioVisualState();
        for (UWidgetAnimation* Clip : {Studio->Player1FocusAnimation.Get(), Studio->Player2FocusAnimation.Get(), Studio->Player1UnfocusAnimation.Get(), Studio->Player2UnfocusAnimation.Get(), Studio->SelectedAnimation.Get(), Studio->DeselectedAnimation.Get()})
            TestFalse(TEXT("Studio pose does not start animation playback"), Studio->IsAnimationPlaying(Clip));
        Runtime->WidgetTree->ForEachWidget([&](UWidget* W)
        {
            auto* Other = Studio->WidgetTree->FindWidget(W->GetFName());
            if (!TestNotNull(TEXT("Matching Studio child"), Other)) return;
            TestTrue(FString::Printf(TEXT("%s settled scale matches Play (Play=%s Studio=%s clip=%s)"), *W->GetName(), *W->GetRenderTransform().Scale.ToString(), *Other->GetRenderTransform().Scale.ToString(), *GetNameSafe(Studio->SlotTransitionAnimation)), W->GetRenderTransform().Scale.Equals(Other->GetRenderTransform().Scale, .001));
            TestTrue(FString::Printf(TEXT("%s settled translation matches Play"), *W->GetName()), W->GetRenderTransform().Translation.Equals(Other->GetRenderTransform().Translation, .001));
            if (auto* Image = Cast<UImage>(W))
                TestTrue(TEXT("Settled image color matches Play"), Image->GetColorAndOpacity().Equals(CastChecked<UImage>(Other)->GetColorAndOpacity(), .001f));
            TestTrue(TEXT("Settled opacity matches Play"), FMath::IsNearlyEqual(W->GetRenderOpacity(), Other->GetRenderOpacity(), .001f));
            TestEqual(FString::Printf(TEXT("%s settled visibility matches Play"), *W->GetName()), Other->GetVisibility(), W->GetVisibility());
        });
    };
    ComparePoses();
    for (int32 Cycle = 0; Cycle < 3; ++Cycle)
    {
        for (int32 Player = 0; Player < 2; ++Player)
        {
            for (auto* W : {Runtime.Get(), Studio.Get()}) W->SetFocusedForPlayer(Player, true);
            ComparePoses();
            for (auto* W : {Runtime.Get(), Studio.Get()}) W->SetSelectedForPlayer(Player, true);
            ComparePoses();
        }
        for (int32 Player = 0; Player < 2; ++Player)
        {
            for (auto* W : {Runtime.Get(), Studio.Get()}) W->SetSelectedForPlayer(Player, false);
            ComparePoses();
            for (auto* W : {Runtime.Get(), Studio.Get()}) W->SetFocusedForPlayer(Player, false);
            ComparePoses();
        }
    }
    Runtime->StopAllAnimations();
    Slot->StopAllAnimations();
    return true;
}
#endif
