#if WITH_DEV_AUTOMATION_TESTS
#include "Misc/AutomationTest.h"
#include "Core/CharacterSelectInputBindings.h"
#include "Core/CharacterSelectStickRepeat.h"
#include "Core/CharacterSelectJoinSession.h"
#include "Core/CharacterSelectionController.h"
#include "Data/CharacterSelectRegistry.h"
#include "Data/CharacterSelectData.h"
#include "Data/CharacterSelectLayoutData.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCharacterCustomControlsTest, "CharacterSelection.Customization.Controls", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FCharacterCustomControlsTest::RunTest(const FString& Parameters)
{
	FCharacterSelectControlSettings Settings;
	Settings.Confirm = {EKeys::J};
	TestEqual(TEXT("Custom key confirms"), FCharacterSelectInputBindings::GetAction(EKeys::J, Settings), ECharacterSelectInputAction::Confirm);
	TestEqual(TEXT("Replaced key no longer confirms"), FCharacterSelectInputBindings::GetAction(EKeys::Enter, Settings), ECharacterSelectInputAction::None);
	FCharacterSelectStickRepeat Stick;
	Settings.EngageThreshold = 0.8f;
	Settings.InitialRepeatDelay = 0.5f;
	Settings.RepeatInterval = 0.2f;
	Stick.Axes.X = 0.7;
	TestEqual(TEXT("Custom threshold blocks low input"), Stick.Update(0, Settings), ECharacterSelectInputAction::None);
	Stick.Axes.X = 0.9;
	TestEqual(TEXT("Engagement moves immediately"), Stick.Update(1, Settings), ECharacterSelectInputAction::NavigateRight);
	TestEqual(TEXT("Custom initial delay"), Stick.Update(1.4, Settings), ECharacterSelectInputAction::None);
	TestEqual(TEXT("Custom repeat starts"), Stick.Update(1.51, Settings), ECharacterSelectInputAction::NavigateRight);
	TestEqual(TEXT("Custom repeat interval"), Stick.Update(1.65, Settings), ECharacterSelectInputAction::None);
	Settings.bEnableLeftStick = false;
	TestEqual(TEXT("Disabled stick stops"), Stick.Update(2, Settings), ECharacterSelectInputAction::None);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCharacterCustomJoinTest, "CharacterSelection.Customization.JoinRules", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FCharacterCustomJoinTest::RunTest(const FString& Parameters)
{
	auto* Registry = NewObject<UCharacterSelectRegistry>();
	Registry->Characters.Add(NewObject<UCharacterSelectData>());
	auto* Layout = NewObject<UCharacterSelectLayoutData>();
	auto* P1 = NewObject<UCharacterSelectionController>();
	auto* P2 = NewObject<UCharacterSelectionController>();
	P1->Initialize(Registry, Layout, 0, 0); P2->Initialize(Registry, Layout, 1, 0);
	auto* Session = NewObject<UCharacterSelectJoinSession>();
	Session->bAutoCPUOpponent = false;
	Session->Initialize({P1, P2});
	Session->HandleInput(ECharacterSelectDeviceType::Keyboard, 0, ECharacterSelectInputAction::Confirm, true);
	Session->HandleInput(ECharacterSelectDeviceType::Keyboard, 0, ECharacterSelectInputAction::Confirm, true);
	TestFalse(TEXT("CPU handoff can be disabled"), Session->IsCPUOpponent());
	TestFalse(TEXT("Missing second human is not ready"), Session->IsReady());
	Session->bAutoCPUOpponent = true;
	Session->bAllowLatePlayerJoin = false;
	TestTrue(TEXT("Default CPU flow available"), Session->IsCPUOpponent());
	Session->HandleInput(ECharacterSelectDeviceType::Gamepad, 4, ECharacterSelectInputAction::Confirm, true);
	TestFalse(TEXT("Late joining can be disabled"), Session->HasJoined(1));
	Session->bAllowLatePlayerJoin = true;
	Session->HandleInput(ECharacterSelectDeviceType::Gamepad, 4, ECharacterSelectInputAction::Confirm, true);
	TestTrue(TEXT("Late joining can be enabled"), Session->HasJoined(1));
	return true;
}
#endif
