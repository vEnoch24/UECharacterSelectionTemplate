#pragma once
#include "CoreMinimal.h"
#include "UI/CharacterSelectPresentationWidget.h"
#include "Types/CharacterSelect3DPresentation.h"
#include "CharacterSelect3DPreviewWidget.generated.h"
class UViewport;
class USkeletalMeshComponent;

/** One isolated, automatically ticking scene per portrait. */
UCLASS()
class CHARACTERSELECTION_API UCharacterSelect3DPreviewWidget : public UCharacterSelectPresentationWidget
{
    GENERATED_BODY()
public:
    void SetStudioAnimationPreview(bool bEnabled) { bStudioAnimationPreview = bEnabled; }
    void AdvanceStudioAnimation(float DeltaTime);
    void Configure(const FCharacterSelect3DPresentation& InSettings, UCharacterSelectData* InCharacter, int32 PlayerIndex, bool bSelected);
    void SetSelected(bool bSelected);
    void SetPreviewPlayerIndex(int32 PlayerIndex);
    UFUNCTION(BlueprintPure, Category="Character Select|3D preview")
    AActor* GetPreviewActor() const { return PreviewActor; }
    UFUNCTION(BlueprintPure, Category="Character Select|3D preview")
    FString GetPreviewError() const { return PreviewError; }
    void ClearScene();
protected:
    virtual TSharedRef<SWidget> RebuildWidget() override;
    virtual void NativeConstruct() override;
    virtual void NativeDestruct() override;
    virtual void NativeTick(const FGeometry& Geometry, float DeltaTime) override;
private:
    friend class FCharacter3DPresentationTest;
    void CreateSceneActor();
    void PlayIdle();
    bool PlayClip(class UAnimSequence* Clip, bool bLoop);
    UPROPERTY(Transient) TObjectPtr<UViewport> SceneViewport;
    UPROPERTY(Transient) TObjectPtr<AActor> PreviewActor;
    UPROPERTY(Transient) TObjectPtr<USkeletalMeshComponent> AnimatedMesh;
    UPROPERTY(Transient) FCharacterSelect3DPresentation Settings;
    FString PreviewError;
    bool bStudioAnimationPreview = false;
    bool bSelectedState = false;
    float SelectionRemaining = 0.f;
    uint8 OriginalAnimationMode = 0;
    UPROPERTY(Transient) TObjectPtr<class UAnimationAsset> OriginalAnimationAsset;
    bool bOriginalLoop = false;
    float OriginalRate = 1.f;
    bool bSelectionOverrideActive = false;
};
