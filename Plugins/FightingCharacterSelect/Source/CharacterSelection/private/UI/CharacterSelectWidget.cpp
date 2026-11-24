#include "UI/CharacterSelectWidget.h"

#include "Core/CharacterSelectionController.h"
#include "Core/CharacterSelectInputController.h"
#include "Data/CharacterSelectData.h"
#include "Data/CharacterSelectLayoutData.h"
#include "Data/CharacterSelectRegistry.h"
#include "UI/CharacterSelectGridWidget.h"
#include "UI/CharacterSelectPreviewWidget.h"

#include "Components/TextBlock.h"
#include "Components/Image.h"
#include "InputCoreTypes.h"
#include "Core/CharacterSelectInputBindings.h"
#include "CharacterSelectDeviceInputProcessor.h"
#include "Framework/Application/SlateApplication.h"
#include "Engine/GameViewportClient.h"
#include "Engine/World.h"
#include "Widgets/SOverlay.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Text/STextBlock.h"
#include "Styling/CoreStyle.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundBase.h"
#include "Widgets/Images/SImage.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SWrapBox.h"


UCharacterSelectWidget::UCharacterSelectWidget(
	const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	InitializeDefaultPlayerConfigs();
}

void UCharacterSelectWidget::NativeConstruct()
{
	Super::NativeConstruct();

	SetIsFocusable(true);

	UpdateControlTargetDisplay();
}

void UCharacterSelectWidget::NativeDestruct()
{
	ShutdownPlayerControllers();

	Super::NativeDestruct();
}

void UCharacterSelectWidget::InitializeDefaultPlayerConfigs()
{
	if (PlayerConfigs.Num() == 0)
	{
		PlayerConfigs.SetNum(2);

		PlayerConfigs[0].PlayerIndex = 0;
		PlayerConfigs[0].PlayerType = ECharacterSelectPlayerType::Human;
		PlayerConfigs[0].DisplayName =
			FText::FromString(TEXT("Player 1"));
		PlayerConfigs[0].InitialFocusIndex = 0;

		PlayerConfigs[1].PlayerIndex = 1;
		PlayerConfigs[1].PlayerType = ECharacterSelectPlayerType::CPU;
		PlayerConfigs[1].DisplayName =
			FText::FromString(TEXT("Player 2"));
		PlayerConfigs[1].InitialFocusIndex = INDEX_NONE;
	}
	else
	{
		// Ensure that both player configurations exist without
		// overwriting configurations already supplied by a subclass.
		const int32 ExistingCount = PlayerConfigs.Num();

		if (ExistingCount < 2)
		{
			PlayerConfigs.SetNum(2);

			if (ExistingCount < 1)
			{
				PlayerConfigs[0].PlayerIndex = 0;
				PlayerConfigs[0].PlayerType =
					ECharacterSelectPlayerType::Human;
				PlayerConfigs[0].DisplayName =
					FText::FromString(TEXT("Player 1"));
				PlayerConfigs[0].InitialFocusIndex = 0;
			}

			if (ExistingCount < 2)
			{
				PlayerConfigs[1].PlayerIndex = 1;
				PlayerConfigs[1].PlayerType =
					ECharacterSelectPlayerType::CPU;
				PlayerConfigs[1].DisplayName =
					FText::FromString(TEXT("Player 2"));
				PlayerConfigs[1].InitialFocusIndex = INDEX_NONE;
			}
		}
	}

	ActiveControlPlayerIndex = 0;
}

