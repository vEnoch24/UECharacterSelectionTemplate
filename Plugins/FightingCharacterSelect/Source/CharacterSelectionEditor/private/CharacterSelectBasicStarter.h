#pragma once
#include "CoreMinimal.h"
class UCharacterSelectSetupOptions;
class UWidgetBlueprint;
struct FCharacterSelectSetupResult;

// Creates editable project assets using only native widgets and engine resources.
UWidgetBlueprint* CreateCharacterSelectBasicStarter(const UCharacterSelectSetupOptions* Options,
    FCharacterSelectSetupResult& Result, FString& Error);
