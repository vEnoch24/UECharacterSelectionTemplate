#include "UI/CharacterSelectPreviewWidget.h"
#include "Data/CharacterSelectData.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "Animation/WidgetAnimation.h"
#include "UObject/UnrealType.h"
#include "Components/NamedSlot.h"
#include "UI/CharacterSelectPresentationWidget.h"
#include "UI/CharacterSelect3DPreviewWidget.h"

void UCharacterSelectPreviewWidget::NativeConstruct()
{
	Super::NativeConstruct();
	RefreshPreview();
}

void UCharacterSelectPreviewWidget::NativeDestruct()
{
    if (ConfiguredPhase == EConfiguredPhase::Exit) Character = PendingCharacter;
    RemovePresentationTransform();
    RestoreConfiguredElements();
    if (auto* Scene = Cast<UCharacterSelect3DPreviewWidget>(CustomPresentationWidget)) Scene->ClearScene();
    if (CustomPresentationSlot) CustomPresentationSlot->ClearChildren();
    CustomPresentationWidget = nullptr;
    Presented2DCharacter.Reset();
    bUsing3DPresentation = false;
    bPresentationSelected = false;
    ResetSequenceConfirmation();
    PresentationSeconds = 0.;
    PresentationFrameIndex = INDEX_NONE;
    ConfiguredPhase = EConfiguredPhase::Idle;
	UnbindExitAnimation();
	if (ActiveOutAnimation) StopAnimation(ActiveOutAnimation);
	if (ActiveInAnimation) StopAnimation(ActiveInAnimation);
	ActiveOutAnimation = nullptr;
	ActiveInAnimation = nullptr;
	PendingCharacter = nullptr;
	bIsChangingCharacter = false;
	bIsPlayingOutAnimation = false;
	Super::NativeDestruct();
}

bool UCharacterSelectPreviewWidget::SetAnimationStyle(FName StyleName)
{
	if (!StyleName.IsNone() && !AnimationStyles.Contains(StyleName)) return false;
	AnimationStyle = StyleName;
	return true;
}

UWidgetAnimation* UCharacterSelectPreviewWidget::FindPreviewAnimation(FName AnimationName) const
{
	if (AnimationName.IsNone()) return nullptr;
	// UMG binds animation properties using the names shown in the Animations panel.
	const FObjectPropertyBase* Property = FindFProperty<FObjectPropertyBase>(GetClass(), AnimationName);
	UWidgetAnimation* Animation = Property ? Cast<UWidgetAnimation>(Property->GetObjectPropertyValue_InContainer(this)) : nullptr;
	if (!Animation)
		UE_LOG(LogTemp, Warning, TEXT("Preview %s: animation '%s' was not found; that phase is skipped."), *GetName(), *AnimationName.ToString());
	return Animation;
}

void UCharacterSelectPreviewWidget::ResolveAnimationStyle()
{
	UnbindExitAnimation();
	UWidgetAnimation* PreviousInAnimation = ActiveInAnimation;
	ActiveOutAnimation = CharacterChangeOutAnimation;
	ActiveInAnimation = CharacterChangeInAnimation;
	ActivePlaybackSpeed = 1.0f;
	if (!AnimationStyle.IsNone())
	{
		if (const FCharacterPreviewAnimationStyle* Style = AnimationStyles.Find(AnimationStyle))
		{
			ActiveOutAnimation = FindPreviewAnimation(Style->OutAnimationName);
			ActiveInAnimation = FindPreviewAnimation(Style->InAnimationName);
			ActivePlaybackSpeed = FMath::Max(0.01f, Style->PlaybackSpeed);
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("Preview %s: style '%s' was not found; using original animations."), *GetName(), *AnimationStyle.ToString());
		}
	}
	// A different style must not leave the previous style's enter tracks running.
	if (PreviousInAnimation && PreviousInAnimation != ActiveInAnimation) StopAnimation(PreviousInAnimation);
}

