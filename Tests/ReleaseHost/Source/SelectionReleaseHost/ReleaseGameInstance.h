#pragma once
#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "Containers/Ticker.h"
#include "ReleaseGameInstance.generated.h"

// Test-host-only driver; never included in the distributed plugin.
UCLASS()
class UReleaseGameInstance : public UGameInstance
{
    GENERATED_BODY()
public:
    virtual void Init() override;
    virtual void Shutdown() override;
private:
    bool TickSmoke(float Delta);
    FTSTicker::FDelegateHandle Handle;
    double Started = 0;
    double NextAction = 0;
    int32 Step = 0;
    FString Mode;
    bool bScreenshot = false;
    FGuid ExpectedID;
    TArray<FName> ExpectedCharacters;
};
