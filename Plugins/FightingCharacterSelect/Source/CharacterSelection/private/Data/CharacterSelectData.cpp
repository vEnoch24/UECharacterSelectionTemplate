// Fill out your copyright notice in the Description page of Project Settings.


#include "Data/CharacterSelectData.h"
#if WITH_EDITOR
#include "Misc/DataValidation.h"
#endif


#if WITH_EDITOR
EDataValidationResult UCharacterSelectData::IsDataValid(
	FDataValidationContext& Context) const
{
	EDataValidationResult Result =
		EDataValidationResult::Valid;

	if (CharacterID.IsNone())
	{
		Context.AddError(
			FText::FromString(
				TEXT("CharacterID must be set.")));

		Result = EDataValidationResult::Invalid;
	}

	if (DisplayName.IsEmpty())
	{
		Context.AddError(
			FText::FromString(
				TEXT("DisplayName must be set.")));

		Result = EDataValidationResult::Invalid;
	}

    if (bOverridePresentation2D) for (const FString& Issue : Presentation2D.GetIssues())
    {
        Context.AddError(FText::FromString(Issue));
        Result = EDataValidationResult::Invalid;
    }
    if (bOverridePresentation3D) for (const FString& Issue : Presentation3D.GetIssues())
    {
        Context.AddError(FText::FromString(Issue));
        Result = EDataValidationResult::Invalid;
    }
	return Result;
}
#endif