void UCharacterSelectPreviewWidget::UnbindExitAnimation()
{
	if (!ActiveOutAnimation) return;
	FWidgetAnimationDynamicEvent Delegate;
	Delegate.BindDynamic(this, &UCharacterSelectPreviewWidget::HandleCharacterChangeAnimationFinished);
	UnbindFromAnimationFinished(ActiveOutAnimation, Delegate);
}

void UCharacterSelectPreviewWidget::SetCharacter(UCharacterSelectData* InCharacter)
{
    // Designer instances have no animation tick. Show the settled content without
    // entering a transition; gameplay instances retain their authored animations.
    if (IsDesignTime() && !bStudioAnimationPreview)
    {
        Character = InCharacter;
        PendingCharacter = nullptr;
        RefreshPreview();
        return;
    }
    if (bStudioAnimationPreview && !ResolveSequence(InCharacter).bEnabled && !ResolveSequence(Character).bEnabled && PreviewAnimationSettings.Mode == ECharacterPreviewAnimationMode::Authored)
    {
        Character = InCharacter; PendingCharacter = nullptr; RefreshPreview(); return;
    }
    const auto* Requested = (bIsChangingCharacter || ConfiguredPhase == EConfiguredPhase::Exit) ? PendingCharacter.Get() : Character.Get();
    if (Requested != InCharacter)
    {
        bPresentationSelected = false;
        ResetSequenceConfirmation();
    }
    if (ConfiguredPhase != EConfiguredPhase::Idle || PreviewAnimationSettings.Mode == ECharacterPreviewAnimationMode::Configured || ResolveSequence(Character).bEnabled || ResolveSequence(InCharacter).bEnabled)
    {
        SetConfiguredCharacter(InCharacter);
        return;
    }
    RestoreConfiguredElements();
	// Keep the latest request, including nullptr or a return to the outgoing character.
	if (bIsChangingCharacter)
	{
		PendingCharacter = InCharacter;
		return;
	}
	if (Character == InCharacter) return;
	PendingCharacter = InCharacter;
	PlayCharacterChangeAnimation();
}

UCharacterSelectData* UCharacterSelectPreviewWidget::GetCharacter() const
{
	return Character;
}

void UCharacterSelectPreviewWidget::ClearPreview()
{
	SetCharacter(nullptr);
}

void UCharacterSelectPreviewWidget::RefreshPreview()
{
	UpdatePreviewContent(Character);
}

void UCharacterSelectPreviewWidget::UpdatePreviewContent(
	UCharacterSelectData* InCharacter)
{
    UpdatePresentation2D(InCharacter);
    if (!InCharacter)
    {
        if (CharacterName) CharacterName->SetText(FText::GetEmpty());
        if (Subtitle) Subtitle->SetText(FText::GetEmpty());
        return;
    }

	// --------------------------------------------------
	// Character Name
	// --------------------------------------------------

	if (CharacterName)
	{
		CharacterName->SetText(
			InCharacter->DisplayName
		);
	}

	// --------------------------------------------------
	// Subtitle
	// --------------------------------------------------

	if (Subtitle)
	{
		Subtitle->SetText(
			InCharacter->Subtitle
		);
	}
}


void UCharacterSelectPreviewWidget::PlayCharacterChangeAnimation()
{
	if (bIsChangingCharacter) return;
	ResolveAnimationStyle();
	bIsChangingCharacter = true;
	if (ActiveOutAnimation)
	{
		bIsPlayingOutAnimation = true;
		StopAnimation(ActiveOutAnimation);
		FWidgetAnimationDynamicEvent Delegate;
		Delegate.BindDynamic(this, &UCharacterSelectPreviewWidget::HandleCharacterChangeAnimationFinished);
		BindToAnimationFinished(ActiveOutAnimation, Delegate);
		PlayAnimation(ActiveOutAnimation, 0.0f, 1, EUMGSequencePlayMode::Forward, ActivePlaybackSpeed);
		return;
	}
	FinishCharacterChange();
}

void UCharacterSelectPreviewWidget::HandleCharacterChangeAnimationFinished()
{
	if (!bIsChangingCharacter || !bIsPlayingOutAnimation) return;
	bIsPlayingOutAnimation = false;
	UnbindExitAnimation();
	FinishCharacterChange();
}

