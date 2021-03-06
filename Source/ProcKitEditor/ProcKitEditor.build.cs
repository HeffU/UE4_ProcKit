// Copyright  2019 Rasmus Egerö All Rights Reserved.

using System.IO;
using UnrealBuildTool;

public class ProcKitEditor : ModuleRules
{
    public ProcKitEditor(ReadOnlyTargetRules rules) : base(rules)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicIncludePaths.AddRange(
            new string[] {
                // ... add public include paths required here ...
            }
            );

        PrivateIncludePaths.AddRange(
            new string[] {
                // ... add other private include paths required here ...
            }
            );

        PublicDependencyModuleNames.AddRange(
            new string[]
            {
                // ... add other public dependencies that you statically link with here ...
            }
            );

        PrivateDependencyModuleNames.AddRange(
            new string[]
            {
                "Core",
                "CoreUObject",
                "Engine",
                "InputCore",
                "Slate",
                "SlateCore",
                "EditorStyle",
                "UnrealEd",
                "RenderCore",
                "ShaderCore",
                "ViewportInteraction",
                "RHI",
                "LevelEditor",
                // ... add private dependencies that you statically link with here ...
            }
            );

        DynamicallyLoadedModuleNames.AddRange(
            new string[]
            {
                // ... add any modules that your module loads dynamically here ...
            }
            );
    }
}