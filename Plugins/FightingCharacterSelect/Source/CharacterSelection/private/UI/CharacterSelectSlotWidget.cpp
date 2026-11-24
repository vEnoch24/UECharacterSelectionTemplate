#include "UI/CharacterSelectSlotWidget.h"
#include "Data/CharacterSelectData.h"
#include "Components/Image.h"
#include "Components/Overlay.h"
#include "Animation/WidgetAnimationState.h"
#include "Animation/WidgetAnimation.h"
#include "Blueprint/WidgetTree.h"
#include "UObject/UnrealType.h"
#include "MovieScene.h"
#include "MovieSceneSection.h"
#include "EntitySystem/MovieSceneSharedPlaybackState.h"
#include "EntitySystem/MovieSceneEntitySystemRunner.h"

void UCharacterSelectSlotWidget::SetStudioAnimationPreview(bool bEnabled)
{
    bStudioPreview = true;
    bStudioAnimationPreview = bEnabled;
    UpdateConfiguredAnimation(false);
}

void UCharacterSelectSlotWidget::ApplyStudioAuthoredPose(UWidgetAnimation* Animation)
{
    if (!Animation) return;
    // Evaluate a stopped pose, without starting playback or invoking animation
    // started/finished Blueprint events. Retire the previous clip's entities so
    // a focus pose cannot continue overriding the unfocus pose.
    for (UWidgetAnimation* Clip : {Player1FocusAnimation.Get(), Player1UnfocusAnimation.Get(),
        Player2FocusAnimation.Get(), Player2UnfocusAnimation.Get(), SelectedAnimation.Get(), DeselectedAnimation.Get()})
        if (const auto State = GetAnimationState(Clip)) State->RemoveEvaluationData();
    if (const auto State = GetOrAddAnimationState(Animation))
    {
        // A clip can outlive its transform/color sections. Evaluate their final
        // valid ticks in order, otherwise a direct jump to the clip end misses
        // the finished track and leaves the previous focus pose behind.
        TArray<float> PoseTimes {Animation->GetStartTime(), Animation->GetEndTime()};
        if (const auto* Scene = Animation->GetMovieScene())
            for (const auto* Section : Scene->GetAllSections())
                if (Section && Section->HasEndFrame())
                {
                    const float End = Scene->GetTickResolution().AsSeconds(Section->GetExclusiveEndFrame() - 1);
                    if (End >= Animation->GetStartTime() && End < Animation->GetEndTime()) PoseTimes.AddUnique(End);
                }
        PoseTimes.Sort();
        for (float Time : PoseTimes)
        {
            State->SetCurrentTime(Time);
            State->Pause();
            FlushAnimations();
        }
        // Match the section's Keep/Restore State completion policy, just as
        // runtime does after its final frame, without broadcasting gameplay events.
        if (const auto Playback = State->GetSharedPlaybackState())
            if (const auto Runner = Playback->GetRunner())
            {
                Runner->QueueFinalUpdate(Playback->GetRootInstanceHandle());
                Runner->Flush();
            }
    }
}

void UCharacterSelectSlotWidget::SyncStudioVisualState()
{
    if (!bStudioPreview || !IsDesignTime()) return;
    if (SlotAnimationSettings.Mode == ECharacterSlotAnimationMode::Authored && bStudioPoseDirty)
    {
        bStudioPoseDirty = false;
        ApplyStudioAuthoredPose(SlotTransitionAnimation);
        // Runtime restores the focused pose when deselection finishes.
        if (SlotTransitionAnimation == DeselectedAnimation && bRestoreFocusAfterDeselection)
        {
            RestoreFocusAfterDeselection();
            ApplyStudioAuthoredPose(SlotTransitionAnimation);
            bStudioPoseDirty = false;
        }
    }
    RefreshCharacterVisual();
    RefreshVisualState();
    // GetVisibility() reads Slate once built, including the designer override.
    // Read the stored value directly so untouched/hidden children also match Play.
    if (WidgetTree) WidgetTree->ForEachWidget([](UWidget* Widget)
    {
        static const FProperty* VisibilityProperty = FindFProperty<FProperty>(UWidget::StaticClass(), TEXT("Visibility"));
        if (VisibilityProperty)
            if (const auto Slate = Widget->GetCachedWidget())
                Slate->SetVisibility(UWidget::ConvertSerializedVisibilityToRuntime(*VisibilityProperty->ContainerPtrToValuePtr<ESlateVisibility>(Widget)));
    });
}

