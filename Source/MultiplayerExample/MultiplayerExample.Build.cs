// Copyright Epic Games, Inc. All Rights Reserved.
using UnrealBuildTool;

public class MultiplayerExample : ModuleRules
{
    public MultiplayerExample(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicDependencyModuleNames
            .AddRange(new string[] {
                "Core",
                "CoreUObject",
                "Engine",
                "InputCore",
                "Json",
                "JsonUtilities",
                "OnlineSubsystemUtils",
                "GameplayAbilities",
                "GameplayTags",
                "GameplayTasks"
            });
        PrivateDependencyModuleNames.AddRange(new string[] { "HTTP", "UMG" });
    }
}
