#pragma once
#include "CoreMinimal.h"
#include "Types/CharacterSelectPreviewAnimation.h"
#include "Engine/DataAsset.h"
#include "CharacterSelectSequence.generated.h"

UENUM(BlueprintType)
enum class ECharacterSequenceOrder : uint8 { Together, PortraitFirst, TextFirst };
UENUM(BlueprintType)
enum class ECharacterPresentationEvent : uint8 { Exit, Entrance, Ready, Confirm, Cancel };

/** A complete override. Disabled explicitly restores the preview's existing animation settings. */
USTRUCT(BlueprintType)
struct CHARACTERSELECTION_API FCharacterSelectSequence
{
    GENERATED_BODY()
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Sequence", meta=(DisplayName="Use sequencing"))
    bool bEnabled = false;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Sequence", meta=(EditCondition="bEnabled"))
    ECharacterSequenceOrder EntranceOrder = ECharacterSequenceOrder::Together;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Sequence", meta=(EditCondition="bEnabled"))
    ECharacterSequenceOrder ExitOrder = ECharacterSequenceOrder::Together;
    /** Pause between the portrait and text groups. Their individual delays still apply. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Sequence", meta=(ClampMin="0", ClampMax="5", Units="s", EditCondition="bEnabled"))
    float GroupGap = .05f;
    /** The entrance finishes before confirmation playback begins. Cancel discards a waiting confirm. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Confirmation", meta=(EditCondition="bEnabled"))
    bool bWaitForEntrance = true;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Confirmation", meta=(ClampMin="0", ClampMax="5", Units="s", EditCondition="bEnabled"))
    float ConfirmDelay = 0.f;
    /** Restart sprite frames when confirmation is delivered. Static artwork is unaffected. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="2D playback", meta=(EditCondition="bEnabled"))
    bool bRestartSpriteOnConfirm = true;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="2D playback", meta=(EditCondition="bEnabled"))
    bool bRestartSpriteOnCancel = true;
    /** Entrance and exit appearance for each element. Sequencing uses these configured transitions. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Transitions", meta=(EditCondition="bEnabled"))
    FCharacterPreviewAnimationSettings Transitions = FCharacterPreviewAnimationSettings::MakePreset(ECharacterPreviewPreset::Fade);
    static FCharacterSelectSequence MakePreset(int32 Preset);
    FCharacterPreviewAnimationSettings GetTransitions(bool bExit) const;
};

/** Create through Content Browser > Miscellaneous > Data Asset, then share between player previews. */
UCLASS(BlueprintType)
class CHARACTERSELECTION_API UCharacterSelectSequenceAsset : public UDataAsset
{
    GENERATED_BODY()
public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Animation sequencing")
    FCharacterSelectSequence Sequence;
};
