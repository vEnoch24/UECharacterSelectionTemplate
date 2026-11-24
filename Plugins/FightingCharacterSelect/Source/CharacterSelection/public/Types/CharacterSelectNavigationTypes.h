// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Types/CharacterSelectGridTypes.h"
#include "CharacterSelectNavigationTypes.generated.h"


UENUM(BlueprintType)
enum class ECharacterSelectNavigationMode : uint8
{
	Grid,
	Horizontal,
	Vertical,
	Radial UMETA(Hidden),
	Custom UMETA(Hidden)
};

UENUM(BlueprintType)
enum class ECharacterSelectEdgeBehavior : uint8
{
	Stop,
	Wrap,
	Clamp
};

USTRUCT(BlueprintType)
struct FCharacterSelectNavigationConfig
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Navigation")
	ECharacterSelectNavigationMode Mode =
		ECharacterSelectNavigationMode::Grid;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Navigation")
	int32 Columns = 7;

	// Supplied by the displayed grid; Columns above remains the navigation column count.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Navigation")
	FCharacterSelectGridLayout GridLayout;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Navigation")
	ECharacterSelectEdgeBehavior HorizontalEdgeBehavior =
		ECharacterSelectEdgeBehavior::Wrap;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Navigation")
	ECharacterSelectEdgeBehavior VerticalEdgeBehavior =
		ECharacterSelectEdgeBehavior::Stop;
};

class CHARACTERSELECTION_API CharacterSelectNavigationTypes
{
public:
	//CharacterSelectNavigationTypes();
	//~CharacterSelectNavigationTypes();
};
