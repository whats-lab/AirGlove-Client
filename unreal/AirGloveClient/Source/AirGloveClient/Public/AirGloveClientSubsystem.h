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

	UFUNCTION(BlueprintPure, Category = "AirGloveClient")
	bool GetWristOrientation(EAirGloveClientSide Side, FQuat& Orientation, float& AgeSeconds) const;

	UFUNCTION(BlueprintPure, Category = "AirGloveClient")
	bool GetDeviceStatus(bool& bLeftConnected, bool& bRightConnected) const;

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
	FTSTicker::FDelegateHandle TickHandle;
	TSharedPtr<FAirGloveClientLiveLinkSource> LiveLinkSource;
	FAirGloveClientHandPose Poses[2];
};
