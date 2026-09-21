// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class AmiyaDemo : ModuleRules
{
	public AmiyaDemo(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] {
			"Core",
			"CoreUObject",
			"Engine",
			"InputCore",
			"EnhancedInput",
			"AIModule",
			"StateTreeModule",
			"GameplayStateTreeModule",
			"UMG",
			"Slate",
            "NavigationSystem"
        });

		PrivateDependencyModuleNames.AddRange(new string[] { });

		PublicIncludePaths.AddRange(new string[] {
			"AmiyaDemo",
			"AmiyaDemo/Variant_Platforming",
			"AmiyaDemo/Variant_Platforming/Animation",
			"AmiyaDemo/Variant_Combat",
			"AmiyaDemo/Variant_Combat/AI",
			"AmiyaDemo/Variant_Combat/Animation",
			"AmiyaDemo/Variant_Combat/Gameplay",
			"AmiyaDemo/Variant_Combat/Interfaces",
			"AmiyaDemo/Variant_Combat/UI",
			"AmiyaDemo/Variant_SideScrolling",
			"AmiyaDemo/Variant_SideScrolling/AI",
			"AmiyaDemo/Variant_SideScrolling/Gameplay",
			"AmiyaDemo/Variant_SideScrolling/Interfaces",
			"AmiyaDemo/Variant_SideScrolling/UI"
		});

		// Uncomment if you are using Slate UI
		// PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });

		// Uncomment if you are using online features
		// PrivateDependencyModuleNames.Add("OnlineSubsystem");

		// To include OnlineSubsystemSteam, add it to the plugins section in your uproject file with the Enabled attribute set to true
	}
}