void UCharacterSelectSlotWidget::NativeOnInitialized()
{
    Super::NativeOnInitialized();
    OnAnimationFinishedPlaying().AddUObject(this, &UCharacterSelectSlotWidget::HandleSlotAnimationFinished);
}

TSharedRef<SWidget> UCharacterSelectSlotWidget::RebuildWidget()
{
    // Shift before Slate builds its children. Keep the original panel slots, their
    // layout settings and animation bindings; never reparent or edit the asset.
    if ((!IsDesignTime() || bStudioPreview) && BothPlayersFocusBorder)
    {
        if (auto* Overlay = Cast<UOverlay>(BothPlayersFocusBorder->GetParent()))
        {
            int32 Index = Overlay->GetChildIndex(BothPlayersFocusBorder);
            for (UWidget* Selected : {Player1SelectedBorder.Get(), Player2SelectedBorder.Get()})
                if (Selected && Selected->GetParent() == Overlay)
                    Index = FMath::Max(Index, Overlay->GetChildIndex(Selected));
            Overlay->ShiftChild(Index, BothPlayersFocusBorder);
        }
    }
    return Super::RebuildWidget();
}

void UCharacterSelectSlotWidget::NativeConstruct()
{
    Super::NativeConstruct();
    RefreshCharacterVisual();
    RefreshVisualState();
    UpdateConfiguredAnimation();
}

void UCharacterSelectSlotWidget::SetCharacter(UCharacterSelectData* InCharacter)
{
    Character = InCharacter;
    RefreshCharacterVisual();
    RefreshVisualState();
}
UCharacterSelectData* UCharacterSelectSlotWidget::GetCharacter() const { return Character; }
void UCharacterSelectSlotWidget::SetRosterIndex(int32 InRosterIndex) { RosterIndex = InRosterIndex; }
int32 UCharacterSelectSlotWidget::GetRosterIndex() const { return RosterIndex; }
void UCharacterSelectSlotWidget::SetLocked(bool bInLocked)
{
    if (bLocked == bInLocked) return;
    bLocked = bInLocked;
    RefreshVisualState();
    UpdateConfiguredAnimation();
}
bool UCharacterSelectSlotWidget::IsLocked() const { return bLocked; }

void UCharacterSelectSlotWidget::SetFocusedForPlayer(int32 PlayerIndex, bool bInFocused)
{
    if (PlayerIndex < 0 || PlayerIndex > 1 || FocusedPlayers.Contains(PlayerIndex) == bInFocused) return;
    const bool bWasOccupied = IsOccupiedByAnyPlayer();
    if (bInFocused) FocusedPlayers.Add(PlayerIndex);
    else FocusedPlayers.Remove(PlayerIndex);

    // These clips animate the same slot transform. A second cursor must not
    // interrupt selection, and one departing cursor must not shrink an occupied slot.
    if (SlotAnimationSettings.Mode == ECharacterSlotAnimationMode::Authored && SelectedPlayers.IsEmpty())
    {
        if (!bWasOccupied && IsOccupiedByAnyPlayer()) PlayFocusAnimationForPlayer(PlayerIndex);
        else if (bWasOccupied && !IsOccupiedByAnyPlayer()) PlayUnfocusAnimationForPlayer(PlayerIndex);
    }
    RefreshVisualState();
    UpdateConfiguredAnimation();
}
bool UCharacterSelectSlotWidget::IsFocusedForPlayer(int32 PlayerIndex) const { return FocusedPlayers.Contains(PlayerIndex); }
bool UCharacterSelectSlotWidget::IsFocusedByAnyPlayer() const { return !FocusedPlayers.IsEmpty(); }
bool UCharacterSelectSlotWidget::IsFocusedByBothPlayers() const { return FocusedPlayers.Contains(0) && FocusedPlayers.Contains(1); }
bool UCharacterSelectSlotWidget::IsOccupiedByAnyPlayer() const { return !FocusedPlayers.IsEmpty() || !SelectedPlayers.IsEmpty(); }
bool UCharacterSelectSlotWidget::IsOccupiedByBothPlayers() const
{
    return (FocusedPlayers.Contains(0) || SelectedPlayers.Contains(0)) &&
        (FocusedPlayers.Contains(1) || SelectedPlayers.Contains(1));
}

