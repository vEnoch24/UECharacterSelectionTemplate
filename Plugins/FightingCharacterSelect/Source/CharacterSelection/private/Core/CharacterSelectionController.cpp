
#include "Core/CharacterSelectionController.h"

#include "Data/CharacterSelectData.h"
#include "Data/CharacterSelectRegistry.h"
#include "Data/CharacterSelectLayoutData.h"
#include "Types/CharacterSelectNavigationTypes.h"

void UCharacterSelectionController::Initialize(
	UCharacterSelectRegistry* InRegistry,
	UCharacterSelectLayoutData* InLayout,
	int32 InPlayerIndex,
	int32 InInitialFocusIndex)
{
	// Reset controller state.
	Registry = InRegistry;
	LayoutData = InLayout;
	NavigationProvider = nullptr;
	bHasDisplayLayout = false;

	SelectionState = FCharacterSelectionState();
	SelectionState.PlayerIndex = InPlayerIndex;

	RosterEntries.Reset();
	CurrentIndex = INDEX_NONE;
	bInitialized = false;

	if (!Registry)
	{
		return;
	}

	if (LayoutData && !UCharacterSelectNavigationProvider::IsModeSupported(LayoutData->NavigationMode))
	{
		UE_LOG(LogTemp, Warning, TEXT("Unsupported saved navigation mode; using Grid. Choose Grid, Horizontal, or Vertical in the layout asset."));
	}

	// Locked entries may be focused for preview, but never confirmed.
	const TArray<UCharacterSelectData*> AvailableCharacters =
		Registry->GetAllCharacters();

	for (UCharacterSelectData* Character : AvailableCharacters)
	{
		if (!Character)
		{
			continue;
		}

		const bool bAvailable = Character->Availability == ECharacterAvailability::Available;
		const bool bLocked = Character->Availability == ECharacterAvailability::Locked;
		if (!bAvailable && !(bLocked && LayoutData && LayoutData->bAllowLockedSlots)) continue;

		FCharacterSelectRosterEntry Entry;
		Entry.Character = Character;
		Entry.SlotType = ECharacterSelectSlotType::Character;
		Entry.bNavigable = true;
		Entry.bSelectable = bAvailable;
		Entry.bVisible = true;

		RosterEntries.Add(Entry);
	}

	if (RosterEntries.IsEmpty())
	{
		return;
	}

	// Gather navigable roster indices.
	TArray<int32> NavigableIndices;

	for (int32 Index = 0; Index < RosterEntries.Num(); ++Index)
	{
		if (RosterEntries[Index].bNavigable && RosterEntries[Index].bVisible)
		{
			NavigableIndices.Add(Index);
		}
	}

	if (NavigableIndices.IsEmpty())
	{
		return;
	}

	SetNavigationProvider(UCharacterSelectNavigationProvider::CreateForMode(this, GetNavigationMode()));

	// Select the configured starting index when valid.
	if (NavigableIndices.Contains(InInitialFocusIndex))
	{
		CurrentIndex = InInitialFocusIndex;
	}
	else if (InPlayerIndex == 1)
	{
		// Player 2 defaults to the last navigable character.
		CurrentIndex = NavigableIndices.Last();
	}
	else
	{
		// Other players default to the first navigable character.
		CurrentIndex = NavigableIndices[0];
	}

	SelectionState.FocusedCharacterIndex = CurrentIndex;
	bInitialized = true;

	// Notify listeners of the initial focus.
	OnCharacterFocusChanged.Broadcast(
		this,
		GetFocusedCharacter());
}

UCharacterSelectData*
UCharacterSelectionController::GetFocusedCharacter() const
{
	if (!RosterEntries.IsValidIndex(CurrentIndex))
	{
		return nullptr;
	}

	return RosterEntries[CurrentIndex].Character;
}

int32 UCharacterSelectionController::GetCurrentIndex() const
{
	return CurrentIndex;
}

int32 UCharacterSelectionController::GetPlayerIndex() const
{
	return SelectionState.PlayerIndex;
}

bool UCharacterSelectionController::IsInitialized() const
{
	return bInitialized;
}

bool UCharacterSelectionController::IsConfirmed() const
{
	return SelectionState.bIsConfirmed;
}

