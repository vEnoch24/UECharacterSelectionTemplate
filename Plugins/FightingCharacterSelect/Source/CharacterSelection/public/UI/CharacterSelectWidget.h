#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Input/Reply.h"
#include "InputCoreTypes.h"

#include "Types/CharacterSelectPlayerTypes.h"
#include "Core/CharacterSelectInputController.h"
#include "Core/CharacterSelectJoinSession.h"
#include "Types/CharacterSelectCustomization.h"
#include "CharacterSelectWidget.generated.h"

class IInputProcessor;
class SWidget;
class UGameViewportClient;
class UTextBlock;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FCharacterSelectPlayersReady);
class UImage;
class UCharacterSelectData;
class UCharacterSelectRegistry;
class UCharacterSelectLayoutData;
class UCharacterSelectGridWidget;
class UCharacterSelectionController;
class UCharacterSelectInputController;
class UCharacterSelectPreviewWidget;

UCLASS(Abstract, Blueprintable)
class CHARACTERSELECTION_API UCharacterSelectWidget
	: public UUserWidget
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Character Select|Feedback")
	FCharacterSelectSoundSettings SoundSettings;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Character Select|Feedback")
	FCharacterSelectHintSettings HintSettings;
	UFUNCTION(BlueprintPure, Category="Character Select|Feedback")
	FKey GetActionHintKey(int32 PlayerIndex, ECharacterSelectInputAction Action) const;
	UFUNCTION(BlueprintPure, Category="Character Select|Feedback")
	FText GetActionHintLabel(int32 PlayerIndex, ECharacterSelectInputAction Action) const;
	UFUNCTION(BlueprintPure, Category="Character Select|Feedback")
	FSlateBrush GetActionHintBrush(int32 PlayerIndex, ECharacterSelectInputAction Action) const;
	/** Result-driven hook for custom visual feedback. PlayerIndex identifies the affected selection slot. */
	UFUNCTION(BlueprintImplementableEvent, Category="Character Select|Feedback")
	void OnSelectionFeedback(ECharacterSelectFeedback Feedback, int32 PlayerIndex);
	UFUNCTION(BlueprintCallable, Category = "Character Select")
	void InitializeCharacterSelect(
		UCharacterSelectRegistry* InRegistry,
		UCharacterSelectLayoutData* InLayout,
		int32 PlayerIndex);

	UFUNCTION(BlueprintPure, Category = "Character Select")
	UCharacterSelectionController* GetSelectionController() const;

	UFUNCTION(BlueprintPure, Category = "Character Select")
	UCharacterSelectRegistry* GetRegistry() const;

	UFUNCTION(BlueprintPure, Category = "Character Select")
	UCharacterSelectLayoutData* GetLayoutData() const;

	UFUNCTION(BlueprintPure, Category = "Character Select|Players")
	const FCharacterSelectPlayerConfig& GetPlayerConfig(
		int32 PlayerIndex) const;

	UFUNCTION(BlueprintPure, Category = "Character Select|Players")
	bool IsCPUPlayer(int32 PlayerIndex) const;

	UFUNCTION(BlueprintPure, Category = "Character Select|Players")
	int32 GetActiveControlPlayerIndex() const;

	UFUNCTION(BlueprintPure, Category = "Character Select|Join")
	UCharacterSelectJoinSession* GetJoinSession() const { return JoinSession; }

	UFUNCTION(BlueprintPure, BlueprintNativeEvent, Category = "Character Select|Join")
	FText GetJoinPrompt(int32 PlayerIndex) const;
	virtual FText GetJoinPrompt_Implementation(int32 PlayerIndex) const;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Select|Join", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float JoinPromptMinOpacity = 0.55f;

	/** Pulse speed in radians per second; zero disables pulsing. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Select|Join", meta = (ClampMin = "0.0"))
	float JoinPromptPulseSpeed = 4.0f;

	UFUNCTION(BlueprintPure, Category = "Character Select|Players")
	UCharacterSelectionController* GetPlayerSelectionController(int32 PlayerIndex) const;

	UPROPERTY(BlueprintAssignable, Category = "Character Select|Join")
	FCharacterSelectPlayersReady OnPlayersReady;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Character Select|Controls")
	FCharacterSelectControlSettings ControlSettings;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Character Select|Join")
	FCharacterSelectPromptAppearance PromptAppearance;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Character Select|Join")
	FCharacterSelectPromptText PromptText;
	/** If disabled, both sides must join before the selection can be ready. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Character Select|Players")
	bool bAutoCPUOpponent = true;
	/** Allow a second human to replace the CPU after P1 confirms. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Character Select|Players")
	bool bAllowLatePlayerJoin = true;

	bool IsDeviceInputActive() const;
	bool HandleDeviceKey(const FKeyEvent& Event);
	void HandleDeviceAction(int32 GamepadDeviceId, ECharacterSelectInputAction Action);
	void NotifyGamepadDisconnected(int32 DeviceId);

protected:
	UCharacterSelectWidget(
		const FObjectInitializer& ObjectInitializer);

	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

	void InitializeDefaultPlayerConfigs();
	void InitializePlayerControllers();
	void ShutdownPlayerControllers();

	int32 GetPlayerIndexFromController(
		UCharacterSelectionController* Controller) const;


	void UpdateControlTargetDisplay();

	UPROPERTY(BlueprintReadOnly, Category = "Character Select")
	TObjectPtr<UCharacterSelectionController> SelectionController;

	UPROPERTY(BlueprintReadOnly, Category = "Character Select")
	TObjectPtr<UCharacterSelectInputController> InputController;

	UPROPERTY(BlueprintReadOnly, Category = "Character Select")
	TObjectPtr<UCharacterSelectRegistry> Registry;

	UPROPERTY(BlueprintReadOnly, Category = "Character Select")
	TObjectPtr<UCharacterSelectLayoutData> LayoutData;

	UPROPERTY(BlueprintReadOnly, Category = "Character Select|Players")
	TArray<TObjectPtr<UCharacterSelectionController>>
		PlayerSelectionControllers;

	UPROPERTY(BlueprintReadOnly, Category = "Character Select|Players")
	TArray<TObjectPtr<UCharacterSelectInputController>>
		PlayerInputControllers;

	// Bind these names to widgets in the UMG Designer.
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UCharacterSelectGridWidget> CharacterGrid;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> FocusedCharacterName;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> FocusedCharacterImage;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UCharacterSelectPreviewWidget> Player1Preview;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UCharacterSelectPreviewWidget> Player2Preview;

	// Optional: add a TextBlock with this name to show the active target.
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> ControlTargetText;

	// Optional replacements for the automatically generated viewport prompts.
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> Player1JoinText;
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> Player2JoinText;

private:
	friend class FCharacterFeedbackTest;
	void HandleSelectionFeedback(ECharacterSelectFeedback Feedback, int32 PlayerIndex);
	int32 GetHintDeviceOwner(int32 PlayerIndex) const;
	const FSlateBrush* FindActionHintBrush(int32 PlayerIndex, ECharacterSelectInputAction Action) const;
	void SetupDeviceInput();
	void RemoveDeviceInput();
	void CreateJoinPrompts();
	float GetPromptOpacity(int32 PlayerIndex) const;
	bool IsPlayerVisible(int32 PlayerIndex) const;
	UFUNCTION()
	void HandleJoinStateChanged();
	UPROPERTY(Transient)
	TObjectPtr<UCharacterSelectJoinSession> JoinSession;
	TSharedPtr<IInputProcessor> DeviceInputProcessor;
	TSharedPtr<SWidget> JoinPromptOverlay;
	TWeakObjectPtr<UGameViewportClient> PromptViewport;
	bool bWasReady = false;

	UFUNCTION()
	void HandleCharacterFocusChanged(
		UCharacterSelectionController* Controller,
		UCharacterSelectData* Character);

	// Array/controller index currently controlled by P1 when P2 is CPU.
	// 0 = Player 1, 1 = CPU opponent.
	int32 ActiveControlPlayerIndex = 0;

public:
	UPROPERTY(
		EditAnywhere,
		BlueprintReadWrite,
		Category = "Character Select|Players",
		meta = (AllowPrivateAccess = "true", EditFixedSize))
	TArray<FCharacterSelectPlayerConfig> PlayerConfigs;
};
