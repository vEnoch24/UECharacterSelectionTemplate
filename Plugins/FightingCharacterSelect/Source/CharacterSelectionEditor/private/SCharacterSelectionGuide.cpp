#include "SCharacterSelectionGuide.h"
#include "Interfaces/IPluginManager.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "Misc/ConfigCacheIni.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"
#include "Brushes/SlateDynamicImageBrush.h"
#include "Framework/Application/SlateApplication.h"
#include "Rendering/SlateRenderer.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Input/SSearchBox.h"
#include "Widgets/Images/SImage.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SScaleBox.h"
#include "Widgets/Layout/SScrollBox.h"
#include "Widgets/Layout/SSplitter.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Text/STextBlock.h"
#include "Styling/CoreStyle.h"

namespace StudioGuide
{
    const TCHAR* Section = TEXT("CharacterSelectionStudioGuide");
    FText Text(const FString& Value) { return FText::FromString(Value); }
}

void SCharacterSelectionGuide::Construct(const FArguments& Args)
{
    if (const auto Plugin = IPluginManager::Get().FindPlugin(TEXT("FightingCharacterSelect")))
        Root = FPaths::ConvertRelativePathToFull(Plugin->GetBaseDir() / TEXT("Resources/Guide"));
    LoadContent();
    FString LastChapter;
    GConfig->GetString(StudioGuide::Section, TEXT("LastChapter"), LastChapter, GEditorPerProjectIni);
    for (int32 Index = 0; Index < Chapters.Num(); ++Index)
        if (Chapters[Index].Id == LastChapter) ActiveChapter = Index;

    ChildSlot
    [
        SNew(SBorder).Padding(18)
        [SNew(SVerticalBox)
            + SVerticalBox::Slot().AutoHeight().Padding(0,0,0,6)
            [SNew(STextBlock).Text(StudioGuide::Text(TEXT("Character Selection Studio Guide")))
                .Font(FCoreStyle::GetDefaultFontStyle("Bold", 24))]
            + SVerticalBox::Slot().AutoHeight().Padding(0,0,0,14)
            [SNew(STextBlock).AutoWrapText(true).Text(StudioGuide::Text(TEXT("Choose a topic or search for help")))]
            + SVerticalBox::Slot().FillHeight(1)
            [SNew(SSplitter)
                + SSplitter::Slot().Value(.27f)
                [SNew(SVerticalBox)
                    + SVerticalBox::Slot().AutoHeight().Padding(0,0,10,10)
                    [SAssignNew(Search, SSearchBox).HintText(StudioGuide::Text(TEXT("Search chapters and instructions...")))
                        .OnTextChanged_Lambda([this](const FText& Text) { Query = Text.ToString().TrimStartAndEnd(); RebuildSidebar(); })]
                    + SVerticalBox::Slot().FillHeight(1).Padding(0,0,10,0)
                    [SNew(SScrollBox) + SScrollBox::Slot()[SAssignNew(Sidebar, SVerticalBox)]]
                    + SVerticalBox::Slot().AutoHeight().Padding(0,10,10,0)
                    [SNew(STextBlock).AutoWrapText(true).Text(StudioGuide::Text(TEXT("Reopen from Studio > Studio Guide")))]]
                + SSplitter::Slot().Value(.73f)
                [SNew(SVerticalBox)
                    + SVerticalBox::Slot().AutoHeight().Padding(16,0,0,12)
                    [SNew(STextBlock).Text_Lambda([this] { return StudioGuide::Text(Chapters.IsValidIndex(ActiveChapter)
                        ? FString::Printf(TEXT("CHAPTER %d OF %d  /  %s"), ActiveChapter + 1, Chapters.Num(), *Chapters[ActiveChapter].Group) : TEXT("Guide unavailable")); })]
                    + SVerticalBox::Slot().FillHeight(1).Padding(16,0,8,0)
                    [SAssignNew(ReadingScroll, SScrollBox) + SScrollBox::Slot()[SAssignNew(Body, SVerticalBox)]]
                    + SVerticalBox::Slot().AutoHeight().Padding(16,14,0,0)
                    [SNew(SHorizontalBox)
                        + SHorizontalBox::Slot().AutoWidth().Padding(0,0,8,0)
                        [SNew(SButton).Text(StudioGuide::Text(TEXT("Previous"))).IsEnabled_Lambda([this] { return ActiveChapter > 0; })
                            .OnClicked_Lambda([this] { Navigate(-1); return FReply::Handled(); })]
                        + SHorizontalBox::Slot().AutoWidth().Padding(0,0,8,0)
                        [SNew(SButton).Text(StudioGuide::Text(TEXT("Next chapter"))).IsEnabled_Lambda([this] { return ActiveChapter + 1 < Chapters.Num(); })
                            .OnClicked_Lambda([this] { Navigate(1); return FReply::Handled(); })]
                        + SHorizontalBox::Slot().AutoWidth().Padding(0,0,8,0)
                        [SNew(SButton).Text(StudioGuide::Text(TEXT("Back to top")))
                            .OnClicked_Lambda([this] { ReadingScroll->ScrollToStart(); return FReply::Handled(); })]
                        + SHorizontalBox::Slot().FillWidth(1)
                        + SHorizontalBox::Slot().AutoWidth()
                        [SNew(SButton).Text(StudioGuide::Text(TEXT("Reload guide")))
                            .ToolTipText(StudioGuide::Text(TEXT("Reload guide.json and newly added images without restarting Unreal. Close and reopen the guide to refresh a replaced image.")))
                            .OnClicked_Lambda([this] { LoadContent(); ActiveChapter = FMath::Clamp(ActiveChapter, 0, FMath::Max(0, Chapters.Num()-1)); ShowChapter(ActiveChapter); return FReply::Handled(); })]]]]]
    ];
    ShowChapter(ActiveChapter);
}

