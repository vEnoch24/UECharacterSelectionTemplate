#include "Types/CharacterSelect3DPresentation.h"
#include "GameFramework/Actor.h"
#include "Engine/SkeletalMesh.h"
#include "Animation/AnimSequence.h"
TArray<FString> FCharacterSelect3DPresentation::GetIssues() const
{
    TArray<FString> Issues;
    if (!ActorClass && !SkeletalMesh) Issues.Add(TEXT("Assign a preview Actor Class or Skeletal Mesh."));
    if (ActorClass && ActorClass->HasAnyClassFlags(CLASS_Abstract)) Issues.Add(TEXT("Preview Actor Class cannot be abstract."));
    if (CameraLocation.ContainsNaN() || CameraTarget.ContainsNaN() || CameraLocation.Equals(CameraTarget)) Issues.Add(TEXT("Camera location and target must be finite and different."));
    if (!ActorTransform.IsValid() || ActorTransform.GetScale3D().GetAbsMin() < .001 || !FMath::IsFinite(Player2YawOffset)) Issues.Add(TEXT("Actor framing requires a finite transform and nonzero scale."));
    if (!FMath::IsFinite(KeyLightIntensity) || !FMath::IsFinite(SkyLightIntensity) || KeyLightIntensity < 0 || SkyLightIntensity < 0 || KeyLightIntensity > 20 || SkyLightIntensity > 20) Issues.Add(TEXT("Light intensities must be between 0 and 20."));
    if (!FMath::IsFinite(AnimationRate) || AnimationRate < .1f || AnimationRate > 4.f) Issues.Add(TEXT("Animation rate must be between 0.1 and 4."));
    for (float Channel : {BackgroundColor.R, BackgroundColor.G, BackgroundColor.B, BackgroundColor.A})
        if (!FMath::IsFinite(Channel)) { Issues.Add(TEXT("Background color must be finite.")); break; }
    for (auto* Clip : {IdleAnimation.Get(), SelectionAnimation.Get()})
        if (Clip && SkeletalMesh && Clip->GetSkeleton() != SkeletalMesh->GetSkeleton()) Issues.Add(TEXT("Preview clips and mesh must use the same skeleton."));
    return Issues;
}
