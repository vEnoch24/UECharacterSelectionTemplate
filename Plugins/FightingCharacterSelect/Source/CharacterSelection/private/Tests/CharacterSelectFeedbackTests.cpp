#if WITH_DEV_AUTOMATION_TESTS
#include "Misc/AutomationTest.h"
#include "UObject/Package.h"
#include "Core/CharacterSelectJoinSession.h"
#include "Core/CharacterSelectionController.h"
#include "UI/CharacterSelectWidget.h"
#include "Data/CharacterSelectData.h"
#include "Data/CharacterSelectRegistry.h"
#include "Data/CharacterSelectLayoutData.h"
#include "Engine/Texture2D.h"
#include "UObject/StrongObjectPtr.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCharacterFeedbackTest, "CharacterSelection.Controls.FeedbackAndHints", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FCharacterFeedbackTest::RunTest(const FString& Parameters)
{
	using A = ECharacterSelectInputAction;
	using D = ECharacterSelectDeviceType;
	using F = ECharacterSelectFeedback;
	auto* Registry = NewObject<UCharacterSelectRegistry>();
	for (int32 I=0; I<2; ++I) Registry->Characters.Add(NewObject<UCharacterSelectData>());
	Registry->Characters[1]->Availability = ECharacterAvailability::Locked;
	auto* Layout = NewObject<UCharacterSelectLayoutData>();
	Layout->Columns = 2; Layout->HorizontalEdgeBehavior = ECharacterSelectEdgeBehavior::Stop;
	auto* P1 = NewObject<UCharacterSelectionController>(); auto* P2 = NewObject<UCharacterSelectionController>();
	P1->Initialize(Registry, Layout, 0, 0); P2->Initialize(Registry, Layout, 1, 0);
	auto* Join = NewObject<UCharacterSelectJoinSession>(); Join->Initialize({P1,P2});
	TStrongObjectPtr<UCharacterSelectJoinSession> KeepSession(Join);
	TArray<F> Events; TArray<int32> Players;
	Join->OnFeedback.AddLambda([&](F Event, int32 Player) { Events.Add(Event); Players.Add(Player); });
	auto Input = [&](A Action, bool Repeat=false) { Join->HandleInput(D::Keyboard,0,Action,true,Repeat); };
	Input(A::Confirm,true); TestEqual(TEXT("Repeat cannot join or emit feedback"), Events.Num(),0);
	Input(A::Confirm); TestTrue(TEXT("Join feedback only"), Events.Num()==1 && Events.Last()==F::Joined && !P1->IsConfirmed());
	Input(A::NavigateLeft); TestEqual(TEXT("Boundary is silent"), Events.Num(),1);
	Input(A::NavigateRight); TestTrue(TEXT("Actual move feedback"), Events.Last()==F::Moved);
	Input(A::Confirm); TestTrue(TEXT("Locked choice rejected"), Events.Last()==F::Rejected && !P1->IsConfirmed());
	Input(A::NavigateLeft); Input(A::Confirm); TestTrue(TEXT("Confirmation feedback"), Events.Last()==F::Confirmed);
	const int32 BeforeRepeat = Events.Num(); Input(A::Confirm,true); TestEqual(TEXT("Repeated confirm silent"), Events.Num(),BeforeRepeat);
	Input(A::Cancel); TestTrue(TEXT("CPU back cancels P1"), Events.Last()==F::Cancelled && Players.Last()==0 && !P1->IsConfirmed());
	const int32 BeforeCancel = Events.Num(); Input(A::Cancel); TestEqual(TEXT("No-op cancel silent"), Events.Num(),BeforeCancel);
	Input(A::Confirm); Input(A::Confirm); TestTrue(TEXT("CPU confirm belongs to slot 2"), Events.Last()==F::Confirmed && Players.Last()==1);
	const int32 BeforeDone = Events.Num(); Input(A::Confirm); TestEqual(TEXT("Already confirmed silent"), Events.Num(),BeforeDone);

	UClass* Class = LoadClass<UCharacterSelectWidget>(nullptr,TEXT("/Game/CharacterSelect/UI/Core/WBP_CharacterSelect.WBP_CharacterSelect_C"));
	if (!TestNotNull(TEXT("Screen class"),Class)) return false;
	auto* Screen = NewObject<UCharacterSelectWidget>(GetTransientPackage(),Class);
	Screen->JoinSession = Join; // No viewport or input processor is needed to resolve hints.
	Screen->ControlSettings.Confirm = {EKeys::J,EKeys::Gamepad_FaceButton_Top};
	TestTrue(TEXT("Remapped keyboard hint"),Screen->GetActionHintKey(0,A::Confirm)==EKeys::J);
	TestTrue(TEXT("CPU hint follows P1 device"),Screen->GetActionHintKey(1,A::Confirm)==EKeys::J);
	Join->HandleInput(D::Gamepad,7,A::Confirm,true);
	TestTrue(TEXT("Late join emits join, not cancel"),Events.Last()==F::Joined && Players.Last()==1);
	TestTrue(TEXT("P2 hint switches to gamepad"),Screen->GetActionHintKey(1,A::Confirm)==EKeys::Gamepad_FaceButton_Top);
	Join->SetGamepadConnected(7,false);
	TestFalse(TEXT("Disconnected hint is hidden"),Screen->GetActionHintKey(1,A::Confirm).IsValid());
	Join->HandleInput(D::Gamepad,7,A::Confirm,true);
	TestTrue(TEXT("Reconnect distinct feedback"),Events.Last()==F::Reconnected);
	TestFalse(TEXT("Reconnect does not confirm"),P2->IsConfirmed());
	Screen->HintSettings.KeyLabels.Add(EKeys::J,FText::FromString(TEXT("Custom J")));
	TestEqual(TEXT("Custom label"),Screen->GetActionHintLabel(0,A::Confirm).ToString(),FString(TEXT("Custom J")));
	TestTrue(TEXT("Missing icon has empty brush"),Screen->GetActionHintBrush(0,A::Confirm).GetResourceObject()==nullptr);
	auto* Texture = NewObject<UTexture2D>(); FSlateBrush Brush; Brush.SetResourceObject(Texture);
	Screen->HintSettings.Player2GamepadGlyphs.Add(EKeys::Gamepad_FaceButton_Top,Brush);
	TestTrue(TEXT("P2 controller artwork"),Screen->GetActionHintBrush(1,A::Confirm).GetResourceObject()==Texture);
	Screen->ControlSettings.Cancel = {EKeys::Escape};
	TestFalse(TEXT("Missing gamepad binding remains unbound"),Screen->GetActionHintKey(1,A::Cancel).IsValid());
	Join->OnFeedback.Clear();
	return true;
}
#endif
