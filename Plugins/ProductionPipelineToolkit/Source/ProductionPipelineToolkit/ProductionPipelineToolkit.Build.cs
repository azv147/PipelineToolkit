// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class ProductionPipelineToolkit : ModuleRules
{
	public ProductionPipelineToolkit(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"Core"
			}
		);

		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"CoreUObject",
				"Engine",
				"Slate",
				"SlateCore",
				"UnrealEd",
				"LevelEditor",
				"ToolMenus",
				"WorkspaceMenuStructure",
				"AssetRegistry",
				"AssetTools",
        		"Json"
			}
		);
	}
}