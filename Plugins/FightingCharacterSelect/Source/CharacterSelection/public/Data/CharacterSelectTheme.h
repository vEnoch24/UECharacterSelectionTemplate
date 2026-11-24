#pragma once
#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Types/CharacterSelectCustomization.h"
#include "Types/CharacterSelectFeedback.h"
#include "Types/CharacterSelectGridTypes.h"
#include "Types/CharacterSelectNavigationTypes.h"
#include "Types/CharacterSelectPlayerTypes.h"
#include "UI/CharacterSelectPreviewWidget.h"
#include "Types/CharacterSelectSlotAnimation.h"
#include "CharacterSelectTheme.generated.h"

USTRUCT(BlueprintType)
struct FCharacterSelectPreviewTheme
{
	GENERATED_BODY()
    /** New snapshots include native presentation settings; old themes remain compatible. */
    UPROPERTY(VisibleAnywhere, Category="Presentation")
    bool bHasPresentation = false;
    UPROPERTY(EditAnywhere, Category="Presentation")
    FCharacterPreviewAnimationSettings Motion;
    UPROPERTY(EditAnywhere, Category="Presentation")
    FCharacterSelect2DPresentation Artwork;
    UPROPERTY(EditAnywhere, Category="Presentation")
    bool bUse3D = false;
    UPROPERTY(EditAnywhere, Category="Presentation")
    FCharacterSelect3DPresentation Scene;
    /** Resolved preview defaults copied by value; character overrides remain independent. */
    UPROPERTY(EditAnywhere, Category="Presentation")
    FCharacterSelectSequence Sequence;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Preview")
	FName ActiveStyle;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Preview")
	TMap<FName, FCharacterPreviewAnimationStyle> Styles;
};

/** Portable configuration snapshot. Apply through the Studio; no live runtime dependency. */
UCLASS(BlueprintType)
class CHARACTERSELECTION_API UCharacterSelectTheme : public UDataAsset
{
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="About", meta=(MultiLine=true))
	FText Description;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="About")
	bool bHasSnapshot = false;
	UPROPERTY(EditAnywhere, Category="Apply sections")
	bool bApplyLayout = true;
	UPROPERTY(EditAnywhere, Category="Apply sections")
	bool bApplyPrompts = true;
	UPROPERTY(EditAnywhere, Category="Apply sections")
	bool bApplyPreviewAnimations = true;
    /** Apply native portrait/text motion, 2D/3D defaults and resolved sequencing. Older themes leave these untouched. */
    UPROPERTY(EditAnywhere, Category="Apply sections")
    bool bApplyPresentation = false;
    /** Changes the grid's shared slot Blueprint defaults, potentially affecting other screens. Opt in explicitly. */
    UPROPERTY(EditAnywhere, Category="Apply sections")
    bool bApplySlotMotion = false;
    UPROPERTY(VisibleAnywhere, Category="Slot motion")
    bool bHasSlotMotion = false;
    UPROPERTY(EditAnywhere, Category="Slot motion")
    FCharacterSlotAnimationSettings SlotMotion;
	/** Opt in to replacing another screen's input bindings. */
	UPROPERTY(EditAnywhere, Category="Apply sections")
	bool bApplyControls = false;
	UPROPERTY(EditAnywhere, Category="Apply sections")
	bool bApplyPlayers = false;
	/** Opt in; older theme snapshots leave these settings untouched. */
	UPROPERTY(EditAnywhere, Category="Apply sections")
	bool bApplyFeedback = false;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Feedback")
	FCharacterSelectSoundSettings SoundSettings;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Feedback")
	FCharacterSelectHintSettings HintSettings;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Layout")
	FCharacterSelectGridLayout GridLayout;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Layout")
	ECharacterSelectNavigationMode NavigationMode = ECharacterSelectNavigationMode::Grid;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Layout")
	ECharacterSelectEdgeBehavior HorizontalEdge = ECharacterSelectEdgeBehavior::Wrap;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Layout")
	ECharacterSelectEdgeBehavior VerticalEdge = ECharacterSelectEdgeBehavior::Stop;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Prompts")
	FCharacterSelectPromptAppearance PromptAppearance;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Prompts")
	FCharacterSelectPromptText PromptText;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Prompts", meta=(ClampMin="0", ClampMax="1"))
	float PromptMinOpacity = 0.55f;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Prompts", meta=(ClampMin="0"))
	float PromptPulseSpeed = 4.0f;
	/** Keyed by preview widget name, e.g. Player1Preview. Tracks are never copied. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Preview animations")
	TMap<FName, FCharacterSelectPreviewTheme> Previews;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Controls")
	FCharacterSelectControlSettings Controls;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Players")
	TArray<FCharacterSelectPlayerConfig> Players;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Players")
	bool bAutoCPUOpponent = true;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Players")
	bool bAllowLatePlayerJoin = true;
};
