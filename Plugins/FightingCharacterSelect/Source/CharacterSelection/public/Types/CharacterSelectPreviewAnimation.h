#pragma once
#include "CoreMinimal.h"
#include "Types/CharacterSelectSlotAnimation.h"
#include "CharacterSelectPreviewAnimation.generated.h"

UENUM(BlueprintType)
enum class ECharacterPreviewAnimationMode : uint8
{
    Authored UMETA(DisplayName="Existing Blueprint animations"),
    Configured UMETA(DisplayName="Independent portrait and text animations")
};
UENUM(BlueprintType)
enum class ECharacterPreviewPreset : uint8 { Fade, Slide, Staggered, Zoom, NoMotion };

/** Entrance starts at this appearance; exit ends at this appearance. */
USTRUCT(BlueprintType)
struct CHARACTERSELECTION_API FCharacterPreviewTransition
{
    GENERATED_BODY()
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Timing", meta=(ClampMin="0", ClampMax="5", Units="s"))
    float Duration = 0.2f;
    /** Delay from the beginning of this phase, in seconds. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Timing", meta=(ClampMin="0", ClampMax="5", Units="s"))
    float Delay = 0.f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Timing")
    ECharacterSlotEasing Easing = ECharacterSlotEasing::SmoothStep;
    /** Multiplies the element's Designer scale. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Appearance", meta=(ClampMin="0.01", ClampMax="4"))
    float Scale = 1.f;
    /** Offset from the element's Designer translation, in Slate units. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Appearance")
    FVector2D Translation = FVector2D::ZeroVector;
    /** Multiplies the element's Designer opacity. Zero fades completely out. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Appearance", meta=(ClampMin="0", ClampMax="1"))
    float Opacity = 0.f;
};
USTRUCT(BlueprintType)
struct CHARACTERSELECTION_API FCharacterPreviewElementAnimation
{
    GENERATED_BODY()
    /** Disabled elements change content at the common swap point without motion. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Animation")
    bool bEnabled = true;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Animation", meta=(EditCondition="bEnabled"))
    FCharacterPreviewTransition Entrance;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Animation", meta=(EditCondition="bEnabled"))
    FCharacterPreviewTransition Exit;
};
USTRUCT(BlueprintType)
struct CHARACTERSELECTION_API FCharacterPreviewAnimationSettings
{
    GENERATED_BODY()
    /** Original UMG timelines remain the default and are never rewritten. */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Animation")
    ECharacterPreviewAnimationMode Mode = ECharacterPreviewAnimationMode::Authored;
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Elements", meta=(EditCondition="Mode == ECharacterPreviewAnimationMode::Configured", EditConditionHides))
    FCharacterPreviewElementAnimation Portrait;
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Elements", meta=(EditCondition="Mode == ECharacterPreviewAnimationMode::Configured", EditConditionHides))
    FCharacterPreviewElementAnimation Name;
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Elements", meta=(EditCondition="Mode == ECharacterPreviewAnimationMode::Configured", EditConditionHides))
    FCharacterPreviewElementAnimation Subtitle;
    static FCharacterPreviewAnimationSettings MakePreset(ECharacterPreviewPreset Preset);
};
