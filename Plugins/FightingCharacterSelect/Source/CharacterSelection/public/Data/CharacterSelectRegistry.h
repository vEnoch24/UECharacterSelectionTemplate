// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "CharacterSelectRegistry.generated.h"

class UCharacterSelectData;

UCLASS()
class CHARACTERSELECTION_API UCharacterSelectRegistry : public UDataAsset
{
	GENERATED_BODY()

public:

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Characters")
	TArray<TObjectPtr<UCharacterSelectData>> Characters;

	UFUNCTION(BlueprintPure, Category = "Character Select")
	const TArray<UCharacterSelectData*>& GetAllCharacters() const;

	UFUNCTION(BlueprintPure, Category = "Character Select")
	UCharacterSelectData* GetCharacterByID(FName CharacterID) const;

	UFUNCTION(BlueprintPure, Category = "Character Select")
	TArray<UCharacterSelectData*> GetCharactersByTag(FName Tag) const;


	UFUNCTION(BlueprintPure, Category = "Character Select")
	TArray<UCharacterSelectData*> GetAvailableCharacters() const;


	UFUNCTION(BlueprintPure, Category = "Character Select")
	TArray<UCharacterSelectData*> GetCharactersSorted() const;

#if WITH_EDITOR
	virtual EDataValidationResult IsDataValid(
		FDataValidationContext& Context) const override;
#endif

};
