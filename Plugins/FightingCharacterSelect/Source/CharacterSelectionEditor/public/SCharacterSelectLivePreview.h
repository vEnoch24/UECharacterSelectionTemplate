#pragma once
#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"
#include "UObject/StrongObjectPtr.h"
#include "Core/CharacterSelectionController.h"

class UWidgetBlueprint;
class UCharacterSelectRegistry;
class UCharacterSelectLayoutData;
class SBox;
class UUserWidget;
class UCharacterSelectGridWidget;

/** Transient designer tree; optional native animation playback never initializes gameplay input. */
class SCharacterSelectLivePreview : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SCharacterSelectLivePreview) {} SLATE_END_ARGS()
	void Construct(const FArguments& Args);
	virtual ~SCharacterSelectLivePreview() override;
	void SetAssets(UWidgetBlueprint* Screen, UCharacterSelectRegistry* Registry, UCharacterSelectLayoutData* Layout);
	void Refresh();
	virtual void Tick(const FGeometry& Geometry, double Time, float Delta) override;
private:
	friend class FCharacterSelectLivePreviewTest;
    void DrawRoster();
    void AdvancePlayback(float DeltaTime, bool bSingleStep = false);
    void ReleasePresentations();
    void SyncVisibility();
    bool bAnimate = false;
    bool bPlaying = true;
    bool bSuspendedForPIE = false;
    bool bCleared[2] = {false, false};
    float PlaybackSpeed = 1.f;
    double PlaybackSeconds = 0.;
	FText GetStatus() const;
	FReply Move(ECharacterSelectDirection Direction);
	TWeakObjectPtr<UWidgetBlueprint> ScreenAsset;
	TWeakObjectPtr<UCharacterSelectRegistry> RegistryAsset;
	TWeakObjectPtr<UCharacterSelectLayoutData> LayoutAsset;
	TStrongObjectPtr<UCharacterSelectionController> Controllers[2];
	FCharacterSelectGridLayout Grid;
	FVector2D Resolution = FVector2D(1280, 720);
	int32 ActivePlayer = 0;
	bool bPendingRefresh = false;
	bool bClipped = false;
	FString Problem;
	FString LinkWarnings;
	TSharedPtr<SBox> Stage;
	TStrongObjectPtr<UUserWidget> PreviewOwner;
	TWeakObjectPtr<UCharacterSelectGridWidget> PreviewGrid;
	TSharedPtr<SWidget> PreviewContent;
	FDelegateHandle PropertyHandle;
};
