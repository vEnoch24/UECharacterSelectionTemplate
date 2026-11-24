#if WITH_DEV_AUTOMATION_TESTS
#include "Misc/AutomationTest.h"
#include "UI/CharacterSelectPreviewWidget.h"
#include "Data/CharacterSelectData.h"
#include "Editor.h"
#include "UObject/StrongObjectPtr.h"
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCharacterSequenceTest, "CharacterSelection.Editor.Sequencing", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FCharacterSequenceTest::RunTest(const FString& Parameters)
{
    auto* Class = LoadClass<UCharacterSelectPreviewWidget>(nullptr, TEXT("/Game/CharacterSelect/UI/Preview/WBP_CharacterSelectPreview.WBP_CharacterSelectPreview_C"));
    if (!TestNotNull(TEXT("Preview loads"), Class)) return false;
    TStrongObjectPtr<UCharacterSelectPreviewWidget> P(NewObject<UCharacterSelectPreviewWidget>(GEditor->GetEditorWorldContext().World(), Class));
    P->Initialize(); const auto Slate = P->TakeWidget();
    TStrongObjectPtr<UCharacterSelectData> A(NewObject<UCharacterSelectData>()), B(NewObject<UCharacterSelectData>());
    P->SharedSequence = NewObject<UCharacterSelectSequenceAsset>(P.Get());
    P->SharedSequence->Sequence = FCharacterSelectSequence::MakePreset(1);
    TestTrue(TEXT("Shared defaults resolve"), P->ResolveSequence(A.Get()).EntranceOrder == ECharacterSequenceOrder::PortraitFirst);
    P->bOverrideSequence = true; P->Sequence = FCharacterSelectSequence::MakePreset(2);
    TestTrue(TEXT("Player overrides shared"), P->ResolveSequence(A.Get()).EntranceOrder == ECharacterSequenceOrder::TextFirst);
    A->bOverrideSequence = true; A->Sequence = FCharacterSelectSequence::MakePreset(0);
    TestTrue(TEXT("Character overrides player"), P->ResolveSequence(A.Get()).EntranceOrder == ECharacterSequenceOrder::Together);
    A->Sequence.bEnabled = false;
    TestFalse(TEXT("Character can explicitly disable sequencing"), P->ResolveSequence(A.Get()).bEnabled);
    A->bOverrideSequence = false; P->bOverrideSequence = false;
    const auto Timing = P->ResolveSequence(A.Get()).GetTransitions(false);
    TestTrue(TEXT("Portrait first waits for portrait plus gap"), Timing.Name.Entrance.Delay >= Timing.Portrait.Entrance.Duration + .049f);
    P->SharedSequence->Sequence.ConfirmDelay = .25f;
    P->SetCharacter(A.Get()); P->SetPreviewSelected(true);
    P->AdvanceSequenceConfirmation(2.f);
    TestFalse(TEXT("Confirm waits during entrance"), P->bConfirmDelivered);
    P->AdvanceConfiguredAnimation(5.f);
    P->AdvanceSequenceConfirmation(.1f);
    TestFalse(TEXT("Confirm respects delay"), P->bConfirmDelivered);
    P->SetPreviewSelected(false); P->AdvanceSequenceConfirmation(2.f);
    TestFalse(TEXT("Cancel drops waiting confirm"), P->bConfirmDelivered);
    P->SetPreviewSelected(true); P->AdvanceSequenceConfirmation(.3f);
    TestTrue(TEXT("Confirm delivered after delay"), P->bConfirmDelivered);
    P->PresentationSeconds = 1.; P->SetPreviewSelected(true); P->AdvanceSequenceConfirmation(2.f);
    TestEqual(TEXT("Repeated confirm does not restart playback"), P->PresentationSeconds, 1.);
    P->SetPreviewSelected(false);
    TestEqual(TEXT("Cancellation resets 2D playback"), P->PresentationSeconds, 0.);
    P->SetCharacter(B.Get()); P->SetCharacter(A.Get()); P->ClearPreview();
    P->AdvanceConfiguredAnimation(10.f);
    TestNull(TEXT("Rapid clear wins over queued characters"), P->GetCharacter());
    TestFalse(TEXT("Clear discards confirm"), P->bPresentationSelected);
    P->SetCharacter(B.Get()); P->SetPreviewSelected(true); P->AdvanceConfiguredAnimation(10.f); P->AdvanceSequenceConfirmation(.3f);
    TestTrue(TEXT("New character can confirm"), P->bConfirmDelivered);
    P->NativeDestruct();
    TestFalse(TEXT("Destruction clears pending confirmation"), P->bConfirmDelivered);
    return true;
}
#endif
