using UnrealBuildTool;

public class ApexCircuit : ModuleRules
{
	public ApexCircuit(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",
				"CoreUObject",
				"Engine",
				"InputCore",
				"EnhancedInput",
				"ProceduralMeshComponent",
				"Json",
				"JsonUtilities"
			});
	}
}