void UCharacterSelectWidget::InitializeCharacterSelect(
	UCharacterSelectRegistry* InRegistry,
	UCharacterSelectLayoutData* InLayout,
	int32 PlayerIndex)
{
	Registry = InRegistry;
	LayoutData = InLayout;

	if (!Registry || !LayoutData)
	{
		UE_LOG(
			LogTemp,
			Error,
			TEXT("InitializeCharacterSelect: Registry or Layout is null."));
		return;
	}

	InitializeDefaultPlayerConfigs();
	InitializePlayerControllers();

	if (PlayerSelectionControllers.Num() == 0)
	{
		UE_LOG(
			LogTemp,
			Error,
			TEXT("InitializeCharacterSelect: No player controllers were created."));
		return;
	}

	if (CharacterGrid)
	{
		CharacterGrid->InitializeGrid(PlayerSelectionControllers);
	}
	else
	{
		UE_LOG(
			LogTemp,
			Warning,
			TEXT("InitializeCharacterSelect: CharacterGrid is not bound."));
	}

	for (UCharacterSelectionController* Controller :
		PlayerSelectionControllers)
	{
		if (!Controller)
		{
			continue;
		}

		HandleCharacterFocusChanged(
			Controller,
			Controller->GetFocusedCharacter());
	}

	ActiveControlPlayerIndex = 0;

	JoinSession = NewObject<UCharacterSelectJoinSession>(this);
	JoinSession->OnStateChanged.AddDynamic(this, &UCharacterSelectWidget::HandleJoinStateChanged);
	JoinSession->OnFeedback.AddUObject(this, &UCharacterSelectWidget::HandleSelectionFeedback);
	TArray<UCharacterSelectionController*> Controllers;
	for (UCharacterSelectionController* Controller : PlayerSelectionControllers) Controllers.Add(Controller);
	JoinSession->bAutoCPUOpponent = bAutoCPUOpponent;
	JoinSession->bAllowLatePlayerJoin = bAllowLatePlayerJoin;
	JoinSession->Initialize(Controllers);
	SetupDeviceInput();
	SetKeyboardFocus();
}

void UCharacterSelectWidget::InitializePlayerControllers()
{
	ShutdownPlayerControllers();

	if (!Registry || !LayoutData)
	{
		return;
	}

	// This widget supports Player 1 and Player 2 / CPU.
	if (PlayerConfigs.Num() < 2)
	{
		UE_LOG(
			LogTemp,
			Error,
			TEXT("InitializePlayerControllers: Two player configurations are required."));
		return;
	}

	for (int32 ArrayIndex = 0; ArrayIndex < 2; ++ArrayIndex)
	{
		const FCharacterSelectPlayerConfig& PlayerConfig =
			PlayerConfigs[ArrayIndex];

		UCharacterSelectionController* NewSelectionController =
			NewObject<UCharacterSelectionController>(this);

		if (!NewSelectionController)
		{
			continue;
		}

		NewSelectionController->Initialize(
			Registry,
			LayoutData,
			ArrayIndex,
			PlayerConfig.InitialFocusIndex);

		NewSelectionController->OnCharacterFocusChanged.AddDynamic(
			this,
			&UCharacterSelectWidget::HandleCharacterFocusChanged);

		UCharacterSelectInputController* NewInputController =
			NewObject<UCharacterSelectInputController>(this);

		if (!NewInputController)
		{
			NewSelectionController->OnCharacterFocusChanged.RemoveDynamic(
				this,
				&UCharacterSelectWidget::HandleCharacterFocusChanged);

			continue;
		}

		// The input controller processes actions for this specific
		// selection controller. CPU status does not disable its
		// controller; it only determines who may send it input.
		NewInputController->Initialize(NewSelectionController);

		PlayerSelectionControllers.Add(NewSelectionController);
		PlayerInputControllers.Add(NewInputController);
	}

	if (PlayerSelectionControllers.Num() > 0)
	{
		SelectionController = PlayerSelectionControllers[0];
		InputController = PlayerInputControllers[0];
	}

	UpdateControlTargetDisplay();
}

