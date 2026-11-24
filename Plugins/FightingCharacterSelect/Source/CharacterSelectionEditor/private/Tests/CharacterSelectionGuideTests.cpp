#if WITH_DEV_AUTOMATION_TESTS
#include "Misc/AutomationTest.h"
#include "SCharacterSelectionGuide.h"
#include "Widgets/Input/SSearchBox.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCharacterSelectionGuideTest, "CharacterSelection.Editor.StudioGuide", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FCharacterSelectionGuideTest::RunTest(const FString& Parameters)
{
    const TSharedRef<SCharacterSelectionGuide> Guide = SNew(SCharacterSelectionGuide);
    TestTrue(TEXT("Bundled guide loads"), Guide->Error.IsEmpty());
    if (!TestEqual(TEXT("All manual chapters are bundled"), Guide->Chapters.Num(), 15)) return false;
    TSet<FString> Ids;
    for (const auto& Chapter : Guide->Chapters)
    {
        TestFalse(TEXT("Chapter IDs are unique"), Ids.Contains(Chapter.Id)); Ids.Add(Chapter.Id);
        TestTrue(TEXT("Chapter includes reading content"), Chapter.Blocks.ContainsByPredicate([](const auto& Block)
        { return Block.Type != TEXT("image") && !Block.Text.IsEmpty(); }));
        bool bImage = false;
        for (const auto& Block : Chapter.Blocks)
        {
            if (Block.Type != TEXT("image")) continue;
            bImage = true;
            FString Path;
            TestTrue(TEXT("Illustration paths stay inside the guide"), Guide->ResolveImage(Block.Image, Path));
            TestFalse(TEXT("Illustrations have explanatory captions"), Block.Caption.IsEmpty());
        }
        TestTrue(TEXT("Each chapter reserves an illustration space"), bImage);
    }
    Guide->Query = TEXT("mEsH CAMERA");
    TestTrue(TEXT("Search matches body text using case insensitive words"), Guide->Matches(Guide->Chapters[8]));
    Guide->Query = TEXT("NoSuchGuideTopic123");
    for (const auto& Chapter : Guide->Chapters) TestFalse(TEXT("Missing topic does not match"), Guide->Matches(Chapter));
    Guide->RebuildSidebar();
    Guide->ShowChapter(0); Guide->Navigate(-1);
    TestEqual(TEXT("Previous clamps at first chapter"), Guide->ActiveChapter, 0);
    TestTrue(TEXT("Sequential navigation clears the filter"), Guide->Query.IsEmpty());
    Guide->Navigate(1); TestEqual(TEXT("Next opens following chapter"), Guide->ActiveChapter, 1);
    Guide->ShowChapter(14); Guide->Navigate(1);
    TestEqual(TEXT("Next clamps at final chapter"), Guide->ActiveChapter, 14);
    FString Path;
    TestFalse(TEXT("Image traversal is rejected"), Guide->ResolveImage(TEXT("../outside.png"), Path));
    TestFalse(TEXT("Absolute images are rejected"), Guide->ResolveImage(TEXT("C:/outside.png"), Path));
    TestFalse(TEXT("Unsupported image extensions are rejected"), Guide->ResolveImage(TEXT("images/example.svg"), Path));
    const FString Root = Guide->Root;
    Guide->Root /= TEXT("MissingGuideFixture");
    TestFalse(TEXT("Missing content fails gracefully"), Guide->LoadContent());
    TestFalse(TEXT("Missing content explains recovery"), Guide->Error.IsEmpty());
    Guide->ShowChapter(0);
    Guide->Root = Root;
    TestTrue(TEXT("Content can be reloaded after recovery"), Guide->LoadContent());
    Guide->ShowChapter(0);
    return true;
}
#endif
