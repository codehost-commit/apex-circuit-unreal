using UnrealBuildTool;

public class ApexCircuitEditorTarget : TargetRules
{
	public ApexCircuitEditorTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Editor;
		DefaultBuildSettings = BuildSettingsVersion.V7;
		IncludeOrderVersion = EngineIncludeOrderVersion.Unreal5_8;
		ExtraModuleNames.Add("ApexCircuit");
	}
}
