
#include "CharacterSelectionEditor.h"

#include "SCharacterSelectionEditor.h"
#include "private/SCharacterSelectionGuide.h"
#include "Editor.h"
#include "Misc/App.h"
#include "Misc/ConfigCacheIni.h"
#include "Framework/Application/SlateApplication.h"
#include "Widgets/SWindow.h"

#include "Framework/Docking/TabManager.h"
#include "ToolMenus.h"
#include "Widgets/Docking/SDockTab.h"
#include "Framework/Commands/UIAction.h"
#include "Textures/SlateIcon.h"

#define LOCTEXT_NAMESPACE "CharacterSelectionEditor"

const FName FCharacterSelectionEditorModule::CharacterSelectionEditorTabName(
	TEXT("CharacterSelectionEditor")
);

void FCharacterSelectionEditorModule::StartupModule()
{
    if (!IsRunningCommandlet() && !FApp::IsUnattended())
        EditorInitializedHandle = FEditorDelegates::OnEditorInitialized.AddRaw(this, &FCharacterSelectionEditorModule::OnEditorInitialized);

	FGlobalTabmanager::Get()->RegisterNomadTabSpawner(
		CharacterSelectionEditorTabName,
		FOnSpawnTab::CreateRaw(
			this,
			&FCharacterSelectionEditorModule::SpawnCharacterSelectionEditorTab
		)
	)
		.SetDisplayName(
			LOCTEXT(
				"CharacterSelectionEditorTabTitle",
				"Character Selection Studio"
			)
		)
		.SetTooltipText(
			LOCTEXT(
				"CharacterSelectionEditorTabTooltip",
				"Customize layout, controls, joining, prompts, previews, roster and visual widgets."
			)
		)
		.SetMenuType(ETabSpawnerMenuType::Hidden);

	UToolMenus::RegisterStartupCallback(
		FSimpleMulticastDelegate::FDelegate::CreateRaw(
			this,
			&FCharacterSelectionEditorModule::RegisterMenus
		)
	);
}

void FCharacterSelectionEditorModule::ShutdownModule()
{
    FEditorDelegates::OnEditorInitialized.Remove(EditorInitializedHandle);
    if (auto Window = GuideWindow.Pin()) Window->RequestDestroyWindow();
    GuideWindow.Reset();

	UToolMenus::UnRegisterStartupCallback(this);
	UToolMenus::UnregisterOwner(this);

	FGlobalTabmanager::Get()->UnregisterNomadTabSpawner(
		CharacterSelectionEditorTabName
	);
}

void FCharacterSelectionEditorModule::OnEditorInitialized(double Duration)
{
    if (IsRunningCommandlet() || FApp::IsUnattended() || GIsAutomationTesting || !FSlateApplication::IsInitialized()) return;
    bool bShown = false;
    GConfig->GetBool(TEXT("CharacterSelectionStudioGuide"), TEXT("StartupGuideShownOnce"), bShown, GEditorPerProjectIni);
    if (!bShown) OpenStudioGuide();
}

void FCharacterSelectionEditorModule::OpenStudioGuide()
{
    if (!FSlateApplication::IsInitialized()) return;
    if (auto Existing = GuideWindow.Pin())
    { Existing->BringToFront(); return; }
    TSharedRef<SWindow> Window = SNew(SWindow)
        .Title(LOCTEXT("GuideWindowTitle", "Character Selection Studio Guide"))
        .ClientSize(FVector2D(1180, 820)).MinWidth(800).MinHeight(550)
        .SupportsMaximize(true).SupportsMinimize(true)
        [SNew(SCharacterSelectionGuide)];
    GuideWindow = Window;
    FSlateApplication::Get().AddWindow(Window);
    GConfig->SetBool(TEXT("CharacterSelectionStudioGuide"), TEXT("StartupGuideShownOnce"), true, GEditorPerProjectIni);
    GConfig->Flush(false, GEditorPerProjectIni);
}

