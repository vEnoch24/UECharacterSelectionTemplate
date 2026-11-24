#include "Types/CharacterSelectSlotAnimation.h"

FCharacterSlotAnimationSettings::FCharacterSlotAnimationSettings()
{
    Focused.Scale = 1.05f;
    SharedFocus.Scale = 1.08f;
    Selected.Scale = 1.1f;
    SharedSelection.Scale = 1.12f;
    Locked.Opacity = 0.6f;
}

FCharacterSlotAnimationSettings FCharacterSlotAnimationSettings::MakePreset(ECharacterSlotAnimationPreset Preset)
{
    FCharacterSlotAnimationSettings Result;
    Result.Mode = ECharacterSlotAnimationMode::Configured;
    if (Preset == ECharacterSlotAnimationPreset::Pop)
    {
        Result.Focused.Scale = 1.10f; Result.SharedFocus.Scale = 1.14f;
        Result.Selected.Scale = 1.16f; Result.SharedSelection.Scale = 1.20f;
        Result.Focused.Duration = Result.SharedFocus.Duration = 0.10f;
        Result.Selected.Duration = Result.SharedSelection.Duration = 0.12f;
        Result.Focused.Easing = Result.SharedFocus.Easing = Result.Selected.Easing = Result.SharedSelection.Easing = ECharacterSlotEasing::EaseOut;
    }
    else if (Preset == ECharacterSlotAnimationPreset::Lift)
    {
        Result.Focused.Translation.Y = -6.f; Result.SharedFocus.Translation.Y = -9.f;
        Result.Selected.Translation.Y = -12.f; Result.SharedSelection.Translation.Y = -15.f;
    }
    else if (Preset == ECharacterSlotAnimationPreset::Fade || Preset == ECharacterSlotAnimationPreset::NoMotion)
    {
        Result.Focused.Scale = Result.SharedFocus.Scale = Result.Selected.Scale = Result.SharedSelection.Scale = 1.f;
        if (Preset == ECharacterSlotAnimationPreset::Fade) Result.Idle.Opacity = 0.7f;
        else
        {
            Result.Idle.Duration = Result.Focused.Duration = Result.SharedFocus.Duration = Result.Selected.Duration = Result.SharedSelection.Duration = Result.Locked.Duration = Result.CancelDuration = 0.f;
            Result.Locked.Opacity = 1.f;
        }
    }
    return Result;
}
