#include "CharacterSelectBasicStarter.h"
#include "CharacterSelectSetup.h"
#include "WidgetBlueprint.h"
#include "Blueprint/WidgetTree.h"
#include "Blueprint/WidgetBlueprintGeneratedClass.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/Border.h"
#include "Components/Image.h"
#include "Components/NamedSlot.h"
#include "Components/TextBlock.h"
#include "UI/CharacterSelectWidget.h"
#include "UI/CharacterSelectGridWidget.h"
#include "UI/CharacterSelectSlotWidget.h"
#include "UI/CharacterSelectPreviewWidget.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "Kismet2/KismetEditorUtilities.h"
#include "Kismet2/BlueprintEditorUtils.h"
#include "UObject/UnrealType.h"

namespace
{
    UWidgetBlueprint* NewWidget(const UCharacterSelectSetupOptions* O, const FString& Suffix, UClass* Parent,
        FCharacterSelectSetupResult& R)
    {
        const FString Name = TEXT("WBP_") + O->SetupName + Suffix;
        // CreateBlueprint compiles immediately. Start with an unbound base until
        // the tree exists, then compile against the actual native parent.
        auto* BP = Cast<UWidgetBlueprint>(FKismetEditorUtilities::CreateBlueprint(UUserWidget::StaticClass(),
            CreatePackage(*(O->DestinationFolder / Name)), *Name, BPTYPE_Normal,
            UWidgetBlueprint::StaticClass(), UWidgetBlueprintGeneratedClass::StaticClass()));
        if (BP)
        {
            BP->ParentClass = Parent;
            R.CreatedAssets.Add(BP);
            FAssetRegistryModule::AssetCreated(BP);
            BP->MarkPackageDirty();
            BP->WidgetTree->RootWidget = BP->WidgetTree->ConstructWidget<UCanvasPanel>(UCanvasPanel::StaticClass(), TEXT("Root"));
        }
        return BP;
    }
    UWidget* Add(UWidgetBlueprint* BP, UClass* Class, FName Name, FAnchors Anchors, FMargin Offsets = FMargin(0))
    {
        UWidget* W = BP->WidgetTree->ConstructWidget<UWidget>(Class, Name);
        auto* Slot = Cast<UCanvasPanel>(BP->WidgetTree->RootWidget)->AddChildToCanvas(W);
        Slot->SetAnchors(Anchors);
        Slot->SetOffsets(Offsets);
        return W;
    }
    bool Compile(UWidgetBlueprint* BP, FString& Error)
    {
        if (!BP) { Error = TEXT("Could not create a basic widget Blueprint."); return false; }
        FBlueprintEditorUtils::MarkBlueprintAsStructurallyModified(BP);
        FKismetEditorUtilities::CompileBlueprint(BP);
        if (BP->Status == BS_Error || !BP->GeneratedClass)
        { Error = TEXT("Basic widget compilation failed: ") + BP->GetName(); return false; }
        return true;
    }
}

