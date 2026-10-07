using System.IO;
using UnrealBuildTool;

public class AirGloveClient : ModuleRules
{
	public AirGloveClient(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
		PublicDependencyModuleNames.AddRange(new[] { "Core", "CoreUObject", "Engine", "LiveLinkInterface", "LiveLinkAnimationCore", "DeveloperSettings" });
		PrivateDependencyModuleNames.AddRange(new[] { "Projects", "LiveLink" });

		string ThirdParty = Path.Combine(ModuleDirectory, "..", "ThirdParty", "AirGloveClient");
		PrivateIncludePaths.Add(Path.Combine(ThirdParty, "include"));

		string Binaries = Path.Combine(PluginDirectory, "Binaries", "ThirdParty", "AirGloveClient");
		if (Target.Platform == UnrealTargetPlatform.Linux)
		{
			RuntimeDependencies.Add(Path.Combine(Binaries, "Linux", "libairglove_client.so"));
		}
		else if (Target.Platform == UnrealTargetPlatform.Win64)
		{
			RuntimeDependencies.Add(Path.Combine(Binaries, "Win64", "airglove_client.dll"));
		}
		else if (Target.Platform == UnrealTargetPlatform.Mac)
		{
			RuntimeDependencies.Add(Path.Combine(Binaries, "Mac", "libairglove_client.dylib"));
		}
	}
}
