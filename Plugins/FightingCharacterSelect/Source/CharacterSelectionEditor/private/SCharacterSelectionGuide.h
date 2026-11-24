#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"

class SScrollBox;
class SVerticalBox;
class SSearchBox;
struct FSlateDynamicImageBrush;

/** Editor-only, data-driven manual. Content and screenshots live in Resources/Guide. */
class SCharacterSelectionGuide : public SCompoundWidget
{
public:
    SLATE_BEGIN_ARGS(SCharacterSelectionGuide) {} SLATE_END_ARGS()
    void Construct(const FArguments& Args);
private:
    friend class FCharacterSelectionGuideTest;
    struct FBlock { FString Type, Text, Image, Caption; };
    struct FChapter { FString Id, Title, Group; TArray<FBlock> Blocks; };
    bool LoadContent();
    void RebuildSidebar();
    void ShowChapter(int32 Index);
    void Navigate(int32 Delta);
    bool Matches(const FChapter& Chapter) const;
    bool ResolveImage(const FString& Relative, FString& Absolute) const;
    void AddImage(const FBlock& Block);
    FString Root, Error, Query;
    TArray<FChapter> Chapters;
    int32 ActiveChapter = 0;
    TSharedPtr<SVerticalBox> Sidebar, Body;
    TSharedPtr<SScrollBox> ReadingScroll;
    TSharedPtr<SSearchBox> Search;
    TArray<TSharedPtr<FSlateDynamicImageBrush>> Images;
};
