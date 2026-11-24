#include "UI/CharacterSelect3DPreviewWidget.h"
#include "Blueprint/WidgetTree.h"
#include "Components/Viewport.h"
#include "Components/SceneComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Animation/AnimSequence.h"
#include "Engine/SkeletalMesh.h"
#include "Animation/AnimInstance.h"
#include "Animation/AnimSingleNodeInstance.h"
#include "Animation/SkeletalMeshActor.h"
#include "Engine/World.h"

TSharedRef<SWidget> UCharacterSelect3DPreviewWidget::RebuildWidget()
{
    if (!SceneViewport)
    {
        SceneViewport = WidgetTree->ConstructWidget<UViewport>();
        WidgetTree->RootWidget = SceneViewport;
    }
    return Super::RebuildWidget();
}
void UCharacterSelect3DPreviewWidget::Configure(const FCharacterSelect3DPresentation& InSettings, UCharacterSelectData* InCharacter, int32 PlayerIndex, bool bSelected)
{
    ClearScene();
    Settings = InSettings;
    SetPresentation(InCharacter, PlayerIndex);
    bSelectedState = bSelected;
    if (SceneViewport && SceneViewport->GetViewportWorld()) CreateSceneActor();
}
void UCharacterSelect3DPreviewWidget::NativeConstruct()
{
    Super::NativeConstruct();
    if (!PreviewActor) CreateSceneActor();
}
void UCharacterSelect3DPreviewWidget::CreateSceneActor()
{
    if (IsDesignTime() || !SceneViewport || !SceneViewport->GetViewportWorld()) return;
    const auto Issues = Settings.GetIssues();
    PreviewError = FString::Join(Issues, TEXT("\n"));
    if (!Issues.IsEmpty()) return;
    UWorld* World = SceneViewport->GetViewportWorld();
    SceneViewport->SetBackgroundColor(Settings.BackgroundColor);
    SceneViewport->SetEnableAdvancedFeatures(true);
    SceneViewport->SetLightIntensity(Settings.KeyLightIntensity);
    SceneViewport->SetSkyIntensity(Settings.SkyLightIntensity);
    SceneViewport->SetViewLocation(Settings.CameraLocation);
    SceneViewport->SetViewRotation((Settings.CameraTarget - Settings.CameraLocation).Rotation());
    FActorSpawnParameters Params;
    Params.ObjectFlags |= RF_Transient;
    Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
    UClass* ActorType = Settings.ActorClass ? Settings.ActorClass.Get() : ASkeletalMeshActor::StaticClass();
    PreviewActor = World->SpawnActor<AActor>(ActorType, Settings.ActorTransform, Params);
    if (!PreviewActor) { PreviewError = TEXT("Could not spawn the preview actor."); return; }
    PreviewActor->SetActorEnableCollision(false);
    if (auto* Root = PreviewActor->GetRootComponent()) Root->SetMobility(EComponentMobility::Movable);
    SetPreviewPlayerIndex(PresentationPlayerIndex);
    AnimatedMesh = PreviewActor->FindComponentByClass<USkeletalMeshComponent>();
    if (AnimatedMesh)
    {
        if (Settings.SkeletalMesh) AnimatedMesh->SetSkeletalMeshAsset(Settings.SkeletalMesh);
        AnimatedMesh->VisibilityBasedAnimTickOption = EVisibilityBasedAnimTickOption::AlwaysTickPoseAndRefreshBones;
        AnimatedMesh->SetSimulatePhysics(false);
        if (bStudioAnimationPreview) AnimatedMesh->SetComponentTickEnabled(false);
        OriginalAnimationMode = uint8(AnimatedMesh->GetAnimationMode());
        if (auto* Single = AnimatedMesh->GetSingleNodeInstance())
        {
            OriginalAnimationAsset = Single->GetCurrentAsset();
            bOriginalLoop = Single->IsLooping();
            OriginalRate = Single->GetPlayRate();
        }
        PlayIdle();
        if (bSelectedState && PlayClip(Settings.SelectionAnimation, false)) { bSelectionOverrideActive = true; SelectionRemaining = Settings.SelectionAnimation->GetPlayLength() / Settings.AnimationRate; }
    }
}
void UCharacterSelect3DPreviewWidget::SetPreviewPlayerIndex(int32 PlayerIndex)
{
    PresentationPlayerIndex = FMath::Clamp(PlayerIndex, 0, 1);
    if (PreviewActor)
    {
        FTransform Transform = Settings.ActorTransform;
        if (PresentationPlayerIndex == 1) Transform.SetRotation(FRotator(0, Settings.Player2YawOffset, 0).Quaternion() * Transform.GetRotation());
        PreviewActor->SetActorTransform(Transform);
    }
}
bool UCharacterSelect3DPreviewWidget::PlayClip(UAnimSequence* Clip, bool bLoop)
{
    if (!Clip || !AnimatedMesh || !AnimatedMesh->GetSkeletalMeshAsset()) return false;
    if (Clip->GetSkeleton() != AnimatedMesh->GetSkeletalMeshAsset()->GetSkeleton())
    { PreviewError = TEXT("Preview animation and mesh have different skeletons; clip skipped."); return false; }
    AnimatedMesh->PlayAnimation(Clip, bLoop);
    AnimatedMesh->SetPlayRate(Settings.AnimationRate);
    return true;
}
void UCharacterSelect3DPreviewWidget::PlayIdle()
{
    SelectionRemaining = 0.f;
    if (!PlayClip(Settings.IdleAnimation, true) && AnimatedMesh && bSelectionOverrideActive)
    {
        if (OriginalAnimationMode == uint8(EAnimationMode::AnimationSingleNode) && OriginalAnimationAsset)
        {
            AnimatedMesh->PlayAnimation(OriginalAnimationAsset, bOriginalLoop);
            AnimatedMesh->SetPlayRate(OriginalRate);
        }
        else AnimatedMesh->SetAnimationMode(EAnimationMode::Type(OriginalAnimationMode));
    }
    bSelectionOverrideActive = false;
}
void UCharacterSelect3DPreviewWidget::SetSelected(bool bSelected)
{
    if (bSelectedState == bSelected) return;
    bSelectedState = bSelected;
    if (bSelected && PlayClip(Settings.SelectionAnimation, false)) { bSelectionOverrideActive = true; SelectionRemaining = Settings.SelectionAnimation->GetPlayLength() / Settings.AnimationRate; }
    else if (!bSelected) PlayIdle();
}
void UCharacterSelect3DPreviewWidget::NativeTick(const FGeometry& Geometry, float DeltaTime)
{
    if (bStudioAnimationPreview) return;
    Super::NativeTick(Geometry, DeltaTime);
    if (SelectionRemaining > 0 && FMath::IsFinite(DeltaTime))
    {
        SelectionRemaining -= FMath::Max(0.f, DeltaTime);
        if (SelectionRemaining <= 0) PlayIdle();
    }
}
void UCharacterSelect3DPreviewWidget::ClearScene()
{
    if (IsValid(PreviewActor)) PreviewActor->Destroy();
    PreviewActor = nullptr;
    AnimatedMesh = nullptr;
    SelectionRemaining = 0;
    OriginalAnimationAsset = nullptr;
    bSelectionOverrideActive = false;
}
void UCharacterSelect3DPreviewWidget::NativeDestruct()
{
    ClearScene();
    Super::NativeDestruct();
}

void UCharacterSelect3DPreviewWidget::AdvanceStudioAnimation(float DeltaTime)
{
    if (!bStudioAnimationPreview || !AnimatedMesh || DeltaTime <= 0.f || !FMath::IsFinite(DeltaTime)) return;
    AnimatedMesh->TickAnimation(DeltaTime, false);
    AnimatedMesh->RefreshBoneTransforms();
    AnimatedMesh->MarkRenderDynamicDataDirty();
    if (SelectionRemaining > 0.f)
    {
        SelectionRemaining -= DeltaTime;
        if (SelectionRemaining <= 0.f) PlayIdle();
    }
}
