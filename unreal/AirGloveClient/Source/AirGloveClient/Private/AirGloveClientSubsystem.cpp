#include "AirGloveClientSubsystem.h"

#include "AirGloveClientLiveLinkSource.h"
#include "AirGloveClientLog.h"
#include "AirGloveClientNative.h"
#include "AirGloveClientSettings.h"
#include "Features/IModularFeatures.h"
#include "HAL/PlatformTime.h"
#include "ILiveLinkClient.h"

void UAirGloveClientSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	for (FAirGloveClientHandPose& Pose : Poses)
	{
		Pose.Joints.Init(FTransform::Identity, AirGloveClient::JointCount);
		Pose.Radii.Init(1.f, AirGloveClient::JointCount);
		Pose.Valid.Init(false, AirGloveClient::JointCount);
	}
	if (GetDefault<UAirGloveClientSettings>()->bAutoStart && !IsRunningCommandlet())
	{
		Start();
	}
}

void UAirGloveClientSubsystem::Deinitialize()
{
	Stop();
	Super::Deinitialize();
}

bool UAirGloveClientSubsystem::Start()
{
	if (Client)
	{
		return true;
	}
	const FAirGloveClientNative* Native = FAirGloveClientNative::Get();
	if (!Native)
	{
		return false;
	}
	const UAirGloveClientSettings* Settings = GetDefault<UAirGloveClientSettings>();
	Client = Native->Create(TCHAR_TO_UTF8(*Settings->ListenAddress), Settings->ListenPort,
		TCHAR_TO_UTF8(*Settings->SpineAddress), Settings->SpinePort);
	if (!Client)
	{
		UE_LOG(LogAirGloveClient, Error, TEXT("cannot start: %s"), *Native->Error());
		return false;
	}
	Native->SetStaleThreshold(Client, Settings->StaleSeconds);
	if (Settings->bLiveLink && IModularFeatures::Get().IsModularFeatureAvailable(ILiveLinkClient::ModularFeatureName))
	{
		ILiveLinkClient& LiveLink = IModularFeatures::Get().GetModularFeature<ILiveLinkClient>(ILiveLinkClient::ModularFeatureName);
		LiveLinkSource = MakeShared<FAirGloveClientLiveLinkSource>(Settings->LeftSubjectName, Settings->RightSubjectName);
		LiveLink.AddSource(LiveLinkSource);
	}
	TickHandle = FTSTicker::GetCoreTicker().AddTicker(FTickerDelegate::CreateUObject(this, &UAirGloveClientSubsystem::Tick));
	UE_LOG(LogAirGloveClient, Log, TEXT("listening on %s:%d, Spine %s:%d"), *Settings->ListenAddress, Settings->ListenPort,
		*Settings->SpineAddress, Settings->SpinePort);
	return true;
}

void UAirGloveClientSubsystem::Stop()
{
	if (TickHandle.IsValid())
	{
		FTSTicker::GetCoreTicker().RemoveTicker(TickHandle);
		TickHandle.Reset();
	}
	if (LiveLinkSource && IModularFeatures::Get().IsModularFeatureAvailable(ILiveLinkClient::ModularFeatureName))
	{
		IModularFeatures::Get().GetModularFeature<ILiveLinkClient>(ILiveLinkClient::ModularFeatureName).RemoveSource(LiveLinkSource);
	}
	LiveLinkSource.Reset();
	if (Client)
	{
		FAirGloveClientNative::Get()->Destroy(Client);
		Client = nullptr;
	}
}

bool UAirGloveClientSubsystem::Tick(float DeltaTime)
{
	Update();
	return true;
}

