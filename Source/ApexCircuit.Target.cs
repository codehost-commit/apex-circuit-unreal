using UnrealBuildTool;

public class ApexCircuitTarget : TargetRules
{
	public ApexCircuitTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Game;
		DefaultBuildSettings = BuildSettingsVersion.V7;
		IncludeOrderVersion = EngineIncludeOrderVersion.Unreal5_8;
		ExtraModuleNames.Add("ApexCircuit");
	}
}
