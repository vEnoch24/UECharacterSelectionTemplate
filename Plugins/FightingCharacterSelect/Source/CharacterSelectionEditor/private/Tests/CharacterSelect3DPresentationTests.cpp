#if WITH_DEV_AUTOMATION_TESTS
#include "Misc/AutomationTest.h"
#include "UI/CharacterSelect3DPreviewWidget.h"
#include "UI/CharacterSelectPreviewWidget.h"
#include "Data/CharacterSelectData.h"
#include "Components/Viewport.h"
#include "Components/NamedSlot.h"
#include "Components/Image.h"
#include "Components/CanvasPanel.h"
#include "Blueprint/WidgetTree.h"
#include "Engine/StaticMeshActor.h"
#include "Engine/World.h"
#include "Editor.h"
#include "UObject/StrongObjectPtr.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCharacter3DPresentationTest, "CharacterSelection.Editor.Presentation3D", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FCharacter3DPresentationTest::RunTest(const FString& Parameters)
{
    FCharacterSelect3DPresentation Settings;
    TestFalse(TEXT("Missing 3D source is reported"), Settings.GetIssues().IsEmpty());
    Settings.ActorClass = AStaticMeshActor::StaticClass();
    Settings.ActorTransform.SetLocation(FVector(12,23,34));
    Settings.Player2YawOffset = 180;
    TestTrue(TEXT("Dedicated actor is a valid source"), Settings.GetIssues().IsEmpty());
    auto* HostWorld = GEditor->GetEditorWorldContext().World();
    TStrongObjectPtr<UCharacterSelect3DPreviewWidget> Left(NewObject<UCharacterSelect3DPreviewWidget>(HostWorld)), Right(NewObject<UCharacterSelect3DPreviewWidget>(HostWorld));
    Left->Initialize(); Right->Initialize();
    Left->Configure(Settings, nullptr, 0, false); Right->Configure(Settings, nullptr, 1, false);
    const auto LeftSlate = Left->TakeWidget();
    const auto RightSlate = Right->TakeWidget();
    if (!TestNotNull(TEXT("Left actor created"), Left->GetPreviewActor()) || !TestNotNull(TEXT("Right actor created"), Right->GetPreviewActor())) return false;
    TestTrue(TEXT("Preview actors never enter the host world"), Left->GetPreviewActor()->GetWorld() != HostWorld && Right->GetPreviewActor()->GetWorld() != HostWorld);
    TestTrue(TEXT("Each player has an independent scene"), Left->GetPreviewActor()->GetWorld() != Right->GetPreviewActor()->GetWorld());
    TestTrue(TEXT("Actor transform applied"), Left->GetPreviewActor()->GetActorLocation().Equals(FVector(12,23,34)));
    TestTrue(TEXT("Second player yaw applied independently"), FMath::Abs(FMath::FindDeltaAngleDegrees(Left->GetPreviewActor()->GetActorRotation().Yaw, Right->GetPreviewActor()->GetActorRotation().Yaw)) > 179.f);
    TestTrue(TEXT("Camera position applied"), Left->SceneViewport->GetViewLocation().Equals(Settings.CameraLocation));
    TestTrue(TEXT("Camera aims at target"), Left->SceneViewport->GetViewRotation().Equals((Settings.CameraTarget-Settings.CameraLocation).Rotation(), .01f));
    TestFalse(TEXT("Preview actor collision disabled"), Left->GetPreviewActor()->GetActorEnableCollision());
    Left->SetSelected(true); TestTrue(TEXT("Selection state delivered"), Left->bSelectedState);
    Left->SetSelected(false); TestFalse(TEXT("Cancellation returns to idle state"), Left->bSelectedState);
    auto* OldActor = Left->GetPreviewActor();
    Left->Configure(Settings, nullptr, 0, false);
    TestTrue(TEXT("Reconfiguration destroys old actor"), !IsValid(OldActor) || OldActor->IsActorBeingDestroyed());
    TestTrue(TEXT("Reconfiguration creates a fresh actor"), Left->GetPreviewActor() && Left->GetPreviewActor() != OldActor);
    Settings.CameraTarget = Settings.CameraLocation;
    Left->Configure(Settings, nullptr, 0, false);
    TestNull(TEXT("Invalid camera prevents scene actor creation"), Left->GetPreviewActor());
    TestFalse(TEXT("Invalid scene supplies guidance"), Left->GetPreviewError().IsEmpty());
    Left->NativeDestruct(); Right->NativeDestruct();
    TestNull(TEXT("Left cleanup releases actor"), Left->GetPreviewActor());
    TestNull(TEXT("Right cleanup releases actor"), Right->GetPreviewActor());

    // Integration with the existing preview: preserve 2D fallback and swap at
    // the same point as the name/subtitle, not at the initial navigation event.
    auto* Class = LoadClass<UCharacterSelectPreviewWidget>(nullptr, TEXT("/Game/CharacterSelect/UI/Preview/WBP_CharacterSelectPreview.WBP_CharacterSelectPreview_C"));
    if (!TestNotNull(TEXT("Original preview loads"), Class)) return false;
    TStrongObjectPtr<UCharacterSelectPreviewWidget> Preview(NewObject<UCharacterSelectPreviewWidget>(HostWorld, Class));
    Preview->Initialize();
    Preview->PreviewAnimationSettings = FCharacterPreviewAnimationSettings::MakePreset(ECharacterPreviewPreset::NoMotion);
    TStrongObjectPtr<UCharacterSelectData> A(NewObject<UCharacterSelectData>()), B(NewObject<UCharacterSelectData>());
    A->PreviewMode = ECharacterPreviewMode::Actor3D;
    A->bOverridePresentation3D = true;
    A->Presentation3D.ActorClass = AStaticMeshActor::StaticClass();
    const auto PreviewSlate = Preview->TakeWidget();
    Preview->SetCharacter(A.Get());
    TestNull(TEXT("Missing host keeps the existing presentation path"), Preview->GetCustomPresentationWidget());
    auto* Canvas = Preview->WidgetTree->ConstructWidget<UCanvasPanel>();
    Canvas->AddChild(Preview->WidgetTree->RootWidget);
    Preview->WidgetTree->RootWidget = Canvas;
    Preview->CustomPresentationSlot = Preview->WidgetTree->ConstructWidget<UNamedSlot>();
    Canvas->AddChildToCanvas(Preview->CustomPresentationSlot);
    const auto CanvasSlate = Canvas->TakeWidget();
    Preview->RefreshPreview();
    auto* Scene = Cast<UCharacterSelect3DPreviewWidget>(Preview->GetCustomPresentationWidget());
    if (!TestNotNull(TEXT("3D source uses the scene widget"), Scene)) return false;
    TestNotNull(TEXT("Scene actor is live in integrated preview"), Scene->GetPreviewActor());
    Preview->SetPreviewSelected(true);
    TestTrue(TEXT("Confirm reaches 3D scene"), Scene->bSelectedState);
    Preview->SetPreviewSelected(false);
    TestFalse(TEXT("Cancel reaches 3D scene"), Scene->bSelectedState);
    Preview->bOverrideSequence = true;
    Preview->Sequence = FCharacterSelectSequence::MakePreset(0);
    Preview->Sequence.ConfirmDelay = .2f;
    Preview->SetPreviewSelected(true);
    TestFalse(TEXT("Sequenced 3D confirmation waits"), Scene->bSelectedState);
    Preview->AdvanceSequenceConfirmation(.25f);
    TestTrue(TEXT("Sequenced confirmation reaches 3D actor"), Scene->bSelectedState);
    Preview->SetPreviewSelected(false);
    TestFalse(TEXT("Sequenced cancel restores 3D idle"), Scene->bSelectedState);
    Preview->bOverrideSequence = false;
    Preview->PreviewAnimationSettings = FCharacterPreviewAnimationSettings::MakePreset(ECharacterPreviewPreset::Fade);
    Preview->SetCharacter(B.Get());
    TestTrue(TEXT("Outgoing scene remains through exit"), Preview->GetCustomPresentationWidget() == Scene);
    Preview->AdvanceConfiguredAnimation(5.f);
    TestTrue(TEXT("Latest character swaps after exit"), Preview->GetCharacter() == B.Get());
    TestNull(TEXT("Switch to 2D removes the viewport"), Preview->GetCustomPresentationWidget());
    TestNull(TEXT("Removed scene destroys its actor"), Scene->GetPreviewActor());
    Preview->SetCharacter(A.Get()); Preview->ClearPreview(); Preview->AdvanceConfiguredAnimation(5.f);
    TestNull(TEXT("Rapid clear wins"), Preview->GetCharacter());
    TestNull(TEXT("Clear removes scene content"), Preview->GetCustomPresentationWidget());
    Preview->NativeDestruct();
    return true;
}
#endif
