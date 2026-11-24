using UnrealBuildTool;

public class CharacterSelectionEditor : ModuleRules
{
    public CharacterSelectionEditor(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicDependencyModuleNames.AddRange(new string[]
        {
            "Core",
            "CoreUObject",
            "Engine"
        });

        PrivateDependencyModuleNames.AddRange(new string[]
        {
            "CharacterSelection",
            "Slate",
            "SlateCore",
            "UnrealEd",
            "InputCore",
            "UMG",
            "UMGEditor",
            "LevelEditor",
            "ToolMenus",
            "PropertyEditor",
            "ContentBrowser",
            "AssetRegistry",
            "AssetTools",
            "Projects",
            "Json"
        });
    }
}
