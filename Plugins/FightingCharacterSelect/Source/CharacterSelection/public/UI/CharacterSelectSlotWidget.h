#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Types/CharacterSelectSlotAnimation.h"
#include "CharacterSelectSlotWidget.generated.h"

class UImage;
class UWidget;
class UCharacterSelectData;
class UWidgetAnimation;

UCLASS(Abstract, Blueprintable)
class CHARACTERSELECTION_API UCharacterSelectSlotWidget
	: public UUserWidget
{
	GENERATED_BODY()

public:
    void SetStudioAnimationPreview(bool bEnabled);
    /** Reapply authoritative indicators after designer synchronization. */
    void SyncStudioVisualState();
    void AdvanceStudioAnimation(float DeltaTime) { if (bStudioAnimationPreview) AdvanceConfiguredAnimation(DeltaTime); }

    /** Configure in Studio > Slot animations or this slot Blueprint's Class Defaults.
     * Applied to new runtime widgets; existing Blueprint tracks are never modified. */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Character Select|Slot|Animation")
    FCharacterSlotAnimationSettings SlotAnimationSettings;

	// =========================================================
	// Character
	// =========================================================

	UFUNCTION(BlueprintCallable, Category = "Character Select|Slot")
	void SetCharacter(
		UCharacterSelectData* InCharacter
	);

	UFUNCTION(BlueprintPure, Category = "Character Select|Slot")
	UCharacterSelectData* GetCharacter() const;


	// =========================================================
	// Lock
	// =========================================================

	UFUNCTION(BlueprintCallable, Category = "Character Select|Slot")
	void SetLocked(
		bool bInLocked
	);

	UFUNCTION(BlueprintPure, Category = "Character Select|Slot")
	bool IsLocked() const;


	// =========================================================
	// Roster
	// =========================================================

	UFUNCTION(BlueprintCallable, Category = "Character Select|Slot")
	void SetRosterIndex(
		int32 InRosterIndex
	);

	UFUNCTION(BlueprintPure, Category = "Character Select|Slot")
	int32 GetRosterIndex() const;


	// =========================================================
	// Focus
	// =========================================================

	UFUNCTION(BlueprintCallable, Category = "Character Select|Slot")
	void SetFocusedForPlayer(
		int32 PlayerIndex,
		bool bInFocused
	);

	UFUNCTION(BlueprintPure, Category = "Character Select|Slot")
	bool IsFocusedForPlayer(
		int32 PlayerIndex
	) const;

	UFUNCTION(BlueprintPure, Category = "Character Select|Slot")
	bool IsFocusedByAnyPlayer() const;

	UFUNCTION(BlueprintPure, Category = "Character Select|Slot")
	bool IsFocusedByBothPlayers() const;


	// =========================================================
	// Selection
	// =========================================================

	UFUNCTION(BlueprintCallable, Category = "Character Select|Slot")
	void SetSelectedForPlayer(
		int32 PlayerIndex,
		bool bInSelected
	);

	UFUNCTION(BlueprintPure, Category = "Character Select|Slot")
	bool IsSelectedForPlayer(
		int32 PlayerIndex
	) const;


/** Includes a player's selection even if their navigation focus has moved. */
	UFUNCTION(BlueprintPure, Category = "Character Select|Slot")
	bool IsOccupiedByBothPlayers() const;

protected:
	virtual void NativeOnInitialized() override;
	virtual TSharedRef<SWidget> RebuildWidget() override;
	virtual void NativeConstruct() override;
    virtual void NativeDestruct() override;
    virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;


private:
    bool bStudioAnimationPreview = false;
    bool bStudioPreview = false;
    bool bStudioPoseDirty = true;
    void ApplyStudioAuthoredPose(UWidgetAnimation* Animation);


	// =========================================================
	// Visual Refresh
	// =========================================================

	void RefreshCharacterVisual();

	void RefreshVisualState();
	bool IsOccupiedByAnyPlayer() const;
	void PlaySlotTransition(UWidgetAnimation* Animation);
	void HandleSlotAnimationFinished(FWidgetAnimationState& State);
	void RestoreFocusAfterDeselection();
	bool bChangingSlotAnimation = false;
	bool bRestoreFocusAfterDeselection = false;
	friend class FCharacterSlotStateTest;
    friend class FCharacterSlotAnimationTest;
    void UpdateConfiguredAnimation(bool bCancelled = false);
    void AdvanceConfiguredAnimation(float DeltaTime);
    void ApplyConfiguredAppearance();
    int32 ResolveConfiguredState() const;
    int32 ConfiguredState = INDEX_NONE;
    bool bHasConfiguredBaseline = false;
    FWidgetTransform ConfiguredBaselineTransform;
    float ConfiguredBaselineOpacity = 1.f;
    FLinearColor ConfiguredBaselineTint = FLinearColor::White;
    FCharacterSlotAnimationState ConfiguredFrom;
    FCharacterSlotAnimationState ConfiguredCurrent;
    FCharacterSlotAnimationState ConfiguredTarget;
    float ConfiguredElapsed = 0.f;
    bool bConfiguredAnimating = false;
	UPROPERTY(Transient)
	TObjectPtr<UWidgetAnimation> SlotTransitionAnimation;


	// =========================================================
	// Focus Animations
	// =========================================================

	void PlayFocusAnimationForPlayer(
		int32 PlayerIndex
	);

	void PlayUnfocusAnimationForPlayer(
		int32 PlayerIndex
	);


	// =========================================================
	// Selection Animations
	// =========================================================

	void PlaySelectedAnimationForPlayer(
		int32 PlayerIndex
	);

	void PlayDeselectedAnimationForPlayer(
		int32 PlayerIndex
	);


private:

	// =========================================================
	// Character
	// =========================================================

	UPROPERTY(
		BlueprintReadOnly,
		Category = "Character Select|Slot",
		meta = (AllowPrivateAccess = "true")
	)
	TObjectPtr<UCharacterSelectData> Character;


	// =========================================================
	// Focus State
	// =========================================================

	/**
	 * Source of truth for which players are currently focused
	 * on this slot.
	 *
	 * 0 = Player 1
	 * 1 = Player 2
	 */
	TSet<int32> FocusedPlayers;


	// =========================================================
	// Selection State
	// =========================================================

	/**
	 * Source of truth for which players have selected this slot.
	 *
	 * 0 = Player 1
	 * 1 = Player 2
	 */
	TSet<int32> SelectedPlayers;


	// =========================================================
	// General State
	// =========================================================

	UPROPERTY(
		BlueprintReadOnly,
		Category = "Character Select|Slot",
		meta = (AllowPrivateAccess = "true")
	)
	bool bLocked = false;

	UPROPERTY(
		BlueprintReadOnly,
		Category = "Character Select|Slot",
		meta = (AllowPrivateAccess = "true")
	)
	int32 RosterIndex = INDEX_NONE;


	// =========================================================
	// Character Visual
	// =========================================================

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> Portrait;


	// =========================================================
	// Player Focus Borders
	// =========================================================

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UWidget> Player1FocusBorder;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UWidget> Player2FocusBorder;

	// =========================================================
	// Both Players Focus Border
	// =========================================================

	/**
	 * Visible when BOTH players occupy this slot through focus or selection.
	 * For sibling Overlay layers it paints above the selection images.
	 */
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UWidget> BothPlayersFocusBorder;


	// =========================================================
	// Player Selection Borders
	// =========================================================

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UWidget> Player1SelectedBorder;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UWidget> Player2SelectedBorder;


	// =========================================================
	// Lock
	// =========================================================

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UWidget> LockOverlay;


	// =========================================================
	// Player 1 Focus Animations
	// =========================================================

	UPROPERTY(
		Transient,
		meta = (BindWidgetAnimOptional)
	)
	TObjectPtr<UWidgetAnimation> Player1FocusAnimation;

	UPROPERTY(
		Transient,
		meta = (BindWidgetAnimOptional)
	)
	TObjectPtr<UWidgetAnimation> Player1UnfocusAnimation;


	// =========================================================
	// Player 2 Focus Animations
	// =========================================================

	UPROPERTY(
		Transient,
		meta = (BindWidgetAnimOptional)
	)
	TObjectPtr<UWidgetAnimation> Player2FocusAnimation;

	UPROPERTY(
		Transient,
		meta = (BindWidgetAnimOptional)
	)
	TObjectPtr<UWidgetAnimation> Player2UnfocusAnimation;


	// =========================================================
	// Selection Animations
	// =========================================================

	UPROPERTY(
		Transient,
		meta = (BindWidgetAnimOptional)
	)
	TObjectPtr<UWidgetAnimation> SelectedAnimation;

	UPROPERTY(
		Transient,
		meta = (BindWidgetAnimOptional)
	)
	TObjectPtr<UWidgetAnimation> DeselectedAnimation;
};