void UCharacterSelectSlotWidget::SetSelectedForPlayer(int32 PlayerIndex, bool bInSelected)
{
    if (PlayerIndex < 0 || PlayerIndex > 1 || SelectedPlayers.Contains(PlayerIndex) == bInSelected) return;
    const bool bWasSelected = !SelectedPlayers.IsEmpty();
    if (bInSelected) SelectedPlayers.Add(PlayerIndex);
    else SelectedPlayers.Remove(PlayerIndex);

    // Selected/Deselected are shared slot animations, not per-player tracks.
    if (SlotAnimationSettings.Mode == ECharacterSlotAnimationMode::Authored)
    {
        if (!bWasSelected && !SelectedPlayers.IsEmpty()) PlaySelectedAnimationForPlayer(PlayerIndex);
        else if (bWasSelected && SelectedPlayers.IsEmpty()) PlayDeselectedAnimationForPlayer(PlayerIndex);
    }
    RefreshVisualState();
    UpdateConfiguredAnimation(bWasSelected && SelectedPlayers.IsEmpty());
}
bool UCharacterSelectSlotWidget::IsSelectedForPlayer(int32 PlayerIndex) const { return SelectedPlayers.Contains(PlayerIndex); }

void UCharacterSelectSlotWidget::RefreshCharacterVisual()
{
    if (!Portrait) return;
    Portrait->SetBrushFromTexture(Character ? Character->Portrait : nullptr);
    Portrait->SetVisibility(Character && Character->Portrait ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
}

void UCharacterSelectSlotWidget::RefreshVisualState()
{
    auto Show = [](UWidget* Widget, bool bVisible)
    {
        if (Widget) Widget->SetVisibility(bVisible ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Collapsed);
    };
    Show(Player1FocusBorder, FocusedPlayers.Contains(0));
    Show(Player2FocusBorder, FocusedPlayers.Contains(1));
    Show(BothPlayersFocusBorder, IsOccupiedByBothPlayers());
    Show(Player1SelectedBorder, SelectedPlayers.Contains(0));
    Show(Player2SelectedBorder, SelectedPlayers.Contains(1));
    Show(LockOverlay, bLocked);
}

void UCharacterSelectSlotWidget::PlaySlotTransition(UWidgetAnimation* Animation)
{
    // Studio displays settled states; designer widgets cannot finish gameplay
    // transitions. Runtime animation playback remains unchanged.
    if (IsDesignTime())
    {
        SlotTransitionAnimation = Animation;
        bStudioPoseDirty = true;
        return;
    }
    TGuardValue<bool> Changing(bChangingSlotAnimation, true);
    SlotTransitionAnimation = Animation;
    // Stop only the known competing slot-transform clips. Custom animations
    // (including the authored combined-focus glow) continue independently.
    for (UWidgetAnimation* Clip : {Player1FocusAnimation.Get(), Player1UnfocusAnimation.Get(),
        Player2FocusAnimation.Get(), Player2UnfocusAnimation.Get(), SelectedAnimation.Get(), DeselectedAnimation.Get()})
        if (Clip) StopAnimation(Clip);
    if (Animation) PlayAnimation(Animation);
}
void UCharacterSelectSlotWidget::PlayFocusAnimationForPlayer(int32 PlayerIndex)
{
    bRestoreFocusAfterDeselection = false;
    PlaySlotTransition(PlayerIndex == 0 ? Player1FocusAnimation : Player2FocusAnimation);
}
void UCharacterSelectSlotWidget::PlayUnfocusAnimationForPlayer(int32 PlayerIndex)
{
    bRestoreFocusAfterDeselection = false;
    PlaySlotTransition(PlayerIndex == 0 ? Player1UnfocusAnimation : Player2UnfocusAnimation);
}
void UCharacterSelectSlotWidget::PlaySelectedAnimationForPlayer(int32 PlayerIndex)
{
    bRestoreFocusAfterDeselection = false;
    PlaySlotTransition(SelectedAnimation);
}
void UCharacterSelectSlotWidget::PlayDeselectedAnimationForPlayer(int32 PlayerIndex)
{
    bRestoreFocusAfterDeselection = IsFocusedByAnyPlayer();
    PlaySlotTransition(DeselectedAnimation);
    if (!DeselectedAnimation) RestoreFocusAfterDeselection();
}
void UCharacterSelectSlotWidget::RestoreFocusAfterDeselection()
{
    if (!bRestoreFocusAfterDeselection) return;
    bRestoreFocusAfterDeselection = false;
    if (SelectedPlayers.IsEmpty() && IsFocusedByAnyPlayer())
        PlayFocusAnimationForPlayer(FocusedPlayers.Contains(0) ? 0 : 1);
}
void UCharacterSelectSlotWidget::HandleSlotAnimationFinished(FWidgetAnimationState& State)
{
    if (SlotAnimationSettings.Mode == ECharacterSlotAnimationMode::Authored && !bChangingSlotAnimation && State.GetAnimation() == DeselectedAnimation &&
        SlotTransitionAnimation == DeselectedAnimation && !IsAnimationPlaying(DeselectedAnimation))
        RestoreFocusAfterDeselection();
    RefreshVisualState();
}


void UCharacterSelectSlotWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
    if (bStudioAnimationPreview) return;
    Super::NativeTick(MyGeometry, InDeltaTime);
    AdvanceConfiguredAnimation(InDeltaTime);
}

