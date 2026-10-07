#pragma once

#include "CoreMinimal.h"
#include "AirGloveClientTypes.generated.h"

UENUM(BlueprintType)
enum class EAirGloveClientSide : uint8
{
	Left = 0,
	Right = 1,
};

UENUM(BlueprintType)
enum class EAirGloveClientJoint : uint8
{
	Palm,
	Wrist,
	ThumbMetacarpal,
	ThumbProximal,
	ThumbDistal,
	ThumbTip,
	IndexMetacarpal,
	IndexProximal,
	IndexIntermediate,
	IndexDistal,
	IndexTip,
	MiddleMetacarpal,
	MiddleProximal,
	MiddleIntermediate,
	MiddleDistal,
	MiddleTip,
	RingMetacarpal,
	RingProximal,
	RingIntermediate,
	RingDistal,
	RingTip,
	LittleMetacarpal,
	LittleProximal,
	LittleIntermediate,
	LittleDistal,
	LittleTip,
};

USTRUCT(BlueprintType)
struct FAirGloveClientHandPose
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "AirGloveClient")
	TArray<FTransform> Joints;

	UPROPERTY(BlueprintReadOnly, Category = "AirGloveClient")
	TArray<float> Radii;

	UPROPERTY(BlueprintReadOnly, Category = "AirGloveClient")
	TArray<bool> Valid;

	UPROPERTY(BlueprintReadOnly, Category = "AirGloveClient")
	int32 Sequence = 0;

	UPROPERTY(BlueprintReadOnly, Category = "AirGloveClient")
	float AgeSeconds = 0.f;

	UPROPERTY(BlueprintReadOnly, Category = "AirGloveClient")
	bool bTracked = false;
};

namespace AirGloveClient
{
	constexpr int32 JointCount = 26;
	constexpr int32 FingerCount = 5;

	AIRGLOVECLIENT_API const TArray<FName>& JointNames();
	AIRGLOVECLIENT_API const TArray<int32>& JointParents();
	AIRGLOVECLIENT_API FVector ToUnrealPosition(float X, float Y, float Z);
	AIRGLOVECLIENT_API FQuat ToUnrealRotation(float QX, float QY, float QZ, float QW);
}
