using UnrealBuildTool;
public class DinosaurBattle : ModuleRules
{
    public DinosaurBattle(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
        PublicDependencyModuleNames.AddRange(new string[] {
            "Core", "CoreUObject", "Engine", "InputCore", "AIModule", "NavigationSystem", "AudioMixer",
            "UMG", "Slate", "SlateCore", "ProceduralMeshComponent", "Json", "JsonUtilities",
            "OnlineSubsystem", "OnlineSubsystemUtils", "OnlineBase"
        });
        PrivateDependencyModuleNames.AddRange(new string[] { "OnlineSubsystemEOS", "SocketSubsystemEOS", "Sockets" });
    }
}
