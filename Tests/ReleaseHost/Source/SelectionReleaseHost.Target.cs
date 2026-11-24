using UnrealBuildTool;
public class SelectionReleaseHostTarget : TargetRules
{
    public SelectionReleaseHostTarget(TargetInfo Target) : base(Target)
    {
        Type = TargetType.Game;
        DefaultBuildSettings = BuildSettingsVersion.V7;
        IncludeOrderVersion = EngineIncludeOrderVersion.Unreal5_8;
        ExtraModuleNames.Add("SelectionReleaseHost");
    }
}