void UCharacterSelectSlotWidget::NativeDestruct()
{
    if (bHasConfiguredBaseline)
    {
        SetRenderTransform(ConfiguredBaselineTransform);
        SetRenderOpacity(ConfiguredBaselineOpacity);
        SetColorAndOpacity(ConfiguredBaselineTint);
    }
    bHasConfiguredBaseline = false;
    bConfiguredAnimating = false;
    ConfiguredState = INDEX_NONE;
    Super::NativeDestruct();
}

int32 UCharacterSelectSlotWidget::ResolveConfiguredState() const
{
    if (bLocked) return 5;
    if (!SelectedPlayers.IsEmpty()) return IsOccupiedByBothPlayers() ? 4 : 3;
    if (IsOccupiedByBothPlayers()) return 2;
    return IsFocusedByAnyPlayer() ? 1 : 0;
}

void UCharacterSelectSlotWidget::UpdateConfiguredAnimation(bool bCancelled)
{
    if ((IsDesignTime() && !bStudioPreview) || SlotAnimationSettings.Mode != ECharacterSlotAnimationMode::Configured) return;
    if (!bHasConfiguredBaseline)
    {
        ConfiguredBaselineTransform = GetRenderTransform();
        ConfiguredBaselineOpacity = GetRenderOpacity();
        ConfiguredBaselineTint = GetColorAndOpacity();
        ConfiguredCurrent = FCharacterSlotAnimationState();
        bHasConfiguredBaseline = true;
    }
    const int32 NextState = ResolveConfiguredState();
    // Repeated refreshes and one player's cancellation must not restart the
    // shared selection transition while the other player remains selected.
    if (NextState == ConfiguredState) return;
    ConfiguredState = NextState;
    const FCharacterSlotAnimationState* States[] = {&SlotAnimationSettings.Idle, &SlotAnimationSettings.Focused,
        &SlotAnimationSettings.SharedFocus, &SlotAnimationSettings.Selected, &SlotAnimationSettings.SharedSelection, &SlotAnimationSettings.Locked};
    ConfiguredFrom = ConfiguredCurrent;
    ConfiguredTarget = *States[NextState];
    if (bCancelled) ConfiguredTarget.Duration = SlotAnimationSettings.CancelDuration;
    // Metadata constrains editor input; runtime sanitization also protects C++ data.
    auto FiniteClamp = [](float Value, float Fallback, float Min, float Max) { return FMath::Clamp(FMath::IsFinite(Value) ? Value : Fallback, Min, Max); };
    ConfiguredTarget.Duration = FiniteClamp(ConfiguredTarget.Duration, 0.f, 0.f, 5.f);
    ConfiguredTarget.Scale = FiniteClamp(ConfiguredTarget.Scale, 1.f, 0.01f, 4.f);
    ConfiguredTarget.Opacity = FiniteClamp(ConfiguredTarget.Opacity, 1.f, 0.f, 1.f);
    if (ConfiguredTarget.Translation.ContainsNaN()) ConfiguredTarget.Translation = FVector2D::ZeroVector;
    if (!FMath::IsFinite(ConfiguredTarget.Tint.R) || !FMath::IsFinite(ConfiguredTarget.Tint.G) ||
        !FMath::IsFinite(ConfiguredTarget.Tint.B) || !FMath::IsFinite(ConfiguredTarget.Tint.A))
        ConfiguredTarget.Tint = FLinearColor::White;
    ConfiguredElapsed = 0.f;
    bConfiguredAnimating = true;
    AdvanceConfiguredAnimation(bStudioPreview && !bStudioAnimationPreview ? 5.f : 0.f);
}

