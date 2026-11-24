#pragma once

#include "CoreMinimal.h" 

#include "CharacterSelectRosterTypes.generated.h" 

class UCharacterSelectData;

UENUM(BlueprintType)
enum class ECharacterSelectSlotType : uint8
{
	Character,
	Random,
	Empty,
	Special
};

USTRUCT(BlueprintType)
struct FCharacterSelectRosterEntry
{
	GENERATED_BODY()

public:

	/**
  * Character represented by this slot.
  * Can be null for Random, Empty or Special slots.
  */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Roster")
	TObjectPtr<UCharacterSelectData> Character = nullptr;

	/**
	 * Determines what this slot represents.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Roster")
	ECharacterSelectSlotType SlotType =
		ECharacterSelectSlotType::Character;

	/**
	 * Can the player's cursor move onto this slot?
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Roster")
	bool bNavigable = true;

	/**
	 * Can this slot actually be selected?
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Roster")
	bool bSelectable = true;

	/**
	 * Should the slot be visually displayed?
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Roster")
	bool bVisible = true;
};
