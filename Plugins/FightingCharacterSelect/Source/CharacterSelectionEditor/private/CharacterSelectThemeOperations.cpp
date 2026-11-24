#include "CharacterSelectThemeOperations.h"
#include "Data/CharacterSelectTheme.h"
#include "Data/CharacterSelectLayoutData.h"
#include "UI/CharacterSelectWidget.h"
#include "UI/CharacterSelectGridWidget.h"
#include "UI/CharacterSelectSlotWidget.h"
#include "Blueprint/WidgetTree.h"
#include "WidgetBlueprint.h"
#include "Components/NamedSlot.h"
#include "Core/CharacterSelectNavigationProvider.h"
#include "Animation/WidgetAnimation.h"
#include "UObject/UnrealType.h"

namespace
{
	void RecordThemeChange(UObject* Object)
	{
		Object->SetFlags(RF_Transactional);
		Object->Modify();
	}
}

bool CharacterSelectThemeOperations::Capture(UCharacterSelectTheme* Theme, const FCharacterSelectThemeTargets& T, FString& Error)
{
	Error.Reset();
	if (!Theme || !T.Screen || !T.Grid || !T.Layout)
	{
		Error = TEXT("Select a theme, a full character-select screen containing exactly one grid, and its Layout Data Asset."); return false;
	}
	RecordThemeChange(Theme);
	Theme->GridLayout = T.Grid->GetGridLayout();
	Theme->NavigationMode = T.Layout->NavigationMode;
	Theme->HorizontalEdge = T.Layout->HorizontalEdgeBehavior;
	Theme->VerticalEdge = T.Layout->VerticalEdgeBehavior;
	Theme->PromptAppearance = T.Screen->PromptAppearance;
	Theme->PromptText = T.Screen->PromptText;
	Theme->PromptMinOpacity = T.Screen->JoinPromptMinOpacity;
	Theme->PromptPulseSpeed = T.Screen->JoinPromptPulseSpeed;
	Theme->Controls = T.Screen->ControlSettings;
	Theme->SoundSettings = T.Screen->SoundSettings;
	Theme->HintSettings = T.Screen->HintSettings;
	Theme->Players = T.Screen->PlayerConfigs;
	Theme->bAutoCPUOpponent = T.Screen->bAutoCPUOpponent;
	Theme->bAllowLatePlayerJoin = T.Screen->bAllowLatePlayerJoin;
    if (!Theme->bHasSnapshot) Theme->bApplyPresentation = true;
    Theme->bHasSlotMotion = T.Slot != nullptr;
    if (T.Slot) Theme->SlotMotion = T.Slot->SlotAnimationSettings;
	Theme->Previews.Reset();
	for (auto* Preview : T.Previews) if (Preview)
	{
		auto& Entry = Theme->Previews.Add(Preview->GetFName());
        Entry.bHasPresentation = true;
        Entry.Motion = Preview->PreviewAnimationSettings;
        Entry.Artwork = Preview->DefaultPresentation2D;
        Entry.bUse3D = Preview->bDefault3DPreview;
        Entry.Scene = Preview->DefaultPresentation3D;
        Entry.Sequence = Preview->ResolveSequence(nullptr);
		Entry.ActiveStyle = Preview->AnimationStyle;
		Entry.Styles = Preview->AnimationStyles;
	}
	Theme->bHasSnapshot = true;
	Theme->MarkPackageDirty();
	return true;
}

