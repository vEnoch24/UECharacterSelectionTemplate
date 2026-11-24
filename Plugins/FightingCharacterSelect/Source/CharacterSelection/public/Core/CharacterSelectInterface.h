// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "Types/CharacterSelectTypes.h"
#include "CharacterSelectInterface.generated.h"


UINTERFACE(BlueprintType)
class CHARACTERSELECTION_API UCharacterSelectInterface : public UInterface
{
	GENERATED_BODY()
};

class CHARACTERSELECTION_API ICharacterSelectInterface
{
	GENERATED_BODY()
public:

	virtual void MoveSelection(
		ECharacterSelectDirection Direction) = 0;

	virtual void SelectCharacter() = 0;

	virtual void ConfirmSelection() = 0;

	virtual void CancelSelection() = 0;
};
