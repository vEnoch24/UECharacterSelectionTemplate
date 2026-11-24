
#pragma once

#include "CoreMinimal.h"
#include "CharacterSelectPlayerTypes.generated.h"

UENUM(BlueprintType)
enum class ECharacterSelectPlayerType : uint8
{
	Human UMETA(DisplayName = "Human"),
	CPU UMETA(DisplayName = "CPU")
};

USTRUCT(BlueprintType)
struct FCharacterSelectPlayerConfig
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Player")
	int32 PlayerIndex = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Player")
	ECharacterSelectPlayerType PlayerType =
		ECharacterSelectPlayerType::Human;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Player")
	FText DisplayName;

	// Index within the controller's available-character roster.
	// INDEX_NONE uses the player's default starting position.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Player")
	int32 InitialFocusIndex = INDEX_NONE;

	FCharacterSelectPlayerConfig()
	{
		DisplayName = FText::FromString(TEXT("Player 1"));
	}
};
