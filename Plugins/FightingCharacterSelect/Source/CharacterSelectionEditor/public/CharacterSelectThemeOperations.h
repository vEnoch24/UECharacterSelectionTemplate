#pragma once
#include "CoreMinimal.h"
class UCharacterSelectTheme;
class UCharacterSelectWidget;
class UCharacterSelectGridWidget;
class UCharacterSelectPreviewWidget;
class UCharacterSelectLayoutData;
class UCharacterSelectSlotWidget;

struct FCharacterSelectThemeTargets
{
	UCharacterSelectWidget* Screen = nullptr;
	UCharacterSelectGridWidget* Grid = nullptr;
	UCharacterSelectLayoutData* Layout = nullptr;
	TArray<UCharacterSelectPreviewWidget*> Previews;
    UCharacterSelectSlotWidget* Slot = nullptr;
};

namespace CharacterSelectThemeOperations
{
	bool Capture(UCharacterSelectTheme* Theme, const FCharacterSelectThemeTargets& Targets, FString& Error);
	// Validates every enabled section before changing any object. Caller owns the transaction.
	bool Apply(const UCharacterSelectTheme* Theme, const FCharacterSelectThemeTargets& Targets, FString& Error);
}