void UCharacterSelectWidget::ShutdownPlayerControllers()
{
	RemoveDeviceInput();
	if (JoinSession) JoinSession->OnStateChanged.RemoveDynamic(this, &UCharacterSelectWidget::HandleJoinStateChanged);
	if (JoinSession) JoinSession->OnFeedback.RemoveAll(this);
	JoinSession = nullptr;
	bWasReady = false;
	for (UCharacterSelectionController* Controller :
		PlayerSelectionControllers)
	{
		if (Controller)
		{
			Controller->OnCharacterFocusChanged.RemoveDynamic(
				this,
				&UCharacterSelectWidget::HandleCharacterFocusChanged);
		}
	}

	PlayerSelectionControllers.Reset();
	PlayerInputControllers.Reset();

	SelectionController = nullptr;
	InputController = nullptr;

	ActiveControlPlayerIndex = 0;
}

void UCharacterSelectWidget::HandleCharacterFocusChanged(
	UCharacterSelectionController* Controller,
	UCharacterSelectData* Character)
{
	const int32 PlayerArrayIndex =
		GetPlayerIndexFromController(Controller);

	if (PlayerArrayIndex == INDEX_NONE)
	{
		return;
	}

	// Update the preview belonging to the controller that moved.
	UCharacterSelectPreviewWidget* PreviewWidget = nullptr;

	if (PlayerArrayIndex == 0)
	{
		PreviewWidget = Player1Preview;
	}
	else if (PlayerArrayIndex == 1)
	{
		PreviewWidget = Player2Preview;
	}

	if (PreviewWidget)
	{
		PreviewWidget->SetPresentationPlayerIndex(PlayerArrayIndex);
		PreviewWidget->SetCharacter(Character);
	}

	// Legacy single-player display is a fallback, never a second portrait beside P1.
	if (Player1Preview)
	{
		if (FocusedCharacterName) FocusedCharacterName->SetVisibility(ESlateVisibility::Collapsed);
		if (FocusedCharacterImage) FocusedCharacterImage->SetVisibility(ESlateVisibility::Collapsed);
		return;
	}
	if (PlayerArrayIndex != 0) return;
	if (Character)
	{
		if (FocusedCharacterName)
		{
			FocusedCharacterName->SetText(Character->DisplayName);
		}

		if (FocusedCharacterImage)
		{
			FocusedCharacterImage->SetBrushFromTexture(
				Character->LargePortrait);
		}
	}
	else
	{
		if (FocusedCharacterName)
		{
			FocusedCharacterName->SetText(FText::GetEmpty());
		}

		if (FocusedCharacterImage)
		{
			FocusedCharacterImage->SetBrush(FSlateBrush());
		}
	}
}

int32 UCharacterSelectWidget::GetPlayerIndexFromController(
	UCharacterSelectionController* Controller) const
{
	return PlayerSelectionControllers.IndexOfByKey(Controller);
}

UCharacterSelectionController*
UCharacterSelectWidget::GetSelectionController() const
{
	return SelectionController;
}

UCharacterSelectRegistry*
UCharacterSelectWidget::GetRegistry() const
{
	return Registry;
}

UCharacterSelectLayoutData*
UCharacterSelectWidget::GetLayoutData() const
{
	return LayoutData;
}

const FCharacterSelectPlayerConfig&
UCharacterSelectWidget::GetPlayerConfig(int32 PlayerIndex) const
{
	static const FCharacterSelectPlayerConfig DefaultConfig;

	return PlayerConfigs.IsValidIndex(PlayerIndex)
		? PlayerConfigs[PlayerIndex]
		: DefaultConfig;
}


bool UCharacterSelectWidget::IsCPUPlayer(int32 PlayerIndex) const
{
	return PlayerIndex == 1 && JoinSession && JoinSession->IsCPUOpponent();
}

int32 UCharacterSelectWidget::GetActiveControlPlayerIndex() const
{
	return JoinSession ? JoinSession->GetPlayerOneControlTarget() : 0;
}

UCharacterSelectionController* UCharacterSelectWidget::GetPlayerSelectionController(int32 PlayerIndex) const
{
	return PlayerSelectionControllers.IsValidIndex(PlayerIndex) ? PlayerSelectionControllers[PlayerIndex].Get() : nullptr;
}

