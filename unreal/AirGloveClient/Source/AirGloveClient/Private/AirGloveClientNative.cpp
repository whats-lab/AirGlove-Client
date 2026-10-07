#include "AirGloveClientNative.h"

#include "AirGloveClientLog.h"
#include "HAL/PlatformProcess.h"
#include "Interfaces/IPluginManager.h"
#include "Misc/Paths.h"

const FAirGloveClientNative* FAirGloveClientNative::Get()
{
	static FAirGloveClientNative Native;
	static bool bLoaded = Native.Load();
	return bLoaded ? &Native : nullptr;
}

FString FAirGloveClientNative::Error() const
{
	return LastError ? FString(UTF8_TO_TCHAR(LastError())) : FString();
}

bool FAirGloveClientNative::Load()
{
	TSharedPtr<IPlugin> Plugin = IPluginManager::Get().FindPlugin(TEXT("AirGloveClient"));
	if (!Plugin)
	{
		return false;
	}
#if PLATFORM_WINDOWS
	const FString File = TEXT("Win64/airglove_client.dll");
#elif PLATFORM_MAC
	const FString File = TEXT("Mac/libairglove_client.dylib");
#else
	const FString File = TEXT("Linux/libairglove_client.so");
#endif
	const FString Path = FPaths::Combine(Plugin->GetBaseDir(), TEXT("Binaries/ThirdParty/AirGloveClient"), File);
	Handle = FPlatformProcess::GetDllHandle(*Path);
	if (!Handle)
	{
		UE_LOG(LogAirGloveClient, Error, TEXT("cannot load %s"), *Path);
		return false;
	}
#define AGC_RESOLVE(Member, Name)                                                                    \
	Member = reinterpret_cast<decltype(Member)>(FPlatformProcess::GetDllExport(Handle, TEXT(#Name))); \
	if (!Member)                                                                                     \
	{                                                                                                \
		UE_LOG(LogAirGloveClient, Error, TEXT("%s: missing symbol %s"), *Path, TEXT(#Name));               \
		return false;                                                                                \
	}
	AGC_RESOLVE(AbiVersion, agc_abi_version)
	AGC_RESOLVE(Version, agc_version)
	AGC_RESOLVE(LastError, agc_last_error)
	AGC_RESOLVE(Create, agc_create)
	AGC_RESOLVE(Destroy, agc_destroy)
	AGC_RESOLVE(GetHand, agc_get_hand)
	AGC_RESOLVE(GetWrist, agc_get_wrist)
	AGC_RESOLVE(GetDeviceStatus, agc_get_device_status)
	AGC_RESOLVE(GetSpineAlive, agc_get_spine_alive)
	AGC_RESOLVE(SetHaptics, agc_set_haptics)
	AGC_RESOLVE(GetHapticsResult, agc_get_haptics_result)
	AGC_RESOLVE(PollAlarm, agc_poll_alarm)
	AGC_RESOLVE(SetStaleThreshold, agc_set_stale_threshold)
#undef AGC_RESOLVE
	if (AbiVersion() != AGC_ABI_VERSION)
	{
		UE_LOG(LogAirGloveClient, Error, TEXT("%s: ABI %d, plugin expects %d"), *Path, AbiVersion(), AGC_ABI_VERSION);
		return false;
	}
	UE_LOG(LogAirGloveClient, Log, TEXT("airglove_client %s loaded"), UTF8_TO_TCHAR(Version()));
	return true;
}
