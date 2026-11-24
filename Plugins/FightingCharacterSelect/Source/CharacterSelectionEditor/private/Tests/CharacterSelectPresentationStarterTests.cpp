#if WITH_DEV_AUTOMATION_TESTS
#include "Misc/AutomationTest.h"
#include "CharacterSelectSetup.h"
#include "WidgetBlueprint.h"
#include "Blueprint/WidgetTree.h"
#include "UI/CharacterSelectPreviewWidget.h"
#include "UI/CharacterSelectGridWidget.h"
#include "UI/CharacterSelectSlotWidget.h"
#include "UI/CharacterSelectWidget.h"
#include "CharacterSelectThemeOperations.h"
#include "Data/CharacterSelectTheme.h"
#include "Data/CharacterSelectLayoutData.h"
#include "Engine/SkeletalMesh.h"
#include "UObject/StrongObjectPtr.h"
#include "UObject/UnrealType.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCharacterPresentationStarterTest, "CharacterSelection.Plugin.PresentationStarters", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FCharacterPresentationStarterTest::RunTest(const FString& Parameters)
{
    for (auto Mode : {ECharacterSelectStarterPresentation::Animated2D, ECharacterSelectStarterPresentation::Character3D})
    {
        TStrongObjectPtr<UCharacterSelectSetupOptions> Options(NewObject<UCharacterSelectSetupOptions>());
        Options->DestinationFolder = TEXT("/Game/__PresentationStarter_") + FGuid::NewGuid().ToString(EGuidFormats::Digits);
        Options->Presentation = Mode;
        FString Error;
        if (Mode == ECharacterSelectStarterPresentation::Character3D)
        {
            TestFalse(TEXT("Missing 3D mesh is rejected before creating assets"), CharacterSelectSetup::Validate(Options.Get(), Error));
            // Use an engine asset: Blueprint compilation deliberately drops transient mesh references.
            Options->PreviewMesh = LoadObject<USkeletalMesh>(nullptr, TEXT("/Engine/EngineMeshes/SkeletalCube.SkeletalCube"));
            if (!TestNotNull(TEXT("Engine skeletal mesh fixture"), Options->PreviewMesh.Get())) return false;
        }
        FCharacterSelectSetupResult Result;
        if (!TestTrue(TEXT("Presentation starter creates successfully: ") + Error, CharacterSelectSetup::Create(Options.Get(), Result, Error))) return false;
        TestEqual(TEXT("Starter generates seven editable assets"), Result.CreatedAssets.Num(), 7);
        for (auto* Asset : Result.CreatedAssets)
            if (auto* BP = Cast<UBlueprint>(Asset)) TestTrue(TEXT("Generated Blueprint compiles"), BP->Status == BS_UpToDate);
        TStrongObjectPtr<UCharacterSelectWidget> Screen(NewObject<UCharacterSelectWidget>(GetTransientPackage(), Result.Screen->GeneratedClass));
        Screen->Initialize();
        for (const TCHAR* Name : {TEXT("Player1Preview"), TEXT("Player2Preview")})
        {
            auto* Preview = Cast<UCharacterSelectPreviewWidget>(Screen->WidgetTree->FindWidget(Name));
            if (!TestNotNull(TEXT("Player preview exists"), Preview)) return false;
            TestTrue(TEXT("Starter has native motion"), Preview->PreviewAnimationSettings.Mode == ECharacterPreviewAnimationMode::Configured);
            TestTrue(TEXT("Starter has portrait-first sequence"), Preview->bOverrideSequence && Preview->Sequence.bEnabled && Preview->Sequence.EntranceOrder == ECharacterSequenceOrder::PortraitFirst);
            TestEqual(TEXT("3D mode follows starter choice"), Preview->bDefault3DPreview, Mode == ECharacterSelectStarterPresentation::Character3D);
            if (Preview->bDefault3DPreview) TestTrue(TEXT("Supplied mesh is retained"), Preview->DefaultPresentation3D.SkeletalMesh == Options->PreviewMesh);
            Preview->Initialize();
            if (!TestNotNull(TEXT("Preview tree initializes"), Preview->WidgetTree.Get())) return false;
            TestNotNull(TEXT("Custom presentation host is ready"), Preview->WidgetTree->FindWidget(TEXT("CustomPresentationSlot")));
        }
        auto* Grid = CastChecked<UCharacterSelectGridWidget>(Screen->WidgetTree->FindWidget(TEXT("CharacterGrid")));
        auto* Property = FindFProperty<FClassProperty>(Grid->GetClass(), TEXT("SlotWidgetClass"));
        auto* Class = CastChecked<UClass>(Property->GetObjectPropertyValue_InContainer(Grid));
        auto* Slot = CastChecked<UCharacterSelectSlotWidget>(Class->GetDefaultObject());
        TestTrue(TEXT("Starter slots use configured motion"), Slot->SlotAnimationSettings.Mode == ECharacterSlotAnimationMode::Configured);
        FCharacterSelectThemeTargets Source;
        Source.Screen = Screen.Get(); Source.Grid = Grid; Source.Layout = Result.Layout; Source.Slot = Slot;
        Screen->WidgetTree->ForEachWidget([&](UWidget* W) { if (auto* P = Cast<UCharacterSelectPreviewWidget>(W)) Source.Previews.Add(P); });
        TStrongObjectPtr<UCharacterSelectWidget> DestinationScreen(NewObject<UCharacterSelectWidget>(GetTransientPackage(), Result.Screen->GeneratedClass));
        DestinationScreen->Initialize();
        FCharacterSelectThemeTargets Destination;
        Destination.Screen = DestinationScreen.Get(); Destination.Layout = NewObject<UCharacterSelectLayoutData>();
        DestinationScreen->WidgetTree->ForEachWidget([&](UWidget* W)
        {
            if (auto* P = Cast<UCharacterSelectPreviewWidget>(W)) { P->Initialize(); Destination.Previews.Add(P); }
            if (auto* G = Cast<UCharacterSelectGridWidget>(W)) Destination.Grid = G;
        });
        auto* Theme = NewObject<UCharacterSelectTheme>();
        TestTrue(TEXT("Starter presentation can be captured"), CharacterSelectThemeOperations::Capture(Theme, Source, Error));
        for (auto* P : Destination.Previews) { P->bDefault3DPreview = false; P->DefaultPresentation3D.SkeletalMesh = nullptr; P->Sequence.bEnabled = false; }
        TestTrue(TEXT("Starter presentation can be reapplied: ") + Error, CharacterSelectThemeOperations::Apply(Theme, Destination, Error));
        for (auto* P : Destination.Previews)
        {
            TestEqual(TEXT("Theme transfers 2D/3D mode"), P->bDefault3DPreview, Mode == ECharacterSelectStarterPresentation::Character3D);
            TestTrue(TEXT("Theme transfers sequence"), P->Sequence.bEnabled);
            if (P->bDefault3DPreview) TestTrue(TEXT("Theme transfers mesh reference"), P->DefaultPresentation3D.SkeletalMesh == Options->PreviewMesh);
        }

    }
    return true;
}
#endif