void UCharacterSelectSlotWidget::AdvanceConfiguredAnimation(float DeltaTime)
{
    if (!bConfiguredAnimating || SlotAnimationSettings.Mode != ECharacterSlotAnimationMode::Configured) return;
    ConfiguredElapsed += FMath::IsFinite(DeltaTime) ? FMath::Max(0.f, DeltaTime) : 0.f;
    float Alpha = ConfiguredTarget.Duration > 0.f ? FMath::Clamp(ConfiguredElapsed / ConfiguredTarget.Duration, 0.f, 1.f) : 1.f;
    bConfiguredAnimating = Alpha < 1.f;
    if (ConfiguredTarget.Easing == ECharacterSlotEasing::EaseOut) Alpha = 1.f - FMath::Pow(1.f - Alpha, 3.f);
    else if (ConfiguredTarget.Easing == ECharacterSlotEasing::SmoothStep) Alpha = Alpha * Alpha * (3.f - 2.f * Alpha);
    ConfiguredCurrent.Scale = FMath::Lerp(ConfiguredFrom.Scale, ConfiguredTarget.Scale, Alpha);
    ConfiguredCurrent.Translation = FMath::Lerp(ConfiguredFrom.Translation, ConfiguredTarget.Translation, Alpha);
    ConfiguredCurrent.Opacity = FMath::Lerp(ConfiguredFrom.Opacity, ConfiguredTarget.Opacity, Alpha);
    ConfiguredCurrent.Tint = FMath::Lerp(ConfiguredFrom.Tint, ConfiguredTarget.Tint, Alpha);
    ApplyConfiguredAppearance();
}

void UCharacterSelectSlotWidget::ApplyConfiguredAppearance()
{
    FWidgetTransform Transform = ConfiguredBaselineTransform;
    Transform.Scale *= ConfiguredCurrent.Scale;
    Transform.Translation += ConfiguredCurrent.Translation;
    SetRenderTransform(Transform);
    SetRenderOpacity(ConfiguredBaselineOpacity * ConfiguredCurrent.Opacity);
    SetColorAndOpacity(ConfiguredBaselineTint * ConfiguredCurrent.Tint);
}
