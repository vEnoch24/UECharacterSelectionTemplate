#include "CharacterSelectSetup.h"
#include "CharacterSelectBasicStarter.h"
#include "WidgetBlueprint.h"
#include "Blueprint/WidgetTree.h"
#include "UI/CharacterSelectWidget.h"
#include "UI/CharacterSelectGridWidget.h"
#include "Data/CharacterSelectLayoutData.h"
#include "Data/CharacterSelectRegistry.h"
#include "Core/CharacterSelectTestController.h"
#include "Core/CharacterSelectNavigationProvider.h"
#include "ObjectTools.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "Kismet2/KismetEditorUtilities.h"
#include "Kismet2/BlueprintEditorUtils.h"
#include "Misc/PackageName.h"
#include "UObject/UnrealType.h"
#include "Editor.h"

namespace
{
	TArray<FString> Names(const UCharacterSelectSetupOptions* O)
	{
		TArray<FString> Result = {TEXT("WBP_") + O->SetupName, TEXT("DA_") + O->SetupName + TEXT("_Layout"),
			TEXT("DA_") + O->SetupName + TEXT("_Roster"), TEXT("BP_") + O->SetupName + TEXT("_Launcher")};
		if (!O->StarterScreen) for (const TCHAR* Suffix : {TEXT("_Slot"), TEXT("_Preview"), TEXT("_Grid")})
			Result.Add(TEXT("WBP_") + O->SetupName + Suffix);
		return Result;
	}
	UCharacterSelectGridWidget* FindGrid(UWidgetBlueprint* BP)
	{
		UCharacterSelectGridWidget* Grid = nullptr;
		int32 Count = 0;
		if (BP && BP->WidgetTree) BP->WidgetTree->ForEachWidget([&](UWidget* W)
		{
			if (auto* G = Cast<UCharacterSelectGridWidget>(W)) { Grid = G; ++Count; }
		});
		return Count == 1 ? Grid : nullptr;
	}
}

bool CharacterSelectSetup::Validate(const UCharacterSelectSetupOptions* O, FString& Error)
{
	Error.Reset();
	if (!O) { Error = TEXT("Setup options are missing."); return false; }
    if (!O->StarterScreen && O->Presentation == ECharacterSelectStarterPresentation::Character3D && !O->PreviewMesh)
    { Error = TEXT("Choose a Preview Mesh for the 3D starter, or choose a 2D starter. No demo mesh is bundled."); return false; }
	if (O->StarterScreen && (!O->StarterScreen->GeneratedClass ||
		!O->StarterScreen->GeneratedClass->IsChildOf(UCharacterSelectWidget::StaticClass()) ||
		O->StarterScreen->Status == BS_Error || !FindGrid(O->StarterScreen)))
	{ Error = TEXT("Choose a compiled full character-select screen with exactly one grid in its own widget tree."); return false; }
	if (O->SetupName.IsEmpty() || !FChar::IsAlpha(O->SetupName[0]))
	{ Error = TEXT("Setup name must begin with a letter and contain only letters, digits and underscores."); return false; }
	for (TCHAR C : O->SetupName) if (!(FChar::IsAlnum(C) || C == TEXT('_')))
	{ Error = TEXT("Setup name may contain only letters, digits and underscores."); return false; }
	if (!O->DestinationFolder.StartsWith(TEXT("/Game/")) || O->DestinationFolder.EndsWith(TEXT("/")) ||
		!FPackageName::IsValidLongPackageName(O->DestinationFolder))
	{ Error = TEXT("Use a Content folder such as /Game/MyFighter/UI, without a trailing slash."); return false; }
	const auto L = O->StarterScreen ? FindGrid(O->StarterScreen)->GetGridLayout() : FCharacterSelectGridLayout();
	if (L.Columns < 1 || L.Rows < 0 || L.SlotSize.X <= 0 || L.SlotSize.Y <= 0 || L.SlotSpacing.X < 0 || L.SlotSpacing.Y < 0)
	{ Error = TEXT("Correct the starter screen's grid dimensions before creating a setup."); return false; }
	if (O->StarterLayout && !UCharacterSelectNavigationProvider::IsModeSupported(O->StarterLayout->NavigationMode))
	{ Error = TEXT("Choose a layout with Grid, Horizontal or Vertical navigation."); return false; }
	for (const FString& Name : Names(O))
	{
		const FString Package = O->DestinationFolder / Name;
		if (FindPackage(nullptr, *Package) || FPackageName::DoesPackageExist(Package))
		{ Error = TEXT("Destination already exists: ") + Package + TEXT(". Choose another name or folder; assets are never overwritten."); return false; }
	}
	return true;
}

FString CharacterSelectSetup::Describe(const UCharacterSelectSetupOptions* O)
{
	if (!O) return FString();
	FString Text = O->StarterScreen ? TEXT("Copy the starter into four assets:\n") : TEXT("Create a basic screen and its editable components (seven assets):\n");
	for (const FString& Name : Names(O)) Text += O->DestinationFolder / Name + TEXT("\n");
    if (!O->StarterScreen)
    {
        if (O->Presentation == ECharacterSelectStarterPresentation::Animated2D)
            Text += TEXT("\n2D example: Lift slot motion, Fade portrait/text motion and a portrait-first sequence. Add character artwork in Character data; use 2D presentation for sprite frames.\n");
        if (O->Presentation == ECharacterSelectStarterPresentation::Character3D)
            Text += TEXT("\n3D example: Lift slot motion, Fade text/portrait transitions, a portrait-first sequence and your mesh in both preview hosts. Adjust camera/framing and add idle/selection clips on 3D presentation. Character-level overrides support a different mesh per fighter.\n");
    }
	Text += FString::Printf(TEXT("\nRoster: %d shared character references.\n"), O->StarterRoster ? O->StarterRoster->Characters.Num() : 0);
	Text += TEXT("An assigned starter is copied with its widget tree and animations; referenced component classes and art remain shared. With no starter, basic screen, grid, slot and preview Blueprints are generated without demo artwork. Layout and roster are new assets.\n\nAfter creation: save the assets, then place ONE generated Launcher in a test level and press Play. Remove or disable another selection launcher in that level to avoid overlapping screens. For an empty roster, add characters first.\n\nCreation does not change your map or game mode and is not an Undo operation. Delete unwanted generated assets in the Content Browser.");
	return Text;
}

