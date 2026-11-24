#pragma once
#include "CoreMinimal.h"
#include "CharacterSelectDirectionalLinks.generated.h"

UENUM(BlueprintType)
enum class ECharacterSelectLinkRule : uint8
{
	Automatic,
	Stop,
	Character UMETA(DisplayName="Go to Character")
};

USTRUCT(BlueprintType)
struct FCharacterSelectDirectionalLink
{
	GENERATED_BODY()
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Link")
	ECharacterSelectLinkRule Rule = ECharacterSelectLinkRule::Automatic;
	/** Stable Character ID from the destination character asset; not its display name or array index. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Link", meta=(EditCondition="Rule == ECharacterSelectLinkRule::Character", EditConditionHides))
	FName TargetCharacterID;
};

USTRUCT(BlueprintType)
struct FCharacterSelectDirectionalLinks
{
	GENERATED_BODY()
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Directions")
	FCharacterSelectDirectionalLink Up;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Directions")
	FCharacterSelectDirectionalLink Down;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Directions")
	FCharacterSelectDirectionalLink Left;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Directions")
	FCharacterSelectDirectionalLink Right;
};