void UCharacterSelectPreviewWidget::FinishCharacterChange()
{
	Character = PendingCharacter;
	PendingCharacter = nullptr;
	RefreshPreview();
	if (ActiveInAnimation)
	{
		StopAnimation(ActiveInAnimation);
		PlayAnimation(ActiveInAnimation, 0.0f, 1, EUMGSequencePlayMode::Forward, ActivePlaybackSpeed);
	}
	bIsChangingCharacter = false;
}


namespace
{
    FCharacterPreviewTransition NeutralPreviewPose()
    {
        FCharacterPreviewTransition Pose;
        Pose.Opacity = 1.f;
        Pose.Duration = Pose.Delay = 0.f;
        return Pose;
    }
    FCharacterPreviewTransition SanitizePreviewTransition(FCharacterPreviewTransition Pose)
    {
        auto Clamp = [](float Value, float Fallback, float Min, float Max)
        { return FMath::Clamp(FMath::IsFinite(Value) ? Value : Fallback, Min, Max); };
        Pose.Duration = Clamp(Pose.Duration, 0.f, 0.f, 5.f);
        Pose.Delay = Clamp(Pose.Delay, 0.f, 0.f, 20.f);
        Pose.Scale = Clamp(Pose.Scale, 1.f, 0.01f, 4.f);
        Pose.Opacity = Clamp(Pose.Opacity, 1.f, 0.f, 1.f);
        if (Pose.Translation.ContainsNaN()) Pose.Translation = FVector2D::ZeroVector;
        return Pose;
    }
}

void UCharacterSelectPreviewWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
    RemovePresentationTransform();
    if (bStudioAnimationPreview) return;
    Super::NativeTick(MyGeometry, InDeltaTime);
    const bool bWaitingForEntrance = ConfiguredPhase != EConfiguredPhase::Idle && ResolveSequence(Character).bWaitForEntrance;
    AdvanceConfiguredAnimation(InDeltaTime);
    if (auto* Scene = Cast<UCharacterSelect3DPreviewWidget>(CustomPresentationWidget))
    {
        if (!Scene->GetPreviewActor() && !Scene->GetPreviewError().IsEmpty())
        {
            bSceneFailedForCharacter = true;
            UpdatePresentation2D(Character);
        }
    }
    AdvanceSequenceConfirmation(bWaitingForEntrance ? 0.f : InDeltaTime);
    AdvancePresentation2D(InDeltaTime);
    ApplyPresentationTransform();
}

void UCharacterSelectPreviewWidget::EnsureConfiguredElements()
{
    if (!ConfiguredElements.IsEmpty()) return;
    RemovePresentationTransform();
    for (UWidget* Widget : {static_cast<UWidget*>(LargePortrait.Get()), static_cast<UWidget*>(CharacterName.Get()), static_cast<UWidget*>(Subtitle.Get())})
    {
        auto& Element = ConfiguredElements.AddDefaulted_GetRef();
        Element.Widget = Widget;
        Element.Current = NeutralPreviewPose();
        if (Widget)
        {
            Element.Baseline = Widget->GetRenderTransform();
            Element.BaselineOpacity = Widget->GetRenderOpacity();
        }
    }
    ApplyPresentationTransform();
}

void UCharacterSelectPreviewWidget::SetConfiguredCharacter(UCharacterSelectData* InCharacter)
{
    // Switching from authored playback must release its completion callback first.
    if (bIsChangingCharacter)
    {
        UnbindExitAnimation();
        if (ActiveOutAnimation) StopAnimation(ActiveOutAnimation);
        bIsChangingCharacter = bIsPlayingOutAnimation = false;
    }
    if (ActiveInAnimation) StopAnimation(ActiveInAnimation);
    EnsureConfiguredElements();
    // Keep one latest request. Do not restart an exit for every navigation repeat.
    if (ConfiguredPhase == EConfiguredPhase::Exit) { PendingCharacter = InCharacter; return; }
    if (Character == InCharacter) return;
    PendingCharacter = InCharacter;
    if (Character) BeginConfiguredPhase(EConfiguredPhase::Exit);
    else
    {
        Character = PendingCharacter;
        PendingCharacter = nullptr;
        RefreshPreview();
        BeginConfiguredPhase(EConfiguredPhase::Entrance);
    }
    AdvanceConfiguredAnimation(0.f);
}

