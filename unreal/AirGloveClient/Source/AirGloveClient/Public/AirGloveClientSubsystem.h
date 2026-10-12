#pragma once

#include "AirGloveClientTypes.h"
#include "Containers/Ticker.h"
#include "CoreMinimal.h"
#include "Subsystems/EngineSubsystem.h"
#include "AirGloveClientSubsystem.generated.h"

class FAirGloveClientLiveLinkSource;
struct agc_client;

UCLASS()
class AIRGLOVECLIENT_API UAirGloveClientSubsystem : public UEngineSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	UFUNCTION(BlueprintCallable, Category = "AirGloveClient")
	bool Start();

	UFUNCTION(BlueprintCallable, Category = "AirGloveClient")
	void Stop();

	UFUNCTION(BlueprintPure, Category = "AirGloveClient")
	bool IsRunning() const { return Client != nullptr; }

	UFUNCTION(BlueprintPure, Category = "AirGloveClient")
	FAirGloveClientHandPose GetHandPose(EAirGloveClientSide Side) const { return Poses[static_cast<int32>(Side)]; }

	UFUNCTION(BlueprintPure, Category = "AirGloveClient", meta = (ToolTip = "Raw dorsum IMU quaternion as Spine sends it, [y, x, z, -w] of the sensor's wxyz, unconverted. Not a world wrist pose and not in Unreal axes."))
	bool GetWristOrientation(EAirGloveClientSide Side, FQuat& Orientation, float& AgeSeconds) const;

	UFUNCTION(BlueprintPure, Category = "AirGloveClient")
	bool GetDeviceStatus(bool& bLeftConnected, bool& bRightConnected) const;

	UFUNCTION(BlueprintPure, Category = "AirGloveClient")
	bool GetSpineProtocol(int32& Version, float& AgeSeconds) const;

	UFUNCTION(BlueprintCallable, Category = "AirGloveClient")
	bool SetPair(int32 NewPair);

	UFUNCTION(BlueprintPure, Category = "AirGloveClient")
	int32 GetPair() const { return Pair; }

	UFUNCTION(BlueprintCallable, Category = "AirGloveClient")
	bool SetHaptics(EAirGloveClientSide Side, const TArray<int32>& Strengths);

	UFUNCTION(BlueprintPure, Category = "AirGloveClient")
	bool GetHapticsResult(EAirGloveClientSide Side, bool& bSuccess) const;

	DECLARE_MULTICAST_DELEGATE_TwoParams(FOnAlarm, const FString&, int32);
	FOnAlarm OnAlarm;

	bool Update();

private:
	bool Tick(float DeltaTime);

	agc_client* Client = nullptr;
	int32 Pair = 1;
	FTSTicker::FDelegateHandle TickHandle;
	TSharedPtr<FAirGloveClientLiveLinkSource> LiveLinkSource;
	FAirGloveClientHandPose Poses[2];
};
