#if WITH_DEV_AUTOMATION_TESTS
#include "Misc/AutomationTest.h"
#include "Core/CharacterSelectionController.h"
#include "Data/CharacterSelectLayoutData.h"
#include "Data/CharacterSelectRegistry.h"
#include "Data/CharacterSelectData.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCharacterDirectionalLinksTest, "CharacterSelection.Navigation.DirectionalLinks", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FCharacterDirectionalLinksTest::RunTest(const FString& Parameters)
{
	using D = ECharacterSelectDirection; using R = ECharacterSelectLinkRule;
	auto* Registry = NewObject<UCharacterSelectRegistry>();
	for (FName ID : {FName(TEXT("A")),FName(TEXT("B")),FName(TEXT("C"))})
	{ auto* C = NewObject<UCharacterSelectData>(); C->CharacterID = ID; Registry->Characters.Add(C); }
	auto* Layout = NewObject<UCharacterSelectLayoutData>(); Layout->Columns = 3;
	auto& Links = Layout->DirectionalLinks.Add(TEXT("A"));
	Links.Right.Rule = R::Character; Links.Right.TargetCharacterID = TEXT("C"); Links.Left.Rule = R::Stop;
	auto* Controller = NewObject<UCharacterSelectionController>();
	auto Reset = [&] { Controller->Initialize(Registry,Layout,0,0); };
	Reset(); Controller->MoveSelection(D::Right);
	TestEqual(TEXT("Disabled links retain automatic navigation"),Controller->GetCurrentIndex(),1);
	Layout->bUseDirectionalLinks = true; Reset(); Controller->MoveSelection(D::Right);
	TestEqual(TEXT("Explicit target bypasses neighbor"),Controller->GetCurrentIndex(),2);
	Controller->MoveSelection(D::Left);
	TestEqual(TEXT("No implicit reciprocal link"),Controller->GetCurrentIndex(),1);
	Reset(); Controller->MoveSelection(D::Left); TestEqual(TEXT("Stop overrides wrap"),Controller->GetCurrentIndex(),0);
	Registry->Characters.Swap(1,2); Reset(); Controller->MoveSelection(D::Right);
	TestEqual(TEXT("Reorder preserves target ID"),Controller->GetCurrentIndex(),1);
	TestTrue(TEXT("Valid links pass validation"),Layout->GetDirectionalLinkIssues(Registry).IsEmpty());
	Registry->Characters[1]->Availability = ECharacterAvailability::Hidden; Reset(); Controller->MoveSelection(D::Right);
	TestEqual(TEXT("Hidden target stops"),Controller->GetCurrentIndex(),0);
	TestFalse(TEXT("Hidden target reported"),Layout->GetDirectionalLinkIssues(Registry).IsEmpty());
	Registry->Characters[1]->Availability = ECharacterAvailability::Locked; Reset(); Controller->MoveSelection(D::Right); Controller->ConfirmSelection();
	TestEqual(TEXT("Locked target focuses"),Controller->GetCurrentIndex(),1); TestFalse(TEXT("Locked target cannot confirm"),Controller->IsConfirmed());
	Layout->bAllowLockedSlots = false; Reset(); Controller->MoveSelection(D::Right);
	TestEqual(TEXT("Excluded locked target stops"),Controller->GetCurrentIndex(),0);
	Links.Right.TargetCharacterID = TEXT("Missing"); Controller->MoveSelection(D::Right);
	TestEqual(TEXT("Missing target stops"),Controller->GetCurrentIndex(),0);
	TestFalse(TEXT("Missing target reported"),Layout->GetDirectionalLinkIssues(Registry).IsEmpty());
	Links.Right.TargetCharacterID = TEXT("A"); Controller->MoveSelection(D::Right);
	TestEqual(TEXT("Self-link stops safely"),Controller->GetCurrentIndex(),0);
	Links.Right.TargetCharacterID = TEXT("B"); Registry->Characters[1]->CharacterID = TEXT("A"); Controller->MoveSelection(D::Right);
	TestEqual(TEXT("Ambiguous source stops"),Controller->GetCurrentIndex(),0);
	TestFalse(TEXT("Ambiguous source reported"),Layout->GetDirectionalLinkIssues(Registry).IsEmpty());
	Registry->Characters[1]->CharacterID = TEXT("B"); Controller->MoveSelection(D::Right);
	TestEqual(TEXT("Ambiguous target stops"),Controller->GetCurrentIndex(),0);
	Registry->Characters[1]->CharacterID = TEXT("C"); Registry->Characters[1]->Availability = ECharacterAvailability::Available;
	Layout->NavigationMode = ECharacterSelectNavigationMode::Vertical; Reset(); Controller->MoveSelection(D::Right);
	TestEqual(TEXT("Link overrides linear axis"),Controller->GetFocusedCharacter()->CharacterID,FName(TEXT("B")));
	Controller->ConfirmSelection(); Controller->MoveSelection(D::Left);
	TestEqual(TEXT("Confirmed cannot move"),Controller->GetFocusedCharacter()->CharacterID,FName(TEXT("B")));
	return true;
}
#endif
