
#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "Core/CharacterSelectInterface.h"
#include "Types/CharacterSelectionState.h"
#include "Types/CharacterSelectRosterTypes.h"
#include "Core/CharacterSelectNavigationProvider.h"
#include "Core/CharacterSelectGridNavigation.h"
#include "CharacterSelectionController.generated.h"


class UCharacterSelectData;
class UCharacterSelectRegistry;
class UCharacterSelectLayoutData;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(
	FOnCharacterFocusChanged,
	UCharacterSelectionController*,
	Controller,
	UCharacterSelectData*,
	Character
);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(
	FOnCharacterSelected,
	UCharacterSelectionController*,
	Controller,
	UCharacterSelectData*,
	Character
);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(
	FOnCharacterSelectionConfirmed,
	UCharacterSelectionController*,
	Controller
);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(
	FOnCharacterSelectionCancelled,
	UCharacterSelectionController*,
	Controller
);

UCLASS(BlueprintType)
class CHARACTERSELECTION_API UCharacterSelectionController
	: public UObject,
	public ICharacterSelectInterface
{
	GENERATED_BODY()

public:

	UFUNCTION(BlueprintCallable, Category = "Character Select")
	void Initialize(
		UCharacterSelectRegistry* InRegistry,
		UCharacterSelectLayoutData* InLayout,
		int32 InPlayerIndex,
		int32 InInitialFocusIndex = -1);

	UFUNCTION(BlueprintCallable, Category = "Character Select")
	void MoveSelection(
		ECharacterSelectDirection Direction);

	UFUNCTION(BlueprintCallable, Category = "Character Select")
	bool FocusCharacterAtIndex(int32 RosterIndex);

	UFUNCTION(BlueprintCallable, Category = "Character Select")
	void SelectCharacter();

	UFUNCTION(BlueprintCallable, Category = "Character Select")
	void ConfirmSelection();

	UFUNCTION(BlueprintCallable, Category = "Character Select")
	void CancelSelection();

	UFUNCTION(BlueprintPure, Category = "Character Select")
	UCharacterSelectData* GetFocusedCharacter() const;

	UFUNCTION(BlueprintPure, Category = "Character Select")
	int32 GetCurrentIndex() const;

	UFUNCTION(BlueprintPure, Category = "Character Select")
	int32 GetPlayerIndex() const;

	UFUNCTION(BlueprintPure, Category = "Character Select")
	bool IsInitialized() const;

	UFUNCTION(BlueprintPure, Category = "Character Select")
	bool IsConfirmed() const;

	UFUNCTION(BlueprintPure, Category = "Character Select")
	UCharacterSelectData* GetSelectedCharacter() const;

	UFUNCTION(BlueprintCallable, Category = "Character Select")
	void SetNavigationProvider(
		UCharacterSelectNavigationProvider* InNavigationProvider);

	UFUNCTION(BlueprintPure, Category = "Character Select")
	const TArray<FCharacterSelectRosterEntry>& GetRosterEntries() const;

	UFUNCTION(BlueprintPure, Category = "Character Select")
	bool HasSelectedCharacter() const;

	UFUNCTION(BlueprintPure, Category = "Character Select")
	const FCharacterSelectionState& GetSelectionState() const;

public:

	UPROPERTY(BlueprintAssignable, Category = "Character Select|Events")
	FOnCharacterFocusChanged OnCharacterFocusChanged;

	UPROPERTY(BlueprintAssignable, Category = "Character Select|Events")
	FOnCharacterSelected OnCharacterSelected;

	UPROPERTY(BlueprintAssignable, Category = "Character Select|Events")
	FOnCharacterSelectionConfirmed OnCharacterSelectionConfirmed;

	UPROPERTY(BlueprintAssignable, Category = "Character Select|Events")
	FOnCharacterSelectionCancelled OnCharacterSelectionCancelled;

public:
	/** Keep directional movement in sync with the displayed slot arrangement. */
	void ConfigureNavigationLayout(const FCharacterSelectGridLayout& InLayout);
	ECharacterSelectNavigationMode GetNavigationMode() const;

private:
	FCharacterSelectNavigationConfig BuildNavigationConfig() const;
	FCharacterSelectGridLayout DisplayLayout;
	bool bHasDisplayLayout = false;

	UPROPERTY()
	TObjectPtr<UCharacterSelectRegistry> Registry;

	UPROPERTY()
	FCharacterSelectionState SelectionState;

	UPROPERTY()
	TArray<FCharacterSelectRosterEntry> RosterEntries;

	UPROPERTY()
	TObjectPtr<UCharacterSelectLayoutData> LayoutData;

	UPROPERTY()
	TObjectPtr<UCharacterSelectNavigationProvider> NavigationProvider;

	int32 CurrentIndex = INDEX_NONE;

	bool bInitialized = false;
};
