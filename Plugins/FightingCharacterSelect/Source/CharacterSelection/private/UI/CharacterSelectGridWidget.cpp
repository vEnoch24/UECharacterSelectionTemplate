#include "UI/CharacterSelectGridWidget.h"

#include "Core/CharacterSelectionController.h"
#include "UI/CharacterSelectSlotWidget.h"
#include "Types/CharacterSelectRosterTypes.h"

#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include <Blueprint/UserWidget.h>


void UCharacterSelectGridWidget::NativeConstruct()
{
	Super::NativeConstruct();
}


void UCharacterSelectGridWidget::NativeDestruct()
{
	SlotWidgets.Reset();
	UnbindControllerEvents();

	SelectionController = nullptr;

	SelectionControllers.Reset();

	Super::NativeDestruct();
}

void UCharacterSelectGridWidget::InitializeGrid(
	const TArray<UCharacterSelectionController*>& InControllers)
{
	UnbindControllerEvents();

	SelectionController = nullptr;

	SelectionControllers.Reset();

	for (UCharacterSelectionController* Controller : InControllers)
	{
		if (!Controller)
		{
			continue;
		}

		SelectionControllers.Add(Controller);
	}

	if (SelectionControllers.IsEmpty())
	{
		UE_LOG(
			LogTemp,
			Warning,
			TEXT("CharacterSelectGridWidget: No valid selection controllers.")
		);

		return;
	}

	SelectionController = SelectionControllers[0];
	BindControllerEvents();

	RebuildGrid();
	RefreshLayout();
}

void UCharacterSelectGridWidget::BindControllerEvents()
{
	for (UCharacterSelectionController* Controller :
		SelectionControllers)
	{
		if (!Controller)
		{
			continue;
		}

		Controller->OnCharacterFocusChanged.AddUniqueDynamic(
			this,
			&UCharacterSelectGridWidget::HandleCharacterFocusChanged
		);

		Controller->OnCharacterSelected.AddUniqueDynamic(
			this,
			&UCharacterSelectGridWidget::HandleCharacterSelected
		);

		Controller->OnCharacterSelectionCancelled.AddUniqueDynamic(
			this,
			&UCharacterSelectGridWidget::HandleCharacterSelectionCancelled
		);

		Controller->OnCharacterSelectionConfirmed.AddUniqueDynamic(
			this,
			&UCharacterSelectGridWidget::HandleCharacterSelectionConfirmed
		);
	}
}

void UCharacterSelectGridWidget::UnbindControllerEvents()
{
	for (UCharacterSelectionController* Controller :
		SelectionControllers)
	{
		if (!Controller)
		{
			continue;
		}

		Controller->OnCharacterFocusChanged.RemoveDynamic(
			this,
			&UCharacterSelectGridWidget::HandleCharacterFocusChanged
		);

		Controller->OnCharacterSelected.RemoveDynamic(
			this,
			&UCharacterSelectGridWidget::HandleCharacterSelected
		);

		Controller->OnCharacterSelectionCancelled.RemoveDynamic(
			this,
			&UCharacterSelectGridWidget::HandleCharacterSelectionCancelled
		);

		Controller->OnCharacterSelectionConfirmed.RemoveDynamic(
			this,
			&UCharacterSelectGridWidget::HandleCharacterSelectionConfirmed
		);
	}

	SelectionControllers.Reset();
}


void UCharacterSelectGridWidget::RebuildGrid()
{
	if (!SlotContainer)
	{
		UE_LOG(
			LogTemp,
			Warning,
			TEXT("CharacterSelectGrid: RebuildGrid failed because SlotContainer is null.")
		);

		return;
	}

	if (!SlotWidgetClass)
	{
		UE_LOG(
			LogTemp,
			Warning,
			TEXT("CharacterSelectGrid: RebuildGrid failed because SlotWidgetClass is null.")
		);

		return;
	}

	SlotContainer->ClearChildren();

	SlotWidgets.Reset();

	GenerateSlots();

	RefreshSlotStates();

	RefreshLayout();

	InvalidateLayoutAndVolatility();

	UE_LOG(
		LogTemp,
		Log,
		TEXT("CharacterSelectGrid: Grid rebuilt and layout applied.")
	);
}



