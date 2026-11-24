#pragma once
#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Types/CharacterSelectSequence.h"
#include "CharacterSelectPresentationWidget.generated.h"
class UCharacterSelectData;

/** Base for a custom portrait region. Its owner handles the name/subtitle and selection logic. */
UCLASS(Blueprintable)
class CHARACTERSELECTION_API UCharacterSelectPresentationWidget : public UUserWidget
{
    GENERATED_BODY()
public:
    UPROPERTY(BlueprintReadOnly, Transient, Category="Character Select")
    TObjectPtr<UCharacterSelectData> PresentedCharacter;
    UPROPERTY(BlueprintReadOnly, Transient, Category="Character Select")
    int32 PresentationPlayerIndex = 0;
    /** Respond with custom 2D/3D behavior. Only emitted when sequencing is enabled. */
    UFUNCTION(BlueprintImplementableEvent, Category="Character Select")
    void OnSequenceEvent(ECharacterPresentationEvent Event);
    void SetPresentation(UCharacterSelectData* InCharacter, int32 InPlayerIndex);
    /** Update your artwork or custom animation from this event. Gameplay only. */
    UFUNCTION(BlueprintImplementableEvent, Category="Character Select")
    void OnPresentationChanged(UCharacterSelectData* InCharacter, int32 InPlayerIndex);
};
