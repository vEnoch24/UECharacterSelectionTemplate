// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Types/CharacterSelectTypes.h"
#include "Types/CharacterSelect2DPresentation.h"
#include "Types/CharacterSelect3DPresentation.h"
#include "Types/CharacterSelectSequence.h"
#include "CharacterSelectData.generated.h"

class UTexture2D;
class ACharacter;

/**
 *
 */
UCLASS()
class CHARACTERSELECTION_API UCharacterSelectData : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	/** Unique identifier used internally by the selection system. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Character")
	FName CharacterID;

	/** Name displayed to the player. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Character")
	FText DisplayName;

	/** Optional subtitle such as "The World Warrior". */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Character")
	FText Subtitle;

	/** Small portrait used by character slots. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Visuals")
	TObjectPtr<UTexture2D> Portrait;

	/** Larger artwork used by the preview system. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Visuals")
	TObjectPtr<UTexture2D> LargePortrait;

    /** Otherwise the preview widget's defaults are used. */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="2D presentation")
    bool bOverridePresentation2D = false;
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="2D presentation", meta=(EditCondition="bOverridePresentation2D"))
    FCharacterSelect2DPresentation Presentation2D;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="3D presentation")
    ECharacterPreviewMode PreviewMode = ECharacterPreviewMode::Inherit;
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="3D presentation")
    bool bOverridePresentation3D = false;
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="3D presentation", meta=(EditCondition="bOverridePresentation3D"))
    FCharacterSelect3DPresentation Presentation3D;

    /** Highest priority; replaces the complete player/shared sequence. */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Animation sequencing")
    bool bOverrideSequence = false;
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Animation sequencing", meta=(EditCondition="bOverrideSequence"))
    FCharacterSelectSequence Sequence;

	/** Character gameplay class. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Gameplay")
	TSoftClassPtr<ACharacter> CharacterClass;

	/** Current availability state. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Selection")
	ECharacterAvailability Availability = ECharacterAvailability::Available;

	/** Determines ordering in the character roster. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Selection")
	int32 SortPriority = 0;

	/** Arbitrary tags for filtering and categorization. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Selection")
	TArray<FName> Tags;

#if WITH_EDITOR
	virtual EDataValidationResult IsDataValid(
		FDataValidationContext& Context) const override;
#endif
};
