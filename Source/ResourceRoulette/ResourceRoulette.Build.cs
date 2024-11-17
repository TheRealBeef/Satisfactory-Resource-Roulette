using UnrealBuildTool;
using System.IO;
using System;

public class ResourceRoulette : ModuleRules
{
	public ResourceRoulette(ReadOnlyTargetRules Target) : base(Target)
	{
        DefaultBuildSettings = BuildSettingsVersion.Latest;
        ShadowVariableWarningLevel = WarningLevel.Error;
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
        bLegacyParentIncludePaths = false;
        CppStandard = CppStandardVersion.Cpp20;
        bUseUnity = false;
        // OptimizeCode = CodeOptimization.Never;
        
		PublicDependencyModuleNames.AddRange(new string[] {
            "Core", "CoreUObject", "Engine",
            "DeveloperSettings", "PhysicsCore", "InputCore",
            "AssetRegistry", "RenderCore", "RHI",
            "SlateCore", "Slate", "UMG", "GameplayTags",
            "DummyHeaders", "FactoryGame", "SML", "Json"

		});
		
		if (Target.Type == TargetRules.TargetType.Editor)
		{
			PublicDependencyModuleNames.AddRange(new string[]
			{
				"OnlineBlueprintSupport",
				"AnimGraph",
				"Kismet",
				"EditorStyle"
			});
		}
	}
}