void UCharacterSelectPreviewWidget::BeginConfiguredPhase(EConfiguredPhase Phase)
{
    ConfiguredPhase = Phase;
    ConfiguredElapsed = ConfiguredPhaseDuration = 0.f;
    const auto EffectiveSequence = ResolveSequence(Character);
    const auto Animation = EffectiveSequence.bEnabled ? EffectiveSequence.GetTransitions(Phase == EConfiguredPhase::Exit) : PreviewAnimationSettings;
    const FCharacterPreviewElementAnimation* Settings[] = {&Animation.Portrait, &Animation.Name, &Animation.Subtitle};
    EmitSequenceEvent(Phase == EConfiguredPhase::Exit ? ECharacterPresentationEvent::Exit : ECharacterPresentationEvent::Entrance);
    for (int32 Index = 0; Index < ConfiguredElements.Num(); ++Index)
    {
        auto& Element = ConfiguredElements[Index];
        const auto& Setting = *Settings[Index];
        if (!Setting.bEnabled || !Element.Widget.IsValid())
        {
            Element.From = Element.To = Element.Current = Element.Timing = NeutralPreviewPose();
            continue;
        }
        Element.Timing = SanitizePreviewTransition(Phase == EConfiguredPhase::Exit ? Setting.Exit : Setting.Entrance);
        Element.From = Phase == EConfiguredPhase::Exit ? Element.Current : Element.Timing;
        Element.To = Phase == EConfiguredPhase::Exit ? Element.Timing : NeutralPreviewPose();
        Element.Current = Element.From;
        ConfiguredPhaseDuration = FMath::Max(ConfiguredPhaseDuration, Element.Timing.Delay + Element.Timing.Duration);
    }
    ApplyConfiguredElements();
}

void UCharacterSelectPreviewWidget::AdvanceConfiguredAnimation(float DeltaTime)
{
    if (ConfiguredPhase == EConfiguredPhase::Idle) return;
    float Remaining = FMath::IsFinite(DeltaTime) ? FMath::Max(0.f, DeltaTime) : 0.f;
    // At most exit -> entrance -> idle. Carry excess frame time into the next phase.
    for (int32 Pass = 0; Pass < 2 && ConfiguredPhase != EConfiguredPhase::Idle; ++Pass)
    {
        const float Used = FMath::Min(Remaining, FMath::Max(0.f, ConfiguredPhaseDuration - ConfiguredElapsed));
        ConfiguredElapsed += Used;
        Remaining -= Used;
        for (auto& Element : ConfiguredElements)
        {
            const float LocalTime = ConfiguredElapsed - Element.Timing.Delay;
            float Alpha = LocalTime < 0.f ? 0.f : Element.Timing.Duration > 0.f ? FMath::Clamp(LocalTime / Element.Timing.Duration, 0.f, 1.f) : 1.f;
            if (Element.Timing.Easing == ECharacterSlotEasing::EaseOut) Alpha = 1.f - FMath::Pow(1.f - Alpha, 3.f);
            else if (Element.Timing.Easing == ECharacterSlotEasing::SmoothStep) Alpha = Alpha * Alpha * (3.f - 2.f * Alpha);
            Element.Current.Scale = FMath::Lerp(Element.From.Scale, Element.To.Scale, Alpha);
            Element.Current.Translation = FMath::Lerp(Element.From.Translation, Element.To.Translation, Alpha);
            Element.Current.Opacity = FMath::Lerp(Element.From.Opacity, Element.To.Opacity, Alpha);
        }
        ApplyConfiguredElements();
        if (ConfiguredElapsed < ConfiguredPhaseDuration) return;
        if (ConfiguredPhase == EConfiguredPhase::Entrance) { ConfiguredPhase = EConfiguredPhase::Idle; EmitSequenceEvent(ECharacterPresentationEvent::Ready); return; }
        // Swap all fields together, once the outgoing content has finished.
        Character = PendingCharacter;
        PendingCharacter = nullptr;
        RefreshPreview();
        if (!Character)
        {
            for (auto& Element : ConfiguredElements) Element.Current = NeutralPreviewPose();
            ApplyConfiguredElements();
            ConfiguredPhase = EConfiguredPhase::Idle;
            return;
        }
        BeginConfiguredPhase(EConfiguredPhase::Entrance);
    }
}

