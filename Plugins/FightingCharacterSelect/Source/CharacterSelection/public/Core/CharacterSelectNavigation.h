// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"

#include "Types/CharacterSelectNavigationTypes.h"
#include "Types/CharacterSelectTypes.h"

#include "CharacterSelectNavigation.generated.h"


UINTERFACE(BlueprintType)
class CHARACTERSELECTION_API UCharacterSelectNavigation : public UInterface
{
	GENERATED_BODY()
};

class CHARACTERSELECTION_API ICharacterSelectNavigation
{
	GENERATED_BODY()

public:

	virtual void Initialize(
		int32 ItemCount,
		const FCharacterSelectNavigationConfig& Config) = 0;

	virtual int32 GetNextIndex(
		int32 CurrentIndex,
		ECharacterSelectDirection Direction) const = 0;
};
