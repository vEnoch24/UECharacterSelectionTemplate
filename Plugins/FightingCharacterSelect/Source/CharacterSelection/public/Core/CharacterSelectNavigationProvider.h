#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "Types/CharacterSelectNavigationTypes.h"
#include "Types/CharacterSelectTypes.h"
#include "CharacterSelectNavigationProvider.generated.h"

UCLASS(Abstract, Blueprintable, BlueprintType)
class CHARACTERSELECTION_API UCharacterSelectNavigationProvider : public UObject
{
	GENERATED_BODY()
public:
	static UCharacterSelectNavigationProvider* CreateForMode(UObject* Outer, ECharacterSelectNavigationMode Mode);
	static bool IsModeSupported(ECharacterSelectNavigationMode Mode);

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Character Select|Navigation")
	void Initialize(int32 ItemCount, const FCharacterSelectNavigationConfig& Config);
	virtual void Initialize_Implementation(int32 ItemCount, const FCharacterSelectNavigationConfig& Config) {}

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Character Select|Navigation")
	int32 GetNextIndex(int32 CurrentIndex, ECharacterSelectDirection Direction) const;
	virtual int32 GetNextIndex_Implementation(int32 CurrentIndex, ECharacterSelectDirection Direction) const { return CurrentIndex; }

	// An explicitly empty list means there are no eligible destinations.
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Character Select|Navigation")
	void SetNavigableIndices(const TArray<int32>& InNavigableIndices);
	virtual void SetNavigableIndices_Implementation(const TArray<int32>& InNavigableIndices) {}
};
