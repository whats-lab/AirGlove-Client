#pragma once

#include "CoreMinimal.h"
#include "ILiveLinkSource.h"

class ILiveLinkClient;

class FAirGloveClientLiveLinkSource : public ILiveLinkSource
{
public:
	FAirGloveClientLiveLinkSource(FName InLeftSubject, FName InRightSubject);

	virtual void ReceiveClient(ILiveLinkClient* InClient, FGuid InSourceGuid) override;
	virtual bool IsSourceStillValid() const override { return Client != nullptr; }
	virtual bool RequestSourceShutdown() override;
	virtual FText GetSourceType() const override;
	virtual FText GetSourceMachineName() const override;
	virtual FText GetSourceStatus() const override;

	void PushHand(int32 Side, const TArray<FTransform>& WristRelative, double WorldTime);
	FName SubjectName(int32 Side) const { return Subjects[Side]; }

private:
	ILiveLinkClient* Client = nullptr;
	FGuid SourceGuid;
	FName Subjects[2];
	bool bStaticSent[2] = { false, false };
};
