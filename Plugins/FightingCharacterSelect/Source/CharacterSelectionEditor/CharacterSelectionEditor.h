
#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

class FSpawnTabArgs;
class SDockTab;
class SWindow;

class FCharacterSelectionEditorModule : public IModuleInterface
{
public:
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
	void OpenStudioGuide();

private:
	static const FName CharacterSelectionEditorTabName;
	void OnEditorInitialized(double Duration);
	FDelegateHandle EditorInitializedHandle;
	TWeakPtr<SWindow> GuideWindow;

	void RegisterMenus();
	void OpenCharacterSelectionEditor();

	TSharedRef<SDockTab> SpawnCharacterSelectionEditorTab(
		const FSpawnTabArgs& Args
	);
};
