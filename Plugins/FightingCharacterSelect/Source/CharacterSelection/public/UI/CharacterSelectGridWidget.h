#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"

#include "Types/CharacterSelectGridTypes.h"

#include "CharacterSelectGridWidget.generated.h"

class UCharacterSelectionController;
class UCharacterSelectSlotWidget;
class UCanvasPanel;

UCLASS(Abstract, Blueprintable)
class CHARACTERSELECTION_API UCharacterSelectGridWidget
	: public UUserWidget
{
	GENERATED_BODY()

public:

	UFUNCTION(BlueprintCallable, Category = "Character Select|Grid")
	void InitializeGrid(
		const TArray<UCharacterSelectionController*>& InControllers
	);

	UFUNCTION(BlueprintCallable, Category = "Character Select|Grid")
	void RebuildGrid();

	UFUNCTION(BlueprintCallable, Category = "Character Select|Grid")
	void RefreshLayout();

	void SetPlayerActive(int32 PlayerIndex, bool bActive);

	UFUNCTION(BlueprintCallable, Category = "Character Select|Grid")
	void SetGridLayout(
		const FCharacterSelectGridLayout& InLayout
	);

	UFUNCTION(BlueprintPure, Category = "Character Select|Grid")
	const FCharacterSelectGridLayout& GetGridLayout() const;

    /** Resolved navigation dimensions without changing the authored GridLayout. */
    FCharacterSelectGridLayout GetEffectiveGridLayout() const;

	UFUNCTION(BlueprintPure, Category = "Character Select|Grid")
	int32 GetCharacterCount() const;

protected:

	virtual void NativeConstruct() override;

	virtual void NativeDestruct() override;


public:

	UPROPERTY(
		EditAnywhere,
		BlueprintReadWrite,
		Category = "Character Select|Grid"
	)
	FCharacterSelectGridLayout GridLayout;

protected:

	UPROPERTY(
		EditDefaultsOnly,
		BlueprintReadOnly,
		Category = "Character Select|Grid"
	)
	TSubclassOf<UCharacterSelectSlotWidget>
		SlotWidgetClass;

	UPROPERTY()
	TArray<TObjectPtr<UCharacterSelectionController>>
		SelectionControllers;

	UPROPERTY()
	TObjectPtr<UCharacterSelectionController>
		SelectionController;

	UPROPERTY(
		BlueprintReadOnly,
		Category = "Character Select|Grid"
	)
	TArray<TObjectPtr<UCharacterSelectSlotWidget>>
		SlotWidgets;

	UPROPERTY(
		meta = (BindWidget)
	)
	TObjectPtr<UCanvasPanel>
		SlotContainer;

private:

	void GenerateSlots();

	void PositionSlots();

	void BindControllerEvents();
	void UnbindControllerEvents();

	UFUNCTION()
	void HandleCharacterFocusChanged(
		UCharacterSelectionController* Controller,
		UCharacterSelectData* Character
	);

	UFUNCTION()
	void HandleCharacterSelected(
		UCharacterSelectionController* Controller,
		UCharacterSelectData* Character
	);

	UFUNCTION()
	void HandleCharacterSelectionCancelled(
		UCharacterSelectionController* Controller
	);

	UFUNCTION()
	void HandleCharacterSelectionConfirmed(
		UCharacterSelectionController* Controller
	);


	void RefreshSlotStates();

	TSet<int32> InactivePlayers;
};