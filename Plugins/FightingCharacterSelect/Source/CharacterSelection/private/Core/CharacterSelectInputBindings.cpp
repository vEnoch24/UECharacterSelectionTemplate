#include "Core/CharacterSelectInputBindings.h"

ECharacterSelectInputAction FCharacterSelectInputBindings::GetAction(FKey Key)
{
	return GetAction(Key, FCharacterSelectControlSettings());
}

ECharacterSelectInputAction FCharacterSelectInputBindings::GetAction(FKey Key, const FCharacterSelectControlSettings& Settings)
{
	if (Settings.Up.Contains(Key)) return ECharacterSelectInputAction::NavigateUp;
	if (Settings.Down.Contains(Key)) return ECharacterSelectInputAction::NavigateDown;
	if (Settings.Left.Contains(Key)) return ECharacterSelectInputAction::NavigateLeft;
	if (Settings.Right.Contains(Key)) return ECharacterSelectInputAction::NavigateRight;
	if (Settings.Confirm.Contains(Key)) return ECharacterSelectInputAction::Confirm;
	if (Settings.Cancel.Contains(Key)) return ECharacterSelectInputAction::Cancel;
	return ECharacterSelectInputAction::None;
}

bool FCharacterSelectInputBindings::CanJoin(FKey Key)
{
	// Stick axes and virtual stick buttons cannot join through drift.
	if (!Key.IsValid() || Key.IsAnalog() || Key.IsMouseButton() || Key.IsModifierKey()) return false;
	if (Key == EKeys::Gamepad_LeftStick_Up || Key == EKeys::Gamepad_LeftStick_Down || Key == EKeys::Gamepad_LeftStick_Left || Key == EKeys::Gamepad_LeftStick_Right || Key == EKeys::Gamepad_RightStick_Up || Key == EKeys::Gamepad_RightStick_Down || Key == EKeys::Gamepad_RightStick_Left || Key == EKeys::Gamepad_RightStick_Right) return false;
	return Key != EKeys::Escape && Key != EKeys::Tilde && Key != EKeys::F11;
}
