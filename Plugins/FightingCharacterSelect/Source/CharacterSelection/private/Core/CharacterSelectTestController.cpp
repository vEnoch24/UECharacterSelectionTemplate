#include "Core/CharacterSelectTestController.h"
#include "Core/CharacterSelectMatchSubsystem.h"
#include "Engine/GameInstance.h"
#include "Kismet/GameplayStatics.h"
#include "Misc/PackageName.h"

#include "Blueprint/UserWidget.h"

#include "UI/CharacterSelectWidget.h"
#include "Data/CharacterSelectRegistry.h"
#include "Data/CharacterSelectLayoutData.h"
#include <GameFramework/Actor.h>


ACharacterSelectTestController::ACharacterSelectTestController()
{
	PrimaryActorTick.bCanEverTick = false;
}


void ACharacterSelectTestController::BeginPlay()
{
	Super::BeginPlay();

	/*
	 * The test actor needs a Character Select widget class.
	 *
	 * Usually this will be:
	 *
	 * WBP_CharacterSelect
	 */
	if (!CharacterSelectWidgetClass)
	{
		UE_LOG(
			LogTemp,
			Error,
			TEXT(
				"Character Select Test Controller: "
				"CharacterSelectWidgetClass is not assigned."
			)
		);

		return;
	}

	/*
	 * The test also needs a character registry.
	 *
	 * This is the Data Asset containing the roster.
	 */
	if (!CharacterRegistry)
	{
		UE_LOG(
			LogTemp,
			Error,
			TEXT(
				"Character Select Test Controller: "
				"CharacterRegistry is not assigned."
			)
		);

		return;
	}

	/*
	 * Create the Character Select UI.
	 *
	 * The Player Controller is used as the owning object
	 * because the widget ultimately belongs to the player's
	 * UI/viewport.
	 */
	APlayerController* PlayerController =
		GetWorld()
		? GetWorld()->GetFirstPlayerController()
		: nullptr;

	if (!PlayerController)
	{
		UE_LOG(
			LogTemp,
			Error,
			TEXT(
				"Character Select Test Controller: "
				"Could not find Player Controller."
			)
		);

		return;
	}

	CharacterSelectWidget =
		CreateWidget<UCharacterSelectWidget>(
			PlayerController,
			CharacterSelectWidgetClass
		);

	if (!CharacterSelectWidget)
	{
		UE_LOG(
			LogTemp,
			Error,
			TEXT(
				"Character Select Test Controller: "
				"Failed to create Character Select Widget."
			)
		);

		return;
	}

	/*
	 * Add the Character Select screen to the viewport.
	 */
	CharacterSelectWidget->AddToViewport();

	/*
	 * Now initialize the Character Select system.
	 *
	 * IMPORTANT:
	 *
	 * We are NOT creating the SelectionController here.
	 *
	 * UCharacterSelectWidget owns that responsibility.
	 */
	CharacterSelectWidget->InitializeCharacterSelect(
		CharacterRegistry,
		CharacterLayout,
		PlayerIndex
	);
	if (bStoreSelectionOnReady)
	{
		if (auto* Store = GetGameInstance()->GetSubsystem<UCharacterSelectMatchSubsystem>()) Store->ClearSelection();
		CharacterSelectWidget->OnPlayersReady.AddDynamic(this, &ACharacterSelectTestController::HandleSelectionReady);
		if (auto* Session = CharacterSelectWidget->GetJoinSession()) Session->OnStateChanged.AddDynamic(this, &ACharacterSelectTestController::HandleSelectionStateChanged);
	}


	UE_LOG(
		LogTemp,
		Log,
		TEXT(
			"Character Select Test Controller: "
			"Character Select initialized successfully."
		)
	);
}

void ACharacterSelectTestController::HandleSelectionReady()
{
	if (bStoreSelectionOnReady) CompleteSelectionHandoff();
}

void ACharacterSelectTestController::HandleSelectionStateChanged()
{
	if (bStoreSelectionOnReady && CharacterSelectWidget && CharacterSelectWidget->GetJoinSession() && !CharacterSelectWidget->GetJoinSession()->IsReady())
		if (auto* Store = GetGameInstance()->GetSubsystem<UCharacterSelectMatchSubsystem>()) Store->ClearSelection();
}

bool ACharacterSelectTestController::CompleteSelectionHandoff()
{
	LastHandoffError.Reset();
	auto* Store = GetGameInstance() ? GetGameInstance()->GetSubsystem<UCharacterSelectMatchSubsystem>() : nullptr;
	if (!Store) { LastHandoffError = TEXT("No Game Instance match store is available."); return false; }
	if (!Store->StoreFromScreen(CharacterSelectWidget, LastHandoffError))
	{ UE_LOG(LogTemp, Warning, TEXT("Match handoff: %s"), *LastHandoffError); return false; }
	if (bTravelOnReady)
	{
		const FString Package = MatchLevel.ToSoftObjectPath().GetLongPackageName();
		if (MatchLevel.IsNull() || !FPackageName::DoesPackageExist(Package))
		{
			LastHandoffError = TEXT("Select an existing Match Level and include it in packaged builds. The snapshot was stored; the menu remains open.");
			UE_LOG(LogTemp, Warning, TEXT("Match handoff: %s"), *LastHandoffError); return false;
		}
		CharacterSelectWidget->RemoveFromParent();
		UGameplayStatics::OpenLevelBySoftObjectPtr(this, MatchLevel);
	}
	return true;
}

void ACharacterSelectTestController::EndPlay(const EEndPlayReason::Type Reason)
{
	if (CharacterSelectWidget)
	{
		CharacterSelectWidget->OnPlayersReady.RemoveDynamic(this, &ACharacterSelectTestController::HandleSelectionReady);
		if (auto* Session = CharacterSelectWidget->GetJoinSession()) Session->OnStateChanged.RemoveDynamic(this, &ACharacterSelectTestController::HandleSelectionStateChanged);
		CharacterSelectWidget->RemoveFromParent(); CharacterSelectWidget = nullptr;
	}
	Super::EndPlay(Reason);
}