void UCharacterSelectGridWidget::GenerateSlots()
{
	if (!SelectionController)
	{
		UE_LOG(
			LogTemp,
			Warning,
			TEXT(
				"CharacterSelectGrid: "
				"SelectionController is null."
			)
		);

		return;
	}

	if (!SlotContainer)
	{
		UE_LOG(
			LogTemp,
			Warning,
			TEXT(
				"CharacterSelectGrid: "
				"SlotContainer is null."
			)
		);

		return;
	}

	const TArray<FCharacterSelectRosterEntry>& RosterEntries =
		SelectionController->GetRosterEntries();

	UE_LOG(
		LogTemp,
		Warning,
		TEXT(
			"CharacterSelectGrid: "
			"Roster contains %d entries."
		),
		RosterEntries.Num()
	);

	for (int32 Index = 0;
		Index < RosterEntries.Num();
		++Index)
	{
		const FCharacterSelectRosterEntry& Entry =
			RosterEntries[Index];

		if (!Entry.bVisible)
		{
			UE_LOG(
				LogTemp,
				Warning,
				TEXT(
					"CharacterSelectGrid: "
					"Roster entry %d is hidden."
				),
				Index
			);

			continue;
		}

		if (!Entry.Character)
		{
			UE_LOG(
				LogTemp,
				Warning,
				TEXT(
					"CharacterSelectGrid: "
					"Roster entry %d has no Character."
				),
				Index
			);

			continue;
		}

		UE_LOG(
			LogTemp,
			Warning,
			TEXT(
				"CharacterSelectGrid: "
				"Creating slot %d for %s."
			),
			Index,
			*Entry.Character.GetName()
		);

		UCharacterSelectSlotWidget* SlotWidget =
			CreateWidget<UCharacterSelectSlotWidget>(
				this,
				SlotWidgetClass
			);

		if (!SlotWidget)
		{
			UE_LOG(
				LogTemp,
				Error,
				TEXT(
					"CharacterSelectGrid: "
					"Failed to create slot %d."
				),
				Index
			);

			continue;
		}

		SlotWidget->SetCharacter(
			Entry.Character
		);

		SlotWidget->SetLocked(
			!Entry.bSelectable
		);

		SlotContainer->AddChild(
			SlotWidget
		);

		SlotWidgets.Add(
			SlotWidget
		);

		SlotWidget->SetRosterIndex(Index);
	}

	UE_LOG(
		LogTemp,
		Warning,
		TEXT(
			"CharacterSelectGrid: "
			"Created %d slot widgets."
		),
		SlotWidgets.Num()
	);
}


FCharacterSelectGridLayout UCharacterSelectGridWidget::GetEffectiveGridLayout() const
{
    FCharacterSelectGridLayout Effective = GridLayout;
    Effective.Columns = FMath::Max(1, Effective.Columns);
    Effective.Rows = FMath::Max(0, Effective.Rows);
    Effective.SlotSize.X = FMath::Max(1.0, Effective.SlotSize.X);
    Effective.SlotSize.Y = FMath::Max(1.0, Effective.SlotSize.Y);
    Effective.SlotSpacing.X = FMath::Max(0.0, Effective.SlotSpacing.X);
    Effective.SlotSpacing.Y = FMath::Max(0.0, Effective.SlotSpacing.Y);
    if (SelectionController)
    {
        if (SelectionController->GetNavigationMode() == ECharacterSelectNavigationMode::Horizontal)
        { Effective.Columns = FMath::Max(1, SelectionController->GetRosterEntries().Num()); Effective.Rows = 1; }
        else if (SelectionController->GetNavigationMode() == ECharacterSelectNavigationMode::Vertical)
        { Effective.Columns = 1; Effective.Rows = 0; }
    }
    return Effective;
}

void UCharacterSelectGridWidget::RefreshLayout()
{
    const FCharacterSelectGridLayout Effective = GetEffectiveGridLayout();
    for (const auto& Controller : SelectionControllers) if (Controller) Controller->ConfigureNavigationLayout(Effective);
    // Canvas anchors resolve against the actual SlotContainer on every Slate layout
    // pass. They do not require cached geometry or a first tick to be valid.
    PositionSlots();
    InvalidateLayoutAndVolatility();
}

void UCharacterSelectGridWidget::SetGridLayout(
	const FCharacterSelectGridLayout& InLayout)
{
	GridLayout = InLayout;

	GridLayout.Columns =
		FMath::Max(1, GridLayout.Columns);

	GridLayout.SlotSize.X =
		FMath::Max(1.0f, GridLayout.SlotSize.X);

	GridLayout.SlotSize.Y =
		FMath::Max(1.0f, GridLayout.SlotSize.Y);

	GridLayout.SlotSpacing.X =
		FMath::Max(0.0f, GridLayout.SlotSpacing.X);

	GridLayout.SlotSpacing.Y =
		FMath::Max(0.0f, GridLayout.SlotSpacing.Y);

	RefreshLayout();
}


const FCharacterSelectGridLayout&
UCharacterSelectGridWidget::GetGridLayout() const
{
	return GridLayout;
}


int32 UCharacterSelectGridWidget::GetCharacterCount() const
{
	return SlotWidgets.Num();
}


