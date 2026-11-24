#pragma once
#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Core/CharacterSelectJoinSession.h"
#include "CharacterSelectMatchSubsystem.generated.h"

class UCharacterSelectData;
class UCharacterSelectRegistry;
class UCharacterSelectWidget;
class ACharacter;

USTRUCT(BlueprintType)
struct FCharacterSelectMatchPlayer
{
	GENERATED_BODY()
	UPROPERTY(BlueprintReadOnly, Category="Match") int32 PlayerIndex = 0;
	UPROPERTY(BlueprintReadOnly, Category="Match") FName CharacterID;
	UPROPERTY(BlueprintReadOnly, Category="Match") TSoftObjectPtr<UCharacterSelectData> CharacterAsset;
	UPROPERTY(BlueprintReadOnly, Category="Match") TSoftClassPtr<ACharacter> CharacterClass;
	UPROPERTY(BlueprintReadOnly, Category="Match") bool bIsCPU = false;
	/** Snapshot only. Device IDs are local to this running process, not local-player indices. */
	UPROPERTY(BlueprintReadOnly, Category="Match") FCharacterSelectDeviceAssignment Device;
	UPROPERTY(BlueprintReadOnly, Category="Match") int32 CostumeIndex = 0;
	UPROPERTY(BlueprintReadOnly, Category="Match") int32 ColorIndex = 0;
};

USTRUCT(BlueprintType)
struct FCharacterSelectMatchResult
{
	GENERATED_BODY()
	UPROPERTY(BlueprintReadOnly, Category="Match") FGuid SelectionID;
	UPROPERTY(BlueprintReadOnly, Category="Match") TArray<FCharacterSelectMatchPlayer> Players;
};

/** Process-local match snapshot, retained across map travel in the same Game Instance. */
UCLASS()
class CHARACTERSELECTION_API UCharacterSelectMatchSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()
public:
	/** Failure clears stale pending data and returns an actionable message. */
	UFUNCTION(BlueprintCallable, Category="Character Select|Match")
	bool StoreFromScreen(UCharacterSelectWidget* Screen, FString& Error);
	UFUNCTION(BlueprintPure, Category="Character Select|Match")
	bool HasSelection() const { return bHasSelection; }
	/** Returns a copy. False produces an empty result. */
	UFUNCTION(BlueprintCallable, Category="Character Select|Match")
	bool GetSelection(FCharacterSelectMatchResult& Result) const;
	UFUNCTION(BlueprintCallable, Category="Character Select|Match")
	bool ConsumeSelection(FCharacterSelectMatchResult& Result);
	UFUNCTION(BlueprintCallable, Category="Character Select|Match")
	void ClearSelection();
	// Native entry point used by the screen adapter and integration tests.
	bool StoreFromSession(UCharacterSelectJoinSession* Session, const UCharacterSelectRegistry* Registry, FString& Error);
private:
	UPROPERTY(Transient) FCharacterSelectMatchResult Pending;
	UPROPERTY(Transient) bool bHasSelection = false;
};
