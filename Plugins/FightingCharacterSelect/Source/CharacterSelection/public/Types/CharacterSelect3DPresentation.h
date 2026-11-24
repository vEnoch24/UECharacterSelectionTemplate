#pragma once
#include "CoreMinimal.h"
#include "CharacterSelect3DPresentation.generated.h"
class AActor;
class USkeletalMesh;
class UAnimSequence;
UENUM(BlueprintType)
enum class ECharacterPreviewMode : uint8 { Inherit, Artwork2D, Actor3D };

USTRUCT(BlueprintType)
struct CHARACTERSELECTION_API FCharacterSelect3DPresentation
{
    GENERATED_BODY()
    /** Dedicated cosmetic actor; spawned in a separate world, never possessed. Empty uses Skeletal Mesh. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Actor")
    TSubclassOf<AActor> ActorClass;
    /** Optional mesh override for the first skeletal mesh component of the actor. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Actor")
    TObjectPtr<USkeletalMesh> SkeletalMesh;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Actor")
    FTransform ActorTransform = FTransform::Identity;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Actor")
    float Player2YawOffset = 0.f;
    /** Camera and target are world-space positions inside the isolated preview scene. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Camera")
    FVector CameraLocation = FVector(300,0,100);
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Camera")
    FVector CameraTarget = FVector(0,0,90);
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Lighting", meta=(ClampMin="0", ClampMax="20"))
    float KeyLightIntensity = 3.f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Lighting", meta=(ClampMin="0", ClampMax="20"))
    float SkyLightIntensity = 1.f;
    /** The embedded viewport has an opaque background. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Lighting")
    FLinearColor BackgroundColor = FLinearColor(.025f,.025f,.025f,1);
    /** Empty preserves the actor's animation Blueprint. Clips require the same skeleton as the mesh. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Animation")
    TObjectPtr<UAnimSequence> IdleAnimation;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Animation")
    TObjectPtr<UAnimSequence> SelectionAnimation;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Animation", meta=(ClampMin="0.1", ClampMax="4"))
    float AnimationRate = 1.f;
    TArray<FString> GetIssues() const;
};