bool CharacterSelectSetup::Create(const UCharacterSelectSetupOptions* O, FCharacterSelectSetupResult& R, FString& Error)
{
	R = {};
	if (!GEditor || GEditor->PlayWorld) { Error = TEXT("Stop Play before creating a setup."); return false; }
	if (!Validate(O, Error)) return false;
	// Use Unreal's duplication path without AssetTools' source-control auto-save.
	auto Duplicate = [](const FString& Name, const FString& Folder, UObject* Source)
	{
		ObjectTools::FPackageGroupName Destination;
		Destination.PackageName = Folder / Name;
		Destination.ObjectName = Name;
		TSet<UPackage*> RefusedPackages;
		UObject* Copy = ObjectTools::DuplicateSingleObject(Source, Destination, RefusedPackages, false);
		if (Copy)
		{
			const bool bWasAsset = Copy->IsAsset();
			Copy->SetFlags(RF_Public | RF_Standalone | RF_Transactional);
			if (!bWasAsset) FAssetRegistryModule::AssetCreated(Copy);
		}
		return Copy;
	};
	const auto N = Names(O);
	auto Remember = [&R](UObject* Object) { if (Object) { R.CreatedAssets.Add(Object); Object->MarkPackageDirty(); } };
	if (O->StarterScreen)
	{
		R.Screen = Cast<UWidgetBlueprint>(Duplicate(N[0], O->DestinationFolder, O->StarterScreen));
		Remember(R.Screen);
	}
	else R.Screen = CreateCharacterSelectBasicStarter(O, R, Error);
	if (!R.Screen) { if (Error.IsEmpty()) Error = TEXT("Could not create the selection screen."); return false; }
	FKismetEditorUtilities::CompileBlueprint(R.Screen);
	if (R.Screen->Status == BS_Error || !R.Screen->GeneratedClass)
	{ Error = TEXT("The copied screen failed to compile. Inspect the generated Blueprint's compiler messages."); return false; }
	if (O->StarterLayout) R.Layout = Cast<UCharacterSelectLayoutData>(Duplicate(N[1], O->DestinationFolder, O->StarterLayout));
	else
	{
		R.Layout = NewObject<UCharacterSelectLayoutData>(CreatePackage(*(O->DestinationFolder / N[1])), *N[1], RF_Public | RF_Standalone | RF_Transactional);
		FAssetRegistryModule::AssetCreated(R.Layout);
	}
	Remember(R.Layout);
	if (!R.Layout) { Error = TEXT("Could not create the layout."); return false; }
	const auto& L = FindGrid(R.Screen)->GetGridLayout();
	R.Layout->Columns = L.Columns; R.Layout->Rows = L.Rows; R.Layout->SlotSize = L.SlotSize;
	R.Layout->SlotSpacing = L.SlotSpacing; R.Layout->GridPadding = L.Padding;
	if (O->StarterRoster) R.Roster = Cast<UCharacterSelectRegistry>(Duplicate(N[2], O->DestinationFolder, O->StarterRoster));
	else
	{
		R.Roster = NewObject<UCharacterSelectRegistry>(CreatePackage(*(O->DestinationFolder / N[2])), *N[2], RF_Public | RF_Standalone | RF_Transactional);
		FAssetRegistryModule::AssetCreated(R.Roster);
	}
	Remember(R.Roster);
	if (!R.Roster) { Error = TEXT("Could not create the roster."); return false; }
	R.Launcher = FKismetEditorUtilities::CreateBlueprint(ACharacterSelectTestController::StaticClass(),
		CreatePackage(*(O->DestinationFolder / N[3])), *N[3], BPTYPE_Normal);
	Remember(R.Launcher);
	if (!R.Launcher || !R.Launcher->GeneratedClass) { Error = TEXT("Could not create the launcher Blueprint."); return false; }
	FAssetRegistryModule::AssetCreated(R.Launcher);
	auto* Defaults = R.Launcher->GeneratedClass->GetDefaultObject();
	auto SetReference = [Defaults](FName Name, UObject* Value)
	{
		FObjectPropertyBase* Property = FindFProperty<FObjectPropertyBase>(Defaults->GetClass(), Name);
		if (!Property) return false;
		Property->SetObjectPropertyValue_InContainer(Defaults, Value);
		return true;
	};
	if (!SetReference(TEXT("CharacterSelectWidgetClass"), R.Screen->GeneratedClass) ||
		!SetReference(TEXT("CharacterRegistry"), R.Roster) || !SetReference(TEXT("CharacterLayout"), R.Layout))
	{ Error = TEXT("Launcher properties could not be assigned. Inspect the generated assets."); return false; }
	FBlueprintEditorUtils::MarkBlueprintAsModified(R.Launcher);
	FKismetEditorUtilities::CompileBlueprint(R.Launcher);
	if (R.Launcher->Status == BS_Error) { Error = TEXT("Launcher compilation failed. Inspect its compiler messages."); return false; }
	return true;
}
