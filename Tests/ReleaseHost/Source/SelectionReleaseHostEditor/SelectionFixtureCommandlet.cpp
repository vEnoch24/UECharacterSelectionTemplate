#include "SelectionFixtureCommandlet.h"
#include "CharacterSelectSetup.h"
#include "WidgetBlueprint.h"
#include "Data/CharacterSelectData.h"
#include "Data/CharacterSelectRegistry.h"
#include "Core/CharacterSelectTestController.h"
#include "Core/CharacterSelectMatchReceiver.h"
#include "Engine/World.h"
#include "Engine/Texture2D.h"
#include "GameFramework/WorldSettings.h"
#include "GameFramework/GameModeBase.h"
#include "Kismet2/KismetEditorUtilities.h"
#include "Kismet2/BlueprintEditorUtils.h"
#include "FileHelpers.h"
#include "Misc/PackageName.h"
#include "UObject/SavePackage.h"
#include "UObject/StrongObjectPtr.h"

int32 USelectionFixtureCommandlet::Main(const FString& Params)
{
    TStrongObjectPtr<UCharacterSelectSetupOptions> Options(NewObject<UCharacterSelectSetupOptions>());
    Options->DestinationFolder = TEXT("/Game/SelectionVerification");
    Options->SetupName = TEXT("Release");
    FCharacterSelectSetupResult Result;
    FString Error;
    if (!CharacterSelectSetup::Create(Options.Get(), Result, Error))
    { UE_LOG(LogTemp, Error, TEXT("Fixture creation failed: %s"), *Error); return 1; }
    for (int32 I = 0; I < 2; ++I)
    {
        const FString Name = FString::Printf(TEXT("DA_Fighter%d"), I + 1);
        auto* Character = NewObject<UCharacterSelectData>(CreatePackage(*(Options->DestinationFolder / Name)), *Name, RF_Public | RF_Standalone);
        Character->CharacterID = *Name;
        Character->DisplayName = FText::FromString(FString::Printf(TEXT("Fighter %d"), I + 1));
        Character->Subtitle = FText::FromString(TEXT("Release test placeholder"));
        Character->Portrait = LoadObject<UTexture2D>(nullptr, TEXT("/Engine/EngineResources/DefaultTexture.DefaultTexture"));
        if (!Character->Portrait) { UE_LOG(LogTemp, Error, TEXT("Engine placeholder texture is missing")); return 5; }
        Character->LargePortrait = Character->Portrait;
        Result.Roster->Characters.Add(Character);
        Result.CreatedAssets.Add(Character);
    }
    auto* LauncherDefaults = Cast<ACharacterSelectTestController>(Result.Launcher->GeneratedClass->GetDefaultObject());
    LauncherDefaults->bStoreSelectionOnReady = true;
    LauncherDefaults->bTravelOnReady = true;
    LauncherDefaults->MatchLevel = TSoftObjectPtr<UWorld>(FSoftObjectPath(TEXT("/Game/SelectionVerification/Destination.Destination")));
    FBlueprintEditorUtils::MarkBlueprintAsModified(Result.Launcher);
    FKismetEditorUtilities::CompileBlueprint(Result.Launcher);
    if (Result.Launcher->Status == BS_Error) return 6;
    for (UObject* Asset : Result.CreatedAssets)
    {
        FSavePackageArgs Args;
        Args.TopLevelFlags = RF_Public | RF_Standalone;
        const FString File = FPackageName::LongPackageNameToFilename(Asset->GetOutermost()->GetName(), FPackageName::GetAssetPackageExtension());
        if (!UPackage::SavePackage(Asset->GetOutermost(), Asset, *File, Args)) return 2;
    }
    for (bool bDestination : {true, false})
    {
        const FString Map = Options->DestinationFolder / (bDestination ? TEXT("Destination") : TEXT("Selection"));
        if (FPackageName::DoesPackageExist(Map)) return 3;
        UWorld* World = UWorld::CreateWorld(EWorldType::Editor, false, bDestination ? TEXT("Destination") : TEXT("Selection"), CreatePackage(*Map));
        World->GetWorldSettings()->DefaultGameMode = AGameModeBase::StaticClass();
        if (bDestination) World->SpawnActor<ACharacterSelectMatchReceiver>();
        else World->SpawnActor<ACharacterSelectTestController>(Result.Launcher->GeneratedClass);
        if (!FEditorFileUtils::SaveMap(World, FPackageName::LongPackageNameToFilename(Map, FPackageName::GetMapPackageExtension()))) return 4;
        World->DestroyWorld(false);
    }
    UE_LOG(LogTemp, Display, TEXT("CHARSELECT_FIXTURE_SAVED: seven setup assets, two characters and two maps"));
    return 0;
}
