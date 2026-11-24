#include "Core/CharacterSelectInputController.h"

#include "Core/CharacterSelectionController.h"

void UCharacterSelectInputController::Initialize(
	UCharacterSelectionController* InSelectionController)
{
	SelectionController = InSelectionController;
}

void UCharacterSelectInputController::ProcessAction(
	ECharacterSelectInputAction Action)
{
	if (!SelectionController)
	{
		return;
	}

	switch (Action)
	{
	case ECharacterSelectInputAction::NavigateUp:
		Navigate(ECharacterSelectDirection::Up);
		break;

	case ECharacterSelectInputAction::NavigateDown:
		Navigate(ECharacterSelectDirection::Down);
		break;

	case ECharacterSelectInputAction::NavigateLeft:
		Navigate(ECharacterSelectDirection::Left);
		break;

	case ECharacterSelectInputAction::NavigateRight:
		Navigate(ECharacterSelectDirection::Right);
		break;

	case ECharacterSelectInputAction::Confirm:
		Confirm();
		break;

	case ECharacterSelectInputAction::Cancel:
		Cancel();
		break;

	case ECharacterSelectInputAction::None:
	default:
		break;
	}
}

void UCharacterSelectInputController::Navigate(
	ECharacterSelectDirection Direction)
{
	if (!SelectionController)
	{
		return;
	}

	SelectionController->MoveSelection(
		Direction
	);
}

void UCharacterSelectInputController::Confirm()
{
	if (!SelectionController)
	{
		return;
	}

	SelectionController->ConfirmSelection();
}

void UCharacterSelectInputController::Cancel()
{
	if (!SelectionController)
	{
		return;
	}

	SelectionController->CancelSelection();
}

UCharacterSelectionController*
UCharacterSelectInputController::GetSelectionController() const
{
	return SelectionController;
}