bool CharacterSelectThemeOperations::Apply(const UCharacterSelectTheme* Theme, const FCharacterSelectThemeTargets& T, FString& Error)
{
	Error.Reset();
	if (!Theme || !Theme->bHasSnapshot || !T.Screen)
	{
		Error = TEXT("Capture a theme first and select a full character-select screen to apply it to."); return false;
	}
	if (!(Theme->bApplyLayout || Theme->bApplyPrompts || Theme->bApplyPreviewAnimations || Theme->bApplyControls || Theme->bApplyPlayers || Theme->bApplyFeedback || Theme->bApplyPresentation || Theme->bApplySlotMotion))
	{
		Error = TEXT("Enable at least one Apply section in the theme."); return false;
	}
	if (Theme->bApplyLayout)
	{
		const auto& L = Theme->GridLayout;
		if (!T.Grid || !T.Layout || !UCharacterSelectNavigationProvider::IsModeSupported(Theme->NavigationMode)
			|| L.Columns < 1 || L.Rows < 0 || L.SlotSize.X <= 0 || L.SlotSize.Y <= 0 || L.SlotSpacing.X < 0 || L.SlotSpacing.Y < 0)
		{ Error = TEXT("Layout needs exactly one grid, a Layout Data Asset, supported navigation, and valid dimensions."); return false; }
	}
	if (Theme->bApplyPlayers && Theme->Players.Num() != 2)
	{ Error = TEXT("Player presets require exactly two player configurations."); return false; }
	if (Theme->bApplyPrompts && (Theme->PromptMinOpacity < 0 || Theme->PromptMinOpacity > 1 || Theme->PromptPulseSpeed < 0))
	{ Error = TEXT("Prompt opacity must be 0–1 and pulse speed cannot be negative."); return false; }
	if (Theme->bApplyFeedback && (Theme->SoundSettings.Volume < 0 || Theme->SoundSettings.Volume > 2 || Theme->SoundSettings.Pitch < 0.1f || Theme->SoundSettings.Pitch > 4 || Theme->HintSettings.IconSize < 8 || Theme->HintSettings.IconSize > 128))
	{ Error = TEXT("Feedback requires volume 0–2, pitch 0.1–4 and icon size 8–128."); return false; }
	if (Theme->bApplyControls)
	{
		const auto& C = Theme->Controls;
		if (C.ReleaseThreshold < 0 || C.EngageThreshold <= 0 || C.EngageThreshold > 1 || C.ReleaseThreshold > C.EngageThreshold || C.InitialRepeatDelay <= 0 || C.RepeatInterval <= 0)
		{ Error = TEXT("Invalid stick thresholds or repeat timings in the preset."); return false; }
		TSet<FKey> Used;
		for (const auto* Keys : {&C.Up, &C.Down, &C.Left, &C.Right, &C.Confirm, &C.Cancel})
		{
			if (Keys->IsEmpty()) { Error = TEXT("Each control action needs at least one key."); return false; }
			for (const FKey& Key : *Keys)
			{
				if (!Key.IsValid() || Key.IsAnalog() || Key.IsMouseButton() || Key.IsModifierKey() || Key == EKeys::F11 || Key == EKeys::Tilde || Used.Contains(Key))
				{ Error = TEXT("Invalid, reserved, or duplicate key in preset: ") + Key.ToString(); return false; }
				Used.Add(Key);
			}
		}
	}
    if (Theme->bApplySlotMotion && (!Theme->bHasSlotMotion || !T.Slot))
    { Error = TEXT("Slot motion needs a captured slot and a destination slot Blueprint. Disable Apply Slot Motion to keep its current settings."); return false; }
	TMap<FName, UCharacterSelectPreviewWidget*> Matched;
	if (Theme->bApplyPreviewAnimations || Theme->bApplyPresentation)
	{
		for (const auto& Pair : Theme->Previews)
		{
            if (!Theme->bApplyPreviewAnimations && !Pair.Value.bHasPresentation) continue;
			UCharacterSelectPreviewWidget* Target = nullptr;
			for (auto* Preview : T.Previews) if (Preview && Preview->GetFName() == Pair.Key) Target = Preview;
			if (!Target) { Error = TEXT("Destination is missing preview widget: ") + Pair.Key.ToString(); return false; }
			if (Theme->bApplyPreviewAnimations && !Pair.Value.ActiveStyle.IsNone() && !Pair.Value.Styles.Contains(Pair.Value.ActiveStyle))
			{ Error = TEXT("Unknown active preview style: ") + Pair.Value.ActiveStyle.ToString(); return false; }
			if (Theme->bApplyPreviewAnimations) for (const auto& Style : Pair.Value.Styles)
			{
				if (Style.Value.PlaybackSpeed <= 0) { Error = TEXT("Animation speeds must be positive."); return false; }
				for (FName Name : {Style.Value.OutAnimationName, Style.Value.InAnimationName})
				{
					if (Name.IsNone()) continue;
					const auto* Property = FindFProperty<FObjectPropertyBase>(Target->GetClass(), Name);
					if (!Property || !Property->PropertyClass->IsChildOf(UWidgetAnimation::StaticClass()))
					{ Error = TEXT("Destination preview lacks animation: ") + Name.ToString() + TEXT(". Author it in UMG or disable preview application."); return false; }
				}
			}
            if (Theme->bApplyPresentation && Pair.Value.bHasPresentation)
            {
                const auto& P = Pair.Value;
                TArray<FString> Issues = P.bUse3D ? P.Scene.GetIssues() : P.Artwork.GetIssues();
                if (!Issues.IsEmpty()) { Error = Pair.Key.ToString() + TEXT(": ") + FString::Join(Issues, TEXT("; ")); return false; }
                const auto* PreviewBP = Cast<UWidgetBlueprint>(Target->GetClass()->ClassGeneratedBy);
                const UWidgetTree* Tree = Target->WidgetTree ? Target->WidgetTree.Get() : PreviewBP ? PreviewBP->WidgetTree.Get() : nullptr;
                if ((P.bUse3D || P.Artwork.Mode == ECharacter2DPresentationMode::CustomWidget) &&
                    (!Tree || !Cast<UNamedSlot>(Tree->FindWidget(TEXT("CustomPresentationSlot")))))
                { Error = Pair.Key.ToString() + TEXT(": add CustomPresentationSlot using the 2D/3D presentation page before applying this theme."); return false; }
                if (!FMath::IsFinite(P.Sequence.GroupGap) || !FMath::IsFinite(P.Sequence.ConfirmDelay) ||
                    P.Sequence.GroupGap < 0 || P.Sequence.GroupGap > 5 || P.Sequence.ConfirmDelay < 0 || P.Sequence.ConfirmDelay > 5)
                { Error = TEXT("Sequence group gap and confirm delay must be between 0 and 5 seconds."); return false; }
            }
			Matched.Add(Pair.Key, Target);
		}
	}
	// All checks have passed. No asset has been changed on a failed preflight.
	RecordThemeChange(T.Screen);
	if (Theme->bApplyLayout)
	{
		RecordThemeChange(T.Grid); RecordThemeChange(T.Layout);
		T.Grid->SetGridLayout(Theme->GridLayout);
		const auto& L = Theme->GridLayout;
		T.Layout->Columns = L.Columns; T.Layout->Rows = L.Rows;
		T.Layout->SlotSize = L.SlotSize; T.Layout->SlotSpacing = L.SlotSpacing; T.Layout->GridPadding = L.Padding;
		T.Layout->NavigationMode = Theme->NavigationMode;
		T.Layout->HorizontalEdgeBehavior = Theme->HorizontalEdge;
		T.Layout->VerticalEdgeBehavior = Theme->VerticalEdge;
		T.Grid->MarkPackageDirty(); T.Layout->MarkPackageDirty();
	}
	if (Theme->bApplyPrompts)
	{
		T.Screen->PromptAppearance = Theme->PromptAppearance; T.Screen->PromptText = Theme->PromptText;
		T.Screen->JoinPromptMinOpacity = Theme->PromptMinOpacity; T.Screen->JoinPromptPulseSpeed = Theme->PromptPulseSpeed;
	}
	if (Theme->bApplyControls) T.Screen->ControlSettings = Theme->Controls;
	if (Theme->bApplyFeedback) { T.Screen->SoundSettings = Theme->SoundSettings; T.Screen->HintSettings = Theme->HintSettings; }
	if (Theme->bApplyPlayers)
	{
		T.Screen->PlayerConfigs = Theme->Players;
		T.Screen->bAutoCPUOpponent = Theme->bAutoCPUOpponent;
		T.Screen->bAllowLatePlayerJoin = Theme->bAllowLatePlayerJoin;
	}
	for (const auto& Pair : Matched)
	{
		RecordThemeChange(Pair.Value);
        const auto& P = Theme->Previews[Pair.Key];
        if (Theme->bApplyPreviewAnimations)
        {
            Pair.Value->AnimationStyles = P.Styles;
            Pair.Value->AnimationStyle = P.ActiveStyle;
        }
        if (Theme->bApplyPresentation && P.bHasPresentation)
        {
            Pair.Value->PreviewAnimationSettings = P.Motion;
            Pair.Value->DefaultPresentation2D = P.Artwork;
            Pair.Value->bDefault3DPreview = P.bUse3D;
            Pair.Value->DefaultPresentation3D = P.Scene;
            Pair.Value->SharedSequence = nullptr;
            Pair.Value->bOverrideSequence = true;
            Pair.Value->Sequence = P.Sequence;
        }
		Pair.Value->MarkPackageDirty();
	}
    if (Theme->bApplySlotMotion)
    {
        RecordThemeChange(T.Slot);
        T.Slot->SlotAnimationSettings = Theme->SlotMotion;
        T.Slot->MarkPackageDirty();
    }
	T.Screen->MarkPackageDirty();
	return true;
}
