#pragma once
#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"
#include "EditorUndoClient.h"
#include "UObject/StrongObjectPtr.h"

class IDetailsView;
class SVerticalBox;
class SCharacterSelectLivePreview;
class SWindow;
class UWidgetBlueprint;
class UBlueprint;
struct FAssetData;
struct FPropertyAndParent;
class FProperty;
struct FPropertyChangedEvent;

/** Asset-backed authoring hub. Property edits use Unreal's native transactions. */
class SCharacterSelectionEditor : public SCompoundWidget, public FEditorUndoClient
{
public:
	SLATE_BEGIN_ARGS(SCharacterSelectionEditor) {} SLATE_END_ARGS()
	void Construct(const FArguments& InArgs);
	virtual ~SCharacterSelectionEditor() override;
	virtual void PostUndo(bool bSuccess) override;
	virtual void PostRedo(bool bSuccess) override { PostUndo(bSuccess); }
private:
	friend class FCharacterSelectionStudioTest;
	struct FTarget
	{
		FString Label;
		TWeakObjectPtr<UObject> Object;
		TWeakObjectPtr<UBlueprint> Blueprint;
	};
	void SelectPage(int32 Page);
	void SetAdvancedSettings(bool bEnabled);
	bool IsAdvancedProperty(const FProperty& Property) const;
	FText GetTargetDescription() const;
	void RebuildTargets();
	void SelectTarget(TSharedPtr<FTarget> Target);
	void AssetChanged(const FAssetData& Data, int32 Index);
	void LoadSelections();
	void SaveSelections() const;
	void OnPropertyChanged(const FPropertyChangedEvent& Event);
	bool IsPropertyVisible(const FPropertyAndParent& Property) const;
	FReply CreateDataAsset(int32 Index);
	FReply OpenSetupWizard();
	FReply OpenLivePreview();
	FReply OpenStudioGuide();
	void RefreshLivePreview();
	void OnPreviewClosed(const TSharedRef<SWindow>& Window);
	TSharedPtr<SCharacterSelectLivePreview> LivePreview;
	TWeakPtr<SWindow> PreviewWindow;
	FText GetSetupChecklist() const;
	FReply TransferTheme(bool bCapture);
	FReply ApplySlotAnimationPreset(int32 Preset);
	FReply ApplyPreviewAnimationPreset(int32 Preset);
	FReply AddPresentationHost();
    FReply ApplySequencePreset(int32 Preset);
    FReply CreateSequenceAsset();
	FReply UseLauncherAssets();
	FString GetLauncherMismatch() const;
	FReply Validate();
	FReply Save();
	FReply OpenTarget();
	FReply SynchronizeLayout();
	FText GetStatus() const;
	TSharedRef<SWidget> BuildSetup();
	int32 ActivePage = 0;
	bool bShowAdvancedSettings = false;
	TArray<TWeakObjectPtr<UObject>> EditedAssets;
	// Screen, layout, registry, character, optional component, theme, generated launcher.
	TStrongObjectPtr<UObject> Assets[7];
	// Include all newly generated components in Studio's save action, even when
	// only the screen/layout/roster/launcher are selected in the asset pickers.
	TArray<TWeakObjectPtr<UObject>> CreatedSetupAssets;
	TArray<TSharedPtr<FTarget>> Targets;
	TSharedPtr<FTarget> CurrentTarget;
	TSharedPtr<IDetailsView> Details;
	TSharedPtr<SVerticalBox> TargetBar;
	FText Message;
	FDelegateHandle CompileHandle;
};
