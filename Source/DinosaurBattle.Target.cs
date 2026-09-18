using UnrealBuildTool;
public class DinosaurBattleTarget : TargetRules
{
    public DinosaurBattleTarget(TargetInfo Target) : base(Target)
    {
        Type = TargetType.Game;
        DefaultBuildSettings = BuildSettingsVersion.V7;
        IncludeOrderVersion = EngineIncludeOrderVersion.Unreal5_8;
        ExtraModuleNames.Add("DinosaurBattle");
    }
}
