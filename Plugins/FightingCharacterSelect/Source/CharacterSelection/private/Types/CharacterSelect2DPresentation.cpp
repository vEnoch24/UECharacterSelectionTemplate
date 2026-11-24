#include "Types/CharacterSelect2DPresentation.h"
#include "UI/CharacterSelectPresentationWidget.h"
int32 FCharacterSelect2DPresentation::GetFrameIndex(double Seconds) const
{
    if (Frames.IsEmpty()) return INDEX_NONE;
    const double FPS = FMath::IsFinite(FramesPerSecond) ? FMath::Clamp(FramesPerSecond, .1f, 120.f) : 12.f;
    Seconds = FMath::IsFinite(Seconds) ? FMath::Max(0., Seconds) : 0.;
    const double Duration = Frames.Num() / FPS;
    const double Position = bLoop ? FMath::Fmod(Seconds, Duration) * FPS : FMath::Min(Seconds, Duration) * FPS;
    return FMath::Clamp(int32(Position), 0, Frames.Num()-1);
}
TArray<FString> FCharacterSelect2DPresentation::GetIssues() const
{
    TArray<FString> Issues;
    if (Offset.ContainsNaN() || Scale.ContainsNaN() || Scale.X < .01 || Scale.Y < .01 || Scale.X > 10 || Scale.Y > 10)
        Issues.Add(TEXT("2D framing requires finite offsets and scale between 0.01 and 10."));
    if (Mode == ECharacter2DPresentationMode::SpriteFrames)
    {
        if (Frames.IsEmpty() || Frames.Contains(nullptr)) Issues.Add(TEXT("Sprite animation requires a non-empty frame list with no empty entries."));
        if (!FMath::IsFinite(FramesPerSecond) || FramesPerSecond < .1f || FramesPerSecond > 120.f) Issues.Add(TEXT("Sprite frame rate must be between 0.1 and 120."));
    }
    if (Mode == ECharacter2DPresentationMode::CustomWidget && (!WidgetClass || WidgetClass->HasAnyClassFlags(CLASS_Abstract)))
        Issues.Add(TEXT("Custom 2D presentation requires a concrete CharacterSelectPresentationWidget class."));
    return Issues;
}
