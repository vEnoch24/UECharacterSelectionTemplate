#pragma once
#include "CoreMinimal.h"
#include "Commandlets/Commandlet.h"
#include "SelectionFixtureCommandlet.generated.h"
UCLASS()
class USelectionFixtureCommandlet : public UCommandlet
{
    GENERATED_BODY()
public:
    virtual int32 Main(const FString& Params) override;
};
