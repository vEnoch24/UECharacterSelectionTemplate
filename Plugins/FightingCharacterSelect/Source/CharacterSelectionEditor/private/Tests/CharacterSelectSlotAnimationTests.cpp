#if WITH_DEV_AUTOMATION_TESTS
#include "Misc/AutomationTest.h"
#include "UI/CharacterSelectSlotWidget.h"
#include "Blueprint/WidgetBlueprintGeneratedClass.h"
#include "UObject/StrongObjectPtr.h"
#include "Misc/PackageName.h"
#include "Editor.h"
#include "Engine/World.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCharacterSlotAnimationTest, "CharacterSelection.Editor.SlotAnimationPresets", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FCharacterSlotAnimationTest::RunTest(const FString& Parameters)
{
    if (!FPackageName::DoesPackageExist(TEXT("/Game/CharacterSelect/UI/Slots/WBP_CharacterSlot")))
    { AddInfo(TEXT("Demo slot is unavailable; skipping authored-slot integration test.")); return true; }
    UClass* Class = LoadClass<UCharacterSelectSlotWidget>(nullptr, TEXT("/Game/CharacterSelect/UI/Slots/WBP_CharacterSlot.WBP_CharacterSlot_C"));
    if (!TestNotNull(TEXT("Authored slot class"), Class)) return false;
    auto* BPClass = Cast<UWidgetBlueprintGeneratedClass>(Class);
    TestTrue(TEXT("Native animation ticking is enabled on the existing Blueprint"), BPClass && BPClass->ClassRequiresNativeTick());
    TStrongObjectPtr<UCharacterSelectSlotWidget> Slot(NewObject<UCharacterSelectSlotWidget>(GEditor->GetEditorWorldContext().World(), Class));
    Slot->Initialize();
    FWidgetTransform Baseline;
    Baseline.Scale = FVector2D(1.2, 0.8); Baseline.Translation = FVector2D(3, 4);
    Baseline.Angle = 7.f; Baseline.Shear = FVector2D(0.1, 0);
    Slot->SetRenderTransform(Baseline);
    Slot->SetRenderOpacity(0.8f);
    const FLinearColor BaseTint(0.9f, 0.8f, 0.7f, 1.f);
    Slot->SetColorAndOpacity(BaseTint);
    Slot->SlotAnimationSettings = FCharacterSlotAnimationSettings::MakePreset(ECharacterSlotAnimationPreset::Lift);
    Slot->SlotAnimationSettings.Focused.Duration = 1.f;
    Slot->SlotAnimationSettings.Focused.Easing = ECharacterSlotEasing::Linear;
    const auto SlateWidget = Slot->TakeWidget();
    Slot->UpdateConfiguredAnimation();
    Slot->AdvanceConfiguredAnimation(1.f);
    TestTrue(TEXT("Idle preserves Designer transform"), Slot->GetRenderTransform() == Baseline);
    Slot->SetFocusedForPlayer(0, true);
    Slot->AdvanceConfiguredAnimation(0.5f);
    TestTrue(TEXT("Halfway linear scale is relative to Designer scale"), FMath::IsNearlyEqual(Slot->GetRenderTransform().Scale.X, 1.2 * 1.025, 0.0001));
    TestTrue(TEXT("Halfway movement adds to Designer translation"), FMath::IsNearlyEqual(Slot->GetRenderTransform().Translation.Y, 1.0, 0.0001));
    const FWidgetTransform MidFocus = Slot->GetRenderTransform();
    Slot->SetFocusedForPlayer(1, true);
    TestTrue(TEXT("Rapid shared focus starts without snapping"), Slot->GetRenderTransform() == MidFocus);
    Slot->AdvanceConfiguredAnimation(1.f);
    TestEqual(TEXT("Shared focus state"), Slot->ConfiguredState, 2);
    Slot->SetSelectedForPlayer(0, true);
    Slot->AdvanceConfiguredAnimation(0.02f);
    TestEqual(TEXT("Mixed focus and selection uses shared selection"), Slot->ConfiguredState, 4);
    Slot->SetSelectedForPlayer(1, true);
    const float Elapsed = Slot->ConfiguredElapsed;
    Slot->SetSelectedForPlayer(0, false);
    TestEqual(TEXT("One cancellation preserves other player's selected state"), Slot->ConfiguredState, 4);
    TestEqual(TEXT("Shared selection animation does not restart"), Slot->ConfiguredElapsed, Elapsed);
    Slot->SetSelectedForPlayer(1, false);
    TestEqual(TEXT("Last cancellation returns to shared focus"), Slot->ConfiguredState, 2);
    TestEqual(TEXT("Cancellation duration honored"), Slot->ConfiguredTarget.Duration, Slot->SlotAnimationSettings.CancelDuration);
    Slot->SetSelectedForPlayer(1, true);
    Slot->AdvanceConfiguredAnimation(1.f);
    TestEqual(TEXT("Rapid reselection wins over cancellation"), Slot->ConfiguredState, 4);
    TestTrue(TEXT("Configured mode does not start authored selection clips"), !Slot->IsAnyAnimationPlaying());
    Slot->SetLocked(true);
    Slot->AdvanceConfiguredAnimation(1.f);
    TestEqual(TEXT("Locked state has priority"), Slot->ConfiguredState, 5);
    TestTrue(TEXT("Locked opacity multiplies Designer opacity"), FMath::IsNearlyEqual(Slot->GetRenderOpacity(), 0.48f, 0.0001f));
    Slot->SetLocked(false);
    TestEqual(TEXT("Unlock restores current shared selection"), Slot->ConfiguredState, 4);
    Slot->SetSelectedForPlayer(1, false);
    Slot->SetFocusedForPlayer(0, false); Slot->SetFocusedForPlayer(1, false);
    Slot->AdvanceConfiguredAnimation(1.f);
    TestTrue(TEXT("Unfocus returns exactly to Designer transform"), Slot->GetRenderTransform() == Baseline);
    TestEqual(TEXT("Idle restores Designer opacity"), Slot->GetRenderOpacity(), 0.8f);
    TestTrue(TEXT("Idle restores Designer tint"), Slot->GetColorAndOpacity().Equals(BaseTint));
    Slot->SlotAnimationSettings.Focused.Duration = 0.f;
    Slot->SlotAnimationSettings.Focused.Tint = FLinearColor(0.5, 1, 1, 1);
    Slot->SetFocusedForPlayer(1, true);
    TestFalse(TEXT("Zero duration settles immediately"), Slot->bConfiguredAnimating);
    TestTrue(TEXT("Tint multiplies Designer color"), Slot->GetColorAndOpacity().Equals(BaseTint * FLinearColor(0.5, 1, 1, 1)));
    Slot->NativeDestruct();
    TestTrue(TEXT("Destruction restores baseline for reconstruction"), Slot->GetRenderTransform() == Baseline);
    TestTrue(TEXT("Destruction restores baseline color"), Slot->GetColorAndOpacity().Equals(BaseTint));
    // Static Studio preview must settle immediately even though its animation
    // clock is disabled, including returning to idle after either player's focus.
    TStrongObjectPtr<UCharacterSelectSlotWidget> Preview(NewObject<UCharacterSelectSlotWidget>(GEditor->GetEditorWorldContext().World(), Class));
    Preview->SetDesignerFlags(EWidgetDesignFlags::Designing);
    Preview->Initialize();
    Preview->SlotAnimationSettings = FCharacterSlotAnimationSettings::MakePreset(ECharacterSlotAnimationPreset::Lift);
    Preview->SetStudioAnimationPreview(false);
    const auto PreviewSlate = Preview->TakeWidget();
    const auto PreviewBaseline = Preview->GetRenderTransform();
    for (int32 P = 0; P < 2; ++P)
        for (int32 Repeat = 0; Repeat < 3; ++Repeat)
        {
            Preview->SetFocusedForPlayer(P, true);
            TestFalse(TEXT("Static preview focus settles without ticking"), Preview->bConfiguredAnimating);
            TestTrue(TEXT("Static preview shows focused scale"), Preview->GetRenderTransform().Scale.Equals(PreviewBaseline.Scale * Preview->SlotAnimationSettings.Focused.Scale));
            Preview->SetFocusedForPlayer(P, false);
            TestFalse(TEXT("Static preview unfocus settles without ticking"), Preview->bConfiguredAnimating);
            TestTrue(TEXT("Static preview restores idle transform"), Preview->GetRenderTransform() == PreviewBaseline);
        }
    for (int32 Preset = 0; Preset <= static_cast<int32>(ECharacterSlotAnimationPreset::NoMotion); ++Preset)
    {
        const auto Settings = FCharacterSlotAnimationSettings::MakePreset(static_cast<ECharacterSlotAnimationPreset>(Preset));
        TestTrue(TEXT("Preset opts into configured mode"), Settings.Mode == ECharacterSlotAnimationMode::Configured);
        TestTrue(TEXT("Preset shared selection remains visible"), Settings.SharedSelection.Opacity > 0.f);
    }
    return true;
}
#endif
