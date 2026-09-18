using UnrealBuildTool;
public class DinosaurBattleEditorTarget : TargetRules
{
    public DinosaurBattleEditorTarget(TargetInfo Target) : base(Target)
    {
        Type = TargetType.Editor;
        DefaultBuildSettings = BuildSettingsVersion.V7;
        IncludeOrderVersion = EngineIncludeOrderVersion.Unreal5_8;
        ExtraModuleNames.Add("DinosaurBattle");
    }
}