void UCharacterSelectWidget::UpdateControlTargetDisplay()
{
	if (!ControlTargetText) return;
	ControlTargetText->SetVisibility(IsCPUPlayer(1) ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Collapsed);
	ControlTargetText->SetText(FText::FromString(TEXT("Player 1 is choosing the CPU opponent")));
}

bool UCharacterSelectWidget::IsPlayerVisible(int32 PlayerIndex) const
{
	return JoinSession && (JoinSession->HasJoined(PlayerIndex) || (PlayerIndex == 1 && JoinSession->IsCPUOpponent()));
}

void UCharacterSelectWidget::HandleJoinStateChanged()
{
	if (!JoinSession) return;
	ActiveControlPlayerIndex = JoinSession->GetPlayerOneControlTarget();
	for (int32 Index = 0; Index < 2; ++Index)
	{
		const bool bVisible = IsPlayerVisible(Index);
		if (PlayerConfigs.IsValidIndex(Index))
		{
			PlayerConfigs[Index].PlayerIndex = Index;
			PlayerConfigs[Index].PlayerType = IsCPUPlayer(Index) ? ECharacterSelectPlayerType::CPU : ECharacterSelectPlayerType::Human;
		}
		if (CharacterGrid) CharacterGrid->SetPlayerActive(Index, bVisible);
		auto* Preview = Index == 0 ? Player1Preview.Get() : Player2Preview.Get();
		if (Preview)
		{
            Preview->SetPresentationPlayerIndex(Index);
			Preview->SetVisibility(bVisible ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Hidden);
			if (bVisible && GetPlayerSelectionController(Index))
            {
                Preview->SetCharacter(GetPlayerSelectionController(Index)->GetFocusedCharacter());
                Preview->SetPreviewSelected(GetPlayerSelectionController(Index)->IsConfirmed());
            }
		}
		auto* Prompt = Index == 0 ? Player1JoinText.Get() : Player2JoinText.Get();
		if (Prompt) Prompt->SetText(GetJoinPrompt(Index));
	}
	if (FocusedCharacterName) FocusedCharacterName->SetVisibility(!Player1Preview && JoinSession->HasJoined(0) ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Collapsed);
	if (FocusedCharacterImage) FocusedCharacterImage->SetVisibility(!Player1Preview && JoinSession->HasJoined(0) ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Collapsed);
	UpdateControlTargetDisplay();
	const bool bReady = JoinSession->IsReady();
	const bool bNotifyReady = bReady && !bWasReady;
	bWasReady = bReady;
	if (bNotifyReady) OnPlayersReady.Broadcast();
}

bool UCharacterSelectWidget::IsDeviceInputActive() const
{
	if (!JoinSession || !IsInViewport() || !GetWorld() || !GetWorld()->IsGameWorld() || !FSlateApplication::IsInitialized()) return false;
	const TSharedPtr<SWidget> SlateWidget = GetCachedWidget();
	return SlateWidget.IsValid() && FSlateApplication::Get().IsActive()
		&& SlateWidget->HasAnyUserFocusOrFocusedDescendants()
		&& FSlateApplication::Get().FindWidgetWindow(SlateWidget.ToSharedRef()) == FSlateApplication::Get().GetActiveTopLevelWindow();
}

bool UCharacterSelectWidget::HandleDeviceKey(const FKeyEvent& Event)
{
	if (!JoinSession) return false;
	const FKey Key = Event.GetKey();
	// Preserve editor/window shortcuts, including Shift+F1 to release PIE input.
	if (Key.IsModifierKey() || Event.IsControlDown() || Event.IsAltDown() || Event.IsCommandDown()
		|| (Event.IsShiftDown() && Key == EKeys::F1) || Key == EKeys::F11 || Key == EKeys::Tilde) return false;
	return JoinSession->HandleInput(Key.IsGamepadKey() ? ECharacterSelectDeviceType::Gamepad : ECharacterSelectDeviceType::Keyboard,
		Event.GetInputDeviceId().GetId(), FCharacterSelectInputBindings::GetAction(Key, ControlSettings), FCharacterSelectInputBindings::CanJoin(Key) && (ControlSettings.bAnyButtonJoins || ControlSettings.Confirm.Contains(Key)), Event.IsRepeat());
}

