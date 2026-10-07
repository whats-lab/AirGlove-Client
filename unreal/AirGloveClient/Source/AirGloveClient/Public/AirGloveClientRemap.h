#pragma once

#include "CoreMinimal.h"
#include "LiveLinkRemapAsset.h"
#include "AirGloveClientRemap.generated.h"

UCLASS(meta = (DisplayName = "AirGlove Client Remap (XRHand_ bones)"))
class AIRGLOVECLIENT_API UAirGloveClientRemap : public ULiveLinkRemapAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, Category = "AirGloveClient")
	FString BonePrefix = TEXT("XRHand_");

	virtual FName GetRemappedBoneName_Implementation(FName BoneName) const override;
};
