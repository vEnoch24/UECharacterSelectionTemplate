#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "Types/CharacterSelectInputTypes.h"
#include "CharacterSelectInputController.generated.h"

class UCharacterSelectionController;

UCLASS(BlueprintType)
class CHARACTERSELECTION_API UCharacterSelectInputController
	: public UObject
{
	GENERATED_BODY()

public:

	void Initialize(
		UCharacterSelectionController* InSelectionController
	);

	void ProcessAction(
		ECharacterSelectInputAction Action
	);

	UFUNCTION(BlueprintCallable, Category = "Character Select|Input")
	void Navigate(
		ECharacterSelectDirection Direction
	);

	UFUNCTION(BlueprintCallable, Category = "Character Select|Input")
	void Confirm();

	UFUNCTION(BlueprintCallable, Category = "Character Select|Input")
	void Cancel();

	UFUNCTION(BlueprintPure, Category = "Character Select|Input")
	UCharacterSelectionController*
		GetSelectionController() const;

private:

	UPROPERTY()
	TObjectPtr<UCharacterSelectionController>
		SelectionController;
};