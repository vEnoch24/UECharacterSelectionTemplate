#pragma once

#include "CoreMinimal.h"
#include "Layout/Margin.h"
#include "CharacterSelectGridTypes.generated.h"

UENUM(BlueprintType)
enum class ECharacterSelectGridPlacement : uint8
{
	Center,

	TopCenter,
	BottomCenter,

	LeftCenter,
	RightCenter,

	TopLeft,
	TopRight,

	BottomLeft,
	BottomRight,

	Custom
};

USTRUCT(BlueprintType)
struct FCharacterSelectGridLayout
{
	GENERATED_BODY()

	// =========================================================
	// Grid
	// =========================================================

	UPROPERTY(
		EditAnywhere,
		BlueprintReadWrite,
		Category = "Grid",
		meta = (ClampMin = "1")
	)
	int32 Columns = 7;

	UPROPERTY(
		EditAnywhere,
		BlueprintReadWrite,
		Category = "Grid",
		meta = (ClampMin = "0")
	)
	int32 Rows = 0;


	// =========================================================
	// Slot
	// =========================================================

	UPROPERTY(
		EditAnywhere,
		BlueprintReadWrite,
		Category = "Slot"
	)
	FVector2D SlotSize = FVector2D(120.0f, 120.0f);

	UPROPERTY(
		EditAnywhere,
		BlueprintReadWrite,
		Category = "Slot"
	)
	FVector2D SlotSpacing = FVector2D(8.0f, 8.0f);


	// =========================================================
	// Placement
	// =========================================================

	UPROPERTY(
		EditAnywhere,
		BlueprintReadWrite,
		Category = "Placement"
	)
	ECharacterSelectGridPlacement Placement =
		ECharacterSelectGridPlacement::BottomCenter;


	// =========================================================
	// Padding
	// =========================================================

	UPROPERTY(
		EditAnywhere,
		BlueprintReadWrite,
		Category = "Placement"
	)
	FMargin Padding = FMargin(40.0f);


	// =========================================================
	// Alignment
	// =========================================================

	UPROPERTY(
		EditAnywhere,
		BlueprintReadWrite,
		Category = "Grid"
	)
	bool bCenterIncompleteLastRow = true;


	// =========================================================
	// Direction
	// =========================================================

	UPROPERTY(
		EditAnywhere,
		BlueprintReadWrite,
		Category = "Grid"
	)
	bool bReverseHorizontal = false;

	UPROPERTY(
		EditAnywhere,
		BlueprintReadWrite,
		Category = "Grid"
	)
	bool bReverseVertical = false;

	/** Shared anchor math for runtime slots and the editor layout preview. */
	void GetPlacement(const FVector2D& GridSize, FVector2D& PlacementAnchor, FVector2D& GridTopLeft) const
	{
		PlacementAnchor = FVector2D(0.5f, 0.5f);
		GridTopLeft = -GridSize * 0.5f;
	
		const float LeftPadding =
			Padding.Left;
	
		const float RightPadding =
			Padding.Right;
	
		const float TopPadding =
			Padding.Top;
	
		const float BottomPadding =
			Padding.Bottom;
	
		switch (Placement)
		{
		case ECharacterSelectGridPlacement::Center:
		{
			PlacementAnchor =
				FVector2D(0.5f, 0.5f);
	
			GridTopLeft =
				FVector2D(
					-GridSize.X * 0.5f,
					-GridSize.Y * 0.5f
				);
	
			break;
		}
	
		case ECharacterSelectGridPlacement::TopCenter:
		{
			PlacementAnchor =
				FVector2D(0.5f, 0.0f);
	
			GridTopLeft =
				FVector2D(
					-GridSize.X * 0.5f,
					TopPadding
				);
	
			break;
		}
	
		case ECharacterSelectGridPlacement::BottomCenter:
		{
			PlacementAnchor =
				FVector2D(0.5f, 1.0f);
	
			GridTopLeft =
				FVector2D(
					-GridSize.X * 0.5f,
					-BottomPadding - GridSize.Y
				);
	
			break;
		}
	
		case ECharacterSelectGridPlacement::LeftCenter:
		{
			PlacementAnchor =
				FVector2D(0.0f, 0.5f);
	
			GridTopLeft =
				FVector2D(
					LeftPadding,
					-GridSize.Y * 0.5f
				);
	
			break;
		}
	
		case ECharacterSelectGridPlacement::RightCenter:
		{
			PlacementAnchor =
				FVector2D(1.0f, 0.5f);
	
			GridTopLeft =
				FVector2D(
					-RightPadding - GridSize.X,
					-GridSize.Y * 0.5f
				);
	
			break;
		}
	
		case ECharacterSelectGridPlacement::TopLeft:
		{
			PlacementAnchor =
				FVector2D(0.0f, 0.0f);
	
			GridTopLeft =
				FVector2D(
					LeftPadding,
					TopPadding
				);
	
			break;
		}
	
		case ECharacterSelectGridPlacement::TopRight:
		{
			PlacementAnchor =
				FVector2D(1.0f, 0.0f);
	
			GridTopLeft =
				FVector2D(
					-RightPadding - GridSize.X,
					TopPadding
				);
	
			break;
		}
	
		case ECharacterSelectGridPlacement::BottomLeft:
		{
			PlacementAnchor =
				FVector2D(0.0f, 1.0f);
	
			GridTopLeft =
				FVector2D(
					LeftPadding,
					-BottomPadding - GridSize.Y
				);
	
			break;
		}
	
		case ECharacterSelectGridPlacement::BottomRight:
		{
			PlacementAnchor =
				FVector2D(1.0f, 1.0f);
	
			GridTopLeft =
				FVector2D(
					-RightPadding - GridSize.X,
					-BottomPadding - GridSize.Y
				);
	
			break;
		}
	
		case ECharacterSelectGridPlacement::Custom:
		default:
		{
			PlacementAnchor =
				FVector2D(0.5f, 0.5f);
	
			GridTopLeft =
				FVector2D(
					-GridSize.X * 0.5f,
					-GridSize.Y * 0.5f
				);
	
			break;
		}
		}
	}

    FVector2D GetGridSize(int32 ItemCount) const
    {
        if (ItemCount <= 0) return FVector2D::ZeroVector;
        const int32 C = FMath::Max(1, Columns), R = GetRowCount(ItemCount);
        return FVector2D(C * SlotSize.X + (C - 1) * SlotSpacing.X, R * SlotSize.Y + (R - 1) * SlotSpacing.Y);
    }

	int32 GetRowCount(int32 ItemCount) const
	{
		return ItemCount > 0 ? FMath::Max(Rows, FMath::DivideAndRoundUp(ItemCount, FMath::Max(1, Columns))) : 0;
	}

	// Coordinates in slot pitches; translation and slot size do not affect neighbors.
	FVector2D GetSlotCoordinates(int32 Index, int32 ItemCount) const
	{
		const int32 SafeColumns = FMath::Max(1, Columns);
		const int32 Row = Index / SafeColumns;
		const int32 RowItems = FMath::Min(SafeColumns, ItemCount - Row * SafeColumns);
		double X = Index % SafeColumns;
		if (bCenterIncompleteLastRow && RowItems < SafeColumns)
		{
			X += (SafeColumns - RowItems) * 0.5;
		}
		if (bReverseHorizontal) X = SafeColumns - 1 - X;
		const int32 Y = bReverseVertical ? GetRowCount(ItemCount) - 1 - Row : Row;
		return FVector2D(X, Y);
	}
};
