#pragma once
#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "Types/CharacterSelectInputTypes.h"
#include "Types/CharacterSelectFeedback.h"
#include "CharacterSelectJoinSession.generated.h"

class UCharacterSelectionController;

UENUM(BlueprintType)
enum class ECharacterSelectDeviceType : uint8 { None, Keyboard, Gamepad };

USTRUCT(BlueprintType)
struct FCharacterSelectDeviceAssignment
{
	GENERATED_BODY()
	UPROPERTY(BlueprintReadOnly, Category = "Input")
	ECharacterSelectDeviceType Type = ECharacterSelectDeviceType::None;
	UPROPERTY(BlueprintReadOnly, Category = "Input")
	int32 DeviceId = INDEX_NONE;
	UPROPERTY(BlueprintReadOnly, Category = "Input")
	bool bConnected = false;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FCharacterSelectJoinStateChanged);
DECLARE_MULTICAST_DELEGATE_TwoParams(FCharacterSelectFeedbackEvent, ECharacterSelectFeedback, int32);

/** Device ownership and CPU handoff, independent of Slate focus or platform user indices. */
UCLASS(BlueprintType)
class CHARACTERSELECTION_API UCharacterSelectJoinSession : public UObject
{
	GENERATED_BODY()
public:
	UCharacterSelectionController* GetController(int32 Index) const { return Controllers.IsValidIndex(Index) ? Controllers[Index].Get() : nullptr; }
	FCharacterSelectFeedbackEvent OnFeedback;
	bool bAutoCPUOpponent = true;
	bool bAllowLatePlayerJoin = true;
	void Initialize(const TArray<UCharacterSelectionController*>& InControllers);
	bool HandleInput(ECharacterSelectDeviceType Type, int32 DeviceId, ECharacterSelectInputAction Action, bool bCanJoin, bool bRepeat = false);
	void SetGamepadConnected(int32 DeviceId, bool bConnected);

	UFUNCTION(BlueprintPure, Category = "Character Select|Join")
	bool HasJoined(int32 PlayerIndex) const;
	UFUNCTION(BlueprintPure, Category = "Character Select|Join")
	bool IsCPUOpponent() const;
	UFUNCTION(BlueprintPure, Category = "Character Select|Join")
	bool IsReady() const;
	UFUNCTION(BlueprintPure, Category = "Character Select|Join")
	int32 GetPlayerForDevice(ECharacterSelectDeviceType Type, int32 DeviceId) const;
	UFUNCTION(BlueprintPure, Category = "Character Select|Join")
	FCharacterSelectDeviceAssignment GetAssignment(int32 PlayerIndex) const;
	UFUNCTION(BlueprintPure, Category = "Character Select|Join")
	int32 GetPlayerOneControlTarget() const;
	UPROPERTY(BlueprintAssignable, Category = "Character Select|Join")
	FCharacterSelectJoinStateChanged OnStateChanged;
private:
	UPROPERTY()
	TArray<TObjectPtr<UCharacterSelectionController>> Controllers;
	UPROPERTY()
	TArray<FCharacterSelectDeviceAssignment> Assignments;
};
