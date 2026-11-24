#pragma once
#include "CoreMinimal.h"
#include "Core/CharacterSelectNavigationProvider.h"
#include "CharacterSelectGridNavigation.generated.h"

UCLASS()
class CHARACTERSELECTION_API UCharacterSelectGridNavigation : public UCharacterSelectNavigationProvider
{
	GENERATED_BODY()
public:
	virtual void Initialize_Implementation(int32 InItemCount, const FCharacterSelectNavigationConfig& Config) override;
	virtual int32 GetNextIndex_Implementation(int32 CurrentIndex, ECharacterSelectDirection Direction) const override;
	virtual void SetNavigableIndices_Implementation(const TArray<int32>& InNavigableIndices) override;
protected:
	int32 ItemCount = 0;
	FCharacterSelectNavigationConfig NavigationConfig;
	TSet<int32> NavigableIndices;
};
