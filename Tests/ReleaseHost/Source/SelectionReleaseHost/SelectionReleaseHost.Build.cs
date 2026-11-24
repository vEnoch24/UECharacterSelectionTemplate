using UnrealBuildTool;
public class SelectionReleaseHost : ModuleRules
{
    public SelectionReleaseHost(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
        PrivateDependencyModuleNames.AddRange(new[] { "Core", "CoreUObject", "Engine", "UMG", "CharacterSelection" });
    }
}
