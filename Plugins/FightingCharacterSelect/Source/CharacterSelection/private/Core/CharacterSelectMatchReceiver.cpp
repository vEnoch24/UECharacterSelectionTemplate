#include "Core/CharacterSelectMatchReceiver.h"
#include "Engine/GameInstance.h"

void ACharacterSelectMatchReceiver::BeginPlay()
{
	Super::BeginPlay(); ReadMatchSelection();
}

bool ACharacterSelectMatchReceiver::ReadMatchSelection()
{
	auto* Store = GetGameInstance() ? GetGameInstance()->GetSubsystem<UCharacterSelectMatchSubsystem>() : nullptr;
	MatchSelection = FCharacterSelectMatchResult();
	bReceivedSelection = Store && (bConsumeOnRead ? Store->ConsumeSelection(MatchSelection) : Store->GetSelection(MatchSelection));
	if (bReceivedSelection)
	{
		for (const auto& Player : MatchSelection.Players)
			UE_LOG(LogTemp, Display, TEXT("Match handoff P%d: %s (%s), device %d"), Player.PlayerIndex+1, *Player.CharacterID.ToString(), Player.bIsCPU ? TEXT("CPU") : TEXT("Human"), Player.Device.DeviceId);
		OnMatchSelectionReceived(MatchSelection);
	}
	return bReceivedSelection;
}
