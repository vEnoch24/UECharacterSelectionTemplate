#pragma once
#include "CoreMinimal.h"
#include "InputCoreTypes.h"
#include "Styling/SlateTypes.h"
#include "CharacterSelectCustomization.generated.h"

USTRUCT(BlueprintType)
struct FCharacterSelectControlSettings
{
	GENERATED_BODY()
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Bindings")
	TArray<FKey> Up = {EKeys::W, EKeys::Up, EKeys::Gamepad_DPad_Up};
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Bindings")
	TArray<FKey> Down = {EKeys::S, EKeys::Down, EKeys::Gamepad_DPad_Down};
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Bindings")
	TArray<FKey> Left = {EKeys::A, EKeys::Left, EKeys::Gamepad_DPad_Left};
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Bindings")
	TArray<FKey> Right = {EKeys::D, EKeys::Right, EKeys::Gamepad_DPad_Right};
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Bindings")
	TArray<FKey> Confirm = {EKeys::Enter, EKeys::SpaceBar, EKeys::Gamepad_FaceButton_Bottom, EKeys::Gamepad_Special_Right};
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Bindings")
	TArray<FKey> Cancel = {EKeys::Escape, EKeys::BackSpace, EKeys::Gamepad_FaceButton_Right};
	/** When false, only a configured Confirm key/button joins. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Joining")
	bool bAnyButtonJoins = true;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Stick")
	bool bEnableLeftStick = true;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Stick", meta=(ClampMin="0.01", ClampMax="1.0"))
	float EngageThreshold = 0.55f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Stick", meta=(ClampMin="0.0", ClampMax="1.0"))
	float ReleaseThreshold = 0.35f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Stick", meta=(ClampMin="0.01"))
	float InitialRepeatDelay = 0.35f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Stick", meta=(ClampMin="0.01"))
	float RepeatInterval = 0.12f;
};

USTRUCT(BlueprintType)
struct FCharacterSelectPromptAppearance
{
	GENERATED_BODY()
	/** Applies only to generated banners. Custom UMG text widgets keep their Designer styling. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Banner")
	bool bShowGeneratedBanners = true;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Banner")
	bool bPlaceAtBottom = false;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Banner")
	FMargin ScreenMargin = FMargin(20.0f, 24.0f);
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Banner")
	FMargin BannerPadding = FMargin(12.0f);
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Banner", meta=(ClampMin="0"))
	float BannerGap = 12.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Banner")
	FLinearColor Background = FLinearColor(0.02f, 0.02f, 0.03f, 0.85f);
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Banner")
	FLinearColor TextColor = FLinearColor::White;
	/** Leave Font Object empty to use the built-in bold font. Size is always used. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Banner")
	FSlateFontInfo Font;
	FCharacterSelectPromptAppearance() { Font.Size = 16; }
};

USTRUCT(BlueprintType)
struct FCharacterSelectPromptText
{
	GENERATED_BODY()
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Text")
	FText JoinAny = NSLOCTEXT("CharacterSelect", "JoinAny", "Press a key or controller button to join");
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Text")
	FText JoinController = NSLOCTEXT("CharacterSelect", "JoinController", "Press a button on another controller to join");
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Text")
	FText Waiting = NSLOCTEXT("CharacterSelect", "Waiting", "WAITING");
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Text")
	FText Selecting = NSLOCTEXT("CharacterSelect", "Selecting", "SELECT CHARACTER");
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Text")
	FText Ready = NSLOCTEXT("CharacterSelect", "Ready", "READY");
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Text")
	FText CPUSelecting = NSLOCTEXT("CharacterSelect", "CPUSelecting", "CPU OPPONENT - PLAYER 1 SELECTING");
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Text")
	FText CPUReady = NSLOCTEXT("CharacterSelect", "CPUReady", "CPU OPPONENT - READY");
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Text", meta=(MultiLine=true))
	FText Disconnected = NSLOCTEXT("CharacterSelect", "Disconnected", "CONTROLLER DISCONNECTED\nReconnect it and press a button");
	/** Keep these hints consistent with your custom key bindings. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Text", meta=(MultiLine=true))
	FText KeyboardHelp = NSLOCTEXT("CharacterSelect", "KeyboardHelp", "WASD / Arrows: Move   Enter / Space: Confirm   Esc: Back");
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Text", meta=(MultiLine=true))
	FText GamepadHelp = NSLOCTEXT("CharacterSelect", "GamepadHelp", "D-pad / Left Stick: Move   A / Cross: Confirm   B / Circle: Back");
};
