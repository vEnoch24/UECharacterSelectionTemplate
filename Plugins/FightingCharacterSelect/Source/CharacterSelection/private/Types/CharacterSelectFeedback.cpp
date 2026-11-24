#include "Types/CharacterSelectFeedback.h"

FKey FCharacterSelectHintSettings::ResolveKey(const FCharacterSelectControlSettings& C, ECharacterSelectInputAction Action, bool bGamepad)
{
	const TArray<FKey>* Keys = nullptr;
	switch (Action)
	{
	case ECharacterSelectInputAction::NavigateUp: Keys = &C.Up; break;
	case ECharacterSelectInputAction::NavigateDown: Keys = &C.Down; break;
	case ECharacterSelectInputAction::NavigateLeft: Keys = &C.Left; break;
	case ECharacterSelectInputAction::NavigateRight: Keys = &C.Right; break;
	case ECharacterSelectInputAction::Confirm: Keys = &C.Confirm; break;
	case ECharacterSelectInputAction::Cancel: Keys = &C.Cancel; break;
	default: break;
	}
	if (Keys) for (FKey Key : *Keys) if (Key.IsValid() && Key.IsGamepadKey() == bGamepad) return Key;
	return FKey();
}

const FSlateBrush* FCharacterSelectHintSettings::FindGlyph(FKey Key, int32 DeviceOwner) const
{
	if (!Key.IsValid()) return nullptr;
	if (DeviceOwner == 1 && Key.IsGamepadKey())
		if (const auto* Brush = Player2GamepadGlyphs.Find(Key); Brush && Brush->GetResourceObject()) return Brush;
	const auto* Brush = Glyphs.Find(Key);
	return Brush && Brush->GetResourceObject() ? Brush : nullptr;
}

FText FCharacterSelectHintSettings::GetKeyLabel(FKey Key) const
{
	if (!Key.IsValid()) return NSLOCTEXT("CharacterSelect", "Unbound", "Unbound");
	if (const auto* Label = KeyLabels.Find(Key)) return *Label;
	return Key.GetDisplayName();
}

FText FCharacterSelectHintSettings::GetActionLabel(ECharacterSelectInputAction Action) const
{
	if (const auto* Label = ActionLabels.Find(Action)) return *Label;
	switch (Action)
	{
	case ECharacterSelectInputAction::NavigateUp: return NSLOCTEXT("CharacterSelect", "HintUp", "Up");
	case ECharacterSelectInputAction::NavigateDown: return NSLOCTEXT("CharacterSelect", "HintDown", "Down");
	case ECharacterSelectInputAction::NavigateLeft: return NSLOCTEXT("CharacterSelect", "HintLeft", "Left");
	case ECharacterSelectInputAction::NavigateRight: return NSLOCTEXT("CharacterSelect", "HintRight", "Right");
	case ECharacterSelectInputAction::Confirm: return NSLOCTEXT("CharacterSelect", "HintConfirm", "Confirm");
	case ECharacterSelectInputAction::Cancel: return NSLOCTEXT("CharacterSelect", "HintCancel", "Back");
	default: return FText::GetEmpty();
	}
}