void UCharacterSelectPreviewWidget::ApplyConfiguredElements()
{
    RemovePresentationTransform();
    for (const auto& Element : ConfiguredElements) if (auto* Widget = Element.Widget.Get())
    {
        FWidgetTransform Transform = Element.Baseline;
        Transform.Scale *= Element.Current.Scale;
        Transform.Translation += Element.Current.Translation;
        Widget->SetRenderTransform(Transform);
        Widget->SetRenderOpacity(Element.BaselineOpacity * Element.Current.Opacity);
    }
    ApplyPresentationTransform();
}

void UCharacterSelectPreviewWidget::RestoreConfiguredElements()
{
    for (const auto& Element : ConfiguredElements) if (auto* Widget = Element.Widget.Get())
    {
        Widget->SetRenderTransform(Element.Baseline);
        Widget->SetRenderOpacity(Element.BaselineOpacity);
    }
    ConfiguredElements.Reset();
}

void UCharacterSelectPreviewWidget::SetPresentationPlayerIndex(int32 Index)
{
    Index = FMath::Clamp(Index, 0, 1);
    if (PresentationPlayerIndex == Index) return;
    RemovePresentationTransform();
    PresentationPlayerIndex = Index;
    if (CustomPresentationWidget) CustomPresentationWidget->SetPresentation(Presented2DCharacter.Get(), Index);
    if (auto* Scene = Cast<UCharacterSelect3DPreviewWidget>(CustomPresentationWidget)) Scene->SetPreviewPlayerIndex(Index);
    ApplyPresentationTransform();
}

