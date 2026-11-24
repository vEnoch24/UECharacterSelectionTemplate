#if WITH_DEV_AUTOMATION_TESTS
#include "Misc/AutomationTest.h"
#include "CharacterSelectSetup.h"
#include "WidgetBlueprint.h"
#include "Blueprint/WidgetTree.h"
#include "UI/CharacterSelectWidget.h"
#include "UI/CharacterSelectGridWidget.h"
#include "UI/CharacterSelectPreviewWidget.h"
#include "Data/CharacterSelectRegistry.h"
#include "UObject/StrongObjectPtr.h"
#include "UObject/UnrealType.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCharacterSelectPluginStarterTest, "CharacterSelection.Plugin.BasicStarter",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FCharacterSelectPluginStarterTest::RunTest(const FString& Parameters)
{
    TStrongObjectPtr<UCharacterSelectSetupOptions> Options(NewObject<UCharacterSelectSetupOptions>());
    Options->DestinationFolder = TEXT("/Game/__BasicStarter_") + FGuid::NewGuid().ToString(EGuidFormats::Digits);
    FCharacterSelectSetupResult Result;
    FString Error;
    const bool bCreated = CharacterSelectSetup::Create(Options.Get(), Result, Error);
    if (!TestTrue(*FString(TEXT("Create without any demo assets: ") + Error), bCreated)) return false;
    TestEqual(TEXT("Screen, three components, layout, roster and launcher"), Result.CreatedAssets.Num(), 7);
    TestTrue(TEXT("Roster starts empty"), Result.Roster->Characters.IsEmpty());
    for (UObject* Asset : Result.CreatedAssets)
    {
        TestTrue(TEXT("All outputs belong to requested folder"), Asset->GetOutermost()->GetName().StartsWith(Options->DestinationFolder));
        if (auto* BP = Cast<UBlueprint>(Asset)) TestTrue(TEXT("Blueprint compiled without warnings"), BP->GeneratedClass && BP->Status == BS_UpToDate);
    }
    TStrongObjectPtr<UCharacterSelectWidget> Screen(NewObject<UCharacterSelectWidget>(GetTransientPackage(), Result.Screen->GeneratedClass));
    TestTrue(TEXT("Generated screen initializes"), Screen->Initialize());
    auto Bound = [](UObject* Object, FName Name)
    {
        auto* Property = FindFProperty<FObjectPropertyBase>(Object->GetClass(), Name);
        return Property ? Property->GetObjectPropertyValue_InContainer(Object) : nullptr;
    };
    auto* Grid = Cast<UCharacterSelectGridWidget>(Screen->WidgetTree->FindWidget(TEXT("CharacterGrid")));
    TestTrue(TEXT("Native screen grid binding is populated"), Grid && Bound(Screen.Get(), TEXT("CharacterGrid")) == Grid);
    if (TestNotNull(TEXT("Grid bound"), Grid))
    {
        Grid->Initialize();
        TestNotNull(TEXT("Grid owns SlotContainer"), Grid->WidgetTree->FindWidget(TEXT("SlotContainer")));
        TestNotNull(TEXT("Native canvas binding is populated"), Bound(Grid, TEXT("SlotContainer")));
        auto* Property = FindFProperty<FClassProperty>(Grid->GetClass(), TEXT("SlotWidgetClass"));
        TestNotNull(TEXT("Grid uses generated slot class"), Property->GetObjectPropertyValue_InContainer(Grid));
    }
    for (FName Name : {FName(TEXT("Player1Preview")), FName(TEXT("Player2Preview"))})
    {
        auto* Preview = Cast<UCharacterSelectPreviewWidget>(Screen->WidgetTree->FindWidget(Name));
        if (TestNotNull(TEXT("Player preview exists"), Preview))
        {
            TestTrue(TEXT("Native player preview binding is populated"), Bound(Screen.Get(), Name) == Preview);
            Preview->Initialize();
            TestNotNull(TEXT("Preview portrait binding"), Preview->WidgetTree->FindWidget(TEXT("LargePortrait")));
            TestNotNull(TEXT("Preview name binding"), Preview->WidgetTree->FindWidget(TEXT("CharacterName")));
            TestNotNull(TEXT("Native preview portrait binding is populated"), Bound(Preview, TEXT("LargePortrait")));
            TestNotNull(TEXT("Generated preview binds custom presentation host"), Bound(Preview, TEXT("CustomPresentationSlot")));
        }
    }
    FCharacterSelectSetupResult Rejected;
    TestFalse(TEXT("Existing basic assets cannot be overwritten"), CharacterSelectSetup::Create(Options.Get(), Rejected, Error));
    TestTrue(TEXT("Collision creates no partial assets"), Rejected.CreatedAssets.IsEmpty());
    return true;
}
#endif