void UCharacterSelectWidget::HandleDeviceAction(int32 GamepadDeviceId, ECharacterSelectInputAction Action)
{
	if (JoinSession) JoinSession->HandleInput(ECharacterSelectDeviceType::Gamepad, GamepadDeviceId, Action, false, true);
}

void UCharacterSelectWidget::NotifyGamepadDisconnected(int32 DeviceId)
{
	if (JoinSession) JoinSession->SetGamepadConnected(DeviceId, false);
}

void UCharacterSelectWidget::SetupDeviceInput()
{
	if (!FSlateApplication::IsInitialized()) return;
	if (!DeviceInputProcessor.IsValid())
	{
		DeviceInputProcessor = MakeShared<FCharacterSelectDeviceInputProcessor>(this);
		FSlateApplication::Get().RegisterInputPreProcessor(DeviceInputProcessor, 0);
	}
	CreateJoinPrompts();
}

void UCharacterSelectWidget::RemoveDeviceInput()
{
	if (DeviceInputProcessor.IsValid() && FSlateApplication::IsInitialized()) FSlateApplication::Get().UnregisterInputPreProcessor(DeviceInputProcessor);
	DeviceInputProcessor.Reset();
	if (PromptViewport.IsValid() && JoinPromptOverlay.IsValid()) PromptViewport->RemoveViewportWidgetContent(JoinPromptOverlay.ToSharedRef());
	JoinPromptOverlay.Reset();
	PromptViewport.Reset();
}

FText UCharacterSelectWidget::GetJoinPrompt_Implementation(int32 PlayerIndex) const
{
	if (PlayerIndex < 0 || PlayerIndex > 1) return FText::GetEmpty();
	const FString Name = PlayerConfigs.IsValidIndex(PlayerIndex) ? PlayerConfigs[PlayerIndex].DisplayName.ToString() : FString::Printf(TEXT("PLAYER %d"), PlayerIndex + 1);
	if (!JoinSession) return FText::FromString(Name + TEXT("\n") + PromptText.JoinAny.ToString());
	const auto Assignment = JoinSession->GetAssignment(PlayerIndex);
	if (Assignment.Type == ECharacterSelectDeviceType::None)
	{
		const bool bKeyboardTaken = JoinSession->GetPlayerForDevice(ECharacterSelectDeviceType::Keyboard, 0) != INDEX_NONE;
		const FString JoinHint = (bKeyboardTaken ? PromptText.JoinController : PromptText.JoinAny).ToString();
		if (PlayerIndex == 1 && IsCPUPlayer(1))
		{
			const auto* Controller = GetPlayerSelectionController(1);
			const FString Status = (Controller && Controller->IsConfirmed() ? PromptText.CPUReady : PromptText.CPUSelecting).ToString();
			return FText::FromString(Status + (bAllowLatePlayerJoin ? TEXT("\n") + JoinHint : FString()));
		}
		return FText::FromString(Name + TEXT(" - ") + PromptText.Waiting.ToString() + TEXT("\n") + JoinHint);
	}
	if (!Assignment.bConnected) return FText::FromString(Name + TEXT(" - ") + PromptText.Disconnected.ToString());
	const bool bKeyboard = Assignment.Type == ECharacterSelectDeviceType::Keyboard;
	const auto* Controller = GetPlayerSelectionController(PlayerIndex);
	const FText Status = Controller && Controller->IsConfirmed() ? PromptText.Ready : PromptText.Selecting;
	const bool bGeneratedHints = HintSettings.bShowActionHints && PromptAppearance.bShowGeneratedBanners && !(PlayerIndex == 0 ? Player1JoinText : Player2JoinText);
	return FText::FromString(Name + (bKeyboard ? TEXT(" / KEYBOARD - ") : TEXT(" / CONTROLLER - ")) + Status.ToString() + TEXT("\n")
		+ (bGeneratedHints ? FString() : (bKeyboard ? PromptText.KeyboardHelp : PromptText.GamepadHelp).ToString()));
}