bool SCharacterSelectionGuide::LoadContent()
{
    Chapters.Reset(); Error.Reset();
    FString Source;
    TSharedPtr<FJsonObject> Json;
    if (Root.IsEmpty() || !FFileHelper::LoadFileToString(Source, *(Root / TEXT("guide.json"))) ||
        !FJsonSerializer::Deserialize(TJsonReaderFactory<>::Create(Source), Json) || !Json.IsValid())
    { Error = TEXT("Could not read Resources/Guide/guide.json. Restore the plugin guide files and click Reload guide."); return false; }
    const TArray<TSharedPtr<FJsonValue>>* Entries = nullptr;
    if (!Json->TryGetArrayField(TEXT("chapters"), Entries))
    { Error = TEXT("The guide has no chapters array. Restore guide.json and reload."); return false; }
    TSet<FString> Ids;
    for (const auto& Entry : *Entries)
    {
        const TSharedPtr<FJsonObject>* Object = nullptr;
        if (!Entry->TryGetObject(Object) || !Object->IsValid()) continue;
        FChapter Chapter;
        if (!(*Object)->TryGetStringField(TEXT("id"), Chapter.Id) || Chapter.Id.IsEmpty() || Ids.Contains(Chapter.Id) ||
            !(*Object)->TryGetStringField(TEXT("title"), Chapter.Title) || Chapter.Title.IsEmpty()) continue;
        (*Object)->TryGetStringField(TEXT("group"), Chapter.Group);
        const TArray<TSharedPtr<FJsonValue>>* Blocks = nullptr;
        if (!(*Object)->TryGetArrayField(TEXT("blocks"), Blocks)) continue;
        for (const auto& Value : *Blocks)
        {
            const TSharedPtr<FJsonObject>* Item = nullptr;
            if (!Value->TryGetObject(Item) || !Item->IsValid()) continue;
            FBlock Block;
            (*Item)->TryGetStringField(TEXT("type"), Block.Type);
            (*Item)->TryGetStringField(TEXT("text"), Block.Text);
            (*Item)->TryGetStringField(TEXT("image"), Block.Image);
            (*Item)->TryGetStringField(TEXT("caption"), Block.Caption);
            Chapter.Blocks.Add(MoveTemp(Block));
        }
        Ids.Add(Chapter.Id); Chapters.Add(MoveTemp(Chapter));
    }
    if (Chapters.IsEmpty()) Error = TEXT("No readable chapters were found. Check chapter IDs, titles and blocks in guide.json, then reload.");
    return !Chapters.IsEmpty();
}

