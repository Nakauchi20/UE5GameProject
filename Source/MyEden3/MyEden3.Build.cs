// Fill out your copyright notice in the Description page of Project Settings.

using UnrealBuildTool;

public class MyEden3 : ModuleRules
{
	public MyEden3(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

        // Unity Build Çñ≥å¯âª
        //bUseUnity = false;

        PublicDependencyModuleNames.AddRange(new string[] { 
			"Core", 
			"CoreUObject", 
			"Engine", 
			"InputCore", 
			"EnhancedInput", 
			"GameplayAbilities", 
			"GameplayTags", 
			"GameplayTasks", 
			"AIModule",
            "StateTreeModule",
            "GameplayStateTreeModule"
        });


		PrivateDependencyModuleNames.AddRange(new string[] {
            "Slate",
            "SlateCore"
        });

        // EQSä÷òAÇÃÉÇÉWÉÖÅ[ÉãÇí«â¡
        if (Target.bBuildDeveloperTools ||
            (Target.Configuration != UnrealTargetConfiguration.Shipping &&
             Target.Configuration != UnrealTargetConfiguration.Test))
        {
            PrivateDependencyModuleNames.Add("AITestSuite");
        }

        // Uncomment if you are using Slate UI
        // PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });

        // Uncomment if you are using online features
        // PrivateDependencyModuleNames.Add("OnlineSubsystem");

        // To include OnlineSubsystemSteam, add it to the plugins section in your uproject file with the Enabled attribute set to true
    }
}
