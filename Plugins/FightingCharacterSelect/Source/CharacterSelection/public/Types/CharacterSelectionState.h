// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CharacterSelectionState.generated.h"

/**
 *
 */
USTRUCT(BlueprintType)
struct FCharacterSelectionState
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Character Select|State")
	int32 PlayerIndex = 0;

	/** Current roster position being focused. */
	UPROPERTY(BlueprintReadOnly, Category = "Character Select|State")
	int32 FocusedCharacterIndex = INDEX_NONE;

	/** Roster position of the selected character. */
	UPROPERTY(BlueprintReadOnly, Category = "Character Select|State")
	int32 SelectedCharacterIndex = INDEX_NONE;

	UPROPERTY(BlueprintReadOnly, Category = "Character Select|State")
	int32 CostumeIndex = 0;

	UPROPERTY(BlueprintReadOnly, Category = "Character Select|State")
	int32 ColorIndex = 0;

	UPROPERTY(BlueprintReadOnly, Category = "Character Select|State")
	bool bHasSelectedCharacter = false;

	UPROPERTY(BlueprintReadOnly, Category = "Character Select|State")
	bool bIsConfirmed = false;
};

//class CHARACTERSELECTION_API CharacterSelectionState
//{
//public:
//	/*CharacterSelectionState();
//	~CharacterSelectionState();*/
//};
