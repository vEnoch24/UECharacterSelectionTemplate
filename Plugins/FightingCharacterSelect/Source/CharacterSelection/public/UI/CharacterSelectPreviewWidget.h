#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Types/CharacterSelectPreviewAnimation.h"
#include "Types/CharacterSelect2DPresentation.h"
#include "Types/CharacterSelect3DPresentation.h"
#include "Types/CharacterSelectSequence.h"
#include "CharacterSelectPreviewWidget.generated.h"

class UCharacterSelectData;
class UImage;
class UTextBlock;
class UWidgetAnimation;
class UNamedSlot;
class UCharacterSelectPresentationWidget;

/** Names refer to animations authored in this preview's Widget Blueprint.
 * None skips that phase. Existing animations remain the default when no style is selected. */
USTRUCT(BlueprintType)
struct FCharacterPreviewAnimationStyle
{
	GENERATED_BODY()
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
	FName OutAnimationName = NAME_None;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
	FName InAnimationName = NAME_None;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation", meta = (ClampMin = "0.01"))
	float PlaybackSpeed = 1.0f;
};

UCLASS(Abstract, Blueprintable)
class CHARACTERSELECTION_API UCharacterSelectPreviewWidget
	: public UUserWidget
{
	GENERATED_BODY()

public:
    // Studio drives only native presentation logic on transient designer instances.
    void SetStudioAnimationPreview(bool bEnabled);
    void AdvanceStudioAnimation(float DeltaTime);
    void ReleaseStudioPresentation();
    FString GetStudioAnimationStatus() const;
    /** Optional common defaults. Create a Character Select Sequence Asset in Content Browser. */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Character Select|Animation sequencing")
    TObjectPtr<UCharacterSelectSequenceAsset> SharedSequence;
    /** Overrides the shared asset for this preview instance (for example Player 2). */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Character Select|Animation sequencing")
    bool bOverrideSequence = false;
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Character Select|Animation sequencing", meta=(EditCondition="bOverrideSequence"))
    FCharacterSelectSequence Sequence;
    UFUNCTION(BlueprintPure, Category="Character Select|Animation sequencing")
    FCharacterSelectSequence ResolveSequence(UCharacterSelectData* ForCharacter) const;
    UFUNCTION(BlueprintPure, Category="Character Select|Animation sequencing")
    FText GetSequenceSource(UCharacterSelectData* ForCharacter) const;
    UFUNCTION(BlueprintImplementableEvent, Category="Character Select|Animation sequencing")
    void OnSequenceEvent(ECharacterPresentationEvent Event);
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Character Select|Preview|3D presentation")
    bool bDefault3DPreview = false;
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Character Select|Preview|3D presentation")
    FCharacterSelect3DPresentation DefaultPresentation3D;
    UFUNCTION(BlueprintCallable, Category="Character Select|Preview")
    void SetPreviewSelected(bool bSelected);
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Character Select|Preview|2D presentation")
    FCharacterSelect2DPresentation DefaultPresentation2D;
    UFUNCTION(BlueprintCallable, Category="Character Select|Preview")
    void SetPresentationPlayerIndex(int32 Index);
    UFUNCTION(BlueprintPure, Category="Character Select|Preview")
    UCharacterSelectPresentationWidget* GetCustomPresentationWidget() const { return CustomPresentationWidget; }
    UFUNCTION(BlueprintPure, Category="Character Select|Preview")
    int32 GetPresentationFrameIndex() const { return PresentationFrameIndex; }
    /** Edit per-player preview instances in Studio > Portrait & text animations. Restart Play after changing defaults. */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Character Select|Preview|Animation")
    FCharacterPreviewAnimationSettings PreviewAnimationSettings;

	UFUNCTION(BlueprintCallable, Category = "Character Select|Preview")
	void SetCharacter(UCharacterSelectData* InCharacter);

	UFUNCTION(BlueprintPure, Category = "Character Select|Preview")
	UCharacterSelectData* GetCharacter() const;

	UFUNCTION(BlueprintCallable, Category = "Character Select|Preview")
	void ClearPreview();

	/** Add named styles in Class Defaults or on each preview instance. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Character Select|Preview|Animation")
	TMap<FName, FCharacterPreviewAnimationStyle> AnimationStyles;

	/** None uses the original CharacterChangeOut/InAnimation pair at speed 1. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Character Select|Preview|Animation")
	FName AnimationStyle = NAME_None;

	/** Takes effect on the next transition; does not interrupt the current animation. */
	UFUNCTION(BlueprintCallable, Category = "Character Select|Preview|Animation")
	bool SetAnimationStyle(FName StyleName);

protected:

	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

private:
    friend class FCharacterPreviewElementAnimationTest;
    friend class FCharacter2DPresentationTest;
    friend class FCharacter3DPresentationTest;
    friend class FCharacterSequenceTest;
    void EmitSequenceEvent(ECharacterPresentationEvent Event);
    void AdvanceSequenceConfirmation(float DeltaTime);
    void ResetSequenceConfirmation();
    bool bStudioAnimationPreview = false;
    bool bConfirmDelivered = false;
    float ConfirmElapsed = 0.f;
    bool bUsing3DPresentation = false;
    bool bSceneFailedForCharacter = false;
    bool bPresentationSelected = false;
    void UpdatePresentation2D(UCharacterSelectData* InCharacter);
    void AdvancePresentation2D(float DeltaTime);
    void RemovePresentationTransform();
    void ApplyPresentationTransform();
    UPROPERTY(Transient)
    FCharacterSelect2DPresentation ActivePresentation2D;
    TWeakObjectPtr<UCharacterSelectData> Presented2DCharacter;
    double PresentationSeconds = 0.;
    int32 PresentationFrameIndex = INDEX_NONE;
    int32 PresentationPlayerIndex = 0;
    bool bPresentationTransformApplied = false;
    FWidgetTransform PresentationBaseline, PresentationApplied;
    TWeakObjectPtr<UWidget> PresentationTransformTarget;
    UPROPERTY(Transient)
    TObjectPtr<UCharacterSelectPresentationWidget> CustomPresentationWidget;
    UPROPERTY(meta=(BindWidgetOptional))
    TObjectPtr<UNamedSlot> CustomPresentationSlot;
    enum class EConfiguredPhase : uint8 { Idle, Exit, Entrance };
    struct FConfiguredElement
    {
        TWeakObjectPtr<UWidget> Widget;
        FWidgetTransform Baseline;
        float BaselineOpacity = 1.f;
        FCharacterPreviewTransition Current, From, To, Timing;
    };
    TArray<FConfiguredElement> ConfiguredElements;
    EConfiguredPhase ConfiguredPhase = EConfiguredPhase::Idle;
    float ConfiguredElapsed = 0.f;
    float ConfiguredPhaseDuration = 0.f;
    void EnsureConfiguredElements();
    void SetConfiguredCharacter(UCharacterSelectData* InCharacter);
    void BeginConfiguredPhase(EConfiguredPhase Phase);
    void AdvanceConfiguredAnimation(float DeltaTime);
    void ApplyConfiguredElements();
    void RestoreConfiguredElements();

	void RefreshPreview();

	void UpdatePreviewContent(
		UCharacterSelectData* InCharacter
	);

	UWidgetAnimation* FindPreviewAnimation(FName AnimationName) const;
	void ResolveAnimationStyle();
	void UnbindExitAnimation();

	void PlayCharacterChangeAnimation();

	void FinishCharacterChange();

	UFUNCTION()
	void HandleCharacterChangeAnimationFinished();

private:

	UPROPERTY(BlueprintReadOnly, Category = "Character Select|Preview", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UCharacterSelectData> Character;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> LargePortrait;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> CharacterName;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> Subtitle;

	UPROPERTY(Transient, meta = (BindWidgetAnimOptional))
	TObjectPtr<UWidgetAnimation> CharacterChangeOutAnimation;

	UPROPERTY(Transient, meta = (BindWidgetAnimOptional))
	TObjectPtr<UWidgetAnimation> CharacterChangeInAnimation;

	UPROPERTY()
	TObjectPtr<UCharacterSelectData> PendingCharacter;

	UPROPERTY(Transient)
	TObjectPtr<UWidgetAnimation> ActiveOutAnimation;
	UPROPERTY(Transient)
	TObjectPtr<UWidgetAnimation> ActiveInAnimation;
	float ActivePlaybackSpeed = 1.0f;

	bool bIsChangingCharacter = false;
	bool bIsPlayingOutAnimation = false;
};