#if WITH_DEV_AUTOMATION_TESTS
#include "Misc/AutomationTest.h"
#include "CharacterSelectSetup.h"
#include "WidgetBlueprint.h"
#include "Blueprint/WidgetTree.h"
#include "UI/CharacterSelectWidget.h"
#include "UI/CharacterSelectGridWidget.h"
#include "UI/CharacterSelectPreviewWidget.h"
#include "Data/CharacterSelectLayoutData.h"
#include "Data/CharacterSelectRegistry.h"
#include "Data/CharacterSelectData.h"
#include "UObject/UnrealType.h"
#include "UObject/StrongObjectPtr.h"
#include "Misc/PackageName.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCharacterSelectSetupTest, "CharacterSelection.Editor.SetupWizard", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FCharacterSelectSetupTest::RunTest(const FString& Parameters)
{
	TStrongObjectPtr<UCharacterSelectSetupOptions> O(NewObject<UCharacterSelectSetupOptions>());
	O->StarterScreen = LoadObject<UWidgetBlueprint>(nullptr, TEXT("/Game/CharacterSelect/UI/Core/WBP_CharacterSelect.WBP_CharacterSelect"));
	if (!TestNotNull(TEXT("Starter screen"), O->StarterScreen.Get())) return false;
	O->DestinationFolder = TEXT("/Game/__WizardAutomation_") + FGuid::NewGuid().ToString(EGuidFormats::Digits);
	// Real unsaved asset packages mirror the picker inputs and survive editor GC
	// while Unreal's duplication tools hold typed-element selection handles.
	O->StarterLayout = NewObject<UCharacterSelectLayoutData>(CreatePackage(*(O->DestinationFolder / TEXT("SourceLayout"))), TEXT("SourceLayout"), RF_Public | RF_Standalone);
	O->StarterLayout->Columns = 99;
	O->StarterLayout->NavigationMode = ECharacterSelectNavigationMode::Horizontal;
	O->StarterRoster = NewObject<UCharacterSelectRegistry>(CreatePackage(*(O->DestinationFolder / TEXT("SourceRoster"))), TEXT("SourceRoster"), RF_Public | RF_Standalone);
	O->StarterRoster->Characters.Add(NewObject<UCharacterSelectData>(CreatePackage(*(O->DestinationFolder / TEXT("SourceCharacter"))), TEXT("SourceCharacter"), RF_Public | RF_Standalone));
	const bool bSourceDirty = O->StarterScreen->GetOutermost()->IsDirty();
	FString Error;
	O->SetupName = TEXT("Bad/Name");
	FCharacterSelectSetupResult Rejected;
	TestFalse(TEXT("Invalid name rejected before creating assets"), CharacterSelectSetup::Create(O.Get(), Rejected, Error));
	TestTrue(TEXT("Rejected setup has no assets"), Rejected.CreatedAssets.IsEmpty());
	O->SetupName = TEXT("TestSelection");
	const FString Folder = O->DestinationFolder;
	O->DestinationFolder = TEXT("/Engine/Unwanted");
	TestFalse(TEXT("Only project content destinations allowed"), CharacterSelectSetup::Validate(O.Get(), Error));
	O->DestinationFolder = Folder;
	FCharacterSelectSetupResult R;
	if (!TestTrue(*FString(TEXT("Create connected setup: ") + Error), CharacterSelectSetup::Create(O.Get(), R, Error))) return false;
	TestEqual(TEXT("Four assets created"), R.CreatedAssets.Num(), 4);
	TestTrue(TEXT("Screen is independent"), R.Screen != O->StarterScreen && R.Screen->WidgetTree != O->StarterScreen->WidgetTree);
	TestTrue(TEXT("Roster is independent"), R.Roster != O->StarterRoster);
	TestTrue(TEXT("Characters remain shared"), R.Roster->Characters[0] == O->StarterRoster->Characters[0]);
	TestTrue(TEXT("Navigation copied"), R.Layout->NavigationMode == ECharacterSelectNavigationMode::Horizontal);
	int32 PreviewCount = 0;
	R.Screen->WidgetTree->ForEachWidget([&](UWidget* W)
	{
		if (auto* Grid = Cast<UCharacterSelectGridWidget>(W)) TestEqual(TEXT("Layout dimensions match copied screen"), R.Layout->Columns, Grid->GetGridLayout().Columns);
		if (auto* Preview = Cast<UCharacterSelectPreviewWidget>(W))
		{
			++PreviewCount;
			auto* Source = Cast<UCharacterSelectPreviewWidget>(O->StarterScreen->WidgetTree->FindWidget(W->GetFName()));
			if (TestNotNull(TEXT("Original preview exists"), Source))
			{
				TestTrue(TEXT("Preview animation class retained"), Source->GetClass() == Preview->GetClass());
				TestEqual(TEXT("Preview active style retained"), Source->AnimationStyle, Preview->AnimationStyle);
				TestEqual(TEXT("Preview styles retained"), Source->AnimationStyles.Num(), Preview->AnimationStyles.Num());
			}
		}
	});
	TestEqual(TEXT("Both previews preserved"), PreviewCount, 2);
	auto* Defaults = R.Launcher->GeneratedClass->GetDefaultObject();
	auto GetReference = [Defaults](FName Name) { return FindFProperty<FObjectPropertyBase>(Defaults->GetClass(), Name)->GetObjectPropertyValue_InContainer(Defaults); };
	TestTrue(TEXT("Compiled launcher uses new screen"), GetReference(TEXT("CharacterSelectWidgetClass")) == R.Screen->GeneratedClass);
	TestTrue(TEXT("Compiled launcher uses new roster"), GetReference(TEXT("CharacterRegistry")) == R.Roster);
	TestTrue(TEXT("Compiled launcher uses new layout"), GetReference(TEXT("CharacterLayout")) == R.Layout);
	TestEqual(TEXT("Starter dirty state unchanged"), O->StarterScreen->GetOutermost()->IsDirty(), bSourceDirty);
	TestEqual(TEXT("Starter layout unchanged"), O->StarterLayout->Columns, 99);
	TestFalse(TEXT("Existing destination rejected"), CharacterSelectSetup::Create(O.Get(), Rejected, Error));
	TestTrue(TEXT("Collision creates nothing"), Rejected.CreatedAssets.IsEmpty());
	for (UObject* Asset : R.CreatedAssets) TestFalse(TEXT("Creation does not save packages"), FPackageName::DoesPackageExist(Asset->GetOutermost()->GetName()));
	O->SetupName = TEXT("EmptySelection"); O->StarterRoster = nullptr; O->StarterLayout = nullptr;
	FCharacterSelectSetupResult Empty;
	if (TestTrue(TEXT("Empty starter setup supported"), CharacterSelectSetup::Create(O.Get(), Empty, Error)))
		TestTrue(TEXT("New empty roster"), Empty.Roster->Characters.IsEmpty());
	// Generated assets exist only in this unattended process, never on disk.
	return true;
}
#endif