bool UCharacterSelectionController::FocusCharacterAtIndex(
	int32 RosterIndex)
{
	if (!bInitialized ||
		SelectionState.bIsConfirmed ||
		!RosterEntries.IsValidIndex(RosterIndex))
	{
		return false;
	}

	const FCharacterSelectRosterEntry& Entry =
		RosterEntries[RosterIndex];

	if (!Entry.bNavigable ||
		!Entry.bVisible ||
		!Entry.Character)
	{
		return false;
	}

	// The requested character is already focused.
	if (CurrentIndex == RosterIndex)
	{
		return true;
	}

	CurrentIndex = RosterIndex;
	SelectionState.FocusedCharacterIndex = CurrentIndex;

	OnCharacterFocusChanged.Broadcast(
		this,
		Entry.Character);

	return true;
}

void UCharacterSelectionController::MoveSelection(
	ECharacterSelectDirection Direction)
{
	if (!bInitialized ||
		SelectionState.bIsConfirmed ||
		!NavigationProvider)
	{
		return;
	}

	int32 TargetIndex = CurrentIndex;
	// Explicit links take precedence over geometry and custom providers. Resolve by
	// stable IDs against the full registry, then the currently eligible roster.
	if (LayoutData && LayoutData->bUseDirectionalLinks && GetFocusedCharacter())
	{
		const FName SourceID = GetFocusedCharacter()->CharacterID;
		if (const auto* Links = LayoutData->DirectionalLinks.Find(SourceID))
		{
			const FCharacterSelectDirectionalLink* Link = nullptr;
			switch (Direction)
			{
			case ECharacterSelectDirection::Up: Link = &Links->Up; break;
			case ECharacterSelectDirection::Down: Link = &Links->Down; break;
			case ECharacterSelectDirection::Left: Link = &Links->Left; break;
			case ECharacterSelectDirection::Right: Link = &Links->Right; break;
			default: break;
			}
			if (Link && Link->Rule != ECharacterSelectLinkRule::Automatic)
			{
				if (Link->Rule != ECharacterSelectLinkRule::Character || SourceID.IsNone() || Link->TargetCharacterID.IsNone()) return;
				int32 Sources = 0, Destinations = 0;
				for (auto* Character : Registry->GetAllCharacters()) if (Character)
				{
					Sources += Character->CharacterID == SourceID;
					Destinations += Character->CharacterID == Link->TargetCharacterID;
				}
				if (Sources != 1 || Destinations != 1) return;
				for (int32 Index = 0; Index < RosterEntries.Num(); ++Index)
					if (RosterEntries[Index].Character && RosterEntries[Index].Character->CharacterID == Link->TargetCharacterID)
					{ FocusCharacterAtIndex(Index); return; }
				return; // Unavailable destinations stop; do not silently choose another character.
			}
		}
	}
	bool bFoundValidSlot = false;

	// Skip non-navigable entries when the provider permits movement.
	for (int32 Attempt = 0;
		Attempt < RosterEntries.Num();
		++Attempt)
	{
		const int32 NextIndex =
			NavigationProvider->GetNextIndex(
				TargetIndex,
				Direction);

		if (NextIndex == INDEX_NONE ||
			NextIndex == TargetIndex ||
			!RosterEntries.IsValidIndex(NextIndex))
		{
			break;
		}

		const FCharacterSelectRosterEntry& Entry =
			RosterEntries[NextIndex];

		if (Entry.bNavigable && Entry.bVisible)
		{
			TargetIndex = NextIndex;
			bFoundValidSlot = true;
			break;
		}

		TargetIndex = NextIndex;

		// Avoid cycling forever when navigation wraps.
		if (TargetIndex == CurrentIndex)
		{
			break;
		}
	}

	if (!bFoundValidSlot || TargetIndex == CurrentIndex)
	{
		return;
	}

	CurrentIndex = TargetIndex;
	SelectionState.FocusedCharacterIndex = CurrentIndex;

	OnCharacterFocusChanged.Broadcast(
		this,
		RosterEntries[CurrentIndex].Character);
}

void UCharacterSelectionController::SelectCharacter()
{
	if (!bInitialized || SelectionState.bIsConfirmed)
	{
		return;
	}

	if (!RosterEntries.IsValidIndex(CurrentIndex))
	{
		return;
	}

	const FCharacterSelectRosterEntry& Entry =
		RosterEntries[CurrentIndex];

	if (Entry.SlotType != ECharacterSelectSlotType::Character ||
		!Entry.bSelectable ||
		!Entry.bVisible ||
		!Entry.Character)
	{
		return;
	}

	SelectionState.SelectedCharacterIndex = CurrentIndex;
	SelectionState.bHasSelectedCharacter = true;
	SelectionState.CostumeIndex = 0;
	SelectionState.ColorIndex = 0;

	OnCharacterSelected.Broadcast(
		this,
		Entry.Character);
}

