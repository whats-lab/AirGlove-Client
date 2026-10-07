#pragma once

#include "AirGloveClientTypes.h"
#include "Animation/AnimInstance.h"
#include "CoreMinimal.h"
#include "AirGloveClientHandAnimInstance.generated.h"

UCLASS(Blueprintable)
class AIRGLOVECLIENT_API UAirGloveClientHandAnimInstance : public UAnimInstance
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AirGloveClient")
	EAirGloveClientSide Side = EAirGloveClientSide::Left;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AirGloveClient")
	FName SubjectName;

protected:
	virtual FAnimInstanceProxy* CreateAnimInstanceProxy() override;
	virtual void DestroyAnimInstanceProxy(FAnimInstanceProxy* InProxy) override;
};