bool UAirGloveClientSubsystem::Update()
{
	if (!Client)
	{
		return false;
	}
	const FAirGloveClientNative* Native = FAirGloveClientNative::Get();
	const float Stale = GetDefault<UAirGloveClientSettings>()->StaleSeconds;
	float Raw[AGC_HAND_FLOATS];
	uint8 Valid[AGC_JOINT_COUNT];
	bool bAny = false;
	for (int32 Side = 0; Side < 2; ++Side)
	{
		int32 Seq = 0;
		int64_t TimeUs = 0;
		double Age = 0.0;
		FAirGloveClientHandPose& Pose = Poses[Side];
		if (Native->GetHand(Client, Side, Raw, AGC_HAND_FLOATS, Valid, &Seq, &TimeUs, &Age) != AGC_FRESH)
		{
			Pose.bTracked = false;
			continue;
		}
		Pose.AgeSeconds = static_cast<float>(Age);
		Pose.bTracked = Age <= Stale;
		const bool bNew = Seq != Pose.Sequence;
		Pose.Sequence = Seq;
		for (int32 J = 0; J < AirGloveClient::JointCount; ++J)
		{
			const float* V = Raw + J * AGC_JOINT_FLOATS;
			Pose.Joints[J] = FTransform(AirGloveClient::ToUnrealRotation(V[0], V[1], V[2], V[3]), AirGloveClient::ToUnrealPosition(V[4], V[5], V[6]));
			Pose.Radii[J] = V[7] * 100.f;
			Pose.Valid[J] = Valid[J] != 0;
		}
		if (bNew && Pose.bTracked && LiveLinkSource)
		{
			const bool bMirror = GetDefault<UAirGloveClientSettings>()->LiveLinkConvention == EAirGloveClientLiveLinkConvention::MirroredY;
			TArray<FTransform> LiveLinkJoints;
			LiveLinkJoints.SetNum(AirGloveClient::JointCount);
			for (int32 J = 0; J < AirGloveClient::JointCount; ++J)
			{
				const float* V = Raw + J * AGC_JOINT_FLOATS;
				LiveLinkJoints[J] = bMirror
					? FTransform(FQuat(V[0], -V[1], V[2], -V[3]), FVector(V[4], -V[5], V[6]))
					: FTransform(FQuat(V[0], V[1], V[2], V[3]), FVector(V[4], V[5], V[6]));
			}
			LiveLinkSource->PushHand(Side, LiveLinkJoints, FPlatformTime::Seconds());
		}
		bAny = true;
	}
	char Code[64];
	int32 AlarmSide = -1;
	while (Native->PollAlarm(Client, Code, sizeof(Code), &AlarmSide) == AGC_FRESH)
	{
		const FString CodeString = UTF8_TO_TCHAR(Code);
		UE_LOG(LogAirGloveClient, Warning, TEXT("alarm %s (side %d)"), *CodeString, AlarmSide);
		OnAlarm.Broadcast(CodeString, AlarmSide);
	}
	return bAny;
}

bool UAirGloveClientSubsystem::GetWristOrientation(EAirGloveClientSide Side, FQuat& Orientation, float& AgeSeconds) const
{
	if (!Client)
	{
		return false;
	}
	float Q[4];
	double Age = 0.0;
	if (FAirGloveClientNative::Get()->GetWrist(Client, static_cast<int32>(Side), Q, nullptr, nullptr, &Age) != AGC_FRESH)
	{
		return false;
	}
	Orientation = AirGloveClient::ToUnrealRotation(Q[0], Q[1], Q[2], Q[3]);
	AgeSeconds = static_cast<float>(Age);
	return true;
}

bool UAirGloveClientSubsystem::GetDeviceStatus(bool& bLeftConnected, bool& bRightConnected) const
{
	int32 L = 0, R = 0;
	if (!Client || FAirGloveClientNative::Get()->GetDeviceStatus(Client, &L, &R, nullptr) != AGC_FRESH)
	{
		return false;
	}
	bLeftConnected = L != 0;
	bRightConnected = R != 0;
	return true;
}

bool UAirGloveClientSubsystem::SetHaptics(EAirGloveClientSide Side, const TArray<int32>& Strengths)
{
	if (!Client || Strengths.Num() == 0)
	{
		return false;
	}
	const int32 Count = FMath::Min(Strengths.Num(), AirGloveClient::FingerCount);
	return FAirGloveClientNative::Get()->SetHaptics(Client, static_cast<int32>(Side), Strengths.GetData(), Count) == AGC_OK;
}

bool UAirGloveClientSubsystem::GetHapticsResult(EAirGloveClientSide Side, bool& bSuccess) const
{
	int32 Ok = 0;
	if (!Client || FAirGloveClientNative::Get()->GetHapticsResult(Client, static_cast<int32>(Side), &Ok, nullptr) != AGC_FRESH)
	{
		return false;
	}
	bSuccess = Ok != 0;
	return true;
}
