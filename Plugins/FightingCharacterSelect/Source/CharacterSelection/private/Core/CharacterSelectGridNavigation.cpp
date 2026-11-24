#include "Core/CharacterSelectGridNavigation.h"

void UCharacterSelectGridNavigation::Initialize_Implementation(int32 InItemCount, const FCharacterSelectNavigationConfig& Config)
{
	ItemCount = FMath::Max(0, InItemCount);
	NavigationConfig = Config;
	NavigationConfig.Columns = FMath::Max(1, Config.Columns);
	NavigationConfig.GridLayout.Columns = NavigationConfig.Columns;
	NavigableIndices.Reset();
	for (int32 Index = 0; Index < ItemCount; ++Index) NavigableIndices.Add(Index);
}

void UCharacterSelectGridNavigation::SetNavigableIndices_Implementation(const TArray<int32>& InNavigableIndices)
{
	NavigableIndices.Reset();
	for (int32 Index : InNavigableIndices)
		if (Index >= 0 && Index < ItemCount) NavigableIndices.Add(Index);
}

int32 UCharacterSelectGridNavigation::GetNextIndex_Implementation(int32 CurrentIndex, ECharacterSelectDirection Direction) const
{
	if (CurrentIndex < 0 || CurrentIndex >= ItemCount) return INDEX_NONE;
	const bool bHorizontal = Direction == ECharacterSelectDirection::Left || Direction == ECharacterSelectDirection::Right;
	const bool bVertical = Direction == ECharacterSelectDirection::Up || Direction == ECharacterSelectDirection::Down;
	if (!bHorizontal && !bVertical) return CurrentIndex;
	const double Sign = (Direction == ECharacterSelectDirection::Right || Direction == ECharacterSelectDirection::Down) ? 1.0 : -1.0;
	const FCharacterSelectGridLayout& Layout = NavigationConfig.GridLayout;
	const FVector2D Current = Layout.GetSlotCoordinates(CurrentIndex, ItemCount);
	const bool bWrap = (bHorizontal ? NavigationConfig.HorizontalEdgeBehavior : NavigationConfig.VerticalEdgeBehavior) == ECharacterSelectEdgeBehavior::Wrap;

	// Prefer the nearest row in the requested direction, then its closest column.
	// Horizontal movement stays in the current row. A second pass wraps to the opposite edge.
	for (int32 Pass = 0; Pass < (bWrap ? 2 : 1); ++Pass)
	{
		int32 Best = INDEX_NONE;
		double BestPrimary = TNumericLimits<double>::Max();
		double BestSecondary = TNumericLimits<double>::Max();
		for (int32 Index = 0; Index < ItemCount; ++Index)
		{
			if (Index == CurrentIndex || !NavigableIndices.Contains(Index)) continue;
			const FVector2D Position = Layout.GetSlotCoordinates(Index, ItemCount);
			if (bHorizontal && Position.Y != Current.Y) continue;
			if (!bHorizontal && Position.Y == Current.Y) continue;
			const double Axis = bHorizontal ? Position.X : Position.Y;
			const double CurrentAxis = bHorizontal ? Current.X : Current.Y;
			const double Delta = (Axis - CurrentAxis) * Sign;
			if (Pass == 0 && Delta <= 0.0) continue;
			const double Primary = Pass == 0 ? Delta : Axis * Sign;
			const double Secondary = bHorizontal ? 0.0 : FMath::Abs(Position.X - Current.X);
			if (Primary < BestPrimary || (Primary == BestPrimary && Secondary < BestSecondary))
			{
				Best = Index;
				BestPrimary = Primary;
				BestSecondary = Secondary;
			}
		}
		if (Best != INDEX_NONE) return Best;
	}
	return CurrentIndex;
}