void UCharacterSelectGridWidget::PositionSlots()
{
    const FCharacterSelectGridLayout Effective = GetEffectiveGridLayout();
	if (!SlotContainer)
	{
		return;
	}

	const int32 CharacterCount = SelectionController ? SelectionController->GetRosterEntries().Num() : SlotWidgets.Num();

	if (CharacterCount <= 0)
	{
		return;
	}

	const int32 Columns =
		FMath::Max(1, Effective.Columns);

	const int32 Rows =
		Effective.GetRowCount(CharacterCount);

	if (Rows <= 0)
	{
		return;
	}

	const FVector2D GridSize =
		Effective.GetGridSize(CharacterCount);

	/*
	 * The important part of this implementation is that the
	 * CanvasPanel anchor itself determines WHERE the grid lives.
	 *
	 * The slot positions are then offsets relative to that anchor.
	 *
	 * This avoids relying on the actual pixel size of the root
	 * widget, which was causing the previous placement calculations
	 * to move the grid outside of the visible area.
	 */

	FVector2D PlacementAnchor, GridTopLeft;
	Effective.GetPlacement(GridSize, PlacementAnchor, GridTopLeft);

	/*
	 * Place every slot relative to the selected anchor.
	 */

	for (int32 Index = 0;
		Index < SlotWidgets.Num();
		++Index)
	{
		UCharacterSelectSlotWidget* SlotWidget =
			SlotWidgets[Index];

		if (!SlotWidget)
		{
			continue;
		}

		const FVector2D Coordinates = Effective.GetSlotCoordinates(SlotWidget->GetRosterIndex(), CharacterCount);
		const FVector2D SlotPosition = GridTopLeft + FVector2D(
			Coordinates.X * (Effective.SlotSize.X + Effective.SlotSpacing.X),
			Coordinates.Y * (Effective.SlotSize.Y + Effective.SlotSpacing.Y));

		UCanvasPanelSlot* CanvasSlot =
			Cast<UCanvasPanelSlot>(
				SlotWidget->Slot
			);

		if (!CanvasSlot)
		{
			continue;
		}

		/*
		 * The anchor is now responsible for positioning
		 * the entire grid inside SlotContainer.
		 */
		CanvasSlot->SetAnchors(
			FAnchors(
				PlacementAnchor.X,
				PlacementAnchor.Y
			)
		);

		/*
		 * Top-left alignment means SlotPosition is the
		 * actual top-left corner of this slot.
		 */
		CanvasSlot->SetAlignment(
			FVector2D(0.0f, 0.0f)
		);

		CanvasSlot->SetAutoSize(false);
		CanvasSlot->SetPosition(
			SlotPosition
		);

		CanvasSlot->SetSize(
			Effective.SlotSize
		);

		CanvasSlot->SetZOrder(
			Index
		);
	}
}


void UCharacterSelectGridWidget::RefreshSlotStates()
{
	if (SelectionControllers.Num() == 0)
	{
		return;
	}


	for (UCharacterSelectionController* Controller :
		SelectionControllers)
	{
		if (!Controller)
		{
			continue;
		}

		const int32 PlayerIndex =
			Controller->GetPlayerIndex();

		if (PlayerIndex < 0)
		{
			continue;
		}

		const int32 CurrentIndex =
			Controller->GetCurrentIndex();

		const int32 SelectedIndex =
			Controller->GetSelectionState()
			.SelectedCharacterIndex;

		const TArray<FCharacterSelectRosterEntry>& RosterEntries =
			Controller->GetRosterEntries();



		for (int32 Index = 0;
			Index < SlotWidgets.Num();
			++Index)
		{
			UCharacterSelectSlotWidget* SlotWidget =
				SlotWidgets[Index];

			if (!SlotWidget)
			{
				continue;
			}

			if (!RosterEntries.IsValidIndex(SlotWidget->GetRosterIndex()))
			{
				continue;
			}

			const FCharacterSelectRosterEntry& Entry =
				RosterEntries[SlotWidget->GetRosterIndex()];



			SlotWidget->SetFocusedForPlayer(
				PlayerIndex,
				!InactivePlayers.Contains(PlayerIndex) && SlotWidget->GetRosterIndex() == CurrentIndex
			);



			SlotWidget->SetSelectedForPlayer(
				PlayerIndex,
				!InactivePlayers.Contains(PlayerIndex) && SlotWidget->GetRosterIndex() == SelectedIndex
			);



			SlotWidget->SetLocked(
				!Entry.bSelectable
			);
		}
	}
}



void UCharacterSelectGridWidget::SetPlayerActive(int32 PlayerIndex, bool bActive)
{
	if (bActive) InactivePlayers.Remove(PlayerIndex);
	else InactivePlayers.Add(PlayerIndex);
	RefreshSlotStates();
}

void UCharacterSelectGridWidget::HandleCharacterFocusChanged(UCharacterSelectionController* Controller, UCharacterSelectData* Character)
{
	RefreshSlotStates();
}
void UCharacterSelectGridWidget::HandleCharacterSelected(UCharacterSelectionController* Controller, UCharacterSelectData* Character)
{
	RefreshSlotStates();
}
void UCharacterSelectGridWidget::HandleCharacterSelectionCancelled(UCharacterSelectionController* Controller)
{
	RefreshSlotStates();
}
void UCharacterSelectGridWidget::HandleCharacterSelectionConfirmed(UCharacterSelectionController* Controller)
{
	RefreshSlotStates();
}

