#pragma once
#include "CoreMinimal.h"
#include "Types/CharacterSelectInputTypes.h"
#include "Types/CharacterSelectCustomization.h"

/** Frame-based, dominant-axis stick navigation with a release dead zone. */
struct FCharacterSelectStickRepeat
{
	FVector2D Axes = FVector2D::ZeroVector;
	ECharacterSelectInputAction Held = ECharacterSelectInputAction::None;
	double NextRepeat = 0.0;
	void Reset() { Axes = FVector2D::ZeroVector; Held = ECharacterSelectInputAction::None; NextRepeat = 0.0; }
	ECharacterSelectInputAction Update(double Now, const FCharacterSelectControlSettings& Settings = FCharacterSelectControlSettings())
	{
		const double Magnitude = FMath::Max(FMath::Abs(Axes.X), FMath::Abs(Axes.Y));
		if (!Settings.bEnableLeftStick) { Reset(); return ECharacterSelectInputAction::None; }
		const double Engage = FMath::Clamp(Settings.EngageThreshold, 0.01f, 1.0f);
		const double Threshold = Held == ECharacterSelectInputAction::None ? Engage : FMath::Clamp<double>(Settings.ReleaseThreshold, 0.0, Engage);
		if (Magnitude < Threshold) { Held = ECharacterSelectInputAction::None; return ECharacterSelectInputAction::None; }
		const auto Direction = FMath::Abs(Axes.X) >= FMath::Abs(Axes.Y)
			? (Axes.X > 0 ? ECharacterSelectInputAction::NavigateRight : ECharacterSelectInputAction::NavigateLeft)
			: (Axes.Y > 0 ? ECharacterSelectInputAction::NavigateUp : ECharacterSelectInputAction::NavigateDown);
		if (Direction != Held) { Held = Direction; NextRepeat = Now + FMath::Max(0.01f, Settings.InitialRepeatDelay); return Held; }
		if (Now >= NextRepeat) { NextRepeat = Now + FMath::Max(0.01f, Settings.RepeatInterval); return Held; }
		return ECharacterSelectInputAction::None;
	}
};
