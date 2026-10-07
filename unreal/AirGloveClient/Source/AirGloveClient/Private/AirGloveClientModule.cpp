#include "AirGloveClientLog.h"
#include "AirGloveClientTypes.h"
#include "Modules/ModuleManager.h"

DEFINE_LOG_CATEGORY(LogAirGloveClient);

namespace AirGloveClient
{
	const TArray<FName>& JointNames()
	{
		static const TArray<FName> Names = {
			"Palm", "Wrist",
			"ThumbMetacarpal", "ThumbProximal", "ThumbDistal", "ThumbTip",
			"IndexMetacarpal", "IndexProximal", "IndexIntermediate", "IndexDistal", "IndexTip",
			"MiddleMetacarpal", "MiddleProximal", "MiddleIntermediate", "MiddleDistal", "MiddleTip",
			"RingMetacarpal", "RingProximal", "RingIntermediate", "RingDistal", "RingTip",
			"LittleMetacarpal", "LittleProximal", "LittleIntermediate", "LittleDistal", "LittleTip",
		};
		return Names;
	}

	const TArray<int32>& JointParents()
	{
		static const TArray<int32> Parents = {
			1, -1,
			1, 2, 3, 4,
			1, 6, 7, 8, 9,
			1, 11, 12, 13, 14,
			1, 16, 17, 18, 19,
			1, 21, 22, 23, 24,
		};
		return Parents;
	}

	FVector ToUnrealPosition(float X, float Y, float Z)
	{
		return FVector(-Z, X, Y) * 100.0;
	}

	FQuat ToUnrealRotation(float QX, float QY, float QZ, float QW)
	{
		return FQuat(-QZ, QX, QY, -QW);
	}
}

IMPLEMENT_MODULE(FDefaultModuleImpl, AirGloveClient)
