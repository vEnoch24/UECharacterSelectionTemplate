#if WITH_DEV_AUTOMATION_TESTS
#include "Misc/AutomationTest.h"
#include "UI/CharacterSelectPreviewWidget.h"
#include "UI/CharacterSelectPresentationWidget.h"
#include "Data/CharacterSelectData.h"
#include "Components/Image.h"
#include "Components/NamedSlot.h"
#include "Blueprint/WidgetTree.h"
#include "Engine/Texture2D.h"
#include "Editor.h"
#include "UObject/StrongObjectPtr.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCharacter2DPresentationTest, "CharacterSelection.Editor.Presentation2D", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FCharacter2DPresentationTest::RunTest(const FString& Parameters)
{
    UClass* Class = LoadClass<UCharacterSelectPreviewWidget>(nullptr, TEXT("/Game/CharacterSelect/UI/Preview/WBP_CharacterSelectPreview.WBP_CharacterSelectPreview_C"));
    if (!TestNotNull(TEXT("Original preview class"), Class)) return false;
    TStrongObjectPtr<UCharacterSelectPreviewWidget> Preview(NewObject<UCharacterSelectPreviewWidget>(GEditor->GetEditorWorldContext().World(), Class));
    Preview->Initialize();
    const auto SlateWidget = Preview->TakeWidget();
    Preview->PreviewAnimationSettings = FCharacterPreviewAnimationSettings::MakePreset(ECharacterPreviewPreset::NoMotion);
    TStrongObjectPtr<UCharacterSelectData> A(NewObject<UCharacterSelectData>()), B(NewObject<UCharacterSelectData>());
    A->LargePortrait = NewObject<UTexture2D>(A.Get());
    B->Portrait = NewObject<UTexture2D>(B.Get());
    FWidgetTransform Baseline;
    Baseline.Scale = FVector2D(-1.2, .8); Baseline.Translation = FVector2D(11,7); Baseline.Angle = 8;
    Preview->LargePortrait->SetRenderTransform(Baseline);
    Preview->SetCharacter(A.Get());
    TestTrue(TEXT("Default static image remains unchanged"), Preview->LargePortrait->GetBrush().GetResourceObject() == A->LargePortrait);
    TestTrue(TEXT("Default keeps authored transform"), Preview->LargePortrait->GetRenderTransform() == Baseline);
    A->bOverridePresentation2D = true;
    auto& P = A->Presentation2D;
    P.Mode = ECharacter2DPresentationMode::SpriteFrames;
    P.Frames = {A->LargePortrait, NewObject<UTexture2D>(A.Get()), NewObject<UTexture2D>(A.Get())};
    P.FramesPerSecond = 10;
    P.Offset = FVector2D(25,-10); P.Scale = FVector2D(2,.5); P.Facing = ECharacter2DFacing::MirrorPlayer2;
    Preview->RefreshPreview();
    TestEqual(TEXT("Sprite starts at first frame"), Preview->GetPresentationFrameIndex(), 0);
    Preview->AdvancePresentation2D(.15f);
    TestEqual(TEXT("Frame rate advances sprite"), Preview->GetPresentationFrameIndex(), 1);
    TestTrue(TEXT("Rendered brush follows frame"), Preview->LargePortrait->GetBrush().GetResourceObject() == P.Frames[1]);
    Preview->AdvancePresentation2D(.2f);
    TestEqual(TEXT("Sprite wraps"), Preview->GetPresentationFrameIndex(), 0);
    P.bLoop = false; Preview->RefreshPreview(); Preview->AdvancePresentation2D(100.f);
    TestEqual(TEXT("Non-looping sprite holds last frame"), Preview->GetPresentationFrameIndex(), 2);
    Preview->SetPresentationPlayerIndex(1);
    const auto Mirrored = Preview->LargePortrait->GetRenderTransform();
    TestTrue(TEXT("P2 mirrors relative to authored facing"), Mirrored.Scale.Equals(FVector2D(2.4,.4), .0001));
    TestTrue(TEXT("Offset is relative to authored position"), Mirrored.Translation == FVector2D(36,-3));
    for (int32 I=0; I<20; ++I) Preview->ApplyPresentationTransform();
    TestTrue(TEXT("Framing does not accumulate"), Preview->LargePortrait->GetRenderTransform() == Mirrored);
    Preview->SetPresentationPlayerIndex(0);
    TestTrue(TEXT("P1 keeps original facing"), Preview->LargePortrait->GetRenderTransform().Scale.Equals(FVector2D(-2.4,.4), .0001));
    Preview->SetCharacter(B.Get());
    TestTrue(TEXT("Switching restores base transform"), Preview->LargePortrait->GetRenderTransform() == Baseline);
    TestTrue(TEXT("Missing large artwork falls back to portrait"), Preview->LargePortrait->GetBrush().GetResourceObject() == B->Portrait);
    Preview->SetCharacter(A.Get());
    TestEqual(TEXT("Returning restarts playback"), Preview->GetPresentationFrameIndex(), 0);
    Preview->PreviewAnimationSettings = FCharacterPreviewAnimationSettings::MakePreset(ECharacterPreviewPreset::Fade);
    Preview->SetCharacter(B.Get()); Preview->SetCharacter(A.Get());
    Preview->AdvanceConfiguredAnimation(5.f);
    TestTrue(TEXT("Rapid requests retain latest 2D character"), Preview->GetCharacter() == A.Get());
    TestTrue(TEXT("Transitions preserve character framing"), Preview->LargePortrait->GetRenderTransform().Translation == FVector2D(36,-3));
    Preview->PreviewAnimationSettings = FCharacterPreviewAnimationSettings::MakePreset(ECharacterPreviewPreset::NoMotion);
    Preview->ClearPreview();
    TestNull(TEXT("Clear removes image resource"), Preview->LargePortrait->GetBrush().GetResourceObject());
    TestEqual(TEXT("Clear resets frame"), Preview->GetPresentationFrameIndex(), INDEX_NONE);
    TestTrue(TEXT("Clear restores authored transform"), Preview->LargePortrait->GetRenderTransform() == Baseline);
    // Missing custom host falls back safely; a connected host receives context.
    P.Mode = ECharacter2DPresentationMode::CustomWidget;
    P.WidgetClass = UCharacterSelectPresentationWidget::StaticClass();
    Preview->SetCharacter(A.Get());
    TestNull(TEXT("Missing host uses artwork"), Preview->GetCustomPresentationWidget());
    TestTrue(TEXT("Missing host keeps fallback visible"), Preview->LargePortrait->GetVisibility() != ESlateVisibility::Collapsed);
    Preview->CustomPresentationSlot = Preview->WidgetTree->ConstructWidget<UNamedSlot>(UNamedSlot::StaticClass());
    Preview->RefreshPreview();
    auto* Custom = Preview->GetCustomPresentationWidget();
    if (TestNotNull(TEXT("Custom content created"), Custom))
    {
        TestTrue(TEXT("Custom widget receives character"), Custom->PresentedCharacter == A.Get());
        Preview->SetPresentationPlayerIndex(1);
        TestEqual(TEXT("Custom widget receives player"), Custom->PresentationPlayerIndex, 1);
        TestEqual(TEXT("Only custom presentation is visible"), Preview->LargePortrait->GetVisibility(), ESlateVisibility::Collapsed);
        Preview->bOverrideSequence = true;
        Preview->Sequence = FCharacterSelectSequence::MakePreset(3);
        Preview->ApplyPresentationTransform();
        TestTrue(TEXT("Sequenced custom widget preserves 2D scale and facing"), Custom->GetRenderTransform().Scale.Equals(FVector2D(2.4,.4), .0001));
        TestTrue(TEXT("Sequenced custom widget preserves 2D offset"), Custom->GetRenderTransform().Translation.Equals(FVector2D(36,-3), .0001));
        Preview->bOverrideSequence = false;
    }
    Preview->SetCharacter(B.Get());
    TestNull(TEXT("Switching releases custom content"), Preview->GetCustomPresentationWidget());
    TestEqual(TEXT("Custom host cleared"), Preview->CustomPresentationSlot->GetChildrenCount(), 0);
    P.Mode = ECharacter2DPresentationMode::SpriteFrames; P.Frames.Reset();
    TestFalse(TEXT("Invalid sprite settings are reported"), P.GetIssues().IsEmpty());
    Preview->SetCharacter(A.Get());
    TestEqual(TEXT("Empty frames do not index invalid memory"), Preview->GetPresentationFrameIndex(), INDEX_NONE);
    TestTrue(TEXT("Invalid sprite falls back to static artwork"), Preview->LargePortrait->GetBrush().GetResourceObject() == A->LargePortrait);
    Preview->NativeDestruct();
    TestNull(TEXT("Destruction releases custom content"), Preview->GetCustomPresentationWidget());
    return true;
}
#endif
