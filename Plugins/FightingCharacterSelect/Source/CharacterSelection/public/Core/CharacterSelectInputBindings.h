#pragma once
#include "CoreMinimal.h"
#include "InputCoreTypes.h"
#include "Types/CharacterSelectInputTypes.h"
#include "Types/CharacterSelectCustomization.h"

struct CHARACTERSELECTION_API FCharacterSelectInputBindings
{
	static ECharacterSelectInputAction GetAction(FKey Key);
	static ECharacterSelectInputAction GetAction(FKey Key, const FCharacterSelectControlSettings& Settings);
	static bool CanJoin(FKey Key);
};
