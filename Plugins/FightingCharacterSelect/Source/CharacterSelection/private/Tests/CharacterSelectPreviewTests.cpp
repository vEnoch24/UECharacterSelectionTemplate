#if WITH_DEV_AUTOMATION_TESTS
#include "Misc/AutomationTest.h"
#include "UObject/Package.h"
#include "UI/CharacterSelectWidget.h"
#include "UI/CharacterSelectPreviewWidget.h"
#include "Core/CharacterSelectJoinSession.h"
#include "Core/CharacterSelectionController.h"
#include "Data/CharacterSelectData.h"
#include "Data/CharacterSelectRegistry.h"
#include "Data/CharacterSelectLayoutData.h"
#include "Components/Widget.h"
#include "Animation/WidgetAnimation.h"
#include "UObject/UnrealType.h"

namespace CharacterPreviewTests
{
	template<typename T> T* GetBound(UObject* Object, FName Name)
	{
		const FObjectPropertyBase* Property = FindFProperty<FObjectPropertyBase>(Object->GetClass(), Name);
		return Property ? Cast<T>(Property->GetObjectPropertyValue_InContainer(Object)) : nullptr;
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCharacterPreviewStyleTest, "CharacterSelection.Preview.StylesAndOriginalBindings", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FCharacterPreviewStyleTest::RunTest(const FString& Parameters)
{
	UClass* Class = LoadClass<UCharacterSelectPreviewWidget>(nullptr, TEXT("/Game/CharacterSelect/UI/Preview/WBP_CharacterSelectPreview.WBP_CharacterSelectPreview_C"));
	if (!TestNotNull(TEXT("Existing preview Blueprint loads"), Class)) return false;
	auto* Preview = NewObject<UCharacterSelectPreviewWidget>(GetTransientPackage(), Class);
	Preview->Initialize();
	TestNotNull(TEXT("Original outgoing animation is preserved"), CharacterPreviewTests::GetBound<UWidgetAnimation>(Preview, TEXT("CharacterChangeOutAnimation")));
	TestNotNull(TEXT("Original incoming animation is preserved"), CharacterPreviewTests::GetBound<UWidgetAnimation>(Preview, TEXT("CharacterChangeInAnimation")));
	TestTrue(TEXT("Default style keeps original pair"), Preview->AnimationStyle.IsNone());
	TestFalse(TEXT("Unknown style rejected"), Preview->SetAnimationStyle(TEXT("Missing")));
	Preview->AnimationStyles.Add(TEXT("Instant"), FCharacterPreviewAnimationStyle());
	TestTrue(TEXT("Named style selected"), Preview->SetAnimationStyle(TEXT("Instant")));
	auto* A = NewObject<UCharacterSelectData>();
	auto* B = NewObject<UCharacterSelectData>();
	Preview->SetCharacter(A);
	TestEqual(TEXT("Instant style updates character"), Preview->GetCharacter(), A);
	Preview->SetCharacter(B);
	TestEqual(TEXT("Next request does not get stuck"), Preview->GetCharacter(), B);
	Preview->ClearPreview();
	TestNull(TEXT("Clear removes character"), Preview->GetCharacter());
	Preview->SetCharacter(A);
	TestEqual(TEXT("Can show character after clear"), Preview->GetCharacter(), A);
	TestTrue(TEXT("Can restore original style"), Preview->SetAnimationStyle(NAME_None));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCharacterDuplicatePreviewTest, "CharacterSelection.Preview.NoDuplicatePortrait", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FCharacterDuplicatePreviewTest::RunTest(const FString& Parameters)
{
	UClass* Class = LoadClass<UCharacterSelectWidget>(nullptr, TEXT("/Game/CharacterSelect/UI/Core/WBP_CharacterSelect.WBP_CharacterSelect_C"));
	if (!TestNotNull(TEXT("Existing selection Blueprint loads"), Class)) return false;
	auto* Screen = NewObject<UCharacterSelectWidget>(GetTransientPackage(), Class);
	Screen->Initialize();
	auto* P1 = CharacterPreviewTests::GetBound<UCharacterSelectPreviewWidget>(Screen, TEXT("Player1Preview"));
	auto* LegacyImage = CharacterPreviewTests::GetBound<UWidget>(Screen, TEXT("FocusedCharacterImage"));
	auto* LegacyName = CharacterPreviewTests::GetBound<UWidget>(Screen, TEXT("FocusedCharacterName"));
	if (!TestNotNull(TEXT("P1 preview is bound"), P1) || !TestNotNull(TEXT("Legacy image is bound"), LegacyImage) || !TestNotNull(TEXT("Legacy name is bound"), LegacyName)) return false;
	P1->AnimationStyles.Add(TEXT("Instant"), FCharacterPreviewAnimationStyle());
	P1->SetAnimationStyle(TEXT("Instant"));
	auto* Registry = NewObject<UCharacterSelectRegistry>();
	Registry->Characters.Add(NewObject<UCharacterSelectData>());
	auto* Layout = NewObject<UCharacterSelectLayoutData>();
	auto* Controller1 = NewObject<UCharacterSelectionController>();
	auto* Controller2 = NewObject<UCharacterSelectionController>();
	Controller1->Initialize(Registry, Layout, 0, 0);
	Controller2->Initialize(Registry, Layout, 1, 0);
	auto* Session = NewObject<UCharacterSelectJoinSession>(Screen);
	Session->Initialize({Controller1, Controller2});
	auto* SessionProperty = FindFProperty<FObjectPropertyBase>(Screen->GetClass(), TEXT("JoinSession"));
	if (!TestNotNull(TEXT("Join session binding"), SessionProperty)) return false;
	SessionProperty->SetObjectPropertyValue_InContainer(Screen, Session);
	Session->HandleInput(ECharacterSelectDeviceType::Keyboard, 0, ECharacterSelectInputAction::Confirm, true);
	Screen->ProcessEvent(Screen->FindFunctionChecked(TEXT("HandleJoinStateChanged")), nullptr);
	TestEqual(TEXT("Old portrait remains collapsed after joining"), LegacyImage->GetVisibility(), ESlateVisibility::Collapsed);
	TestEqual(TEXT("Old name remains collapsed after joining"), LegacyName->GetVisibility(), ESlateVisibility::Collapsed);
	TestEqual(TEXT("Actual P1 preview is visible"), P1->GetVisibility(), ESlateVisibility::HitTestInvisible);
	Screen->RemoveFromParent();
	return true;
}
#endif
