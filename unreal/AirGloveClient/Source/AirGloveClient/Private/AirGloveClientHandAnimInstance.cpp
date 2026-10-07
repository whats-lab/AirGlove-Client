#include "AirGloveClientHandAnimInstance.h"

#include "AirGloveClientRemap.h"
#include "AirGloveClientSettings.h"
#include "AnimNode_LiveLinkPose.h"
#include "Animation/AnimInstanceProxy.h"

namespace
{
	struct FAirGloveClientHandProxy : public FAnimInstanceProxy
	{
		explicit FAirGloveClientHandProxy(UAnimInstance* Instance) : FAnimInstanceProxy(Instance) {}

		FAnimNode_LiveLinkPose Node;

		virtual void Initialize(UAnimInstance* InAnimInstance) override
		{
			FAnimInstanceProxy::Initialize(InAnimInstance);
			const UAirGloveClientHandAnimInstance* Hand = CastChecked<UAirGloveClientHandAnimInstance>(InAnimInstance);
			const UAirGloveClientSettings* Settings = GetDefault<UAirGloveClientSettings>();
			const bool bLeft = Hand->Side == EAirGloveClientSide::Left;
			const FName Subject = Hand->SubjectName.IsNone() ? (bLeft ? Settings->LeftSubjectName : Settings->RightSubjectName) : Hand->SubjectName;
			Node.LiveLinkSubjectName = FLiveLinkSubjectName(Subject);
			Node.RetargetAsset = UAirGloveClientRemap::StaticClass();
			FAnimationInitializeContext Context(this);
			Node.Initialize_AnyThread(Context);
		}

		virtual void PreUpdate(UAnimInstance* InAnimInstance, float DeltaSeconds) override
		{
			FAnimInstanceProxy::PreUpdate(InAnimInstance, DeltaSeconds);
			Node.PreUpdate(InAnimInstance);
		}

		virtual void UpdateAnimationNode(const FAnimationUpdateContext& InContext) override
		{
			Node.Update_AnyThread(InContext);
		}

		virtual bool Evaluate(FPoseContext& Output) override
		{
			Output.ResetToRefPose();
			Node.Evaluate_AnyThread(Output);
			return true;
		}
	};
}

FAnimInstanceProxy* UAirGloveClientHandAnimInstance::CreateAnimInstanceProxy()
{
	return new FAirGloveClientHandProxy(this);
}

void UAirGloveClientHandAnimInstance::DestroyAnimInstanceProxy(FAnimInstanceProxy* InProxy)
{
	delete InProxy;
}