void UCharacterSelectPreviewWidget::UpdatePresentation2D(UCharacterSelectData* InCharacter)
{
    RemovePresentationTransform();
    const bool bChanged = Presented2DCharacter.Get() != InCharacter;
    Presented2DCharacter = InCharacter;
    if (bChanged) bSceneFailedForCharacter = false;
    if (bChanged) { PresentationSeconds = 0.; PresentationFrameIndex = INDEX_NONE; }
    ActivePresentation2D = InCharacter && InCharacter->bOverridePresentation2D ? InCharacter->Presentation2D : DefaultPresentation2D;
    bUsing3DPresentation = InCharacter && (InCharacter->PreviewMode == ECharacterPreviewMode::Actor3D || (InCharacter->PreviewMode == ECharacterPreviewMode::Inherit && bDefault3DPreview));
    const FCharacterSelect3DPresentation& SceneSettings = InCharacter && InCharacter->bOverridePresentation3D ? InCharacter->Presentation3D : DefaultPresentation3D;
    UClass* DesiredClass = bUsing3DPresentation ? UCharacterSelect3DPreviewWidget::StaticClass() : ActivePresentation2D.WidgetClass.Get();
    const bool bCustom = InCharacter && CustomPresentationSlot && DesiredClass && !DesiredClass->HasAnyClassFlags(CLASS_Abstract) &&
        (bUsing3DPresentation ? !bSceneFailedForCharacter && (!IsDesignTime() || bStudioAnimationPreview) && SceneSettings.GetIssues().IsEmpty() && (!bStudioAnimationPreview || SceneSettings.SkeletalMesh) : ActivePresentation2D.Mode == ECharacter2DPresentationMode::CustomWidget && !bStudioAnimationPreview);
    if (CustomPresentationWidget && (!bCustom || bChanged || CustomPresentationWidget->GetClass() != DesiredClass))
    {
        if (auto* Scene = Cast<UCharacterSelect3DPreviewWidget>(CustomPresentationWidget)) Scene->ClearScene();
        CustomPresentationWidget->RemoveFromParent();
        CustomPresentationWidget = nullptr;
    }
    if (bCustom && !CustomPresentationWidget)
    {
        CustomPresentationWidget = CreateWidget<UCharacterSelectPresentationWidget>(this, DesiredClass);
        if (CustomPresentationWidget)
        {
            if (auto* Scene = Cast<UCharacterSelect3DPreviewWidget>(CustomPresentationWidget))
            {
                auto SafeSettings = SceneSettings;
                if (bStudioAnimationPreview)
                {
                    SafeSettings.ActorClass = nullptr; // Use the assigned mesh without running gameplay actor Blueprints.
#if WITH_EDITOR
                    Scene->SetDesignerFlags(EWidgetDesignFlags::None);
#endif
                    Scene->SetStudioAnimationPreview(true);
                }
                Scene->Configure(SafeSettings, InCharacter, PresentationPlayerIndex, bPresentationSelected && !ResolveSequence(InCharacter).bEnabled);
            }
            CustomPresentationSlot->SetContent(CustomPresentationWidget);
            CustomPresentationWidget->SetPresentation(InCharacter, PresentationPlayerIndex);
        }
    }
    if (CustomPresentationSlot) CustomPresentationSlot->SetVisibility(CustomPresentationWidget ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Collapsed);
    if (LargePortrait)
    {
        UTexture2D* Texture = InCharacter ? (ActivePresentation2D.Artwork ? ActivePresentation2D.Artwork.Get() : InCharacter->LargePortrait ? InCharacter->LargePortrait.Get() : InCharacter->Portrait.Get()) : nullptr;
        if (InCharacter && !bUsing3DPresentation && ActivePresentation2D.Mode == ECharacter2DPresentationMode::SpriteFrames)
        {
            PresentationFrameIndex = ActivePresentation2D.GetFrameIndex(PresentationSeconds);
            if (ActivePresentation2D.Frames.IsValidIndex(PresentationFrameIndex) && ActivePresentation2D.Frames[PresentationFrameIndex]) Texture = ActivePresentation2D.Frames[PresentationFrameIndex];
        }
        else PresentationFrameIndex = INDEX_NONE;
        LargePortrait->SetBrushFromTexture(Texture);
        LargePortrait->SetVisibility(Texture && !CustomPresentationWidget ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Collapsed);
    }
    ApplyPresentationTransform();
}

void UCharacterSelectPreviewWidget::AdvancePresentation2D(float DeltaTime)
{
    if (bUsing3DPresentation || (IsDesignTime() && !bStudioAnimationPreview) || !Presented2DCharacter.IsValid() || ActivePresentation2D.Mode != ECharacter2DPresentationMode::SpriteFrames || ActivePresentation2D.Frames.IsEmpty()) return;
    if (FMath::IsFinite(DeltaTime)) PresentationSeconds += FMath::Max(0.f, DeltaTime);
    const int32 Next = ActivePresentation2D.GetFrameIndex(PresentationSeconds);
    if (Next != PresentationFrameIndex) UpdatePresentation2D(Presented2DCharacter.Get());
}

void UCharacterSelectPreviewWidget::RemovePresentationTransform()
{
    if (!bPresentationTransformApplied) return;
    if (auto* Target = PresentationTransformTarget.Get())
    {
        FWidgetTransform Current = Target->GetRenderTransform();
        // Preserve components changed by authored animation since the last frame.
        if (Current.Scale == PresentationApplied.Scale) Current.Scale = PresentationBaseline.Scale;
        if (Current.Translation == PresentationApplied.Translation) Current.Translation = PresentationBaseline.Translation;
        Target->SetRenderTransform(Current);
    }
    bPresentationTransformApplied = false;
}

