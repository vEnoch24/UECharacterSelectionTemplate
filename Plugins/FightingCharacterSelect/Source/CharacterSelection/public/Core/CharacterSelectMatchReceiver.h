#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Core/CharacterSelectMatchSubsystem.h"
#include "CharacterSelectMatchReceiver.generated.h"

/** Example destination-level actor. Read the snapshot without spawning fighters or stealing devices. */
UCLASS(Blueprintable)
class CHARACTERSELECTION_API ACharacterSelectMatchReceiver : public AActor
{
	GENERATED_BODY()
public:
	/** Consume after reading when this receiver is the single destination for the pending match. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Character Select|Match") bool bConsumeOnRead = false;
	UPROPERTY(BlueprintReadOnly, Category="Character Select|Match") FCharacterSelectMatchResult MatchSelection;
	UPROPERTY(BlueprintReadOnly, Category="Character Select|Match") bool bReceivedSelection = false;
	UFUNCTION(BlueprintCallable, Category="Character Select|Match") bool ReadMatchSelection();
	UFUNCTION(BlueprintImplementableEvent, Category="Character Select|Match") void OnMatchSelectionReceived(const FCharacterSelectMatchResult& Result);
protected:
	virtual void BeginPlay() override;
};
