#pragma once
#include "CoreMinimal.h"
#include "CharacterSelect2DPresentation.generated.h"
class UTexture2D;
class UCharacterSelectPresentationWidget;

UENUM(BlueprintType)
enum class ECharacter2DPresentationMode : uint8 { StaticArtwork, SpriteFrames, CustomWidget };
UENUM(BlueprintType)
enum class ECharacter2DFacing : uint8 { AsAuthored, Mirror, MirrorPlayer2 };

/** Optional presentation settings; identity defaults preserve the original portrait. */
USTRUCT(BlueprintType)
struct CHARACTERSELECTION_API FCharacterSelect2DPresentation
{
    GENERATED_BODY()
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Source")
    ECharacter2DPresentationMode Mode = ECharacter2DPresentationMode::StaticArtwork;
    /** Empty uses the character's Large Portrait, then Portrait. Also used as fallback. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Source")
    TObjectPtr<UTexture2D> Artwork;
    /** One transparent texture per frame, in playback order. Uniform canvas sizes are recommended. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Sprite animation", meta=(EditCondition="Mode == ECharacter2DPresentationMode::SpriteFrames", EditConditionHides))
    TArray<TObjectPtr<UTexture2D>> Frames;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Sprite animation", meta=(ClampMin="0.1", ClampMax="120", EditCondition="Mode == ECharacter2DPresentationMode::SpriteFrames", EditConditionHides))
    float FramesPerSecond = 12.f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Sprite animation", meta=(EditCondition="Mode == ECharacter2DPresentationMode::SpriteFrames", EditConditionHides))
    bool bLoop = true;
    /** Requires a Named Slot called CustomPresentationSlot in the preview Blueprint. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Custom widget", meta=(EditCondition="Mode == ECharacter2DPresentationMode::CustomWidget", EditConditionHides))
    TSubclassOf<UCharacterSelectPresentationWidget> WidgetClass;
    /** Offset in logical UMG units, relative to the authored portrait/host. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Framing")
    FVector2D Offset = FVector2D::ZeroVector;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Framing", meta=(ClampMin="0.01", ClampMax="10"))
    FVector2D Scale = FVector2D(1,1);
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Framing")
    ECharacter2DFacing Facing = ECharacter2DFacing::AsAuthored;
    int32 GetFrameIndex(double Seconds) const;
    TArray<FString> GetIssues() const;
};