UWidgetBlueprint* CreateCharacterSelectBasicStarter(const UCharacterSelectSetupOptions* O,
    FCharacterSelectSetupResult& R, FString& Error)
{
    auto* SlotBP = NewWidget(O, TEXT("_Slot"), UCharacterSelectSlotWidget::StaticClass(), R);
    if (!SlotBP) return nullptr;
    auto* Background = Cast<UBorder>(Add(SlotBP, UBorder::StaticClass(), TEXT("Background"), FAnchors(0,0,1,1)));
    Background->SetBrushColor(FLinearColor(.08f,.08f,.1f,1));
    Add(SlotBP, UImage::StaticClass(), TEXT("Portrait"), FAnchors(0, 0, 1, 1), FMargin(6));
    // Thin, separately positioned rails keep both players visible without obscuring the portrait.
    auto Rail = [&](FName Name, FLinearColor Color, FAnchors Anchors, FMargin Offsets)
    {
        auto* Border = Cast<UBorder>(Add(SlotBP, UBorder::StaticClass(), Name, Anchors, Offsets));
        Border->SetBrushColor(Color);
        Border->SetVisibility(ESlateVisibility::Collapsed);
    };
    Rail(TEXT("Player1FocusBorder"), FLinearColor(0.05f, .35f, 1), FAnchors(0,0,0,1), FMargin(0,0,5,0));
    Rail(TEXT("Player2FocusBorder"), FLinearColor(0, 1, .3f), FAnchors(1,0,1,1), FMargin(-5,0,5,0));
    Rail(TEXT("Player1SelectedBorder"), FLinearColor(0.05f, .35f, 1), FAnchors(0,0,1,0), FMargin(0,0,0,5));
    Rail(TEXT("Player2SelectedBorder"), FLinearColor(0, 1, .3f), FAnchors(0,1,1,1), FMargin(0,-5,0,5));
    Rail(TEXT("LockOverlay"), FLinearColor(0,0,0,.65f), FAnchors(0,0,1,1), FMargin(0));
    if (!Compile(SlotBP, Error)) return nullptr;

    auto* PreviewBP = NewWidget(O, TEXT("_Preview"), UCharacterSelectPreviewWidget::StaticClass(), R);
    if (!PreviewBP) return nullptr;
    Add(PreviewBP, UImage::StaticClass(), TEXT("LargePortrait"), FAnchors(0,0,1,.8f));
    Add(PreviewBP, UNamedSlot::StaticClass(), TEXT("CustomPresentationSlot"), FAnchors(0,0,1,.8f))->SetVisibility(ESlateVisibility::Collapsed);
    auto* Name = Cast<UTextBlock>(Add(PreviewBP, UTextBlock::StaticClass(), TEXT("CharacterName"), FAnchors(0,.8f,1,.9f)));
    Name->SetText(NSLOCTEXT("CharacterSelect", "BasicName", "Character"));
    auto* Subtitle = Cast<UTextBlock>(Add(PreviewBP, UTextBlock::StaticClass(), TEXT("Subtitle"), FAnchors(0,.9f,1,1)));
    Subtitle->SetText(FText::GetEmpty());
    if (!Compile(PreviewBP, Error)) return nullptr;

    if (O->Presentation != ECharacterSelectStarterPresentation::Basic)
    {
        auto* SlotDefaults = CastChecked<UCharacterSelectSlotWidget>(SlotBP->GeneratedClass->GetDefaultObject());
        SlotDefaults->SlotAnimationSettings = FCharacterSlotAnimationSettings::MakePreset(ECharacterSlotAnimationPreset::Lift);
        auto* PreviewDefaults = CastChecked<UCharacterSelectPreviewWidget>(PreviewBP->GeneratedClass->GetDefaultObject());
        PreviewDefaults->PreviewAnimationSettings = FCharacterPreviewAnimationSettings::MakePreset(ECharacterPreviewPreset::Fade);
        PreviewDefaults->bOverrideSequence = true;
        PreviewDefaults->Sequence = FCharacterSelectSequence::MakePreset(1);
        PreviewDefaults->bDefault3DPreview = O->Presentation == ECharacterSelectStarterPresentation::Character3D;
        if (PreviewDefaults->bDefault3DPreview) PreviewDefaults->DefaultPresentation3D.SkeletalMesh = O->PreviewMesh;
        FBlueprintEditorUtils::MarkBlueprintAsModified(SlotBP);
        FBlueprintEditorUtils::MarkBlueprintAsModified(PreviewBP);
        if (!Compile(SlotBP, Error) || !Compile(PreviewBP, Error)) return nullptr;
    }

    auto* GridBP = NewWidget(O, TEXT("_Grid"), UCharacterSelectGridWidget::StaticClass(), R);
    if (!GridBP) return nullptr;
    GridBP->WidgetTree->RootWidget->Rename(TEXT("SlotContainer"), GridBP->WidgetTree);
    if (!Compile(GridBP, Error)) return nullptr;
    auto* SlotClass = FindFProperty<FClassProperty>(UCharacterSelectGridWidget::StaticClass(), TEXT("SlotWidgetClass"));
    if (!SlotClass) { Error = TEXT("Grid slot-class property is missing."); return nullptr; }
    SlotClass->SetObjectPropertyValue_InContainer(GridBP->GeneratedClass->GetDefaultObject(), SlotBP->GeneratedClass);
    auto* GridDefaults = Cast<UCharacterSelectGridWidget>(GridBP->GeneratedClass->GetDefaultObject());
    GridDefaults->GridLayout.Columns = 4;
    GridDefaults->GridLayout.Placement = ECharacterSelectGridPlacement::Center;
    GridDefaults->GridLayout.Padding = FMargin(20);
    FBlueprintEditorUtils::MarkBlueprintAsModified(GridBP);
    if (!Compile(GridBP, Error)) return nullptr;

    auto* ScreenBP = NewWidget(O, TEXT(""), UCharacterSelectWidget::StaticClass(), R);
    if (!ScreenBP) return nullptr;
    Add(ScreenBP, PreviewBP->GeneratedClass, TEXT("Player1Preview"), FAnchors(.02f,.18f,.25f,.95f));
    Add(ScreenBP, PreviewBP->GeneratedClass, TEXT("Player2Preview"), FAnchors(.75f,.18f,.98f,.95f));
    Add(ScreenBP, GridBP->GeneratedClass, TEXT("CharacterGrid"), FAnchors(.25f,.2f,.75f,.9f));
    Add(ScreenBP, UTextBlock::StaticClass(), TEXT("FocusedCharacterName"), FAnchors())->SetVisibility(ESlateVisibility::Collapsed);
    Add(ScreenBP, UImage::StaticClass(), TEXT("FocusedCharacterImage"), FAnchors())->SetVisibility(ESlateVisibility::Collapsed);
    return Compile(ScreenBP, Error) ? ScreenBP : nullptr;
}
