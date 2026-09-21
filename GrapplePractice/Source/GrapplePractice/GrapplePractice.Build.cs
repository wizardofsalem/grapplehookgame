// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class GrapplePractice : ModuleRules
{
	public GrapplePractice(ReadOnlyTargetRules Target) : base(Target)
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
			"Slate"
		});

		PrivateDependencyModuleNames.AddRange(new string[] { });

		PublicIncludePaths.AddRange(new string[] {
			"GrapplePractice",
			"GrapplePractice/Variant_Platforming",
			"GrapplePractice/Variant_Platforming/Animation",
			"GrapplePractice/Variant_Combat",
			"GrapplePractice/Variant_Combat/AI",
			"GrapplePractice/Variant_Combat/Animation",
			"GrapplePractice/Variant_Combat/Gameplay",
			"GrapplePractice/Variant_Combat/Interfaces",
			"GrapplePractice/Variant_Combat/UI",
			"GrapplePractice/Variant_SideScrolling",
			"GrapplePractice/Variant_SideScrolling/AI",
			"GrapplePractice/Variant_SideScrolling/Gameplay",
			"GrapplePractice/Variant_SideScrolling/Interfaces",
			"GrapplePractice/Variant_SideScrolling/UI"
		});

		// Uncomment if you are using Slate UI
		// PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });

		// Uncomment if you are using online features
		// PrivateDependencyModuleNames.Add("OnlineSubsystem");

		// To include OnlineSubsystemSteam, add it to the plugins section in your uproject file with the Enabled attribute set to true
	}
}
