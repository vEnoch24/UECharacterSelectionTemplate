#if WITH_DEV_AUTOMATION_TESTS
#include "Misc/AutomationTest.h"
#include "Core/CharacterSelectJoinSession.h"
#include "Core/CharacterSelectionController.h"
#include "Core/CharacterSelectInputBindings.h"
#include "Core/CharacterSelectStickRepeat.h"
#include "Data/CharacterSelectData.h"
#include "Data/CharacterSelectRegistry.h"
#include "Data/CharacterSelectLayoutData.h"

namespace CharacterSelectJoinTests
{
	struct FSession
	{
		UCharacterSelectJoinSession* Join;
		UCharacterSelectionController* P1;
		UCharacterSelectionController* P2;
		FSession()
		{
			auto* Registry = NewObject<UCharacterSelectRegistry>();
			for (int32 Index = 0; Index < 4; ++Index) Registry->Characters.Add(NewObject<UCharacterSelectData>());
			auto* Layout = NewObject<UCharacterSelectLayoutData>();
			Layout->Columns = 4;
			P1 = NewObject<UCharacterSelectionController>();
			P2 = NewObject<UCharacterSelectionController>();
			P1->Initialize(Registry, Layout, 0, 0);
			P2->Initialize(Registry, Layout, 1, 3);
			Join = NewObject<UCharacterSelectJoinSession>();
			Join->Initialize({P1, P2});
		}
		void Key(ECharacterSelectInputAction Action, bool Repeat = false) { Join->HandleInput(ECharacterSelectDeviceType::Keyboard, 0, Action, true, Repeat); }
		void Pad(int32 Id, ECharacterSelectInputAction Action, bool Repeat = false) { Join->HandleInput(ECharacterSelectDeviceType::Gamepad, Id, Action, true, Repeat); }
	};
}
using namespace CharacterSelectJoinTests;
using EAction = ECharacterSelectInputAction;
using EDevice = ECharacterSelectDeviceType;

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCharacterJoinOwnershipTest, "CharacterSelection.Controls.DeviceOwnership", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FCharacterJoinOwnershipTest::RunTest(const FString& Parameters)
{
	FSession S;
	TestFalse(TEXT("P1 initially waiting"), S.Join->HasJoined(0));
	TestFalse(TEXT("P2 initially waiting"), S.Join->HasJoined(1));
	S.Key(EAction::Confirm, true);
	TestFalse(TEXT("Repeat cannot claim a device"), S.Join->HasJoined(0));
	S.Key(EAction::Confirm);
	TestTrue(TEXT("First keyboard joins P1"), S.Join->HasJoined(0));
	TestFalse(TEXT("Join press does not confirm"), S.P1->IsConfirmed());
	S.Join->HandleInput(EDevice::Keyboard, 99, EAction::NavigateRight, true);
	TestFalse(TEXT("Another keyboard ID cannot join P2"), S.Join->HasJoined(1));
	TestEqual(TEXT("All keyboards operate their sole owner"), S.P1->GetCurrentIndex(), 1);
	S.Pad(12, EAction::Confirm);
	TestTrue(TEXT("Controller joins P2 alongside keyboard"), S.Join->HasJoined(1));
	TestFalse(TEXT("P2 join press consumed"), S.P2->IsConfirmed());
	S.Pad(12, EAction::NavigateLeft);
	TestEqual(TEXT("Controller moves only P2"), S.P2->GetCurrentIndex(), 2);
	TestEqual(TEXT("P1 unchanged by P2 controller"), S.P1->GetCurrentIndex(), 1);
	S.Pad(13, EAction::Confirm);
	TestEqual(TEXT("Third device cannot steal a player"), S.Join->GetAssignment(1).DeviceId, 12);
	S.Key(EAction::Confirm);
	TestFalse(TEXT("Human P2 prevents CPU handoff"), S.Join->IsCPUOpponent());
	S.Key(EAction::NavigateRight);
	TestEqual(TEXT("P1 cannot operate human P2"), S.P2->GetCurrentIndex(), 2);
	S.Pad(12, EAction::Confirm);
	TestTrue(TEXT("Two humans ready"), S.Join->IsReady());
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCharacterJoinGamepadsTest, "CharacterSelection.Controls.GamepadsAndKeyboardP2", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FCharacterJoinGamepadsTest::RunTest(const FString& Parameters)
{
	FSession S;
	S.Pad(4, EAction::Confirm);
	S.Pad(4, EAction::NavigateRight);
	TestFalse(TEXT("Same controller cannot join both players"), S.Join->HasJoined(1));
	S.Pad(8, EAction::Confirm);
	TestEqual(TEXT("Two gamepads have independent owners"), S.Join->GetPlayerForDevice(EDevice::Gamepad, 8), 1);
	S.Pad(8, EAction::NavigateLeft);
	TestEqual(TEXT("P1 gamepad stays independent"), S.P1->GetCurrentIndex(), 1);
	TestEqual(TEXT("P2 gamepad moves P2"), S.P2->GetCurrentIndex(), 2);
	FSession KeyboardP2;
	KeyboardP2.Pad(7, EAction::Confirm);
	KeyboardP2.Key(EAction::Confirm);
	KeyboardP2.Key(EAction::NavigateLeft);
	TestEqual(TEXT("Keyboard can belong to P2"), KeyboardP2.Join->GetPlayerForDevice(EDevice::Keyboard, 99), 1);
	TestEqual(TEXT("P2 keyboard moves P2"), KeyboardP2.P2->GetCurrentIndex(), 2);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCharacterJoinCPUTest, "CharacterSelection.Controls.CPUHandoffAndLateJoin", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FCharacterJoinCPUTest::RunTest(const FString& Parameters)
{
	FSession S;
	S.Key(EAction::Confirm);
	TestFalse(TEXT("No CPU selection until P1 confirms"), S.Join->IsCPUOpponent());
	S.Key(EAction::Confirm);
	TestTrue(TEXT("P1 confirms and controls CPU"), S.Join->IsCPUOpponent());
	TestEqual(TEXT("Target automatically becomes CPU"), S.Join->GetPlayerOneControlTarget(), 1);
	S.Key(EAction::Confirm, true);
	TestFalse(TEXT("Held confirm cannot confirm CPU"), S.P2->IsConfirmed());
	S.Key(EAction::NavigateLeft);
	TestEqual(TEXT("P1 device moves CPU"), S.P2->GetCurrentIndex(), 2);
	TestEqual(TEXT("P1 character remains locked in"), S.P1->GetCurrentIndex(), 0);
	S.Key(EAction::Confirm);
	TestTrue(TEXT("P1 and CPU ready"), S.Join->IsReady());
	S.Key(EAction::Cancel);
	TestFalse(TEXT("Back first unconfirms CPU"), S.P2->IsConfirmed());
	TestTrue(TEXT("P1 still confirmed after CPU undo"), S.P1->IsConfirmed());
	S.Key(EAction::Cancel);
	TestFalse(TEXT("Back again returns to P1 selection"), S.P1->IsConfirmed());
	TestEqual(TEXT("Control returns to P1"), S.Join->GetPlayerOneControlTarget(), 0);
	S.Key(EAction::Confirm);
	S.Key(EAction::Confirm);
	S.Pad(10, EAction::Confirm);
	TestFalse(TEXT("Late join replaces CPU mode"), S.Join->IsCPUOpponent());
	TestTrue(TEXT("Late join preserves P1 confirmation"), S.P1->IsConfirmed());
	TestFalse(TEXT("Late join clears inherited CPU confirmation"), S.P2->IsConfirmed());
	TestFalse(TEXT("Late join must choose before ready"), S.Join->IsReady());
	TestEqual(TEXT("Late join preserves preview position"), S.P2->GetCurrentIndex(), 2);
	S.Key(EAction::NavigateLeft);
	TestEqual(TEXT("P1 no longer controls P2"), S.P2->GetCurrentIndex(), 2);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCharacterJoinDisconnectTest, "CharacterSelection.Controls.DisconnectAndReset", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FCharacterJoinDisconnectTest::RunTest(const FString& Parameters)
{
	FSession S;
	S.Pad(4, EAction::Confirm);
	S.Key(EAction::Confirm);
	S.Pad(4, EAction::Confirm);
	S.Key(EAction::Confirm);
	TestTrue(TEXT("Ready before disconnect"), S.Join->IsReady());
	S.Join->SetGamepadConnected(4, false);
	TestFalse(TEXT("Disconnect blocks ready"), S.Join->IsReady());
	TestTrue(TEXT("Disconnected slot keeps its owner"), S.Join->HasJoined(0));
	S.Pad(5, EAction::Cancel);
	TestEqual(TEXT("Another pad cannot steal disconnected slot"), S.Join->GetAssignment(0).DeviceId, 4);
	S.Join->HandleInput(EDevice::Gamepad, 4, EAction::NavigateRight, false, true);
	TestFalse(TEXT("Analog or repeat cannot reconnect"), S.Join->GetAssignment(0).bConnected);
	S.Pad(4, EAction::Cancel);
	TestTrue(TEXT("Button on original device reconnects"), S.Join->GetAssignment(0).bConnected);
	TestTrue(TEXT("Reconnect press does not cancel confirmation"), S.P1->IsConfirmed());
	S.Join->Initialize({S.P1, S.P2});
	TestFalse(TEXT("New session clears ownership"), S.Join->HasJoined(0));
	TestFalse(TEXT("Reset cannot report ready"), S.Join->IsReady());
	TestFalse(TEXT("Reset clears P1 confirmation"), S.P1->IsConfirmed());
	TestFalse(TEXT("Reset clears P2 confirmation"), S.P2->IsConfirmed());
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCharacterJoinBindingsTest, "CharacterSelection.Controls.BindingsAndStickRepeat", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FCharacterJoinBindingsTest::RunTest(const FString& Parameters)
{
	TestTrue(TEXT("Keyboard joins"), FCharacterSelectInputBindings::CanJoin(EKeys::Enter));
	TestTrue(TEXT("Start button joins"), FCharacterSelectInputBindings::CanJoin(EKeys::Gamepad_Special_Right));
	TestFalse(TEXT("Mouse is not a join device"), FCharacterSelectInputBindings::CanJoin(EKeys::LeftMouseButton));
	TestFalse(TEXT("Analog drift cannot join"), FCharacterSelectInputBindings::CanJoin(EKeys::Gamepad_LeftX));
	TestFalse(TEXT("Virtual stick direction cannot join"), FCharacterSelectInputBindings::CanJoin(EKeys::Gamepad_LeftStick_Up));
	TestEqual(TEXT("Arrow and WASD share mapping"), FCharacterSelectInputBindings::GetAction(EKeys::Left), FCharacterSelectInputBindings::GetAction(EKeys::A));
	TestEqual(TEXT("Bottom face button confirms"), FCharacterSelectInputBindings::GetAction(EKeys::Gamepad_FaceButton_Bottom), EAction::Confirm);
	TestEqual(TEXT("Right face button cancels"), FCharacterSelectInputBindings::GetAction(EKeys::Gamepad_FaceButton_Right), EAction::Cancel);
	FCharacterSelectStickRepeat Stick;
	Stick.Axes = FVector2D(0.3, 0.2);
	TestEqual(TEXT("Dead zone blocks drift"), Stick.Update(0), EAction::None);
	Stick.Axes = FVector2D(0.8, 0.7);
	TestEqual(TEXT("Dominant axis moves immediately"), Stick.Update(1), EAction::NavigateRight);
	TestEqual(TEXT("Initial repeat delay"), Stick.Update(1.2), EAction::None);
	TestEqual(TEXT("Repeat after delay"), Stick.Update(1.36), EAction::NavigateRight);
	TestEqual(TEXT("Rate limited repeat"), Stick.Update(1.4), EAction::None);
	Stick.Axes = FVector2D(0.1, 0.1);
	TestEqual(TEXT("Release resets repeat"), Stick.Update(1.42), EAction::None);
	Stick.Axes = FVector2D(0, -0.9);
	TestEqual(TEXT("New direction immediate after release"), Stick.Update(1.43), EAction::NavigateDown);
	Stick.Reset();
	TestEqual(TEXT("Focus loss reset clears held direction"), Stick.Update(2), EAction::None);
	return true;
}
#endif
