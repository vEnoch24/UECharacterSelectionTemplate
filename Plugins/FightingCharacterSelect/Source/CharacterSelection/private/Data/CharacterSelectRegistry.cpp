// Fill out your copyright notice in the Description page of Project Settings.

#include "Data/CharacterSelectRegistry.h"
#include "Data/CharacterSelectData.h"
#if WITH_EDITOR
#include "Misc/DataValidation.h"
#endif


const TArray<UCharacterSelectData*>& UCharacterSelectRegistry::GetAllCharacters() const
{
	return ObjectPtrDecay(Characters);
}

UCharacterSelectData* UCharacterSelectRegistry::GetCharacterByID(FName CharacterID) const
{
	if (CharacterID.IsNone())
	{
		return nullptr;
	}

	for (UCharacterSelectData* Character : Characters)
	{
		if (!Character)
		{
			continue;
		}

		if (Character->CharacterID == CharacterID)
		{
			return Character;
		}
	}

	return nullptr;
}

TArray<UCharacterSelectData*> UCharacterSelectRegistry::GetCharactersByTag(FName Tag) const
{
	TArray<UCharacterSelectData*> Result;

	if (Tag.IsNone())
	{
		return Result;
	}

	for (UCharacterSelectData* Character : Characters)
	{
		if (!Character)
		{
			continue;
		}

		if (Character->Tags.Contains(Tag))
		{
			Result.Add(Character);
		}
	}

	return Result;
}

TArray<UCharacterSelectData*> UCharacterSelectRegistry::GetAvailableCharacters() const
{
	TArray<UCharacterSelectData*> Result;

	for (UCharacterSelectData* Character : Characters)
	{
		if (!Character)
		{
			continue;
		}

		if (Character->Availability ==
			ECharacterAvailability::Available)
		{
			Result.Add(Character);
		}
	}

	return Result;
}


TArray<UCharacterSelectData*> UCharacterSelectRegistry::GetCharactersSorted() const
{
	TArray<UCharacterSelectData*> Result;

	for (const TObjectPtr<UCharacterSelectData>& Character : Characters)
	{
		if (Character)
		{
			Result.Add(Character.Get());
		}
	}

	Result.Sort(
		[](const UCharacterSelectData& A,
			const UCharacterSelectData& B)
		{
			return A.SortPriority < B.SortPriority;
		});

	return Result;
}


#if WITH_EDITOR
EDataValidationResult UCharacterSelectRegistry::IsDataValid(
	FDataValidationContext& Context) const
{
	EDataValidationResult Result =
		EDataValidationResult::Valid;

	TSet<FName> CharacterIDs;

	for (const UCharacterSelectData* Character : Characters)
	{
		if (!Character)
		{
			Context.AddError(
				FText::FromString(
					TEXT("Registry contains a null character.")));

			Result = EDataValidationResult::Invalid;
			continue;
		}

		if (CharacterIDs.Contains(Character->CharacterID))
		{
			Context.AddError(
				FText::Format(
					FText::FromString(
						TEXT("Duplicate CharacterID: {0}")),
					FText::FromName(Character->CharacterID)));

			Result = EDataValidationResult::Invalid;
		}

		CharacterIDs.Add(Character->CharacterID);
	}

	return Result;
}

#endif