bool SCharacterSelectionGuide::Matches(const FChapter& Chapter) const
{
    if (Query.IsEmpty()) return true;
    FString Haystack = Chapter.Title + TEXT(" ") + Chapter.Group;
    for (const auto& Block : Chapter.Blocks) Haystack += TEXT(" ") + Block.Text + TEXT(" ") + Block.Caption;
    TArray<FString> Words; Query.ParseIntoArrayWS(Words);
    for (const FString& Word : Words) if (!Haystack.Contains(Word, ESearchCase::IgnoreCase)) return false;
    return true;
}

void SCharacterSelectionGuide::RebuildSidebar()
{
    if (!Sidebar) return;
    Sidebar->ClearChildren();
    FString LastGroup;
    int32 Count = 0;
    for (int32 Index = 0; Index < Chapters.Num(); ++Index)
    {
        const auto& Chapter = Chapters[Index];
        if (!Matches(Chapter)) continue;
        ++Count;
        if (LastGroup != Chapter.Group)
        {
            LastGroup = Chapter.Group;
            Sidebar->AddSlot().AutoHeight().Padding(4,14,0,5)
                [SNew(STextBlock).Text(StudioGuide::Text(LastGroup)).Font(FCoreStyle::GetDefaultFontStyle("Bold", 11))];
        }
        Sidebar->AddSlot().AutoHeight().Padding(0,2)
            [SNew(SButton).HAlign(HAlign_Fill).ContentPadding(FMargin(10,9))
                .ButtonColorAndOpacity(Index == ActiveChapter ? FLinearColor(.12f,.35f,.6f) : FLinearColor::White)
                .OnClicked_Lambda([this, Index] { ShowChapter(Index); return FReply::Handled(); })
                [SNew(STextBlock).AutoWrapText(true).Text(StudioGuide::Text(FString::Printf(TEXT("%d. %s"), Index+1, *Chapter.Title)))]];
    }
    if (!Count) Sidebar->AddSlot().AutoHeight().Padding(6,14)
        [SNew(STextBlock).AutoWrapText(true).Text(StudioGuide::Text(TEXT("No matching chapters. Try a shorter search, such as portrait, joining or theme.")))];
}

void SCharacterSelectionGuide::Navigate(int32 Delta)
{
    Query.Reset();
    Search->SetText(FText::GetEmpty());
    ShowChapter(FMath::Clamp(ActiveChapter + Delta, 0, FMath::Max(0, Chapters.Num()-1)));
}

bool SCharacterSelectionGuide::ResolveImage(const FString& Relative, FString& Absolute) const
{
    if (Root.IsEmpty() || Relative.IsEmpty() || !FPaths::IsRelative(Relative)) return false;
    Absolute = FPaths::ConvertRelativePathToFull(Root / Relative);
    FPaths::NormalizeFilename(Absolute);
    FString NormalRoot = Root; FPaths::NormalizeFilename(NormalRoot);
    return FPaths::IsUnderDirectory(Absolute, NormalRoot) && FPaths::GetExtension(Absolute).Equals(TEXT("png"), ESearchCase::IgnoreCase);
}