float UCharacterSelectWidget::GetPromptOpacity(int32 PlayerIndex) const
{
	const bool bWaiting = !JoinSession || !JoinSession->HasJoined(PlayerIndex) || !JoinSession->GetAssignment(PlayerIndex).bConnected;
	const float MinOpacity = FMath::Clamp(JoinPromptMinOpacity, 0.0f, 1.0f);
	return bWaiting && JoinPromptPulseSpeed > 0.0f
		? MinOpacity + (1.0f - MinOpacity) * (0.5f + 0.5f * FMath::Sin(FPlatformTime::Seconds() * JoinPromptPulseSpeed)) : 1.0f;
}

void UCharacterSelectWidget::CreateJoinPrompts()
{
	if (!PromptAppearance.bShowGeneratedBanners || JoinPromptOverlay.IsValid() || (Player1JoinText && Player2JoinText) || !GetWorld()) return;
	UGameViewportClient* Viewport = GetWorld()->GetGameViewport();
	if (!Viewport) return;
	TSharedRef<SHorizontalBox> Prompts = SNew(SHorizontalBox);
	for (int32 Index = 0; Index < 2; ++Index)
	{
		const bool bHasCustomPrompt = Index == 0 ? Player1JoinText != nullptr : Player2JoinText != nullptr;
		TSharedRef<SWrapBox> Hints = SNew(SWrapBox).UseAllottedSize(true).HAlign(HAlign_Center)
			.Visibility_Lambda([this, Index] { return HintSettings.bShowActionHints && GetHintDeviceOwner(Index) != INDEX_NONE ? EVisibility::HitTestInvisible : EVisibility::Collapsed; });
		for (auto Action : {ECharacterSelectInputAction::NavigateUp, ECharacterSelectInputAction::NavigateDown,
			ECharacterSelectInputAction::NavigateLeft, ECharacterSelectInputAction::NavigateRight, ECharacterSelectInputAction::Confirm, ECharacterSelectInputAction::Cancel})
		{
			Hints->AddSlot().Padding(6,2)[SNew(SHorizontalBox)
				+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)
				[SNew(SBox).WidthOverride(FMath::Clamp(HintSettings.IconSize,8.f,128.f)).HeightOverride(FMath::Clamp(HintSettings.IconSize,8.f,128.f))
					.Visibility_Lambda([this, Index, Action] { return FindActionHintBrush(Index, Action) ? EVisibility::HitTestInvisible : EVisibility::Collapsed; })
					[SNew(SImage).Image_Lambda([this, Index, Action] { return FindActionHintBrush(Index, Action); })]]
				+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)
				[SNew(STextBlock).ColorAndOpacity(PromptAppearance.TextColor)
					.Text_Lambda([this, Index, Action]
					{
						const FText Label = HintSettings.GetActionLabel(Action);
						return FindActionHintBrush(Index, Action) ? Label : FText::Format(NSLOCTEXT("CharacterSelect", "KeyActionHint", "{0}: {1}"), GetActionHintLabel(Index, Action), Label);
					})]];
		}
		Prompts->AddSlot().FillWidth(1.0f).Padding(FMath::Max(0.0f, PromptAppearance.BannerGap))
		[
			SNew(SBorder).Padding(PromptAppearance.BannerPadding)
			.Visibility(bHasCustomPrompt ? EVisibility::Hidden : EVisibility::HitTestInvisible)
			.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
			.BorderBackgroundColor(PromptAppearance.Background)
			[
				SNew(SVerticalBox)
				+ SVerticalBox::Slot().AutoHeight()
				[
				SNew(STextBlock)
				.Text_Lambda([this, Index]() { return GetJoinPrompt(Index); })
				.ColorAndOpacity_Lambda([this, Index]() { FLinearColor Color = PromptAppearance.TextColor; Color.A *= GetPromptOpacity(Index); return FSlateColor(Color); })
				.Font(PromptAppearance.Font.FontObject ? PromptAppearance.Font : FCoreStyle::GetDefaultFontStyle("Bold", FMath::Max(1, PromptAppearance.Font.Size)))
				.Justification(ETextJustify::Center).AutoWrapText(true)
				]
				+ SVerticalBox::Slot().AutoHeight()[Hints]
			]
		];
	}
	JoinPromptOverlay = SNew(SOverlay).Visibility(EVisibility::HitTestInvisible)
		+ SOverlay::Slot().HAlign(HAlign_Fill).VAlign(PromptAppearance.bPlaceAtBottom ? VAlign_Bottom : VAlign_Top).Padding(PromptAppearance.ScreenMargin)[Prompts];
	PromptViewport = Viewport;
	Viewport->AddViewportWidgetContent(JoinPromptOverlay.ToSharedRef(), 100);
}

void UCharacterSelectWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);
	if (Player1JoinText) Player1JoinText->SetRenderOpacity(GetPromptOpacity(0));
	if (Player2JoinText) Player2JoinText->SetRenderOpacity(GetPromptOpacity(1));
	if (JoinPromptOverlay.IsValid()) JoinPromptOverlay->Invalidate(EInvalidateWidgetReason::Paint);
}

void UCharacterSelectWidget::HandleSelectionFeedback(ECharacterSelectFeedback Feedback, int32 PlayerIndex)
{
	if (GetWorld() && GetWorld()->IsGameWorld())
		if (const auto* Sound = SoundSettings.Sounds.Find(Feedback); Sound && *Sound && SoundSettings.Volume > 0)
			UGameplayStatics::PlaySound2D(this, *Sound, FMath::Clamp(SoundSettings.Volume,0.f,2.f), FMath::Clamp(SoundSettings.Pitch,0.1f,4.f));
	OnSelectionFeedback(Feedback, PlayerIndex);
}

int32 UCharacterSelectWidget::GetHintDeviceOwner(int32 PlayerIndex) const
{
	if (!JoinSession || PlayerIndex < 0 || PlayerIndex > 1) return INDEX_NONE;
	const int32 Owner = PlayerIndex == 1 && JoinSession->IsCPUOpponent() ? 0 : PlayerIndex;
	const auto Assignment = JoinSession->GetAssignment(Owner);
	return Assignment.Type != ECharacterSelectDeviceType::None && Assignment.bConnected ? Owner : INDEX_NONE;
}

FKey UCharacterSelectWidget::GetActionHintKey(int32 PlayerIndex, ECharacterSelectInputAction Action) const
{
	const int32 Owner = GetHintDeviceOwner(PlayerIndex);
	return Owner == INDEX_NONE ? FKey() : FCharacterSelectHintSettings::ResolveKey(ControlSettings, Action, JoinSession->GetAssignment(Owner).Type == ECharacterSelectDeviceType::Gamepad);
}

FText UCharacterSelectWidget::GetActionHintLabel(int32 PlayerIndex, ECharacterSelectInputAction Action) const
{
	return HintSettings.GetKeyLabel(GetActionHintKey(PlayerIndex, Action));
}

const FSlateBrush* UCharacterSelectWidget::FindActionHintBrush(int32 PlayerIndex, ECharacterSelectInputAction Action) const
{
	return HintSettings.FindGlyph(GetActionHintKey(PlayerIndex, Action), GetHintDeviceOwner(PlayerIndex));
}

FSlateBrush UCharacterSelectWidget::GetActionHintBrush(int32 PlayerIndex, ECharacterSelectInputAction Action) const
{
	const auto* Brush = FindActionHintBrush(PlayerIndex, Action);
	return Brush ? *Brush : FSlateBrush();
}
