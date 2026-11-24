#pragma once
#include "Core/CharacterSelectGridNavigation.h"
#include "CharacterSelectLinearNavigation.generated.h"

/** Single-axis navigation with skipping and optional wrapping. */
UCLASS()
class CHARACTERSELECTION_API UCharacterSelectLinearNavigation : public UCharacterSelectGridNavigation
{
	GENERATED_BODY()
public:
	virtual int32 GetNextIndex_Implementation(int32 CurrentIndex, ECharacterSelectDirection Direction) const override;
};
