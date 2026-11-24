#if WITH_DEV_AUTOMATION_TESTS
#include "Misc/AutomationTest.h"
#include "UI/CharacterSelectPreviewWidget.h"
#include "Data/CharacterSelectData.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "Blueprint/WidgetBlueprintGeneratedClass.h"
#include "UObject/StrongObjectPtr.h"
#include "Editor.h"
#include "Engine/World.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCharacterPreviewElementAnimationTest, "CharacterSelection.Editor.PortraitAndTextAnimations", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FCharacterPreviewElementAnimationTest::RunTest(const FString& Parameters)
{
    UClass* Class = LoadClass<UCharacterSelectPreviewWidget>(nullptr, TEXT("/Game/CharacterSelect/UI/Preview/WBP_CharacterSelectPreview.WBP_CharacterSelectPreview_C"));
    if (!TestNotNull(TEXT("Original preview loads"), Class)) return false;
    auto* BPClass = Cast<UWidgetBlueprintGeneratedClass>(Class);
    TestTrue(TEXT("Original Blueprint permits native ticking"), BPClass && BPClass->ClassRequiresNativeTick());
    TStrongObjectPtr<UCharacterSelectPreviewWidget> Preview(NewObject<UCharacterSelectPreviewWidget>(GEditor->GetEditorWorldContext().World(), Class));
    Preview->Initialize();
    if (!TestNotNull(TEXT("Portrait binding"), Preview->LargePortrait.Get()) || !TestNotNull(TEXT("Name binding"), Preview->CharacterName.Get()) || !TestNotNull(TEXT("Subtitle binding"), Preview->Subtitle.Get())) return false;
    Preview->PreviewAnimationSettings = FCharacterPreviewAnimationSettings::MakePreset(ECharacterPreviewPreset::Staggered);
    FWidgetTransform Baseline;
    Baseline.Scale = FVector2D(-1.2, 0.8); Baseline.Translation = FVector2D(12, 7);
    Baseline.Angle = 4.f;
    Preview->CharacterName->SetRenderTransform(Baseline);
    Preview->CharacterName->SetRenderOpacity(0.8f);
    auto& NameSettings = Preview->PreviewAnimationSettings.Name;
    NameSettings.Entrance.Delay = 0.1f; NameSettings.Entrance.Duration = 0.2f;
    NameSettings.Entrance.Easing = ECharacterSlotEasing::Linear;
    TStrongObjectPtr<UCharacterSelectData> A(NewObject<UCharacterSelectData>()), B(NewObject<UCharacterSelectData>()), C(NewObject<UCharacterSelectData>());
    A->DisplayName = FText::FromString(TEXT("A")); B->DisplayName = FText::FromString(TEXT("B")); C->DisplayName = FText::FromString(TEXT("C"));
    C->Subtitle = FText::FromString(TEXT("Latest"));
    const auto SlateWidget = Preview->TakeWidget();
    Preview->SetCharacter(A.Get());
    TestEqual(TEXT("Initial content assigned without empty exit"), Preview->GetCharacter(), A.Get());
    Preview->AdvanceConfiguredAnimation(0.05f);
    TestEqual(TEXT("Name waits for its entrance delay"), Preview->CharacterName->GetRenderOpacity(), 0.f);
    TestTrue(TEXT("Portrait animates independently"), Preview->LargePortrait->GetRenderOpacity() > 0.f);
    Preview->AdvanceConfiguredAnimation(0.15f);
    TestTrue(TEXT("Delayed name interpolates opacity relative to Designer"), FMath::IsNearlyEqual(Preview->CharacterName->GetRenderOpacity(), 0.4f, 0.0001f));
    const auto Interrupted = Preview->CharacterName->GetRenderTransform();
    Preview->SetCharacter(B.Get());
    TestTrue(TEXT("Interrupted entrance starts exit without snapping"), Preview->CharacterName->GetRenderTransform() == Interrupted);
    Preview->AdvanceConfiguredAnimation(0.04f);
    const float ExitElapsed = Preview->ConfiguredElapsed;
    Preview->SetCharacter(C.Get());
    TestEqual(TEXT("Rapid request does not restart exit"), Preview->ConfiguredElapsed, ExitElapsed);
    TestEqual(TEXT("Outgoing content retained until swap"), Preview->CharacterName->GetText().ToString(), FString(TEXT("A")));
    Preview->AdvanceConfiguredAnimation(1.f);
    TestEqual(TEXT("Latest request wins"), Preview->GetCharacter(), C.Get());
    TestEqual(TEXT("Name swaps with portrait data"), Preview->CharacterName->GetText().ToString(), FString(TEXT("C")));
    TestEqual(TEXT("Subtitle swaps with name"), Preview->Subtitle->GetText().ToString(), FString(TEXT("Latest")));
    TestTrue(TEXT("Long frame completes both phases"), Preview->ConfiguredPhase == UCharacterSelectPreviewWidget::EConfiguredPhase::Idle);
    TestTrue(TEXT("Designer transform and mirrored scale restored"), Preview->CharacterName->GetRenderTransform() == Baseline);
    TestTrue(TEXT("Designer opacity restored"), FMath::IsNearlyEqual(Preview->CharacterName->GetRenderOpacity(), 0.8f));
    TestFalse(TEXT("Configured transitions do not start authored clips"), Preview->IsAnyAnimationPlaying());
    Preview->SetCharacter(B.Get()); Preview->ClearPreview();
    Preview->AdvanceConfiguredAnimation(1.f);
    TestNull(TEXT("Clear during exit wins"), Preview->GetCharacter());
    TestTrue(TEXT("Clear empties text"), Preview->CharacterName->GetText().IsEmpty());
    TestEqual(TEXT("Missing portrait is collapsed"), Preview->LargePortrait->GetVisibility(), ESlateVisibility::Collapsed);
    Preview->SetCharacter(A.Get()); Preview->SetCharacter(B.Get()); Preview->SetCharacter(A.Get());
    Preview->AdvanceConfiguredAnimation(1.f);
    TestEqual(TEXT("Return to outgoing character is handled"), Preview->GetCharacter(), A.Get());
    Preview->SetCharacter(B.Get());
    Preview->NativeDestruct();
    TestTrue(TEXT("Destruction restores Designer transform"), Preview->CharacterName->GetRenderTransform() == Baseline);
    TestEqual(TEXT("Destruction keeps latest requested character"), Preview->GetCharacter(), B.Get());
    Preview->NativeConstruct();
    Preview->PreviewAnimationSettings = FCharacterPreviewAnimationSettings::MakePreset(ECharacterPreviewPreset::NoMotion);
    Preview->SetCharacter(C.Get());
    TestEqual(TEXT("Disabled elements change immediately"), Preview->GetCharacter(), C.Get());
    TestTrue(TEXT("No motion settles synchronously"), Preview->ConfiguredPhase == UCharacterSelectPreviewWidget::EConfiguredPhase::Idle);
    Preview->ClearPreview();
    TestNull(TEXT("No motion clear is immediate"), Preview->GetCharacter());
    Preview->PreviewAnimationSettings = FCharacterPreviewAnimationSettings::MakePreset(ECharacterPreviewPreset::Fade);
    Preview->PreviewAnimationSettings.Portrait.Entrance.Duration = 0.f;
    Preview->PreviewAnimationSettings.Name.Entrance.Duration = 0.f;
    Preview->PreviewAnimationSettings.Subtitle.Entrance.Duration = 0.f;
    Preview->SetCharacter(A.Get());
    TestTrue(TEXT("Zero duration entrance settles synchronously"), Preview->ConfiguredPhase == UCharacterSelectPreviewWidget::EConfiguredPhase::Idle);
    Preview->NativeDestruct();
    return true;
}
#endif