void FCharacterSelectionEditorModule::RegisterMenus()
{
	FToolMenuOwnerScoped OwnerScoped(this);

	UToolMenu* ToolsMenu = UToolMenus::Get()->ExtendMenu(
		TEXT("MainFrame.MainMenu.Tools")
	);

	if (!ToolsMenu)
	{
		return;
	}

	FToolMenuSection& Section = ToolsMenu->FindOrAddSection(
		TEXT("CharacterSelection"),
		LOCTEXT("CharacterSelectionSectionLabel", "Character Selection"),
		FToolMenuInsert(NAME_None, EToolMenuInsertType::First)
	);

	FText MenuLabel = LOCTEXT("CharacterSelectionMenuLabel", "Character Selection");
	FText MenuTooltip = LOCTEXT("CharacterSelectionMenuTooltip", "Tools for configuring the character selection system.");

	Section.AddSubMenu(
		TEXT("CharacterSelectionSubMenu"),
		MenuLabel,
		MenuTooltip,
		FNewToolMenuDelegate::CreateLambda([this](UToolMenu* SubMenu)
			{
				// Use FindOrAddSection to safeguard internal menu construction
				FToolMenuSection& SubMenuSection = SubMenu->FindOrAddSection(TEXT("CharacterSelectionTools"));

				SubMenuSection.AddMenuEntry(
					TEXT("OpenCharacterSelectionEditor"),
					LOCTEXT("OpenCharacterSelectionEditorLabel", "Character Selection Studio"),
					LOCTEXT("OpenCharacterSelectionEditorDescription", "Open the character selection configuration tool."),
					FSlateIcon(),
					FUIAction(FExecuteAction::CreateLambda([this]()
						{
							this->OpenCharacterSelectionEditor();
						}))
				);
			})
	);
}






//void FCharacterSelectionEditorModule::RegisterMenus()
//{
//	FToolMenuOwnerScoped OwnerScoped(this);
//
//	UToolMenu* ToolsMenu = UToolMenus::Get()->ExtendMenu(
//		TEXT("LevelEditor.MainMenu.Tools")
//	);
//
//	if (!ToolsMenu)
//	{
//		return;
//	}
//
//	FToolMenuSection& Section = ToolsMenu->FindOrAddSection(
//		TEXT("CharacterSelection"),
//		FToolMenuInsert(NAME_None, EToolMenuInsertType::First)
//	);
//
//	FToolMenuEntry SubMenuEntry = FToolMenuEntry::InitSubMenu(
//		TEXT("CharacterSelectionSubMenu"),
//		LOCTEXT(
//			"CharacterSelectionMenuLabel",
//			"Character Selection"
//		),
//		LOCTEXT(
//			"CharacterSelectionMenuTooltip",
//			"Tools for configuring the character selection system."
//		),
//		FNewToolMenuDelegate::CreateLambda(
//			[this](UToolMenu* SubMenu)
//			{
//				FToolMenuSection& SubMenuSection =
//					SubMenu->AddSection(
//						TEXT("CharacterSelectionTools")
//					);
//
//				SubMenuSection.AddMenuEntry(
//					TEXT("OpenCharacterSelectionEditor"),
//					LOCTEXT(
//						"OpenCharacterSelectionEditorLabel",
//						"Character Selection Studio"
//					),
//					LOCTEXT(
//						"OpenCharacterSelectionEditorDescription",
//						"Open the character selection configuration tool."
//					),
//					FSlateIcon(),
//					FUIAction(
//						FExecuteAction::CreateRaw(
//							this,
//							&FCharacterSelectionEditorModule::
//							OpenCharacterSelectionEditor
//						)
//					)
//				);
//			}
//		),
//		false,
//		FSlateIcon()
//	);
//
//	Section.AddEntry(SubMenuEntry);
//}

void FCharacterSelectionEditorModule::OpenCharacterSelectionEditor()
{
	FGlobalTabmanager::Get()->TryInvokeTab(
		CharacterSelectionEditorTabName
	);
}

TSharedRef<SDockTab>
FCharacterSelectionEditorModule::SpawnCharacterSelectionEditorTab(
	const FSpawnTabArgs& Args
)
{
	return SNew(SDockTab)
		.TabRole(ETabRole::NomadTab)
		[
			SNew(SCharacterSelectionEditor)
		];
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(
	FCharacterSelectionEditorModule,
	CharacterSelectionEditor
)
