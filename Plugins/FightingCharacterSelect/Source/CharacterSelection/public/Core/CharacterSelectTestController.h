#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"

#include "CharacterSelectTestController.generated.h"

class UCharacterSelectRegistry;
class UCharacterSelectLayoutData;
class UCharacterSelectWidget;

UCLASS(Blueprintable)
class CHARACTERSELECTION_API ACharacterSelectTestController : public AActor
{
	GENERATED_BODY()

public:

	ACharacterSelectTestController();
	/** Opt-in example flow. Existing launchers retain their current behavior. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Character Select|Match handoff")
	bool bStoreSelectionOnReady = false;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Character Select|Match handoff", meta=(EditCondition="bStoreSelectionOnReady"))
	bool bTravelOnReady = false;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Character Select|Match handoff", meta=(EditCondition="bStoreSelectionOnReady && bTravelOnReady"))
	TSoftObjectPtr<UWorld> MatchLevel;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Character Select|Match handoff")
	FString LastHandoffError;
	UFUNCTION(BlueprintCallable, Category="Character Select|Match handoff")
	bool CompleteSelectionHandoff();
	UFUNCTION() void HandleSelectionReady();
	UFUNCTION() void HandleSelectionStateChanged();

protected:

	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type Reason) override;

protected:

	/**
	 * Character Select screen widget class.
	 *
	 * Assign:
	 * WBP_CharacterSelect
	 */
	UPROPERTY(
		EditAnywhere,
		BlueprintReadOnly,
		Category = "Character Select|Test"
	)
	TSubclassOf<UCharacterSelectWidget>
		CharacterSelectWidgetClass;

	/**
	 * Character registry containing the characters
	 * used by this Character Select screen.
	 *
	 * Example:
	 * DA_CharacterRegistry_Main
	 */
	UPROPERTY(
		EditAnywhere,
		BlueprintReadOnly,
		Category = "Character Select|Test"
	)
	TObjectPtr<UCharacterSelectRegistry>
		CharacterRegistry;

	/**
	 * Character Select layout/navigation configuration.
	 *
	 * Example:
	 * DA_CharacterSelectLayout
	 */
	UPROPERTY(
		EditAnywhere,
		BlueprintReadOnly,
		Category = "Character Select|Test"
	)
	TObjectPtr<UCharacterSelectLayoutData>
		CharacterLayout;

	/**
	 * Runtime-created Character Select widget.
	 *
	 * This should NOT be assigned manually.
	 */
	UPROPERTY(
		Transient,
		BlueprintReadOnly,
		Category = "Character Select|Runtime"
	)
	TObjectPtr<UCharacterSelectWidget>
		CharacterSelectWidget;

	/**
	 * Player that owns this Character Select screen.
	 *
	 * 0 = Player 1
	 * 1 = Player 2
	 */
	UPROPERTY(
		EditAnywhere,
		BlueprintReadOnly,
		Category = "Character Select|Test",
		meta = (ClampMin = "0")
	)
	int32 PlayerIndex = 0;
};
