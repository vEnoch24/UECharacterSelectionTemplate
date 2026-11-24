#pragma once
#include "CoreMinimal.h"
#include "Types/CharacterSelectCustomization.h"
#include "Types/CharacterSelectInputTypes.h"
#include "Styling/SlateBrush.h"
#include "CharacterSelectFeedback.generated.h"

class USoundBase;

UENUM(BlueprintType)
enum class ECharacterSelectFeedback : uint8 { Joined, Reconnected, Moved, Confirmed, Cancelled, Rejected };

USTRUCT(BlueprintType)
struct FCharacterSelectSoundSettings
{
	GENERATED_BODY()
	/** Unassigned events are silent. Use short, non-looping UI sounds. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Sounds")
	TMap<ECharacterSelectFeedback, TObjectPtr<USoundBase>> Sounds;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Sounds", meta=(ClampMin="0", ClampMax="2"))
	float Volume = 1.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Sounds", meta=(ClampMin="0.1", ClampMax="4"))
	float Pitch = 1.0f;
};

USTRUCT(BlueprintType)
struct CHARACTERSELECTION_API FCharacterSelectHintSettings
{
	GENERATED_BODY()
	/** Adds an action row to generated banners; custom UMG banners use the Blueprint getters. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Hints")
	bool bShowActionHints = false;
	/** Keyed by the actual keyboard/gamepad key. Unassigned icons use readable key labels. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Hints")
	TMap<FKey, FSlateBrush> Glyphs;
	/** Optional alternate controller artwork for Player 2. Hardware brand is not auto-detected. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Hints")
	TMap<FKey, FSlateBrush> Player2GamepadGlyphs;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Hints")
	TMap<FKey, FText> KeyLabels;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Hints")
	TMap<ECharacterSelectInputAction, FText> ActionLabels;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Hints", meta=(ClampMin="8", ClampMax="128"))
	float IconSize = 24.0f;
	static FKey ResolveKey(const FCharacterSelectControlSettings& Controls, ECharacterSelectInputAction Action, bool bGamepad);
	const FSlateBrush* FindGlyph(FKey Key, int32 DeviceOwner) const;
	FText GetKeyLabel(FKey Key) const;
	FText GetActionLabel(ECharacterSelectInputAction Action) const;
};
