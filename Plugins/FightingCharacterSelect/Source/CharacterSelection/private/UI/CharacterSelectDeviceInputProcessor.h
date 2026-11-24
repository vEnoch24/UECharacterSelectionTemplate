#pragma once
#include "Framework/Application/IInputProcessor.h"
#include "Core/CharacterSelectStickRepeat.h"

class UCharacterSelectWidget;
class FCharacterSelectDeviceInputProcessor : public IInputProcessor
{
public:
	explicit FCharacterSelectDeviceInputProcessor(UCharacterSelectWidget* InWidget);
	virtual ~FCharacterSelectDeviceInputProcessor() override;
	virtual void Tick(float DeltaTime, FSlateApplication& SlateApp, TSharedRef<ICursor> Cursor) override;
	virtual bool HandleKeyDownEvent(FSlateApplication& SlateApp, const FKeyEvent& Event) override;
	virtual bool HandleKeyUpEvent(FSlateApplication& SlateApp, const FKeyEvent& Event) override;
	virtual bool HandleAnalogInputEvent(FSlateApplication& SlateApp, const FAnalogInputEvent& Event) override;
	virtual const TCHAR* GetDebugName() const override { return TEXT("CharacterSelectDeviceInput"); }
private:
	void OnConnectionChanged(EInputDeviceConnectionState State, FPlatformUserId User, FInputDeviceId Device);
	TWeakObjectPtr<UCharacterSelectWidget> Widget;
	TMap<int32, FCharacterSelectStickRepeat> Sticks;
	FDelegateHandle ConnectionHandle;
};
