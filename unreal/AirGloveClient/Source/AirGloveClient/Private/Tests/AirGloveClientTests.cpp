#include "AirGloveClientSettings.h"
#include "AirGloveClientSubsystem.h"
#include "AirGloveClientTypes.h"
#include "Engine/Engine.h"
#include "Features/IModularFeatures.h"
#include "HAL/PlatformProcess.h"
#include "ILiveLinkClient.h"
#include "Misc/AutomationTest.h"
#include "Tests/AutomationCommon.h"
#include "Misc/FileHelper.h"
#include "HAL/PlatformMisc.h"
#include "Roles/LiveLinkAnimationRole.h"
#include "Roles/LiveLinkAnimationTypes.h"

#if WITH_DEV_AUTOMATION_TESTS

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FAirGloveClientConversionTest, "AirGloveClient.Conversion",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FAirGloveClientConversionTest::RunTest(const FString& Parameters)
{
	auto ToUnrealVector = [](const FVector& V) { return FVector(-V.Z, V.X, V.Y); };
	FRandomStream Random(7);
	for (int32 I = 0; I < 100; ++I)
	{
		FQuat Q(Random.FRandRange(-1, 1), Random.FRandRange(-1, 1), Random.FRandRange(-1, 1), Random.FRandRange(-1, 1));
		Q.Normalize();
		const FVector V(Random.FRandRange(-1, 1), Random.FRandRange(-1, 1), Random.FRandRange(-1, 1));
		const FVector Expected = ToUnrealVector(Q.RotateVector(V));
		const FVector Actual = AirGloveClient::ToUnrealRotation(Q.X, Q.Y, Q.Z, Q.W).RotateVector(ToUnrealVector(V));
		if (!Expected.Equals(Actual, 1e-5))
		{
			AddError(FString::Printf(TEXT("rotation basis mismatch: %s vs %s"), *Expected.ToString(), *Actual.ToString()));
			return false;
		}
	}
	TestTrue(TEXT("bone axis -Z maps to +X"), AirGloveClient::ToUnrealPosition(0, 0, -0.01f).Equals(FVector(1, 0, 0), 1e-6));
	TestEqual(TEXT("joint count"), AirGloveClient::JointNames().Num(), AirGloveClient::JointCount);
	TestEqual(TEXT("parent count"), AirGloveClient::JointParents().Num(), AirGloveClient::JointCount);
	return true;
}