void UCharacterSelectPreviewWidget::ApplyPresentationTransform()
{
    RemovePresentationTransform();
    if (bUsing3DPresentation || (CustomPresentationWidget && ResolveSequence(Character).bEnabled))
    {
        // The 3D viewport follows the existing portrait's transition tracks. The
        // host supplies layout; camera/actor settings supply 3D framing.
        if (CustomPresentationWidget && LargePortrait)
        {
            CustomPresentationWidget->SetRenderTransform(LargePortrait->GetRenderTransform());
            CustomPresentationWidget->SetRenderOpacity(LargePortrait->GetRenderOpacity());
        }
        if (bUsing3DPresentation) return;
    }
    UWidget* Target = CustomPresentationWidget ? static_cast<UWidget*>(CustomPresentationWidget.Get()) : static_cast<UWidget*>(LargePortrait.Get());
    if (!Target || !Presented2DCharacter.IsValid()) return;
    PresentationTransformTarget = Target;
    PresentationBaseline = Target->GetRenderTransform();
    PresentationApplied = PresentationBaseline;
    const auto& Settings = ActivePresentation2D;
    const FVector2D Scale = Settings.Scale.ContainsNaN() ? FVector2D(1,1) : FVector2D(FMath::Clamp(Settings.Scale.X, .01, 10.), FMath::Clamp(Settings.Scale.Y, .01, 10.));
    PresentationApplied.Scale *= Scale;
    if (Settings.Facing == ECharacter2DFacing::Mirror || (Settings.Facing == ECharacter2DFacing::MirrorPlayer2 && PresentationPlayerIndex == 1)) PresentationApplied.Scale.X *= -1;
    PresentationApplied.Translation += Settings.Offset.ContainsNaN() ? FVector2D::ZeroVector : Settings.Offset;
    Target->SetRenderTransform(PresentationApplied);
    bPresentationTransformApplied = true;
}

FCharacterSelectSequence UCharacterSelectPreviewWidget::ResolveSequence(UCharacterSelectData* ForCharacter) const
{
    if (ForCharacter && ForCharacter->bOverrideSequence) return ForCharacter->Sequence;
    if (bOverrideSequence) return Sequence;
    return SharedSequence ? SharedSequence->Sequence : FCharacterSelectSequence();
}
FText UCharacterSelectPreviewWidget::GetSequenceSource(UCharacterSelectData* ForCharacter) const
{
    return FText::FromString(ForCharacter && ForCharacter->bOverrideSequence ? TEXT("Character override") : bOverrideSequence ? TEXT("Player preview override") : SharedSequence ? TEXT("Shared sequence asset") : TEXT("Existing animations"));
}
void UCharacterSelectPreviewWidget::EmitSequenceEvent(ECharacterPresentationEvent Event)
{
    if (IsDesignTime() || !Character || !ResolveSequence(Character).bEnabled) return;
    OnSequenceEvent(Event);
    if (CustomPresentationWidget) CustomPresentationWidget->OnSequenceEvent(Event);
}
void UCharacterSelectPreviewWidget::ResetSequenceConfirmation()
{
    bConfirmDelivered = false;
    ConfirmElapsed = 0.f;
}
void UCharacterSelectPreviewWidget::AdvanceSequenceConfirmation(float DeltaTime)
{
    const auto Settings = ResolveSequence(Character);
    if (!Settings.bEnabled || !Character || !bPresentationSelected || bConfirmDelivered || ConfiguredPhase == EConfiguredPhase::Exit || bIsChangingCharacter) return;
    if (Settings.bWaitForEntrance && ConfiguredPhase != EConfiguredPhase::Idle) return;
    ConfirmElapsed += FMath::IsFinite(DeltaTime) ? FMath::Max(0.f, DeltaTime) : 0.f;
    const float Delay = FMath::Clamp(FMath::IsFinite(Settings.ConfirmDelay) ? Settings.ConfirmDelay : 0.f, 0.f, 5.f);
    if (ConfirmElapsed < Delay) return;
    bConfirmDelivered = true;
    if (Settings.bRestartSpriteOnConfirm) { PresentationSeconds = 0.; UpdatePresentation2D(Character); }
    if (auto* Scene = Cast<UCharacterSelect3DPreviewWidget>(CustomPresentationWidget)) Scene->SetSelected(true);
    EmitSequenceEvent(ECharacterPresentationEvent::Confirm);
}
void UCharacterSelectPreviewWidget::SetPreviewSelected(bool bSelected)
{
    const bool bChanged = bPresentationSelected != bSelected;
    bPresentationSelected = bSelected;
    if (ConfiguredPhase == EConfiguredPhase::Exit || bIsChangingCharacter) { if (!bSelected) ResetSequenceConfirmation(); return; }
    const auto Settings = ResolveSequence(Character);
    if (!Settings.bEnabled)
    {
        if (auto* Scene = Cast<UCharacterSelect3DPreviewWidget>(CustomPresentationWidget)) Scene->SetSelected(bSelected);
        return;
    }
    if (!bChanged) return;
    ResetSequenceConfirmation();
    if (!bSelected)
    {
        if (auto* Scene = Cast<UCharacterSelect3DPreviewWidget>(CustomPresentationWidget)) Scene->SetSelected(false);
        if (Settings.bRestartSpriteOnCancel) { PresentationSeconds = 0.; UpdatePresentation2D(Character); }
        EmitSequenceEvent(ECharacterPresentationEvent::Cancel);
    }
    else AdvanceSequenceConfirmation(0.f);
}

