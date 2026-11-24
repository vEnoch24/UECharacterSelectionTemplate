#include "Data/CharacterSelectLayoutData.h"
#include "Data/CharacterSelectRegistry.h"
#include "Data/CharacterSelectData.h"

TArray<FString> UCharacterSelectLayoutData::GetDirectionalLinkIssues(const UCharacterSelectRegistry* Registry) const
{
	TArray<FString> Issues;
	if (!bUseDirectionalLinks) return Issues;
	if (!Registry) { Issues.Add(TEXT("Select a registry to validate directional links.")); return Issues; }
	TMap<FName, int32> Counts;
	TMap<FName, UCharacterSelectData*> Characters;
	for (auto* C : Registry->GetAllCharacters()) if (C) { ++Counts.FindOrAdd(C->CharacterID); Characters.Add(C->CharacterID,C); }
	for (const auto& Pair : DirectionalLinks)
	{
		if (Pair.Key.IsNone() || Counts.FindRef(Pair.Key) != 1)
			Issues.Add(TEXT("Directional links: source ID must identify exactly one character: ") + Pair.Key.ToString());
		const FCharacterSelectDirectionalLink* Links[] = {&Pair.Value.Up,&Pair.Value.Down,&Pair.Value.Left,&Pair.Value.Right};
		const TCHAR* Directions[] = {TEXT("Up"),TEXT("Down"),TEXT("Left"),TEXT("Right")};
		for (int32 I=0; I<4; ++I)
		{
			const auto& Link = *Links[I];
			if (Link.Rule != ECharacterSelectLinkRule::Character) continue;
			const FString Prefix = TEXT("Directional link ") + Pair.Key.ToString() + TEXT(" / ") + Directions[I] + TEXT(": ");
			if (Link.TargetCharacterID.IsNone() || Counts.FindRef(Link.TargetCharacterID) != 1)
				Issues.Add(Prefix + TEXT("target ID must identify exactly one character: ") + Link.TargetCharacterID.ToString());
			else
			{
				const auto Availability = Characters.FindChecked(Link.TargetCharacterID)->Availability;
				if (Availability != ECharacterAvailability::Available && !(Availability == ECharacterAvailability::Locked && bAllowLockedSlots))
					Issues.Add(Prefix + TEXT("target is excluded by availability/locked-slot settings; this move will stop."));
			}
		}
	}
	return Issues;
}