namespace
{
	void CheckHands(FAutomationTestBase* Test, UAirGloveClientSubsystem* Subsystem)
	{
		ILiveLinkClient& LiveLink = IModularFeatures::Get().GetModularFeature<ILiveLinkClient>(ILiveLinkClient::ModularFeatureName);
		const UAirGloveClientSettings* Settings = GetDefault<UAirGloveClientSettings>();
		for (EAirGloveClientSide Side : { EAirGloveClientSide::Left, EAirGloveClientSide::Right })
		{
			const FAirGloveClientHandPose Pose = Subsystem->GetHandPose(Side);
			const TCHAR* Name = Side == EAirGloveClientSide::Left ? TEXT("left") : TEXT("right");
			Test->TestTrue(FString::Printf(TEXT("%s tracked"), Name), Pose.bTracked);
			Test->TestEqual(FString::Printf(TEXT("%s joints"), Name), Pose.Joints.Num(), AirGloveClient::JointCount);
			int32 ValidCount = 0;
			for (bool bValid : Pose.Valid)
			{
				ValidCount += bValid ? 1 : 0;
			}
			Test->TestEqual(FString::Printf(TEXT("%s valid joints"), Name), ValidCount, AirGloveClient::JointCount);
			Test->TestTrue(FString::Printf(TEXT("%s wrist identity"), Name), Pose.Joints[1].Equals(FTransform::Identity, 1e-3));
			const double Reach = Pose.Joints[static_cast<int32>(EAirGloveClientJoint::MiddleTip)].GetLocation().Size();
			Test->AddInfo(FString::Printf(TEXT("%s seq %d, wrist-to-middle-tip %.2f cm"), Name, Pose.Sequence, Reach));
			const FVector Metacarpal = Pose.Joints[static_cast<int32>(EAirGloveClientJoint::MiddleMetacarpal)].GetLocation();
			const FVector Proximal = Pose.Joints[static_cast<int32>(EAirGloveClientJoint::MiddleProximal)].GetLocation();
			const FVector BoneAxis = Pose.Joints[static_cast<int32>(EAirGloveClientJoint::MiddleMetacarpal)].GetRotation().GetAxisX();
			Test->TestTrue(FString::Printf(TEXT("%s bone points along +X"), Name),
				FVector::DotProduct(BoneAxis, (Proximal - Metacarpal).GetSafeNormal()) > 0.9);
			const FName Subject = Side == EAirGloveClientSide::Left ? Settings->LeftSubjectName : Settings->RightSubjectName;
			FLiveLinkSubjectFrameData Frame;
			if (!Test->TestTrue(FString::Printf(TEXT("%s live link frame"), Name),
					LiveLink.EvaluateFrame_AnyThread(Subject, ULiveLinkAnimationRole::StaticClass(), Frame)))
			{
				continue;
			}
			const FLiveLinkAnimationFrameData* Data = Frame.FrameData.Cast<FLiveLinkAnimationFrameData>();
			if (!Data || !Test->TestEqual(TEXT("live link bones"), Data->Transforms.Num(), AirGloveClient::JointCount))
			{
				continue;
			}
			const TArray<int32>& Parents = AirGloveClient::JointParents();
			TArray<FTransform> Component;
			Component.SetNum(AirGloveClient::JointCount);
			Component[1] = Data->Transforms[1];
			for (int32 J : { 0, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25 })
			{
				Component[J] = Data->Transforms[J] * Component[Parents[J]];
			}
			const FAirGloveClientHandPose Latest = Subsystem->GetHandPose(Side);
			double Bone = 0.0, LatestBone = 0.0;
			for (int32 J = 2; J < AirGloveClient::JointCount; ++J)
			{
				Bone = FMath::Max(Bone, FMath::Abs((Component[J].GetLocation() - Component[Parents[J]].GetLocation()).Size() * 100.0 -
					(Latest.Joints[J].GetLocation() - Latest.Joints[Parents[J]].GetLocation()).Size()));
			}
			Test->AddInfo(FString::Printf(TEXT("%s live link FK bone-length error %.4f cm"), Name, Bone));
			Test->TestTrue(FString::Printf(TEXT("%s live link skeleton matches pose"), Name), Bone < 0.05);
		}
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FAirGloveClientReplayTest, "AirGloveClient.Replay",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FAirGloveClientReplayTest::RunTest(const FString& Parameters)
{
	UAirGloveClientSubsystem* Subsystem = GEngine ? GEngine->GetEngineSubsystem<UAirGloveClientSubsystem>() : nullptr;
	if (!TestNotNull(TEXT("subsystem"), Subsystem) || !TestTrue(TEXT("start"), Subsystem->Start()))
	{
		return false;
	}
	const double Deadline = FPlatformTime::Seconds() + 5.0;
	ADD_LATENT_AUTOMATION_COMMAND(FFunctionLatentCommand([Subsystem, Deadline]() {
		const bool bBoth = Subsystem->GetHandPose(EAirGloveClientSide::Left).bTracked && Subsystem->GetHandPose(EAirGloveClientSide::Right).bTracked;
		return bBoth || FPlatformTime::Seconds() > Deadline;
	}));
	const FString DumpPath = FPlatformMisc::GetEnvironmentVariable(TEXT("AIRGLOVE_UE_DUMP"));
	const double DumpEnd = FPlatformTime::Seconds() + 15.0;
	TSharedRef<TArray<FString>> Lines = MakeShared<TArray<FString>>();
	TSharedRef<TArray<int32>> LastSeq = MakeShared<TArray<int32>>(TArray<int32>{ -1, -1 });
	ADD_LATENT_AUTOMATION_COMMAND(FFunctionLatentCommand([Subsystem, DumpPath, DumpEnd, Lines, LastSeq]() {
		if (DumpPath.IsEmpty())
		{
			return true;
		}
		for (int32 Side = 0; Side < 2; ++Side)
		{
			const FAirGloveClientHandPose Pose = Subsystem->GetHandPose(static_cast<EAirGloveClientSide>(Side));
			if (!Pose.bTracked || Pose.Sequence == (*LastSeq)[Side])
			{
				continue;
			}
			(*LastSeq)[Side] = Pose.Sequence;
			FString Line = FString::Printf(TEXT("{\"side\":\"%s\",\"seq\":%d,\"joints\":["), Side == 0 ? TEXT("left") : TEXT("right"), Pose.Sequence);
			for (int32 J = 0; J < Pose.Joints.Num(); ++J)
			{
				const FQuat Q = Pose.Joints[J].GetRotation();
				const FVector P = Pose.Joints[J].GetLocation();
				Line += FString::Printf(TEXT("%s[%.9g,%.9g,%.9g,%.9g,%.9g,%.9g,%.9g]"), J ? TEXT(",") : TEXT(""), Q.X, Q.Y, Q.Z, Q.W, P.X, P.Y, P.Z);
			}
			Lines->Add(Line + TEXT("]}"));
		}
		if (FPlatformTime::Seconds() < DumpEnd)
		{
			return false;
		}
		FFileHelper::SaveStringArrayToFile(*Lines, *DumpPath);
		return true;
	}));
	ADD_LATENT_AUTOMATION_COMMAND(FFunctionLatentCommand([this, Subsystem]() {
		CheckHands(this, Subsystem);
		Subsystem->Stop();
		return true;
	}));
	return true;
}

#endif
