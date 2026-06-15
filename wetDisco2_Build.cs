// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class wetDisco2 : ModuleRules
{
	public wetDisco2(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[]
		{
			"AIModule",
			"Core",
			"CoreUObject",
			"Engine",
			"EnhancedInput",
			"GameplayTags",
			"InputCore",
			"MovieScene",
			"NavigationSystem",
			"Niagara",
			"UMG",
			"Slate",
			"SlateCore",
			"ArticyRuntime"
		});

		if (Target.Type == TargetType.Editor)
		{
			PrivateDependencyModuleNames.Add("ArticyEditor");
		}
	}
}
