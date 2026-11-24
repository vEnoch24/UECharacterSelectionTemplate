#include "SCharacterSelectionEditor.h"
#include "CharacterSelectSetup.h"
#include "WidgetBlueprint.h"
#include "Data/CharacterSelectLayoutData.h"
#include "Data/CharacterSelectRegistry.h"
#include "Editor.h"
#include "PropertyEditorModule.h"
#include "IDetailsView.h"
#include "Widgets/SWindow.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Layout/SWidgetSwitcher.h"
#include "Widgets/Layout/SScrollBox.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Text/STextBlock.h"
#include "AssetToolsModule.h"
#include "IAssetTools.h"

#define LOCTEXT_NAMESPACE "CharacterSelectSetupWizard"
FReply SCharacterSelectionEditor::OpenSetupWizard()
{
	if (!GEditor || GEditor->PlayWorld) return FReply::Handled();
	TStrongObjectPtr<UCharacterSelectSetupOptions> Options(NewObject<UCharacterSelectSetupOptions>());
	Options->StarterScreen = Cast<UWidgetBlueprint>(Assets[0].Get());
	Options->StarterLayout = Cast<UCharacterSelectLayoutData>(Assets[1].Get());
	Options->StarterRoster = Cast<UCharacterSelectRegistry>(Assets[2].Get());
	FDetailsViewArgs Args;
	Args.bAllowSearch = false; Args.bHideSelectionTip = true; Args.NameAreaSettings = FDetailsViewArgs::HideNameArea;
	auto Form = FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor").CreateDetailView(Args);
	Form->SetObject(Options.Get());
	int32 Step = 0;
	FString Feedback;
	bool bCreated = false;
	FCharacterSelectSetupResult Result;
	TSharedPtr<SWidgetSwitcher> Pages;
	auto Window = SNew(SWindow).Title(LOCTEXT("Title", "New character-selection setup"))
		.ClientSize(FVector2D(760, 640)).SupportsMinimize(false).SupportsMaximize(false);
	Window->SetContent(
		SNew(SVerticalBox)
		+ SVerticalBox::Slot().AutoHeight().Padding(18)
		[SNew(STextBlock).Text_Lambda([&Step] { return Step == 0 ? LOCTEXT("Configure", "1. Choose a destination (starter optional)") : LOCTEXT("Review", "2. Review and create"); })]
		+ SVerticalBox::Slot().FillHeight(1).Padding(18,0)
		[SAssignNew(Pages, SWidgetSwitcher)
			+ SWidgetSwitcher::Slot()[Form]
			+ SWidgetSwitcher::Slot()[SNew(SScrollBox) + SScrollBox::Slot()
				[SNew(STextBlock).AutoWrapText(true).Text_Lambda([&Options] { return FText::FromString(CharacterSelectSetup::Describe(Options.Get())); })]]]
		+ SVerticalBox::Slot().AutoHeight().Padding(18,10)
		[SNew(STextBlock).AutoWrapText(true).Text_Lambda([&Feedback] { return FText::FromString(Feedback); })]
		+ SVerticalBox::Slot().AutoHeight().Padding(18)
		[SNew(SHorizontalBox)
			+ SHorizontalBox::Slot().AutoWidth().Padding(0,0,10,0)
			[SNew(SButton).Text(LOCTEXT("Back", "Back")).IsEnabled_Lambda([&Step] { return Step == 1; })
			.OnClicked_Lambda([&] { Step = 0; Pages->SetActiveWidgetIndex(0); Feedback.Reset(); return FReply::Handled(); })]
			+ SHorizontalBox::Slot().AutoWidth().Padding(0,0,10,0)
			[SNew(SButton).Text_Lambda([&Step] { return Step == 0 ? LOCTEXT("Next", "Review setup") : LOCTEXT("Create", "Create setup"); })
			.OnClicked_Lambda([&]
			{
				if (!CharacterSelectSetup::Validate(Options.Get(), Feedback)) return FReply::Handled();
				if (Step == 0) { Step = 1; Pages->SetActiveWidgetIndex(1); return FReply::Handled(); }
				bCreated = CharacterSelectSetup::Create(Options.Get(), Result, Feedback);
				if (bCreated || !Result.CreatedAssets.IsEmpty()) Window->RequestDestroyWindow();
				return FReply::Handled();
			})]
			+ SHorizontalBox::Slot().AutoWidth()
			[SNew(SButton).Text(LOCTEXT("Cancel", "Cancel")).OnClicked_Lambda([&Window] { Window->RequestDestroyWindow(); return FReply::Handled(); })]]);
	GEditor->EditorAddModalWindow(Window);
	// Modal callbacks cannot outlive the local state captured above.
	Window->SetContent(SNullWidget::NullWidget);
	Form->SetObject(nullptr);
	if (Result.CreatedAssets.IsEmpty()) return FReply::Handled();
	for (UObject* Asset : Result.CreatedAssets) CreatedSetupAssets.AddUnique(Asset);
	Assets[0].Reset(Result.Screen);
	Assets[1].Reset(Result.Layout);
	Assets[2].Reset(Result.Roster);
	Assets[3].Reset(); Assets[4].Reset();
	Assets[6].Reset(Result.Launcher);
	SaveSelections();
	SelectPage(0);
	FModuleManager::LoadModuleChecked<FAssetToolsModule>("AssetTools").Get().SyncBrowserToAssets(Result.CreatedAssets);
	Message = bCreated
		? FText::FromString(TEXT("Setup created and selected. Save selected assets, then place ") + Result.Launcher->GetName() + TEXT(" in a test level and press Play. Use only one selection launcher. Add characters on Roster first if the roster is empty. An assigned starter shares preview/slot classes and art. Without a starter, editable basic component Blueprints are created."))
		: FText::FromString(Feedback + TEXT(" Part of the setup was created but not saved; those assets are selected in the Content Browser. Inspect or delete them before retrying with a new name."));
	return FReply::Handled();
}
#undef LOCTEXT_NAMESPACE
