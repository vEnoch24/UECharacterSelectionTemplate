using UnrealBuildTool;

public class CharacterSelectionDemo : ModuleRules
{
    public CharacterSelectionDemo(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
        PrivateDependencyModuleNames.AddRange(new[] { "Core", "CoreUObject", "Engine", "CharacterSelection" });
    }
}
