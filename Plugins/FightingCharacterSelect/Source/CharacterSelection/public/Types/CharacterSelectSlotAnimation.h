#pragma once
#include "CoreMinimal.h"
#include "CharacterSelectSlotAnimation.generated.h"

UENUM(BlueprintType)
enum class ECharacterSlotAnimationMode : uint8
{
    Authored UMETA(DisplayName="Existing Blueprint animations"),
    Configured UMETA(DisplayName="Configurable state animations")
};

UENUM(BlueprintType)
enum class ECharacterSlotEasing : uint8 { Linear, EaseOut, SmoothStep };

UENUM(BlueprintType)
enum class ECharacterSlotAnimationPreset : uint8 { Subtle, Pop, Lift, Fade, NoMotion };

/** Appearance relative to the slot widget's Designer transform, opacity and tint. */
USTRUCT(BlueprintType)
struct CHARACTERSELECTION_API FCharacterSlotAnimationState
{
    GENERATED_BODY()
    /** Seconds to reach this state. Zero applies it immediately. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Transition", meta=(ClampMin="0", ClampMax="5", Units="s"))
    float Duration = 0.15f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Transition")
    ECharacterSlotEasing Easing = ECharacterSlotEasing::SmoothStep;
    /** Multiplier: 1 keeps the Designer size. Layout spacing does not change. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Appearance", meta=(ClampMin="0.01", ClampMax="4"))
    float Scale = 1.f;
    /** Offset in Slate units, added to the Designer translation. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Appearance")
    FVector2D Translation = FVector2D::ZeroVector;
    /** Multiplies the Designer opacity. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Appearance", meta=(ClampMin="0", ClampMax="1"))
    float Opacity = 1.f;
    /** Multiplies the entire slot's colors, including its portrait and player markers. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Appearance")
    FLinearColor Tint = FLinearColor::White;
};

USTRUCT(BlueprintType)
struct CHARACTERSELECTION_API FCharacterSlotAnimationSettings
{
    GENERATED_BODY()
    /** Existing projects keep their authored timelines until explicitly switched. */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Animation")
    ECharacterSlotAnimationMode Mode = ECharacterSlotAnimationMode::Authored;
    /** Return target when neither player focuses or selects this slot. */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="States", meta=(EditCondition="Mode == ECharacterSlotAnimationMode::Configured", EditConditionHides))
    FCharacterSlotAnimationState Idle;
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="States", meta=(EditCondition="Mode == ECharacterSlotAnimationMode::Configured", EditConditionHides))
    FCharacterSlotAnimationState Focused;
    /** Both players occupy this slot and neither has selected it. */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="States", meta=(EditCondition="Mode == ECharacterSlotAnimationMode::Configured", EditConditionHides))
    FCharacterSlotAnimationState SharedFocus;
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="States", meta=(EditCondition="Mode == ECharacterSlotAnimationMode::Configured", EditConditionHides))
    FCharacterSlotAnimationState Selected;
    /** Both players occupy this slot and at least one has selected it. */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="States", meta=(EditCondition="Mode == ECharacterSlotAnimationMode::Configured", EditConditionHides))
    FCharacterSlotAnimationState SharedSelection;
    /** Locked appearance takes priority over focus and selection. Player markers still update. */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="States", meta=(EditCondition="Mode == ECharacterSlotAnimationMode::Configured", EditConditionHides))
    FCharacterSlotAnimationState Locked;
    /** Time to return to the current focus/idle state when the last selection is cancelled. */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Cancellation", meta=(ClampMin="0", ClampMax="5", Units="s", EditCondition="Mode == ECharacterSlotAnimationMode::Configured", EditConditionHides))
    float CancelDuration = 0.18f;

    FCharacterSlotAnimationSettings();
    static FCharacterSlotAnimationSettings MakePreset(ECharacterSlotAnimationPreset Preset);
};
