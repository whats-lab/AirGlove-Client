#pragma once

#include "CoreMinimal.h"
#include "airglove_client.h"

struct FAirGloveClientNative
{
	decltype(&agc_abi_version) AbiVersion = nullptr;
	decltype(&agc_version) Version = nullptr;
	decltype(&agc_last_error) LastError = nullptr;
	decltype(&agc_create) Create = nullptr;
	decltype(&agc_destroy) Destroy = nullptr;
	decltype(&agc_get_hand) GetHand = nullptr;
	decltype(&agc_get_wrist) GetWrist = nullptr;
	decltype(&agc_get_device_status) GetDeviceStatus = nullptr;
	decltype(&agc_get_spine_alive) GetSpineAlive = nullptr;
	decltype(&agc_set_haptics) SetHaptics = nullptr;
	decltype(&agc_get_haptics_result) GetHapticsResult = nullptr;
	decltype(&agc_poll_alarm) PollAlarm = nullptr;
	decltype(&agc_set_stale_threshold) SetStaleThreshold = nullptr;

	static const FAirGloveClientNative* Get();
	FString Error() const;

private:
	bool Load();
	void* Handle = nullptr;
};
