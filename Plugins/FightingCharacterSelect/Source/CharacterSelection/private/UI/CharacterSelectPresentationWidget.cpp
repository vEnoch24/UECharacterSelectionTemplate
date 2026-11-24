#include "UI/CharacterSelectPresentationWidget.h"
#include "Data/CharacterSelectData.h"
void UCharacterSelectPresentationWidget::SetPresentation(UCharacterSelectData* InCharacter, int32 InPlayerIndex)
{
    PresentedCharacter = InCharacter;
    PresentationPlayerIndex = FMath::Clamp(InPlayerIndex, 0, 1);
    if (!IsDesignTime()) OnPresentationChanged(PresentedCharacter, PresentationPlayerIndex);
}
