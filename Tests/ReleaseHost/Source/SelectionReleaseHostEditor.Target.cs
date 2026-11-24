using UnrealBuildTool;
public class SelectionReleaseHostEditorTarget : TargetRules
{
    public SelectionReleaseHostEditorTarget(TargetInfo Target) : base(Target)
    {
        Type = TargetType.Editor;
        DefaultBuildSettings = BuildSettingsVersion.V7;
        IncludeOrderVersion = EngineIncludeOrderVersion.Unreal5_8;
        ExtraModuleNames.AddRange(new[] { "SelectionReleaseHost", "SelectionReleaseHostEditor" });
    }
}
