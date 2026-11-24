#if WITH_DEV_AUTOMATION_TESTS
#include "Misc/AutomationTest.h"
#include "UObject/Package.h"
#include "Core/CharacterSelectGridNavigation.h"
#include "Core/CharacterSelectLinearNavigation.h"
#include "Core/CharacterSelectionController.h"
#include "Data/CharacterSelectData.h"
#include "Data/CharacterSelectRegistry.h"
#include "Data/CharacterSelectLayoutData.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCharacterGridNavigationTest, "CharacterSelection.Navigation.Grid", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FCharacterGridNavigationTest::RunTest(const FString& Parameters)
{
	auto* Nav = NewObject<UCharacterSelectGridNavigation>();
	FCharacterSelectNavigationConfig Config;
	Config.Columns = 7;
	Config.HorizontalEdgeBehavior = ECharacterSelectEdgeBehavior::Wrap;
	Nav->Initialize(9, Config);
	TestEqual(TEXT("Left side finds nearest centered final-row slot"), Nav->GetNextIndex(0, ECharacterSelectDirection::Down), 7);
	TestEqual(TEXT("Right side finds nearest centered final-row slot"), Nav->GetNextIndex(6, ECharacterSelectDirection::Down), 8);
	TestEqual(TEXT("Partial row wraps right"), Nav->GetNextIndex(8, ECharacterSelectDirection::Right), 7);
	TestEqual(TEXT("Partial row wraps left"), Nav->GetNextIndex(7, ECharacterSelectDirection::Left), 8);
	TestEqual(TEXT("Stop at top"), Nav->GetNextIndex(0, ECharacterSelectDirection::Up), 0);
	Config.VerticalEdgeBehavior = ECharacterSelectEdgeBehavior::Wrap;
	Nav->Initialize(9, Config);
	TestEqual(TEXT("Vertical wrap uses closest visible column"), Nav->GetNextIndex(6, ECharacterSelectDirection::Up), 8);
	Config.GridLayout.bReverseHorizontal = true;
	Config.GridLayout.bReverseVertical = true;
	Nav->Initialize(9, Config);
	TestEqual(TEXT("Left follows reversed columns"), Nav->GetNextIndex(0, ECharacterSelectDirection::Left), 1);
	TestEqual(TEXT("Up follows reversed rows"), Nav->GetNextIndex(0, ECharacterSelectDirection::Up), 7);
	TestEqual(TEXT("Mirrored centered row stays centered"), Config.GridLayout.GetSlotCoordinates(7, 9).X, 3.5);
	Config.Columns = 3;
	Config.GridLayout.bReverseHorizontal = false;
	Config.GridLayout.bReverseVertical = false;
	Nav->Initialize(6, Config);
	Nav->SetNavigableIndices({0, 2, 5});
	TestEqual(TEXT("Skip excluded horizontal slot"), Nav->GetNextIndex(0, ECharacterSelectDirection::Right), 2);
	TestEqual(TEXT("Find eligible neighbor in next row"), Nav->GetNextIndex(0, ECharacterSelectDirection::Down), 5);
	Nav->SetNavigableIndices({});
	TestEqual(TEXT("Empty eligibility never moves"), Nav->GetNextIndex(0, ECharacterSelectDirection::Right), 0);
	Nav->Initialize(0, Config);
	TestEqual(TEXT("Empty roster returns no index"), Nav->GetNextIndex(0, ECharacterSelectDirection::Right), INDEX_NONE);
	Nav->Initialize(1, Config);
	TestEqual(TEXT("Single slot wrap stays put"), Nav->GetNextIndex(0, ECharacterSelectDirection::Right), 0);
	TestEqual(TEXT("Invalid index returns no index"), Nav->GetNextIndex(1, ECharacterSelectDirection::Right), INDEX_NONE);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCharacterLinearNavigationTest, "CharacterSelection.Navigation.Linear", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FCharacterLinearNavigationTest::RunTest(const FString& Parameters)
{
	for (const auto Mode : {ECharacterSelectNavigationMode::Horizontal, ECharacterSelectNavigationMode::Vertical})
	{
		auto* Nav = UCharacterSelectNavigationProvider::CreateForMode(GetTransientPackage(), Mode);
		TestTrue(TEXT("Factory selects linear provider"), Nav->IsA<UCharacterSelectLinearNavigation>());
		FCharacterSelectNavigationConfig Config;
		Config.Mode = Mode;
		Config.HorizontalEdgeBehavior = ECharacterSelectEdgeBehavior::Stop;
		const bool Horizontal = Mode == ECharacterSelectNavigationMode::Horizontal;
		const auto Forward = Horizontal ? ECharacterSelectDirection::Right : ECharacterSelectDirection::Down;
		const auto Backward = Horizontal ? ECharacterSelectDirection::Left : ECharacterSelectDirection::Up;
		const auto OffAxis = Horizontal ? ECharacterSelectDirection::Down : ECharacterSelectDirection::Right;
		Nav->Initialize(4, Config);
		Nav->SetNavigableIndices({0, 2, 3});
		TestEqual(TEXT("Skips excluded entries"), Nav->GetNextIndex(0, Forward), 2);
		TestEqual(TEXT("Ignores perpendicular input"), Nav->GetNextIndex(0, OffAxis), 0);
		TestEqual(TEXT("Stops at edge"), Nav->GetNextIndex(3, Forward), 3);
		Config.HorizontalEdgeBehavior = Config.VerticalEdgeBehavior = ECharacterSelectEdgeBehavior::Wrap;
		Nav->Initialize(4, Config);
		TestEqual(TEXT("Wraps forward"), Nav->GetNextIndex(3, Forward), 0);
		TestEqual(TEXT("Wraps backward"), Nav->GetNextIndex(0, Backward), 3);
		Config.GridLayout.bReverseHorizontal = Config.GridLayout.bReverseVertical = true;
		Nav->Initialize(4, Config);
		TestEqual(TEXT("Reversed visual direction"), Nav->GetNextIndex(0, Backward), 1);
		Nav->SetNavigableIndices({});
		TestEqual(TEXT("All slots excluded"), Nav->GetNextIndex(0, Forward), 0);
		Config.HorizontalEdgeBehavior = Config.VerticalEdgeBehavior = ECharacterSelectEdgeBehavior::Clamp;
		Config.GridLayout.bReverseHorizontal = Config.GridLayout.bReverseVertical = false;
		Nav->Initialize(4, Config);
		TestEqual(TEXT("Clamp stays at boundary"), Nav->GetNextIndex(3, Forward), 3);
	}
	TestTrue(TEXT("Grid factory"), UCharacterSelectNavigationProvider::CreateForMode(GetTransientPackage(), ECharacterSelectNavigationMode::Grid)->IsA<UCharacterSelectGridNavigation>());
	TestFalse(TEXT("Radial is not offered"), UCharacterSelectNavigationProvider::IsModeSupported(ECharacterSelectNavigationMode::Radial));
	TestFalse(TEXT("Custom is not offered"), UCharacterSelectNavigationProvider::IsModeSupported(ECharacterSelectNavigationMode::Custom));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCharacterNavigationControllerTest, "CharacterSelection.Navigation.Controller", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FCharacterNavigationControllerTest::RunTest(const FString& Parameters)
{
	auto* Registry = NewObject<UCharacterSelectRegistry>();
	for (int32 Index = 0; Index < 4; ++Index) Registry->Characters.Add(NewObject<UCharacterSelectData>());
	Registry->Characters[1]->Availability = ECharacterAvailability::Locked;
	Registry->Characters[3]->Availability = ECharacterAvailability::Hidden;
	auto* Layout = NewObject<UCharacterSelectLayoutData>();
	auto* Controller = NewObject<UCharacterSelectionController>();
	Layout->NavigationMode = ECharacterSelectNavigationMode::Horizontal;
	Controller->Initialize(Registry, Layout, 0, 0);
	TestEqual(TEXT("Hidden character omitted"), Controller->GetRosterEntries().Num(), 3);
	Controller->MoveSelection(ECharacterSelectDirection::Down);
	TestEqual(TEXT("Controller uses horizontal provider"), Controller->GetCurrentIndex(), 0);
	Controller->MoveSelection(ECharacterSelectDirection::Right);
	TestEqual(TEXT("Locked character can be focused"), Controller->GetCurrentIndex(), 1);
	Controller->ConfirmSelection();
	TestFalse(TEXT("Locked character cannot confirm"), Controller->IsConfirmed());
	TestFalse(TEXT("Locked character cannot select"), Controller->HasSelectedCharacter());
	Controller->MoveSelection(ECharacterSelectDirection::Right);
	Controller->ConfirmSelection();
	TestTrue(TEXT("Available character confirms"), Controller->IsConfirmed());
	Controller->MoveSelection(ECharacterSelectDirection::Left);
	TestEqual(TEXT("Confirmed selection cannot move"), Controller->GetCurrentIndex(), 2);
	Controller->CancelSelection();
	Controller->MoveSelection(ECharacterSelectDirection::Left);
	TestEqual(TEXT("Cancel restores movement"), Controller->GetCurrentIndex(), 1);
	Layout->bAllowLockedSlots = false;
	Layout->NavigationMode = ECharacterSelectNavigationMode::Vertical;
	Controller->Initialize(Registry, Layout, 0, 0);
	TestEqual(TEXT("Locked slots omitted when disabled"), Controller->GetRosterEntries().Num(), 2);
	Controller->MoveSelection(ECharacterSelectDirection::Right);
	TestEqual(TEXT("Vertical ignores horizontal input"), Controller->GetCurrentIndex(), 0);
	Controller->MoveSelection(ECharacterSelectDirection::Down);
	TestEqual(TEXT("Vertical moves down"), Controller->GetCurrentIndex(), 1);
	Layout->NavigationMode = ECharacterSelectNavigationMode::Grid;
	Controller->Initialize(Registry, Layout, 0, 0);
	FCharacterSelectGridLayout Display;
	Display.Columns = 2;
	Display.bReverseHorizontal = true;
	Controller->ConfigureNavigationLayout(Display);
	Controller->MoveSelection(ECharacterSelectDirection::Left);
	TestEqual(TEXT("Controller uses displayed reversal and column count"), Controller->GetCurrentIndex(), 1);
	return true;
}
#endif