void UCharacterSelectionController::ConfirmSelection()
{
	if (!bInitialized || SelectionState.bIsConfirmed)
	{
		return;
	}

	if (!SelectionState.bHasSelectedCharacter)
	{
		SelectCharacter();
	}

	if (!SelectionState.bHasSelectedCharacter)
	{
		return;
	}

	SelectionState.bIsConfirmed = true;

	OnCharacterSelectionConfirmed.Broadcast(this);
}

void UCharacterSelectionController::CancelSelection()
{
	if (!bInitialized || !SelectionState.bHasSelectedCharacter)
	{
		return;
	}

	SelectionState.SelectedCharacterIndex = INDEX_NONE;
	SelectionState.bHasSelectedCharacter = false;
	SelectionState.bIsConfirmed = false;
	SelectionState.CostumeIndex = 0;
	SelectionState.ColorIndex = 0;

	OnCharacterSelectionCancelled.Broadcast(this);
}

UCharacterSelectData*
UCharacterSelectionController::GetSelectedCharacter() const
{
	const int32 SelectedIndex =
		SelectionState.SelectedCharacterIndex;

	if (!RosterEntries.IsValidIndex(SelectedIndex))
	{
		return nullptr;
	}

	const FCharacterSelectRosterEntry& Entry =
		RosterEntries[SelectedIndex];

	if (Entry.SlotType != ECharacterSelectSlotType::Character ||
		!Entry.Character)
	{
		return nullptr;
	}

	return Entry.Character;
}

void UCharacterSelectionController::SetNavigationProvider(
	UCharacterSelectNavigationProvider* InNavigationProvider)
{
	NavigationProvider = InNavigationProvider;

	if (!NavigationProvider)
	{
		return;
	}

	const FCharacterSelectNavigationConfig Config = BuildNavigationConfig();

	NavigationProvider->Initialize(
		RosterEntries.Num(),
		Config);

	TArray<int32> NavigableIndices;

	for (int32 Index = 0; Index < RosterEntries.Num(); ++Index)
	{
		if (RosterEntries[Index].bNavigable && RosterEntries[Index].bVisible)
		{
			NavigableIndices.Add(Index);
		}
	}

	NavigationProvider->SetNavigableIndices(NavigableIndices);
}

const TArray<FCharacterSelectRosterEntry>&
UCharacterSelectionController::GetRosterEntries() const
{
	return RosterEntries;
}

bool UCharacterSelectionController::HasSelectedCharacter() const
{
	return
		SelectionState.bHasSelectedCharacter &&
		RosterEntries.IsValidIndex(
			SelectionState.SelectedCharacterIndex);
}

const FCharacterSelectionState&
UCharacterSelectionController::GetSelectionState() const
{
	return SelectionState;
}

ECharacterSelectNavigationMode UCharacterSelectionController::GetNavigationMode() const
{
	const ECharacterSelectNavigationMode Mode = LayoutData ? LayoutData->NavigationMode : ECharacterSelectNavigationMode::Grid;
	return UCharacterSelectNavigationProvider::IsModeSupported(Mode) ? Mode : ECharacterSelectNavigationMode::Grid;
}

FCharacterSelectNavigationConfig UCharacterSelectionController::BuildNavigationConfig() const
{
	FCharacterSelectNavigationConfig Config;
	Config.Mode = GetNavigationMode();
	if (LayoutData)
	{
		Config.Columns = LayoutData->Columns;
		Config.HorizontalEdgeBehavior = LayoutData->HorizontalEdgeBehavior;
		Config.VerticalEdgeBehavior = LayoutData->VerticalEdgeBehavior;
		Config.GridLayout.Columns = LayoutData->Columns;
		Config.GridLayout.Rows = LayoutData->Rows;
	}
	if (bHasDisplayLayout)
	{
		Config.GridLayout = DisplayLayout;
		Config.Columns = DisplayLayout.Columns;
	}
	if (Config.Mode == ECharacterSelectNavigationMode::Horizontal)
	{
		Config.Columns = FMath::Max(1, RosterEntries.Num());
		Config.GridLayout.Rows = 1;
	}
	else if (Config.Mode == ECharacterSelectNavigationMode::Vertical)
	{
		Config.Columns = 1;
		Config.GridLayout.Rows = 0;
	}
	Config.GridLayout.Columns = Config.Columns;
	return Config;
}

void UCharacterSelectionController::ConfigureNavigationLayout(const FCharacterSelectGridLayout& InLayout)
{
	DisplayLayout = InLayout;
	bHasDisplayLayout = true;
	SetNavigationProvider(NavigationProvider);
}
