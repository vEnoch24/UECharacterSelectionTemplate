#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"

#include "Types/CharacterSelectNavigationTypes.h"
#include "Types/CharacterSelectDirectionalLinks.h"
#include "Layout/Margin.h" 

#include "CharacterSelectLayoutData.generated.h"

UCLASS(BlueprintType)
class CHARACTERSELECTION_API UCharacterSelectLayoutData : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	TArray<FString> GetDirectionalLinkIssues(const class UCharacterSelectRegistry* Registry) const;
	/** Override individual moves while retaining the selected mode for Automatic directions. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Directional links")
	bool bUseDirectionalLinks = false;
	/** Key = source Character ID. Links are one-way. Missing entries use automatic navigation. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Directional links", meta=(EditCondition="bUseDirectionalLinks"))
	TMap<FName, FCharacterSelectDirectionalLinks> DirectionalLinks;

	// =========================================================
	// Navigation
	// =========================================================

	UPROPERTY(
		EditDefaultsOnly,
		BlueprintReadOnly,
		Category = "Navigation"
	)
	ECharacterSelectNavigationMode NavigationMode =
		ECharacterSelectNavigationMode::Grid;

	UPROPERTY(
		EditDefaultsOnly,
		BlueprintReadOnly,
		Category = "Navigation",
		meta = (ClampMin = "1")
	)
	int32 Columns = 7;

	UPROPERTY(
		EditDefaultsOnly,
		BlueprintReadOnly,
		Category = "Navigation",
		meta = (ClampMin = "0")
	)
	int32 Rows = 0;

	UPROPERTY(
		EditDefaultsOnly,
		BlueprintReadOnly,
		Category = "Navigation"
	)
	ECharacterSelectEdgeBehavior HorizontalEdgeBehavior =
		ECharacterSelectEdgeBehavior::Wrap;

	UPROPERTY(
		EditDefaultsOnly,
		BlueprintReadOnly,
		Category = "Navigation"
	)
	ECharacterSelectEdgeBehavior VerticalEdgeBehavior =
		ECharacterSelectEdgeBehavior::Stop;


	// =========================================================
	// Visual Layout
	// =========================================================

	UPROPERTY(
		EditDefaultsOnly,
		BlueprintReadOnly,
		Category = "Visual Layout"
	)
	FVector2D SlotSize = FVector2D(120.0f, 120.0f);

	UPROPERTY(
		EditDefaultsOnly,
		BlueprintReadOnly,
		Category = "Visual Layout"
	)
	FVector2D SlotSpacing = FVector2D(8.0f, 8.0f);

	UPROPERTY(
		EditDefaultsOnly,
		BlueprintReadOnly,
		Category = "Visual Layout"
	)
	FMargin GridPadding = FMargin(0.0f);


	// =========================================================
	// Roster Behavior
	// =========================================================

	UPROPERTY(
		EditDefaultsOnly,
		BlueprintReadOnly,
		Category = "Roster"
	)
	bool bAllowEmptySlots = true;

	UPROPERTY(
		EditDefaultsOnly,
		BlueprintReadOnly,
		Category = "Roster"
	)
	bool bAllowLockedSlots = true;

	UPROPERTY(
		EditDefaultsOnly,
		BlueprintReadOnly,
		Category = "Roster"
	)
	bool bAllowHiddenSlots = false;
};
