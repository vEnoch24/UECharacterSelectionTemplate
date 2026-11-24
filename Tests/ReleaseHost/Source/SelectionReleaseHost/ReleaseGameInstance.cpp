#include "ReleaseGameInstance.h"
#include "Blueprint/WidgetBlueprintLibrary.h"
#include "UI/CharacterSelectWidget.h"
#include "Core/CharacterSelectMatchSubsystem.h"
#include "Core/CharacterSelectMatchReceiver.h"
#include "Data/CharacterSelectData.h"
#include "EngineUtils.h"
#include "Engine/World.h"
#include "Engine/GameViewportClient.h"
#include "UnrealClient.h"
#include "Misc/CommandLine.h"
#include "Misc/Parse.h"

void UReleaseGameInstance::Init()
{
    Super::Init();
    if (!FParse::Value(FCommandLine::Get(), TEXT("SelectionSmoke="), Mode)) return;
    Started = FPlatformTime::Seconds();
    NextAction = Started + 5;
    Handle = FTSTicker::GetCoreTicker().AddTicker(FTickerDelegate::CreateUObject(this, &UReleaseGameInstance::TickSmoke), .2f);
}
void UReleaseGameInstance::Shutdown()
{
    FTSTicker::GetCoreTicker().RemoveTicker(Handle);
    Super::Shutdown();
}
bool UReleaseGameInstance::TickSmoke(float Delta)
{
    auto Finish = [](bool bPassed, const FString& Reason)
    {
        UE_LOG(LogTemp, Display, TEXT("CHARSELECT_RELEASE_%s: %s"), bPassed ? TEXT("PASS") : TEXT("FAIL"), *Reason);
        FPlatformMisc::RequestExitWithStatus(true, bPassed ? 0 : 1);
        return false;
    };
    const double Now = FPlatformTime::Seconds();
    if (Now - Started > 60) return Finish(false, TEXT("Timed out waiting for selection or destination receiver"));
    if (!GetWorld() || Now < NextAction) return true;
    if (Mode != TEXT("CPU") && Mode != TEXT("Mixed") && Mode != TEXT("Pads")) return Finish(false, TEXT("Unknown smoke mode"));
    const bool bCPU = Mode == TEXT("CPU");
    auto* Store = GetSubsystem<UCharacterSelectMatchSubsystem>();
    if (Step >= 5)
    {
        for (TActorIterator<ACharacterSelectMatchReceiver> It(GetWorld()); It; ++It)
        {
            if (!It->bReceivedSelection) return true;
            const auto& Result = It->MatchSelection;
            bool bValid = Result.SelectionID == ExpectedID && Result.Players.Num() == 2 && ExpectedCharacters.Num() == 2;
            if (bValid) for (int32 I = 0; I < 2; ++I)
                bValid &= Result.Players[I].CharacterID == ExpectedCharacters[I] && Result.Players[I].CharacterAsset.LoadSynchronous() != nullptr;
            bValid &= Result.Players.Num() == 2 && Result.Players[1].bIsCPU == bCPU;
            if (Result.Players.Num() == 2)
            {
                const auto& First = Result.Players[0];
                const auto& Second = Result.Players[1];
                const bool bTwoPads = Mode == TEXT("Pads");
                bValid &= !First.bIsCPU && First.PlayerIndex == 0 && Second.PlayerIndex == 1;
                bValid &= First.Device.Type == (bTwoPads ? ECharacterSelectDeviceType::Gamepad : ECharacterSelectDeviceType::Keyboard);
                bValid &= First.Device.DeviceId == (bTwoPads ? 6 : 0) && First.Device.bConnected;
                bValid &= bCPU
                    ? Second.Device.Type == ECharacterSelectDeviceType::None && !Second.Device.bConnected
                    : Second.Device.Type == ECharacterSelectDeviceType::Gamepad && Second.Device.DeviceId == 7 && Second.Device.bConnected;
            }
            FCharacterSelectMatchResult Consumed;
            bValid &= Store && Store->ConsumeSelection(Consumed) && !Store->HasSelection();
            return Finish(bValid, Mode + TEXT(" selection survived cooked level travel; receiver and soft assets checked"));
        }
        return true;
    }
    TArray<UUserWidget*> Widgets;
    UWidgetBlueprintLibrary::GetAllWidgetsOfClass(GetWorld(), Widgets, UCharacterSelectWidget::StaticClass(), true);
    if (Widgets.Num() != 1) return true;
    auto* Screen = Cast<UCharacterSelectWidget>(Widgets[0]);
    auto* Join = Screen->GetJoinSession();
    if (!Join) return true;
    using D = ECharacterSelectDeviceType;
    using A = ECharacterSelectInputAction;
    const D P1 = Mode == TEXT("Pads") ? D::Gamepad : D::Keyboard;
    const int32 P1ID = P1 == D::Gamepad ? 6 : 0;
    if (Step == 0) Join->HandleInput(P1, P1ID, A::Confirm, true);
    if (Step == 1) Join->HandleInput(bCPU ? P1 : D::Gamepad, bCPU ? P1ID : 7, bCPU ? A::NavigateRight : A::Confirm, true);
    if (Step == 2) Join->HandleInput(P1, P1ID, A::Confirm, true);
    if (Step == 3)
    {
        Join->HandleInput(bCPU ? P1 : D::Gamepad, bCPU ? P1ID : 7, A::NavigateRight, true);
        if (FParse::Param(FCommandLine::Get(), TEXT("SelectionScreenshot")))
        {
            FScreenshotRequest::RequestScreenshot(TEXT("SelectionRelease.png"), true, false);
            bScreenshot = true;
        }
    }
    if (Step == 4)
    {
        Join->HandleInput(bCPU ? P1 : D::Gamepad, bCPU ? P1ID : 7, A::Confirm, true);
        FCharacterSelectMatchResult Snapshot;
        if (!Store || !Store->GetSelection(Snapshot) || Snapshot.Players.Num() != 2)
            return Finish(false, TEXT("Launcher did not store a ready selection"));
        ExpectedID = Snapshot.SelectionID;
        for (const auto& Player : Snapshot.Players) ExpectedCharacters.Add(Player.CharacterID);
    }
    ++Step;
    NextAction = Now + (bScreenshot && Step == 4 ? 3 : .5);
    return true;
}
