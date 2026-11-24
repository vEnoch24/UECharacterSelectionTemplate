#include "Types/CharacterSelectPreviewAnimation.h"
FCharacterPreviewAnimationSettings FCharacterPreviewAnimationSettings::MakePreset(ECharacterPreviewPreset Preset)
{
    FCharacterPreviewAnimationSettings Result;
    Result.Mode = ECharacterPreviewAnimationMode::Configured;
    for (auto* Element : {&Result.Portrait, &Result.Name, &Result.Subtitle})
    {
        Element->Exit.Duration = 0.12f;
        if (Preset == ECharacterPreviewPreset::Slide || Preset == ECharacterPreviewPreset::Staggered)
        {
            Element->Entrance.Translation.X = 28.f;
            Element->Exit.Translation.X = -18.f;
        }
        if (Preset == ECharacterPreviewPreset::Zoom)
        {
            Element->Entrance.Scale = 0.92f;
            Element->Exit.Scale = 1.04f;
        }
        if (Preset == ECharacterPreviewPreset::NoMotion) Element->bEnabled = false;
    }
    if (Preset == ECharacterPreviewPreset::Staggered)
    {
        Result.Name.Entrance.Delay = 0.07f;
        Result.Subtitle.Entrance.Delay = 0.14f;
    }
    return Result;
}