void UCharacterSelectPreviewWidget::AdvanceStudioAnimation(float DeltaTime)
{
    if (!bStudioAnimationPreview) return;
    const bool bWaiting = ConfiguredPhase != EConfiguredPhase::Idle && ResolveSequence(Character).bWaitForEntrance;
    AdvanceConfiguredAnimation(DeltaTime);
    AdvanceSequenceConfirmation(bWaiting ? 0.f : DeltaTime);
    AdvancePresentation2D(DeltaTime);
    if (auto* Scene = Cast<UCharacterSelect3DPreviewWidget>(CustomPresentationWidget)) Scene->AdvanceStudioAnimation(DeltaTime);
    ApplyPresentationTransform();
}
void UCharacterSelectPreviewWidget::ReleaseStudioPresentation()
{
    if (auto* Scene = Cast<UCharacterSelect3DPreviewWidget>(CustomPresentationWidget)) Scene->ClearScene();
    if (CustomPresentationSlot) CustomPresentationSlot->ClearChildren();
    CustomPresentationWidget = nullptr;
}
FString UCharacterSelectPreviewWidget::GetStudioAnimationStatus() const
{
    if (bUsing3DPresentation && !CustomPresentationWidget) return TEXT("3D fallback: assign Skeletal Mesh and a presentation host. Actor Blueprint behavior requires Play.");
    if (ActivePresentation2D.Mode == ECharacter2DPresentationMode::CustomWidget && !bUsing3DPresentation) return TEXT("Custom widget fallback: Blueprint presentation events require Play.");
    if (!ResolveSequence(Character).bEnabled && PreviewAnimationSettings.Mode == ECharacterPreviewAnimationMode::Authored) return TEXT("Authored timelines require Play; choose configured animations or a sequence to preview motion here.");
    return ConfiguredPhase == EConfiguredPhase::Exit ? TEXT("Exit") : ConfiguredPhase == EConfiguredPhase::Entrance ? TEXT("Entrance") : bPresentationSelected ? (bConfirmDelivered ? TEXT("Confirmed") : TEXT("Waiting for confirm")) : TEXT("Ready");
}

void UCharacterSelectPreviewWidget::SetStudioAnimationPreview(bool bEnabled)
{
    bStudioAnimationPreview = bEnabled;
    if (!IsDesignTime()) return;
    RemovePresentationTransform();
    RestoreConfiguredElements();
    ReleaseStudioPresentation();
    ConfiguredPhase = EConfiguredPhase::Idle;
    Character = PendingCharacter = nullptr;
    bPresentationSelected = false;
    ResetSequenceConfirmation();
    RefreshPreview();
}
