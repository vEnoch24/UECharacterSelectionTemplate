#include "Core/CharacterSelectJoinSession.h"
#include "Core/CharacterSelectionController.h"

void UCharacterSelectJoinSession::Initialize(const TArray<UCharacterSelectionController*>& InControllers)
{
	Controllers.Reset();
	for (auto* Controller : InControllers)
	{
		Controllers.Add(Controller);
		if (Controller) Controller->CancelSelection();
	}
	Assignments.Reset();
	Assignments.SetNum(2);
	OnStateChanged.Broadcast();
}

bool UCharacterSelectJoinSession::HasJoined(int32 PlayerIndex) const
{
	return Assignments.IsValidIndex(PlayerIndex) && Assignments[PlayerIndex].Type != ECharacterSelectDeviceType::None;
}

FCharacterSelectDeviceAssignment UCharacterSelectJoinSession::GetAssignment(int32 PlayerIndex) const
{
	return Assignments.IsValidIndex(PlayerIndex) ? Assignments[PlayerIndex] : FCharacterSelectDeviceAssignment();
}

int32 UCharacterSelectJoinSession::GetPlayerForDevice(ECharacterSelectDeviceType Type, int32 DeviceId) const
{
	if (Type == ECharacterSelectDeviceType::None) return INDEX_NONE;
	for (int32 Index = 0; Index < Assignments.Num(); ++Index)
	{
		const auto& Assignment = Assignments[Index];
		if (Assignment.Type == Type && (Type == ECharacterSelectDeviceType::Keyboard || Assignment.DeviceId == DeviceId)) return Index;
	}
	return INDEX_NONE;
}

bool UCharacterSelectJoinSession::IsCPUOpponent() const
{
	return bAutoCPUOpponent && HasJoined(0) && !HasJoined(1) && Controllers.IsValidIndex(0) && Controllers[0] && Controllers[0]->IsConfirmed();
}

int32 UCharacterSelectJoinSession::GetPlayerOneControlTarget() const
{
	return IsCPUOpponent() ? 1 : 0;
}

bool UCharacterSelectJoinSession::IsReady() const
{
	if (Controllers.Num() != 2 || !Controllers[0] || !Controllers[1] || !HasJoined(0) || !Assignments[0].bConnected) return false;
	if (!HasJoined(1) && !bAutoCPUOpponent) return false;
	if (HasJoined(1) && !Assignments[1].bConnected) return false;
	return Controllers[0]->IsConfirmed() && Controllers[1]->IsConfirmed();
}

bool UCharacterSelectJoinSession::HandleInput(ECharacterSelectDeviceType Type, int32 DeviceId, ECharacterSelectInputAction Action, bool bCanJoin, bool bRepeat)
{
	if (Controllers.Num() != 2 || !Controllers[0] || !Controllers[1] || !Controllers[0]->IsInitialized() || !Controllers[1]->IsInitialized()) return false;
	if (Type == ECharacterSelectDeviceType::None || (Type == ECharacterSelectDeviceType::Gamepad && DeviceId < 0)) return false;
	if (Type == ECharacterSelectDeviceType::Keyboard) DeviceId = 0; // All keyboards form one logical device.
	int32 Player = GetPlayerForDevice(Type, DeviceId);
	if (Player == INDEX_NONE)
	{
		if (!bCanJoin || bRepeat) return true;
		Player = !HasJoined(0) ? 0 : (!HasJoined(1) ? 1 : INDEX_NONE);
		if (Player == INDEX_NONE) return true;
		if (Player == 1 && IsCPUOpponent() && !bAllowLatePlayerJoin) return true;
		Assignments[Player].Type = Type;
		Assignments[Player].DeviceId = DeviceId;
		Assignments[Player].bConnected = true;
		// A late human takes over the CPU slot, without inheriting its confirmation.
		Controllers[Player]->CancelSelection();
		OnFeedback.Broadcast(ECharacterSelectFeedback::Joined, Player);
		OnStateChanged.Broadcast();
		return true; // The join press never also moves or confirms.
	}
	if (!Assignments[Player].bConnected)
	{
		if (!bCanJoin || bRepeat) return true;
		Assignments[Player].bConnected = true;
		OnFeedback.Broadcast(ECharacterSelectFeedback::Reconnected, Player);
		OnStateChanged.Broadcast();
		return true;
	}
	if (bRepeat && (Action == ECharacterSelectInputAction::Confirm || Action == ECharacterSelectInputAction::Cancel)) return true;
	if (Action == ECharacterSelectInputAction::None) return true;

	const int32 Target = Player == 0 ? GetPlayerOneControlTarget() : 1;
	auto* Controller = Controllers[Target].Get();
	const int32 PreviousIndex = Controller->GetCurrentIndex();
	const bool bPreviouslyConfirmed = Controller->IsConfirmed();
	const bool bPreviouslySelected = Controller->HasSelectedCharacter();
	const bool bP1PreviouslySelected = Controllers[0]->HasSelectedCharacter();
	switch (Action)
	{
	case ECharacterSelectInputAction::NavigateUp: Controller->MoveSelection(ECharacterSelectDirection::Up); break;
	case ECharacterSelectInputAction::NavigateDown: Controller->MoveSelection(ECharacterSelectDirection::Down); break;
	case ECharacterSelectInputAction::NavigateLeft: Controller->MoveSelection(ECharacterSelectDirection::Left); break;
	case ECharacterSelectInputAction::NavigateRight: Controller->MoveSelection(ECharacterSelectDirection::Right); break;
	case ECharacterSelectInputAction::Confirm: Controller->ConfirmSelection(); break;
	case ECharacterSelectInputAction::Cancel:
		if (Player == 0 && IsCPUOpponent() && !Controller->HasSelectedCharacter())
		{
			Controllers[0]->CancelSelection(); // Back from CPU picking to P1 picking.
		}
		else Controller->CancelSelection();
		break;
	default: break;
	}
	if (Controller->GetCurrentIndex() != PreviousIndex) OnFeedback.Broadcast(ECharacterSelectFeedback::Moved, Target);
	else if (Action == ECharacterSelectInputAction::Confirm && !bPreviouslyConfirmed)
		OnFeedback.Broadcast(Controller->IsConfirmed() ? ECharacterSelectFeedback::Confirmed : ECharacterSelectFeedback::Rejected, Target);
	else if (Action == ECharacterSelectInputAction::Cancel)
	{
		if (bPreviouslySelected && !Controller->HasSelectedCharacter()) OnFeedback.Broadcast(ECharacterSelectFeedback::Cancelled, Target);
		else if (bP1PreviouslySelected && !Controllers[0]->HasSelectedCharacter()) OnFeedback.Broadcast(ECharacterSelectFeedback::Cancelled, 0);
	}
	OnStateChanged.Broadcast();
	return true;
}

void UCharacterSelectJoinSession::SetGamepadConnected(int32 DeviceId, bool bConnected)
{
	const int32 Player = GetPlayerForDevice(ECharacterSelectDeviceType::Gamepad, DeviceId);
	if (Player == INDEX_NONE || Assignments[Player].bConnected == bConnected) return;
	Assignments[Player].bConnected = bConnected;
	OnStateChanged.Broadcast();
}
