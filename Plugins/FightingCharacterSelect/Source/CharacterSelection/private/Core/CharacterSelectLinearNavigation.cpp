#include "Core/CharacterSelectLinearNavigation.h"

int32 UCharacterSelectLinearNavigation::GetNextIndex_Implementation(int32 CurrentIndex, ECharacterSelectDirection Direction) const
{
	if (CurrentIndex < 0 || CurrentIndex >= ItemCount) return INDEX_NONE;
	const bool bHorizontal = NavigationConfig.Mode == ECharacterSelectNavigationMode::Horizontal;
	int32 Step = 0;
	if (bHorizontal)
	{
		if (Direction == ECharacterSelectDirection::Left) Step = -1;
		if (Direction == ECharacterSelectDirection::Right) Step = 1;
		if (NavigationConfig.GridLayout.bReverseHorizontal) Step *= -1;
	}
	else
	{
		if (Direction == ECharacterSelectDirection::Up) Step = -1;
		if (Direction == ECharacterSelectDirection::Down) Step = 1;
		if (NavigationConfig.GridLayout.bReverseVertical) Step *= -1;
	}
	if (Step == 0) return CurrentIndex;
	const bool bWrap = (bHorizontal ? NavigationConfig.HorizontalEdgeBehavior : NavigationConfig.VerticalEdgeBehavior) == ECharacterSelectEdgeBehavior::Wrap;
	int32 Target = CurrentIndex;
	for (int32 Attempt = 0; Attempt < ItemCount; ++Attempt)
	{
		Target += Step;
		if (Target < 0 || Target >= ItemCount)
		{
			if (!bWrap) return CurrentIndex;
			Target = Target < 0 ? ItemCount - 1 : 0;
		}
		if (NavigableIndices.Contains(Target)) return Target;
	}
	return CurrentIndex;
}
