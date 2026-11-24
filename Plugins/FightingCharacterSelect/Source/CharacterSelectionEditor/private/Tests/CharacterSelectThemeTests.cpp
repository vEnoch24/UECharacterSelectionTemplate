#if WITH_DEV_AUTOMATION_TESTS
#include "Misc/AutomationTest.h"
#include "CharacterSelectThemeOperations.h"
#include "Data/CharacterSelectTheme.h"
#include "Data/CharacterSelectLayoutData.h"
#include "UI/CharacterSelectWidget.h"
#include "UI/CharacterSelectGridWidget.h"
#include "UI/CharacterSelectSlotWidget.h"
#include "UObject/UnrealType.h"
#include "Blueprint/WidgetTree.h"
#include "Editor.h"
#include "ScopedTransaction.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCharacterThemeTest, "CharacterSelection.Editor.ThemeSnapshots", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FCharacterThemeTest::RunTest(const FString& Parameters)
{
	UClass* Class = LoadClass<UCharacterSelectWidget>(nullptr, TEXT("/Game/CharacterSelect/UI/Core/WBP_CharacterSelect.WBP_CharacterSelect_C"));
	if (!TestNotNull(TEXT("Screen class loads"), Class)) return false;
	auto MakeTargets = [Class]()
	{
		FCharacterSelectThemeTargets T;
		T.Screen = NewObject<UCharacterSelectWidget>(GetTransientPackage(), Class);
		T.Screen->Initialize();
		T.Layout = NewObject<UCharacterSelectLayoutData>();
		T.Screen->WidgetTree->ForEachWidget([&T](UWidget* Widget)
		{
			if (auto* Grid = Cast<UCharacterSelectGridWidget>(Widget)) T.Grid = Grid;
			if (auto* Preview = Cast<UCharacterSelectPreviewWidget>(Widget)) T.Previews.Add(Preview);
		});
		return T;
	};
	auto Source = MakeTargets(); auto Destination = MakeTargets();
	if (!TestNotNull(TEXT("Source grid"), Source.Grid) || !TestNotNull(TEXT("Destination grid"), Destination.Grid) || !TestTrue(TEXT("Two preview targets"), Source.Previews.Num() == 2)) return false;
	Source.Grid->GridLayout.Columns = 4;
	Source.Grid->GridLayout.Placement = ECharacterSelectGridPlacement::TopLeft;
	Source.Screen->PromptAppearance.TextColor = FLinearColor::Red;
	Source.Screen->ControlSettings.Confirm = {EKeys::J};
	Source.Screen->SoundSettings.Volume = 0.25f;
	const float OriginalVolume = Destination.Screen->SoundSettings.Volume;
	Source.Screen->PlayerConfigs[0].DisplayName = FText::FromString(TEXT("Preset player"));
	Source.Previews[0]->AnimationStyles.Add(TEXT("Instant"), FCharacterPreviewAnimationStyle());
	Source.Previews[0]->AnimationStyle = TEXT("Instant");
    auto* SlotClassProperty = FindFProperty<FClassProperty>(Source.Grid->GetClass(), TEXT("SlotWidgetClass"));
    auto* SlotClass = SlotClassProperty ? Cast<UClass>(SlotClassProperty->GetObjectPropertyValue_InContainer(Source.Grid)) : nullptr;
    if (!TestNotNull(TEXT("Slot class for isolated snapshots"), SlotClass)) return false;
    Source.Slot = NewObject<UCharacterSelectSlotWidget>(GetTransientPackage(), SlotClass);
    Destination.Slot = NewObject<UCharacterSelectSlotWidget>(GetTransientPackage(), SlotClass);
    Source.Slot->SlotAnimationSettings = FCharacterSlotAnimationSettings::MakePreset(ECharacterSlotAnimationPreset::Lift);
    const auto OriginalSlotMode = Destination.Slot->SlotAnimationSettings.Mode;
    Source.Previews[0]->PreviewAnimationSettings = FCharacterPreviewAnimationSettings::MakePreset(ECharacterPreviewPreset::Slide);
    Source.Previews[0]->DefaultPresentation2D.Offset = FVector2D(37, 12);
    auto* Shared = NewObject<UCharacterSelectSequenceAsset>();
    Shared->Sequence = FCharacterSelectSequence::MakePreset(1);
    Source.Previews[0]->SharedSequence = Shared;
    Source.Previews[0]->bOverrideSequence = false;
	auto* Theme = NewObject<UCharacterSelectTheme>();
	FString Error;
	TestFalse(TEXT("Uncaptured theme cannot apply"), CharacterSelectThemeOperations::Apply(Theme, Destination, Error));
	TestTrue(TEXT("Capture existing configuration"), CharacterSelectThemeOperations::Capture(Theme, Source, Error));
	TestEqual(TEXT("Both previews captured"), Theme->Previews.Num(), 2);
	auto* Copy = DuplicateObject<UCharacterSelectTheme>(Theme, GetTransientPackage());
	TestEqual(TEXT("Asset duplication preserves grid"), Copy->GridLayout.Columns, 4);
	TestEqual(TEXT("Asset duplication preserves named styles"), Copy->Previews.Num(), 2);
	const TArray<FKey> OriginalConfirm = Destination.Screen->ControlSettings.Confirm;
	TestTrue(TEXT("Apply visual sections"), CharacterSelectThemeOperations::Apply(Copy, Destination, Error));
	TestEqual(TEXT("Grid columns applied"), Destination.Grid->GridLayout.Columns, 4);
	TestEqual(TEXT("Navigation dimensions synchronized"), Destination.Layout->Columns, 4);
	TestTrue(TEXT("Prompt colors applied"), Destination.Screen->PromptAppearance.TextColor == FLinearColor::Red);
	TestTrue(TEXT("Controls preserved without opt-in"), Destination.Screen->ControlSettings.Confirm == OriginalConfirm);
	TestTrue(TEXT("Players preserved without opt-in"), Destination.Screen->PlayerConfigs[0].DisplayName.ToString() != TEXT("Preset player"));
	TestEqual(TEXT("Feedback preserved without opt-in"), Destination.Screen->SoundSettings.Volume, OriginalVolume);
    TestTrue(TEXT("Native motion captured and applied"), Destination.Previews[0]->PreviewAnimationSettings.Mode == ECharacterPreviewAnimationMode::Configured);
    TestTrue(TEXT("2D framing captured and applied"), Destination.Previews[0]->DefaultPresentation2D.Offset == FVector2D(37,12));
    TestTrue(TEXT("Resolved sequence copied by value"), Destination.Previews[0]->bOverrideSequence && Destination.Previews[0]->SharedSequence == nullptr && Destination.Previews[0]->Sequence.EntranceOrder == ECharacterSequenceOrder::PortraitFirst);
    TestEqual(TEXT("Slot defaults remain unchanged without opt-in"), Destination.Slot->SlotAnimationSettings.Mode, OriginalSlotMode);
    Copy->bApplySlotMotion = true;
    TestTrue(TEXT("Slot motion applies with opt-in"), CharacterSelectThemeOperations::Apply(Copy, Destination, Error));
    TestEqual(TEXT("Slot focus motion transferred"), Destination.Slot->SlotAnimationSettings.Focused.Translation, Source.Slot->SlotAnimationSettings.Focused.Translation);
    const auto OriginalGap = Destination.Previews[0]->Sequence.GroupGap;
    Copy->Previews.FindChecked(Source.Previews[0]->GetFName()).Sequence.GroupGap = -1.f;
    Copy->GridLayout.Columns = 11;
    TestFalse(TEXT("Invalid presentation rejects entire snapshot"), CharacterSelectThemeOperations::Apply(Copy, Destination, Error));
    TestEqual(TEXT("Invalid presentation does not partly change layout"), Destination.Grid->GridLayout.Columns, 4);
    Copy->Previews.FindChecked(Source.Previews[0]->GetFName()).Sequence.GroupGap = OriginalGap;
    Copy->GridLayout.Columns = 4;
    auto* Legacy = DuplicateObject<UCharacterSelectTheme>(Copy, GetTransientPackage());
    Legacy->bApplySlotMotion = false;
    for (auto& Pair : Legacy->Previews) Pair.Value.bHasPresentation = false;
    Destination.Previews[0]->DefaultPresentation2D.Offset = FVector2D(5, 6);
    TestTrue(TEXT("Old snapshot applies without new presentation data"), CharacterSelectThemeOperations::Apply(Legacy, Destination, Error));
    TestEqual(TEXT("Old snapshot preserves new presentation defaults"), Destination.Previews[0]->DefaultPresentation2D.Offset, FVector2D(5,6));
	Copy->bApplyFeedback = true;
	Copy->bApplyControls = true; Copy->bApplyPlayers = true;
	TestTrue(TEXT("Apply opted-in sections"), CharacterSelectThemeOperations::Apply(Copy, Destination, Error));
	TestEqual(TEXT("Opted-in feedback applied"), Destination.Screen->SoundSettings.Volume, 0.25f);
	TestTrue(TEXT("Custom controls applied"), Destination.Screen->ControlSettings.Confirm == TArray<FKey>({EKeys::J}));
	TestEqual(TEXT("Player settings applied"), Destination.Screen->PlayerConfigs[0].DisplayName.ToString(), FString(TEXT("Preset player")));
	Copy->GridLayout.Columns = 9;
    Copy->Previews.FindChecked(Source.Previews[0]->GetFName()).Artwork.Offset = FVector2D(81, 9);
	Copy->Previews.FindChecked(Source.Previews[0]->GetFName()).Styles.FindChecked(TEXT("Instant")).InAnimationName = TEXT("MissingAnimation");
	TestFalse(TEXT("Missing animation rejects entire apply"), CharacterSelectThemeOperations::Apply(Copy, Destination, Error));
	TestEqual(TEXT("Failed preflight leaves grid untouched"), Destination.Grid->GridLayout.Columns, 4);
	Copy->bApplyPreviewAnimations = false;
	{
		const FScopedTransaction Transaction(FText::FromString(TEXT("Test apply theme")));
		TestTrue(TEXT("Skip incompatible preview section"), CharacterSelectThemeOperations::Apply(Copy, Destination, Error));
	}
	TestEqual(TEXT("Presentation changed in transaction"), Destination.Previews[0]->DefaultPresentation2D.Offset, FVector2D(81,9));
	TestEqual(TEXT("Selected layout applied"), Destination.Grid->GridLayout.Columns, 9);
	GEditor->UndoTransaction();
	TestEqual(TEXT("Undo restores presentation"), Destination.Previews[0]->DefaultPresentation2D.Offset, FVector2D(37,12));
	TestEqual(TEXT("Undo restores grid"), Destination.Grid->GridLayout.Columns, 4);
	TestEqual(TEXT("Undo restores layout asset"), Destination.Layout->Columns, 4);
	// All edited objects are transient instances. No project assets are changed or saved.
	return true;
}
#endif
