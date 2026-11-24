#pragma once

#include "CoreMinimal.h"
#include "CharacterSelectTypes.h"
#include "CharacterSelectInputTypes.generated.h"

UENUM(BlueprintType)
enum class ECharacterSelectInputAction : uint8
{
	None,

	NavigateUp,
	NavigateDown,
	NavigateLeft,
	NavigateRight,

	Confirm,
	Cancel
};