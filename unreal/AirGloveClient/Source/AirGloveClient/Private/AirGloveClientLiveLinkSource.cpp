#include "AirGloveClientLiveLinkSource.h"

#include "AirGloveClientTypes.h"
#include "ILiveLinkClient.h"
#include "Roles/LiveLinkAnimationRole.h"
#include "Roles/LiveLinkAnimationTypes.h"

#define LOCTEXT_NAMESPACE "AirGloveClientLiveLink"

FAirGloveClientLiveLinkSource::FAirGloveClientLiveLinkSource(FName InLeftSubject, FName InRightSubject)
{
	Subjects[0] = InLeftSubject;
	Subjects[1] = InRightSubject;
}

void FAirGloveClientLiveLinkSource::ReceiveClient(ILiveLinkClient* InClient, FGuid InSourceGuid)
{
	Client = InClient;
	SourceGuid = InSourceGuid;
	bStaticSent[0] = bStaticSent[1] = false;
}

bool FAirGloveClientLiveLinkSource::RequestSourceShutdown()
{
	Client = nullptr;
	return true;
}

FText FAirGloveClientLiveLinkSource::GetSourceType() const
{
	return LOCTEXT("Type", "AirGloveClient");
}

FText FAirGloveClientLiveLinkSource::GetSourceMachineName() const
{
	return LOCTEXT("Machine", "Spine (localhost)");
}

FText FAirGloveClientLiveLinkSource::GetSourceStatus() const
{
	return LOCTEXT("Status", "Active");
}

void FAirGloveClientLiveLinkSource::PushHand(int32 Side, const TArray<FTransform>& WristRelative, double WorldTime)
{
	if (Client == nullptr || WristRelative.Num() != AirGloveClient::JointCount)
	{
		return;
	}
	const FLiveLinkSubjectKey Key(SourceGuid, Subjects[Side]);
	if (!bStaticSent[Side])
	{
		FLiveLinkStaticDataStruct Static(FLiveLinkSkeletonStaticData::StaticStruct());
		FLiveLinkSkeletonStaticData& Skeleton = *Static.Cast<FLiveLinkSkeletonStaticData>();
		Skeleton.SetBoneNames(AirGloveClient::JointNames());
		Skeleton.SetBoneParents(AirGloveClient::JointParents());
		Client->PushSubjectStaticData_AnyThread(Key, ULiveLinkAnimationRole::StaticClass(), MoveTemp(Static));
		bStaticSent[Side] = true;
	}
	FLiveLinkFrameDataStruct Frame(FLiveLinkAnimationFrameData::StaticStruct());
	FLiveLinkAnimationFrameData& Data = *Frame.Cast<FLiveLinkAnimationFrameData>();
	const TArray<int32>& Parents = AirGloveClient::JointParents();
	Data.Transforms.SetNum(AirGloveClient::JointCount);
	for (int32 J = 0; J < AirGloveClient::JointCount; ++J)
	{
		Data.Transforms[J] = Parents[J] < 0 ? WristRelative[J] : WristRelative[J].GetRelativeTransform(WristRelative[Parents[J]]);
	}
	Data.WorldTime = FLiveLinkWorldTime(WorldTime);
	Client->PushSubjectFrameData_AnyThread(Key, MoveTemp(Frame));
}

#undef LOCTEXT_NAMESPACE
