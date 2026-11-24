using UnrealBuildTool;
public class SelectionReleaseHostEditor : ModuleRules
{
    public SelectionReleaseHostEditor(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.NoPCHs;
        PrivateDependencyModuleNames.AddRange(new[] { "Core", "CoreUObject", "Engine", "UnrealEd", "UMG", "UMGEditor", "CharacterSelection", "CharacterSelectionEditor" });
    }
}
