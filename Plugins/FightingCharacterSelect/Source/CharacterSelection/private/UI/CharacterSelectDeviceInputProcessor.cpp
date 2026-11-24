#include "CharacterSelectDeviceInputProcessor.h"
#include "UI/CharacterSelectWidget.h"
#include "Core/CharacterSelectJoinSession.h"
#include "Input/Events.h"
#include "GenericPlatform/GenericPlatformInputDeviceMapper.h"

FCharacterSelectDeviceInputProcessor::FCharacterSelectDeviceInputProcessor(UCharacterSelectWidget* InWidget) : Widget(InWidget)
{
	ConnectionHandle = IPlatformInputDeviceMapper::Get().GetOnInputDeviceConnectionChange().AddRaw(this, &FCharacterSelectDeviceInputProcessor::OnConnectionChanged);
}
FCharacterSelectDeviceInputProcessor::~FCharacterSelectDeviceInputProcessor()
{
	IPlatformInputDeviceMapper::Get().GetOnInputDeviceConnectionChange().Remove(ConnectionHandle);
}
void FCharacterSelectDeviceInputProcessor::OnConnectionChanged(EInputDeviceConnectionState State, FPlatformUserId User, FInputDeviceId Device)
{
	Sticks.Remove(Device.GetId());
	if (State == EInputDeviceConnectionState::Disconnected && Widget.IsValid()) Widget->NotifyGamepadDisconnected(Device.GetId());
}
void FCharacterSelectDeviceInputProcessor::Tick(float DeltaTime, FSlateApplication& SlateApp, TSharedRef<ICursor> Cursor)
{
	if (!Widget.IsValid() || !Widget->IsDeviceInputActive()) { Sticks.Reset(); return; }
	for (auto& Pair : Sticks)
	{
		auto* Session = Widget->GetJoinSession();
		if (!Session || Session->GetPlayerForDevice(ECharacterSelectDeviceType::Gamepad, Pair.Key) == INDEX_NONE) { Pair.Value.Reset(); continue; }
		const auto Action = Pair.Value.Update(FPlatformTime::Seconds(), Widget->ControlSettings);
		if (Action != ECharacterSelectInputAction::None) Widget->HandleDeviceAction(Pair.Key, Action);
	}
}
bool FCharacterSelectDeviceInputProcessor::HandleKeyDownEvent(FSlateApplication& SlateApp, const FKeyEvent& Event)
{
	return Widget.IsValid() && Widget->IsDeviceInputActive() && Widget->HandleDeviceKey(Event);
}
bool FCharacterSelectDeviceInputProcessor::HandleKeyUpEvent(FSlateApplication& SlateApp, const FKeyEvent& Event)
{
	return Widget.IsValid() && Widget->IsDeviceInputActive() && Event.GetKey().IsGamepadKey();
}
bool FCharacterSelectDeviceInputProcessor::HandleAnalogInputEvent(FSlateApplication& SlateApp, const FAnalogInputEvent& Event)
{
	if (!Widget.IsValid() || !Widget->IsDeviceInputActive() || !Event.GetKey().IsGamepadKey()) return false;
	const int32 Device = Event.GetInputDeviceId().GetId();
	if (Device >= 0)
	{
		if (Event.GetKey() == EKeys::Gamepad_LeftX) Sticks.FindOrAdd(Device).Axes.X = Event.GetAnalogValue();
		if (Event.GetKey() == EKeys::Gamepad_LeftY) Sticks.FindOrAdd(Device).Axes.Y = Event.GetAnalogValue();
	}
	return true;
}