void SCharacterSelectionGuide::AddImage(const FBlock& Block)
{
    FString Path;
    TSharedPtr<FSlateDynamicImageBrush> Brush;
    if (ResolveImage(Block.Image, Path) && FPaths::FileExists(Path) && FSlateApplication::IsInitialized())
    {
        const FName Resource(*Path);
        const FIntPoint Size = FSlateApplication::Get().GetRenderer()->GenerateDynamicImageResource(Resource);
        if (Size.X > 0 && Size.Y > 0)
        { Brush = MakeShared<FSlateDynamicImageBrush>(Resource, FVector2D(Size.X, Size.Y)); Images.Add(Brush); }
    }
    if (Brush)
        Body->AddSlot().AutoHeight().Padding(0,12,0,6)
            [SNew(SBox).HeightOverride(300)[SNew(SScaleBox).Stretch(EStretch::ScaleToFit)
                [SNew(SImage).Image(Brush.Get())]]];
    else
        Body->AddSlot().AutoHeight().Padding(0,12,0,6)
            [SNew(SBorder).Padding(20)[SNew(SBox).MinDesiredHeight(170).VAlign(VAlign_Center)
                [SNew(STextBlock).AutoWrapText(true).Justification(ETextJustify::Center)
                    .Text(StudioGuide::Text(TEXT("Image coming soon")))]]];
    Body->AddSlot().AutoHeight().Padding(0,0,0,16)
        [SNew(STextBlock).AutoWrapText(true).Text(StudioGuide::Text(Block.Caption))];
}

void SCharacterSelectionGuide::ShowChapter(int32 Index)
{
    Body->ClearChildren(); Images.Reset();
    if (!Chapters.IsValidIndex(Index))
    {
        Body->AddSlot().AutoHeight()[SNew(STextBlock).AutoWrapText(true).Text(StudioGuide::Text(Error))];
        RebuildSidebar(); return;
    }
    ActiveChapter = Index;
    const auto& Chapter = Chapters[Index];
    Body->AddSlot().AutoHeight().Padding(0,0,0,16)
        [SNew(STextBlock).AutoWrapText(true).Text(StudioGuide::Text(Chapter.Title)).Font(FCoreStyle::GetDefaultFontStyle("Bold", 22))];
    // Jump links scroll to real subsection widgets rather than fixed pixel offsets.
    TSharedRef<SVerticalBox> JumpLinks = SNew(SVerticalBox);
    Body->AddSlot().AutoHeight().Padding(0,0,0,16)[JumpLinks];
    for (const auto& Block : Chapter.Blocks)
    {
        if (Block.Type == TEXT("image")) { AddImage(Block); continue; }
        const bool Heading = Block.Type == TEXT("heading");
        const FString Value = Block.Type == TEXT("bullet") ? TEXT("• ") + Block.Text : Block.Text;
        TSharedRef<STextBlock> Text = SNew(STextBlock).AutoWrapText(true).Text(StudioGuide::Text(Value))
            .HighlightText(StudioGuide::Text(Query))
            .Font(FCoreStyle::GetDefaultFontStyle(Heading ? "Bold" : "Regular", Heading ? 16 : 12));
        Body->AddSlot().AutoHeight().Padding(0, Heading ? 18 : 0, 10, Heading ? 10 : 12)[Text];
        if (Heading)
            JumpLinks->AddSlot().AutoHeight().Padding(0,2)
                [SNew(SButton).HAlign(HAlign_Left).ContentPadding(FMargin(8,4))
                    .OnClicked_Lambda([this, Weak = TWeakPtr<SWidget>(Text)] { if (auto Target = Weak.Pin()) ReadingScroll->ScrollDescendantIntoView(Target, true, EDescendantScrollDestination::TopOrLeft); return FReply::Handled(); })
                    [SNew(STextBlock).Text(StudioGuide::Text(Block.Text)).AutoWrapText(true)]];
    }
    ReadingScroll->ScrollToStart(); RebuildSidebar();
    if (!GIsAutomationTesting && !IsRunningCommandlet())
    {
        GConfig->SetString(StudioGuide::Section, TEXT("LastChapter"), *Chapter.Id, GEditorPerProjectIni);
        GConfig->Flush(false, GEditorPerProjectIni);
    }
}
