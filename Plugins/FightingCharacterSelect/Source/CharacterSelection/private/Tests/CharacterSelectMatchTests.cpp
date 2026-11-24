#if WITH_DEV_AUTOMATION_TESTS
#include "Misc/AutomationTest.h"
#include "Core/CharacterSelectMatchSubsystem.h"
#include "Core/CharacterSelectionController.h"
#include "Data/CharacterSelectData.h"
#include "Data/CharacterSelectRegistry.h"
#include "Data/CharacterSelectLayoutData.h"
#include "Engine/GameInstance.h"
#include "UObject/StrongObjectPtr.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCharacterSelectMatchTest, "CharacterSelection.Match.SnapshotLifecycle", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FCharacterSelectMatchTest::RunTest(const FString& Parameters)
{
	using D = ECharacterSelectDeviceType; using A = ECharacterSelectInputAction;
	TStrongObjectPtr<UGameInstance> Instance(NewObject<UGameInstance>());
	TStrongObjectPtr<UCharacterSelectMatchSubsystem> Store(NewObject<UCharacterSelectMatchSubsystem>(Instance.Get()));
	auto* Registry = NewObject<UCharacterSelectRegistry>();
	for (FName ID : {FName(TEXT("Alpha")),FName(TEXT("Beta"))})
	{ auto* C = NewObject<UCharacterSelectData>(); C->CharacterID=ID; Registry->Characters.Add(C); }
	auto* Layout = NewObject<UCharacterSelectLayoutData>();
	auto* P1 = NewObject<UCharacterSelectionController>(); auto* P2 = NewObject<UCharacterSelectionController>();
	P1->Initialize(Registry,Layout,0,0); P2->Initialize(Registry,Layout,1,1);
	auto* Join = NewObject<UCharacterSelectJoinSession>(); Join->Initialize({P1,P2});
	FString Error; FCharacterSelectMatchResult Result;
	TestFalse(TEXT("Incomplete selection rejected"),Store->StoreFromSession(Join,Registry,Error));
	TestFalse(TEXT("Failure explains requirement"),Error.IsEmpty());
	auto Key = [&] { Join->HandleInput(D::Keyboard,0,A::Confirm,true); };
	Key(); Key(); Key();
	TestTrue(TEXT("CPU ready snapshot"),Store->StoreFromSession(Join,Registry,Error));
	TestTrue(TEXT("Snapshot readable"),Store->GetSelection(Result));
	if (!TestEqual(TEXT("Two players"),Result.Players.Num(),2)) return false;
	TestTrue(TEXT("Snapshot has identity"),Result.SelectionID.IsValid());
	TestEqual(TEXT("P1 selected ID"),Result.Players[0].CharacterID,FName(TEXT("Alpha")));
	TestEqual(TEXT("CPU selected ID"),Result.Players[1].CharacterID,FName(TEXT("Beta")));
	TestTrue(TEXT("CPU has no owned device"),Result.Players[1].bIsCPU && Result.Players[1].Device.Type==D::None && Result.Players[1].Device.DeviceId==INDEX_NONE);
	TestTrue(TEXT("P1 keyboard retained"),Result.Players[0].Device.Type==D::Keyboard && !Result.Players[0].bIsCPU);
	const FGuid FirstID = Result.SelectionID;
	Result.Players.Empty(); Store->GetSelection(Result);
	TestEqual(TEXT("Caller edits cannot mutate stored copy"),Result.Players.Num(),2);
	Join->Initialize({P1,P2});
	TestTrue(TEXT("Session teardown does not mutate snapshot"),Store->GetSelection(Result) && Result.SelectionID==FirstID);
	TestFalse(TEXT("Unready recapture clears stale snapshot"),Store->StoreFromSession(Join,Registry,Error));
	TestFalse(TEXT("No stale result"),Store->GetSelection(Result)); TestTrue(TEXT("False read clears output"),Result.Players.IsEmpty());
	Join->HandleInput(D::Gamepad,42,A::Confirm,true); Key();
	Join->HandleInput(D::Gamepad,42,A::Confirm,true); Key();
	TestTrue(TEXT("Gamepad P1 keyboard P2 captured"),Store->StoreFromSession(Join,Registry,Error)); Store->GetSelection(Result);
	TestTrue(TEXT("Human devices preserved"),!Result.Players[1].bIsCPU && Result.Players[0].Device.DeviceId==42 && Result.Players[1].Device.Type==D::Keyboard);
	TestTrue(TEXT("New capture has new identity"),Result.SelectionID != FirstID);
	Join->SetGamepadConnected(42,false);
	TestFalse(TEXT("Disconnected human rejected"),Store->StoreFromSession(Join,Registry,Error));
	Join->HandleInput(D::Gamepad,42,A::Confirm,true);
	Registry->Characters[1]->CharacterID = TEXT("Alpha");
	TestFalse(TEXT("Ambiguous ID rejected"),Store->StoreFromSession(Join,Registry,Error));
	Registry->Characters[1]->CharacterID = NAME_None;
	TestFalse(TEXT("Empty ID rejected"),Store->StoreFromSession(Join,Registry,Error));
	Registry->Characters[1]->CharacterID = TEXT("Beta");
	Registry->Characters[1]->Availability = ECharacterAvailability::Locked;
	TestFalse(TEXT("Unavailable selection rejected"),Store->StoreFromSession(Join,Registry,Error));
	Registry->Characters[1]->Availability = ECharacterAvailability::Available;
	TestTrue(TEXT("Valid recapture"),Store->StoreFromSession(Join,Registry,Error));
	TestTrue(TEXT("Consume returns result"),Store->ConsumeSelection(Result));
	TestFalse(TEXT("Consume clears store"),Store->HasSelection());
	TestFalse(TEXT("Second consume fails"),Store->ConsumeSelection(Result));
	TestTrue(TEXT("Failed consume clears output"),Result.Players.IsEmpty());
	TestFalse(TEXT("Null screen handled"),Store->StoreFromScreen(nullptr,Error));
	return true;
}
#endif
