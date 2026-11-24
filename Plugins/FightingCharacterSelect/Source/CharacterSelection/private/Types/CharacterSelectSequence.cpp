#include "Types/CharacterSelectSequence.h"
FCharacterSelectSequence FCharacterSelectSequence::MakePreset(int32 Preset)
{
    FCharacterSelectSequence Result;
    Result.bEnabled = true;
    Result.Transitions = FCharacterPreviewAnimationSettings::MakePreset(Preset == 3 ? ECharacterPreviewPreset::NoMotion : ECharacterPreviewPreset::Fade);
    Result.EntranceOrder = Preset == 1 ? ECharacterSequenceOrder::PortraitFirst : Preset == 2 ? ECharacterSequenceOrder::TextFirst : ECharacterSequenceOrder::Together;
    Result.ExitOrder = Preset == 1 ? ECharacterSequenceOrder::TextFirst : Preset == 2 ? ECharacterSequenceOrder::PortraitFirst : ECharacterSequenceOrder::Together;
    return Result;
}
FCharacterPreviewAnimationSettings FCharacterSelectSequence::GetTransitions(bool bExit) const
{
    auto Result = Transitions;
    Result.Mode = ECharacterPreviewAnimationMode::Configured;
    const auto Order = bExit ? ExitOrder : EntranceOrder;
    auto Phase = [bExit](FCharacterPreviewElementAnimation& Element) -> FCharacterPreviewTransition& { return bExit ? Element.Exit : Element.Entrance; };
    auto Length = [&Phase](FCharacterPreviewElementAnimation& Element) { const auto& P = Phase(Element); return Element.bEnabled ? FMath::Clamp(FMath::IsFinite(P.Delay) ? P.Delay : 0.f, 0.f, 5.f) + FMath::Clamp(FMath::IsFinite(P.Duration) ? P.Duration : 0.f, 0.f, 5.f) : 0.f; };
    const float Gap = FMath::Clamp(FMath::IsFinite(GroupGap) ? GroupGap : 0.f, 0.f, 5.f);
    if (Order == ECharacterSequenceOrder::PortraitFirst)
    {
        const float Offset = Length(Result.Portrait) + Gap;
        Phase(Result.Name).Delay += Offset; Phase(Result.Subtitle).Delay += Offset;
    }
    else if (Order == ECharacterSequenceOrder::TextFirst)
        Phase(Result.Portrait).Delay += FMath::Max(Length(Result.Name), Length(Result.Subtitle)) + Gap;
    return Result;
}
