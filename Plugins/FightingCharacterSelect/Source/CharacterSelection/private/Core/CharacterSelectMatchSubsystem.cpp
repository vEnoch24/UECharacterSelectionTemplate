#include "Core/CharacterSelectMatchSubsystem.h"
#include "Core/CharacterSelectionController.h"
#include "Data/CharacterSelectData.h"
#include "Data/CharacterSelectRegistry.h"
#include "UI/CharacterSelectWidget.h"

bool UCharacterSelectMatchSubsystem::StoreFromScreen(UCharacterSelectWidget* Screen, FString& Error)
{
	return StoreFromSession(Screen ? Screen->GetJoinSession() : nullptr, Screen ? Screen->GetRegistry() : nullptr, Error);
}

bool UCharacterSelectMatchSubsystem::StoreFromSession(UCharacterSelectJoinSession* Session, const UCharacterSelectRegistry* Registry, FString& Error)
{
	ClearSelection(); Error.Reset();
	if (!Session || !Registry || !Session->IsReady())
	{ Error = TEXT("Both selections must be confirmed and all human devices connected before storing a match."); return false; }
	FCharacterSelectMatchResult Result;
	Result.SelectionID = FGuid::NewGuid();
	for (int32 Index=0; Index<2; ++Index)
	{
		auto* Controller = Session->GetController(Index);
		auto* Character = Controller ? Controller->GetSelectedCharacter() : nullptr;
		if (!Character || Character->Availability != ECharacterAvailability::Available || Character->CharacterID.IsNone())
		{ Error = TEXT("Selected characters must be available and have non-empty Character IDs."); return false; }
		int32 Matches = 0;
		bool bInRegistry = false;
		for (auto* Entry : Registry->GetAllCharacters()) if (Entry)
		{ Matches += Entry->CharacterID == Character->CharacterID; bInRegistry |= Entry == Character; }
		if (Matches != 1 || !bInRegistry)
		{ Error = TEXT("Each selected Character ID must identify exactly one asset in the registry: ") + Character->CharacterID.ToString(); return false; }
		auto& Player = Result.Players.AddDefaulted_GetRef();
		Player.PlayerIndex = Index; Player.CharacterID = Character->CharacterID;
		Player.CharacterAsset = Character; Player.CharacterClass = Character->CharacterClass;
		Player.bIsCPU = Index == 1 && Session->IsCPUOpponent();
		Player.Device = Player.bIsCPU ? FCharacterSelectDeviceAssignment() : Session->GetAssignment(Index);
		Player.CostumeIndex = Controller->GetSelectionState().CostumeIndex;
		Player.ColorIndex = Controller->GetSelectionState().ColorIndex;
	}
	Pending = MoveTemp(Result); bHasSelection = true;
	return true;
}

bool UCharacterSelectMatchSubsystem::GetSelection(FCharacterSelectMatchResult& Result) const
{
	Result = bHasSelection ? Pending : FCharacterSelectMatchResult(); return bHasSelection;
}

bool UCharacterSelectMatchSubsystem::ConsumeSelection(FCharacterSelectMatchResult& Result)
{
	const bool bFound = GetSelection(Result); ClearSelection(); return bFound;
}

void UCharacterSelectMatchSubsystem::ClearSelection()
{
	Pending = FCharacterSelectMatchResult(); bHasSelection = false;
}
