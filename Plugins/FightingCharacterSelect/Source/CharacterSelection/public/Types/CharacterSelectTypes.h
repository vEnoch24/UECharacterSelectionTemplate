// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/ObjectMacros.h"

/**
 *
 */

UENUM(BlueprintType)
enum class ECharacterAvailability : uint8
{
	Available,
	Locked,
	Hidden,
	DLC,
	ComingSoon
};

UENUM(BlueprintType)
enum class ECharacterSelectionState : uint8
{
	None,
	Focused,
	Selected,
	Confirmed
};

UENUM(BlueprintType)
enum class ECharacterSelectDirection : uint8
{
	Up,
	Down,
	Left,
	Right
};

class CHARACTERSELECTION_API CharacterSelectTypes
{
public:
	//CharacterSelectTypes();
	//~CharacterSelectTypes();
};
