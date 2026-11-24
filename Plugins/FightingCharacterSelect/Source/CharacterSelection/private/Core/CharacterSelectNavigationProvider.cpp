#include "Core/CharacterSelectNavigationProvider.h"
#include "Core/CharacterSelectGridNavigation.h"
#include "Core/CharacterSelectLinearNavigation.h"

bool UCharacterSelectNavigationProvider::IsModeSupported(ECharacterSelectNavigationMode Mode)
{
	return Mode == ECharacterSelectNavigationMode::Grid || Mode == ECharacterSelectNavigationMode::Horizontal || Mode == ECharacterSelectNavigationMode::Vertical;
}

UCharacterSelectNavigationProvider* UCharacterSelectNavigationProvider::CreateForMode(UObject* Outer, ECharacterSelectNavigationMode Mode)
{
	switch (Mode)
	{
	case ECharacterSelectNavigationMode::Horizontal:
	case ECharacterSelectNavigationMode::Vertical:
		return NewObject<UCharacterSelectLinearNavigation>(Outer);
	case ECharacterSelectNavigationMode::Grid:
		return NewObject<UCharacterSelectGridNavigation>(Outer);
	default:
		UE_LOG(LogTemp, Warning, TEXT("Unsupported character navigation mode; falling back to Grid."));
		return NewObject<UCharacterSelectGridNavigation>(Outer);
	}
}
